/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_remisecheques.h"
#include "icons.h"
#include "ui_dlg_remisecheques.h"

dlg_remisecheques::dlg_remisecheques(Procedures *procAPasser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_remisecheques)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    gidCompteACrediter  = -1;
    gidUserACrediter    = -1;

    proc        = procAPasser;

    db = DataBase::getInstance()->getDataBase();
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionRemiseCheques").toByteArray());

    connect (ui->AnnulupPushButton,                         SIGNAL(clicked()),                              this,           SLOT (Slot_AnnulupPushButton()));
    connect (ui->ListeChequesupTableWidget,                 SIGNAL(cellEntered(int,int)),                   this,           SLOT (Slot_ToolTip(int, int)));
    connect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellEntered(int,int)),                   this,           SLOT (Slot_ToolTip(int, int)));
    connect (ui->ImprimeupPushButton,                       SIGNAL(clicked()),                              this,           SLOT (Slot_ImprimepushButton()));

    ui->ImprimeupPushButton->move(this->size().width()-123,this->size().height()-60);
    ui->AnnulupPushButton->move(this->size().width()-233,this->size().height()-60);
    ui->RemisesPrecsPushButton->move(7,this->size().height()-60);

    ui->ListeChequesupTableWidget->installEventFilter(this);


    ui->ListeChequesupTableWidget->setColumnCount(6);
    ui->ChequesEnAttenteupTableWidget->setColumnCount(6);
    ui->ListeChequesupTableWidget->setColumnWidth(0,30);     //Checkbox
    ui->ListeChequesupTableWidget->setColumnWidth(1,30);     //No Ligne
    ui->ListeChequesupTableWidget->setColumnWidth(2,190);    //Tireur
    ui->ListeChequesupTableWidget->setColumnWidth(3,80);     //Banque
    ui->ListeChequesupTableWidget->setColumnWidth(4,58);     //Montant
    ui->ListeChequesupTableWidget->setColumnWidth(5,50);     //idCheque
    ui->ListeChequesupTableWidget->setColumnHidden(5,true);

    ui->ChequesEnAttenteupTableWidget->setColumnWidth(0,30);    //Checkbox
    ui->ChequesEnAttenteupTableWidget->setColumnWidth(1,30);    //No Ligne
    ui->ChequesEnAttenteupTableWidget->setColumnWidth(2,190);   //Tireur
    ui->ChequesEnAttenteupTableWidget->setColumnWidth(3,80);    //Banque
    ui->ChequesEnAttenteupTableWidget->setColumnWidth(4,58);    //Montant
    ui->ChequesEnAttenteupTableWidget->setColumnWidth(5,50);    //idCheque
    ui->ChequesEnAttenteupTableWidget->setColumnHidden(5,true);

    ui->ListeChequesupTableWidget->setGridStyle(Qt::DashDotDotLine);
    ui->ChequesEnAttenteupTableWidget->setGridStyle(Qt::DashDotDotLine);

    QStringList         LabelTableCheques;
    LabelTableCheques << " ";
    LabelTableCheques << "N°";
    LabelTableCheques << "Tireur";
    LabelTableCheques << "Banque";
    LabelTableCheques << "Montant";
    LabelTableCheques << "";
    ui->ListeChequesupTableWidget->setHorizontalHeaderLabels(LabelTableCheques);
    ui->ChequesEnAttenteupTableWidget->horizontalHeader()->setVisible(false);

    UpLineEdit *ListRem = new UpLineEdit(this);
    ListRem->setStyleSheet(
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}");
    ListRem->setReadOnly(true);
    ui->RemisePrecsupComboBox->setLineEdit(ListRem);

    ReconstruitListeUsers();
    connect (ui->UserComboBox,          SIGNAL(currentIndexChanged(int)),   this,   SLOT (Slot_ChangeCompte()));
    InitOK = true;
}

dlg_remisecheques::~dlg_remisecheques()
{
}

void dlg_remisecheques::reject()
{
    if (ui->ListeChequesupTableWidget->currentColumn() == 4)
    {
        QLineEdit* Line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(ui->ListeChequesupTableWidget->currentRow(),4));
        QString b ;
        b.setNum(Line->text().toDouble(),'f',2);
        Line->setText(ValeurAvantChangement);
        return;
    }
    QDialog::reject();
}


/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
LES SLOTS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_remisecheques::Slot_AnnulupPushButton()
{
    if (gMode == NouvelleRemise) {
        ui->ListeChequesupTableWidget->setCurrentCell(0,0); // pour valider une éventuelle modification dans un champ montant
        reject();
    }
    else {
        QString req;
        //1, on recherche les chèques à déposer
        req =   "SELECT idRecette, TireurCheque, BanqueCheque, Montant FROM " NOM_TABLE_RECETTES " pai"
                " WHERE pai.idRecette in (SELECT lig.idRecette FROM " NOM_TABLE_LIGNESPAIEMENTS " lig WHERE lig.idActe in"
                " (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE UserComptable = " + QString::number(gidUserACrediter) + "))"
                " AND pai.IdRemise IS NULL"
                " AND EnAttente IS NULL"
                " AND ModePaiement = 'C'"
                " ORDER BY TireurCheque";
        QSqlQuery ChequeARemettreQuery (req,db);
        DataBase::getInstance()->traiteErreurRequete(ChequeARemettreQuery,req,"");

        //1, on recherche les chèques à déposer mais dont le tireur à indiqué qu'il souhaitait qu'on attende pour le remettre en banque
        req =   "SELECT idRecette, TireurCheque, BanqueCheque, Montant FROM " NOM_TABLE_RECETTES " pai"
                " WHERE pai.idRecette in (SELECT lig.idRecette FROM " NOM_TABLE_LIGNESPAIEMENTS " lig WHERE lig.idActe in"
                " (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE UserComptable = " + QString::number(gidUserACrediter) +"))"
                " AND pai.IdRemise IS NULL"
                " AND EnAttente IS NOT NULL"
                " AND ModePaiement = 'C' ORDER BY TireurCheque";
        QSqlQuery ChequesEnAttenteQuery (req,db);
        DataBase::getInstance()->traiteErreurRequete(ChequesEnAttenteQuery,req,"");

        if (ChequeARemettreQuery.size() == 0 && ChequesEnAttenteQuery.size() == 0)
            reject();
        else
            ConfigMode(NouvelleRemise);
    }
}

