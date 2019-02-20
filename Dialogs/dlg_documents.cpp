/* (C) 2018 LAINE SERGE
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

#include "dlg_documents.h"
#include "ui_dlg_documents.h"

dlg_documents::dlg_documents(int idPatAPasser, QString NomPatient, QString PrenomPatient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_documents)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    gidPatient          = idPatAPasser;
    gNomPat             = NomPatient;
    gPrenomPat          = PrenomPatient;

    proc                = Procedures::I();
    db                  = DataBase::getInstance();
    gidUser             = db->getUserConnected()->id();
    gidUserSuperviseur  = db->getUserConnected()->getIdUserActeSuperviseur();

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionDocuments").toByteArray());
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    setWindowTitle(tr("Liste des documents prédéfinis"));
    ui->PrescriptioncheckBox->setVisible(db->getUserConnected()->isSoignant());
    widgButtonsDocs     = new WidgetButtonFrame(ui->DocupTableWidget);
    widgButtonsDocs     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    widgButtonsDocs     ->layButtons()->insertWidget(0, ui->ChercheupLineEdit);
    widgButtonsDocs     ->layButtons()->insertWidget(0, ui->label);
    widgButtonsDossiers = new WidgetButtonFrame(ui->DossiersupTableWidget);
    widgButtonsDossiers ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    ui->upTextEdit->disconnect(); // pour déconnecter la fonction MenuContextuel intrinsèque de la classe UpTextEdit

    // Initialisation des slots.
    connect (ui->ChercheupLineEdit,             &QLineEdit::textEdited,                 this,   [=] {FiltreListe(ui->ChercheupLineEdit->text());});
    connect (ui->OKupPushButton,                &QPushButton::clicked,                  this,   &dlg_documents::Validation);
    connect (ui->AnnulupPushButton,             &QPushButton::clicked,                  this,   &dlg_documents::Annulation);
    connect (ui->DocPubliccheckBox,             &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocPubliccheckBox);});
    connect (ui->DocEditcheckBox,               &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocEditcheckBox);});
    connect (ui->DocAdministratifcheckBox,      &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocAdministratifcheckBox);});
    connect (ui->PrescriptioncheckBox,          &QPushButton::clicked,                  this,   [=] {CheckPublicEditablAdmin(ui->PrescriptioncheckBox);});
    connect (ui->upTextEdit,                    &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel(ui->upTextEdit);});
    connect (ui->upTextEdit,                    &QTextEdit::textChanged,                this,   [=] {EnableOKPushButton();});
    connect (ui->upTextEdit,                    &UpTextEdit::dblclick,                  this,   &dlg_documents::dblClicktextEdit);
    connect (ui->DupliOrdocheckBox,             &QCheckBox::clicked,                    this,   [=] {OrdoAvecDupli(ui->DupliOrdocheckBox->isChecked());});
    connect (widgButtonsDocs,                   &WidgetButtonFrame::choix,              this,   [=] {ChoixButtonFrame(widgButtonsDocs->Reponse(), widgButtonsDocs);});
    connect (widgButtonsDossiers,               &WidgetButtonFrame::choix,              this,   [=] {ChoixButtonFrame(widgButtonsDossiers->Reponse(), widgButtonsDossiers);});

    // Mise en forme de la table Documents
    ui->DocupTableWidget->setPalette(QPalette(Qt::white));
    ui->DocupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->DocupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DocupTableWidget->verticalHeader()->setVisible(false);
    ui->DocupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->DocupTableWidget->setColumnCount(13);
    ui->DocupTableWidget->setColumnWidth(0,30);      // checkbox
    ui->DocupTableWidget->setColumnWidth(1,289);     // Resume
    ui->DocupTableWidget->setColumnHidden(2,true);   // colonne texte masquee
    ui->DocupTableWidget->setColumnHidden(3,true);   // idDocument
    ui->DocupTableWidget->setColumnHidden(4,true);   // DocPublic
    ui->DocupTableWidget->setColumnHidden(5,true);   // idUserqui a créé le document
    ui->DocupTableWidget->setColumnHidden(6,true);   // Prescription
    ui->DocupTableWidget->setColumnWidth(7,30);      // Public   - affiche un check si document public
    ui->DocupTableWidget->setColumnHidden(8,true);   // Editable - masqué    ->     1 si Editable - vide sinon
    ui->DocupTableWidget->setColumnWidth(9,30);      // Editable - affiche un check si document editable
    ui->DocupTableWidget->setColumnHidden(10,true);  // Checked
    ui->DocupTableWidget->setColumnHidden(11,true);  // Medical  - masqué    ->     1 si Editable - vide sinon
    ui->DocupTableWidget->setColumnWidth(12,30);     // Medical  - affiche un check si document medical
    ui->DocupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Icons::icImprimer(),""));
    ui->DocupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("TITRES DES DOCUMENTS")));
    ui->DocupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(6, new QTableWidgetItem(""));
    QTableWidgetItem *docpublic, *editable,*admin;
    docpublic   = new QTableWidgetItem(Icons::icFamily(),"");
    docpublic   ->setToolTip(tr("Document disponible pour tous les utilisateurs"));
    editable    = new QTableWidgetItem(Icons::icEditer(),"");
    editable    ->setToolTip(tr("Document éditable au moment de l'impression"));
    admin       = new QTableWidgetItem(Icons::icAdministratif(),"");
    admin       ->setToolTip(tr("Document administratif"));
    ui->DocupTableWidget->setHorizontalHeaderItem(7, docpublic);
    ui->DocupTableWidget->setHorizontalHeaderItem(8, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(9, editable);
    ui->DocupTableWidget->setHorizontalHeaderItem(10,new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(11,new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(12,admin);
    ui->DocupTableWidget->horizontalHeader()->setVisible(true);
    ui->DocupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->DocupTableWidget->horizontalHeader()->setIconSize(QSize(25,25));
    ui->DocupTableWidget->setGridStyle(Qt::DotLine);
    ui->DocupTableWidget->FixLargeurTotale();

    // Mise en forme de la table Dossiers
    ui->DossiersupTableWidget->setPalette(QPalette(Qt::white));
    ui->DossiersupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->DossiersupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DossiersupTableWidget->verticalHeader()->setVisible(false);
    ui->DossiersupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->DossiersupTableWidget->setColumnCount(5);
    ui->DossiersupTableWidget->setColumnWidth(0,30);      // checkbox
    ui->DossiersupTableWidget->setColumnWidth(1,259);     // Resume
    ui->DossiersupTableWidget->setColumnHidden(2,true);   // idDocument
    ui->DossiersupTableWidget->setColumnHidden(3,true);   // Public
    ui->DossiersupTableWidget->setColumnWidth(4,30);
    ui->DossiersupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Icons::icImprimer(),""));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("DOSSIERS")));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(Icons::icFamily(),""));
    ui->DossiersupTableWidget->horizontalHeader()->setVisible(true);
    ui->DossiersupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->DossiersupTableWidget->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    ui->DossiersupTableWidget->horizontalHeader()->setIconSize(QSize(25,25));
    ui->DossiersupTableWidget->FixLargeurTotale();

    QHBoxLayout *doclay = new QHBoxLayout();
    doclay      ->addWidget(widgButtonsDossiers->widgButtonParent());
    doclay      ->addWidget(widgButtonsDocs->widgButtonParent());
    doclay      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding));
    int marge   = 0;
    int space   = 5;
    doclay      ->setContentsMargins(marge,marge,marge,marge);
    doclay      ->setSpacing(space);
    ui->widget  ->setLayout(doclay);

    ui->AnnulupPushButton->setUpButtonStyle(UpPushButton::ANNULBUTTON, UpPushButton::Mid);
    ui->OKupPushButton->setUpButtonStyle(UpPushButton::OKBUTTON, UpPushButton::Mid);

    ui->OKupPushButton->setIcon(Icons::icImprimer());
    ui->OKupPushButton->setIconSize(QSize(30,30));

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setMaximumDate(QDate::currentDate());

    ui->DupliOrdocheckBox->setChecked(proc->gsettingsIni->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES");
    ui->label->setPixmap(Icons::pxLoupe().scaled(30,30)); //WARNING : icon scaled : pxLoupe 20,20
    ui->ChercheupLineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 3px solid rgb(164, 205, 255);border-radius: 10px;}");


    ui->textFrame->installEventFilter(this);
    ui->DocupTableWidget->installEventFilter(this);
    ui->DossiersupTableWidget->installEventFilter(this);
    gOp             = new QGraphicsOpacityEffect();
    gTimerEfface    = new QTimer(this);

    bool ok;
    QString ALDrequete = "select idPat from " NOM_TABLE_DONNEESSOCIALESPATIENTS " where idpat = " + QString::number(gidPatient) + " and PatALD = 1";
    ui->ALDcheckBox->setChecked(db->StandardSelectSQL(ALDrequete,ok).size()>0);

    //nettoyage de la table metadocs
    db->StandardSQL("delete from " NOM_TABLE_JOINTURESDOCS " where iddocument not in (select iddocument from " NOM_TABLE_COURRIERS ")");

    Remplir_TableWidget();
    if (ui->DocupTableWidget->rowCount() == 0)  ConfigMode(CreationDOC);    else ConfigMode(Selection);
    ui->ChercheupLineEdit->setFocus();
    ui->lineEdit->setVisible(false);

    gChampsMap[TITRUSER]        = tr("Titre, nom et prénom de l'utilisateur");
    gChampsMap[NOMPAT]          = tr("Nom du patient");
    gChampsMap[DATEDOC]         = tr("Date du jour");
    gChampsMap[DDNPAT]          = tr("Date de naissance");
    gChampsMap[TITREPAT]        = tr("Titre du patient");
    gChampsMap[AGEPAT]          = tr("Âge du patient");
    gChampsMap[PRENOMPAT]       = tr("Prénom du patient");
    gChampsMap[MGPAT]           = tr("Médecin du patient");
    gChampsMap[POLITESSEMG]     = tr("formule de politesse médecin patient");
    gChampsMap[PRENOMMG]        = tr("Prénom du médecin");
    gChampsMap[NOMMG]           = tr("Nom du médecin");
    gChampsMap[REFRACT]         = tr("Refraction du patient'");
    gChampsMap[KERATO]          = tr("Keratométrie du patient");
    gChampsMap[CORPAT]          = tr("Correspondant du patient");
    gChampsMap[POLITESSECOR]    = tr("formule de politesse correspondant");
    gChampsMap[PRENOMCOR]       = tr("Prénom du correspondant");
    gChampsMap[NOMCOR]          = tr("Nom du correspondant");
}

dlg_documents::~dlg_documents()
{
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton ANNULER.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_documents::Annulation()
{
    if (gMode == CreationDOC || gMode == ModificationDOC || gMode == ModificationDOSS || gMode == CreationDOSS)
    {
        QString TableAmodifier = "";
        int     row = -1;
        UpLineEdit *line;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->focusPolicy() == Qt::WheelFocus)
            {
                row = line->getRowTable();
                TableAmodifier = "Docs";
                break;
            }
        }
        if (row == -1)
        {
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->focusPolicy() == Qt::WheelFocus)
                {
                    row = line->getRowTable();
                    TableAmodifier = "Dossiers";
                    break;
                }
            }
        }
        Remplir_TableWidget();
        if (ui->DocupTableWidget->rowCount() == 0)
            ConfigMode(CreationDOC);
        else
        {
            ConfigMode(Selection);
            if (TableAmodifier == "Docs")
                LineSelect(ui->DocupTableWidget,row);
            else if (TableAmodifier == "Dossiers")
                LineSelect(ui->DossiersupTableWidget, row);
        }
    }
    else
        reject();
}

void dlg_documents::ChoixButtonFrame(int j, WidgetButtonFrame *widgbutt)
{
    UpLineEdit *line = new UpLineEdit();
    int row = 0;
    if (widgbutt== widgButtonsDocs)
    {
        switch (j) {
        case 1:
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) {row = line->getRowTable(); break;}
            }
            ConfigMode(CreationDOC, row);
            break;
        case 0:
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            ConfigMode(ModificationDOC,line->getRowTable());
            break;
        case -1:
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            DisableLines();
            SupprimmDocument(line->getRowTable());
            break;
        default:
            break;
        }
    }
    else if (widgbutt== widgButtonsDossiers)
    {
        switch (j) {
        case 1:
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) {row = line->getRowTable(); break;}
            }
            ConfigMode(CreationDOSS, row);
            break;
        case 0:
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            ConfigMode(ModificationDOSS,line->getRowTable());
            break;
        case -1:
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            SupprimmDossier(line->getRowTable());
            break;
        default:
            break;
        }
    }
}

void dlg_documents::CheckPublicEditablAdmin(QCheckBox *check)
{
    UpLineEdit *line =new UpLineEdit(this);
    bool a = false;
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)  {
        line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line->isEnabled()) {a = true; break;}
    }
    if (!a)
        return;
    int row = line->getRowTable();

    int b(-1), c(-1);
    if (check == ui->DocPubliccheckBox) {
        b = 4;
        c = 7;
    }
    else if (check == ui->DocEditcheckBox)  {
        b = 8;
        c = 9;
    }
    else if (check == ui->DocAdministratifcheckBox)  {
        b = 11;
        c = 12;
    }
    else if (check == ui->PrescriptioncheckBox)
    {
        if (check->isChecked() && ui->DocAdministratifcheckBox->isChecked())
        {
            ui->DocAdministratifcheckBox->setChecked(false);
            b = 11;
            c = 12;
            ui->DocupTableWidget->item(row,b)->setText("1");
            UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),c));
            lbl->clear();
        }
        return;
    }
    if (check->isChecked()) {
        QString Text = (check == ui->DocAdministratifcheckBox? "0" : "1");
        ui->DocupTableWidget->item(row,b)->setText(Text);
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),c));
        lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        if (check == ui->DocAdministratifcheckBox)
        {
            ui->PrescriptioncheckBox->setChecked(false);
            b = 6;
            c = 7;
            ui->DocupTableWidget->item(row,b)->setText("");
            UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),c));
            lbl->clear();
        }
    }
    else {
        QString Text = (check == ui->DocAdministratifcheckBox? "1" : "");
        ui->DocupTableWidget->item(row,b)->setText(Text);
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),c));
        lbl->clear();
    }
}

void dlg_documents::dblClicktextEdit()
{
    if (gMode == Selection)
    {
        UpLineEdit *line;
        int row = 0;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText())
            {
                row = line->getRowTable();
                break;
            }
        }
        int idUser = ui->DocupTableWidget->item(row,5)->text().toInt();
        if (idUser == db->getUserConnected()->id())
            ConfigMode(ModificationDOC,row);
    }
}

// ----------------------------------------------------------------------------------
// On entre sur une ligne de comm. On affiche le tooltip
// ----------------------------------------------------------------------------------
void dlg_documents::DocCellEnter(UpLineEdit *line)
{
    QPoint pos = cursor().pos();
    QRect rect = QRect(pos,QSize(10,10));
    int row = line->getRowTable();

    if (ui->DocupTableWidget->isAncestorOf(line))
    {
        //    QRect rect = QRect(itemselect->tableWidget()->pos(),itemselect->tableWidget()->size());
        QTextEdit *text = new QTextEdit;
        MetAJour(ui->DocupTableWidget->item(row,2)->text(),false);
        text->setText(glisttxt.at(0));
        QString ab = text->toPlainText();
        ab.replace(QRegExp("\n\n[\n]*"),"\n");
        if (ab.endsWith("\n"))
            ab = ab.left(ab.size()-1);
        if (ab.size()>300)
        {
            ab = ab.left(300);
            if (ab.endsWith("\n"))
                ab = ab.left(ab.size()-1);
            else if (!ab.endsWith("."))
            {
                if (ab.endsWith(" "))
                    ab = ab.left(ab.size()-1) + "...";
                else
                {
                    int a = ab.lastIndexOf(" ");
                    int b = ab.lastIndexOf(".");
                    int c = b;
                    if (a>b) c = a;
                    ab = ab.left(c) + "...";
                }
            }
        }
        int cassure = 50;
        QStringList listhash1;
        listhash1 = ab.split("\n");
        QString ResumeItem = "";
        for (int i=0; i<listhash1.size(); i++)
        {
            QString abc = listhash1.at(i);
            if (listhash1.at(i).size() > cassure)
            {
                QStringList listhash;
                listhash = listhash1.at(i).split(" ");
                abc = listhash.at(0);
                int count = abc.size() + 1;
                for (int j=1; j<listhash.size(); j++)
                {
                    count += listhash.at(j).size();
                    if (count>cassure)
                    {
                        abc += "\n";
                        count = listhash.at(j).size() + 1;
                    }
                    else
                    {
                        abc += " ";
                        count += 1;
                    }
                    abc += listhash.at(j);
                }
            }
            ResumeItem += abc;
            if (i <(listhash1.size()-1)) ResumeItem += "\n";
        }
        delete text;
        QToolTip::showText(QPoint(pos.x()+50,pos.y()), ResumeItem, ui->DocupTableWidget, rect, 2000);
    }
    else if (ui->DossiersupTableWidget->isAncestorOf(line))
    {
        bool ok;
        int idMetaDoc = ui->DossiersupTableWidget->item(row,2)->text().toInt();
        QString req = "select resumedocument, iddocument from " NOM_TABLE_COURRIERS
                " where iddocument in (select iddocument from " NOM_TABLE_JOINTURESDOCS
                " where idmetadocument = " + QString::number(idMetaDoc) + ")";
        //UpMessageBox::Watch(this,req);
        QList<QList<QVariant>> listdocs = db->StandardSelectSQL(req,ok);
        QString resume = "";
        if (listdocs.size()>0)
        {
            resume += listdocs.at(0).at(0).toString();
            for (int i = 1; i< listdocs.size(); i++)
                resume += "\n" + listdocs.at(i).at(0).toString();
            QToolTip::showText(QPoint(pos.x()+50,pos.y()), resume, ui->DossiersupTableWidget, rect, 2000);
        }
    }
}

// ----------------------------------------------------------------------------------
// On a cliqué une ligne. On affiche le détail
// ----------------------------------------------------------------------------------
void dlg_documents::DocCellDblClick(UpLineEdit *line)
{
    if (ui->DocupTableWidget->isAncestorOf(line))
        ConfigMode(ModificationDOC, line->getRowTable());
    else if (ui->DossiersupTableWidget->isAncestorOf(line))
        ConfigMode(ModificationDOSS, line->getRowTable());
}

// ----------------------------------------------------------------------------------
// Enable OKpushbutton
// ----------------------------------------------------------------------------------
void dlg_documents::EnableOKPushButton(UpCheckBox *Check)
{
    if (gMode == CreationDOC || gMode == ModificationDOC)
    {
        UpLineEdit *line = new UpLineEdit(this);
        bool a = false;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) {a = true; break;}
        }
        if (a == false)
        {
            ui->OKupPushButton->setEnabled(false);
            return;
        }
        if (line->text().size() == 0)
        {
            ui->OKupPushButton->setEnabled(false);
            return;
        }
        if (ui->upTextEdit->toPlainText().size() == 0)
        {
            ui->OKupPushButton->setEnabled(false);
            return;
        }
        ui->OKupPushButton->setEnabled(true);
    }
    else if (gMode == CreationDOSS || gMode == ModificationDOSS)
    {
        UpLineEdit *line= new UpLineEdit(this);
        bool a = false;
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) {a = true; break;}
        }
        if (a == false)
        {
            ui->OKupPushButton->setEnabled(false);
            return;
        }
        if (line->text().size() == 0)
        {
            ui->OKupPushButton->setEnabled(false);
            return;
        }
        a = false;
        for (int i=0 ; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if(Check->isChecked()) {a = true;   break;}
            }
        }
        ui->OKupPushButton->setEnabled(a);
    }
    else if (gMode == Selection)
    {
        if (Check != Q_NULLPTR)
        {
            if (ui->DossiersupTableWidget->isAncestorOf(Check))
            {
                bool A = Check->isChecked();
                int idDossier = ui->DossiersupTableWidget->item(Check->getRowTable(),2)->text().toInt();
                CocheLesDocs(idDossier,A);
            }
            if (ui->DocupTableWidget->isAncestorOf(Check))
            {
                QString nomdoc = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(Check->getRowTable(),1))->text();
                if (!Check->isChecked())
                {
                    VerifDossiers();
                    ui->DocupTableWidget->item(Check->getRowTable(),10)->setText("1" + nomdoc);
                }
                else
                    ui->DocupTableWidget->item(Check->getRowTable(),10)->setText("0" + nomdoc);
            }
        }
        if (ui->ChercheupLineEdit->text() =="")
            glistid.clear();
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if(Check->isChecked())
                    glistid << ui->DocupTableWidget->item(i,3)->text();
                else
                {
                    for (int k=0; k<glistid.size();k++){
                        if (glistid.at(k) == ui->DocupTableWidget->item(Check->getRowTable(),3)->text()){
                            glistid.removeAt(k);
                            k--;
                        }
                    }
                }
            }
        }
        bool a = false;
        for (int i=0 ; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if(Check->isChecked()) {a = true;   break;}
            }
        }
        ui->OKupPushButton->setEnabled(a);
    }
}

void dlg_documents::FiltreListe(QString)
{
    Remplir_TableWidget();
    EnableLines();
    for (int j=0; j<glistid.size(); j++)
    {
        QList<QTableWidgetItem*> listitems = ui->DocupTableWidget->findItems(glistid.at(j),Qt::MatchExactly);
        for (int k=0; k<listitems.size(); k++)
        {
            if (listitems.at(k)->column() == 3)
            {
                QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(listitems.at(k)->row(),0));
                if (Widg)
                {
                    UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                    Check->setChecked(true);
                }
            }
        }
    }
    if (ui->DocupTableWidget->rowCount()>0)
        dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(0,1))->selectAll();
}

void dlg_documents::MenuContextuel(QWidget *widg)
{
    gmenuContextuel = new QMenu(this);
    QAction *pAction_ModifDossier;
    QAction *pAction_SupprDossier;
    QAction *pAction_CreerDossier;
    QAction *pAction_PublicDossier;
    QAction *pAction_ModifDoc;
    QAction *pAction_SupprDoc;
    QAction *pAction_CreerDoc;
    QAction *pAction_PublicDoc;
    QAction *pAction_PrescripDoc;
    QAction *pAction_EditableDoc;
    QAction *pAction_AdminDoc;
    QAction *pAction_InsertChamp;
    QAction *pAction_ModifPolice;
    QAction *pAction_Fontbold;
    QAction *pAction_Fontitalic;
    QAction *pAction_Fontunderline;
    QAction *pAction_Fontnormal;
    QAction *pAction_Blockcentr;
    QAction *pAction_Blockjust;
    QAction *pAction_Blockright;
    QAction *pAction_Blockleft;
    QAction *pAction_Copier;
    QAction *pAction_Cut;
    QAction *pAction_Coller;
    QAction *pAction_InsInterroDate;
    QAction *pAction_InsInterroCote;
    QAction *pAction_InsInterroHeure;
    QAction *pAction_InsInterroMontant;
    QAction *pAction_InsInterroText;
    QMenu *interro = new QMenu(this);
    UpLineEdit *line0;
    UpLineEdit *line = dynamic_cast<UpLineEdit*>(widg);

    if (line)
    {
        if (ui->DossiersupTableWidget->isAncestorOf(line))
        {
            LineSelect(ui->DossiersupTableWidget,line->getRowTable());

            pAction_CreerDossier            = gmenuContextuel->addAction(Icons::icCreer(), tr("Créer un dossier")) ;
            pAction_ModifDossier            = gmenuContextuel->addAction(Icons::icEditer(), tr("Modifier ce dossier")) ;
            pAction_SupprDossier            = gmenuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce dossier")) ;
            gmenuContextuel->addSeparator();
            UpLabel *lbl                    = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(line->getRowTable(),4));
            if (lbl->pixmap())
                pAction_PublicDossier       = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Public")) ;
            else
                pAction_PublicDossier       = gmenuContextuel->addAction(tr("Public")) ;

            connect (pAction_ModifDossier,  &QAction::triggered,    [=] {ChoixMenuContextuel("ModifierDossier");});
            connect (pAction_SupprDossier,  &QAction::triggered,    [=] {ChoixMenuContextuel("SupprimerDossier");});
            connect (pAction_CreerDossier,  &QAction::triggered,    [=] {ChoixMenuContextuel("CreerDossier");});
            connect (pAction_PublicDossier, &QAction::triggered,    [=] {ChoixMenuContextuel("PublicDossier");});
        }
        else if (ui->DocupTableWidget->isAncestorOf(line))
        {
            LineSelect(ui->DocupTableWidget,line->getRowTable());

            pAction_ModifDoc                = gmenuContextuel->addAction(Icons::icEditer(), tr("Modifier ce document"));
            pAction_SupprDoc                = gmenuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce document"));
            pAction_CreerDoc                = gmenuContextuel->addAction(Icons::icCreer(), tr("Créer un document"));
            if (ui->DocupTableWidget->item(line->getRowTable(),4)->text().toInt() == 1)
                pAction_PublicDoc           = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Public"));
            else
                pAction_PublicDoc           = gmenuContextuel->addAction(tr("Public"));
            if (db->getUserConnected()->isMedecin() || db->getUserConnected()->isOrthoptist())
            {
                if (ui->DocupTableWidget->item(line->getRowTable(),6)->text().toInt() == 1)
                    pAction_PrescripDoc         = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Prescription"));
                else
                    pAction_PrescripDoc         = gmenuContextuel->addAction(tr("Prescription"));
                pAction_PrescripDoc ->setToolTip(tr("si cette option est cochée\nce document sera considéré comme une prescription"));
                connect (pAction_PrescripDoc,   &QAction::triggered,    [=] {ChoixMenuContextuel("PrescripDoc");});
            }
            if (ui->DocupTableWidget->item(line->getRowTable(),8)->text().toInt() == 1)
                pAction_EditableDoc         = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Editable"));
            else
                pAction_EditableDoc         = gmenuContextuel->addAction(tr("Editable"));
            if (db->getUserConnected()->isMedecin() || db->getUserConnected()->isOrthoptist())
            {
                if (ui->DocupTableWidget->item(line->getRowTable(),11)->text().toInt() != 1)
                    pAction_AdminDoc        = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Document administratif"));
                else
                    pAction_AdminDoc        = gmenuContextuel->addAction(tr("Document administratif"));
                pAction_AdminDoc    ->setToolTip(tr("si cette option est cochée\nle document est considéré comme un document non médical"));
                connect (pAction_AdminDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("AdminDoc");});
            }

            pAction_PublicDoc   ->setToolTip(tr("si cette option est cochée\ntous les utilisateurs\nauront accès à ce document"));
            pAction_EditableDoc ->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));

            connect (pAction_ModifDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("ModifierDoc");});
            connect (pAction_SupprDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("SupprimerDoc");});
            connect (pAction_CreerDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("CreerDoc");});
            connect (pAction_PublicDoc,     &QAction::triggered,    [=] {ChoixMenuContextuel("PublicDoc");});
            connect (pAction_EditableDoc,   &QAction::triggered,    [=] {ChoixMenuContextuel("EditDoc");});
        }
    }

    else if (gMode == Selection)
    {
        bool a = false;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line0 = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line0->hasSelectedText())
                if (ui->DocupTableWidget->item(line0->getRowTable(),5)->text().toInt() == gidUser)
                {a =true; break;}
        }
        if (a)
        {
            pAction_ModifDoc       = gmenuContextuel->addAction(Icons::icEditer(), tr("Modifier ce document"));
            connect (pAction_ModifDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("ModifierDoc");});
        }
    }
    else if (widg == ui->upTextEdit)
    {
        pAction_InsertChamp         = gmenuContextuel->addAction    (Icons::icAjouter(), tr("Insérer un champ"));
        interro                     = gmenuContextuel->addMenu      (Icons::icAjouter(), tr("Insérer une interrogation"));
        pAction_InsInterroDate      = interro->addAction            (Icons::icDate(),   tr("Date"));
        pAction_InsInterroHeure     = interro->addAction            (Icons::icClock(),  tr("Heure"));
        pAction_InsInterroCote      = interro->addAction            (Icons::icSide(),   tr("Côté"));
        pAction_InsInterroMontant   = interro->addAction            (Icons::icEuro(),   tr("Montant"));
        pAction_InsInterroText      = interro->addAction            (Icons::icText(),   tr("Texte libre"));

        gmenuContextuel->addSeparator();
        if (ui->upTextEdit->textCursor().selectedText().size() > 0)   {
            pAction_ModifPolice     = gmenuContextuel->addAction(Icons::icFont(),           tr("Modifier la police"));
            pAction_Fontbold        = gmenuContextuel->addAction(Icons::icFontbold(),       tr("Gras"));
            pAction_Fontitalic      = gmenuContextuel->addAction(Icons::icFontitalic(),     tr("Italique"));
            pAction_Fontunderline   = gmenuContextuel->addAction(Icons::icFontunderline(),  tr("Souligné"));
            pAction_Fontnormal      = gmenuContextuel->addAction(Icons::icFontnormal(),     tr("Normal"));

            connect (pAction_ModifPolice,       &QAction::triggered,    [=] {ChoixMenuContextuel("Police");});
            connect (pAction_Fontbold,          &QAction::triggered,    [=] {ChoixMenuContextuel("Gras");});
            connect (pAction_Fontitalic,        &QAction::triggered,    [=] {ChoixMenuContextuel("Italique");});
            connect (pAction_Fontunderline,     &QAction::triggered,    [=] {ChoixMenuContextuel("Souligne");});
            connect (pAction_Fontnormal,        &QAction::triggered,    [=] {ChoixMenuContextuel("Normal");});
            gmenuContextuel->addSeparator();
        }
        pAction_Blockleft       = gmenuContextuel->addAction(Icons::icBlockLeft(),          tr("Aligné à gauche"));
        pAction_Blockright      = gmenuContextuel->addAction(Icons::icBlockRight(),         tr("Aligné à droite"));
        pAction_Blockcentr      = gmenuContextuel->addAction(Icons::icBlockCenter(),        tr("Centré"));
        pAction_Blockjust       = gmenuContextuel->addAction(Icons::icBlockJustify(),       tr("Justifié"));
        gmenuContextuel->addSeparator();
        if (ui->upTextEdit->textCursor().selectedText().size() > 0)   {
            pAction_Copier          = gmenuContextuel->addAction(Icons::icCopy(),   tr("Copier"));
            pAction_Cut             = gmenuContextuel->addAction(Icons::icCut(),    tr("Couper"));
            connect (pAction_Copier,            &QAction::triggered,    [=] {ChoixMenuContextuel("Copier");});
            connect (pAction_Cut,               &QAction::triggered,    [=] {ChoixMenuContextuel("Couper");});
        }
        if (qApp->clipboard()->mimeData()->hasText()
                || qApp->clipboard()->mimeData()->hasUrls()
                || qApp->clipboard()->mimeData()->hasImage()
                || qApp->clipboard()->mimeData()->hasHtml())
        pAction_Coller              = gmenuContextuel->addAction(Icons::icPaste(),  tr("Coller"));

        connect (pAction_InsertChamp,       &QAction::triggered,    [=] {ChoixMenuContextuel("Inserer");});
        connect (pAction_InsInterroDate,    &QAction::triggered,    [=] {ChoixMenuContextuel("Date");});
        connect (pAction_InsInterroCote,    &QAction::triggered,    [=] {ChoixMenuContextuel("Cote");});
        connect (pAction_InsInterroHeure,   &QAction::triggered,    [=] {ChoixMenuContextuel("Heure");});
        connect (pAction_InsInterroMontant, &QAction::triggered,    [=] {ChoixMenuContextuel("Montant");});
        connect (pAction_InsInterroText,    &QAction::triggered,    [=] {ChoixMenuContextuel("Texte");});
        connect (pAction_Blockcentr,        &QAction::triggered,    [=] {ChoixMenuContextuel("Centre");});
        connect (pAction_Blockright,        &QAction::triggered,    [=] {ChoixMenuContextuel("Droite");});
        connect (pAction_Blockleft,         &QAction::triggered,    [=] {ChoixMenuContextuel("Gauche");});
        connect (pAction_Blockjust,         &QAction::triggered,    [=] {ChoixMenuContextuel("Justifie");});
        connect (pAction_Coller,            &QAction::triggered,    [=] {ChoixMenuContextuel("Coller");});
    }

    // ouvrir le menu
    gmenuContextuel->exec(cursor().pos());
    delete gmenuContextuel;
}

void dlg_documents::ChoixMenuContextuel(QString choix)
{
    bool a = false;
    QPoint pos = ui->DocupTableWidget->viewport()->mapFromGlobal(gmenuContextuel->pos());
    if (choix       == "Coller")    ui->upTextEdit->paste();

    else if (choix  == "Copier")    ui->upTextEdit->copy();

    else if (choix  == "Couper")    ui->upTextEdit->cut();

    else if (choix  == "ModifierDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        ConfigMode(ModificationDOC,line->getRowTable());
    }
    else if (choix  == "SupprimerDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        SupprimmDocument(line->getRowTable());
    }
    else if (choix  == "ModifierDossier")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        ConfigMode(ModificationDOSS,line->getRowTable());
    }
    else if (choix  == "PublicDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),7));
        QString a = "null";
        if (!lbl->pixmap())
        {
            a = "1";
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        }
        else
        {
            if (!VerifDocumentPublic(line->getRowTable())) return;
            lbl->clear();
        }
        ui->DocPubliccheckBox->toggle();
        if (gMode == Selection)
            db->StandardSQL("update " NOM_TABLE_COURRIERS " set DocPublic = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text());
        if (a=="null") a= "";
        ui->DocupTableWidget->item(line->getRowTable(),4)->setText(a);
    }
    else if (choix  == "EditDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),9));
        QString a = "null";
        if (!lbl->pixmap())
        {
            a = "1";
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        }
        else
            lbl->clear();
        ui->DocEditcheckBox->toggle();
        if (gMode == Selection)
            db->StandardSQL("update " NOM_TABLE_COURRIERS " set Editable = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text());
        if (a=="null") a= "";
        ui->DocupTableWidget->item(line->getRowTable(),8)->setText(a);
    }
    else if (choix  == "AdminDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),12));
        QString a = "null";
        if (!lbl->pixmap())
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        else
        {
            a = "1";
            lbl->clear();
        }
        ui->DocAdministratifcheckBox->toggle();
        if (gMode == Selection)
            db->StandardSQL("update " NOM_TABLE_COURRIERS " set Medical = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text());
        if (a=="null") a= "";
        ui->DocupTableWidget->item(line->getRowTable(),11)->setText(a);
    }
    else if (choix  == "PublicDossier")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        UpLabel *lbl = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(line->getRowTable(),4));
        QString a = "null";
        if (!lbl->pixmap())
        {
            if (!VerifDossierPublic(line->getRowTable())) return;
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
            a = "1";
        }
        else
            lbl->clear();
        if (gMode == Selection)
            db->StandardSQL("update " NOM_TABLE_METADOCUMENTS " set Public = " + a + " where idMetaDocument = " +
                       ui->DossiersupTableWidget->item(line->getRowTable(),2)->text());
    }
    else if (choix  == "PrescripDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        ui->PrescriptioncheckBox->toggle();
        QString a = "null";
        if (ui->PrescriptioncheckBox->isChecked()) a = "1";
        if (gMode == Selection)
            db->StandardSQL("update " NOM_TABLE_COURRIERS " set Prescription = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text());
        if (a=="null") a= "";
        ui->DocupTableWidget->item(line->getRowTable(),6)->setText(a);
    }
    else if (choix  == "CreerDoc")
    {
        int row = ui->DocupTableWidget->rowAt(pos.y());
        ConfigMode(CreationDOC,row);
    }
    else if (choix  == "CreerDossier")
    {
        int row = ui->DossiersupTableWidget->rowAt(pos.y());
        ConfigMode(CreationDOSS,row);
    }
    else if (choix  == "SupprimerDossier")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a == false) return;
        SupprimmDossier(line->getRowTable());
    }
    else if (choix  == "Police")    {
        bool ok = false;
        QFont police = QFontDialog::getFont(&ok, ui->upTextEdit->textCursor().charFormat().font(), this, "Choisissez une police");
        if (ok){
            QTextCharFormat format;
            format.setFont(police);
            ui->upTextEdit->textCursor().setCharFormat(format);
        }
    }
    else if (choix  == "Gras")    {
        QTextCharFormat format  = ui->upTextEdit->textCursor().charFormat();
        format.setFontWeight(QFont::Bold);
        format.setFontUnderline(format.fontUnderline());
        format.setFontItalic(format.fontItalic());
        ui->upTextEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Italique")    {
        QTextCharFormat format  = ui->upTextEdit->textCursor().charFormat();
        format.setFontItalic(true);
        format.setFontUnderline(format.fontUnderline());
        format.setFontWeight(format.fontWeight());
        ui->upTextEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Souligne")    {
        QTextCharFormat format  = ui->upTextEdit->textCursor().charFormat();
        format.setFontUnderline(true);
        format.setFontItalic(format.fontItalic());
        format.setFontWeight(format.fontWeight());
        ui->upTextEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Normal")    {
        QTextCharFormat format  = ui->upTextEdit->textCursor().charFormat();
        format.setFont(qApp->font());
        ui->upTextEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Gauche")    {
        QTextCursor curs = ui->upTextEdit->textCursor();
        ui->upTextEdit->moveCursor(QTextCursor::StartOfBlock);
        ui->upTextEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignLeft);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
        ui->upTextEdit->setTextCursor(curs);
    }
    else if (choix  == "Justifie")    {
        QTextCursor curs = ui->upTextEdit->textCursor();
        ui->upTextEdit->moveCursor(QTextCursor::StartOfBlock);
        ui->upTextEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignJustify);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
        ui->upTextEdit->setTextCursor(curs);
    }
    else if (choix  == "Droite")    {
        QTextCursor curs = ui->upTextEdit->textCursor();
        ui->upTextEdit->moveCursor(QTextCursor::StartOfBlock);
        ui->upTextEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignRight);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
        ui->upTextEdit->setTextCursor(curs);
    }
    else if (choix  == "Centre")    {
        QTextCursor curs = ui->upTextEdit->textCursor();
        ui->upTextEdit->moveCursor(QTextCursor::StartOfBlock);
        ui->upTextEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignCenter);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
        ui->upTextEdit->setTextCursor(curs);
    }
    else if (choix  == "Inserer")   {
        UpDialog        *ListChamps     = new UpDialog(this);
        QVBoxLayout     *globallayout   = dynamic_cast<QVBoxLayout*>(ListChamps->layout());

        UpTableWidget   *tabChamps      = new UpTableWidget();
        QFontMetrics    fm(qApp->font());

        tabChamps->verticalHeader()->setVisible(false);
        tabChamps->horizontalHeader()->setVisible(false);
        tabChamps->setFocusPolicy(Qt::NoFocus);
        tabChamps->setSelectionMode(QAbstractItemView::SingleSelection);
        tabChamps->setSelectionBehavior(QAbstractItemView::SelectRows);
        tabChamps->setColumnCount(2);
        tabChamps->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tabChamps->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tabChamps->setFixedHeight(int(fm.height()*1.1*9));
        tabChamps->setGridStyle(Qt::DotLine);
        tabChamps->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tabChamps->setColumnWidth(1,0); // si on met setcolumnhidden, ça ne rentre pas dans les selecteditems()
        QTableWidgetItem        *pitem0, *pitem1;
        int                     i = 0;
        for(QMap<QString, QString>::iterator it = gChampsMap.begin(); it!=gChampsMap.end(); ++it)
        {
            tabChamps   ->insertRow(i);
            pitem0       = new QTableWidgetItem;
            pitem0       ->setText(gChampsMap[it.key()]);
            tabChamps   ->setItem(i,0,pitem0);
            pitem1       = new QTableWidgetItem;
            pitem1      ->setText(it.key());
            tabChamps   ->setItem(i,0,pitem0);
            tabChamps   ->setItem(i,1,pitem1);
            tabChamps   ->setRowHeight(i,int(fm.height()*1.1));
            i++;
        }
        tabChamps->sortItems(0);
        tabChamps->resizeColumnToContents(0);
        tabChamps->setColumnWidth(0,tabChamps->columnWidth(0)+30);
        if (tabChamps->columnWidth(0) < 250)
            tabChamps->setColumnWidth(0,250);
        tabChamps->setFixedWidth(tabChamps->columnWidth(0)+2);

        ListChamps->AjouteLayButtons(UpDialog::ButtonOK);
        ListChamps->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
        globallayout->insertWidget(0,tabChamps);

        ListChamps->setModal(true);
        ListChamps->move(QPoint(x()+width()/2,y()+height()/2));

        connect(ListChamps->OKButton,   &QPushButton::clicked,          [=] {ListChamps->accept();});
        ListChamps->setFixedWidth(tabChamps->width() + globallayout->contentsMargins().left()*2);
        connect(tabChamps,              &QTableWidget::doubleClicked,   [=] {ListChamps->accept();});
        globallayout->setSizeConstraint(QLayout::SetFixedSize);

        if (ListChamps->exec()>0)   {
            if (tabChamps->selectedItems().size()>0)
            {
                QString champ = tabChamps->selectedItems().at(1)->text();
                ui->upTextEdit->textCursor().insertText("{{" + champ + "}}");
            }
        }
        delete ListChamps;
    }
    else if (choix == "Date")
    {
        if (AskDialog("Choix de date")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + gAskDialog->findChildren<UpLineEdit*>().at(0)->text() + "//DATE))");
        delete gAskDialog;
    }
    else if (choix == "Texte")
    {
        if (AskDialog("Choix de texte")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + gAskDialog->findChildren<UpLineEdit*>().at(0)->text() + "//TEXTE))");
        delete gAskDialog;
    }
    else if (choix == "Montant")
    {
        if (AskDialog("Choix de texte")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + gAskDialog->findChildren<UpLineEdit*>().at(0)->text() + "//MONTANT))");
        delete gAskDialog;
    }
    else if (choix == "Heure")
    {
        if (AskDialog("Choix d'une heure")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + gAskDialog->findChildren<UpLineEdit*>().at(0)->text() + "//HEURE))");
        delete gAskDialog;
    }
    else if (choix == "Cote")
    {
        if (AskDialog("Choix d'un côté")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + gAskDialog->findChildren<UpLineEdit*>().at(0)->text() + "//COTE))");
        delete gAskDialog;
    }
}


// ----------------------------------------------------------------------------------
// Clic sur le bouton OK.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_documents::Validation()
{
    UpLineEdit *line = new UpLineEdit();
    int         c = 0;
    QStringList listQuestions, listtypeQuestions;
    QStringList ExpARemplacer, Rempla;
    QString listusers = "ListUsers";
    gidUserEntete = -1;
    if (gidUser == gidUserSuperviseur)
        gidUserEntete = gidUser;

    switch (gMode) {
    case CreationDOC:
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        InsertDocument(line->getRowTable());
        break;
    case CreationDOSS:
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        InsertDossier(line->getRowTable());
        break;
    case ModificationDOC:
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        UpdateDocument(line->getRowTable());
        break;
    case ModificationDOSS:
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        UpdateDossier(line->getRowTable());
        break;
    case Selection:                                                         // -> On imprime le
        for (int i =0 ; i < ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if(Check->isChecked()) c++;
            }
        }
        if (c == 0)
        {
            UpMessageBox::Watch(this,"Euuhh... " + db->getUserConnected()->getLogin() + ", " + tr("il doit y avoir une erreur..."), tr("Vous n'avez sélectionné aucun document."));
            break;
        }

        for (int i =0 ; i < ui->DocupTableWidget->rowCount(); i++)
        {
            // on établit d'abord la liste de questions à poser
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if (Check->isChecked())
                {
                    QString text = ui->DocupTableWidget->item(i,2)->text();
                    //QString questdate = "([(][(][\\S ]*//(DATE|TEXTE)[)][)])";
                    QString quest = "([(][(][éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°, -]*//(DATE|TEXTE|COTE|HEURE|MONTANT)[)][)])";
                    QRegExp reg;
                    reg.setPattern(quest);
                    int count = 0;
                    int pos = 0;
                    while (reg.indexIn(text, pos) != -1)
                    {
                        pos = reg.indexIn(text, pos);
                        ++count;
                        pos += reg.matchedLength();
                        int fin = reg.cap(1).indexOf("//");
                        int lengthquest = fin-2;
                        int lengthtype = reg.cap(1).length() - fin;
                        bool a = true;
                        if (listQuestions.size()>0)
                            for (int j=0; j<listQuestions.size();j++)
                            {
                                if (listQuestions.at(j) == reg.cap(1).mid(2,lengthquest))
                                {
                                    a = false;
                                    break;
                                }
                            }
                        if (a){
                            listQuestions << reg.cap(1).mid(2,lengthquest);
                            listtypeQuestions << reg.cap(1).mid(fin+2,lengthtype-4);
                        }
                    }
                }
            }
        }
        // On a établi la liste de questions - on prépare la fiche qui va les poser
        if (listQuestions.size()>0 || gidUser != gidUserSuperviseur)
        {
            gAsk = new UpDialog(this);
            gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
            gAsk->setModal(true);
            gAsk->setSizeGripEnabled(false);
            gAsk->move(QPoint(x()+width()/2,y()+height()/2));
            gAsk->setWindowTitle(tr("Complétez la fiche"));

            QVBoxLayout *layWidg = new QVBoxLayout();
            QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(gAsk->layout());
            for (int m=0; m<listQuestions.size();m++)
            {
                QHBoxLayout *lay = new QHBoxLayout();
                lay->setContentsMargins(5,0,5,0);
                layWidg->addLayout(lay);
                UpLabel *label = new UpLabel();
                label->setText(listQuestions.at(m));
                label->setFixedSize(150,25);
                lay->addWidget(label);
                QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Expanding);
                lay->addSpacerItem(spacer);
                if (listtypeQuestions.at(m)  == "TEXTE")
                {
                    UpLineEdit *Line = new UpLineEdit();
                    Line->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
                    Line->setMaxLength(60);
                    Line->setFixedHeight(23);
                    lay->addWidget(Line);
                }
                else if (listtypeQuestions.at(m)  == "MONTANT")
                {
                    UpLineEdit *Line = new UpLineEdit(gAsk);
                    QDoubleValidator *val= new QDoubleValidator(this);
                    val->setDecimals(2);
                    Line->setValidator(val);
                    Line->setMaxLength(15);
                    Line->setFixedHeight(23);
                    Line->installEventFilter(this);
                    lay->addWidget(Line);
                }
                else if (listtypeQuestions.at(m)  == "DATE")
                {
                    QDateEdit *Date = new QDateEdit();
                    Date->setCalendarPopup(false); //si on met ce paramètre à true et qu'on utilise le calendarpopup,
                                                   //la fonction QLayout::count() utilisée un peu plus loin plante le programme..(?)..
                    Date->setContentsMargins(0,0,0,0);
                    Date->setFixedSize(110,30);
                    Date->setDate(QDate::currentDate());
                    lay->addWidget(Date);
                }
                else if (listtypeQuestions.at(m)  == "HEURE")
                {
                    QTimeEdit *Time = new QTimeEdit();
                    Time->setContentsMargins(0,0,0,0);
                    Time->setFixedSize(70,30);
                    Time->setTime(QTime::currentTime());
                    Time->setTimeSpec(Qt::LocalTime);
                    lay->addWidget(Time);
                }
                else if (listtypeQuestions.at(m)  == "COTE")
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    QStringList listcote;
                    listcote << tr("chaque oeil") << tr("l'oeil droit") << tr("l'oeil gauche");
                    Combo->addItems(listcote);
                    //Combo->setStyleSheet("QComboBox {margin-top: 0px; pxmargin-bottom: 10px;}");
                    lay->addWidget(Combo);
                }
            }
            if (listQuestions.size()>0 && gidUser != gidUserSuperviseur)
            {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                layWidg->addWidget(line);
            }
            if (gidUser != gidUserSuperviseur)
            {
                QHBoxLayout *lay = new QHBoxLayout();
                lay->setContentsMargins(5,0,5,0);
                layWidg->addLayout(lay);
                UpLabel *label = new UpLabel();
                label->setText(tr("Quel soignant?"));
                label->setFixedSize(150,25);
                lay->addWidget(label);
                QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Expanding);
                lay->addSpacerItem(spacer);
                UpComboBox *Combo = new UpComboBox();
                Combo->setContentsMargins(0,0,0,0);
                Combo->setFixedHeight(34);
                Combo->setEditable(false);

                for( QMap<int, User*>::const_iterator itSup = Datas::I()->users->superviseurs()->constBegin();
                     itSup != Datas::I()->users->superviseurs()->constEnd(); ++itSup )
                    Combo->addItem(itSup.value()->getLogin(), QString::number(itSup.value()->id()) );

                Combo->setAccessibleDescription(listusers);
                lay->addWidget(Combo);
            }
            globallay   ->setContentsMargins(5,5,5,5);
            layWidg     ->setContentsMargins(0,0,0,0);
            layWidg     ->setSpacing(10);
            globallay   ->setSpacing(5);
            globallay   ->insertLayout(0,layWidg);

            gAsk->AjouteLayButtons();
            globallay->setSizeConstraint(QLayout::SetFixedSize);
            connect(gAsk->OKButton,     &QPushButton::clicked,   [=] {VerifCoherencegAsk();});

            if (gAsk->exec() == 0)
            {
                delete gAsk;
                return;
            }
            else
            {
                for (int n=0; n< layWidg->count(); n++)
                {
                    QHBoxLayout *hlay = dynamic_cast<QHBoxLayout*>(layWidg->itemAt(n));
                    if (hlay != Q_NULLPTR)
                    {
                        bool b = false;
                        for (int r=0; r<hlay->count(); r++)
                        {
                            QWidget *w = hlay->itemAt(r)->widget();
                            b = (dynamic_cast<UpLabel*>(w)!=Q_NULLPTR);
                            if (b) break;
                            w=Q_NULLPTR;
                            delete w;
                        }
                        if (b)
                        {
                            for (int q=0; q<hlay->count(); q++)
                            {
                                QLayoutItem *layoutItem = hlay->itemAt(q);
                                if (layoutItem->spacerItem()) {
                                    hlay->removeItem(layoutItem);
                                    delete layoutItem;
                                    --q;
                                }
                            }
                            QList<QWidget*> listwidg;
                            for (int s=0; s<hlay->count(); s++)
                                listwidg << hlay->itemAt(s)->widget();
                            QString minidou;
                            for (int p=0; p<listwidg.size(); p++)
                            {
                                if (listwidg.at(p)->inherits("UpLabel"))
                                {
                                    UpLabel *linelabel = static_cast<UpLabel*>(listwidg.at(p));
                                    minidou = "((" + linelabel->text();
                                    delete linelabel;
                                }
                                else if (listwidg.at(p)->inherits("UpLineEdit"))
                                {
                                    UpLineEdit *linetext = static_cast<UpLineEdit*>(listwidg.at(p));
                                    Rempla          << linetext->text();
                                    const QDoubleValidator *val = dynamic_cast<const QDoubleValidator*>(linetext->validator());
                                    if (val)
                                        ExpARemplacer   << minidou + "//MONTANT))";
                                    else
                                        ExpARemplacer   << minidou + "//TEXTE))";
                                    delete val;
                                    delete linetext;
                                }
                                else if (listwidg.at(p)->inherits("QDateEdit"))
                                {
                                    QDateEdit *linedate = static_cast<QDateEdit*>(listwidg.at(p));
                                    Rempla          << linedate->date().toString(tr("d MMMM yyyy"));
                                    ExpARemplacer   << minidou + "//DATE))";
                                    delete linedate;
                                }
                                else if (listwidg.at(p)->inherits("QTimeEdit"))
                                {
                                    QTimeEdit *linetime = static_cast<QTimeEdit*>(listwidg.at(p));
                                    Rempla          << linetime->time().toString("H'H'mm");
                                    ExpARemplacer   << minidou + "//HEURE))";
                                    delete linetime;
                                }
                                else if (listwidg.at(p)->inherits("UpComboBox"))
                                {
                                    UpComboBox *linecombo = static_cast<UpComboBox*>(listwidg.at(p));
                                    if (linecombo->accessibleDescription() != listusers)
                                    {
                                        Rempla          << linecombo->currentText();
                                        ExpARemplacer   << minidou + "//COTE))";
                                    }
                                    else
                                        gidUserEntete = linecombo->currentData().toInt();
                                    delete linecombo;
                                }
                            }
                        }
                    }
                }
            }
            delete gAsk;
        }
        for (int i =0 ; i < ui->DocupTableWidget->rowCount(); i++)
        {
            /* On alimente 5 QStringList. Chaque iteration correspond à la description d'un document
             * TitreDocumentAImprimerList       -> le titre qui sera inséré dans la fiche docsexternes et dans la conclusion
             * prescriptionAImprimerList        -> précise si le document est une prescription - le formatage n'est pas le même
             * DupliAImprimerList               -> la nécessité ou non d'mprimer un dupli
             * TextDocumentsAImprimerList       -> le corps du document à imprimer
             * AdministratifAImprimerList       -> document administratif ou pas
            */
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if (Check->isChecked())
                {
                    // on effectue les corrections de chacun des documents
                    QString textAimprimer = ui->DocupTableWidget->item(i,2)->text();
                    if (ExpARemplacer.size() > 0)
                        for (int y=0; y<ExpARemplacer.size(); y++)
                            textAimprimer.replace(ExpARemplacer.at(y),Rempla.at(y));
                    MetAJour(textAimprimer,true);
                    for (int j=0; j<glisttxt.size();j++)
                    {
                        QString txtdoc = glisttxt.at(j);

                        // on détermine le titre du document à inscrire en conclusion et le statut de prescription (si prescription => impression d'un dupli)
                        QString titre                   = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1))->text();
                        TitreDocumentAImprimerList      << titre;
                        PrescriptionAImprimerList       << ui->DocupTableWidget->item(i,6)->text();
                        QString dupli                   = ((ui->DocupTableWidget->item(i,6)->text() == "1" && ui->DupliOrdocheckBox->isChecked())? "1": "");
                        DupliAImprimerList              << dupli;
                        AdministratifAImprimerList      << ui->DocupTableWidget->item(i,11)->text();
                        // on visualise le document pour correction s'il est éditable
                        txtdoc                          = ((ui->DocupTableWidget->item(i,8)->text() == "1")? proc->Edit(txtdoc, titre): txtdoc);
                        if (txtdoc == "")               // si le texte du document est vide, on annule l'impression de cette itération
                        {
                            TitreDocumentAImprimerList  .removeLast();
                            PrescriptionAImprimerList   .removeLast();
                            DupliAImprimerList          .removeLast();
                            AdministratifAImprimerList  .removeLast();
                        }
                        else TextDocumentsAImprimerList << txtdoc;
                    }
                }
            }
        }
        accept();
        break;
    default:
        break;
    }
}

