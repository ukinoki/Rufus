/* (C) 2020 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_actesprecedents.h"
#include "ui_dlg_actesprecedents.h"

//Uniquement consultative, cette fiche ne permet de modifier aucun élément de la base
dlg_actesprecedents::dlg_actesprecedents(Patient *pat, QWidget *parent) :
    QDialog(parent),

ui(new Ui::dlg_actesprecedents)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    if (pat == Q_NULLPTR)
    {
        m_initOK = false;
        return;
    }
    m_idpatient = pat->id();
    if (Datas::I()->patients->currentpatient() != Q_NULLPTR)
    m_iscurrentpatient          = (pat == Datas::I()->patients->currentpatient());
    if (m_iscurrentpatient)                                                 /*! la fiche a été appelée depuis le dossier d'un patient, cas le plus fréquent,
                                                                               elle affiche tous les actes sauf le dernier */
    {
        m_actes                 = Datas::I()->actes;
        m_currentpatient        = Datas::I()->patients->currentpatient();
        m_listepaiements        = Datas::I()->lignespaiements;
        m_avantdernieracte      = true;
        map_actes               = m_actes->actes();
        m_initOK                = map_actes->size() > 1;
    }
    else                                                                    /*! la fiche a été appelée depuis le menu contextuel de la liste des patients
                                                                                elle affiche tous les actes */
    {
        m_actes                     = new Actes;
        m_actes                     ->initListeByPatient(pat);
        m_currentpatient            = Datas::I()->patients->getById(m_idpatient);
        m_listepaiements            = new LignesPaiements();
        m_listepaiements            ->initListeByPatient(m_currentpatient);
        m_avantdernieracte          = false;
        map_actes                   = m_actes->actes();
        m_initOK                    = map_actes->size() > 0;
        if (!m_initOK)
        {
            ItemsList::clearAll(m_listepaiements->lignespaiements());
            delete m_listepaiements;
            ItemsList::clearAll(map_actes);
            delete m_actes;
        }
    }
    if (!m_initOK) return;

    setWindowTitle(tr("Consultations précédentes de ") + m_currentpatient->nom() + " " + m_currentpatient->prenom());
    setWindowIcon(Icons::icLoupe());

    QString style = "background-color:qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 230, 250, 50));"
                    " border: 1px solid rgb(150,150,150); border-radius: 10px;";
    ui->EnteteupLabel           ->setStyleSheet(style);
    ui->CorpsupTextEdit         ->setStyleSheet(style);
    ui->ConclusionupTextEdit    ->setStyleSheet(style);
    ui->FermepushButton->installEventFilter(this);

    if (m_avantdernieracte)
        restoreGeometry(proc->settings()->value(Position_Fiche Nom_fiche_ActesPrecedents).toByteArray());
    else
        restoreGeometry(proc->settings()->value(Position_Fiche Nom_fiche_AutreDossier).toByteArray());

    connect (ui->FermepushButton,           &QPushButton::clicked,  this,   &dlg_actesprecedents::close);
    connect (ui->ActeSuivantpushButton,     &QPushButton::clicked,  this,   [=] {NavigationConsult(ItemsList::Suiv);});
    connect (ui->DernierActepushButton,     &QPushButton::clicked,  this,   [=] {NavigationConsult(ItemsList::Fin);});
    connect (ui->PremierActepushButton,     &QPushButton::clicked,  this,   [=] {NavigationConsult(ItemsList::Debut);});
    connect (ui->ActePrecedentpushButton,   &QPushButton::clicked,  this,   [=] {NavigationConsult(ItemsList::Prec);});

    ui->FermepushButton->setShortcut(QKeySequence("Meta+Return"));
    proc->ModifTailleFont(ui->RenseignementsWidget, -3);
    Actualise();
}

dlg_actesprecedents::~dlg_actesprecedents()
{
    if (!m_iscurrentpatient)
    {
        ItemsList::clearAll(m_listepaiements->lignespaiements());
        delete m_listepaiements;
        ItemsList::clearAll(map_actes);
        delete m_actes;
    }
    delete ui;
}

/*!
 * \brief dlg_actesprecedents::Actualise
 * Cette fonction est appelée par Rufus.cpp
 * quand un acte est créé ou supprimé
 */