void dlg_remisecheques::Slot_CorrigeRemise()
{
    proc->EnChantier();
    return;
    if (ui->RemisePrecsupComboBox->currentIndex()<0) return;

    int idremise = glistidRem.at(ui->RemisePrecsupComboBox->currentIndex());

    // supprimer la remise dans la table Remises
    QString req = "delete from from " NOM_TABLE_REMISECHEQUES " where idRemise = " + QString::number(idremise);
    QSqlQuery (req,db);

    // supprimer l'idRemise dans les lignes de la table lignesrecettes
    req = "update " NOM_TABLE_RECETTES " set idremise = null where idremise = " + QString::number(idremise);
    QSqlQuery (req,db);

    // supprimer la remise dans la table lignescomptes
    req = "delete from from " NOM_TABLE_LIGNESCOMPTES " where Lignelibelle = 'Remise de chèques n°" + QString::number(idremise) + "'";
    QSqlQuery (req,db);

    // revenir au mode nouvelle remise
    ConfigMode(NouvelleRemise);
}

void dlg_remisecheques::Slot_ImprimepushButton()
{
    if (ui->ListeChequesupTableWidget->rowCount() == 0)
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Il n'y a rien à imprimer!"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("OK je corrige"));
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton) reject();
        return;
    }
    QString A;
    QString req;
    ui->ListeChequesupTableWidget->setCurrentCell(0,0); // pour valider une éventuelle modification dans un champ montant

    if (gMode == NouvelleRemise) {
        // On vérifie que toutes les banques sont identifiées
        for (int i = 0; i < ui->ListeChequesupTableWidget->rowCount(); i++)
        {
            A = ui->ListeChequesupTableWidget->item(i,3)->text();
            if (A == "")
            {
                ui->ListeChequesupTableWidget->setCurrentCell(i,3);
                UpMessageBox::Watch(this,tr("Vous avez oublié d'identifier la banque émettrice de ce chèque!"));
                return;
            }
        }
        // On vérifie que tous les tireurs sont identifiés
        for (int i = 0; i < ui->ListeChequesupTableWidget->rowCount(); i++)
        {
            A = ui->ListeChequesupTableWidget->item(i,2)->text();
            if (A == "")
            {
                ui->ListeChequesupTableWidget->setCurrentCell(i,2);
                UpMessageBox::Watch(this,tr("Vous avez oublié d'identifier le tireur de ce chèque!"));
                return;
            }
        }

        // Recalcul du total
        double Total = 0;
        for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
        {
            QLineEdit* Line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(k,4));
            Total = Total + QLocale().toDouble(Line->text());
        }

        // On verrouille la table RemisesCheques
        QSqlQuery ("SET AUTOCOMMIT = 0;", DataBase::getInstance()->getDataBase());
        QString lockrequete = "LOCK TABLES " NOM_TABLE_REMISECHEQUES " WRITE;";
        QSqlQuery lockquery (lockrequete, DataBase::getInstance()->getDataBase());
        if (DataBase::getInstance()->traiteErreurRequete(lockquery,lockrequete,"Impossible de verrouiller " NOM_TABLE_REMISECHEQUES))
            return;
        //On récupére l'idRemise
        QString RecupMaxrequete = "SELECT MAX(idRemCheq) FROM " NOM_TABLE_REMISECHEQUES;
        QSqlQuery MaxIdRemiseQuery (RecupMaxrequete,db);
        DataBase::getInstance()->traiteErreurRequete(MaxIdRemiseQuery,RecupMaxrequete,"");
        MaxIdRemiseQuery.first();
        int idRemise = MaxIdRemiseQuery.value(0).toInt() + 1;

        QString Remiserequete = " INSERT INTO " NOM_TABLE_REMISECHEQUES " (idRemCheq, Montant, RCDate, idCompte) VALUES (" + QString::number(idRemise) +
                                "," + QString::number(Total) + ", NOW(),'" + QString::number(gidCompteACrediter) +"')";
        QSqlQuery   MAJRemiseChequesQuery (Remiserequete,db);
        proc->commit();
        if (DataBase::getInstance()->traiteErreurRequete(MAJRemiseChequesQuery,Remiserequete, tr("Impossible de mettre à jour la table des remises de chèques")))
            return;


        // On imprime la remise
        if (!ImprimerRemise(idRemise))
        {
            QString Annulrequete = "delete from " NOM_TABLE_REMISECHEQUES " where idRemCheq = " + QString::number(idRemise);
            QSqlQuery annulRemiseQuery (Annulrequete,db);
            DataBase::getInstance()->traiteErreurRequete(annulRemiseQuery,Annulrequete,"");
            UpMessageBox::Watch(this,tr("Impression annulée"));
            return;      // CZ001
        }

        //  et on update tous les chèques déposés avec cet IdRemise dans la table chèques
        for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
        {
            int idChequeAMettreAJour = ui->ListeChequesupTableWidget->item(k,5)->text().toInt();
            req = "UPDATE " NOM_TABLE_RECETTES " SET idRemise = " + QString::number(idRemise) + ", EnAttente = null WHERE idRecette = " + QString::number(idChequeAMettreAJour);
            QSqlQuery UpdatePaiementsQuery (req,db);
            DataBase::getInstance()->traiteErreurRequete(UpdatePaiementsQuery,req,"");
        }

        // On enregitre dans la table GestionComptes cettte remise
        req =  "INSERT INTO " NOM_TABLE_LIGNESCOMPTES " (idLigne, idCompte,LigneDate,LigneLibelle,LigneMontant,LigneDebitCredit,LigneTypeOperation) VALUES (" +
                QString::number(proc->getMAXligneBanque()) + "," +
                QString::number(gidCompteACrediter) +
                ", NOW(),"
                "'" + tr("Remise de chèques n°") + QString::number(idRemise) +
                "','" + QString::number(Total) +
                "','1','" + tr("Remise de chèques") + "')";
        QSqlQuery InsertLigneComptesQuery (req,db);
         DataBase::getInstance()->traiteErreurRequete(InsertLigneComptesQuery,req,"");

        // On corrige les intitulés de banque et les tireurs dans ActeBanque de la table Actes
        for (int l = 0; l < ui->ListeChequesupTableWidget->rowCount(); l++)
        {
            QString AB = ui->ListeChequesupTableWidget->item(l,3)->text();
            req = "UPDATE " NOM_TABLE_RECETTES " Act SET BanqueCheque = '" + AB  + "', TireurCheque = '" + proc->CorrigeApostrophe(ui->ListeChequesupTableWidget->item(l,2)->text()) +
                    + "', DateEnregistrement = NOW() WHERE Act.idRecette = " + ui->ListeChequesupTableWidget->item(l,5)->text();
            QSqlQuery UpdateLignesPaiementsQuery (req,db);
            DataBase::getInstance()->traiteErreurRequete(UpdateLignesPaiementsQuery,req,"");
        }

        // On corrige les chèques mis en attente
        for (int l = 0; l < ui->ChequesEnAttenteupTableWidget->rowCount(); l++)
        {
            req = "UPDATE " NOM_TABLE_RECETTES " SET EnAttente = 1 WHERE idRecette = " + ui->ChequesEnAttenteupTableWidget->item(l,5)->text();
            QSqlQuery CorrigeChequesEnAttenteQuery (req,db);
            DataBase::getInstance()->traiteErreurRequete(CorrigeChequesEnAttenteQuery,req,"");
        }
        accept();
    }
    else if(gMode == RevoirRemisesPrecs)
        ImprimerRemise(glistidRem.at(ui->RemisePrecsupComboBox->currentIndex()));
}