void dlg_documents::OrdoAvecDupli(bool a)
{
    proc->gsettingsIni->setValue("Param_Imprimante/OrdoAvecDupli",(a? "YES" : "NO"));
}


/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Vérifie que les champs sont remplis avant la fermeture de gAsk ------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_documents::VerifCoherencegAsk()
{
    QList<UpLineEdit*> listUpline = gAsk->findChildren<UpLineEdit*>();
    bool a = true;
    for (int i=0;i<listUpline.size();i++)
    {
        if (listUpline.at(i)->text() == "")
        {
            a = false;
            listUpline.at(i)->setFocus();
            QSound::play(NOM_ALARME);
        }
    }
    if (a)
    {
        QList<UpComboBox*> listCombo = gAsk->findChildren<UpComboBox*>();
        for (int i=0;i<listCombo.size();i++)
        {
            if (listCombo.at(i)->currentText() == "")
            {
                a = false;
                listCombo.at(i)->setFocus();
                QSound::play(NOM_ALARME);
            }
        }
    }
    if (a) gAsk->accept();
}

bool dlg_documents::event(QEvent *event)
{
/*    if (event->type() == QEvent::ToolTip)
    {
        QWidget* rec            = ui->DocupTableWidget;
        QRect rect              = QRect(rec->pos(),rec->size());
        QHelpEvent *helpEvent   = static_cast<QHelpEvent *>(event);
        QTableWidgetItem *itemM = new QTableWidgetItem;
        QPoint pos              = helpEvent->pos();
        QPoint fromPpos         = mapFromParent(pos);
        QPoint fromGpos         = mapFromGlobal(pos);
        QPoint ToPpos           = mapToParent(pos);
        QPoint ToGpos           = mapToGlobal(pos);
        QPoint ToThispos        = rec->mapTo(this,pos);
        QPoint FromThispos      = rec->mapFrom(this,pos);
        QString tru = "true";
        if (!rect.contains(pos)) tru = "false";
        ui->lineEdit->setText(" pos = (" + QString::number(pos.x()) + "," + QString::number(pos.y()) + ") " +
                              " FromParent = (" + QString::number(fromPpos.x()) + "," + QString::number(fromPpos.y()) + ") " +
                              " FromGlobal = (" + QString::number(fromGpos.x()) + "," + QString::number(fromGpos.y()) + ") " +
                              " ToParent = (" + QString::number(ToPpos.x()) + "," + QString::number(ToPpos.y()) + ") " +
                              " ToGlobal = ("+ QString::number(ToGpos.x()) + "," + QString::number(ToGpos.y()) + ")" +
                              " ToThis = (" + QString::number(ToThispos.x()) + "," + QString::number(ToThispos.y()) + ") " +
                              " FromThis = ("+ QString::number(FromThispos.x()) + "," + QString::number(FromThispos.y()) + ")" +
                              " Rect = (" + QString::number(rec->x()) + "," + QString::number(rec->y()) + ") " +
                              " (" + QString::number(rec->x()+rec->width()) + "," + QString::number(rec->y()+rec->height()) + ")" +
                              " " + tru);
        itemM = ui->DocupTableWidget->itemAt(rec->mapFrom(this,pos));
        if (itemM == 0) return false;
        if (itemM->row() > -1) {
            QTextEdit *text = new QTextEdit;
            text->setText(MetAJour(ui->DocupTableWidget->item(itemM->row(),2)->text()));
            QString ab = text->toPlainText().left(300);
            QToolTip::showText(helpEvent->globalPos(), ab);
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }*/
    return QWidget::event(event);
}
void dlg_documents::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void dlg_documents::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionDocuments",saveGeometry());
    event->accept();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_documents::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::FocusIn)
    {
        if (obj->inherits("UpLineEdit"))
        {
            UpLineEdit *linetext = static_cast<UpLineEdit*>(obj);
                linetext->selectAll();
        }
    }
    if(event->type() == QEvent::FocusOut)
    {
        if (obj->inherits("UpLineEdit"))
        {
            UpLineEdit *linetext = static_cast<UpLineEdit*>(obj);
            const QDoubleValidator *val = dynamic_cast<const QDoubleValidator*>(linetext->validator());
            if (val)
                linetext->setText(QLocale().toString(QLocale().toDouble(linetext->text()),'f',2));
        }
    }
    if(event->type() == QEvent::MouseMove)
    {
        if (obj == ui->textFrame || obj == ui->upTextEdit)
            if (gMode == Selection)
            {
                QRect rect = QRect(ui->textFrame->pos(),ui->textFrame->size());
                QPoint pos = mapFromParent(cursor().pos());
                if (rect.contains(pos) && ui->upTextEdit->toPlainText() != "")
                    EffaceWidget(ui->textFrame, false);
            }
    }
    if (event->type() == QEvent::KeyPress)
        if (obj->inherits("UpTableWidget"))
        {
            QKeyEvent       *keyEvent = static_cast<QKeyEvent*>(event);
            UpTableWidget   *table    = static_cast<UpTableWidget *>(obj);
            UpLineEdit      *line;
            if (keyEvent->key() == Qt::Key_Up)
            {
                for (int i=0; i<table->rowCount(); i++)
                {
                    line = static_cast<UpLineEdit *>(table->cellWidget(i,1));
                    if (line->selectedText() != "")
                    {
                        if (line->getRowTable() > 0){
                            LineSelect(table, line->getRowTable()-1);
                            QPoint posdebut = mapFrom(this, table->cellWidget(0,0)->pos());
                            QPoint poscell  = mapFrom(this, table->cellWidget(line->getRowTable(),0)->pos());
                            if (posdebut.y() < -1 && poscell.y() < table->height()-(line->height()*7))
                                table->viewport()->scroll(0,line->height());
                        }
                        break;
                    }
                }
            }
            if (keyEvent->key() == Qt::Key_Down)
            {
                for (int i=0; i<table->rowCount(); i++)
                {
                    line = static_cast<UpLineEdit *>(table->cellWidget(i,1));
                    if (line->selectedText() != "")
                    {
                        if (line->getRowTable() < table->rowCount()-1){
                            LineSelect(table, line->getRowTable()+1);
                            QPoint poscell = mapFrom(this, table->cellWidget(line->getRowTable(),0)->pos());
                            QPoint posfin = mapFrom(this, table->cellWidget(table->rowCount()-1,0)->pos());
                            if (posfin.y() > table->height()-(line->height()*2) && poscell.y() > line->height()*5)
                                table->viewport()->scroll(0,-line->height());
                        }
                        break;
                    }
                }
            }
        }
    return QWidget::eventFilter(obj, event);
}

