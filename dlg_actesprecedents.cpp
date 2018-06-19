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

dlg_actesprecedents::dlg_actesprecedents(int idPatient, int idActeAPasser, Procedures *procAPAsser, bool AvantDernier, QWidget *parent) :
    QDialog(parent),

ui(new Ui::dlg_actesprecedents)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    gidPatient      = idPatient;
    gidActe         = idActeAPasser; //jamais utilisé
    proc            = procAPAsser;
    gAvantDernier   = AvantDernier;
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);

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
    ActesPrecsAfficheActe(ChercheActeAAfficher());
}

dlg_actesprecedents::~dlg_actesprecedents()
{
    delete ui;
    proc = 0;
    delete proc;
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

void dlg_actesprecedents::Actualise()
{
    ActesPrecsAfficheActe(ChercheActeAAfficher());
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher les champs ---------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_actesprecedents::ActesPrecsAfficheActe(int idActeAAfficher)
{
    acte = DataBase::getInstance()->loadActeById(idActeAAfficher);

    if( !acte->isValid() )    // Aucune consultation trouvee pour ces criteres
        return;

    ui->idPatientlineEdit->setText(QString::number(gidPatient));

    //1.  Afficher les éléments de la tables Actes
    ui->ActeDatedateEdit->setDate(acte->date());
    ui->MotifupTextEdit->setText(acte->motif());
    ui->TexteupTextEdit->setText(acte->texte());
    ui->ConclusionupTextEdit->setText(acte->conclusion());
    ui->CourrierAFairecheckBox->setChecked(acte->courrierAFaire());
    ui->idActelineEdit->setText(QString::number(acte->id()));

    //2. retrouver le créateur de l'acte
    //idUser = ActesPrecsQuery.value(2).toInt();
    ui->CreateurConsultlineEdit->setText(tr("Créé par ") + proc->getLogin(acte->idCreatedBy() ) +
                                         tr(" pour ") + proc->getLogin(acte->idPatient()));

    //3. calcul de l'age
    if( !acte->agePatient().isNull() ) //TODO : TEST ICI
        ui->AgelineEdit->setText( Item::CalculAge(acte->agePatient(), ui->ActeDatedateEdit->date())["toString"].toString() );

    //4. Mettre à jour le numéro d'acte
    bool canprec = (acte->nbActes() > 1 && acte->noActe() > 1);
    ui->ActePrecedentpushButton->setEnabled(canprec);

    bool cansui = (acte->nbActes() > 1 && acte->noActe() < acte->nbActes());
    ui->ActeSuivantpushButton->setEnabled(cansui);

    bool canfirst = (acte->nbActes() > 1 && acte->noActe() > 1);
    ui->PremierActepushButton->setEnabled(canfirst);

    bool canlast = (acte->nbActes() > 1 && acte->noActe() < acte->nbActes());
    ui->DernierActepushButton->setEnabled(canlast);

    ui->NoActelabel->setText(QString::number(acte->noActe()) + " / " + QString::number(acte->nbActes()));

    //5. Afficher les renseignements comptables
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
            double montant = DataBase::getInstance()->getActeMontant(idActeAAfficher);
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
    gidActe = idActeAAfficher;
}


/*------------------------------------------------------------------------------------------------------------------------------------
-- Retrouver l'acte à afficher -------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
int dlg_actesprecedents::ChercheActeAAfficher()
{
    //TODO : SQL
    QString requete = "SELECT idActe FROM " NOM_TABLE_ACTES
              " WHERE idPat = '" + QString::number(gidPatient) + "' order by actedate asc";

    QSqlQuery ActeAAfficherQuery (requete, DataBase::getInstance()->getDataBase());
    if (DataBase::getInstance()->traiteErreurRequete(ActeAAfficherQuery,requete,tr("Impossible de retrouver la dernière consultation")))     // on retrouve les actes du patient
        return 0;
    if (ActeAAfficherQuery.size() == 0) return 0;
    ActeAAfficherQuery.last();
    if (ActeAAfficherQuery.size() == 1)
        return ActeAAfficherQuery.value(0).toInt();
    else
    {
        if (gAvantDernier) ActeAAfficherQuery.previous();
        return ActeAAfficherQuery.value(0).toInt();
    }
}

int dlg_actesprecedents::getActeAffiche()
{
    return ui->idActelineEdit->text().toInt();
}

//TODO : On charge tout et on navigue dedans
/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation précédente ou suivante d'un patient ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_actesprecedents::NavigationConsult(int i)
{
    //TODO : SQL
    //  Afficher les éléments de la tables Actes
    if( acte->nbActes() == 1 )
    {
        ActesPrecsAfficheActe(gidActe);
        return true;
    }
    int idActe = -1;
    QString requete = "SELECT idActe FROM " NOM_TABLE_ACTES
            " WHERE idPat = '" + QString::number(gidPatient) + "'";
    QSqlQuery NavigationConsultQuery (requete, DataBase::getInstance()->getDataBase());
    if (DataBase::getInstance()->traiteErreurRequete(NavigationConsultQuery,requete,tr("Impossible de retrouver les consultations de ce patient!")))
        return false;
    if (NavigationConsultQuery.size() < 2)  // Pas plus d'une consultation trouvée
        return false;
    NavigationConsultQuery.first();
    for (int a = 0; a < NavigationConsultQuery.size(); a++)
    {
        if (NavigationConsultQuery.value(0).toInt() == gidActe) break;
        NavigationConsultQuery.next();
    }

    if (i == Suiv)
    {
        if (NavigationConsultQuery.next())
            idActe = NavigationConsultQuery.value(0).toInt();
    }
    else if (i == Prec)
    {
        if (NavigationConsultQuery.previous())
            idActe = NavigationConsultQuery.value(0).toInt();
    }
    else if (i == Debut)
    {
        if (NavigationConsultQuery.first())
            idActe = NavigationConsultQuery.value(0).toInt();
    }
    else if (i == Fin)
    {
        if (NavigationConsultQuery.last())
            idActe = NavigationConsultQuery.value(0).toInt();
    }
    if (idActe > -1)
    {
        ActesPrecsAfficheActe(idActe);
        return true;
    }
    else
        return false;
}

int dlg_actesprecedents::getidPatient()
{
    return gidPatient;
}