void dlg_remisecheques::Slot_ItemChequeARemettreClicked(int A, int B)
{
    if (B == 0)
    {
        gBloqueCellChanged = false;

        QTableWidgetItem    *pItem0                 = new QTableWidgetItem;
        QTableWidgetItem    *pItem1                 = new QTableWidgetItem;
        QTableWidgetItem    *pItem2                 = new QTableWidgetItem;
        QTableWidgetItem    *pItem3                 = new QTableWidgetItem;
        QDoubleValidator    *val                    = new QDoubleValidator(this);
        QLineEdit           *LigneMontant2          = new QLineEdit();
        QLineEdit           *NoLigne                = new QLineEdit();
        int                 i                       = ui->ChequesEnAttenteupTableWidget->rowCount();
        QString             idRec                   = ui->ListeChequesupTableWidget->item(A,5)->text();
        QFontMetrics        fm(qApp->font());

        ui->ListeChequesupTableWidget->setCurrentCell(A,B); // nécessaire parce que si on est en mode édition dans la colonne montant, le programme plante
        ui->ChequesEnAttenteupTableWidget->insertRow(i);
        LigneMontant2->setAlignment(Qt::AlignRight);
        val->setDecimals(2);
        LigneMontant2->setValidator(val);
        NoLigne->setAlignment(Qt::AlignRight);
        NoLigne->setFocusPolicy(Qt::NoFocus);
        NoLigne->setStyleSheet("border-style: none;");
        LigneMontant2->setStyleSheet("border-style: none;");

        pItem0->setText("");
        pItem0->setCheckState(Qt::Unchecked);
        ui->ChequesEnAttenteupTableWidget->setItem(i,0,pItem0);
        ui->ChequesEnAttenteupTableWidget->item(i,0)->setToolTip(tr("Cochez la case pour\nmettre ce chèque dans la liste des encaissements"));
        NoLigne->setText(QString::number(i+1));
        ui->ChequesEnAttenteupTableWidget->setCellWidget(i,1,NoLigne);                          // NoLigne
        pItem1->setText(ui->ListeChequesupTableWidget->item(A,2)->text());
        ui->ChequesEnAttenteupTableWidget->setItem(i,2,pItem1);                                 // Tireur
        pItem2->setText(ui->ListeChequesupTableWidget->item(A,3)->text());
        ui->ChequesEnAttenteupTableWidget->setItem(i,3,pItem2);                                 // Banque
        QLineEdit* LigneMontant = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(A,4));
        LigneMontant2->setText(LigneMontant->text());
        ui->ChequesEnAttenteupTableWidget->setCellWidget(i,4,LigneMontant2);                     // Montant
        pItem3->setText(idRec);
        ui->ChequesEnAttenteupTableWidget->setItem(i,5,pItem3);                                 // idRecette

        ui->ChequesEnAttenteupTableWidget->setRowHeight(i,fm.height()*1.1);
        ui->ChequesEnAttenteupTableWidget->sortItems(2);

        ui->ListeChequesupTableWidget->removeRow(A);
        // recalcul des numeros de lignes
        for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
        {
            NoLigne = new QLineEdit();
            NoLigne->setText(QString::number(k+1));
            NoLigne->setFocusPolicy(Qt::NoFocus);
            NoLigne->setStyleSheet("border-style: none;");
            ui->ListeChequesupTableWidget->setCellWidget(k,1,NoLigne);
        }
        for (int k = 0; k < ui->ChequesEnAttenteupTableWidget->rowCount(); k++)
        {
            NoLigne = new QLineEdit();
            NoLigne->setText(QString::number(k+1));
            NoLigne->setFocusPolicy(Qt::NoFocus);
            NoLigne->setStyleSheet("border-style: none;");
            ui->ChequesEnAttenteupTableWidget->setCellWidget(k,1,NoLigne);
        }
        Slot_RecalculeMontant();
        gBloqueCellChanged = true;

        //mise à jour de la table lignesrecettes
        QString UpdateidRec = "update " NOM_TABLE_RECETTES " set EnAttente = 1 where idRecette = " + idRec;
        QSqlQuery UpdateRecQuery (UpdateidRec,db);
        DataBase::getInstance()->traiteErreurRequete(UpdateRecQuery, UpdateidRec,"void dlg_remisecheques::Slot_ItemChequeARemettreClicked(int A, int B)");
    }
}

