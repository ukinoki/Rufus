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
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_bilanortho.h"
#include "ui_dlg_bilanortho.h"

dlg_bilanortho::dlg_bilanortho(Procedures *procAPasser, int idActeAPAsser, int idBilanOrtho, QWidget *parent, int idpat) :
    QDialog(parent),
    ui(new Ui::dlg_bilanortho)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    idBilan     = idBilanOrtho;
    idActe      = idActeAPAsser;
    proc        = procAPasser;
    gidpat      = idpat;
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionBilanOrtho").toByteArray());

    XELlist << "-";
    XELlist << "Xt";
    XELlist << "XXt";
    XELlist << "X";
    XELlist << "O";
    XELlist << "E";
    XELlist << "EEt";
    XELlist << "Et";

    XEPlist << "-";
    XEPlist << "X't";
    XEPlist << "X'X't";
    XEPlist << "X'";
    XEPlist << "O'";
    XEPlist << "E'";
    XEPlist << "E'E't";
    XEPlist << "E't";

    HLlist << "-";
    HLlist << "Ht";
    HLlist << "HHt";
    HLlist << "H";
    HLlist << "O";
    HLlist << "ht";
    HLlist << "hht";
    HLlist << "h";

    HPlist << "-";
    HPlist << "H't";
    HPlist << "H'H't";
    HPlist << "H'";
    HPlist << "O'";
    HPlist << "h't";
    HPlist << "h'h't";
    HPlist << "h'";

    OcclAlternlist << "-";
    OcclAlternlist << tr("supportée des 2 côtés");
    OcclAlternlist << tr("non supportée à droite");
    OcclAlternlist << tr("non supportée à gauche");
    OcclAlternlist << tr("test non fiable");

    Dioptrieslist << "-" << "1" << "2" << "3" << "4" << "6" << "8" << "10" << "12" << "14" << "16" << "18" << "20" << "25" << "30" << "35" << "40" << "45";
    DioptriesNeutraliselist << "-" << tr("neutralise OD") << tr("neutralise OG") << "1" << "2" << "4" << "6" << "8" << "10" << "12" << "14" << "16" << "18" << "20" << "25" << "30" << "35" << "40" << "45";
    HDioptrieslist << "-" << "1" << "2" << "3"  << "4" << "5" << "6"  << "8" << "10" << "12" << "14" << "16" << "18" << "20" << "25" << "30";
    HDioptriesNeutraliselist << "-" << tr("neutralise OD") << tr("neutralise OG") << "1" << "2" << "3"  << "4" << "5" << "6"  << "8" << "10" << "12" << "14" << "16" << "18" << "20" << "25" << "30";

    PPClist << "-";
    for (int i=2; i<11; i++)
        PPClist << QString::number(i) + " cm";

    Wirtlist << "-";
    for (int i=9; i>0; i--)
        Wirtlist << QString::number(i);
    Wirtlist << "voit la mouche";
    Wirtlist << "non vu";

    AnimauxWirtlist << "-";
    AnimauxWirtlist << "A";
    AnimauxWirtlist << "B";
    AnimauxWirtlist << "C";
    AnimauxWirtlist << "0";

    Langlist << "-";
    Langlist << tr("Etoile");
    Langlist << "200\"";
    Langlist << "400\"";
    Langlist << "550\"";
    Langlist << "600\"";
    Langlist << "1200\"";
    Langlist << tr("non vu");

    TNOlist << "-";
    TNOlist << "15\"";
    TNOlist << "30\"";
    TNOlist << "60\"";
    TNOlist << "120\"";
    TNOlist << "240\"";
    TNOlist << "480\"";
    TNOlist << tr("non vu");

    VergencesCLlist << "-";
    for (int i=1; i<Dioptrieslist.size(); i++)
        VergencesCLlist << "C " + Dioptrieslist.at(i);
    VergencesCPlist << "-";
    for (int i=1; i<Dioptrieslist.size(); i++)
        VergencesCPlist << "C'" + Dioptrieslist.at(i);
    VergencesDPlist << "-";
    for (int i=1; i<Dioptrieslist.size(); i++)
        VergencesDPlist << "D'" + Dioptrieslist.at(i);
    VergencesDLlist << "-";
    for (int i=1; i<Dioptrieslist.size(); i++)
        VergencesDLlist << "D " + Dioptrieslist.at(i);

    ui->OcclAlterncomboBox->insertItems(0,OcclAlternlist);

    ui->WirtcomboBox->insertItems(0,Wirtlist);
    ui->AnimauxWirtcomboBox->insertItems(0,AnimauxWirtlist);
    ui->LangcomboBox->insertItems(0,Langlist);
    ui->TNOcomboBox->insertItems(0,TNOlist);

    ui->EcranVLSCcomboBox->insertItems(0,XELlist);
    ui->EcranVLSCDcomboBox->insertItems(0,Dioptrieslist);
    ui->EcranVPSCcomboBox->insertItems(0,XEPlist);
    ui->EcranVPSCDcomboBox->insertItems(0,Dioptrieslist);
    ui->EcranVLASCcomboBox->insertItems(0,XELlist);
    ui->EcranVLASCDcomboBox->insertItems(0,Dioptrieslist);
    ui->EcranVPASCcomboBox->insertItems(0,XEPlist);
    ui->EcranVPASCDcomboBox->insertItems(0,Dioptrieslist);

    ui->HEcranVLSCcomboBox->insertItems(0,HLlist);
    ui->HEcranVLSCDcomboBox->insertItems(0,HDioptrieslist);
    ui->HEcranVLASCcomboBox->insertItems(0,HLlist);
    ui->HEcranVLASCDcomboBox->insertItems(0,HDioptrieslist);
    ui->HEcranVPSCcomboBox->insertItems(0,HPlist);
    ui->HEcranVPSCDcomboBox->insertItems(0,HDioptrieslist);
    ui->HEcranVPASCcomboBox->insertItems(0,HPlist);
    ui->HEcranVPASCDcomboBox->insertItems(0,HDioptrieslist);

    ui->MaddoxVLSCcomboBox->insertItems(0,XELlist);
    ui->MaddoxVPSCcomboBox->insertItems(0,XEPlist);
    ui->MaddoxVLASCcomboBox->insertItems(0,XELlist);
    ui->MaddoxVPASCcomboBox->insertItems(0,XEPlist);
    ui->HMaddoxVLSCcomboBox->insertItems(0,HLlist);
    ui->HMaddoxVPSCcomboBox->insertItems(0,HPlist);
    ui->HMaddoxVLASCcomboBox->insertItems(0,HLlist);
    ui->HMaddoxVPASCcomboBox->insertItems(0,HPlist);

    ui->PPCcomboBox->insertItems(0,PPClist);
    ui->VergenceDLcomboBox->insertItems(0,VergencesDLlist);
    ui->VergenceCLcomboBox->insertItems(0,VergencesCLlist);
    ui->VergenceRestDLcomboBox->insertItems(0,VergencesDLlist);
    ui->VergenceDPcomboBox->insertItems(0,VergencesDPlist);
    ui->VergenceCPcomboBox->insertItems(0,VergencesCPlist);
    ui->VergenceRestDPcomboBox->insertItems(0,VergencesDPlist);

    ui->AnimauxWirtcomboBox->setEnabled(false);

    ui->EcranVLSCDcomboBox->setVisible(false);
    ui->fixSCVLcomboBox->setVisible(false);
    ui->EcranVPSCDcomboBox->setVisible(false);
    ui->fixSCVPcomboBox->setVisible(false);
    ui->EcranVLASCDcomboBox->setVisible(false);
    ui->fixASCVLcomboBox->setVisible(false);
    ui->EcranVPASCDcomboBox->setVisible(false);
    ui->fixASCVPcomboBox->setVisible(false);

    ui->HEcranVLSCDcomboBox->setVisible(false);
    ui->HfixSCVLcomboBox->setVisible(false);
    ui->HEcranVPSCDcomboBox->setVisible(false);
    ui->HfixSCVPcomboBox->setVisible(false);
    ui->HEcranVLASCDcomboBox->setVisible(false);
    ui->HfixASCVLcomboBox->setVisible(false);
    ui->HEcranVPASCDcomboBox->setVisible(false);
    ui->HfixASCVPcomboBox->setVisible(false);

    ui->MaddoxVLSCDcomboBox->setVisible(false);
    ui->MaddoxVLASCDcomboBox->setVisible(false);
    ui->MaddoxVPSCDcomboBox->setVisible(false);
    ui->MaddoxVPASCDcomboBox->setVisible(false);
    ui->HMaddoxVLSCDcomboBox->setVisible(false);
    ui->HMaddoxVLASCDcomboBox->setVisible(false);
    ui->HMaddoxVPSCDcomboBox->setVisible(false);
    ui->HMaddoxVPASCDcomboBox->setVisible(false);

    CouleurTitres = proc->CouleurTitres;

    connect(ui->OKupPushButton,         &QPushButton::clicked,                                  [=] {accept();});
    connect(ui->AnnulupPushButton,      &QPushButton::clicked,                                  [=] {reject();});
    connect(ui->ImprimeupPushButton,    &QPushButton::clicked,                                  [=] {ImprimeBOClicked();});
    QList<UpComboBox*> listcombo = findChildren<UpComboBox*>();
    foreach(UpComboBox *cbox, listcombo)
        connect(cbox,                   QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {AfficheCombobox(cbox->currentIndex(), cbox);});
    connect(ui->WirtcomboBox,           &QComboBox::currentTextChanged,                         [=] {EnableAnimauxWirtcomboBox(ui->WirtcomboBox->currentText());});

    ui->AnnulupPushButton->setUpButtonStyle(UpPushButton::ANNULBUTTON, UpPushButton::Small);
    ui->OKupPushButton->setUpButtonStyle(UpPushButton::OKBUTTON, UpPushButton::Small);
    ui->ImprimeupPushButton->setUpButtonStyle(UpPushButton::IMPRIMEBUTTON,UpPushButton::Small);

    ui->OrientationgroupBox->setFont(qApp->font());
    ui->OeilDirecteurgroupBox->setFont(qApp->font());

    if (idBilan > 0)    AfficheBilan(idBilan);
}