void dlg_documents::keyPressEvent(QKeyEvent * event )
{
    switch (event->key()) {
    case Qt::Key_Escape:
    {
        Annulation();
        break;
    }
    default:
        break;
    }
}

int dlg_documents::AskDialog(QString titre)
{
    gAskDialog                  = new UpDialog(this);
    gAskDialog                  ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
    QVBoxLayout *globallay      = dynamic_cast<QVBoxLayout*>(gAskDialog->layout());
    UpLineEdit  *Line           = new UpLineEdit(gAskDialog);
    UpLabel     *label          = new UpLabel(gAskDialog);

    globallay->setSpacing(4);
    globallay->insertWidget(0,Line);
    globallay->insertWidget(0,label);

    gAskDialog->setModal(true);
    gAskDialog->setSizeGripEnabled(false);
    gAskDialog->setFixedSize(270,100);
    gAskDialog->move(QPoint(x()+width()/2,y()+height()/2));
    gAskDialog->setWindowTitle(titre);
    gAskDialog->AjouteLayButtons();

    connect(gAskDialog->OKButton,   &QPushButton::clicked,   [=] {gAskDialog->accept();});

    label->setText(tr("Entrez la question que vous voulez poser."));
    Line->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    Line->setMaxLength(60);
    return gAskDialog->exec();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de document
// ----------------------------------------------------------------------------------
bool dlg_documents::ChercheDoublon(QString str, int row)
{
    QString req, nom;
    switch (gMode) {
    case CreationDOC:
    case ModificationDOC:
        req = "select resumedocument, iduser from " NOM_TABLE_COURRIERS " where iddocument <> " + ui->DocupTableWidget->item(row,3)->text();
        nom = tr("document");
        break;
    case CreationDOSS:
    case ModificationDOSS:
        req = "select resumemetadocument, iduser from " NOM_TABLE_METADOCUMENTS " where idmetadocument <> " + ui->DossiersupTableWidget->item(row,2)->text();
        nom = tr("dossier");
        break;
    default:
        return false;
    }
    bool a = false;
    QList<QList<QVariant>> listdocs;
    bool ok;
    listdocs = db->StandardSelectSQL(req,ok);
    if (listdocs.size() > 0)
    {
        for (int i=0; i<listdocs.size() ; i++)
        {
            if (listdocs.at(i).at(0).toString().toUpper() == str.toUpper())
            {
                a = true;
                QString b = "vous";
                if (listdocs.at(i).at(1).toInt() != gidUser)
                    b = Datas::I()->users->getUserById(listdocs.at(i).at(1).toInt())->getLogin();
                UpMessageBox::Watch(this,tr("Il existe déjà un") + " " + nom + " " + tr("portant ce nom créé par ") + b);
                break;
            }
        }
    }
    return a;
}

// ----------------------------------------------------------------------------------
// On a clique sur une ligne de dossiers . on coche-décoche les docs correspondants
// ----------------------------------------------------------------------------------
void dlg_documents::CocheLesDocs(int iddoss, bool A)
{
    bool ok;
    QString idDossier = QString::number(iddoss);
    QString req = "select idDocument from " NOM_TABLE_JOINTURESDOCS " where idMetaDocument = " + idDossier;
    QList<QList<QVariant>> listdocmts = db->StandardSelectSQL(req,ok);
    if (listdocmts.size() > 0)
    {
        QStringList listiddocs;
        for (int i=0; i<listdocmts.size(); i++)
            listiddocs << listdocmts.at(i).at(0).toString();
        for (int k=0; k<ui->DocupTableWidget->rowCount(); k++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(k,0));
            if (Widg)
            {
                UpCheckBox *DocCheck = Widg->findChildren<UpCheckBox*>().at(0);
                if (listiddocs.contains(ui->DocupTableWidget->item(k,3)->text()))
                {
                    if (A)
                        DocCheck->setChecked(A);
                    else                 // on vérifie qu'on peut décocher un doc et qu'il n'est pas nécessité par un autre dossier coché
                    {
                        bool a = false;
                        for (int j=0; j<ui->DossiersupTableWidget->rowCount(); j++)
                        {
                            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(j,0));
                            if (Widg)
                            {
                                UpCheckBox *DossCheck = Widg->findChildren<UpCheckBox*>().at(0);
                                if (ui->DossiersupTableWidget->item(j,2)->text().toInt() != iddoss)
                                {
                                    if (DossCheck->isChecked())
                                    {
                                        req = "select idDocument from " NOM_TABLE_JOINTURESDOCS
                                                " where idMetaDocument = " + ui->DossiersupTableWidget->item(j,2)->text();
                                        QList<QList<QVariant>> listdocmts2 = db->StandardSelectSQL(req,ok);
                                        if (listdocmts2.size() > 0)
                                        {
                                            QStringList listid;
                                            for (int i=0; i<listdocmts2.size(); i++)
                                                listid << listdocmts2.at(i).at(0).toString();
                                            if (listid.contains(ui->DocupTableWidget->item(k,3)->text()))
                                            {
                                                a = true;
                                                break;
                        }   }   }   }   }   }
                        DocCheck->setChecked(a);
                    }
                    QString nomdoc = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(DocCheck->getRowTable(),1))->text();
                    if (!DocCheck->isChecked())
                        ui->DocupTableWidget->item(DocCheck->getRowTable(),10)->setText("1" + nomdoc);
                    else
                         ui->DocupTableWidget->item(DocCheck->getRowTable(),10)->setText("0" + nomdoc);
                }

            }   }   }
    // tri de la table docuptablewidget
    TriDocupTableWidget();

    // enable okpushbutton
    for (int l=0; l<ui->DocupTableWidget->rowCount(); l++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(l,0));
        if (Widg)
        {
            UpCheckBox *DocCheck = Widg->findChildren<UpCheckBox*>().at(0);
            if (DocCheck->isChecked())
            {
                ui->OKupPushButton->setEnabled(true);
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------------
// Configuration de le fiche en fonction du mode de fonctionnement
// ----------------------------------------------------------------------------------
void dlg_documents::ConfigMode(int mode, int row)
{
    gMode = mode;
    ui->ChercheupLineEdit->setEnabled       (gMode == Selection);
    ui->dateEdit->setEnabled                (gMode == Selection);
    ui->ALDcheckBox->setVisible             (gMode == Selection);
    widgButtonsDossiers->setEnabled         (gMode == Selection);
    ui->DossiersupTableWidget->setEnabled   (gMode == Selection);
    ui->OKupPushButton->setEnabled          (false);
    ui->textFrame->setVisible               (gMode != CreationDOSS && gMode!= ModificationDOSS && gMode != Selection);

    if (gMode != Selection) {
        gTimerEfface->disconnect();
        ui->textFrame->setGraphicsEffect(new QGraphicsOpacityEffect());
    }
    else
    {
        gOp = new QGraphicsOpacityEffect();
        gOp->setOpacity(0.1);
        ui->textFrame->setGraphicsEffect(gOp);
    }

    if (mode == Selection)
    {
        EnableLines();
        widgButtonsDocs                 ->setEnabled(true);
        ui->DocPubliccheckBox           ->setChecked(false);
        ui->DocPubliccheckBox           ->setEnabled(false);
        ui->DocPubliccheckBox           ->setToolTip("");
        ui->DocupTableWidget            ->setEnabled(true);
        ui->DocupTableWidget            ->setFocus();
        ui->DocupTableWidget            ->setStyleSheet("");
        widgButtonsDossiers             ->setEnabled(true);
        ui->DossiersupTableWidget       ->setEnabled(true);
        ui->DocEditcheckBox             ->setChecked(false);
        ui->DocEditcheckBox             ->setEnabled(false);
        ui->DocEditcheckBox             ->setToolTip("");
        ui->DocAdministratifcheckBox    ->setChecked(false);
        ui->DocAdministratifcheckBox    ->setEnabled(false);
        ui->DocAdministratifcheckBox    ->setToolTip("");
        ui->Expliclabel                 ->setText(tr("SELECTION - Cochez les dossiers ou les documents que vous voulez imprimer"));
        widgButtonsDocs->modifBouton    ->setEnabled(false);
        widgButtonsDossiers->modifBouton->setEnabled(false);
        ui->PrescriptioncheckBox        ->setEnabled(false);
        widgButtonsDocs->moinsBouton    ->setEnabled(false);
        widgButtonsDossiers->moinsBouton->setEnabled(false);
        ui->textFrame                   ->setStyleSheet("");
        ui->textFrame                   ->setEnabled(true);
        ui->upTextEdit                  ->clear();
        ui->upTextEdit                  ->setFocusPolicy(Qt::NoFocus);
        ui->upTextEdit                  ->setStyleSheet("");

        ui->AnnulupPushButton           ->setIcon(Icons::icAnnuler());
        ui->AnnulupPushButton           ->setToolTip(tr("Annuler et fermer la fiche"));
        ui->OKupPushButton              ->setText(tr("Imprimer\nla sélection"));
        ui->OKupPushButton              ->setIcon(Icons::icImprimer());
        ui->OKupPushButton              ->setIconSize(QSize(30,30));

        int nbCheck = 0;
        for (int i =0 ; i < ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(true);
                if (Check->isChecked()) nbCheck ++;
            }
        }
        ui->OKupPushButton->setEnabled(nbCheck>0);
    }

    else if (mode == ModificationDOC)
    {
        DisableLines();
        UpLineEdit *line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(row,1));
        line->setEnabled(true);
        line->setFocusPolicy(Qt::WheelFocus);
        line->setFocus();
        line->selectAll();
        connect(line,   &QLineEdit::textEdited, [=] {EnableOKPushButton();});

        ui->upTextEdit->setText(ui->DocupTableWidget->item(row,2)->text());

        ui->DocPubliccheckBox->setEnabled(VerifDocumentPublic(row,false));
        ui->DocPubliccheckBox->setToolTip(tr("Cochez cette case si vous souhaitez\nque ce document soit visible par tous les utilisateurs"));
        ui->DocupTableWidget->setEnabled(true);
        ui->DocupTableWidget->setStyleSheet("");
        widgButtonsDocs->setEnabled(false);
        ui->DossiersupTableWidget->setEnabled(false);
        widgButtonsDossiers->setEnabled(false);
        ui->textFrame->setEnabled(true);
        ui->Expliclabel->setText(tr("DOCUMENTS - MODIFICATION"));
        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));
        ui->DocAdministratifcheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setToolTip(tr("si cette option est cochée\nle document est considéré comme purement administratif"));
        ui->PrescriptioncheckBox    ->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");
        if (!db->getUserConnected()->isMedecin() && !db->getUserConnected()->isOrthoptist())
        {
            ui->PrescriptioncheckBox->setChecked(false);
            ui->DocAdministratifcheckBox->setChecked(true);
        }
        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
        ui->OKupPushButton->setText(tr("Enregistrer"));
    }

    else if (mode == ModificationDOSS)
    {
        DisableLines();
        UpLineEdit *line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(row,1));
        line->setEnabled(true);
        line->setFocusPolicy(Qt::WheelFocus);
        line->setFocus();
        line->selectAll();
        connect(line,   &QLineEdit::textEdited, [=] {EnableOKPushButton();});
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(true);
                Check->setChecked(false);
            }
        }
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(false);
                Check->setChecked(i==row);
            }
        }
        int iddossier = ui->DossiersupTableWidget->item(row,2)->text().toInt();
        CocheLesDocs(iddossier, true);

        ui->DocupTableWidget->setEnabled(true);
        ui->DocupTableWidget->setStyleSheet("UpTableWidget {border: 2px solid rgb(251, 51, 61);}");
        ui->DossiersupTableWidget->setEnabled(true);
        ui->Expliclabel->setText(tr("DOSSIERS - MODIFICATION"));

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
        ui->OKupPushButton->setText(tr("Enregistrer"));
    }
    else if (mode == CreationDOC)
    {
        DisableLines();
        if (ui->DocupTableWidget->rowCount() > 0)
            DisableLines();
        ui->DocupTableWidget->insertRow(row);
        QWidget * w = new QWidget(ui->DocupTableWidget);
        UpCheckBox *Check = new UpCheckBox(w);
        Check->setCheckState(Qt::Unchecked);
        Check->setRowTable(row);
        Check->setFocusPolicy(Qt::NoFocus);
        Check->setEnabled(false);
        QHBoxLayout *l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Check);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);
        ui->DocupTableWidget->setCellWidget(row,0,w);
        UpLineEdit *upLine0 = new UpLineEdit;
        upLine0->setText(tr("Nouveau document"));                          // resume
        upLine0->setRowTable(row);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::WheelFocus);
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        upLine0->selectAll();
        ui->DocupTableWidget->setCellWidget(row,1,upLine0);
        QTableWidgetItem    *pItem1 = new QTableWidgetItem;
        QTableWidgetItem    *pItem2 = new QTableWidgetItem;
        QTableWidgetItem    *pItem3 = new QTableWidgetItem;
        QTableWidgetItem    *pItem4 = new QTableWidgetItem;
        QTableWidgetItem    *pItem5 = new QTableWidgetItem;
        QTableWidgetItem    *pItem6 = new QTableWidgetItem;
        QTableWidgetItem    *pItem10= new QTableWidgetItem;
        QTableWidgetItem    *pItem11= new QTableWidgetItem;
        int col = 2;
        pItem1->setText("");                           // text
        ui->DocupTableWidget->setItem(row,col,pItem1);
        col++; //3
        pItem2->setText("0");                           // idDocument
        ui->DocupTableWidget->setItem(row,col,pItem2);
        col++; //4
        pItem3->setText("");                           // DocPublic
        ui->DocupTableWidget->setItem(row,col,pItem3);
        col++; //5
        pItem4->setText("");                           // idUser
        ui->DocupTableWidget->setItem(row,col,pItem4);
        col++; //6
        pItem5->setText("");                           // Prescription
        ui->DocupTableWidget->setItem(row,col,pItem5);
        col++; //7
        UpLabel*lbl = new UpLabel(ui->DocupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        ui->DocupTableWidget->setCellWidget(row,col,lbl);
        col++; //8
        pItem6->setText("");                           // Editable
        ui->DocupTableWidget->setItem(row,col,pItem6);
        col++; //9
        UpLabel*lbl1 = new UpLabel(ui->DocupTableWidget);
        lbl1->setAlignment(Qt::AlignCenter);
        ui->DocupTableWidget->setCellWidget(row,col,lbl1);
        connect(upLine0,   &QLineEdit::textEdited, [=] {EnableOKPushButton();});
        col++; //10
        pItem10->setText("");                           // Check+text   -> sert pour le tri de la table
        ui->DocupTableWidget->setItem(row,col,pItem10);
        col++; //11
        pItem11->setText("");                           // Medical      0 = doc administratif 1 = doc médical
        ui->DocupTableWidget->setItem(row,col,pItem11);
        col++; //12
        UpLabel*lbl12 = new UpLabel(ui->DocupTableWidget);
        lbl12->setAlignment(Qt::AlignCenter);
        ui->DocupTableWidget->setCellWidget(row,col,lbl12);
        ui->DocupTableWidget->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));

        ui->DocPubliccheckBox->setChecked(false);
        ui->DocPubliccheckBox->setEnabled(true);
        ui->DocPubliccheckBox->setToolTip(tr("Cochez cette case si vous souhaitez\nque ce document soit visible par tous les utilisateurs"));
        ui->DossiersupTableWidget->setEnabled(false);
        widgButtonsDossiers->setEnabled(false);
        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setChecked(false);
        ui->DocEditcheckBox->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));
        ui->Expliclabel->setText(tr("DOCUMENTS - CREATION - Remplissez les champs définissant le document que vous voulez créer"));
        ui->PrescriptioncheckBox->setChecked(Qt::Unchecked);
        ui->PrescriptioncheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setChecked(false);
        ui->DocAdministratifcheckBox->setToolTip(tr("si cette option est cochée\nle document est considéré comme purement administratif"));
        widgButtonsDocs->moinsBouton->setEnabled(false);
        ui->upTextEdit->clear();
        ui->upTextEdit->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");
        if (!db->getUserConnected()->isMedecin() && !db->getUserConnected()->isOrthoptist())
        {
            ui->PrescriptioncheckBox->setChecked(false);
            ui->DocAdministratifcheckBox->setChecked(true);
        }

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setEnabled(true);
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setText(tr("Enregistrer\nle document"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
        upLine0->setFocus();
    }

    else if (mode == CreationDOSS)
    {
        DisableLines();
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(true);
                Check->setChecked(false);
            }
            UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line0) {
                line0->deselect();
                line0->setEnabled(false);
            }
        }
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(false);
                Check->setChecked(false);
            }
        }
        ui->DossiersupTableWidget->insertRow(row);
        QWidget * w = new QWidget(ui->DocupTableWidget);
        UpCheckBox *Check = new UpCheckBox(w);
        Check->setCheckState(Qt::Checked);
        Check->setRowTable(row);
        Check->setFocusPolicy(Qt::NoFocus);
        Check->setEnabled(false);
        QHBoxLayout *l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Check);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);
        ui->DossiersupTableWidget->setCellWidget(row,0,w);
        UpLineEdit *upLine0 = new UpLineEdit;
        upLine0->setText(tr("Nouveau dossier"));                          // resume
        upLine0->setRowTable(row);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::WheelFocus);
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        upLine0->setFocus();
        upLine0->selectAll();
        ui->DossiersupTableWidget->setCellWidget(row,1,upLine0);
        QTableWidgetItem    *pItem1 = new QTableWidgetItem;
        QTableWidgetItem    *pItem2 = new QTableWidgetItem;
        int col = 2;
        pItem1->setText("0");                           // idMetaDocument
        ui->DossiersupTableWidget->setItem(row,col,pItem1);
        col++; //3
        pItem2->setText("");                           // idUser
        ui->DossiersupTableWidget->setItem(row,col,pItem2);
        col++; //4
        UpLabel*lbl = new UpLabel(ui->DossiersupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        ui->DossiersupTableWidget->setCellWidget(row,col,lbl);
        connect(upLine0,   &QLineEdit::textEdited, [=] {EnableOKPushButton();});

        ui->DossiersupTableWidget->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));

        widgButtonsDocs->setEnabled(false);
        ui->DocupTableWidget->setEnabled(true);
        ui->DocupTableWidget->setStyleSheet("UpTableWidget {border: 2px solid rgb(251, 51, 61);}");
        ui->DossiersupTableWidget->setEnabled(true);
        widgButtonsDossiers->setEnabled(false);
        ui->Expliclabel->setText(tr("DOSSIER - CREATION - Cochez les cases correspondants au dossier que vous voulez créer"));

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setEnabled(true);
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setText(tr("Enregistrer\nle document"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
    }
    if (!db->getUserConnected()->isMedecin() && !db->getUserConnected()->isOrthoptist())
    {
        ui->PrescriptioncheckBox->setVisible(false);
        ui->DocAdministratifcheckBox->setVisible(false);
    }
}