void dlg_remisecheques::Slot_ItemChequeEnAttenteClicked(int A, int B)
{
    if (B == 0)
    {
        gBloqueCellChanged = false;

        QTableWidgetItem    *pItem0                 = new QTableWidgetItem;
        QTableWidgetItem    *pItem1                 = new QTableWidgetItem;
        QTableWidgetItem    *pItem2                 = new QTableWidgetItem;
        QTableWidgetItem    *pItem3                 = new QTableWidgetItem;
        QLineEdit           *LigneMontant2          = new QLineEdit();
        QLineEdit           *NoLigne                = new QLineEdit;
        int                 i                       = ui->ListeChequesupTableWidget->rowCount();
        QString             idRec                   = ui->ChequesEnAttenteupTableWidget->item(A,5)->text();
        QDoubleValidator    *val                    = new QDoubleValidator(this);

        ui->ChequesEnAttenteupTableWidget->setCurrentCell(A,B); // nécessaire parce que si on est en mode édition dans la colonne montant, le programme plante
        ui->ListeChequesupTableWidget->insertRow(i);
        LigneMontant2->setAlignment(Qt::AlignRight);
        val->setDecimals(2);
        LigneMontant2->setValidator(val);
        NoLigne->setAlignment(Qt::AlignRight);
        NoLigne->setFocusPolicy(Qt::NoFocus);
        NoLigne->setStyleSheet("border-style: none;");
        LigneMontant2->setStyleSheet("border-style: none;");

        pItem0->setText("");
        pItem0->setCheckState(Qt::Checked);
        ui->ListeChequesupTableWidget->setItem(i,0,pItem0);
        ui->ListeChequesupTableWidget->item(i,0)->setToolTip(tr("Décochez la case\npour mettre ce chèqe en attente"));
        pItem1->setText(ui->ChequesEnAttenteupTableWidget->item(A,2)->text());
        ui->ListeChequesupTableWidget->setItem(i,2,pItem1);                                 // Tireur
        pItem2->setText(ui->ChequesEnAttenteupTableWidget->item(A,3)->text());
        ui->ListeChequesupTableWidget->setItem(i,3,pItem2);                                 // Banque
        QLineEdit* LigneMontant = static_cast<QLineEdit*>(ui->ChequesEnAttenteupTableWidget->cellWidget(A,4));
        LigneMontant2->setText(LigneMontant->text());
        ui->ListeChequesupTableWidget->setCellWidget(i,4,LigneMontant2);                     // Montant
        pItem3->setText(idRec);
        ui->ListeChequesupTableWidget->setItem(i,5,pItem3);                                 // idPRecette

        QFontMetrics fm(qApp->font());
        ui->ListeChequesupTableWidget->setRowHeight(i,fm.height()*1.1);
        ui->ListeChequesupTableWidget->sortItems(2);

        ui->ChequesEnAttenteupTableWidget->removeRow(A);

//        // recalcul des numeros de lignes
        for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
        {
            NoLigne = new QLineEdit();
            NoLigne->setText(QString::number(k+1));
            NoLigne->setFocusPolicy(Qt::NoFocus);
            NoLigne->setStyleSheet("border-style: none;");
            ui->ListeChequesupTableWidget->setCellWidget(k,1,NoLigne);
        }
        for (int k = 0; k < ui->ChequesEnAttenteupTableWidget->rowCount(); k++)
        {
            NoLigne = new QLineEdit();
            NoLigne->setText(QString::number(k+1));
            NoLigne->setFocusPolicy(Qt::NoFocus);
            NoLigne->setStyleSheet("border-style: none;");
            ui->ChequesEnAttenteupTableWidget->setCellWidget(k,1,NoLigne);
        }
        Slot_RecalculeMontant();


        //mise à jour de la table lignesrecettes
        QString UpdateidRec = "update " NOM_TABLE_RECETTES " set EnAttente = null where idRecette = " + idRec;
        QSqlQuery UpdateRecQuery (UpdateidRec,db);
        DataBase::getInstance()->traiteErreurRequete(UpdateRecQuery, UpdateidRec,"void dlg_remisecheques::Slot_ItemChequeARemettreClicked(int A, int B)");
    }
}

void dlg_remisecheques::Slot_MiseEnFormeMontant(int A, int B, int C, int D)
{
    if (B == 4)   // on arrive dans la case montant et on met en mémoire le montant qui y figure
    {
        QLineEdit* Line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(A,B));
        ValeurAvantChangement.setNum(Line->text().toDouble(),'f',2);
    }
    if (D == 4)   // on quitte une case montant et on valide le montant qui y figure sinon, on remet l'ancien
    {
        QLineEdit* Line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(C,4));
        QString b ;
        b = QLocale().toString(QLocale().toDouble(Line->text()),'f',2);
        if (b == "" || QLocale().toDouble(b) <= 0)
            Line->setText(ValeurAvantChangement);
        else
        {
            Line->setText(b);
            Slot_RecalculeMontant();
        }
    }
}

void dlg_remisecheques::Slot_ChangeCompte()
{
    int anciduser   = gidUserACrediter;
    int ancidcompte = gidCompteACrediter;
    gidUserACrediter    = ui->UserComboBox->currentData().toMap().value("iduser").toInt();
    gidCompteACrediter  = ui->UserComboBox->currentData().toMap().value("idcompte").toInt();
    QString req = "SELECT IntituleCompte, IBAN FROM " NOM_TABLE_COMPTES
                  " WHERE idCompte = " + QString::number(gidCompteACrediter);
    QSqlQuery   IntituleCompteQuery (req,db);
    DataBase::getInstance()->traiteErreurRequete(IntituleCompteQuery,req, "impossible de se connecter à la table des comptes");
    IntituleCompteQuery.first();
    ui->IntituleComptetextEdit->setText(IntituleCompteQuery.value(0).toString() + "\n" + IntituleCompteQuery.value(1).toString());
    bool chgOK = true;
    if (gidUserACrediter != anciduser)   // si on a changé d'utilisateur, il faut recalculer les tables
    {
        if (!ConfigMode(NouvelleRemise))
            if (!ConfigMode(RevoirRemisesPrecs)){
                UpMessageBox::Watch(this,tr("Pas d'archive de remises de chèques non plus"));
                chgOK = false;
            }
    }
    if (!chgOK)
    {
        QMap<QString, QVariant> map;
        map["iduser"]   = anciduser;
        map["idcompte"] = ancidcompte;
        int a = ui->UserComboBox->findData(map);
        ui->UserComboBox->setCurrentIndex(a);
    }
}

void dlg_remisecheques::Slot_RemplirRemisesPrecs(int id)
{
    QFontMetrics        fm(qApp->font());
    QString A;
    disconnect (ui->ListeChequesupTableWidget,     SIGNAL(itemEntered(QTableWidgetItem*)), 0, 0);
    QToolTip::showText(cursor().pos(),"");
    ui->ListeChequesupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ListeChequesupTableWidget->clearContents();
    QString req = "select idRecette, TireurCheque, BanqueCheque, Montant"
                  " from " NOM_TABLE_RECETTES
                  " where idRemise = " + QString::number(glistidRem.at(id)) +
                  " and idUser = " + QString::number(gidUserACrediter) +
                  " order by Tireurcheque";
    QSqlQuery quer(req,db);
    ui->ListeChequesupTableWidget->setRowCount(quer.size());
    quer.first();
    if (quer.size()>0) {
        QTableWidgetItem *pItem0    = new QTableWidgetItem() ;
        QTableWidgetItem *pItem1    = new QTableWidgetItem() ;
        QTableWidgetItem *pItem2    = new QTableWidgetItem() ;
        QTableWidgetItem *pItem3    = new QTableWidgetItem() ;
        QTableWidgetItem *pItem4    = new QTableWidgetItem() ;
        UpCheckBox *check           = new UpCheckBox() ;
        for (int i = 0; i < quer.size(); i++)
        {
            pItem0 = new QTableWidgetItem() ;
            pItem1 = new QTableWidgetItem() ;
            pItem2 = new QTableWidgetItem() ;
            pItem3 = new QTableWidgetItem() ;
            pItem4 = new QTableWidgetItem() ;
            check  = new UpCheckBox() ;

            check->setText("");
            check->setCheckState(Qt::Checked);
            check->setToggleable(false);
            check->setEnabled(false);
            check->setFocusPolicy(Qt::NoFocus);
            ui->ListeChequesupTableWidget->setCellWidget(i,0,check);
            pItem0->setText(QString::number(i+1));
            ui->ListeChequesupTableWidget->setItem(i,1,pItem0);
            A = quer.value(1).toString();                           // Tireur
            pItem1->setText(A);
            ui->ListeChequesupTableWidget->setItem(i,2,pItem1);
            A = quer.value(2).toString();                           // Banque
            pItem2->setText(A);
            ui->ListeChequesupTableWidget->setItem(i,3,pItem2);
            A = QLocale().toString(quer.value(3).toDouble(),'f',2);   // Montant
            pItem3->setText(A);
            pItem3->setTextAlignment(Qt::AlignRight);
            ui->ListeChequesupTableWidget->setItem(i,4,pItem3);
            A = quer.value(0).toString();                           // idRecette
            pItem4->setText(A);
            ui->ListeChequesupTableWidget->setItem(i,5,pItem4);
            ui->ListeChequesupTableWidget->setRowHeight(i,fm.height()*1.1);
            quer.next();
        }
        ui->ListeChequesupTableWidget->setFocusPolicy(Qt::NoFocus);
        double Total = 0;
        for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
            Total += QLocale().toDouble(ui->ListeChequesupTableWidget->item(k,4)->text());
        ui->TotallineEdit->setText(QString::number(ui->ListeChequesupTableWidget->rowCount()) + tr(" chèques -> ") + QLocale().toString(Total,'f',2) + tr(" euros"));
    }
    req = "select idligne from " NOM_TABLE_LIGNESCOMPTES " where lignelibelle = '" + tr("Remise de chèques n°") + QString::number(glistidRem.at(id)) +"'";
    QSqlQuery cquer(req,db);
    ui->RemisesPrecsPushButton->setEnabled(cquer.size()>0);
}