dlg_bilanortho::~dlg_bilanortho()
{
    delete ui;
}

void dlg_bilanortho::EnableAnimauxWirtcomboBox(QString PionWirt)
{
    bool *b = new bool;
    int a = PionWirt.toInt(b);
    if (b)
    {
        if (a > 0 && a < 5)
            ui->AnimauxWirtcomboBox->setEnabled(true);
        else
        {
            ui->AnimauxWirtcomboBox->setEnabled(false);
            ui->AnimauxWirtcomboBox->setCurrentText("-");
        }
    }
    else
        ui->AnimauxWirtcomboBox->setCurrentText("-");
    delete b;
}

void dlg_bilanortho::ImprimeBOClicked()
{
    bool AvecDupli   = false;
    bool AvecPrevisu = true;
    bool AvecNumPage = false;

    QString requete = " select Patnom, patprenom, actedate, creepar, idUser  from " NOM_TABLE_PATIENTS " pat," NOM_TABLE_ACTES " act"
            " where act.idacte = " + QString::number(idActe) + " and act.idpat = pat.idpat";
    //UpMessageBox::Watch(this,requete);
    QSqlQuery cherchepatquery (requete,DataBase::getInstance()->getDataBase());
    DataBase::getInstance()->traiteErreurRequete(cherchepatquery,requete,tr("erreur dans dlg_bilanortho") + " - Slot_ImprimeBOClicekd()");
    if (cherchepatquery.size() == 0) return;
    cherchepatquery.first();
    int userentete = cherchepatquery.value(4).toInt();
    gDataUser = proc->getUserById(userentete);
    if (gDataUser == nullptr)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
        return;
    }
    QString date = cherchepatquery.value(2).toDate().toString(tr("d MMM yyyy"));
    QString nom = cherchepatquery.value(0).toString().toUpper();
    QString prenom = cherchepatquery.value(1).toString();

    QString Entete, Pied;

    //création de l'entête
    Entete = proc->ImpressionEntete(QDate::currentDate(), gDataUser).value("Norm");
    if (Entete == "") return;
    Entete.replace("{{TITRE1}}"            , "");
    Entete.replace("{{TITRE}}"             , "<font color = \"" + proc->CouleurTitres + "\">" + tr("BILAN ORTHOPTIQUE DU ") + date + "</font>");
    Entete.replace("{{PRENOM PATIENT}}"       , prenom);
    Entete.replace("{{NOM PATIENT}}"       , nom);
    Entete.replace("{{DDN}}"               , "");
    Entete.replace("{{DATE}}"              , gDataUser->getSite()->ville() + ", le " + QDate::currentDate().toString(tr("d MMM yyyy")));

    // création du pied
    Pied = proc->ImpressionPied();
    if (Pied == "") return;

    // creation du corps du bilan
    QTextEdit *textHtml;
    textHtml = new QTextEdit;
    //Motif
    QString Motif= ui->MotiftextEdit->toPlainText();
    if (Motif != "")
         Motif = "<p><td width=\"70\"><font color = \"" + CouleurTitres + "\">" + tr("MOTIF") + "</font></td></p>"
                 + "<p><td width=\"40\"></td><td width=\"400\">" + ui->MotiftextEdit->toHtml() + "</td></p><p></p>";

    textHtml->setText(Motif + calcReponsehTml() + "<p></p>");

    if (ui->ConclusiontextEdit->toPlainText() != "")
        textHtml->setText(textHtml->toHtml() + "<p></p><p><td width=\"140\"><font color = \"" + CouleurTitres + "\">" + tr("CONCLUSION") + "</font></td></p>"
                          +"<p><td width=\"40\"></td><td width=\"400\">" + ui->ConclusiontextEdit->toHtml() + "</td></p>");
    textHtml->setText(textHtml->toHtml().replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:9pt"));

    bool aa = proc->Imprime_Etat(textHtml, Entete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        QSqlQuery query = QSqlQuery(DataBase::getInstance()->getDataBase());
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, sousTypedoc, Titre, TextEntete, TextCorps, TextPied, Dateimpression, UserEmetteur, ALD, EmisRecu, FormatDoc, idLieu)"
                                                           " values(:iduser, :idpat, :typeDoc, :soustypedoc, :titre, :textEntete, :textCorps, :textPied, :dateimpression, :useremetteur, :ald, :emisrecu, :formatdoc, :idlieu)");
        query.bindValue(":iduser", QString::number(proc->getUserConnected()->id()));
        query.bindValue(":idpat", QString::number(gidpat));
        query.bindValue(":typeDoc", "Orthoptie");
        query.bindValue(":soustypedoc", "Bilan");
        query.bindValue(":titre", "Bilan orthoptique");
        query.bindValue(":textEntete", Entete);
        query.bindValue(":textCorps", textHtml->toHtml());
        query.bindValue(":textPied", Pied);
        query.bindValue(":dateimpression", cherchepatquery.value(2).toDate().toString("yyyy-MM-dd"));
        query.bindValue(":useremetteur", QString::number(proc->getUserConnected()->id()));
        query.bindValue(":emisrecu", "0");
        query.bindValue(":formatdoc", BILANORTHOPTIQUE);
        query.bindValue(":idlieu", QString::number(proc->getUserConnected()->getSite()->id()));
        if(!query.exec())
            UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
    }
    delete textHtml;
 }

