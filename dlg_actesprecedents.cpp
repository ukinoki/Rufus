/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cls_user.h"
#include "dlg_actesprecedents.h"
#include "icons.h"
#include "ui_dlg_actesprecedents.h"

//TODO : Mettre en place Patient
//Uniquement consultatif
dlg_actesprecedents::dlg_actesprecedents(int idPatient, int idActeAPasser, Procedures *procAPAsser, bool AvantDernier, QWidget *parent) :
    QDialog(parent),

ui(new Ui::dlg_actesprecedents)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint);
    gidPatient      = idPatient;
    gidActe         = idActeAPasser; //jamais utilisé
    proc            = procAPAsser;
    gAvantDernier   = AvantDernier;
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    QString style = "background-color:qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 230, 250, 50));"
                    " border: 1px solid rgb(150,150,150); border-radius: 10px;";
    ui->EnteteupLabel           ->setStyleSheet(style);
    ui->CorpsupTextEdit         ->setStyleSheet(style);
    ui->ConclusionupTextEdit    ->setStyleSheet(style);
    ui->FermepushButton->installEventFilter(this);

    if (gAvantDernier)
        restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionActesPrec").toByteArray());
    else
        restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionAutreDossier").toByteArray());

    connect (ui->FermepushButton,                       &QPushButton::clicked,                                  [=] {close();});
    connect (ui->ActePrecedentpushButton,               &QPushButton::clicked,                                  [=] {NavigationConsult(Prec);});
    connect (ui->ActeSuivantpushButton,                 &QPushButton::clicked,                                  [=] {NavigationConsult(Suiv);});
    connect (ui->DernierActepushButton,                 &QPushButton::clicked,                                  [=] {NavigationConsult(Fin);});
    connect (ui->PremierActepushButton,                 &QPushButton::clicked,                                  [=] {NavigationConsult(Debut);});

    ui->FermepushButton->setShortcut(QKeySequence("Meta+Return"));
    proc->ModifTailleFont(ui->RenseignementsWidget, -3);

    Actualise();
}

dlg_actesprecedents::~dlg_actesprecedents()
{
    delete ui;
    proc = 0;
    delete proc;
}

/*!
 * \brief dlg_actesprecedents::Actualise
 * Cette function est appelée par Rufus.cpp
 * quand un acte est créé ou supprimé
 */
void dlg_actesprecedents::Actualise()
{
    //TODO : utiliser actes de la classe Patient car il sera toujours à jours.

    //TODO modifier
    m_listeActes = DataBase::getInstance()->loadActesByIdPat(gidPatient);
    if( m_listeActes.size() == 0 )
    {
        //ERROR
        //tr("Impossible de retrouver la dernière consultation")
        return;
    }

    int initScrollValue;
    itCurrentActe = m_listeActes.find(m_listeActes.lastKey());
    initScrollValue = m_listeActes.size() - 1;
    if( gAvantDernier )
    {
        --initScrollValue;
        --itCurrentActe;
        if( itCurrentActe == 0 )
        {
            initScrollValue = 0;
            itCurrentActe = m_listeActes.constBegin();
        }
    }
    disconnect(ui->ScrollBar, 0, this, 0);
    ui->ScrollBar->setMinimum(0);
    ui->ScrollBar->setMaximum(m_listeActes.size() - 1);
    ui->ScrollBar->setSingleStep(1);
    ui->ScrollBar->setValue(initScrollValue);
    ActesPrecsAfficheActe( );

    if( ui->ScrollBar->maximum() > 0 )
        connect(ui->ScrollBar, &QScrollBar::valueChanged, this, [=](int newValue) {
            itCurrentActe = m_listeActes.find( m_listeActes.keys().at(newValue) );
            ActesPrecsAfficheActe();
        });
}

//JAMAIS ECOUTE
//JAMAIS UTILISE
void dlg_actesprecedents::keyPressEvent(QKeyEvent *keyEvent)
{
    return;
    if (keyEvent->key() == Qt::Key_Down)
    {
        if (ui->ScrollBar->value() >= ui->ScrollBar->maximum())
            return;

        ++itCurrentActe;
        ActesPrecsAfficheActe();
    }

    if (keyEvent->key() == Qt::Key_Up)
    {
        if (ui->ScrollBar->value() <= ui->ScrollBar->minimum())
            return;

        --itCurrentActe;
        if( itCurrentActe == 0 )
            itCurrentActe = m_listeActes.constBegin();
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
        //int pas = 10;
        int deplacemtsouris = event->angleDelta().y();
        if( deplacemtsouris > 0 ) //on affiche l'acte précédent
            NavigationConsult(Prec);
        else //on affiche l'acte suivant
            NavigationConsult(Suiv);
        /*
        //pas = pas + pas/deplacemtsouris;  // tentative de gérer l'accélération
        if (deplacemtsouris!=0 && fabs(deplacemtsouris)<pas)
            deplacemtsouris = pas*(fabs(deplacemtsouris)/deplacemtsouris);
        int destination = ui->ScrollBar->value() - deplacemtsouris/pas;
        if (destination < ui->ScrollBar->minimum() || destination > ui->ScrollBar->maximum())
        {
            if( event->angleDelta().y() <= 0 )
                itCurrentActe = m_listeActes.find(m_listeActes.lastKey());
            else
                itCurrentActe = m_listeActes.constBegin();
            ActesPrecsAfficheActe();
        }
        else
        {
            itCurrentActe = m_listeActes.find( m_listeActes.keys().at(destination) );
            ActesPrecsAfficheActe();
        }
        */
    }
}