// --------------------------------------------------------------------------------------------------
// SetEnabled = false et disconnect toutes les lignes des UpTableWidget - SetEnabled = false checkBox
// --------------------------------------------------------------------------------------------------
void dlg_documents::DisableLines()
{
    widgButtonsDossiers->setEnabled(false);
    widgButtonsDocs->setEnabled(false);
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(false);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line0) {
            line0->deselect();
            line0->setEnabled(false);
            line0->setFocusPolicy(Qt::NoFocus);
            line0 ->disconnect();
        }
    }
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(false);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (line0) {
            line0->deselect();
            line0->setEnabled(false);
            line0->setFocusPolicy(Qt::NoFocus);
            line0->disconnect();
        }
    }
}

// -------------------------------------------------------------------------------------------
// SetEnabled = true, connect toutes les lignes des UpTableWidget - SetEnabled = true checkBox
// -------------------------------------------------------------------------------------------
void dlg_documents::EnableLines()
{
    widgButtonsDossiers->setEnabled(true);
    widgButtonsDocs->setEnabled(true);
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(true);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line0) {
            line0->deselect();
            line0->setEnabled(true);
            line0->setFocusPolicy(Qt::NoFocus);
            if (ui->DocupTableWidget->item(i,5)->text().toInt() == gidUser)
            {
                connect(line0,          &UpLineEdit::mouseDoubleClick,          [=] {DocCellDblClick(line0);});
                connect(line0,          &QWidget::customContextMenuRequested,   [=] {MenuContextuel(line0);});
                connect(line0,          &QLineEdit::textEdited,                 [=] {EnableOKPushButton();});
            }
            connect(line0,              &UpLineEdit::mouseEnter,                [=] {DocCellEnter(line0);});
            connect(line0,              &UpLineEdit::mouseRelease,              [=] {LineSelect(ui->DocupTableWidget, line0->getRowTable());
});
        }
    }
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(true);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (line0) {
            line0->deselect();
            line0->setEnabled(true);
            line0->setFocusPolicy(Qt::NoFocus);
            if (ui->DossiersupTableWidget->item(i,3)->text().toInt() == gidUser)
            {
                connect(line0,          &UpLineEdit::mouseDoubleClick,          [=] {DocCellDblClick(line0);});
                connect(line0,          &QWidget::customContextMenuRequested,   [=] {MenuContextuel(line0);});
                connect(line0,          &QLineEdit::textEdited,                 [=] {EnableOKPushButton();});
            }
            connect(line0,              &UpLineEdit::mouseEnter,                [=] {DocCellEnter(line0);});
            connect(line0,              &UpLineEdit::mouseRelease,              [=] {LineSelect(ui->DossiersupTableWidget, line0->getRowTable());
;});
        }
    }
}