void dlg_remisecheques::Slot_RecalculeMontant()
{
    double Total = 0;
    for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
    {
        QLineEdit* Line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(k,4));
        Total = Total + QLocale().toDouble(Line->text());
    }
    ui->TotallineEdit->setText(QString::number(ui->ListeChequesupTableWidget->rowCount()) + tr(" chèques -> ") + QString::number(Total) + tr(" euros"));
}

void dlg_remisecheques::Slot_ToolTip(int A, int B)
{
    int col = 5;
    UpTableWidget *tabl = dynamic_cast<UpTableWidget *>(sender());
    if (tabl)   {
        if (B == 2 || B == 3)
        {
            QString requete = "SELECT PatNom, PatPrenom, ActeCotation, ActeDate From " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_ACTES " act WHERE act.idActe in (SELECT idActe FROM "
                    NOM_TABLE_LIGNESPAIEMENTS " WHERE idRecette = " + tabl->item(A,col)->text() + ") AND pat.idPat = act.idPat";
            QSqlQuery ResultQuery(requete,db);
            DataBase::getInstance()->traiteErreurRequete(ResultQuery,requete,"");
            ResultQuery.first();
            QString ABC;
            for (int i = 0; i < ResultQuery.size();i++)
            {
                ABC += ResultQuery.value(0).toString() + " " + ResultQuery.value(1).toString() + " " + ResultQuery.value(2).toString() + " du " + ResultQuery.value(3).toDate().toString(tr("dd-MM-yyyy"));
                if (i< ResultQuery.size()-1)
                    ABC += "\n";
                ResultQuery.next();
            }
            QToolTip::showText(cursor().pos(),ABC);
        }
    }
}

void dlg_remisecheques::Slot_TrierChequesEnAttente(int, int B)
{
    if (B == 2 && gBloqueCellChanged) ui->ChequesEnAttenteupTableWidget->sortItems(2);
}

void dlg_remisecheques::Slot_TrierListeCheques(int, int B)
{
    if (B == 2 && gBloqueCellChanged) ui->ListeChequesupTableWidget->sortItems(2);
}

void dlg_remisecheques::Slot_VoirRemisesPrecs()
{
    ConfigMode(RevoirRemisesPrecs);
}

//-------------------------------------------------------------------------------------
// Gestion des touches ...
//-------------------------------------------------------------------------------------
void dlg_remisecheques::keyPressEvent ( QKeyEvent * event )
{
    switch (event->key())
    {
    case Qt::Key_Escape:
    {
        QKeyEvent *newevent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Return , Qt::NoModifier);
        QCoreApplication::postEvent (focusWidget(), newevent);
        QWidget::eventFilter(focusWidget(), newevent);
        break;
    }
    default:
        break;
    }
}

void dlg_remisecheques::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionRemiseCheques", saveGeometry());
    event->accept();
}