void    dlg_bilanortho::AfficheCombobox(int i, QWidget *widg)
{
    if (widg == ui->EcranVLSCcomboBox)
    {
        ui->EcranVLSCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->EcranVLSCcomboBox->currentText();
        ui->fixSCVLcomboBox->setVisible(a == "Et" || a == "EEt" || a == "Xt" || a == "XXt");
        ui->fixSCVLcomboBox->clear();
        ui->fixSCVLcomboBox->clearEditText();
        QStringList typefix;
        if (a == "Et" || a == "Xt")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixSCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "EEt" || a == "XXt")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixSCVLcomboBox->insertItems(0,typefix);
        }
        ui->fixSCVLcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->EcranVLASCcomboBox)
    {
        ui->EcranVLASCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->EcranVLASCcomboBox->currentText();
        ui->fixASCVLcomboBox->setVisible(a == "Et" || a == "EEt" || a == "Xt" || a == "XXt");
        ui->fixASCVLcomboBox->clear();
        ui->fixASCVLcomboBox->clearEditText();
        QStringList typefix;
        if (a == "Et" || a == "Xt")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixASCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "EEt" || a == "XXt")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixASCVLcomboBox->insertItems(0,typefix);
        }
        ui->fixASCVLcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->EcranVPSCcomboBox)
    {
        ui->EcranVPSCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->EcranVPSCcomboBox->currentText();
        ui->fixSCVPcomboBox->setVisible(a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't");
        ui->fixSCVPcomboBox->clear();
        ui->fixSCVPcomboBox->clearEditText();
        QStringList typefix;
        if (a == "E't" || a == "X't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixSCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "E'E't" || a == "X'X't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixSCVPcomboBox->insertItems(0,typefix);
        }
        ui->fixSCVPcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->EcranVPASCcomboBox)
    {
        ui->EcranVPASCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->EcranVPASCcomboBox->currentText();
        ui->fixASCVPcomboBox->setVisible(a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't");
        ui->fixASCVPcomboBox->clear();
        ui->fixASCVPcomboBox->clearEditText();
        QStringList typefix;
        if (a == "E't" || a == "X't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixASCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "E'E't" || a == "X'X't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixASCVPcomboBox->insertItems(0,typefix);
        }
        ui->fixASCVPcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->HEcranVLSCcomboBox)
    {
        ui->HEcranVLSCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->HEcranVLSCcomboBox->currentText();
        ui->HfixSCVLcomboBox->setVisible(a == "Ht" || a == "HHt" || a == "ht" || a == "hht");
        ui->HfixSCVLcomboBox->clear();
        ui->HfixSCVLcomboBox->clearEditText();
        QStringList typefix;
        if (a == "Ht" || a == "ht")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixSCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "HHt" || a == "hht")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixSCVLcomboBox->insertItems(0,typefix);
        }
        ui->HfixSCVLcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->HEcranVLASCcomboBox)
    {
        ui->HEcranVLASCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->HEcranVLASCcomboBox->currentText();
        ui->HfixASCVLcomboBox->setVisible(a == "Ht" || a == "HHt" || a == "ht" || a == "hht");
        ui->HfixASCVLcomboBox->clear();
        ui->HfixASCVLcomboBox->clearEditText();
        QStringList typefix;
        if (a == "Ht" || a == "ht")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixASCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "HHt" || a == "hht")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixASCVLcomboBox->insertItems(0,typefix);
        }
        ui->HfixASCVLcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->HEcranVPSCcomboBox)
    {
        ui->HEcranVPSCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->HEcranVPSCcomboBox->currentText();
        ui->HfixSCVPcomboBox->setVisible(a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't");
        ui->HfixSCVPcomboBox->clear();
        ui->HfixSCVPcomboBox->clearEditText();
        QStringList typefix;
        if (a == "H't" || a == "h't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixSCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "H'H't" || a == "h'h't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixSCVPcomboBox->insertItems(0,typefix);
        }
        ui->HfixSCVPcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->HEcranVPASCcomboBox)
    {
        ui->HEcranVPASCDcomboBox->setVisible(i!=4 && i!=0);
        QString a = ui->HEcranVPASCcomboBox->currentText();
        ui->HfixASCVPcomboBox->setVisible(a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't");
        ui->HfixASCVPcomboBox->clear();
        ui->HfixASCVPcomboBox->clearEditText();
        QStringList typefix;
        if (a == "H't" || a == "h't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixASCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "H'H't" || a == "h'h't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixASCVPcomboBox->insertItems(0,typefix);
        }
        ui->HfixASCVPcomboBox->setCurrentIndex(0);
    }
    if (widg == ui->MaddoxVLSCcomboBox)
    {
        ui->MaddoxVLSCDcomboBox->setVisible(i!=4 && i!=0);
        ui->MaddoxVLSCDcomboBox->clear();
        ui->MaddoxVLSCDcomboBox->clearEditText();
        QString a = ui->MaddoxVLSCcomboBox->currentText();
        int w = 50;
        if (a == "Et" || a == "EEt" || a == "Xt" || a == "XXt")
        {
            ui->MaddoxVLSCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVLSCDcomboBox->insertItems(0,HDioptrieslist);
        ui->MaddoxVLSCDcomboBox->resize(w,22);
    }
    if (widg == ui->MaddoxVLASCcomboBox)
    {
        ui->MaddoxVLASCDcomboBox->setVisible(i!=4 && i!=0);
        ui->MaddoxVLASCDcomboBox->clear();
        ui->MaddoxVLASCDcomboBox->clearEditText();
        QString a = ui->MaddoxVLASCcomboBox->currentText();
        int w = 50;
        if (a == "Et" || a == "EEt" || a == "Xt" || a == "XXt")
        {
            ui->MaddoxVLASCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVLASCDcomboBox->insertItems(0,HDioptrieslist);
        ui->MaddoxVLASCDcomboBox->resize(w,22);
    }
    if (widg == ui->MaddoxVPSCcomboBox)
    {
        ui->MaddoxVPSCDcomboBox->setVisible(i!=4 && i!=0);
        ui->MaddoxVPSCDcomboBox->clear();
        ui->MaddoxVPSCDcomboBox->clearEditText();
        QString a = ui->MaddoxVPSCcomboBox->currentText();
        int w = 50;
        if (a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't")
        {
            ui->MaddoxVPSCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVPSCDcomboBox->insertItems(0,HDioptrieslist);
        ui->MaddoxVPSCDcomboBox->resize(w,22);
    }
    if (widg == ui->MaddoxVPASCcomboBox)
    {
        ui->MaddoxVPASCDcomboBox->setVisible(i!=4 && i!=0);
        ui->MaddoxVPASCDcomboBox->clear();
        ui->MaddoxVPASCDcomboBox->clearEditText();
        QString a = ui->MaddoxVPASCcomboBox->currentText();
        int w = 50;
        if (a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't")
        {
            ui->MaddoxVPASCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVPASCDcomboBox->insertItems(0,HDioptrieslist);
        ui->MaddoxVPASCDcomboBox->resize(w,22);
    }
    if (widg == ui->HMaddoxVLSCcomboBox)
    {
        ui->HMaddoxVLSCDcomboBox->setVisible(i!=4 && i!=0);
        ui->HMaddoxVLSCDcomboBox->clear();
        ui->HMaddoxVLSCDcomboBox->clearEditText();
        QString a = ui->HMaddoxVLSCcomboBox->currentText();
        int w = 50;
        if (a == "Ht" || a == "HHt" || a == "ht" || a == "hht")
        {
            ui->HMaddoxVLSCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVLSCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVLSCDcomboBox->resize(w,22);
    }
    if (widg == ui->HMaddoxVLASCcomboBox)
    {
        ui->HMaddoxVLASCDcomboBox->setVisible(i!=4 && i!=0);
        ui->HMaddoxVLASCDcomboBox->clear();
        ui->HMaddoxVLASCDcomboBox->clearEditText();
        QString a = ui->HMaddoxVLASCcomboBox->currentText();
        int w = 50;
        if (a == "Ht" || a == "HHt" || a == "ht" || a == "hht")
        {
            ui->HMaddoxVLASCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVLASCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVLASCDcomboBox->resize(w,22);
    }
    if (widg == ui->HMaddoxVPSCcomboBox)
    {
        ui->HMaddoxVPSCDcomboBox->setVisible(i!=4 && i!=0);
        ui->HMaddoxVPSCDcomboBox->clear();
        ui->HMaddoxVPSCDcomboBox->clearEditText();
        QString a = ui->HMaddoxVPSCcomboBox->currentText();
        int w = 50;
        if (a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't")
        {
            ui->HMaddoxVPSCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVPSCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVPSCDcomboBox->resize(w,22);
    }
    if (widg == ui->HMaddoxVPASCcomboBox)
    {
        ui->HMaddoxVPASCDcomboBox->setVisible(i!=4 && i!=0);
        ui->HMaddoxVPASCDcomboBox->clear();
        ui->HMaddoxVPASCDcomboBox->clearEditText();
        QString a = ui->HMaddoxVPASCcomboBox->currentText();
        int w = 50;
        if (a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't")
        {
            ui->HMaddoxVPASCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVPASCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVPASCDcomboBox->resize(w,22);
    }
}

void dlg_bilanortho::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionBilanOrtho", saveGeometry());
    event->accept();
}

void dlg_bilanortho::AfficheBilan(int idBilan)
{
    QString chborequete = "select idBilanOrtho from " NOM_TABLE_BILANORTHO " where idBilanOrtho = " + QString::number(idBilan);
    QSqlQuery chboquery(chborequete,DataBase::getInstance()->getDataBase());
    if (chboquery.size() > 0)
    {
        QString a;
        QString affichBOrequete =
                "select AVOD, AVOG, OcclAltern, Wirt, Lang"                                                 // 0,1,2,3,4
                ", ODirecteur, Orientation, EcranVLSC, EcranVLSCD, EcranfixresVLSC"                         // 5,6,7,8,9
                ", EcranVPSC, EcranVPSCD, EcranfixresVPSC, EcranVLASC, EcranVLASCD"                         // 10,11,12,13,14
                ", EcranfixresVLASC, EcranVPASC, EcranVPASCD, EcranfixresVPASC, MaddoxVLSC"                 // 15,16,17,18,19
                ", MaddoxVLSCD, MaddoxVPSC, MaddoxVPSCD, MaddoxVLASC, MaddoxVLASCD"                         // 20,21,22,23,24
                ", MaddoxVPASC, MaddoxVPASCD, Motilite, PPC, PPCComment"                                    // 25,26,27,28,29
                ", Saccades, Poursuite, Worth1, Worth2, Bagolini1"                                          // 30,31,32,33,34
                ", Bagolini2, VergenceVLD, VergenceVLC, VergenceVPD, VergenceVPC"                           // 35,36,37,38,39
                ", Synopt1, Synopt2, Synopt3, Conclusion, Motif"                                            // 40,41,42,43,44
                ", WirtAnimaux, TNO, VergenceRestVLD, VergenceRestVPD, HEcranVLSC"                          // 45,46,47,48,49
                ", HEcranVLSCD, HEcranfixresVLSC, HEcranVPSC, HEcranVPSCD, HEcranfixresVPSC"                // 50,51,52,53,54
                ", HEcranVLASC, HEcranVLASCD, HEcranfixresVLASC, HEcranVPASC, HEcranVPASCD"                 // 55,56,57,58,59
                ", HEcranfixresVPASC, HMaddoxVLSC, HMaddoxVLSCD, HMaddoxVPSC, HMaddoxVPSCD"                 // 60,61,62,63,64
                ", HMaddoxVLASC, HMaddoxVLASCD, HMaddoxVPASC, HMaddoxVPASCD, Motilite"                      // 65,66,67,68,69
                " from " NOM_TABLE_BILANORTHO     // 65,66,67,68
                " where idBilanOrtho = " + QString::number(idBilan);
        //UpMessageBox::Watch(this,affichBOrequete);
        QSqlQuery affichBOquery(affichBOrequete,DataBase::getInstance()->getDataBase());
        DataBase::getInstance()->traiteErreurRequete(affichBOquery,affichBOrequete,"");
        affichBOquery.first();
        ui->MotiftextEdit->setText(affichBOquery.value(44).toString());
        ui->AVODlineEdit->setText(affichBOquery.value(0).toString());
        ui->AVOGlineEdit->setText(affichBOquery.value(1).toString());
        ui->OcclAlterncomboBox->setCurrentText(affichBOquery.value(2).toString());
        ui->WirtcomboBox->setCurrentText(affichBOquery.value(3).toString());
        ui->AnimauxWirtcomboBox->setCurrentText(affichBOquery.value(45).toString());
        ui->LangcomboBox->setCurrentText(affichBOquery.value(4).toString());
        ui->TNOcomboBox->setCurrentText(affichBOquery.value(46).toString());
        if (affichBOquery.value(5).toString() == "D")
            ui->ODdirecteurradioButton->setChecked(true);
        else if (affichBOquery.value(5).toString() == "G")
            ui->OGdirecteurradioButton->setChecked(true);
        if (affichBOquery.value(6).toString() == "D")
            ui->ODOrientationradioButton->setChecked(true);
        else if (affichBOquery.value(6).toString() == "G")
            ui->OGOrientationradioButton->setChecked(true);

        // ECRAN ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        ui->EcranVLSCcomboBox->setCurrentText(affichBOquery.value(7).toString());
        a = ui->EcranVLSCcomboBox->currentText();
        ui->EcranVLSCDcomboBox->setVisible(a == "E" || a == "Et" || a == "EEt" || a == "X" || a == "Xt" || a == "XXt");
        ui->fixSCVLcomboBox->setVisible(a == "Et" || a == "EEt" || a == "Xt" || a == "XXt");
        ui->fixSCVLcomboBox->clear();
        QStringList typefix;
        if (a == "Et" || a == "Xt")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixSCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "EEt" || a == "XXt")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixSCVLcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(8).toString();
        if (a == "") a = "-";
        ui->EcranVLSCDcomboBox->setCurrentText(a);
        ui->fixSCVLcomboBox->setCurrentText(affichBOquery.value(9).toString());
        ui->EcranVPSCcomboBox->setCurrentText(affichBOquery.value(10).toString());
        a = ui->EcranVPSCcomboBox->currentText();
        ui->EcranVPSCDcomboBox->setVisible(a == "E'" || a == "E't" || a == "E'E't" || a == "X'" || a == "X't" || a == "X'X't");
        ui->fixSCVPcomboBox->setVisible(a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't");
        ui->fixSCVPcomboBox->clear();
        typefix.clear();
        if (a == "E't" || a == "X't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixSCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "E'E't" || a == "X'X't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixSCVPcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(11).toString();
        if (a == "") a = "-";
        ui->EcranVPSCDcomboBox->setCurrentText(a);
        ui->fixSCVPcomboBox->setCurrentText(affichBOquery.value(12).toString());
        ui->EcranVLASCcomboBox->setCurrentText(affichBOquery.value(13).toString());
        a = ui->EcranVLASCcomboBox->currentText();
        ui->EcranVLASCDcomboBox->setVisible(a == "E" || a == "Et" || a == "EEt" || a == "X" || a == "Xt" || a == "XXt");
        ui->fixASCVLcomboBox->setVisible(a == "Et" || a == "EEt" || a == "Xt" || a == "XXt");
        ui->fixASCVLcomboBox->clear();
        typefix.clear();
        if (a == "Et" || a == "Xt")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixASCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "EEt" || a == "XXt")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixASCVLcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(14).toString();
        if (a == "") a = "-";
        ui->EcranVLASCDcomboBox->setCurrentText(a);
        ui->fixASCVLcomboBox->setCurrentText(affichBOquery.value(15).toString());
        ui->EcranVPASCcomboBox->setCurrentText(affichBOquery.value(16).toString());
        a = ui->EcranVPASCcomboBox->currentText();
        ui->EcranVPASCDcomboBox->setVisible(a == "E'" || a == "E't" || a == "E'E't" || a == "X'" || a == "X't" || a == "X'X't");
        ui->fixASCVPcomboBox->setVisible(a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't");
        ui->fixASCVPcomboBox->clear();
        typefix.clear();
        if (a == "E't" || a == "X't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->fixASCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "E'E't" || a == "X'X't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->fixASCVPcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(17).toString();
        if (a == "") a = "-";
        ui->EcranVPASCDcomboBox->setCurrentText(a);
        ui->fixASCVPcomboBox->setCurrentText(affichBOquery.value(18).toString());

        // HECRAN -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        ui->HEcranVLSCcomboBox->setCurrentText(affichBOquery.value(49).toString());
        a = ui->HEcranVLSCcomboBox->currentText();
        ui->HEcranVLSCDcomboBox->setVisible(a == "H" || a == "Ht" || a == "HHt" || a == "h" || a == "ht" || a == "hht");
        ui->HfixSCVLcomboBox->setVisible(a == "Ht" || a == "HHt" || a == "ht" || a == "hht");
        ui->HfixSCVLcomboBox->clear();
        typefix.clear();
        if (a == "Ht" || a == "ht")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixSCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "HHt" || a == "hht")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixSCVLcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(50).toString();
        if (a == "") a = "-";
        ui->HEcranVLSCDcomboBox->setCurrentText(a);
        ui->HfixSCVLcomboBox->setCurrentText(affichBOquery.value(51).toString());
        ui->HEcranVPSCcomboBox->setCurrentText(affichBOquery.value(52).toString());
        a = ui->HEcranVPSCcomboBox->currentText();
        ui->HEcranVPSCDcomboBox->setVisible(a == "H'" || a == "H't" || a == "H'H't" || a == "h'" || a == "h't" || a == "h'h't");
        ui->HfixSCVPcomboBox->setVisible(a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't");
        ui->HfixSCVPcomboBox->clear();
        typefix.clear();
        if (a == "H't" || a == "h't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixSCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "H'H't" || a == "h'h't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixSCVPcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(53).toString();
        if (a == "") a = "-";
        ui->HEcranVPSCDcomboBox->setCurrentText(a);
        ui->HfixSCVPcomboBox->setCurrentText(affichBOquery.value(54).toString());
        ui->HEcranVLASCcomboBox->setCurrentText(affichBOquery.value(55).toString());
        a = ui->HEcranVLASCcomboBox->currentText();
        ui->HEcranVLASCDcomboBox->setVisible(a == "H" || a == "Ht" || a == "HHt" || a == "h" || a == "ht" || a == "hht");
        ui->HfixASCVLcomboBox->setVisible(a == "Ht" || a == "HHt" || a == "ht" || a == "hht");
        ui->HfixASCVLcomboBox->clear();
        typefix.clear();
        if (a == "Ht" || a == "ht")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixASCVLcomboBox->insertItems(0,typefix);
        }
        if (a == "HHt" || a == "hht")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixASCVLcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(56).toString();
        if (a == "") a = "-";
        ui->HEcranVLASCDcomboBox->setCurrentText(a);
        ui->HfixASCVLcomboBox->setCurrentText(affichBOquery.value(57).toString());
        ui->HEcranVPASCcomboBox->setCurrentText(affichBOquery.value(58).toString());
        a = ui->HEcranVPASCcomboBox->currentText();
        ui->HEcranVPASCDcomboBox->setVisible(a == "H'" || a == "H't" || a == "H'H't" || a == "h'" || a == "h't" || a == "h'h't");
        ui->HfixASCVPcomboBox->setVisible(a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't");
        ui->HfixASCVPcomboBox->clear();
        typefix.clear();
        if (a == "H't" || a == "h't")
        {
            typefix << "-";
            typefix << tr("OD fixateur");
            typefix << tr("OG fixateur");
            typefix << tr("alternant");
            ui->HfixASCVPcomboBox->insertItems(0,typefix);
        }
        if (a == "H'H't" || a == "h'h't")
        {
            typefix << "-";
            typefix << tr("rest. lente");
            typefix << tr("rest. moyenne");
            typefix << tr("rest. rapide");
            ui->HfixASCVPcomboBox->insertItems(0,typefix);
        }
        a = affichBOquery.value(59).toString();
        if (a == "") a = "-";
        ui->HEcranVPASCDcomboBox->setCurrentText(a);
        ui->HfixASCVPcomboBox->setCurrentText(affichBOquery.value(60).toString());

        // MADDOX -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        ui->MaddoxVLSCcomboBox->setCurrentText(affichBOquery.value(19).toString());
        a = ui->MaddoxVLSCcomboBox->currentText();
        ui->MaddoxVLSCDcomboBox->setVisible(a == "E" || a == "Et" || a == "EEt" || a == "X" || a == "Xt" || a == "XXt");
        ui->MaddoxVLSCDcomboBox->clear();
        int w = 50;
        if (a == "Et" || a == "EEt" || a == "Xt" || a == "XXt")
        {
            ui->MaddoxVLSCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVLSCDcomboBox->insertItems(0,Dioptrieslist);
        ui->MaddoxVLSCDcomboBox->resize(w,22);
        a = affichBOquery.value(20).toString();
        if (a == "") a = "-";
        ui->MaddoxVLSCDcomboBox->setCurrentText(a);
        ui->MaddoxVPSCcomboBox->setCurrentText(affichBOquery.value(21).toString());
        a = ui->MaddoxVPSCcomboBox->currentText();
        ui->MaddoxVPSCDcomboBox->setVisible(a == "E'" || a == "E't" || a == "E'E't" || a == "X'" || a == "X't" || a == "X'X't");
        ui->MaddoxVPSCDcomboBox->clear();
        w = 50;
        if (a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't")
        {
            ui->MaddoxVPSCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVPSCDcomboBox->insertItems(0,Dioptrieslist);
        ui->MaddoxVPSCDcomboBox->resize(w,22);
        a = affichBOquery.value(22).toString();
        if (a == "") a = "-";
        ui->MaddoxVPSCDcomboBox->setCurrentText(a);
        ui->MaddoxVLASCcomboBox->setCurrentText(affichBOquery.value(23).toString());
        a = ui->MaddoxVLASCcomboBox->currentText();
        ui->MaddoxVLASCDcomboBox->setVisible(a == "E" || a == "Et" || a == "EEt" || a == "X" || a == "Xt" || a == "XXt");
        ui->MaddoxVLASCDcomboBox->clear();
        w = 50;
        if (a == "Et" || a == "EEt" || a == "Xt" || a == "XXt")
        {
            ui->MaddoxVLASCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVLASCDcomboBox->insertItems(0,Dioptrieslist);
        ui->MaddoxVLASCDcomboBox->resize(w,22);
        a = affichBOquery.value(24).toString();
        if (a == "") a = "-";
        ui->MaddoxVLASCDcomboBox->setCurrentText(a);
        ui->MaddoxVPASCcomboBox->setCurrentText(affichBOquery.value(25).toString());
        a = ui->MaddoxVPASCcomboBox->currentText();
        ui->MaddoxVPASCDcomboBox->setVisible(a == "E'" || a == "E't" || a == "E'E't" || a == "X'" || a == "X't" || a == "X'X't");
        ui->MaddoxVPASCDcomboBox->clear();
        w = 50;
        if (a == "E't" || a == "E'E't" || a == "X't" || a == "X'X't")
        {
            ui->MaddoxVPASCDcomboBox->insertItems(0,DioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->MaddoxVPASCDcomboBox->insertItems(0,Dioptrieslist);
        ui->MaddoxVPASCDcomboBox->resize(w,22);
        a = affichBOquery.value(26).toString();
        if (a == "") a = "-";
        ui->MaddoxVPASCDcomboBox->setCurrentText(a);

        // HMADDOX -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        ui->HMaddoxVLSCcomboBox->setCurrentText(affichBOquery.value(61).toString());
        a = ui->HMaddoxVLSCcomboBox->currentText();
        ui->HMaddoxVLSCDcomboBox->setVisible(a == "H" || a == "Ht" || a == "HHt" || a == "h" || a == "ht" || a == "hht");
        ui->HMaddoxVLSCDcomboBox->clear();
        w = 50;
        if (a == "Ht" || a == "HHt" || a == "ht" || a == "hht")
        {
            ui->HMaddoxVLSCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVLSCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVLSCDcomboBox->resize(w,22);
        a = affichBOquery.value(62).toString();
        if (a == "") a = "-";
        ui->HMaddoxVLSCDcomboBox->setCurrentText(a);
        ui->HMaddoxVPSCcomboBox->setCurrentText(affichBOquery.value(63).toString());
        a = ui->HMaddoxVPSCcomboBox->currentText();
        ui->HMaddoxVPSCDcomboBox->setVisible(a == "H'" || a == "H't" || a == "H'H't" || a == "h'" || a == "h't" || a == "h'h't");
        ui->HMaddoxVPSCDcomboBox->clear();
        w = 50;
        if (a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't")
        {
            ui->HMaddoxVPSCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVPSCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVPSCDcomboBox->resize(w,22);
        a = affichBOquery.value(64).toString();
        if (a == "") a = "-";
        ui->HMaddoxVPSCDcomboBox->setCurrentText(a);
        ui->HMaddoxVLASCcomboBox->setCurrentText(affichBOquery.value(65).toString());
        a = ui->HMaddoxVLASCcomboBox->currentText();
        ui->HMaddoxVLASCDcomboBox->setVisible(a == "H" || a == "Ht" || a == "HHt" || a == "h" || a == "ht" || a == "hht");
        ui->HMaddoxVLASCDcomboBox->clear();
        w = 50;
        if (a == "Ht" || a == "HHt" || a == "ht" || a == "hht")
        {
            ui->HMaddoxVLASCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVLASCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVLASCDcomboBox->resize(w,22);
        a = affichBOquery.value(66).toString();
        if (a == "") a = "-";
        ui->HMaddoxVLASCDcomboBox->setCurrentText(a);
        ui->HMaddoxVPASCcomboBox->setCurrentText(affichBOquery.value(67).toString());
        a = ui->HMaddoxVPASCcomboBox->currentText();
        ui->HMaddoxVPASCDcomboBox->setVisible(a == "H'" || a == "H't" || a == "H'H't" || a == "h'" || a == "h't" || a == "h'h't");
        ui->HMaddoxVPASCDcomboBox->clear();
        w = 50;
        if (a == "H't" || a == "H'H't" || a == "h't" || a == "h'h't")
        {
            ui->HMaddoxVPASCDcomboBox->insertItems(0,HDioptriesNeutraliselist);
            w = 115;
        }
        else
            ui->HMaddoxVPASCDcomboBox->insertItems(0,HDioptrieslist);
        ui->HMaddoxVPASCDcomboBox->resize(w,22);
        a = affichBOquery.value(68).toString();
        if (a == "") a = "-";
        ui->HMaddoxVPASCDcomboBox->setCurrentText(a);

        ui->MotilitetextEdit->setText(affichBOquery.value(69).toString());
        a = affichBOquery.value(28).toString();
        if (a == "") a = "-";
        ui->PPCcomboBox->setCurrentText(a);
        ui->PPClineEdit->setText(affichBOquery.value(29).toString());
        ui->SaccadeslineEdit->setText(affichBOquery.value(30).toString());
        ui->PoursuitelineEdit->setText(affichBOquery.value(31).toString());
        ui->Worth1lineEdit->setText(affichBOquery.value(32).toString());
        ui->Worth2lineEdit->setText(affichBOquery.value(33).toString());
        ui->Bagolini1lineEdit->setText(affichBOquery.value(34).toString());
        ui->Bagolini2lineEdit->setText(affichBOquery.value(35).toString());
        a = affichBOquery.value(36).toString();
        if (a == "") a = "-";
        ui->VergenceDLcomboBox->setCurrentText(a);
        a = affichBOquery.value(37).toString();
        if (a == "") a = "-";
        ui->VergenceCLcomboBox->setCurrentText(a);
        a = affichBOquery.value(47).toString();
        if (a == "") a = "-";
        ui->VergenceRestDLcomboBox->setCurrentText(a);
        a = affichBOquery.value(38).toString();
        if (a == "") a = "-";
        ui->VergenceDPcomboBox->setCurrentText(a);
        a = affichBOquery.value(39).toString();
        if (a == "") a = "-";
        ui->VergenceCPcomboBox->setCurrentText(a);
        a = affichBOquery.value(48).toString();
        if (a == "") a = "-";
        ui->VergenceRestDPcomboBox->setCurrentText(a);
        ui->Degre1lineEdit->setText(affichBOquery.value(40).toString());
        ui->Degre2lineEdit->setText(affichBOquery.value(41).toString());
        ui->Degre3lineEdit->setText(affichBOquery.value(42).toString());
        ui->ConclusiontextEdit->setText(affichBOquery.value(43).toString());
    }
}

QString dlg_bilanortho::calcReponsehTml()
{
    QString Reponse =        "<html><head><meta name=\"qrichtext\" content=\"1\" />"
                             "<style type=\"text/css\">"
                             "p {margin-top:0px; margin-bottom:0px;margin-left: 0px}, li { white-space: pre-wrap; }"
                             "</style>"
                             "</head><body>";
    QString a;

    //Acuité visuelle ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString AVOD = ui->AVODlineEdit->text();
    QString AVOG = ui->AVOGlineEdit->text();
    if (AVOD + AVOG != "")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Acuité visuelle") + "</font></td>";
        if (AVOD != "" && AVOG == "")
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("OD") + "</font></td><td width=\"300\">" + AVOD + "</td></p>";
        else if (AVOD == "" && AVOG !="")
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("OG") + "</font></td><td width=\"300\">" + AVOG + "</td></p>";
        else if (AVOD != "" && AVOG !="")
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("OD") + "<br>" + tr("OG") + "</font></td><td width=\"300\">" + AVOD +"<br>" + AVOG + "</td></p>";
    }

    // l'occlusion alternée ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString OcclAltern = ui->OcclAlterncomboBox->currentText();
    if (OcclAltern != "-")
        Reponse += "<p><font color = \"" + CouleurTitres + "\">" + tr("Occlusion alternée ") + "</font>" + OcclAltern + "</p>";

    // la vision stereoscopique - Wirt, Lang, TNO  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString Wirt = ui->WirtcomboBox->currentText();
    QString Lang = ui->LangcomboBox->currentText();
    QString TNO  = ui->TNOcomboBox->currentText();
    if (Wirt.length() > 4)
        Wirt = "<td width=\"30\"><font color = \"" + CouleurTitres + "\">Wirt </font></td><td width=\"100\">" + Wirt + "</td>";
    else if (Wirt != "-")
    {
        Wirt = "<td width=\"30\"><font color = \"" + CouleurTitres + "\">Wirt</font></td><td width=\"45\">pion " + Wirt + "</td>";
        if (ui->AnimauxWirtcomboBox->isEnabled() && ui->AnimauxWirtcomboBox->currentText() != "-")
            Wirt += "<td width=\"90\"><font color = \"" + CouleurTitres + "\">" + tr("animaux ") + "</font>" + ui->AnimauxWirtcomboBox->currentText() + "</td>";

    }
    if (Lang != "-")
        Lang = "<td width=\"110\"><font color = \"" + CouleurTitres + "\">Lang </font>" + Lang + "</td>";
    if (TNO != "-")
        TNO = "<td width=\"110\"><font color = \"" + CouleurTitres + "\">TNO </font>" + TNO + "</td>";

    if (Wirt != "-" || Lang != "-" || TNO != "-")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Vision stereo") + "</font></td>";
        if (Wirt != "-")
            Reponse += Wirt;
        if (Lang != "-")
            Reponse += Lang;
        if (TNO != "-")
            Reponse += TNO;
        Reponse += "</p>";
    }

    //oeil directeur et orientation ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString ODirecteur, Orientation;
    ODirecteur = "-";
    Orientation = "-";
    if (ui->ODOrientationradioButton->isChecked())
        Orientation = "Droit";
    else if (ui->OGOrientationradioButton->isChecked())
        Orientation = "Gauche";
    if (ui->ODdirecteurradioButton->isChecked())
        ODirecteur = "Droit";
    else if (ui->OGdirecteurradioButton->isChecked())
        ODirecteur = "Gauche";
    if (ODirecteur != "-" || Orientation != "-")
    {
        Reponse += "<p>";
        if (ODirecteur != "-")
            Reponse += "<td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Oeil directeur ") + "</font></td><td width=\"80\">" + ODirecteur + "</td>";
        if (Orientation != "-")
            Reponse += "<td width=\"90\"><font color = \"" + CouleurTitres + "\">" + tr("Orientation ") + "</font></td><td width=\"80\">" + Orientation + "</td>";
        Reponse += "<p>";
    }

    //ECRAN ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString EcranSCVL, EcranSCVLD, EcranfixresSCVL, EcranSCVP, EcranSCVPD, EcranfixresSCVP;
    QString EcranASCVL, EcranASCVLD, EcranfixresASCVL, EcranASCVP, EcranASCVPD, EcranfixresASCVP;
    EcranSCVL               = ui->EcranVLSCcomboBox->currentText();
    EcranSCVLD              = ui->EcranVLSCDcomboBox->currentText();
    EcranfixresSCVL         = ui->fixSCVLcomboBox->currentText();
    EcranSCVP               = ui->EcranVPSCcomboBox->currentText();
    EcranSCVPD              = ui->EcranVPSCDcomboBox->currentText();
    EcranfixresSCVP         = ui->fixSCVPcomboBox->currentText();
    EcranASCVL              = ui->EcranVLASCcomboBox->currentText();
    EcranASCVLD             = ui->EcranVLASCDcomboBox->currentText();
    EcranfixresASCVL        = ui->fixASCVLcomboBox->currentText();
    EcranASCVP              = ui->EcranVPASCcomboBox->currentText();
    EcranASCVPD             = ui->EcranVPASCDcomboBox->currentText();
    EcranfixresASCVP        = ui->fixASCVPcomboBox->currentText();

    EcranfixresSCVL.replace("rest.",tr("restitution"));
    EcranfixresSCVP.replace("rest.",tr("restitution"));
    EcranfixresASCVL.replace("rest.",tr("restitution"));
    EcranfixresASCVP.replace("rest.",tr("restitution"));

    if (EcranSCVL  != "-" && EcranSCVL  != "O" && EcranSCVLD   != "-")  EcranSCVL   += " " + EcranSCVLD;
    if (EcranSCVP  != "-" && EcranSCVP  != "O" && EcranSCVPD   != "-")  EcranSCVP   += " " + EcranSCVPD;
    if (EcranASCVL != "-" && EcranASCVL != "O" && EcranASCVLD  != "-")  EcranASCVL  += " " + EcranASCVLD;
    if (EcranASCVP != "-" && EcranASCVP != "O" && EcranASCVPD  != "-")  EcranASCVP  += " " + EcranASCVPD;

    if (EcranSCVL == "-")                           EcranSCVL   = "";
    if (EcranSCVP == "-")                           EcranSCVP   = "";
    QString EcranSC = EcranSCVL;
    if (EcranSC     != "")                          EcranSC     += " ";
    EcranSC += EcranSCVP;

    if (EcranASCVL  == "-")                         EcranASCVL  = "";
    if (EcranASCVP  == "-")                         EcranASCVP  = "";
    QString EcranASC = EcranASCVL;
    if (EcranASC    != "")                          EcranASC    += " ";
    EcranASC += EcranASCVP;

    if (EcranSC + EcranASC != "")
    {
        QString longtabfixresVL = "190";
        QString longtabfixresVP = "190";
        QString longtabfixresAVL = "190";
        QString longtabfixresAVP = "190";
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Ecran horiz.") + "</font></td>";
        if (EcranSC  != "" && EcranASC == "")
        {
            a = ui->EcranVLSCcomboBox->currentText();
            if ((a != "Et" && a != "EEt" && a != "Xt" && a != "XXt") || EcranfixresSCVL == "-")
            {
                EcranfixresSCVL = "";
                longtabfixresVL = "55";
            }
            else
                EcranfixresSCVL = " - " + EcranfixresSCVL;
            a = ui->EcranVPSCcomboBox->currentText();
            if ((a != "E't" && a != "E'E't" && a != "X't" && a != "X'X't") || EcranfixresSCVP == "-")
            {
                EcranfixresSCVP = "";
                longtabfixresVP = "55";
            }
            else
                EcranfixresSCVP = " - " + EcranfixresSCVP;
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">SC</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">VL</font></td>";
            Reponse += "<td width=\"" + longtabfixresVL + "\">" + EcranSCVL + EcranfixresSCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">VP</font></td>";
            Reponse += "<td width=\"" + longtabfixresVL + "\">" + EcranSCVP + EcranfixresSCVP + "</td></p>";
        }
        else if (EcranSC == "" && EcranASC != "")
        {
            a = ui->EcranVLASCcomboBox->currentText();
            if ((a != "Et" && a != "EEt" && a != "Xt" && a != "XXt") || EcranfixresASCVL == "-")
            {
                EcranfixresASCVL = "";
                longtabfixresAVL = "55";
            }
            else
                EcranfixresASCVL = " - " + EcranfixresASCVL;
            a = ui->EcranVPASCcomboBox->currentText();
            if ((a != "E't" && a != "E'E't" && a != "X't" && a != "X'X't") || EcranfixresASCVP == "-")
            {
                EcranfixresASCVP = "";
                longtabfixresAVP = "55";
            }
            else
                EcranfixresASCVP = " - " + EcranfixresASCVP;
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("ASC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresAVL + "\">" + EcranASCVL + EcranfixresASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresAVP + "\">" + EcranASCVP + EcranfixresASCVP + "</td></p>";
        }
        else if (EcranSC != "" && EcranASC != "")
        {
            a = ui->EcranVLSCcomboBox->currentText();
            if ((a != "Et" && a != "EEt" && a != "Xt" && a != "XXt") || EcranfixresSCVL == "-")
            {
                EcranfixresSCVL = "";
                longtabfixresVL = "55";
            }
            else
                EcranfixresSCVL = " - " + EcranfixresSCVL;
            a = ui->EcranVPSCcomboBox->currentText();
            if ((a != "E't" && a != "E'E't" && a != "X't" && a != "X'X't") || EcranfixresSCVP == "-")
            {
                EcranfixresSCVP = "";
                longtabfixresVP = "55";
            }
            else
                EcranfixresSCVP = " - " + EcranfixresSCVP;
            a = ui->EcranVLASCcomboBox->currentText();
            if ((a != "Et" && a != "EEt" && a != "Xt" && a != "XXt") || EcranfixresASCVL == "-")
                EcranfixresASCVL = "";
            else
            {
                EcranfixresASCVL = " - " + EcranfixresASCVL;
                if (longtabfixresVL == "55") longtabfixresVL = "190";
            }
            a = ui->EcranVPASCcomboBox->currentText();
            if ((a != "E't" && a != "E'E't" && a != "X't" && a != "X'X't") || EcranfixresASCVP == "-")
                EcranfixresASCVP = "";
            else
            {
                EcranfixresASCVP = " - " + EcranfixresASCVP;
                if (longtabfixresVP == "55") longtabfixresVL = "190";
            }
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "<br>ASC</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "<br>VL</font></td>";
            Reponse += "<td width=\"" + longtabfixresVL + "\">" + EcranSCVL + EcranfixresSCVL + "<br>" + EcranASCVL + EcranfixresASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "<br>VP</font></td>";
            Reponse += "<td width=\"" + longtabfixresVP + "\">" + EcranSCVP + EcranfixresSCVP + "<br>" + EcranASCVP + EcranfixresASCVP + "</td></p>";
        }
    }

    //HECRAN ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString HEcranSCVL, HEcranSCVLD, HEcranfixresSCVL, HEcranSCVP, HEcranSCVPD, HEcranfixresSCVP;
    QString HEcranASCVL, HEcranASCVLD, HEcranfixresASCVL, HEcranASCVP, HEcranASCVPD, HEcranfixresASCVP;
    HEcranSCVL               = ui->HEcranVLSCcomboBox->currentText();
    HEcranSCVLD              = ui->HEcranVLSCDcomboBox->currentText();
    HEcranfixresSCVL         = ui->HfixSCVLcomboBox->currentText();
    HEcranSCVP               = ui->HEcranVPSCcomboBox->currentText();
    HEcranSCVPD              = ui->HEcranVPSCDcomboBox->currentText();
    HEcranfixresSCVP         = ui->HfixSCVPcomboBox->currentText();
    HEcranASCVL              = ui->HEcranVLASCcomboBox->currentText();
    HEcranASCVLD             = ui->HEcranVLASCDcomboBox->currentText();
    HEcranfixresASCVL        = ui->HfixASCVLcomboBox->currentText();
    HEcranASCVP              = ui->HEcranVPASCcomboBox->currentText();
    HEcranASCVPD             = ui->HEcranVPASCDcomboBox->currentText();
    HEcranfixresASCVP        = ui->HfixASCVPcomboBox->currentText();

    HEcranSCVL. replace("h",tr("hypo"));
    HEcranASCVL.replace("h",tr("hypo"));
    HEcranSCVP. replace("h",tr("hypo"));
    HEcranASCVP.replace("h",tr("hypo"));

    if (HEcranSCVL  != "-" && HEcranSCVL  != "O" && HEcranSCVLD   != "-")  HEcranSCVL   += " " + HEcranSCVLD;
    if (HEcranSCVP  != "-" && HEcranSCVP  != "O" && HEcranSCVPD   != "-")  HEcranSCVP   += " " + HEcranSCVPD;
    if (HEcranASCVL != "-" && HEcranASCVL != "O" && HEcranASCVLD  != "-")  HEcranASCVL  += " " + HEcranASCVLD;
    if (HEcranASCVP != "-" && HEcranASCVP != "O" && HEcranASCVPD  != "-")  HEcranASCVP  += " " + HEcranASCVPD;

    if (HEcranSCVL == "-")                           HEcranSCVL   = "";
    if (HEcranSCVP == "-")                           HEcranSCVP   = "";
    QString HEcranSC = HEcranSCVL;
    if (HEcranSC     != "")                          HEcranSC     += " ";
    HEcranSC += HEcranSCVP;

    if (HEcranASCVL  == "-")                         HEcranASCVL  = "";
    if (HEcranASCVP  == "-")                         HEcranASCVP  = "";
    QString HEcranASC = HEcranASCVL;
    if (HEcranASC    != "")                          HEcranASC    += " ";
    HEcranASC += HEcranASCVP;

    if (HEcranSC + HEcranASC != "")
    {
        QString longtabfixresVL = "190";
        QString longtabfixresVP = "190";
        QString longtabfixresAVL = "190";
        QString longtabfixresAVP = "190";
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Ecran vertic.") + "</font></td>";
        if (HEcranSC  != "" && HEcranASC == "")
        {
            a = ui->HEcranVLSCcomboBox->currentText();
            if ((a != "Ht" && a != "HHt" && a != "ht" && a != "hht") || HEcranfixresSCVL == "-")
            {
                HEcranfixresSCVL = "";
                longtabfixresVL = "55";
            }
            else
                HEcranfixresSCVL = " - " + HEcranfixresSCVL;
            a = ui->HEcranVPSCcomboBox->currentText();
            if ((a != "H't" && a != "H'H't" && a != "h't" && a != "h'h't") || HEcranfixresSCVP == "-")
            {
                HEcranfixresSCVP = "";
                longtabfixresVP = "55";
            }
            else
                HEcranfixresSCVP = " - " + HEcranfixresSCVP;
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresVL + "\">" + HEcranSCVL + HEcranfixresSCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresVL + "\">" + HEcranSCVP + HEcranfixresSCVP + "</td></p>";
        }
        else if (HEcranSC == "" && HEcranASC != "")
        {
            a = ui->HEcranVLASCcomboBox->currentText();
            if ((a != "Ht" && a != "HHt" && a != "ht" && a != "hht") || HEcranfixresASCVL == "-")
            {
                HEcranfixresASCVL = "";
                longtabfixresAVL = "55";
            }
            else
                HEcranfixresASCVL = " - " + HEcranfixresASCVL;
            a = ui->HEcranVPASCcomboBox->currentText();
            if ((a != "H't" && a != "H'H't" && a != "h't" && a != "h'h't") || HEcranfixresASCVP == "-")
            {
                HEcranfixresASCVP = "";
                longtabfixresAVP = "55";
            }
            else
                HEcranfixresASCVP = " - " + HEcranfixresASCVP;
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("ASC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresAVL + "\">" + HEcranASCVL + HEcranfixresASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresAVP + "\">" + HEcranASCVP + HEcranfixresASCVP + "</td></p>";
        }
        else if (HEcranSC != "" && HEcranASC != "")
        {
            a = ui->HEcranVLSCcomboBox->currentText();
            if ((a != "Ht" && a != "HHt" && a != "ht" && a != "hht") || HEcranfixresSCVL == "-")
            {
                HEcranfixresSCVL = "";
                longtabfixresVL = "55";
            }
            else
                HEcranfixresSCVL = " - " + HEcranfixresSCVL;
            a = ui->HEcranVPSCcomboBox->currentText();
            if ((a != "H't" && a != "H'H't" && a != "h't" && a != "h'h't") || HEcranfixresSCVP == "-")
            {
                HEcranfixresSCVP = "";
                longtabfixresVP = "55";
            }
            else
                HEcranfixresSCVP = " - " + HEcranfixresSCVP;
            a = ui->HEcranVLASCcomboBox->currentText();
            if ((a != "Ht" && a != "HHt" && a != "ht" && a != "hht") || HEcranfixresASCVL == "-")
                HEcranfixresASCVL = "";
            else
            {
                HEcranfixresASCVL = " - " + HEcranfixresASCVL;
                if (longtabfixresVL == "55") longtabfixresVL = "190";
            }
            a = ui->HEcranVPASCcomboBox->currentText();
            if ((a != "H't" && a != "H'H't" && a != "h't" && a != "h'h't") || HEcranfixresASCVP == "-")
                HEcranfixresASCVP = "";
            else
            {
                HEcranfixresASCVP = " - " + HEcranfixresASCVP;
                if (longtabfixresVP == "55") longtabfixresVL = "190";
            }
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "<br>ASC</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "<br>VL</font></td>";
            Reponse += "<td width=\"" + longtabfixresVL + "\">" + HEcranSCVL + HEcranfixresSCVL + "<br>" + HEcranASCVL + HEcranfixresASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "<br>" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + longtabfixresVP + "\">" + HEcranSCVP + HEcranfixresSCVP + "<br>" + HEcranASCVP + HEcranfixresASCVP + "</td></p>";
        }
    }

    //Maddox ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString MaddoxSCVL, MaddoxSCVLD, MaddoxSCVP, MaddoxSCVPD;
    QString MaddoxASCVL, MaddoxASCVLD, MaddoxASCVP, MaddoxASCVPD;
    MaddoxSCVL       = ui->MaddoxVLSCcomboBox->currentText();
    MaddoxSCVLD      = ui->MaddoxVLSCDcomboBox->currentText();
    MaddoxSCVP       = ui->MaddoxVPSCcomboBox->currentText();
    MaddoxSCVPD      = ui->MaddoxVPSCDcomboBox->currentText();
    MaddoxASCVL      = ui->MaddoxVLASCcomboBox->currentText();
    MaddoxASCVLD     = ui->MaddoxVLASCDcomboBox->currentText();
    MaddoxASCVP      = ui->MaddoxVPASCcomboBox->currentText();
    MaddoxASCVPD     = ui->MaddoxVPASCDcomboBox->currentText();
    if (MaddoxSCVL   != "-" && MaddoxSCVL  != "O" && MaddoxSCVLD  != "-")    MaddoxSCVL   += " " + MaddoxSCVLD;
    if (MaddoxSCVP   != "-" && MaddoxSCVP  != "O" && MaddoxSCVPD  != "-")    MaddoxSCVP   += " " + MaddoxSCVPD;
    if (MaddoxASCVL  != "-" && MaddoxASCVL != "O" && MaddoxASCVLD != "-")    MaddoxASCVL  += " " + MaddoxASCVLD;
    if (MaddoxASCVP  != "-" && MaddoxASCVP != "O" && MaddoxASCVPD != "-")    MaddoxASCVP  += " " + MaddoxASCVPD;

    if (MaddoxSCVL == "-")                           MaddoxSCVL   = "";
    if (MaddoxSCVP == "-")                           MaddoxSCVP   = "";
    QString MaddoxSC = MaddoxSCVL;
    if (MaddoxSC     != "")                          MaddoxSC     += " ";
    MaddoxSC += MaddoxSCVP;

    if (MaddoxASCVL  == "-")                         MaddoxASCVL  = "";
    if (MaddoxASCVP  == "-")                         MaddoxASCVP  = "";
    QString MaddoxASC = MaddoxASCVL;
    if (MaddoxASC    != "")                          MaddoxASC    += " ";
    MaddoxASC += MaddoxASCVP;
    QString lengMaddoxSCVL = "190";
    QString lengMaddoxSCVP = "190";
    QString lengMaddoxASCVL = "190";
    QString lengMaddoxASCVP = "190";

    if (MaddoxSC + MaddoxASC != "")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Maddox horiz.</font></td>";
        if (MaddoxSC  != "" && MaddoxASC == "")
        {
            if (MaddoxSCVLD.length()<4) lengMaddoxSCVL = "55";
            if (MaddoxSCVPD.length()<4) lengMaddoxSCVP = "55";
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + lengMaddoxSCVL + "\">" + MaddoxSCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengMaddoxSCVP + "\">" + MaddoxSCVP + "</td></p>";
        }
        else if (MaddoxSC == "" && MaddoxASC != "")
        {
            if (MaddoxASCVLD.length()<4) lengMaddoxASCVL = "55";
            if (MaddoxASCVPD.length()<4) lengMaddoxASCVP = "55";
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("ASC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + lengMaddoxASCVL + "\">" + MaddoxASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengMaddoxASCVP + "\">" + MaddoxASCVP + "</td></p>";
        }
        else if (MaddoxSC  != "" && MaddoxASC != "")
        {
            if (MaddoxSCVLD.length()<4) lengMaddoxSCVL = "55";
            if (MaddoxSCVPD.length()<4) lengMaddoxSCVP = "55";
            if (MaddoxASCVLD.length()>4) lengMaddoxSCVL = "190";
            if (MaddoxASCVPD.length()>4) lengMaddoxSCVP = "190";
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "<br>" + tr("ASC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "<br>VP</font></td>";
            Reponse += "<td width=\"" + lengMaddoxSCVL + "\">" + MaddoxSCVL + "<br>" + MaddoxASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "<br>" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengMaddoxSCVP + "\">" + MaddoxSCVP + "<br>" + MaddoxASCVP + "</td></p>";
        }
    }

    //HMaddox ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString HMaddoxSCVL, HMaddoxSCVLD, HMaddoxSCVP, HMaddoxSCVPD;
    QString HMaddoxASCVL, HMaddoxASCVLD, HMaddoxASCVP, HMaddoxASCVPD;
    HMaddoxSCVL       = ui->HMaddoxVLSCcomboBox->currentText();
    HMaddoxSCVLD      = ui->HMaddoxVLSCDcomboBox->currentText();
    HMaddoxSCVP       = ui->HMaddoxVPSCcomboBox->currentText();
    HMaddoxSCVPD      = ui->HMaddoxVPSCDcomboBox->currentText();
    HMaddoxASCVL      = ui->HMaddoxVLASCcomboBox->currentText();
    HMaddoxASCVLD     = ui->HMaddoxVLASCDcomboBox->currentText();
    HMaddoxASCVP      = ui->HMaddoxVPASCcomboBox->currentText();
    HMaddoxASCVPD     = ui->HMaddoxVPASCDcomboBox->currentText();

    HMaddoxSCVL .replace("h",   tr("hypo"));
    HMaddoxASCVL.replace("h",   tr("hypo"));
    HMaddoxSCVP .replace("h",   tr("hypo"));
    HMaddoxASCVP.replace("h",   tr("hypo"));

    if (HMaddoxSCVL   != "-" && HMaddoxSCVL  != "O" && HMaddoxSCVLD  != "-")    HMaddoxSCVL   += " " + HMaddoxSCVLD;
    if (HMaddoxSCVP   != "-" && HMaddoxSCVP  != "O" && HMaddoxSCVPD  != "-")    HMaddoxSCVP   += " " + HMaddoxSCVPD;
    if (HMaddoxASCVL  != "-" && HMaddoxASCVL != "O" && HMaddoxASCVLD != "-")    HMaddoxASCVL  += " " + HMaddoxASCVLD;
    if (HMaddoxASCVP  != "-" && HMaddoxASCVP != "O" && HMaddoxASCVPD != "-")    HMaddoxASCVP  += " " + HMaddoxASCVPD;

    if (HMaddoxSCVL == "-")                           HMaddoxSCVL   = "";
    if (HMaddoxSCVP == "-")                           HMaddoxSCVP   = "";
    QString HMaddoxSC = HMaddoxSCVL;
    if (HMaddoxSC     != "")                          HMaddoxSC     += " ";
    HMaddoxSC += HMaddoxSCVP;

    if (HMaddoxASCVL  == "-")                         HMaddoxASCVL  = "";
    if (HMaddoxASCVP  == "-")                         HMaddoxASCVP  = "";
    QString HMaddoxASC = HMaddoxASCVL;
    if (HMaddoxASC    != "")                          HMaddoxASC    += " ";
    HMaddoxASC += HMaddoxASCVP;
    QString lengHMaddoxSCVL = "190";
    QString lengHMaddoxSCVP = "190";
    QString lengHMaddoxASCVL = "190";
    QString lengHMaddoxASCVP = "190";

    if (HMaddoxSC + HMaddoxASC != "")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Maddox vertic.</font></td>";
        if (HMaddoxSC  != "" && HMaddoxASC == "")
        {
            if (HMaddoxSCVLD.length()<4) lengHMaddoxSCVL = "55";
            if (HMaddoxSCVPD.length()<4) lengHMaddoxSCVP = "55";
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + lengHMaddoxSCVL + "\">" + HMaddoxSCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengHMaddoxSCVP + "\">" + HMaddoxSCVP + "</td></p>";
        }
        else if (HMaddoxSC == "" && HMaddoxASC != "")
        {
            if (HMaddoxASCVLD.length()<4) lengHMaddoxASCVL = "55";
            if (HMaddoxASCVPD.length()<4) lengHMaddoxASCVP = "55";
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("ASC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"" + lengHMaddoxASCVL + "\">" + HMaddoxASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengHMaddoxASCVP + "\">" + HMaddoxASCVP + "</td></p>";
        }
        else if (HMaddoxSC  != "" && HMaddoxASC != "")
        {
            if (HMaddoxSCVLD.length()<4) lengHMaddoxSCVL = "55";
            if (HMaddoxSCVPD.length()<4) lengHMaddoxSCVP = "55";
            if (HMaddoxASCVLD.length()>4) lengHMaddoxSCVL = "190";
            if (HMaddoxASCVPD.length()>4) lengHMaddoxSCVP = "190";
            Reponse += "<td width=\"30\"><font color = \"" + CouleurTitres + "\">" + tr("SC") + "<br>" + tr("ASC") + "</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "<br>" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengHMaddoxSCVL + "\">" + HMaddoxSCVL + "<br>" + HMaddoxASCVL + "</td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "<br>" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"" + lengHMaddoxSCVP + "\">" + HMaddoxSCVP + "<br>" + HMaddoxASCVP + "</td></p>";
        }
    }

    //Motilité ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString Motilite = ui->MotilitetextEdit->toPlainText();
    if (Motilite != "")
    {
        //for (int i= 0; i<Motilite.size();i++)
        //if (Motilite.at(i).unicode() == 10) Motilite.replace(Motilite.at(i),"<br>");
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Motilité") + "</font></td>";
        Reponse += "<td width=\"300\">" + ui->MotilitetextEdit->toHtml() + "</td></p>";
    }

    //PPC ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString PPC = ui->PPCcomboBox->currentText();
    if (PPC != "-")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("PPC") + "</font></td>";
        Reponse += "<td width=\"300\">" + PPC + " " + ui->PPClineEdit->text() + "</td></p>";
    }

    //Saccades - Poursuite
    QString Saccades = ui->SaccadeslineEdit->text();
    QString Poursuite = ui->PoursuitelineEdit->text();
    if (Saccades != "")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Saccades") + "</font></td>";
        Reponse += "<td width=\"300\">" + Saccades + "</td></p>";
    }
    if (Poursuite != "")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">" + tr("Poursuite") + "</font></td>";
        Reponse += "<td width=\"300\">" + Poursuite + "</td></p>";
    }

    //Worth ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString Worth1 = ui->Worth1lineEdit->text();
    QString Worth2 = ui->Worth2lineEdit->text();
    if (Worth1 != "" && Worth2 != "")
    {
        if (Worth1 != "" && Worth2 == "")
        {
            Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Worth</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"300\">" + Worth1 + "</td>";
            Reponse += "</p>";
        }
        else if (Worth1 == "" && Worth2 != "")
        {
            Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Worth</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"300\">" + Worth2 + "</td>";
            Reponse += "</p>";
        }
        else
        {
            Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Worth</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "<br>" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"300\">" + Worth1 + "<br>" + Worth2 + "</td>";
            Reponse += "</p>";
        }
    }

    //Bagolini ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString Bagolini1 = ui->Bagolini1lineEdit->text();
    QString Bagolini2 = ui->Bagolini2lineEdit->text();
    if (Bagolini1 != "" && Bagolini2 != "")
    {
        if (Bagolini1 != "" && Bagolini2 == "")
        {
            Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Bagolini</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td>";
            Reponse += "<td width=\"300\">" + Bagolini1 + "</td>";
            Reponse += "</p>";
        }
        else if (Bagolini1 == "" && Bagolini2 != "")
        {
            Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Bagolini</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"300\">" + Bagolini2 + "</td>";
            Reponse += "</p>";
        }
        else
        {
            Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Bagolini</font></td>";
            Reponse += "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "<br>" + tr("VP") + "</font></td>";
            Reponse += "<td width=\"300\">" + Bagolini1 + "<br>" + Bagolini2 + "</td>";
            Reponse += "</p>";
        }
    }

    //Vergences ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString VergenceVL = "";
    if (ui->VergenceDLcomboBox->currentText() != "-")
        VergenceVL = ui->VergenceDLcomboBox->currentText() + " ";
    if (ui->VergenceCLcomboBox->currentText() != "-")
        VergenceVL += ui->VergenceCLcomboBox->currentText() + " ";
    if (ui->VergenceRestDLcomboBox->currentText() != "-")
        VergenceVL += ui->VergenceRestDLcomboBox->currentText();
    QString VergenceVP = "";
    if (ui->VergenceDPcomboBox->currentText() != "-")
        VergenceVP = ui->VergenceDPcomboBox->currentText() + " ";
    if (ui->VergenceCPcomboBox->currentText() != "-")
        VergenceVP += ui->VergenceCPcomboBox->currentText() + " ";
    if (ui->VergenceRestDPcomboBox->currentText() != "-")
        VergenceVP += ui->VergenceRestDPcomboBox->currentText();
    if (VergenceVL + VergenceVP !="")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Vergences</font></td>";
        if (VergenceVL  != "")  VergenceVL = "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VL") + "</font></td><td width=\"100\">" + VergenceVL + "</td>";
        if (VergenceVP != "")   VergenceVP = "<td width=\"17\"><font color = \"" + CouleurTitres + "\">" + tr("VP") + "</font></td><td width=\"100\">" + VergenceVP + "</td>";
        if (VergenceVL  != "")
        {
            Reponse += VergenceVL;
            if (VergenceVP != "")     Reponse += VergenceVP;
        }
        else
            Reponse += VergenceVP;
        Reponse += "</p>";
    }

    //Synoptophore ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString Synopt1 = ui->Degre1lineEdit->text();
    QString Synopt2 = ui->Degre2lineEdit->text();
    QString Synopt3 = ui->Degre3lineEdit->text();
    if (Synopt1 + Synopt2 + Synopt3 != "")
    {
        Reponse += "<p><td width=\"100\"><font color = \"" + CouleurTitres + "\">Synoptophore</font></td>";
        if (Synopt1 !="" && Synopt2 =="" && Synopt3 == "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("1er degré") + "</font></td><td width=\"350\">" + Synopt1 + "</td></p>";
        else if (Synopt1 =="" && Synopt2 !="" && Synopt3 == "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("2ème degré") + "</font></td><td width=\"350\">" + Synopt2 + "</td></p>";
        else if (Synopt1 =="" && Synopt2 =="" && Synopt3 != "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("3ème degré") + "</font></td><td width=\"350\">" + Synopt3 + "</td></p>";
        else if (Synopt1 !="" && Synopt2 !="" && Synopt3 == "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("1er degré") + "<br>" + tr("2ème degré") + "</font></td><td width=\"350\">" + Synopt1 + "<br>" + Synopt2 + "</td></p>";
        else if (Synopt1 !="" && Synopt2 =="" && Synopt3 != "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("1er degré") + "<br>" + tr("3ème degré") + "</font></td><td width=\"350\">" + Synopt1 + "<br>" + Synopt3 + "</td></p>";
        else if (Synopt1 =="" && Synopt2 !="" && Synopt3 != "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("2ème degré") + "<br>" + tr("3ème degré") + "</font></td><td width=\"350\">" + Synopt2 + "<br>" + Synopt3 + "</td></p>";
        else if (Synopt1 !="" && Synopt2 !="" && Synopt3 != "")
            Reponse += "<td width=\"80\"><font color = \"" + CouleurTitres + "\">" + tr("1er degré") + "<br>" + tr("2ème degré") + "<br>" + tr("3ème degré") + "</font></td><td width=\"350\">" + Synopt1 + "<br>" + Synopt2 + "<br>" + Synopt3 + "</td></p>";
    }
    return Reponse;
}

void dlg_bilanortho::setDateBO(QDate dateBO)
{
    DateBO = dateBO;
}

QDate dlg_bilanortho::getDateBO()
{
    return DateBO;
}
