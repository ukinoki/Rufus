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

#include "dlg_actesprecedents.h"
#include "ui_dlg_actesprecedents.h"

dlg_actesprecedents::dlg_actesprecedents(int idPatient, int idActeAPasser, Procedures *procAPAsser, bool AvantDernier, QWidget *parent) :
    QDialog(parent),

ui(new Ui::dlg_actesprecedents)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    gidPatient      = idPatient;
    gidActe         = idActeAPasser;
    proc            = procAPAsser;
    gAvantDernier   = AvantDernier;
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);

    ui->FermepushButton->installEventFilter(this);

    if (gAvantDernier)
        restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionActesPrec").toByteArray());
    else
        restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionAutreDossier").toByteArray());

    db = proc->getDataBase();

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
            Button->setIcon(proc->giconFermeAppuye);
        if(event->type() == QEvent::MouseButtonRelease)
            Button->setIcon(proc->giconFermeRelache);
        if(event->type() == QEvent::MouseMove)
        {
            QRect rect = QRect(Button->mapToGlobal(QPoint(0,0)),Button->size());
            QPoint pos = cursor().pos();
            if (rect.contains(pos))
                Button->setIcon(proc->giconFermeAppuye);
            else
                Button->setIcon(proc->giconFermeRelache);
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

    if (idActeAAfficher == 0)
    {
        nbActes = 0;
        noActe = 0;
        return;
    }
    QString requete = "SELECT idActe, idPat, idUser, ActeDate, ActeMotif, ActeTexte, ActeConclusion, ActeCourrierAFaire, ActeCotation, ActeMontant, ActeMonnaie, CreePar FROM " NOM_TABLE_ACTES
              " WHERE idActe = '" + QString::number(idActeAAfficher) + "'";

    QSqlQuery ActesPrecsQuery (requete,db);
    if (proc->TraiteErreurRequete(ActesPrecsQuery, requete))
    {
        nbActes = 0;
        noActe = 0;
        return;
    }
    if (ActesPrecsQuery.size() == 0)    // Aucune consultation trouvee pour ces criteres
    {
        nbActes = 0;
        noActe = 0;
        return;
    }
    else
    {
        ActesPrecsQuery.first();
        ui->idPatientlineEdit->setText(QString::number(gidPatient));

        //1.  Afficher les éléments de la tables Actes
        ui->ActeDatedateEdit->setDate(ActesPrecsQuery.value(3).toDate());
        ui->MotiftextEdit->setText(ActesPrecsQuery.value(4).toString());
        ui->TextetextEdit->setText(ActesPrecsQuery.value(5).toString());
        ui->ConclusiontextEdit->setText(ActesPrecsQuery.value(6).toString());
        ui->CourrierAFairecheckBox->setChecked(ActesPrecsQuery.value(7).toString() == "T");
        ui->idActelineEdit->setText(ActesPrecsQuery.value(0).toString());
        if (ActesPrecsQuery.value(7).toInt() == 1) ui->CourrierAFairecheckBox->setChecked(true);

        //2. retrouver le créateur de l'acte
        //idUser = ActesPrecsQuery.value(2).toInt();
        ui->CreateurConsultlineEdit->setText(tr("Créé par ") + proc->getLogin(ActesPrecsQuery.value(11).toInt()) +
                                             tr(" pour ") + proc->getLogin(ActesPrecsQuery.value(2).toInt()));


        //3. calcul de l'age
        requete = "SELECT PatDDN FROM " NOM_TABLE_PATIENTS " WHERE idPat = " + QString::number(gidPatient);
        QSqlQuery AgePatientQuery (requete,db);
        if (proc->TraiteErreurRequete(AgePatientQuery,requete,tr("Impossible de retrouver la date de naissance")))
            return;
        else
        {
            AgePatientQuery.first();
            ui->AgelineEdit->setText(proc->CalculAge(AgePatientQuery.value(0).toDate(),ui->ActeDatedateEdit->date())["Total"].toString());
        }

        //4. Mettre à jour le numéro d'acte
        requete = "SELECT idActe FROM " NOM_TABLE_ACTES
                  " WHERE idPat = '" + QString::number(gidPatient) + "' ORDER BY ActeDate";
        QSqlQuery ChercheNoActeQuery (requete,db);
        if (proc->TraiteErreurRequete(ChercheNoActeQuery,requete,tr("Impossible de retrouver les consultations de ce patient")))
            return;
        else
        {
            if (ChercheNoActeQuery.size() > 0)           // Il y a des consultations
            {
                nbActes = ChercheNoActeQuery.size();
                if (nbActes == 1)
                {
                    ui->ActePrecedentpushButton->setEnabled(false);
                    ui->ActeSuivantpushButton->setEnabled(false);
                    ui->PremierActepushButton->setEnabled(false);
                    ui->DernierActepushButton->setEnabled(false);
                    noActe = 1;
                }
                if (nbActes > 1)
                {
                    ChercheNoActeQuery.first();
                    for (int i = 0; i < nbActes; i++)
                    {
                        if (ChercheNoActeQuery.value(0).toInt() == idActeAAfficher)
                        {
                            noActe =  ChercheNoActeQuery.at() + 1;
                            break;
                        }
                        else
                        ChercheNoActeQuery.next();
                    }
                    if (noActe == 1)
                    {
                            ui->ActePrecedentpushButton->setEnabled(false);
                            ui->PremierActepushButton->setEnabled(false);
                            ui->ActeSuivantpushButton->setEnabled(true);
                            ui->DernierActepushButton->setEnabled(true);
                    }
                    else
                    {
                        if (noActe == nbActes)
                        {
                            ui->ActePrecedentpushButton->setEnabled(true);
                            ui->ActeSuivantpushButton->setEnabled(false);
                            ui->PremierActepushButton->setEnabled(true);
                            ui->DernierActepushButton->setEnabled(false);
                        }
                        else
                        {
                            ui->ActePrecedentpushButton->setEnabled(true);
                            ui->ActeSuivantpushButton->setEnabled(true);
                            ui->PremierActepushButton->setEnabled(true);
                            ui->DernierActepushButton->setEnabled(true);
                        }
                    }
                }
                ui->NoActelabel->setText(QString::number(noActe) + " / " + QString::number(nbActes));
            }
        }
        //5. Afficher les renseignements comptables
        ui->ActeCotationlineEdit->setText(ActesPrecsQuery.value(8).toString());
        QString b ;
        if (ActesPrecsQuery.value(10).toString() == "F")
        {
            ui->ActeMontantLabel->setFixedHeight(30);
            ui->ActeMontantLabel->setText("Montant (€)\n(payé en F)");
            b = QLocale().toString(ActesPrecsQuery.value(9).toDouble()/6.55957,'f',2);
            ui->ActeMontantlineEdit->setText(b) ;
        }
        else
        {
            ui->ActeMontantLabel->setFixedHeight(15);
            ui->ActeMontantLabel->setText("Montant (€)");
            b = QLocale().toString(ActesPrecsQuery.value(9).toDouble(),'f',2);
            ui->ActeMontantlineEdit->setText(b) ;
        }

        requete = "SELECT TypePaiement, Tiers From " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(idActeAAfficher);
        QSqlQuery AfficheTypePaiementQuery (requete,db);
        if (proc->TraiteErreurRequete(AfficheTypePaiementQuery,requete,tr("Impossible de retrouver les renseignements comptables")))
            return;
        if (AfficheTypePaiementQuery.size() == 0)
        {
            ui->Comptaframe->setVisible(false);
        }
        else
        {
            ui->Comptaframe->setVisible(true);
            ui->PaiementlineEdit->setVisible(true);

            //2. on recheche ensuite le type de paiement : espèces, chèque, tiers, cb, impayé, gratuit

            AfficheTypePaiementQuery.first();
            if (AfficheTypePaiementQuery.value(0).toString() == "T"  && AfficheTypePaiementQuery.value(1).toString() != "CB") ui->PaiementlineEdit->setText(AfficheTypePaiementQuery.value(1).toString());

            if (AfficheTypePaiementQuery.value(0).toString() == "C") ui->PaiementlineEdit->setText(tr("chèque"));
            if (AfficheTypePaiementQuery.value(0).toString() == "E") ui->PaiementlineEdit->setText(tr("espèces"));
            if (AfficheTypePaiementQuery.value(0).toString() == "B") ui->PaiementlineEdit->setText(tr("carte de crédit"));
            if (AfficheTypePaiementQuery.value(0).toString() == "G") ui->PaiementlineEdit->setText(tr("Acte gratuit"));
            if (AfficheTypePaiementQuery.value(0).toString() == "I") ui->PaiementlineEdit->setText(tr("Impayé"));
            if (AfficheTypePaiementQuery.value(0).toString() == "T"
                    && AfficheTypePaiementQuery.value(1).toString() == "CB") ui->PaiementlineEdit->setText(tr("carte de crédit"));
            if (AfficheTypePaiementQuery.value(0).toString() == "T"
                    && AfficheTypePaiementQuery.value(1).toString() != "CB") ui->PaiementlineEdit->setText(AfficheTypePaiementQuery.value(1).toString());

            // on calcule le montant payé pour l'acte
            if (AfficheTypePaiementQuery.value(0).toString() != "G" || AfficheTypePaiementQuery.value(0).toString() != "I")
            {
                double TotalPaye = 0;
                // on récupère les lignes de paiement
                requete = " SELECT idRecette, Paye FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(idActeAAfficher);
                QSqlQuery ListePaiementsQuery (requete,db);
                proc->TraiteErreurRequete(ListePaiementsQuery, requete, "");
                ListePaiementsQuery.first();
                for (int l = 0; l < ListePaiementsQuery.size(); l++)
                {
                    requete = "SELECT Monnaie FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + ListePaiementsQuery.value(0).toString();
                    QSqlQuery MonnaieQuery (requete,db);
                    proc->TraiteErreurRequete(MonnaieQuery,requete,"");
                    MonnaieQuery.first();
                    if (MonnaieQuery.value(0).toString() == "F")
                        TotalPaye = TotalPaye + (ListePaiementsQuery.value(1).toDouble() / 6.55957);
                    else
                        TotalPaye = TotalPaye + ListePaiementsQuery.value(1).toDouble();
                    ListePaiementsQuery.next();
                }
                ui->PayelineEdit->setText(QLocale().toString(TotalPaye,'f',2));
            }


            if (AfficheTypePaiementQuery.value(0).toString() == "B"
                    ||(AfficheTypePaiementQuery.value(0).toString() == "T"
                       && AfficheTypePaiementQuery.value(1).toString() == "CB")
                    || AfficheTypePaiementQuery.value(0).toString() == "C"
                    || AfficheTypePaiementQuery.value(0).toString() == "E")
            {
                ui->PaiementLabel->setVisible(true);
                ui->PaiementLabel->setText("Paiement:");
                ui->PayeLabel->setVisible(true);
                ui->PayelineEdit->setVisible(true);
                ui->Comptaframe->setFixedHeight(74);
                ui->PaiementlineEdit->setGeometry(79,25,91,18);
            }
            else if (AfficheTypePaiementQuery.value(0).toString() == "G"
                     || AfficheTypePaiementQuery.value(0).toString() == "I")
            {
                ui->PaiementLabel->setVisible(false);
                ui->PayeLabel->setVisible(false);
                ui->PayelineEdit->setVisible(false);
                ui->Comptaframe->setFixedHeight(50);
                ui->PaiementlineEdit->setGeometry(8,25,164,18);
            }
            if (AfficheTypePaiementQuery.value(0).toString() == "T"
                    && AfficheTypePaiementQuery.value(1).toString() != "CB")
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
}


/*------------------------------------------------------------------------------------------------------------------------------------
-- Retrouver l'acte à afficher -------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
int dlg_actesprecedents::ChercheActeAAfficher()
{
    QString requete = "SELECT idActe FROM " NOM_TABLE_ACTES
              " WHERE idPat = '" + QString::number(gidPatient) + "' order by actedate asc";

    QSqlQuery ActeAAfficherQuery (requete,db);
    if (proc->TraiteErreurRequete(ActeAAfficherQuery,requete,tr("Impossible de retrouver la dernière consultation")))     // on retrouve les actes du patient
        return 0;
    if (ActeAAfficherQuery.size() == 0) return 0;
    ActeAAfficherQuery.last();
    if (ActeAAfficherQuery.size() == 1)
        return ActeAAfficherQuery.value(0).toInt();
    else {
        if (gAvantDernier) ActeAAfficherQuery.previous();
        return ActeAAfficherQuery.value(0).toInt();
    }
 }

int dlg_actesprecedents::getActeAffiche()
{
    return ui->idActelineEdit->text().toInt();
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Afficher la consultation précédente ou suivante d'un patient ----------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_actesprecedents::NavigationConsult(int i)
{
    //  Afficher les éléments de la tables Actes
    if (nbActes == 1)
    {
        ActesPrecsAfficheActe(gidActe);
        return true;
    }
    int idActe = -1;
    QString requete = "SELECT idActe FROM " NOM_TABLE_ACTES
            " WHERE idPat = '" + QString::number(gidPatient) + "'";
    QSqlQuery NavigationConsultQuery (requete,db);
    if (proc->TraiteErreurRequete(NavigationConsultQuery,requete,tr("Impossible de retrouver les consultations de ce patient!")))
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