// ----------------------------------------------------------------------------------
// Effacement progressif d'un textEdit.
// ----------------------------------------------------------------------------------
void dlg_documents::EffaceWidget(QWidget* widg, bool AvecOuSansPause)
{
    QTime DebutTimer     = QTime::currentTime();
    gOpacity = 1;
    widg->setVisible(true);
    widg->setAutoFillBackground(true);
    gTimerEfface->disconnect();
    gTimerEfface->start(70);
    connect(gTimerEfface, &QTimer::timeout, [=]
    {
        QRect rect = QRect(widg->pos(),widg->size());
        QPoint pos = mapFromParent(cursor().pos());
        int Pause = (AvecOuSansPause? 4000: 0);
        if (DebutTimer.msecsTo(QTime::currentTime()) > Pause  && !rect.contains(pos))
        {
            gOpacity = gOpacity*0.9;
            gOp->setOpacity(gOpacity);
            widg->setGraphicsEffect(gOp);
            if (gOpacity < 0.10)
                gTimerEfface->disconnect();
        }
        else
        {
            gOp->setOpacity(1);
            widg->setGraphicsEffect(gOp);
        }
    });
}

// ----------------------------------------------------------------------------------
// Creation du Document dans la base.
// ----------------------------------------------------------------------------------
void dlg_documents::InsertDocument(int row)
{
    // controle validate des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));
    if (line->text().length() < 1)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Creation de document"), tr("Veuillez renseigner le champ Résumé, SVP !"));
        return;
    }
    if (line->text() == tr("Nouveau document"))
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Creation de document"), tr("Votre document ne peut pas s'appeler \"Nouveau document\""));
        return;
    }
    if (ui->upTextEdit->document()->toPlainText().length() < 1)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Creation de document"), tr("Veuillez renseigner le champ Document, SVP !"));
        return;
    }
    // Creation du Document dans la table
    if (ChercheDoublon(line->text(), row))
    {
        line->setFocus();
        line->selectAll();
        return;
    }

    QString requete = "INSERT INTO " NOM_TABLE_COURRIERS
            " (TextDocument, ResumeDocument, idUser, DocPublic, Prescription, Editable, Medical) "
            " VALUES ('" + Utils::correctquoteSQL(ui->upTextEdit->document()->toHtml()) +
            "', '" + Utils::correctquoteSQL(line->text().left(100)) +
            "', " + QString::number(gidUser);
    QString Public          = (ui->DocPubliccheckBox->isChecked()?          "1" : "null");
    QString Prescription    = (ui->PrescriptioncheckBox->isChecked()?       "1" : "null");
    QString Editable        = (ui->DocEditcheckBox->isChecked()?            "1" : "null");
    QString Admin           = (ui->DocAdministratifcheckBox->isChecked()?   "0" : "1");
    requete += ", " + Public;
    requete += ", " + Prescription;
    requete += ", " + Editable;
    requete += ", " + Admin;
    requete += ")";
    db->StandardSQL(requete,tr("Erreur d'enregistrement du document dans ") + NOM_TABLE_COURRIERS);

    Remplir_TableWidget();

    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        QString resume = line->text();
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
             line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
             if (line->text() == resume)
             {
                LineSelect(ui->DocupTableWidget,line->getRowTable());
                QModelIndex index = ui->DocupTableWidget->model()->index(line->getRowTable(),1);
                ui->DocupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                break;
             }
         }
    }
}