//-------------------------------------------------------------------------------------
// Gestion des évènements
//-------------------------------------------------------------------------------------
bool dlg_remisecheques::eventFilter(QObject *obj, QEvent *event) // A REVOIR
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && keyEvent->modifiers() == Qt::NoModifier)
        {
            if (obj == ui->ListeChequesupTableWidget)
            {
                if (ui->ListeChequesupTableWidget->currentColumn() == 3)
                {
                    int AD = ui->ListeChequesupTableWidget->currentRow();
                    if (AD <  ui->ListeChequesupTableWidget->rowCount()-1)
                        ui->ListeChequesupTableWidget->setCurrentCell(AD + 1,3);
                    else
                        ui->ListeChequesupTableWidget->setCurrentCell(0,3);
                    return false;
                }
                else
                {
                    QKeyEvent *newevent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab , Qt::NoModifier);
                    QCoreApplication::postEvent (obj, newevent);
                    return QWidget::eventFilter(obj, newevent);
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

bool dlg_remisecheques::getInitOK()
{
    return InitOK;
}

void dlg_remisecheques::setInitOK(bool init)
{
    InitOK = init;
}


/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
LES FONCTIONS ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_remisecheques::ConfigMode(int mode)
{
    gMode = mode;

    if (gMode == NouvelleRemise)
    {
        ui->UserComboBox                    ->setEnabled(true);
        ui->ChequesEnAttentelabel           ->setVisible(true);
        ui->ChequesEnAttenteupTableWidget   ->setVisible(true);
        ui->ListeChequesupTableWidget       ->setFixedSize(ui->ListeChequesupTableWidget->width(),360);
        ui->TotallineEdit                   ->move(189,520);
        ui->AnnulupPushButton               ->setUpButtonStyle(UpPushButton::ANNULBUTTON, UpPushButton::Small);
        ui->ImprimeupPushButton             ->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Small);
        ui->ImprimeupPushButton             ->setIcon(Icons::icImprimer());
        ui->ImprimeupPushButton             ->setText("Imprimer");
        ui->ImprimeupPushButton             ->setShortcut(QKeySequence("Meta+Return"));
        ui->RemisePrecsupComboBox           ->setVisible(false);
        ui->RemisesPrecsPushButton          ->setText("Remises\nprécédentes");
        ui->RemisesPrecsPushButton          ->setIcon(Icons::icArchive());
        ui->RemisesPrecsPushButton          ->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Mid);
        ui->RemisesPrecsPushButton          ->setEnabled(true);

        gBloqueCellChanged = true;

        QString req;
        //1, on recherche les chèques à déposer
        req =   "SELECT idRecette, TireurCheque, BanqueCheque, Montant FROM " NOM_TABLE_RECETTES " pai"
                " WHERE pai.idRecette in (SELECT lig.idRecette FROM " NOM_TABLE_LIGNESPAIEMENTS " lig WHERE lig.idActe in"
                " (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE UserComptable = " + QString::number(gidUserACrediter) + "))"
                " AND pai.IdRemise IS NULL"
                " AND EnAttente IS NULL"
                " AND ModePaiement = 'C'"
                " ORDER BY TireurCheque";
        //qDebug() << req;
        QSqlQuery ChequeARemettreQuery (req,db);
        DataBase::getInstance()->traiteErreurRequete(ChequeARemettreQuery,req,"");

        //1, on recherche les chèques à déposer mais dont le tireur à indiqué qu'il souhaitait qu'on attende pour le remettre en banque
        req =   "SELECT idRecette, TireurCheque, BanqueCheque, Montant FROM " NOM_TABLE_RECETTES " pai"
                " WHERE pai.idRecette in (SELECT lig.idRecette FROM " NOM_TABLE_LIGNESPAIEMENTS " lig WHERE lig.idActe in"
                " (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE UserComptable = " + QString::number(gidUserACrediter) +"))"
                " AND pai.IdRemise IS NULL"
                " AND EnAttente IS NOT NULL"
                " AND ModePaiement = 'C' ORDER BY TireurCheque";
        //qDebug() << req;
        QSqlQuery ChequesEnAttenteQuery (req,db);
        DataBase::getInstance()->traiteErreurRequete(ChequesEnAttenteQuery,req,"");


        if (ChequeARemettreQuery.size() == 0 && ChequesEnAttenteQuery.size() == 0)
        {
            UpMessageBox::Watch(this,tr("Aucune remise de chèques à effectuer!"));
            return false;
        }

        disconnect (ui->ListeChequesupTableWidget,                 SIGNAL(cellClicked(int, int)),                  0, 0);
        disconnect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellClicked(int, int)),                  0, 0);
        disconnect (ui->ListeChequesupTableWidget,                 SIGNAL(currentCellChanged(int,int,int,int)),    0, 0);
        disconnect (ui->ListeChequesupTableWidget,                 SIGNAL(cellChanged(int,int)),                   0, 0);
        disconnect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellChanged(int,int)),                   0, 0);
        disconnect (ui->RemisePrecsupComboBox,                     SIGNAL(currentIndexChanged(int)),               0, 0);
        disconnect (ui->RemisesPrecsPushButton,                    SIGNAL(clicked()),                              0, 0);

        connect (ui->ListeChequesupTableWidget,                 SIGNAL(cellClicked(int, int)),                  this,           SLOT (Slot_ItemChequeARemettreClicked(int, int)));
        connect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellClicked(int, int)),                  this,           SLOT (Slot_ItemChequeEnAttenteClicked(int, int)));
        connect (ui->ListeChequesupTableWidget,                 SIGNAL(currentCellChanged(int,int,int,int)),    this,           SLOT (Slot_MiseEnFormeMontant(int,int,int,int)));
        connect (ui->ListeChequesupTableWidget,                 SIGNAL(cellChanged(int,int)),                   this,           SLOT (Slot_TrierListeCheques(int,int)));
        connect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellChanged(int,int)),                   this,           SLOT (Slot_TrierChequesEnAttente(int,int)));
        connect (ui->RemisesPrecsPushButton,                    SIGNAL(clicked()),                              this,           SLOT (Slot_VoirRemisesPrecs()));

        QTableWidgetItem    *pItem1, *pItem2, *pItem3, *pItem4;
        QString             A;
        QFontMetrics        fm(qApp->font());
        double              Total = 0;

        ui->ListeChequesupTableWidget->clearContents();
        ui->ChequesEnAttenteupTableWidget->clearContents();

        ui->ListeChequesupTableWidget->setRowCount(ChequeARemettreQuery.size());
        ui->ChequesEnAttenteupTableWidget->setRowCount(ChequesEnAttenteQuery.size());

        ChequeARemettreQuery.first();
        for (int i = 0; i < ChequeARemettreQuery.size(); i++)
        {
            pItem1 = new QTableWidgetItem() ;
            pItem2 = new QTableWidgetItem() ;
            pItem3 = new QTableWidgetItem() ;
            pItem4 = new QTableWidgetItem() ;

            QLineEdit           *LigneMontant   = new QLineEdit;
            QLineEdit           *NoLigne        = new QLineEdit();
            QDoubleValidator    *val            = new QDoubleValidator(this);
            val->setDecimals(2);
            LigneMontant->setValidator(val);

            LigneMontant->setAlignment(Qt::AlignRight);
            NoLigne->setAlignment(Qt::AlignRight);
            NoLigne->setFocusPolicy(Qt::NoFocus);
            NoLigne->setStyleSheet("border-style: none;");
            LigneMontant->setStyleSheet("border-style: none;");

            pItem4->setText("");
            pItem4->setCheckState(Qt::Checked);
            ui->ListeChequesupTableWidget->setItem(i,0,pItem4);
            ui->ListeChequesupTableWidget->item(i,0)->setToolTip(tr("Décochez la case\npour mettre ce chèqe en attente"));
            NoLigne->setText(QString::number(i+1));
            ui->ListeChequesupTableWidget->setCellWidget(i,1,NoLigne);
            A = ChequeARemettreQuery.value(1).toString();                           // Tireur
            pItem1->setText(A);
            ui->ListeChequesupTableWidget->setItem(i,2,pItem1);
            A = ChequeARemettreQuery.value(2).toString();                           // Banque
            pItem2->setText(A);
            ui->ListeChequesupTableWidget->setItem(i,3,pItem2);
            A = QLocale().toString(ChequeARemettreQuery.value(3).toDouble(),'f',2);   // Montant
            LigneMontant->setText(A);
            ui->ListeChequesupTableWidget->setCellWidget(i,4,LigneMontant);
            A = ChequeARemettreQuery.value(0).toString();                           // idRecette
            pItem3->setText(A);
            ui->ListeChequesupTableWidget->setItem(i,5,pItem3);
            ui->ListeChequesupTableWidget->setRowHeight(i,fm.height()*1.1);

            ChequeARemettreQuery.next();
        }

        ChequesEnAttenteQuery.first();
        for (int i = 0; i < ChequesEnAttenteQuery.size(); i++)
        {
            pItem1 = new QTableWidgetItem() ;
            pItem2 = new QTableWidgetItem() ;
            pItem3 = new QTableWidgetItem() ;
            pItem4 = new QTableWidgetItem() ;

            QLineEdit           *LigneMontant   = new QLineEdit;
            QLineEdit           *NoLigne        = new QLineEdit();
            QDoubleValidator    *val            = new QDoubleValidator(this);
            val->setDecimals(2);
            LigneMontant->setValidator(val);

            LigneMontant->setAlignment(Qt::AlignRight);
            LigneMontant->setFocusPolicy(Qt::NoFocus);
            NoLigne->setAlignment(Qt::AlignRight);
            NoLigne->setFocusPolicy(Qt::NoFocus);
            NoLigne->setStyleSheet("border-style: none;");
            LigneMontant->setStyleSheet("border-style: none;");

            pItem4->setText("");
            pItem4->setCheckState(Qt::Unchecked);
            ui->ChequesEnAttenteupTableWidget->setItem(i,0,pItem4);
            ui->ChequesEnAttenteupTableWidget->item(i,0)->setToolTip(tr("Cochez la case pour\nmettre ce chèque dans la liste des encaissements"));
            NoLigne->setText(QString::number(i+1));
            ui->ChequesEnAttenteupTableWidget->setCellWidget(i,1,NoLigne);
            A = ChequesEnAttenteQuery.value(1).toString();              // Tireur
            pItem1->setText(A);
            ui->ChequesEnAttenteupTableWidget->setItem(i,2,pItem1);
            A = ChequesEnAttenteQuery.value(2).toString();              // Banque
            pItem2->setText(A);
            ui->ChequesEnAttenteupTableWidget->setItem(i,3,pItem2);
            A = QLocale().toString(ChequesEnAttenteQuery.value(3).toDouble(),'f',2);  // Montant
            LigneMontant->setText(A);
            ui->ChequesEnAttenteupTableWidget->setCellWidget(i,4,LigneMontant);
            A = ChequesEnAttenteQuery.value(0).toString();              // idCheque
            pItem3->setText(A);
            ui->ChequesEnAttenteupTableWidget->setItem(i,5,pItem3);
            ui->ChequesEnAttenteupTableWidget->setRowHeight(i,fm.height()*1.1);
            ChequesEnAttenteQuery.next();
        }
        for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
        {
            QLineEdit* Line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(k,4));
            Total = Total + QLocale().toDouble(Line->text());
        }
        ui->TotallineEdit->setText(QString::number(ui->ListeChequesupTableWidget->rowCount()) + tr(" chèques -> ") + QLocale().toString(Total,'f',2) + tr(" euros"));
        ui->TotallineEdit->setAlignment(Qt::AlignRight);
    }
    else if (gMode == RevoirRemisesPrecs)
    {
        ui->UserComboBox                    ->setEnabled(true);
        ui->ChequesEnAttentelabel           ->setVisible(false);
        ui->ChequesEnAttenteupTableWidget   ->setVisible(false);
        ui->ListeChequesupTableWidget       ->setFixedSize(ui->ListeChequesupTableWidget->width(),580);
        ui->ImprimeupPushButton             ->setIcon(Icons::icLoupe());
        ui->ImprimeupPushButton             ->setText("Revoir");
        ui->TotallineEdit                   ->move(189,740);
        ui->RemisePrecsupComboBox           ->setVisible(true);
        ui->RemisesPrecsPushButton          ->setText("Corriger");
        ui->RemisesPrecsPushButton          ->setIcon(Icons::icEditer());

        disconnect (ui->ListeChequesupTableWidget,                 SIGNAL(cellClicked(int, int)),                  0, 0);
        disconnect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellClicked(int, int)),                  0, 0);
        disconnect (ui->ListeChequesupTableWidget,                 SIGNAL(currentCellChanged(int,int,int,int)),    0, 0);
        disconnect (ui->ListeChequesupTableWidget,                 SIGNAL(cellChanged(int,int)),                   0, 0);
        disconnect (ui->ChequesEnAttenteupTableWidget,             SIGNAL(cellChanged(int,int)),                   0, 0);
        disconnect (ui->RemisePrecsupComboBox,                     SIGNAL(currentIndexChanged(int)),               0, 0);
        disconnect (ui->RemisesPrecsPushButton,                    SIGNAL(clicked()),                              0, 0);

        QString req = "select idRemCheq, RCDate, Montant, idCompte from " NOM_TABLE_REMISECHEQUES " where idcompte in (select idcompte from " NOM_TABLE_COMPTES
                      " where idUser = " + QString::number(gidUserACrediter) + ") order by idremcheq desc";
        QSqlQuery quer(req,db);
        if (DataBase::getInstance()->traiteErreurRequete(quer,req,"")) return false;
        if (quer.size() == 0) return false;
        quer.first();
        QStringList listrem;
        glistidRem.clear();
        for (int i=0; i<quer.size(); i++){
            listrem << "Remise n° " + quer.value(0).toString() + " du " + quer.value(1).toDate().toString(tr("d MMMM yyyy")) + "\t" + QLocale().toString(quer.value(2).toDouble(),'f',2);
            glistidRem << quer.value(0).toInt();
            quer.next();
        }
        ui->RemisePrecsupComboBox->addItems(listrem);
        ui->RemisePrecsupComboBox->setMaxVisibleItems(10);
        connect(ui->RemisePrecsupComboBox,      SIGNAL(currentIndexChanged(int)),   this,       SLOT (Slot_RemplirRemisesPrecs(int)));
        connect(ui->RemisesPrecsPushButton,     SIGNAL(clicked(bool)),              this,       SLOT (Slot_CorrigeRemise()));
        ui->RemisePrecsupComboBox->setFocus();
        Slot_RemplirRemisesPrecs(0);
    }
    return true;
}