void dlg_actesprecedents::Actualise()
{
    int n = (m_iscurrentpatient? 1 : 0);
    if( map_actes->size() == n )
         close();
    int initScrollValue;
    it_currentacte = map_actes->constFind(map_actes->lastKey());
    if (m_iscurrentpatient)
        it_currentacte --;

    ui->ScrollBar->disconnect();
    ui->ScrollBar->setMinimum(0);
    n = (m_iscurrentpatient? 2 : 1);
    initScrollValue = map_actes->size()-n+1;
    ui->ScrollBar->setMaximum(map_actes->size()-n);
    ui->ScrollBar->setSingleStep(1);
    ui->ScrollBar->setValue(initScrollValue);
    ui->ScrollBar->setVisible(map_actes->size()>1);
    ActesPrecsAfficheActe();

    if( ui->ScrollBar->maximum() > 0 )
        connect(ui->ScrollBar, &QScrollBar::valueChanged, this, [=](int newValue) {
            QList<int> listid = map_actes->keys();
            it_currentacte = map_actes->constFind(listid.at(newValue));
            ActesPrecsAfficheActe();
        });
}

//Je n'arrives pas à le tester, les touches se sont pas capturées
//sur ma VM Linux
//J'ai toutes les touches sauf les fleches, page up et page down.
void dlg_actesprecedents::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Down)
    {
        if (ui->ScrollBar->value() >= ui->ScrollBar->maximum())
            return;

        //++it_currentacte;
        if( it_currentacte != map_actes->constEnd())
            ++it_currentacte;

        ActesPrecsAfficheActe();
    }

    if (keyEvent->key() == Qt::Key_Up)
    {
        if (ui->ScrollBar->value() <= ui->ScrollBar->minimum())
            return;

//        --it_currentacte;
//        if( it_currentacte == Q_NULLPTR)
//            it_currentacte = map_actes->constBegin();

        if( it_currentacte != map_actes->constBegin())
            --it_currentacte;

        ActesPrecsAfficheActe();
    }


    if (keyEvent->key() == Qt::Key_PageDown)
    {
        /*if (ui->ScrollBar->value()+ui->ScrollBar->pageStep() < ui->ScrollBar->maximum())
            ActesPrecsAfficheActe(listactes.at(ui->ScrollBar->value()+ui->ScrollBar->pageStep()));
        else
            ActesPrecsAfficheActe(listactes.last());*/
    }

    if (keyEvent->key() == Qt::Key_PageUp)
    {
        /*if (ui->ScrollBar->value()-ui->ScrollBar->pageStep() > ui->ScrollBar->minimum())
            ActesPrecsAfficheActe(listactes.at(ui->ScrollBar->value()-ui->ScrollBar->pageStep()));
        else
            ActesPrecsAfficheActe(listactes.first());*/
    }

}

void dlg_actesprecedents::wheelEvent(QWheelEvent *event)
{
    /* la roulette de la souris fait défiler les actes.
     * On neutralise la fonction si le curseur est dans ui->CorpsupTextEdit ou ui->ConclusionupTextEdit
     * pour pouvoir utiliser à la souris les ascenseurs éventuels dans ces zones
    */
    if (!ui->CorpsupTextEdit->underMouse() && !ui->ConclusionupTextEdit->underMouse())
    {
        int deplacemtsouris = event->angleDelta().y();
        if( deplacemtsouris > 0 ) //on affiche l'acte précédent
            NavigationConsult(ItemsList::Prec);
        else //on affiche l'acte suivant
            NavigationConsult(ItemsList::Suiv);
    }
}

void dlg_actesprecedents::closeEvent(QCloseEvent *event)
{
    if (m_avantdernieracte)
        proc->settings()->setValue(Position_Fiche Nom_fiche_ActesPrecedents, saveGeometry());
    else
        proc->settings()->setValue(Position_Fiche Nom_fiche_AutreDossier, saveGeometry());
    event->accept();
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------
bool dlg_actesprecedents::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->FermepushButton)
    {
        QPushButton* Button = qobject_cast<QPushButton*>(obj);
        if(event->type() == QEvent::MouseButtonPress)
            Button->setIcon(Icons::icFermeAppuye());
        if(event->type() == QEvent::MouseButtonRelease)
            Button->setIcon(Icons::icFerme());
        if(event->type() == QEvent::MouseMove)
        {
            QRect rect = QRect(Button->mapToGlobal(QPoint(0,0)),Button->size());
            QPoint pos = cursor().pos();
            if (rect.contains(pos))
                Button->setIcon(Icons::icFermeAppuye());
            else
                Button->setIcon(Icons::icFerme());
        }
        return false;
    }
    return dlg_actesprecedents::eventFilter(obj, event);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher les champs ---------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_actesprecedents::ActesPrecsAfficheActe(Acte *acte)
{
    it_currentacte = map_actes->constFind(acte->id());
    if( it_currentacte == map_actes->constEnd() )
        return;
    ActesPrecsAfficheActe();
}