// ----------------------------------------------------------------------------------
// Creation du Dossier dans la base.
// ----------------------------------------------------------------------------------
void dlg_documents::InsertDossier(int row)
{
    // controle validité des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));
    if (line->text().length() < 1)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Creation de dossier"), tr("Veuillez renseigner le champ Résumé, SVP !"));
        return;
    }

    // Creation du Dossier dans la table
    if (ChercheDoublon(line->text(), row))
    {
        line->setFocus();
        line->selectAll();
        return;
    }

    QString requete = "INSERT INTO " NOM_TABLE_METADOCUMENTS
            " (ResumeMetaDocument, idUser, Public) "
            " VALUES ('" + Utils::correctquoteSQL(line->text().left(100)) +
            "'," + QString::number(gidUser);
    UpLabel *lbl = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(row,4));
    QString a = "null";
    if (lbl->pixmap())
        a = "1";
    requete += "," + a + ")";
    if (db->StandardSQL(requete, tr("Erreur d'enregistrement du dossier dans ") +  NOM_TABLE_METADOCUMENTS))
    {
        QStringList listdocs;
        QString idmetadoc;
        requete = "select idmetadocument from " NOM_TABLE_METADOCUMENTS " where ResumeMetadocument = '" + Utils::correctquoteSQL(line->text().left(100)) + "'";
        bool ok;
        QList<QList<QVariant>> listdocmts = db->StandardSelectSQL(requete,ok);
        if (listdocmts.size()>0)
        {
            idmetadoc = listdocmts.at(0).at(0).toString();
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                QWidget * w = static_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
                QHBoxLayout* boxlay = w->findChildren<QHBoxLayout*>().at(0);
                UpCheckBox* Check = static_cast<UpCheckBox*>(boxlay->itemAt(0)->widget());
                if (Check->isChecked())
                    listdocs << ui->DocupTableWidget->item(i,3)->text();
            }
            if (listdocs.size()>0)
            {
                requete = "insert into " NOM_TABLE_JOINTURESDOCS " (idMetadocument, iddocument) VALUES ";
                for (int k=0; k<listdocs.size(); k++)
                {
                    requete += "(" + idmetadoc + ", " + listdocs.at(k) + ")";
                    if (k<listdocs.size()-1)    requete += ",";
                }
                //UpMessageBox::Watch(this,requete);
                db->StandardSQL(requete);
            }
        }
    }
    Remplir_TableWidget();

    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        QString resume = line->text();
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
             line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(i,1));
             if (line->text() == resume)
             {
                LineSelect(ui->DossiersupTableWidget,line->getRowTable());
                QModelIndex index = ui->DossiersupTableWidget->model()->index(line->getRowTable(),1);
                ui->DocupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                break;
             }
         }
    }
}

// ----------------------------------------------------------------------------------
// On sélectionne une ligne. On affiche le détail et on met en édition
// ----------------------------------------------------------------------------------
void dlg_documents::LineSelect(UpTableWidget *table, int row)
{
    if (table->rowCount() == 0) return;
    if (row < 0) row = 0;
    if (row > table->rowCount()-1) row = table->rowCount()-1;

    UpLineEdit *line        = dynamic_cast<UpLineEdit*>(table->cellWidget(row,1));
    if (!line) return;

    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line0) line0->deselect();
    }
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (line0) line0->deselect();
    }
    if (table == ui->DocupTableWidget)
    {
        widgButtonsDocs->modifBouton->setEnabled(ui->DocupTableWidget->item(row,5)->text().toInt() == gidUser);
        widgButtonsDocs->moinsBouton->setEnabled(ui->DocupTableWidget->item(row,5)->text().toInt() == gidUser);
        widgButtonsDossiers->modifBouton->setEnabled(false);
        widgButtonsDossiers->moinsBouton->setEnabled(false);
        if (gMode == Selection)
        {
            ui->textFrame->setVisible(true);
            MetAJour(ui->DocupTableWidget->item(row,2)->text(), false);
            ui->upTextEdit->setText(glisttxt.at(0));
            EffaceWidget(ui->textFrame);
            ui->DocPubliccheckBox->setChecked(ui->DocupTableWidget->item(row,4)->text().toInt() == 1);
            ui->DocEditcheckBox->setChecked(ui->DocupTableWidget->item(row,8)->text().toInt() == 1);
            ui->PrescriptioncheckBox->setChecked(ui->DocupTableWidget->item(row,6)->text().toInt() == 1);
            ui->DocAdministratifcheckBox->setChecked(ui->DocupTableWidget->item(row,11)->text().toInt() != 1);
        }
    }
    else if (table == ui->DossiersupTableWidget)
    {
        ui->textFrame->setVisible(false);
        widgButtonsDocs->modifBouton->setEnabled(false);
        widgButtonsDossiers->modifBouton->setEnabled(ui->DossiersupTableWidget->item(row,3)->text().toInt() == gidUser);
        widgButtonsDocs->moinsBouton->setEnabled(false);
        widgButtonsDossiers->moinsBouton->setEnabled(ui->DossiersupTableWidget->item(row,3)->text().toInt() == gidUser);
    }
    line->selectAll();
}