/*---------------------------------------------------------------------------------
    Impression remise de chèques
-----------------------------------------------------------------------------------*/
bool dlg_remisecheques::ImprimerRemise(int idRemise)
{
    bool AvecDupli   = false;
    bool AvecPrevisu = proc->ApercuAvantImpression();
    bool AvecNumPage = true;

    QString LigneChq;

    //--------------------------------------------------------------------
    // Préparation de l'état "bordereau de remise" dans un QplainTextEdit
    //--------------------------------------------------------------------
    double  gtotalMontRemise    = 0;
    int     gtotalNbrePieces    = 0;
    QDate   date;
    int idcompte;
    int iduser(-1);
    QString req;

    if (gMode == RevoirRemisesPrecs) {
        req =  "select Rcdate, idcompte from " NOM_TABLE_REMISECHEQUES " where idremcheq = " + QString::number(idRemise);
        QSqlQuery quer(req,db);
        if (quer.size() == 0) return false;
        quer.first();
        idcompte = quer.value(1).toInt();
        date = quer.value(0).toDate();
        iduser = ui->UserComboBox->currentData().toMap().value("user").toInt();

        AvecPrevisu = true;
    }
    else if (gMode == NouvelleRemise) {
        idcompte    = gidCompteACrediter;
        iduser      = gidUserACrediter;
        date        = QDate::currentDate();
    }

    //création de l'entête
    QString EnTete;
    if (iduser == -1) return false;
    User *userEntere = proc->getUserById(iduser);
    if(userEntere == nullptr)
        return false;
    EnTete = proc->ImpressionEntete(date, userEntere).value("Norm");
    if (EnTete == "") return false;

    req = "select cmpt.idbanque, IBAN, intitulecompte, NomBanque "
          " from " NOM_TABLE_COMPTES " as cmpt "
          " left outer join " NOM_TABLE_BANQUES " as bank on cmpt.idbanque = bank.idbanque "
          " where idcompte = " + QString::number(idcompte) ;
    QSqlQuery  RetrouveBanqueQuery (req,db);
    DataBase::getInstance()->traiteErreurRequete( RetrouveBanqueQuery,req,"");
    if ( RetrouveBanqueQuery.size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver cette banque!"));
        return false;
    }
    RetrouveBanqueQuery.first();
    EnTete.replace("{{TITRE1}}"            , RetrouveBanqueQuery.value(3).toString().toUpper());
    EnTete.replace("{{PRENOM PATIENT}}"    , "");
    EnTete.replace("{{NOM PATIENT}}"       , RetrouveBanqueQuery.value(2).toString());
    EnTete.replace("{{TITRE}}"             , "Compte " + RetrouveBanqueQuery.value(1).toString());
    EnTete.replace("{{DDN}}"               , "<font color = \"" + proc->CouleurTitres + "\">Remise de chèques n° " + QString::number(idRemise) + "</font>");

    // création du pied
    QString Pied = proc->ImpressionPied();
    if (Pied == "") return false;

    // creation du corps
    QString lignecheque = "<table width=\"490\" border=\"1\" cellspacing=\"0\" cellpadding=\"5\">"
            "<tr>"
              "<td width=\"200\" ><span style=\"font-size:9pt;\">{{NOM PATIENT}}</span></td>"
              "<td width=\"180\" ><span style=\"font-size:9pt;\">{{NOM BANQUE}}</span></td>"
              "<td width=\"110\" ><div align=\"right\"><span style=\"font-size:9pt;\">{{MONT REGLT}}</span></div></td>"
          "</tr>"
          "</table>";

    QString texteordo;
    for (int k = 0; k < ui->ListeChequesupTableWidget->rowCount(); k++)
    {
        // Remplacement des variables par les valeurs lues.
        LigneChq = lignecheque;
        LigneChq.replace("{{NOM PATIENT}}", ui->ListeChequesupTableWidget->item(k,2)->text());
        LigneChq.replace("{{NOM BANQUE}}", ui->ListeChequesupTableWidget->item(k,3)->text());
        if (gMode == RevoirRemisesPrecs) {
            LigneChq.replace("{{MONT REGLT}}", ui->ListeChequesupTableWidget->item(k,4)->text());
            gtotalMontRemise += QLocale().toDouble(ui->ListeChequesupTableWidget->item(k,4)->text());
        }
        else if (gMode == NouvelleRemise) {
            QLineEdit *line = static_cast<QLineEdit*>(ui->ListeChequesupTableWidget->cellWidget(k,4));
            LigneChq.replace("{{MONT REGLT}}", line->text());
            gtotalMontRemise += QLocale().toDouble(line->text());
        }
        texteordo += LigneChq;
        gtotalNbrePieces ++;
    }
    texteordo += "<table width=\"490\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\">"
            "<tr>"
              "<td width=\"490\" ><div align=\"right\"><span style=\"font-size:10pt;font-weight:bold\">{{TOTAL REMISE}}</span></div></td>"
            "</tr>"
            "</table>";
    QString totalchq = QString::number(gtotalNbrePieces) + " chèque";
    if (gtotalNbrePieces>1) totalchq += "s";
    texteordo.replace("{{TOTAL REMISE}}", tr("TOTAL - ") + totalchq + tr(" en euros - ") + QString::number(gtotalMontRemise,'f',2));
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setHtml(texteordo);

    bool a = proc->Imprime_Etat(textEdit, EnTete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
    delete textEdit;
    return a;
}

/*---------------------------------------------------------------------------------
    Reconstruit la liste des utilisateurs du combobox
-----------------------------------------------------------------------------------*/
void dlg_remisecheques::ReconstruitListeUsers()
{
    QString req = "select userlogin, ut.iduser, idCompte, NomCompteAbrege from " NOM_TABLE_UTILISATEURS " ut," NOM_TABLE_COMPTES " cpt"
            " where ut.iduser = cpt.idUser"
            " and (((Soignant < 4 and  UserEnregHonoraires = 1) or soignant = 5)"
            " and userdesactive is null)"
            " and desactive is null";
    //UpMessageBox::Watch(this,req);
    QSqlQuery quer(req,db);
    ui->UserComboBox->clear();
    for (int i=0; i<quer.size(); i++)
    {
        quer.seek(i);
        QString req;
        req =   "SELECT idRecette FROM " NOM_TABLE_RECETTES " pai"
                " WHERE pai.idRecette in (SELECT lig.idRecette FROM " NOM_TABLE_LIGNESPAIEMENTS " lig WHERE lig.idActe in"
                " (SELECT act.idActe FROM " NOM_TABLE_ACTES " act WHERE UserComptable = " + quer.value(1).toString() + "))"
                " AND pai.IdRemise IS NULL"
                " AND ModePaiement = 'C'"
                " ORDER BY TireurCheque";
        if (QSqlQuery(req, db).size()>0)
        {
            QMap<QString, QVariant> map;
            map["iduser"]   = quer.value(1).toInt();
            map["idcompte"] = quer.value(2).toInt();
            ui->UserComboBox->insertItem(0, quer.value(0).toString() + " - " + quer.value(3).toString(), map);
        }
    }
    ui->UserComboBox->setCurrentIndex(0);
    QString req1 = "select idCompteEncaissHonoraires from " NOM_TABLE_UTILISATEURS
            " where iduser = " + ui->UserComboBox->currentData().toMap().value("iduser").toString() +
            " and idCompteEncaissHonoraires is not null";
    //qDebug() << req1;
    QSqlQuery idcptdefquer(req1,db);
    if (idcptdefquer.size()>0)
    {
        idcptdefquer.first();
        QMap<QString, QVariant> map;
        map["iduser"]   = ui->UserComboBox->currentData().toMap().value("iduser").toInt();
        map["idcompte"] = idcptdefquer.value(0).toInt();
        int a = ui->UserComboBox->findData(map);
        ui->UserComboBox->setCurrentIndex(a<0? 0 : a);
    }
    Slot_ChangeCompte();
}