void dlg_actesprecedents::closeEvent(QCloseEvent *event)
{
    if (gAvantDernier)
        proc->gsettingsIni->setValue("PositionsFiches/PositionActesPrec", saveGeometry());
    else
        proc->gsettingsIni->setValue("PositionsFiches/PositionAutreDossier", saveGeometry());
    event->accept();
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------
bool dlg_actesprecedents::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->FermepushButton)
    {
        QPushButton* Button = static_cast<QPushButton*>(obj);
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
void dlg_actesprecedents::ActesPrecsAfficheActe(int idActeAAfficher)
{
    itCurrentActe = m_listeActes.find(idActeAAfficher);
    if( itCurrentActe == m_listeActes.constEnd() )
    {
        //ERROR
    }
    //acte = itCurrentActe.value();//DataBase::getInstance()->loadActeById(idActeAAfficher);
    ActesPrecsAfficheActe();
}
void dlg_actesprecedents::ActesPrecsAfficheActe()
{
    acte = itCurrentActe.value();
    if( !acte->isValid() )    // Aucune consultation trouvee pour ces criteres
        return;

    ui->idPatientlineEdit->setText(QString::number(acte->idPatient()));

    //1.  Afficher les éléments de la tables Actes
    ui->EnteteupLabel->clear();
    ui->CorpsupTextEdit->clear();
    ui->ConclusionupTextEdit->clear();

    if( acte->motif().size() || acte->texte().size() || acte->conclusion().size() )
    {
        User *user = proc->getUserById(acte->idUser());
        QString textHTML = "<p style = \"margin-top:0px; margin-bottom:10px;\">"
                          "<td width=\"130\"><font color = \"" + proc->CouleurTitres + "\" ><u><b>" + acte->date().toString(tr("d MMMM yyyy")) + "</b></u></font></td>"
                          "<td width=\"60\">" + Item::CalculAge(acte->agePatient(), acte->date())["toString"].toString() + "</td>"
                          "<td width=\"400\">" + user->getPrenom() + " " + user->getNom() + "</td></p>";
        ui->EnteteupLabel->setText(textHTML);
        textHTML = "";
        if( acte->motif().size() )
        {
            QString texte = Utils::convertHTML(acte->motif());
            textHTML += "<p style = \"margin-top:0px; margin-bottom:0px;\"><td width=\"450\"><font color = \"" + proc->CouleurTitres + "\">" + tr("MOTIF") + "</font></td></p>";
            textHTML += "<p style = \"margin-top:0px; margin-bottom:10px;\"><td width=\"10\"></td><td width=\"450\">" + texte + "</td></p>";
        }

        if( acte->texte().size() )
        {
            QString texte = Utils::convertHTML(acte->texte());
            textHTML += "<p style = \"margin-top:0px; margin-bottom:0px;\" ><td width=\"450\"><font color = \"" + proc->CouleurTitres + "\">" + tr("EXAMEN") + "</font></td></p>";
            textHTML += "<p style = \"margin-top:0px; margin-bottom:10px;\" ><td width=\"10\"></td><td width=\"450\">" + texte + "</td></p>";
        }
        ui->CorpsupTextEdit->setText(textHTML);
        textHTML = "";

        if( acte->conclusion().size() )
        {
            QString texte = Utils::convertHTML(acte->conclusion());
            if( acte->motif().size() || acte->texte().size() )
                textHTML += "<p style = \"margin-top:0px; margin-bottom:0px;\" ><td width=\"450\"><font color = \"" + proc->CouleurTitres + "\">" + tr("CONCLUSION") + "</font></td></p>";
            textHTML += "<p style = \"margin-top:0px; margin-bottom:10px;\" ><td width=\"10\"></td><td width=\"450\">" + texte + "</td></p>";
        }
        ui->ConclusionupTextEdit->setText(textHTML);
        textHTML = "";
    }

    ui->CourrierAFairecheckBox->setChecked(acte->courrierAFaire());
    ui->idActelineEdit->setText(QString::number(acte->id()));

    //2. retrouver le créateur de l'acte
    //idUser = ActesPrecsQuery.value(2).toInt();
    ui->CreateurConsultlineEdit->setText(tr("Créé par ") + proc->getLogin(acte->idCreatedBy() ) +
                                         tr(" pour ") + proc->getLogin(acte->idUser())); //Avant idPatient

    //3. Mettre à jour le numéro d'acte
    if( acte->nbActes() > 1 )
    {
        //disconnect(ui->ScrollBar, 0, this, 0);
        int scrolPos = m_listeActes.keys().indexOf(acte->id());
        ui->ScrollBar->setValue(scrolPos);
        /*connect(ui->ScrollBar, &QScrollBar::valueChanged, this, [=](int newValue) {
            itCurrentActe = m_listeActes.find( m_listeActes.keys().at(newValue) );
            ActesPrecsAfficheActe();
        });*/
    }

    bool canprec = (acte->nbActes() > 1 && acte->noActe() > 1);
    ui->ActePrecedentpushButton->setEnabled(canprec);

    bool cansui = (acte->nbActes() > 1 && acte->noActe() < acte->nbActes());
    ui->ActeSuivantpushButton->setEnabled(cansui);

    bool canfirst = (acte->nbActes() > 1 && acte->noActe() > 1);
    ui->PremierActepushButton->setEnabled(canfirst);

    bool canlast = (acte->nbActes() > 1 && acte->noActe() < acte->nbActes());
    ui->DernierActepushButton->setEnabled(canlast);

    ui->NoActelabel->setText(QString::number(acte->noActe()) + " / " + QString::number(acte->nbActes()));

    //4. Afficher les renseignements comptables
    ui->ActeCotationlineEdit->setText(acte->cotation());
    if( acte->isPayeEnFranc() )
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
        if (acte->paiementType() == "C") ui->PaiementlineEdit->setText(tr("chèque"));
        else if (acte->paiementType() == "E") ui->PaiementlineEdit->setText(tr("espèces"));
        else if (acte->paiementType() == "B") ui->PaiementlineEdit->setText(tr("carte de crédit"));
        else if (acte->paiementType() == "G") ui->PaiementlineEdit->setText(tr("Acte gratuit"));
        else if (acte->paiementType() == "I") ui->PaiementlineEdit->setText(tr("Impayé"));
        else if (acte->paiementType() == "T" && acte->paiementTiers() == "CB") ui->PaiementlineEdit->setText(tr("carte de crédit"));
        else if (acte->paiementType() == "T" && acte->paiementTiers() != "CB") ui->PaiementlineEdit->setText(acte->paiementTiers());

        // on calcule le montant payé pour l'acte
        if (acte->paiementType() != "G" || acte->paiementType() != "I")
        {
            double montant = DataBase::getInstance()->getActeMontant(acte->id());
            ui->PayelineEdit->setText(QLocale().toString(montant,'f',2));
        }


        if (acte->paiementType() == "B"
              ||(acte->paiementType() == "T" && acte->paiementTiers() == "CB")
              || acte->paiementType() == "C"
              || acte->paiementType() == "E")
        {
            ui->PaiementLabel->setVisible(true);
            ui->PaiementLabel->setText("Paiement:");
            ui->PayeLabel->setVisible(true);
            ui->PayelineEdit->setVisible(true);
            ui->Comptaframe->setFixedHeight(74);
            ui->PaiementlineEdit->setGeometry(79,25,91,18);
        }
        else if (acte->paiementType() == "G"
                 || acte->paiementType() == "I")
        {
            ui->PaiementLabel->setVisible(false);
            ui->PayeLabel->setVisible(false);
            ui->PayelineEdit->setVisible(false);
            ui->Comptaframe->setFixedHeight(50);
            ui->PaiementlineEdit->setGeometry(8,25,164,18);
        }
        if (acte->paiementType() == "T" && acte->paiementTiers() != "CB")
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
    gidActe = acte->id();
}


/*------------------------------------------------------------------------------------------------------------------------------------
-- Retrouver l'acte à afficher -------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
int dlg_actesprecedents::getActeAffiche()
{
    return ui->idActelineEdit->text().toInt();
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation précédente ou suivante d'un patient ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_actesprecedents::NavigationConsult(int i)
{
    if( acte->nbActes() == 1 )
        return true;

    int idActe = -1;
    if (i == Suiv)
    {
        ++itCurrentActe;
        if( itCurrentActe == m_listeActes.constEnd() )
            itCurrentActe = m_listeActes.find(m_listeActes.lastKey());
    }
    else if (i == Prec)
    {
        --itCurrentActe;
        if( itCurrentActe == 0 )
            itCurrentActe = m_listeActes.constBegin();
    }
    else if (i == Debut)
    {
        itCurrentActe = m_listeActes.constBegin();
    }
    else if (i == Fin)
    {
        itCurrentActe = m_listeActes.find(m_listeActes.lastKey());
    }

    idActe = itCurrentActe.value()->id();
    if (idActe > -1)
    {
        ActesPrecsAfficheActe();
        return true;
    }

    return false;
}

int dlg_actesprecedents::getidPatient()
{
    return gidPatient;
}