void dlg_actesprecedents::ActesPrecsAfficheActe()
{
    Acte *acte = it_currentacte.value();
    if( acte == Q_NULLPTR )    // Aucune consultation trouvee pour ces criteres
        return;
    User * usr = Datas::I()->users->getById(acte->idUserSuperviseur());
    QString nomcomplet(""), login ("");
    if (usr != Q_NULLPTR)
    {
        nomcomplet =  usr->prenom() + " " + usr->nom();
        login = usr->login();
    }

    ui->idPatientlineEdit->setText(QString::number(acte->idPatient()));

    //1.  Afficher les éléments de la tables Actes
    ui->EnteteupLabel->clear();
    ui->CorpsupTextEdit->clear();
    ui->ConclusionupTextEdit->clear();
    Site *sit = Datas::I()->sites->getById(acte->idsite());
    QMap<QString,QVariant> mapage = Utils::CalculAge(m_currentpatient->datedenaissance(), acte->date());
    QString age = mapage["toString"].toString();
    QString textHTML = "<p style = \"margin-top:0px; margin-bottom:10px;\">"
                      "<td width=\"130\"><font color = \"" COULEUR_TITRES "\" ><u><b>" + QLocale::system().toString(acte->date(),tr("d MMMM yyyy")) + "</b></u></font></td>"
                      "<td width=\"60\">" + age + "</td>"
                      "<td width=\"300\">" + nomcomplet + " - <font color = \"" COULEUR_TITRES "\" ><b>" + (sit? sit->nom() : "") + "</b></font></td></p>";
    ui->EnteteupLabel->setText(textHTML);
    if( acte->motif().size() || acte->texte().size() || acte->conclusion().size() )
    {
        textHTML = "";
        if( acte->motif().size() )
        {
            QString texte = acte->motif();
            Utils::convertHTML(texte);
            textHTML += HTML_RETOURLIGNE "<td width=\"450\"><font color = \"" COULEUR_TITRES "\">" + tr("MOTIF") + "</font></td></p>";
            textHTML += "<p style = \"margin-top:0px; margin-bottom:10px;\"><td width=\"10\"></td><td width=\"(550\">" + texte + "</td></p>";
        }

        if( acte->texte().size() )
        {
            QString texte = acte->texte();
            Utils::convertHTML(texte);
            textHTML += HTML_RETOURLIGNE "<td width=\"450\"><font color = \"" COULEUR_TITRES "\">" + tr("EXAMEN") + "</font></td></p>";
            textHTML += HTML_RETOURLIGNE "<td width=\"10\"></td><td width=\"550\">" + texte + "</td></p>";
        }
        ui->CorpsupTextEdit->setText(textHTML);
        textHTML = "";

        if( acte->conclusion().size() )
        {
            QString texte = acte->conclusion();
            if( acte->motif().size() || acte->texte().size() )
                textHTML += HTML_RETOURLIGNE "<td width=\"450\"><font color = \"" COULEUR_TITRES "\">" + tr("CONCLUSION") + "</font></td></p>";
            textHTML += HTML_RETOURLIGNE "<td width=\"10\"></td><td width=\"550\">" + texte + "</td></p>";
        }
        ui->ConclusionupTextEdit->setText(textHTML);
        textHTML = "";
    }

    ui->CourrierAFairecheckBox->setChecked(acte->courrierAFaire());
    ui->idActelineEdit->setText(QString::number(acte->id()));

    //2. retrouver le créateur de l'acte
    //idUser = ActesPrecsQuery.value(2).toInt();
    QString createurconsult ("");
    if (Datas::I()->users->getById(acte->idCreatedBy()) != Q_NULLPTR)
        createurconsult = tr("Créé par ")
                          + Datas::I()->users->getById(acte->idCreatedBy())->login()
                          + tr(" pour ") + login; //Avant idPatient
    ui->CreateurConsultlineEdit->setText(createurconsult); //Avant idPatient
    ui->SitelineEdit->setText(sit? sit->nom() : "");

    //3. Mettre à jour le numéro d'acte
    QList<int> listid = map_actes->keys();
    if (m_iscurrentpatient)
        listid.removeLast();
    if( map_actes->size() > 1 )
    {
        int scrolPos = listid.indexOf(acte->id());
        ui->ScrollBar->setValue(scrolPos);
    }
    int n = (m_iscurrentpatient? 2 : 1);
    bool canprec = (map_actes->size() > 1 && listid.indexOf(acte->id()) > 0);
    ui->ActePrecedentpushButton->setEnabled(canprec);

    bool cansui = (map_actes->size() > 1 && listid.indexOf(acte->id()) < map_actes->size() - n);
    ui->ActeSuivantpushButton->setEnabled(cansui);

    bool canfirst = (map_actes->size() > 1 && listid.indexOf(acte->id()) > 0);
    ui->PremierActepushButton->setEnabled(canfirst);

    bool canlast = (map_actes->size() > 1 && listid.indexOf(acte->id()) < map_actes->size() - n);
    ui->DernierActepushButton->setEnabled(canlast);

    ui->NoActelabel->setText(QString::number(listid.indexOf(acte->id()) + 1) + " / " + QString::number(map_actes->size()));

    //4. Afficher les renseignements comptables
    ui->ActeCotationlineEdit->setText(acte->cotation());
    if( acte->isFactureEnFranc() )
    {
        ui->ActeMontantLabel->setFixedHeight(30);
        ui->ActeMontantLabel->setText("Montant (€)\n(payé en F)");
    }
    else
    {
        ui->ActeMontantLabel->setFixedHeight(15);
        ui->ActeMontantLabel->setText("Montant (€)");
    }
    ui->ActeMontantlineEdit->setText( QLocale().toString(acte->montant(),'f',2) ) ;

    if (acte->paiementType().isEmpty() )
    {
        ui->Comptaframe->setVisible(false);
    }
    else
    {
        ui->Comptaframe->setVisible(true);
        ui->PaiementlineEdit->setVisible(true);

        //2. on recheche ensuite le type de paiement : espèces, chèque, tiers, cb, impayé, gratuit
        QString txtpaiement = Utils::ConvertitModePaiement(acte->paiementType());
        if (acte->paiementType() == TRS
                && acte->paiementTiers() == "CB") txtpaiement = tr(CARTECREDIT);
         else if (acte->paiementType() == TRS) txtpaiement = acte->paiementTiers();
        ui->PaiementlineEdit->setText(txtpaiement);

        // on calcule le montant payé pour l'acte
        if (acte->paiementType() != GRAT || acte->paiementType() != IMP)
        {
            double TotalPaye = 0;
            for (auto it = m_listepaiements->lignespaiements()->constBegin(); it != m_listepaiements->lignespaiements()->constEnd(); ++it)
            {
                LignePaiement *lign = const_cast<LignePaiement*>(it.value());
                if (lign->idacte() == acte->id())
                {
                    if (lign->monnaie() == "F")
                        TotalPaye += lign->paye() / 6.55957;
                    else
                        TotalPaye += lign->paye();
                }
            }
            ui->PayelineEdit->setText(QLocale().toString(TotalPaye,'f',2));
        }


        if ((acte->paiementType() == TRS && acte->paiementTiers() == "CB")
              || acte->paiementType() == CHQ
              || acte->paiementType() == ESP)
        {
            ui->PaiementLabel->setVisible(true);
            ui->PaiementLabel->setText("Paiement:");
            ui->PayeLabel->setVisible(true);
            ui->PayelineEdit->setVisible(true);
            ui->Comptaframe->setFixedHeight(74);
            ui->PaiementlineEdit->setGeometry(79,25,91,18);
        }
        else if (acte->paiementType() == GRAT
                 || acte->paiementType() == IMP)
        {
            ui->PaiementLabel->setVisible(false);
            ui->PayeLabel->setVisible(false);
            ui->PayelineEdit->setVisible(false);
            ui->Comptaframe->setFixedHeight(50);
            ui->PaiementlineEdit->setGeometry(8,25,164,18);
        }
        if (acte->paiementType() == TRS && acte->paiementTiers() != "CB")
        {
            ui->PaiementLabel->setText("Tiers");
            ui->PaiementLabel->setVisible(true);
            ui->PaiementlineEdit->setVisible(true);
            ui->PayeLabel->setVisible(true);
            ui->PayelineEdit->setVisible(true);
            ui->Comptaframe->setFixedHeight(74);
            ui->PaiementlineEdit->setGeometry(79,25,91,18);
        }
    }
}


/*------------------------------------------------------------------------------------------------------------------------------------
-- Retrouver l'acte à afficher -------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
Acte* dlg_actesprecedents::currentacte() const
{
    return it_currentacte.value();
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation précédente ou suivante d'un patient ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_actesprecedents::NavigationConsult(ItemsList::POSITION i)
{
    if( m_actes->actes()->size() == 1 )
        return true;

    int idActe = -1;
    if (i == ItemsList::Suiv)
    {
        if( it_currentacte != std::prev(map_actes->constEnd(),2) )
            ++it_currentacte;
    }
    else if (i == ItemsList::Prec)
    {
        if( it_currentacte != map_actes->constBegin() )
            --it_currentacte;
    }
    else if (i == ItemsList::Debut)
    {
        it_currentacte = map_actes->constBegin();
    }
    else if (i == ItemsList::Fin)
    {
        it_currentacte = map_actes->constFind(map_actes->lastKey());
        if (m_iscurrentpatient)
            it_currentacte--;
    }

    idActe = it_currentacte.value()->id();
    if (idActe > -1)
    {
        ActesPrecsAfficheActe();
        return true;
    }

    return false;
}

int dlg_actesprecedents::idcurrentpatient() const
{
    return m_idpatient;
}