// ----------------------------------------------------------------------------------
// Met à jour les champs du texte à afficher
// ----------------------------------------------------------------------------------
void dlg_documents::MetAJour(QString texte, bool pourVisu)
{
    glistidCor.clear();
    glisttxt.clear();

    int idusr = (proc->UserSuperviseur()<1? Datas::I()->users->superviseurs()->first()->id() : proc->UserSuperviseur());
    User *userEntete = Datas::I()->users->getUserById(idusr, true);
    if (userEntete == Q_NULLPTR)
        return;

    QString req = "select patDDN, Sexe "
                  " from " NOM_TABLE_PATIENTS
                  " where idPat = " + QString::number(gidPatient);
    bool ok;
    QList<QList<QVariant>> listpat = db->StandardSelectSQL(req,ok,tr("Impossible de retrouver la date de naissance de ce patient"));
    if (!ok)
        texte.replace("{{DDN}}"                 ,"xx xx xxxx");
    QString Sexe                        = listpat.at(0).at(1).toString();
    QDate ddn                           = listpat.at(0).at(0).toDate();
    QMap<QString,QVariant>  AgeTotal    = Item::CalculAge(ddn, Sexe);
    QString age                         = AgeTotal["toString"].toString();
    QString formule                     = AgeTotal["formule"].toString();
    req = "select idcormedmg, cornom, corprenom, corsexe "
          " from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp, " NOM_TABLE_CORRESPONDANTS " cor "
          " where idPat = " + QString::number(gidPatient) + " and rmp.idcormedmg = cor.idcor";
    QList<QList<QVariant>> listdatapat = db->StandardSelectSQL(req,ok);

    texte.replace("{{" + DATEDOC + "}}"         , QDate::currentDate().toString(tr("d MMMM yyyy")));
    texte.replace("{{" + NOMPAT + "}},"         , gNomPat + ",");
    texte.replace("{{" + NOMPAT + "}} "         , gNomPat + " ");
    texte.replace("{{" + NOMPAT + "}}"          , gNomPat);
    texte.replace("{{" + PRENOMPAT + "}},"      , gPrenomPat + ",");
    texte.replace("{{" + PRENOMPAT + "}} "      , gPrenomPat + " ");
    texte.replace("{{" + PRENOMPAT + "}}"       , gPrenomPat);
    if (userEntete->getTitre().size())
        texte.replace("{{" + TITRUSER + "}}"    , userEntete->getTitre() + " " + userEntete->getPrenom() + " " + userEntete->getNom());
    else
        texte.replace("{{" + TITRUSER + "}}"    , userEntete->getPrenom() + " " + userEntete->getNom());
    texte.replace("{{" + DDNPAT + "}}"          , listpat.at(0).at(0).toDate().toString((tr("d MMMM yyyy"))));
    texte.replace("{{" + TITREPAT + "}} "       , formule + " ");
    texte.replace("{{" + TITREPAT + "}}"        , formule);
    texte.replace("{{" + AGEPAT + "}}"          , age);
    if (Sexe == "F")    texte.replace("(e)" ,"e");
    if (Sexe == "M")    texte.replace("(e)" ,"");

    if (listdatapat.size() == 0) {
        texte.replace("{{" + MGPAT + "}},"      ,"");
        texte.replace("{{" + MGPAT + "}} "      ,"");
        texte.replace("{{" + MGPAT + "}}"       ,"");
        texte.replace("{{" + POLITESSEMG + "}}" ,tr("Ma chère consoeur, mon cher confrère"));
        texte.replace("{{" + NOMMG + "}}},"     ,"");
        texte.replace("{{" + PRENOMMG + "}},"   ,"");
        texte.replace("{{" + NOMMG + "}}} "     ,"");
        texte.replace("{{" + PRENOMMG + "}} "   ,"");
        texte.replace("{{" + NOMMG + "}}}"      ,"");
        texte.replace("{{" + PRENOMMG + "}}"    ,"");
    }
    else {
        QString form = "", form2 = "";
        if (listdatapat.at(0).at(3).toString() == "F")
        {
            form = tr("Madame le docteur ");
            form2 = tr("Ma chère consoeur");
        }
        else if (listdatapat.at(0).at(3).toString() == "M")
        {
            form = tr("Monsieur le docteur ");
            form2 = tr("Mon cher confrère");
        }
        else
            form2 = tr("Ma chère consoeur, mon cher confrère");
        form += listdatapat.at(0).at(2).toString() + " " + listdatapat.at(0).at(1).toString();
        texte.replace("{{" + MGPAT + "}}"               ,form);
        texte.replace("{{" + POLITESSEMG + "}}"         ,form2);
        texte.replace("{{" + NOMMG + "}}}"              ,listdatapat.at(0).at(1).toString());
        texte.replace("{{" + PRENOMMG + "}}"            ,listdatapat.at(0).at(2).toString());
    }
    if (texte.contains("{{" + KERATO + "}}"))
    {
        req = "select K1OD, K2OD, AxeKOD, DioptrieK1OD, DioptrieK2OD, DioptrieKOD, K1OG, K2OG, AxeKOG, DioptrieK1OG, DioptrieK2OG, DioptrieKOG from " NOM_TABLE_DONNEES_OPHTA_PATIENTS
              " where idpat = " + QString::number(gidPatient) + " and (K1OD <> 'null' or K1OG <> 'null')";
        QList<QList<QVariant>> listker = db->StandardSelectSQL(req,ok);
        if (listker.size()>0)
        {
            QList<QVariant> ker = listker.last();
            QString kerato = "";
            if (ker.at(0).toDouble()>0)
            {
                if (ker.at(3).toDouble()!=0.0)
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOD:") + "</b></font> " + QString::number(ker.at(0).toDouble(),'f',2) + "/" + QString::number(ker.at(1).toDouble(),'f',2) + " Km = " + QString::number((ker.at(0).toDouble() + ker.at(1).toDouble())/2,'f',2) +
                              " - " + QString::number(ker.at(3).toDouble(),'f',2) + "/" + QString::number(ker.at(4).toDouble(),'f',2) + " " + QString::number(ker.at(5).toDouble(),'f',2) +  " à " + ker.at(2).toString() + "°</td></p>";
                else
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOD:") + "</b></font> " + QString::number(ker.at(0).toDouble(),'f',2) + " à " + ker.at(2).toString() + "°/" + QString::number(ker.at(1).toDouble(),'f',2) + " Km = " + QString::number((ker.at(0).toDouble() + ker.at(1).toDouble())/2,'f',2) ;
            }
            if (ker.at(0).toDouble()>0 && ker.at(6).toDouble()>0)
                kerato += "<br/>";
            if (ker.at(6).toDouble()>0.0)
            {
                if (ker.at(9).toDouble()!=0.0)
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOG:") + "</b></font> " + QString::number(ker.at(6).toDouble(),'f',2) + "/" +QString::number( ker.at(7).toDouble(),'f',2) + " Km = " + QString::number((ker.at(6).toDouble() + ker.at(7).toDouble())/2,'f',2) +
                            " - " + QString::number(ker.at(9).toDouble(),'f',2) + "/" + QString::number(ker.at(10).toDouble(),'f',2) + " " + QString::number(ker.at(11).toDouble(),'f',2) +  " à " + ker.at(8).toString() + "°</td></p>";
                else
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOG:") + "</b></font> " + QString::number(ker.at(6).toDouble(),'f',2) + " à " + ker.at(8).toString() + "°/" + QString::number(ker.at(7).toDouble(),'f',2) + " Km = " + QString::number((ker.at(6).toDouble() + ker.at(7).toDouble())/2,'f',2) ;
            }
            texte.replace("{{" + KERATO + "}}",kerato);
        }
        else
            texte.replace("{{" + KERATO + "}}",tr("pas de données de keratométrie retrouvées"));
    }
    if (texte.contains("{{" + REFRACT + "}}"))
    {
        req = "select FormuleOD, FormuleOG from " NOM_TABLE_REFRACTION
              " where idpat = " + QString::number(gidPatient) + " and (FormuleOD <> 'null' or FormuleOG <> 'null') and QuelleMesure = 'R'";
        QList<QList<QVariant>> listref = db->StandardSelectSQL(req,ok);
        if (listref.size()>0)
        {
            QList<QVariant> ref = listref.last();
            QString refract = "";
            if (ref.at(0).toString() != "")
                refract += "<font color = " + proc->CouleurTitres + "><b>" + tr("OD:") + "</b></font> " + ref.at(0).toString();
            if (ref.at(0).toString() != ""&& ref.at(1).toString() != "")
                refract += "<br />";
            if (ref.at(1).toString() != "")
                refract += "<font color = " + proc->CouleurTitres + "><b>" + tr("OG:") + "</b></font> " + ref.at(1).toString();
            texte.replace("{{" + REFRACT + "}}",refract);
        }
        else
            texte.replace("{{" + REFRACT + "}}",tr("pas de données de refraction retrouvées"));
    }
    int pos = 0;
    QRegExp reg;
    reg.setPattern("([{][{].*CORRESPONDANT.*[}][}])");
    if (reg.indexIn(texte, pos) != -1)
    {
        req = "select idcormedmg, cornom, corprenom, corsexe, CorSpecialite, CorMedecin, CorAutreProfession from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp," NOM_TABLE_CORRESPONDANTS " cor"
              " where idPat = " + QString::number(gidPatient) + " and rmp.idcormedmg = cor.idcor"
              " union "
              "select idcormedspe1, cornom, corprenom, corsexe, CorSpecialite, CorMedecin, CorAutreProfession from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp," NOM_TABLE_CORRESPONDANTS " cor"
              " where idPat = " + QString::number(gidPatient) + " and rmp.idcormedspe1 = cor.idcor"
              " union "
              "select idcormedspe2, cornom, corprenom, corsexe, CorSpecialite, CorMedecin, CorAutreProfession from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp," NOM_TABLE_CORRESPONDANTS " cor"
              " where idPat = " + QString::number(gidPatient) + " and rmp.idcormedspe2 = cor.idcor";
        //qDebug() << req;
        QList<QList<QVariant>> listcor = db->StandardSelectSQL(req,ok);
        if (listcor.size()==0)
            texte.replace(reg,tr("PAS DE CORRESPONDANT RÉFÉRENCÉ POUR CE PATIENT"));
        else if (listcor.size()==1)
        {
            QString form = "", form2 = "";
            if (listcor.at(0).at(3).toString() == "F")
            {
                if (listcor.at(0).at(5).toInt() == 1)
                {
                    form = tr("Madame le docteur ");
                    form2 = tr("Ma chère consoeur");
                }
                else
                {
                    form = tr("Madame ");
                    form2 = tr("Madame");
                }
            }
            else if (listcor.at(0).at(3).toString() == "M")
            {
                if (listcor.at(0).at(5).toInt() == 1)
                {
                    form = tr("Monsieur le docteur ");
                    form2 = tr("Mon cher confrère");
                }
                else
                {
                    form = tr("Monsieur ");
                    form2 = tr("Monsieur");
                }
            }
            else
                form2 = tr("Madame, Monsieur");

            form += listcor.at(0).at(2).toString() + " " + listcor.at(0).at(1).toString();
            texte.replace("{{" + CORPAT + "}}"             ,form);
            texte.replace("{{" + POLITESSECOR + "}}"   ,form2);
            texte.replace("{{" + NOMCOR + "}}}"                ,listcor.at(0).at(1).toString());
            texte.replace("{{" + PRENOMCOR + "}}"              ,listcor.at(0).at(2).toString());
        }
        else if (!pourVisu)
        {
            texte.replace("{{" + CORPAT + "}}"         ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
            texte.replace("{{" + POLITESSECOR + "}}"   ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
            texte.replace("{{" + NOMCOR + "}}}"        ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
            texte.replace("{{" + PRENOMCOR + "}}"      ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
        }
        else
        {
            ChoixCorrespondant(listcor);
            if (glistidCor.size()>0)
            {
                //qDebug() << QString::number(glistidCor.size()) + " correspondants sélectionnés";
                req = "select idcor, cornom, corprenom, corsexe, CorSpecialite, CorMedecin, CorAutreProfession from " NOM_TABLE_CORRESPONDANTS
                        " where idcor in (";
                for (int i=0; i<glistidCor.size(); i++)
                {
                    if (i<glistidCor.size()-1)
                        req += QString::number(glistidCor.at(i)) + ", ";
                    else
                        req += QString::number(glistidCor.at(i)) + ")";
                }
                //qDebug() << req;
                QList<QList<QVariant>> listtxt = db->StandardSelectSQL(req,ok);
                for (int j=0; j<listtxt.size(); j++)
                {
                    QString txtdef = texte;
                    QString form = "", form2 = "";
                    if (listtxt.at(j).at(3).toString() == "F")
                    {
                        if (listtxt.at(j).at(5).toInt() == 1)
                        {
                            form = tr("Madame le docteur ");
                            form2 = tr("Ma chère consoeur");
                        }
                        else
                        {
                            form = tr("Madame ");
                            form2 = tr("Madame");
                        }
                    }
                    else if (listtxt.at(j).at(3).toString() == "M")
                    {
                        if (listtxt.at(j).at(5).toInt() == 1)
                        {
                            form = tr("Monsieur le docteur ");
                            form2 = tr("Mon cher confrère");
                        }
                        else
                        {
                            form = tr("Monsieur ");
                            form2 = tr("Monsieur");
                        }
                    }
                    else
                        form2 = tr("Madame, Monsieur");

                    form += listtxt.at(j).at(2).toString() + " " + listtxt.at(j).at(1).toString();
                    txtdef.replace("{{" + CORPAT + "}}"             ,form);
                    txtdef.replace("{{" + POLITESSECOR + "}}"   ,form2);
                    txtdef.replace("{{" + NOMCOR + "}}}"                ,listtxt.at(j).at(1).toString());
                    txtdef.replace("{{" + PRENOMCOR + "}}"              ,listtxt.at(j).at(2).toString());
                    glisttxt << txtdef;
                }
            }
        }
    }
    if (glistidCor.size() == 0)
        glisttxt << texte;
}

void dlg_documents::ChoixCorrespondant(QList<QList<QVariant>> listcor)
{
    glistidCor.clear();
    gAskCorresp                 = new UpDialog(this);
    gAskCorresp                 ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
    gAskCorresp                 ->AjouteLayButtons();
    QVBoxLayout *globallay      = dynamic_cast<QVBoxLayout*>(gAskCorresp->layout());
    QTableView  *tblCorresp     = new QTableView(gAskCorresp);
    QStandardItemModel *gmodele = new QStandardItemModel;
    QStandardItem *pitem;
    UpLabel     *label          = new UpLabel(gAskCorresp);
    QFontMetrics fm             = QFontMetrics(qApp->font());
    int largeurcolonne          = 0;
    const QString lbltxt        = tr("À qui adresser ce courrier?");
    int largfinal               = fm.width(lbltxt);
    int hauteurligne            = int(fm.height()*1.1);

    label       ->setText(lbltxt);
    label       ->setAlignment(Qt::AlignCenter);

    tblCorresp  ->verticalHeader()->setVisible(false);
    tblCorresp  ->horizontalHeader()->setVisible(false);
    tblCorresp  ->setFocusPolicy(Qt::NoFocus);
    tblCorresp  ->setSelectionMode(QAbstractItemView::NoSelection);
    tblCorresp  ->setGridStyle(Qt::DotLine);
    tblCorresp  ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i=0; i<listcor.size(); i++)
    {
        pitem       = new QStandardItem(listcor.at(i).at(1).toString() + " " + listcor.at(i).at(2).toString());
        pitem       ->setAccessibleDescription(listcor.at(i).at(0).toString());
        pitem       ->setEditable(false);
        pitem       ->setCheckable(true);
        pitem       ->setCheckState(Qt::Unchecked);
        if (fm.width(pitem->text()) > largeurcolonne)
            largeurcolonne = fm.width(pitem->text());
        gmodele     ->appendRow(pitem);
    }
    tblCorresp  ->setModel(gmodele);
    int nrows   = gmodele->rowCount();
    int haut    = nrows*30 + 2;             //la valeur 30 correpsond à la hauteur figée de la ligne par la présence du checkbox
    tblCorresp  ->setFixedHeight(haut);
    if ((largeurcolonne + 40 + 2) > largfinal)
        largfinal = largeurcolonne + 40 + 2;
    tblCorresp  ->setColumnWidth(0, largfinal - 2);
    tblCorresp  ->setFixedWidth(largfinal);
    label       ->setFixedWidth(largfinal);
    label       ->setFixedHeight(hauteurligne + 2);
    globallay   ->insertWidget(0,tblCorresp);
    globallay   ->insertWidget(0,label);

    gAskCorresp ->setModal(true);
    globallay   ->setSizeConstraint(QLayout::SetFixedSize);

    connect(gAskCorresp->OKButton,   &QPushButton::clicked, [=] {ListidCor();});

    gAskCorresp ->exec();
    delete gAskCorresp;
}

void dlg_documents::ListidCor()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(gAskCorresp->findChildren<QTableView *>().at(0)->model());
    for (int i=0; i< model->rowCount(); i++)
        if (model->item(i)->checkState() == Qt::Checked)
            glistidCor << model->item(i)->accessibleDescription().toInt();
    gAskCorresp->accept();
}

// ----------------------------------------------------------------------------------
// Remplissage des UpTableWidget avec les documents de la base.
// ----------------------------------------------------------------------------------
void dlg_documents::Remplir_TableWidget()
{
    UpLineEdit          *upLine0;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem2;
    QTableWidgetItem    *pItem3;
    QTableWidgetItem    *pItem4;
    QTableWidgetItem    *pItem5;
    QTableWidgetItem    *pItem6;
    QTableWidgetItem    *pItem10;
    QTableWidgetItem    *pItem11;
    int i;
    QFontMetrics fm(qApp->font());
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    QPalette palette;
    palette.setColor(QPalette::Text,QColor(0,0,140));

    //Remplissage Table Documents
    for (int i = 0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        upLine0 = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (upLine0)
            upLine0->disconnect();
    }

    ui->DocupTableWidget->clearContents();
    QString  req = "SELECT ResumeDocument, TextDocument, idDocument, DocPublic, idUser, Prescription, editable, Medical"
              " FROM "  NOM_TABLE_COURRIERS
              " WHERE (idUser = " + QString::number(gidUser) + " Or (DocPublic = 1 and iduser <> " + QString::number(gidUser) + "))";
    req += " and ResumeDocument LIKE '" + Utils::correctquoteSQL(ui->ChercheupLineEdit->text()) + "%'";
    req += " ORDER BY ResumeDocument";
    bool ok;
    QList<QList<QVariant>> listdocs = db->StandardSelectSQL(req,ok);
    if (!ok)
        return;
    ui->DocupTableWidget->setRowCount(listdocs.size());
    for (i = 0; i < listdocs.size(); i++)
    {
        pItem1  = new QTableWidgetItem() ;
        upLine0 = new UpLineEdit() ;
        pItem2  = new QTableWidgetItem() ;
        pItem3  = new QTableWidgetItem() ;
        pItem4  = new QTableWidgetItem() ;
        pItem5  = new QTableWidgetItem() ;
        pItem6  = new QTableWidgetItem() ;
        pItem10 = new QTableWidgetItem() ;
        pItem11 = new QTableWidgetItem() ;

        int col = 0;
        QWidget * w = new QWidget(ui->DocupTableWidget);
        UpCheckBox *Check = new UpCheckBox(w);
        Check->setCheckState(Qt::Unchecked);
        Check->setRowTable(i);
        Check->setFocusPolicy(Qt::NoFocus);
        connect(Check, &QCheckBox::clicked,[=] {EnableOKPushButton(Check);});
        QHBoxLayout *l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Check);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);
        ui->DocupTableWidget->setCellWidget(i,col,w);
        col++; //1
        upLine0->setText(listdocs.at(i).at(0).toString());                          // resume
        upLine0->setRowTable(i);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::NoFocus);
        if (listdocs.at(i).at(4).toInt() != gidUser)
        {
            upLine0->setFont(disabledFont);
            upLine0->setPalette(palette);
            upLine0->setContextMenuPolicy(Qt::NoContextMenu);
        }
        else
            upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->DocupTableWidget->setCellWidget(i,col,upLine0);
        col++; //2
        pItem1->setText(listdocs.at(i).at(1).toString());                             // text
        ui->DocupTableWidget->setItem(i,col,pItem1);
        col++; //3
        pItem2->setText(listdocs.at(i).at(2).toString());                             // idDocument
        ui->DocupTableWidget->setItem(i,col,pItem2);
        col++; //4
        pItem3->setText(listdocs.at(i).at(3).toString());                             // DocPublic
        ui->DocupTableWidget->setItem(i,col,pItem3);
        col++; //5
        pItem4->setText(listdocs.at(i).at(4).toString());                             // idUser
        ui->DocupTableWidget->setItem(i,col,pItem4);
        col++; //6
        pItem5->setText(listdocs.at(i).at(5).toString());                             // Prescription 0 = pas prescription 1 = prescription
        ui->DocupTableWidget->setItem(i,col,pItem5);
        col++; //7
        UpLabel*lbl = new UpLabel(ui->DocupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        if (listdocs.at(i).at(3).toInt()==1)
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        ui->DocupTableWidget->setCellWidget(i,col,lbl);                                         // Public       0 = doc public 1 = doc privé
        col++; //8
        pItem6->setText(listdocs.at(i).at(6).toString());                             // Editable     0 = non éditable 1 = éditable
        ui->DocupTableWidget->setItem(i,col,pItem6);
        col++; //9
        UpLabel*lbl1 = new UpLabel(ui->DocupTableWidget);
        lbl1->setAlignment(Qt::AlignCenter);
        if (listdocs.at(i).at(6).toInt()==1)
            lbl1->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        ui->DocupTableWidget->setCellWidget(i,col,lbl1);
        col++; //10
        pItem10->setText("1" + upLine0->text());                                                // Check+text   -> sert pour le tri de la table
        ui->DocupTableWidget->setItem(i,col,pItem10);
        col++; //11
        pItem11->setText(listdocs.at(i).at(7).toString());                            // Medical      0 = doc administratif 1 = doc médical
        ui->DocupTableWidget->setItem(i,col,pItem11);
        col++; //12
        UpLabel*lbl11 = new UpLabel(ui->DocupTableWidget);
        lbl11->setAlignment(Qt::AlignCenter);
        if (listdocs.at(i).at(7).toInt()!=1)
            lbl11->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        ui->DocupTableWidget->setCellWidget(i,col,lbl11);

        ui->DocupTableWidget->setRowHeight(i,int(fm.height()*1.3));
    }


    //Remplissage Table Dossiers
    for (int i = 0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        upLine0 = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (upLine0)
            upLine0->disconnect();
    }
    ui->DossiersupTableWidget->clearContents();
    req =  "SELECT ResumeMetaDocument, idMetaDocument, idUser, Public"
           " FROM "  NOM_TABLE_METADOCUMENTS
           " WHERE idUser = " + QString::number(gidUser);
    req += " UNION \n";
    req += "select ResumeMetaDocument, idMetaDocument, idUser, Public from " NOM_TABLE_METADOCUMENTS
           " where idMetaDocument not in\n"
           " (select met.idMetaDocument from " NOM_TABLE_METADOCUMENTS " as met, "
           NOM_TABLE_JOINTURESDOCS " as joi, "
           NOM_TABLE_COURRIERS " as doc\n"
           " where joi.idmetadocument = met.idMetaDocument\n"
           " and joi.idDocument = doc.iddocument\n"
           " and doc.docpublic is null)\n";
    req += " ORDER BY ResumeMetaDocument;";
    //UpMessageBox::Watch(this,RemplirtableDossiersrequete);
    QList<QList<QVariant>> listdossiers = db->StandardSelectSQL(req,ok);
    if (!ok)
        return;

    ui->DossiersupTableWidget->setRowCount(listdossiers.size());
    for (i = 0; i < listdossiers.size(); i++)
    {
        pItem1  = new QTableWidgetItem() ;
        upLine0 = new UpLineEdit() ;
        pItem2  = new QTableWidgetItem() ;

        int col = 0;
        QWidget * w = new QWidget(ui->DossiersupTableWidget);
        UpCheckBox *Check = new UpCheckBox(w);
        Check->setCheckState(Qt::Unchecked);
        Check->setRowTable(i);
        Check->setFocusPolicy(Qt::NoFocus);
        connect(Check, &QCheckBox::clicked,[=] {EnableOKPushButton(Check);});
        QHBoxLayout *l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Check);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);
        ui->DossiersupTableWidget->setCellWidget(i,col,w);

        col++; //1
        upLine0->setText(listdossiers.at(i).at(0).toString());                           // resume
        upLine0->setRowTable(i);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::NoFocus);
        if (listdossiers.at(i).at(2).toInt() != gidUser)
        {
            upLine0->setFont(disabledFont);
            upLine0->setPalette(palette);
            upLine0->setContextMenuPolicy(Qt::NoContextMenu);
        }
        else
            upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->DossiersupTableWidget->setCellWidget(i,col,upLine0);

        col++; //2
        pItem1->setText(listdossiers.at(i).at(1).toString());                           // idMetaDocument
        ui->DossiersupTableWidget->setItem(i,col,pItem1);

        col++; //3
        pItem2->setText(listdossiers.at(i).at(2).toString());                           // idUser
        ui->DossiersupTableWidget->setItem(i,col,pItem2);

        col++; //4
        UpLabel*lbl = new UpLabel(ui->DossiersupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        if (listdossiers.at(i).at(3).toInt()==1)
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        ui->DossiersupTableWidget->setCellWidget(i,col,lbl);

        ui->DossiersupTableWidget->setRowHeight(i,int(fm.height()*1.3));
    }
}

// ----------------------------------------------------------------------------------
// Supprime Document
// ----------------------------------------------------------------------------------
void dlg_documents::SupprimmDocument(int row)
{
    DisableLines();
    UpLineEdit *line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(row,1));

    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le document\n") + line->text().toUpper() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton NoBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer le document"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != &NoBouton)
    {
        int iddoc = ui->DocupTableWidget->item(row,3)->text().toInt();
        db->SupprRecordFromTable(iddoc,
                                 "idDocument",
                                 NOM_TABLE_COURRIERS,
                                 tr("Impossible de supprimer le document\n") + line->text().toUpper() + tr("\n ... et je ne sais pas pourquoi..."));
        Remplir_TableWidget();
    }
    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        LineSelect(ui->DocupTableWidget,row);
    }
}

// ----------------------------------------------------------------------------------
// Supprime Dossier
// ----------------------------------------------------------------------------------
void dlg_documents::SupprimmDossier(int row)
{
    UpLineEdit *line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(row,1));

    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le  dossier\n") + line->text().toUpper() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton(tr("Supprimer le dosssier"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != &NoBouton)
    {
        int iddos = ui->DossiersupTableWidget->item(row,2)->text().toInt();
        QStringList locklist;
        locklist << NOM_TABLE_METADOCUMENTS << NOM_TABLE_JOINTURESDOCS ;
        if (db->SupprRecordFromTable(iddos,
                                      "idMetaDocument",
                                      NOM_TABLE_METADOCUMENTS,
                                      tr("Impossible de suppprimer le dossier") + "\n" + line->text().toUpper() + "!\n ... " + tr("et je ne sais pas pourquoi") + "...\nRufus"))
        db->SupprRecordFromTable(iddos, "idMetaDocument", NOM_TABLE_JOINTURESDOCS);
        Remplir_TableWidget();
    }
    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        LineSelect(ui->DossiersupTableWidget,row);
    }
}

// ----------------------------------------------------------------------------------
// Tri de la table documents.
// ----------------------------------------------------------------------------------
void dlg_documents::TriDocupTableWidget()
{
    ui->DocupTableWidget->sortByColumn(10,Qt::AscendingOrder);
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1))->setRowTable(i);
        static_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0))->findChildren<UpCheckBox*>().at(0)->setRowTable(i);
    }
}

// ----------------------------------------------------------------------------------
// Modification du Document dans la base.
// ----------------------------------------------------------------------------------
void dlg_documents::UpdateDocument(int row)
{
    // recherche de l'enregistrement modifié
    // controle validate des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));
    if (line->text().length() < 1)    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Modification de document"), tr("Veuillez renseigner le champ Résumé, SVP !"));
        return;
    }
    if (line->text() == tr("Nouveau document"))
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Création de document"), tr("Votre document ne peut pas s'appeler \"Nouveau document\""));
        return;
    }
    if (ui->upTextEdit->document()->toPlainText().length() < 1)   {
        UpMessageBox::Watch(Q_NULLPTR,tr("Modification de document"), tr("Veuillez renseigner le champ Document, SVP !"));
        return;
    }

    if (ChercheDoublon(line->text(), row))
    {
        line->setFocus();
        line->selectAll();
        return;
    }

    QString idAmodifier = ui->DocupTableWidget->item(row,3)->text();
    QString req =   "UPDATE " NOM_TABLE_COURRIERS
            " SET TextDocument = '" + Utils::correctquoteSQL(ui->upTextEdit->toHtml())     + "'"
            ", ResumeDocument = '"  + Utils::correctquoteSQL(line->text().left(100)) + "'";
    if (ui->DocPubliccheckBox->isChecked())         req += " , DocPublic = 1";      else req += " , DocPublic = null";
    if (ui->PrescriptioncheckBox->isChecked())      req += " , Prescription = 1";   else req += " , Prescription = null";
    if (ui->DocEditcheckBox->isChecked())           req += " , Editable = 1";       else req += " , Editable = null";
    if (ui->DocAdministratifcheckBox->isChecked())  req += " , Medical = 1";        else req += " , Medical = 0";
    req += " WHERE  idDocument = " + idAmodifier;
    db->StandardSQL(req, tr("Erreur de mise à jour du document dans ") + NOM_TABLE_COURRIERS);
    Remplir_TableWidget();

    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        QString resume = line->text();
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->text() == resume)
            {
                LineSelect(ui->DocupTableWidget,line->getRowTable());
                QModelIndex index = ui->DocupTableWidget->model()->index(line->getRowTable(),1);
                ui->DocupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
                if (Widg)
                {
                    Widg->findChildren<UpCheckBox*>().at(0)->setChecked(true);
                    ui->OKupPushButton->setEnabled(true);
                }
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------------
// Modification du Dossier dans la base.
// ----------------------------------------------------------------------------------
void dlg_documents::UpdateDossier(int row)
{
    QStringList listid;
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));
    for (int l=0; l<ui->DocupTableWidget->rowCount(); l++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(l,0));
        if (Widg)
        {
            UpCheckBox *DocCheck = Widg->findChildren<UpCheckBox*>().at(0);
            if (DocCheck->isChecked())
                listid << ui->DocupTableWidget->item(l,3)->text();
        }
    }
    if (listid.size() == 0)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Modification de Dossier"), tr("Veuillez cocher au moins un document, SVP !"));
        return;
    }

    QString iddoss  = ui->DossiersupTableWidget->item(row,2)->text();

    if (ChercheDoublon(line->text(), row))
    {
        line->setFocus();
        line->selectAll();
        return;
    }

    db->SupprRecordFromTable(iddoss.toInt(), "idMetaDocument", NOM_TABLE_JOINTURESDOCS);

    QString req     = "insert into " NOM_TABLE_JOINTURESDOCS " (idmetadocument,iddocument) Values ";
    for (int i=0; i<listid.size(); i++)
    {
        req += "(" + iddoss + ", " + listid.at(i) + ")";
        if (i<listid.size()-1)
            req += ",";
    }
    db->StandardSQL(req);

    req =   "UPDATE " NOM_TABLE_METADOCUMENTS
            " SET ResumeMetaDocument = '"  + Utils::correctquoteSQL(line->text().left(100)) + "'"
            " WHERE  idmetaDocument = " + iddoss;
    db->StandardSQL(req, tr("Erreur de mise à jour du dossier dans ") + NOM_TABLE_METADOCUMENTS);

    Remplir_TableWidget();
    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            if (ui->DossiersupTableWidget->item(i,2)->text() == iddoss)
            {
                QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
                if (Widg)
                {
                    UpCheckBox *DossCheck = Widg->findChildren<UpCheckBox*>().at(0);
                    DossCheck->setChecked(true);
                }
                CocheLesDocs(iddoss.toInt(),true);
                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------------
// Verifie qu'un document peut devenir privé
// ----------------------------------------------------------------------------------
bool dlg_documents::VerifDocumentPublic(int row, bool msg)
{
    bool ok;
    int iddoc = ui->DocupTableWidget->item(row,3)->text().toInt();
    QString req = "select idmetadocument, resumemetadocument from " NOM_TABLE_METADOCUMENTS
                  " where idmetadocument in (select idmetadocument from " NOM_TABLE_JOINTURESDOCS " where iddocument = " + QString::number(iddoc) +
                  ") and public =1";
    QList<QList<QVariant>> listdossiers = db->StandardSelectSQL(req,ok);
    if (listdossiers.size()>0)
    {
        if (msg)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas rendre privé ce document"), tr("Il est incorporé dans le dossier public\n- ") + listdossiers.at(0).at(1).toString() +
                             tr(" -\nVous devez d'abord rendre ce dossier privé!"));
        }
        return false;
    }
    else return true;
}

// ----------------------------------------------------------------------------------
// Verifie qu'un dossier ne reste pas coché pour rien
// ----------------------------------------------------------------------------------
void dlg_documents::VerifDossiers()
{
    bool ok;
    for (int j=0; j<ui->DossiersupTableWidget->rowCount(); j++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(j,0));
        if (Widg)
        {
            UpCheckBox *DossCheck = Widg->findChildren<UpCheckBox*>().at(0);
            if (DossCheck->isChecked())
            {
                QString req = "select idDocument from " NOM_TABLE_JOINTURESDOCS
                              " where idMetaDocument = " + ui->DossiersupTableWidget->item(j,2)->text();
                QList<QList<QVariant>> listdocs = db->StandardSelectSQL(req,ok);
                if (listdocs.size() > 0)
                {
                    QStringList listid;
                    for (int i=0; i<listdocs.size(); i++)
                        listid << listdocs.at(i).at(0).toString();
                    bool a = false;
                    for (int k=0; k<listid.size(); k++)
                    {
                        for (int l=0; l<ui->DocupTableWidget->rowCount(); l++)
                            if (listid.contains(ui->DocupTableWidget->item(l,3)->text()))
                            {
                                QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(l,0));
                                if (Widg)
                                {
                                    UpCheckBox *DocCheck = Widg->findChildren<UpCheckBox*>().at(0);
                                    if (DocCheck->isChecked())
                                    {a = true;  break;}
                                }
                            }
                        if (a) break;
                    }
                    if (!a) DossCheck->setChecked(false);
    }   }   }   }
}

// ----------------------------------------------------------------------------------
// Verifie qu'un dossier peut être rendu public
// ----------------------------------------------------------------------------------
bool dlg_documents::VerifDossierPublic(int row, bool msg)
{
    bool ok;
    int iddossier = ui->DossiersupTableWidget->item(row,2)->text().toInt();
    QString req = "select iddocument, resumedocument from " NOM_TABLE_COURRIERS
                  " where iddocument in (select iddocument from " NOM_TABLE_JOINTURESDOCS " where idmetadocument = " + QString::number(iddossier) +
                  ") and docpublic is null";
    QList<QList<QVariant>> listdocs = db->StandardSelectSQL(req,ok);
    if (listdocs.size()>0)
    {
        if (msg)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas rendre public ce dossier.\nIl incorpore le document\n- ") +
                             listdocs.at(0).at(1).toString() + tr(" -\nqui est un document privé!"));
        }
        return false;
    }
    else return true;
}
