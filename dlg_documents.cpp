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

#include "dlg_documents.h"
#include "icons.h"
#include "ui_dlg_documents.h"

dlg_documents::dlg_documents(int idPatAPasser, QString NomPatient, QString PrenomPatient,
                             Procedures *procAPasser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_documents)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    gidPatient          = idPatAPasser;
    gNomPat             = NomPatient;
    gPrenomPat          = PrenomPatient;

    proc                = procAPasser;
    gidUser             = proc->getDataUser()["idUser"].toInt();
    gidUserSuperviseur  = proc->getDataUser()["UserSuperviseur"].toInt();
    db                  = proc->getDataBase();

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionDocuments").toByteArray());
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    setWindowTitle(tr("Liste des documents prédéfinis"));

    ui->PrescriptioncheckBox
                        ->setVisible(proc->getDataUser()["Soignant"].toInt() == 1 || proc->getDataUser()["Soignant"].toInt() == 2 || proc->getDataUser()["Soignant"].toInt() == 3);
    widgButtonsDocs     = new WidgetButtonFrame(ui->DocupTableWidget);
    widgButtonsDocs     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    widgButtonsDocs     ->layButtons()->insertWidget(0, ui->ChercheupLineEdit);
    widgButtonsDocs     ->layButtons()->insertWidget(0, ui->label);
    widgButtonsDossiers = new WidgetButtonFrame(ui->DossiersupTableWidget);
    widgButtonsDossiers ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    // Initialisation des slots.
    connect (ui->ChercheupLineEdit,             &QLineEdit::textEdited,                 [=] {FiltreListe(ui->ChercheupLineEdit->text());});
    connect (ui->OKupPushButton,                &QPushButton::clicked,                  [=] {Validation();});
    connect (ui->AnnulupPushButton,             &QPushButton::clicked,                  [=] {Annulation();});
    connect (ui->DocPubliccheckBox,             &QCheckBox::clicked,                    [=] {CheckPublicEditable(ui->DocPubliccheckBox);});
    connect (ui->DocEditcheckBox,               &QCheckBox::clicked,                    [=] {CheckPublicEditable(ui->DocEditcheckBox);});
    connect (ui->PrescriptioncheckBox,          &QPushButton::clicked,                  [=] {EnableOKPushButton();});
    connect (ui->upTextEdit,                    &QWidget::customContextMenuRequested,   [=] {MenuContextuel(ui->upTextEdit);});
    connect (ui->upTextEdit,                    &QTextEdit::textChanged,                [=] {EnableOKPushButton();});
    connect (ui->upTextEdit,                    &UpTextEdit::dblclick,                  [=] {dblClicktextEdit();});
    connect (ui->DupliOrdocheckBox,             &QCheckBox::clicked,                    [=] {OrdoAvecDupli(ui->DupliOrdocheckBox->isChecked());});
    connect (widgButtonsDocs,                   &WidgetButtonFrame::choix,              [=] {ChoixButtonFrame(widgButtonsDocs->Reponse(), widgButtonsDocs);});
    connect (widgButtonsDossiers,               &WidgetButtonFrame::choix,              [=] {ChoixButtonFrame(widgButtonsDossiers->Reponse(), widgButtonsDossiers);});

    // Mise en forme de la table Documents
    ui->DocupTableWidget->setPalette(QPalette(Qt::white));
    ui->DocupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->DocupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DocupTableWidget->verticalHeader()->setVisible(false);
    ui->DocupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->DocupTableWidget->setColumnCount(11);
    ui->DocupTableWidget->setColumnWidth(0,30);      // checkbox
    ui->DocupTableWidget->setColumnWidth(1,289);     // Resume
    ui->DocupTableWidget->setColumnHidden(2,true);   // colonne texte masquee
    ui->DocupTableWidget->setColumnHidden(3,true);   // idDocument
    ui->DocupTableWidget->setColumnHidden(4,true);   // DocPublic
    ui->DocupTableWidget->setColumnHidden(5,true);   // idUserqui a créé le document
    ui->DocupTableWidget->setColumnHidden(6,true);   // Prescription
    ui->DocupTableWidget->setColumnWidth(7,30);      // Public
    ui->DocupTableWidget->setColumnHidden(8,true);   // Editable
    ui->DocupTableWidget->setColumnWidth(9,30);
    ui->DocupTableWidget->setColumnHidden(10,true);  // Checked
    ui->DocupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Icons::icImprimer(),""));
    ui->DocupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("TITRES DES DOCUMENTS")));
    ui->DocupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(6, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(7, new QTableWidgetItem(Icons::icFamily(),""));
    ui->DocupTableWidget->setHorizontalHeaderItem(8, new QTableWidgetItem(""));
    ui->DocupTableWidget->setHorizontalHeaderItem(9, new QTableWidgetItem(Icons::icEditer(),""));
    ui->DocupTableWidget->setHorizontalHeaderItem(10,new QTableWidgetItem(""));
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
    ui->DossiersupTableWidget->setColumnWidth(1,289);     // Resume
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
    ui->label->setPixmap(Icons::pxLoupe().scaled(30,30)); //TODO : icon scaled : pxLoupe 20,20
    ui->ChercheupLineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 3px solid rgb(164, 205, 255);border-radius: 10px;}");


    ui->textFrame->installEventFilter(this);
    ui->DocupTableWidget->installEventFilter(this);
    ui->DossiersupTableWidget->installEventFilter(this);
    gOp             = new QGraphicsOpacityEffect();
    gTimerEfface    = new QTimer(this);

    QString ALDrequete = "select idPat from " NOM_TABLE_DONNEESSOCIALESPATIENTS " where idpat = " + QString::number(gidPatient) + " and PatALD = 1";
    QSqlQuery ALDQuery (ALDrequete,db);
    proc->TraiteErreurRequete(ALDQuery,ALDrequete,"");
    if (ALDQuery.size() > 0)
        ui->ALDcheckBox->setChecked(true);

    //nettoyage de la table metadocs
    QString req = "delete from " NOM_TABLE_JOINTURESDOCS " where iddocument not in (select iddocument from " NOM_TABLE_COURRIERS ")";
    QSqlQuery (req,db);

    Remplir_TableView();
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
        Remplir_TableView();
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

void dlg_documents::CheckPublicEditable(QCheckBox *check)
{
    UpLineEdit *line;
    bool a = false;
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)  {
        line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line->isEnabled()) {a = true; break;}
    }
    if (!a) return;

    int b(-1), c(-1);
    if (check == ui->DocPubliccheckBox) {
        b = 4;
        c = 7;
    }
    else if (check == ui->DocEditcheckBox)  {
        b = 8;
        c = 9;
    }
    if (check->isChecked()) {
        ui->DocupTableWidget->item(line->getRowTable(),b)->setText("1");
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->getRowTable(),c));
        lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
    }
    else    {
        ui->DocupTableWidget->item(line->getRowTable(),b)->setText("");
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
        if (idUser == proc->getDataUser()["idUser"].toInt())
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
        int idMetaDoc = ui->DossiersupTableWidget->item(row,2)->text().toInt();
        QString req = "select resumedocument, iddocument from " NOM_TABLE_COURRIERS
                " where iddocument in (select iddocument from " NOM_TABLE_JOINTURESDOCS
                " where idmetadocument = " + QString::number(idMetaDoc) + ")";
        //UpMessageBox::Watch(this,req);
        QSqlQuery quer(req,db);
        QString resume = "";
        if (quer.size()>0)
        {
            quer.first();
            resume += quer.value(0).toString();
            quer.next();
            for (int i = 1; i< quer.size(); i++)
            {
                resume += "\n" + quer.value(0).toString();
                quer.next();
            }
            QToolTip::showText(QPoint(pos.x()+50,pos.y()), resume, ui->DossiersupTableWidget, rect, 2000);
        }
    }
}

// ----------------------------------------------------------------------------------
// On clique une ligne de com. On affiche le détail et on met en édition
// ----------------------------------------------------------------------------------
void dlg_documents::DocCellClick(UpLineEdit *line)
{
    if (ui->DocupTableWidget->isAncestorOf(line))
        LineSelect(ui->DocupTableWidget, line->getRowTable());
    if (ui->DossiersupTableWidget->isAncestorOf(line))
        LineSelect(ui->DossiersupTableWidget, line->getRowTable());
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
// Efface Widget progressivement
// ----------------------------------------------------------------------------------
void dlg_documents::EffaceWidget()
{
    QRect rect = QRect(gWidg->pos(),gWidg->size());
    QPoint pos = mapFromParent(cursor().pos());
    int Pause = 4000;
    if(!gPause) Pause = 0;
    if (gDebutTimer.msecsTo(QTime::currentTime()) > Pause  && !rect.contains(pos))
    {
        gOpacity = gOpacity*0.9;
        gOp->setOpacity(gOpacity);
        gWidg->setGraphicsEffect(gOp);
        if (gOpacity < 0.10)
        {
            disconnect(gTimerEfface, 0,0,0);
            gTimerEfface->stop();
            gWidg = 0;
        }
    }
    else
    {
        gOpacity = 1;
        gOp->setOpacity(gOpacity);
        gWidg->setGraphicsEffect(gOp);
    }
}

// ----------------------------------------------------------------------------------
// Enable OKpushbutton
// ----------------------------------------------------------------------------------
void dlg_documents::EnableOKPushButton(UpCheckBox *Check)
{
    if (gMode == CreationDOC || gMode == ModificationDOC)
    {
        UpLineEdit *line;
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
        UpLineEdit *line;
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
    Remplir_TableView();
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
    QAction *pAction_ModifDossier       = 0;
    QAction *pAction_SupprDossier       = 0;
    QAction *pAction_CreerDossier       = 0;
    QAction *pAction_PublicDossier      = 0;
    QAction *pAction_ModifDoc           = 0;
    QAction *pAction_SupprDoc           = 0;
    QAction *pAction_CreerDoc           = 0;
    QAction *pAction_PublicDoc          = 0;
    QAction *pAction_PrescripDoc        = 0;
    QAction *pAction_EditableDoc        = 0;
    QAction *pAction_InsertChamp        = 0;
    QAction *pAction_ModifPolice        = 0;
    QAction *pAction_Fontbold           = 0;
    QAction *pAction_Fontitalic         = 0;
    QAction *pAction_Fontunderline      = 0;
    QAction *pAction_Fontnormal         = 0;
    QAction *pAction_Blockcentr         = 0;
    QAction *pAction_Blockjust          = 0;
    QAction *pAction_Blockright         = 0;
    QAction *pAction_Blockleft          = 0;
    QAction *pAction_Copier             = 0;
    QAction *pAction_Cut                = 0;
    QAction *pAction_Coller             = 0;
    QAction *pAction_InsInterroDate     = 0;
    QAction *pAction_InsInterroCote     = 0;
    QAction *pAction_InsInterroHeure    = 0;
    QAction *pAction_InsInterroMontant  = 0;
    QAction *pAction_InsInterroText     = 0;
    QMenu *interro                      = 0;
    UpLabel *lbl                        = 0;
    UpLineEdit *line0                   = 0;
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
            lbl                             = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(line->getRowTable(),4));
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
            if (ui->DocupTableWidget->item(line->getRowTable(),6)->text().toInt() == 1)
                pAction_PrescripDoc         = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Prescription"));
            else
                pAction_PrescripDoc         = gmenuContextuel->addAction(tr("Prescription"));
            if (ui->DocupTableWidget->item(line->getRowTable(),8)->text().toInt() == 1)
                pAction_EditableDoc         = gmenuContextuel->addAction(Icons::icBlackCheck(), tr("Editable"));
            else
                pAction_EditableDoc         = gmenuContextuel->addAction(tr("Editable"));

            pAction_PublicDoc->setToolTip(tr("si cette option est cochée\ntous les utilisateurs\nauront accès à ce document"));
            pAction_PrescripDoc->setToolTip(tr("si cette option est cochée\nce document sera considéré comme une prescription"));
            pAction_EditableDoc->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));

            connect (pAction_ModifDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("ModifierDoc");});
            connect (pAction_SupprDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("SupprimerDoc");});
            connect (pAction_CreerDoc,      &QAction::triggered,    [=] {ChoixMenuContextuel("CreerDoc");});
            connect (pAction_PublicDoc,     &QAction::triggered,    [=] {ChoixMenuContextuel("PublicDoc");});
            connect (pAction_PrescripDoc,   &QAction::triggered,    [=] {ChoixMenuContextuel("PrescripDoc");});
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
    pAction_ModifDossier        = 0;
    pAction_SupprDossier        = 0;
    pAction_CreerDossier        = 0;
    pAction_PublicDossier       = 0;
    pAction_ModifDoc            = 0;
    pAction_SupprDoc            = 0;
    pAction_CreerDoc            = 0;
    pAction_PublicDoc           = 0;
    pAction_PrescripDoc         = 0;
    pAction_EditableDoc         = 0;
    pAction_InsertChamp         = 0;
    pAction_ModifPolice         = 0;
    pAction_Fontbold            = 0;
    pAction_Fontitalic          = 0;
    pAction_Fontunderline       = 0;
    pAction_Fontnormal          = 0;
    pAction_Blockcentr          = 0;
    pAction_Blockleft           = 0;
    pAction_Blockright          = 0;
    pAction_Blockjust           = 0;
    pAction_Copier              = 0;
    pAction_Cut                 = 0;
    pAction_Coller              = 0;
    pAction_InsInterroDate      = 0;
    pAction_InsInterroCote      = 0;
    pAction_InsInterroHeure     = 0;
    pAction_InsInterroMontant   = 0;
    pAction_InsInterroText      = 0;
    interro                     = 0;
    lbl                         = 0;
    line0                       = 0;
    delete pAction_ModifDossier;
    delete pAction_SupprDossier;
    delete pAction_CreerDossier;
    delete pAction_PublicDossier;
    delete pAction_ModifDoc;
    delete pAction_SupprDoc;
    delete pAction_CreerDoc;
    delete pAction_PublicDoc;
    delete pAction_PrescripDoc;
    delete pAction_EditableDoc;
    delete pAction_InsertChamp;
    delete pAction_ModifPolice;
    delete pAction_Fontbold;
    delete pAction_Fontitalic;
    delete pAction_Fontunderline;
    delete pAction_Fontnormal;
    delete pAction_Blockcentr;
    delete pAction_Blockleft;
    delete pAction_Blockright;
    delete pAction_Blockjust;
    delete pAction_Copier;
    delete pAction_Cut;
    delete pAction_Coller;
    delete pAction_InsInterroDate;
    delete pAction_InsInterroCote;
    delete pAction_InsInterroHeure;
    delete pAction_InsInterroMontant;
    delete pAction_InsInterroText;
    delete interro;
    delete lbl;
    delete line0;
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
        UpLineEdit *line;
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
        UpLineEdit *line;
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
        UpLineEdit *line;
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
        UpLineEdit *line;
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
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
        }
        else
        {
            if (!VerifDocumentPublic(line->getRowTable())) return;
            lbl->clear();
        }
        ui->DocPubliccheckBox->toggle();
        if (gMode == Selection)
            QSqlQuery ("update " NOM_TABLE_COURRIERS " set DocPublic = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text(),db);
        if (a=="null") a= "";
        ui->DocupTableWidget->item(line->getRowTable(),4)->setText(a);
    }
    else if (choix  == "EditDoc")
    {
        UpLineEdit *line;
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
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
        }
        else
            lbl->clear();
        ui->DocEditcheckBox->toggle();
        if (gMode == Selection)
            QSqlQuery ("update " NOM_TABLE_COURRIERS " set Editable = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text(),db);
        if (a=="null") a= "";
        ui->DocupTableWidget->item(line->getRowTable(),8)->setText(a);
    }
    else if (choix  == "PublicDossier")
    {
        UpLineEdit *line;
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
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
            a = "1";
        }
        else
            lbl->clear();
        if (gMode == Selection)
            QSqlQuery ("update " NOM_TABLE_METADOCUMENTS " set Public = " + a + " where idMetaDocument = " +
                       ui->DossiersupTableWidget->item(line->getRowTable(),2)->text(), db);
    }
    else if (choix  == "PrescripDoc")
    {
        UpLineEdit *line;
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
            QSqlQuery ("update " NOM_TABLE_COURRIERS " set Prescription = " + a + " where idDocument = " +
                       ui->DocupTableWidget->item(line->getRowTable(),3)->text(),db);
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
        UpLineEdit *line;
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
        UpDialog *ListChamps            = new UpDialog(this);
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
        tabChamps->setFixedHeight(fm.height()*1.1*9);
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
            tabChamps   ->setRowHeight(i,fm.height()*1.1);
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
            UpMessageBox::Watch(this,"Euuhh... " + proc->getDataUser()["UserLogin"].toString() + ", " + tr("il doit y avoir une erreur..."), tr("Vous n'avez sélectionné aucun document."));
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
                    Line->setValidator(new QRegExpValidator(proc->getrxAdresse(),this));
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
                for (int i=0; i<proc->getListeSuperviseurs()->rowCount(); i++)
                    Combo->addItem(proc->getListeSuperviseurs()->item(i,1)->text(), proc->getListeSuperviseurs()->item(i,0)->text());
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
                            b = (dynamic_cast<UpLabel*>(w)!=NULL);
                            if (b) break;
                            w=0;
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
                        // on visualise le document pour correction s'il est éditable
                        txtdoc                          = ((ui->DocupTableWidget->item(i,8)->text() == "1")? proc->Edit(txtdoc, titre): txtdoc);
                        if (txtdoc == "")               // si le texte du document est vide, on annule l'impression de cette itération
                        {
                            TitreDocumentAImprimerList  .removeLast();
                            PrescriptionAImprimerList   .removeLast();
                            DupliAImprimerList          .removeLast();
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
                QWidget* rec = ui->textFrame;
                QRect rect = QRect(rec->pos(),rec->size());
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
    Line->setValidator(new QRegExpValidator(proc->getrxAdresse(),this));
    Line->setMaxLength(60);
    return gAskDialog->exec();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de document
// ----------------------------------------------------------------------------------
bool dlg_documents::ChercheDoublon(QString str, int row)
{
    bool a = false;
    QString req;
    QSqlQuery quer;

    switch (gMode) {
    case CreationDOC:
    case ModificationDOC:
        req = "select resumedocument, iduser from " NOM_TABLE_COURRIERS " where iddocument <> " + ui->DocupTableWidget->item(row,3)->text();
        quer = QSqlQuery(req,db);
        if (quer.size() > 0)
        {
            quer.first();
            for (int i=0; i<quer.size() ; i++)
            {
                if (quer.value(0).toString().toUpper() == str.toUpper())
                {
                    a = true;
                    QString b = "vous";
                    if (quer.value(1).toInt() != gidUser)
                        b = proc->getLogin(quer.value(1).toInt());
                    UpMessageBox::Watch(this,tr("Il existe déjà un document portant ce nom créé par ") + b);
                    break;
                }
                quer.next();
            }
        }
        break;
    case CreationDOSS:
    case ModificationDOSS:
        req = "select resumemetadocument, iduser from " NOM_TABLE_METADOCUMENTS " where idmetadocument <> " + ui->DossiersupTableWidget->item(row,2)->text();
        quer = QSqlQuery(req,db);
        if (quer.size() > 0)
        {
            quer.first();
            for (int i=0; i<quer.size() ; i++)
            {
                if (quer.value(0).toString().toUpper() == str.toUpper())
                {
                    a = true;
                    QString b = "vous";
                    if (quer.value(1).toInt() != gidUser)
                        b = proc->getLogin(quer.value(1).toInt());
                    UpMessageBox::Watch(this,tr("Il existe déjà un dossier portant ce nom créé par ") + b);
                    break;
                }
                quer.next();
            }
        }
        break;
    default:
        break;
    }
    return a;
}

// ----------------------------------------------------------------------------------
// On a clique sur une ligne de dossiers . on coche-décoche les docs correspondants
// ----------------------------------------------------------------------------------
void dlg_documents::CocheLesDocs(int iddoss, bool A)
{
    QString idDossier = QString::number(iddoss);
    QString requete = "select idDocument from " NOM_TABLE_JOINTURESDOCS
            " where idMetaDocument = " + idDossier;
    QSqlQuery listdocsquery(requete,db);
    //UpMessageBox::Watch(this,requete);
    proc->TraiteErreurRequete(listdocsquery,requete,"");
    if (listdocsquery.size() > 0)
    {
        QStringList listiddocs;
        listdocsquery.first();
        for (int i=0; i<listdocsquery.size(); i++)
        {
            listiddocs << listdocsquery.value(0).toString();
            listdocsquery.next();
        }
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
                                        requete = "select idDocument from " NOM_TABLE_JOINTURESDOCS
                                                " where idMetaDocument = " + ui->DossiersupTableWidget->item(j,2)->text();
                                        QSqlQuery docsquer(requete,db);
                                        proc->TraiteErreurRequete(docsquer,requete,"");
                                        if (docsquer.size() > 0)
                                        {
                                            QStringList listid;
                                            docsquer.first();
                                            for (int i=0; i<docsquer.size(); i++)
                                            {
                                                listid << docsquer.value(0).toString();
                                                docsquer.next();
                                            }
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
    widgButtonsDossiers->setEnabled    (gMode == Selection);
    ui->DossiersupTableWidget->setEnabled   (gMode == Selection);
    ui->OKupPushButton->setEnabled          (false);
    ui->textFrame->setVisible               (gMode != CreationDOSS && gMode!= ModificationDOSS && gMode != Selection);

    gOpacity = 0.1;
    if (gMode != Selection) {
        disconnect(gTimerEfface, 0,0,0);
        gTimerEfface->stop();
        gOpacity = 1;
    }
    else
        gOpacity = 0.1;
    gOp->setOpacity(gOpacity);
    ui->textFrame->setGraphicsEffect(gOp);

    if (mode == Selection)
    {
        EnableLines();
        widgButtonsDocs->setEnabled(true);
        ui->DocPubliccheckBox   ->setChecked(false);
        ui->DocPubliccheckBox   ->setEnabled(false);
        ui->DocPubliccheckBox   ->setToolTip("");
        ui->DocupTableWidget    ->setEnabled(true);
        ui->DocupTableWidget    ->setFocus();
        ui->DocupTableWidget    ->setStyleSheet("");
        widgButtonsDossiers     ->setEnabled(true);
        ui->DossiersupTableWidget->setEnabled(true);
        ui->DocEditcheckBox     ->setChecked(false);
        ui->DocEditcheckBox     ->setEnabled(false);
        ui->DocEditcheckBox     ->setToolTip("");
        ui->Expliclabel->setText(tr("SELECTION - Cochez les dossiers ou les documents que vous voulez imprimer"));
        widgButtonsDocs->modifBouton->setEnabled(false);
        widgButtonsDossiers->modifBouton->setEnabled(false);
        ui->PrescriptioncheckBox->setEnabled(false);
        widgButtonsDocs->moinsBouton->setEnabled(false);
        widgButtonsDossiers->moinsBouton->setEnabled(false);
        ui->textFrame->setStyleSheet("");
        ui->textFrame->setEnabled(true);
        ui->upTextEdit->clear();
        ui->upTextEdit->setFocusPolicy(Qt::NoFocus);
        ui->upTextEdit->setStyleSheet("");

        ui->AnnulupPushButton->setIcon(Icons::icAnnuler());
        ui->AnnulupPushButton->setToolTip(tr("Annuler et fermer la fiche"));
        ui->OKupPushButton->setText(tr("Imprimer\nla sélection"));
        ui->OKupPushButton->setIcon(Icons::icImprimer());
        ui->OKupPushButton->setIconSize(QSize(30,30));

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
        ui->Expliclabel->setText(tr("DOCUMENTS - MODIFICATION"));
        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));
        ui->PrescriptioncheckBox    ->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

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
        ui->DocupTableWidget->setRowHeight(row,QFontMetrics(qApp->font()).height()*1.3);

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
        widgButtonsDocs->moinsBouton->setEnabled(false);
        ui->upTextEdit->clear();
        ui->upTextEdit->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

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

        ui->DossiersupTableWidget->setRowHeight(row,QFontMetrics(qApp->font()).height()*1.3);

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
}

// --------------------------------------------------------------------------------------------------
// SetEnabled = false et disconnect toute sles lignes des UpTableWidget - SetEnabled = false checkBox
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
            disconnect(line0,  0, 0, 0);
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
            disconnect(line0,  0, 0, 0);
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
            connect(line0,              &UpLineEdit::mouseRelease,              [=] {DocCellClick(line0);});
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
            connect(line0,              &UpLineEdit::mouseRelease,              [=] {DocCellClick(line0);});
        }
    }
}

// ----------------------------------------------------------------------------------
// Effacement progressif d'un textEdit.
// ----------------------------------------------------------------------------------
void dlg_documents::EffaceWidget(QWidget* widg, bool AvecOuSansPause)
{
    gWidg           = widg;
    gPause          = AvecOuSansPause;
    gDebutTimer     = QTime::currentTime();
    gWidg->setVisible(true);
    gWidg->setAutoFillBackground(true);
    gTimerEfface->stop();
    disconnect(gTimerEfface, 0,0,0);
    gTimerEfface->start(70);
    connect(gTimerEfface, &QTimer::timeout, [=] {EffaceWidget();});
}

// ----------------------------------------------------------------------------------
// Creation du Document dans la base.
// ----------------------------------------------------------------------------------
void dlg_documents::InsertDocument(int row)
{
    // controle validate des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(row,1));
    line->setText(proc->MajusculePremiereLettre(line->text(), true, false, false));
    if (line->text().length() < 1)
    {
        UpMessageBox::Watch(0,tr("Creation de document"), tr("Veuillez renseigner le champ Résumé, SVP !"));
        return;
    }
    if (line->text() == tr("Nouveau document"))
    {
        UpMessageBox::Watch(0,tr("Creation de document"), tr("Votre document ne peut pas s'appeler \"Nouveau document\""));
        return;
    }
    if (ui->upTextEdit->document()->toPlainText().length() < 1)
    {
        UpMessageBox::Watch(0,tr("Creation de document"), tr("Veuillez renseigner le champ Document, SVP !"));
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
            " (TextDocument, ResumeDocument, idUser, DocPublic, Prescription, Editable) "
            " VALUES ('" + proc->CorrigeApostrophe(ui->upTextEdit->document()->toHtml()) +
            "', '" + proc->CorrigeApostrophe(line->text().left(100)) +
            "', " + QString::number(gidUser);
    if (ui->DocPubliccheckBox->isChecked())
        requete += ", 1";
    else
        requete += ", null";
    if (ui->PrescriptioncheckBox->isChecked())
        requete += ", 1";
    else
        requete += ", null";
    if (ui->DocEditcheckBox->isChecked())
        requete += ", 1)";
    else
        requete += ", null)";

    QSqlQuery InsertDocumentQuery (requete,db);
    proc->TraiteErreurRequete(InsertDocumentQuery, requete,tr("Erreur d'enregistrement du document dans ") + NOM_TABLE_COURRIERS);

    Remplir_TableView();

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
    line->setText(proc->MajusculePremiereLettre(line->text(), true, false, false));
    if (line->text().length() < 1)
    {
        UpMessageBox::Watch(0,tr("Creation de dossier"), tr("Veuillez renseigner le champ Résumé, SVP !"));
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
            " VALUES ('" + proc->CorrigeApostrophe(line->text().left(100)) +
            "'," + QString::number(gidUser);
    UpLabel *lbl = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(row,4));
    QString a = "null";
    if (lbl->pixmap())
        a = "1";
    requete += "," + a + ")";
    QSqlQuery InsertDossierQuery (requete,db);
    if (!proc->TraiteErreurRequete(InsertDossierQuery, requete, tr("Erreur d'enregistrement du dossier dans ") +  NOM_TABLE_METADOCUMENTS))
    {
        QStringList listdocs;
        QString idmetadoc;
        requete = "select idmetadocument from " NOM_TABLE_METADOCUMENTS " where ResumeMetadocument = '" + proc->CorrigeApostrophe(line->text().left(100)) + "'";
        QSqlQuery quer(requete,db);
        if (quer.size()>0)
        {
            quer.first();
            idmetadoc = quer.value(0).toString();
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
                QSqlQuery (requete,db);
            }
        }
    }
    Remplir_TableView();

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
            gOpacity = 1;
            EffaceWidget(ui->textFrame);
            ui->DocPubliccheckBox->setChecked(ui->DocupTableWidget->item(row,4)->text().toInt() == 1);
            ui->DocEditcheckBox->setChecked(ui->DocupTableWidget->item(row,8)->text().toInt() == 1);
            ui->PrescriptioncheckBox->setChecked(ui->DocupTableWidget->item(row,6)->text().toInt() == 1);
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

    int         userentete = (proc->UserSuperviseur()<1? proc->getListeSuperviseurs()->item(0)->text().toInt() : proc->UserSuperviseur());
    gDataUser = proc->setDataOtherUser(userentete);
    if (!gDataUser.value("Success").toBool())
        return;
//    gDataUser = proc->setDataOtherUser(proc->UserSuperviseur());
//    if (!gDataUser.value("Success").toBool())
//        return;
    QString req = "select patDDN, Sexe from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(gidPatient);
    QSqlQuery quer(req,db);
    if (proc->TraiteErreurRequete(quer,req,tr("Impossible de retrouver la date de naissance de ce patient")))
        texte.replace("{{DDN}}"                 ,"xx xx xxxx");
    quer.first();
    QString Sexe                        = quer.value(1).toString();
    QDate ddn                           = quer.value(0).toDate();
    QMap<QString,QVariant>  AgeTotal    = proc->CalculAge(ddn, QDate::currentDate(), Sexe);
    QString age                         = AgeTotal["Total"].toString();
    QString formule                     = AgeTotal["Formule"].toString();
    req = "select idcormedmg, cornom, corprenom, corsexe from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp, " NOM_TABLE_CORRESPONDANTS " cor where idPat = " + QString::number(gidPatient) + " and rmp.idcormedmg = cor.idcor";
    QSqlQuery quer2(req,db);

    texte.replace("{{" + DATEDOC + "}}"         , QDate::currentDate().toString(tr("d MMMM yyyy")));
    texte.replace("{{" + NOMPAT + "}},"         , gNomPat + ",");
    texte.replace("{{" + NOMPAT + "}} "         , gNomPat + " ");
    texte.replace("{{" + NOMPAT + "}}"          , gNomPat);
    texte.replace("{{" + PRENOMPAT + "}},"      , gPrenomPat + ",");
    texte.replace("{{" + PRENOMPAT + "}} "      , gPrenomPat + " ");
    texte.replace("{{" + PRENOMPAT + "}}"       , gPrenomPat);
    if (gDataUser["Titre"].toString() != "")
         texte.replace("{{" + TITRUSER + "}}"   , gDataUser["Titre"].toString() + " " + gDataUser["Prenom"].toString() + " " + gDataUser["Nom"].toString());
    else texte.replace("{{" + TITRUSER + "}}"   , gDataUser["Prenom"].toString() + " " + gDataUser["Nom"].toString());
    texte.replace("{{" + DDNPAT + "}}"                 ,quer.value(0).toDate().toString((tr("d MMMM yyyy"))));
    texte.replace("{{" + TITREPAT + "}} "       , formule + " ");
    texte.replace("{{" + TITREPAT + "}}"        , formule);
    texte.replace("{{" + AGEPAT + "}}"          , age);
    if (Sexe == "F")    texte.replace("(e)" ,"e");
    if (Sexe == "M")    texte.replace("(e)" ,"");

    if (quer2.size() == 0) {
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
        quer2.first();
        QString form = "", form2 = "";
        if (quer2.value(3).toString() == "F")
        {
            form = tr("Madame le docteur ");
            form2 = tr("Ma chère consoeur");
        }
        else if (quer2.value(3).toString() == "M")
        {
            form = tr("Monsieur le docteur ");
            form2 = tr("Mon cher confrère");
        }
        else
            form2 = tr("Ma chère consoeur, mon cher confrère");
        form += quer2.value(2).toString() + " " + quer2.value(1).toString();
        texte.replace("{{" + MGPAT + "}}"             ,form);
        texte.replace("{{" + POLITESSEMG + "}}"   ,form2);
        texte.replace("{{" + NOMMG + "}}}"                ,quer2.value(1).toString());
        texte.replace("{{" + PRENOMMG + "}}"              ,quer2.value(2).toString());
    }
    if (texte.contains("{{" + KERATO + "}}"))
    {
        req = "select K1OD, K2OD, AxeKOD, DioptrieK1OD, DioptrieK2OD, DioptrieKOD, K1OG, K2OG, AxeKOG, DioptrieK1OG, DioptrieK2OG, DioptrieKOG from " NOM_TABLE_DONNEES_OPHTA_PATIENTS
              " where idpat = " + QString::number(gidPatient) + " and (K1OD <> 'null' or K1OG <> 'null')";
        QSqlQuery Kerquery(req, db);
        if (Kerquery.size()>0)
        {
            Kerquery.last();
            QString kerato = "";
            if (Kerquery.value(0).toDouble()>0)
            {
                if (Kerquery.value(3).toDouble()!=0)
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOD:") + "</b></font> " + QString::number(Kerquery.value(0).toDouble(),'f',2) + "/" + QString::number(Kerquery.value(1).toDouble(),'f',2) + " Km = " + QString::number((Kerquery.value(0).toDouble() + Kerquery.value(1).toDouble())/2,'f',2) +
                              " - " + QString::number(Kerquery.value(3).toDouble(),'f',2) + "/" + QString::number(Kerquery.value(4).toDouble(),'f',2) + " " + QString::number(Kerquery.value(5).toDouble(),'f',2) +  " à " + Kerquery.value(2).toString() + "°</td></p>";
                else
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOD:") + "</b></font> " + QString::number(Kerquery.value(0).toDouble(),'f',2) + " à " + Kerquery.value(2).toString() + "°/" + QString::number(Kerquery.value(1).toDouble(),'f',2) + " Km = " + QString::number((Kerquery.value(0).toDouble() + Kerquery.value(1).toDouble())/2,'f',2) ;
            }
            if (Kerquery.value(0).toDouble()>0 && Kerquery.value(6).toDouble()>0)
                kerato += "<br/>";
            if (Kerquery.value(6).toDouble()>0)
            {
                if (Kerquery.value(9).toDouble()!=0)
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOG:") + "</b></font> " + QString::number(Kerquery.value(6).toDouble(),'f',2) + "/" +QString::number( Kerquery.value(7).toDouble(),'f',2) + " Km = " + QString::number((Kerquery.value(6).toDouble() + Kerquery.value(7).toDouble())/2,'f',2) +
                            " - " + QString::number(Kerquery.value(9).toDouble(),'f',2) + "/" + QString::number(Kerquery.value(10).toDouble(),'f',2) + " " + QString::number(Kerquery.value(11).toDouble(),'f',2) +  " à " + Kerquery.value(8).toString() + "°</td></p>";
                else
                    kerato += "<font color = " + proc->CouleurTitres + "><b>" + tr("KOG:") + "</b></font> " + QString::number(Kerquery.value(6).toDouble(),'f',2) + " à " + Kerquery.value(8).toString() + "°/" + QString::number(Kerquery.value(7).toDouble(),'f',2) + " Km = " + QString::number((Kerquery.value(6).toDouble() + Kerquery.value(7).toDouble())/2,'f',2) ;
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
        QSqlQuery Refquery(req, db);
        if (Refquery.size()>0)
        {
            Refquery.last();
            QString refract = "";
            if (Refquery.value(0).toString() != "")
                refract += "<font color = " + proc->CouleurTitres + "><b>" + tr("OD:") + "</b></font> " + Refquery.value(0).toString();
            if (Refquery.value(0).toString() != ""&& Refquery.value(1).toString() != "")
                refract += "<br />";
            if (Refquery.value(1).toString() != "")
                refract += "<font color = " + proc->CouleurTitres + "><b>" + tr("OG:") + "</b></font> " + Refquery.value(1).toString();
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
        QSqlQuery corquer(req,db);
        if (corquer.size()==0)
            texte.replace(reg,tr("PAS DE CORRESPONDANT RÉFÉRENCÉ POUR CE PATIENT"));
        else if (corquer.size()==1)
        {
            corquer.first();
            QString form = "", form2 = "";
            if (corquer.value(3).toString() == "F")
            {
                if (corquer.value(5).toInt() == 1)
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
            else if (corquer.value(3).toString() == "M")
            {
                if (corquer.value(5).toInt() == 1)
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

            form += corquer.value(2).toString() + " " + corquer.value(1).toString();
            texte.replace("{{" + CORPAT + "}}"             ,form);
            texte.replace("{{" + POLITESSECOR + "}}"   ,form2);
            texte.replace("{{" + NOMCOR + "}}}"                ,corquer.value(1).toString());
            texte.replace("{{" + PRENOMCOR + "}}"              ,corquer.value(2).toString());
        }
        else if (!pourVisu)
        {
            texte.replace("{{" + CORPAT + "}}"             ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
            texte.replace("{{" + POLITESSECOR + "}}"   ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
            texte.replace("{{" + NOMCOR + "}}}"                ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
            texte.replace("{{" + PRENOMCOR + "}}"              ,tr("CORRESPONDANTS À CHOISIR À L'IMPRESSION"));
        }
        else
        {
            ChoixCorrespondant(corquer);
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
            QSqlQuery txtquer(req,db);
            txtquer.first();
            for (int j=0; j<txtquer.size(); j++)
            {
                QString txtdef = texte;
                QString form = "", form2 = "";
                if (txtquer.value(3).toString() == "F")
                {
                    if (txtquer.value(5).toInt() == 1)
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
                else if (txtquer.value(3).toString() == "M")
                {
                    if (txtquer.value(5).toInt() == 1)
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

                form += txtquer.value(2).toString() + " " + txtquer.value(1).toString();
                txtdef.replace("{{" + CORPAT + "}}"             ,form);
                txtdef.replace("{{" + POLITESSECOR + "}}"   ,form2);
                txtdef.replace("{{" + NOMCOR + "}}}"                ,txtquer.value(1).toString());
                txtdef.replace("{{" + PRENOMCOR + "}}"              ,txtquer.value(2).toString());
                glisttxt << txtdef;
                txtquer.next();
            }
        }
    }
    if (glistidCor.size() == 0)
        glisttxt << texte;
}

void dlg_documents::ChoixCorrespondant(QSqlQuery quer)
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
    int hauteurligne            = fm.height()*1.1;

    label       ->setText(lbltxt);
    label       ->setAlignment(Qt::AlignCenter);

    tblCorresp  ->verticalHeader()->setVisible(false);
    tblCorresp  ->horizontalHeader()->setVisible(false);
    tblCorresp  ->setFocusPolicy(Qt::NoFocus);
    tblCorresp  ->setSelectionMode(QAbstractItemView::NoSelection);
    tblCorresp  ->setGridStyle(Qt::DotLine);
    tblCorresp  ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i=0; i<quer.size(); i++)
    {
        quer        .seek(i);
        pitem       = new QStandardItem(quer.value(1).toString() + " " + quer.value(2).toString());
        pitem       ->setAccessibleDescription(quer.value(0).toString());
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
// Remplissage de tree view avec les Documents de la base.
// ----------------------------------------------------------------------------------
void dlg_documents::Remplir_TableView()
{
    UpLineEdit          *upLine0;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem2;
    QTableWidgetItem    *pItem3;
    QTableWidgetItem    *pItem4;
    QTableWidgetItem    *pItem5;
    QTableWidgetItem    *pItem6;
    QTableWidgetItem    *pItem10;
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
        if (upLine0){
            disconnect(upLine0, 0,0, 0);
        }
    }

    ui->DocupTableWidget->clearContents();
    QString  Remplirtablerequete = "SELECT ResumeDocument, TextDocument, idDocument, DocPublic, idUser, Prescription, editable"
              " FROM "  NOM_TABLE_COURRIERS
              " WHERE (idUser = " + QString::number(gidUser);
    if (proc->getDataUser()["Fonction"].toString() == tr("Médecin") || proc->getDataUser()["Fonction"].toString() == tr("Orthoptiste"))
        Remplirtablerequete += " Or (DocPublic = 1 and iduser <> " + QString::number(gidUser) + ")";
    else
        Remplirtablerequete += " Or DocPublic = 1";
    Remplirtablerequete += ") and ResumeDocument LIKE '" + proc->CorrigeApostrophe(ui->ChercheupLineEdit->text()) + "%'";
    Remplirtablerequete += " ORDER BY ResumeDocument";

    QSqlQuery RemplirTableViewQuery (Remplirtablerequete,db);
    if (proc->TraiteErreurRequete(RemplirTableViewQuery, Remplirtablerequete,""))
        return;
    ui->DocupTableWidget->setRowCount(RemplirTableViewQuery.size());
    RemplirTableViewQuery.first();
    for (i = 0; i < RemplirTableViewQuery.size(); i++)
    {
        pItem1  = new QTableWidgetItem() ;
        upLine0 = new UpLineEdit() ;
        pItem2  = new QTableWidgetItem() ;
        pItem3  = new QTableWidgetItem() ;
        pItem4  = new QTableWidgetItem() ;
        pItem5  = new QTableWidgetItem() ;
        pItem6  = new QTableWidgetItem() ;
        pItem10  = new QTableWidgetItem() ;

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
        upLine0->setText(RemplirTableViewQuery.value(0).toString());                          // resume
        upLine0->setRowTable(i);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::NoFocus);
        if (RemplirTableViewQuery.value(4).toInt() != gidUser)
        {
            upLine0->setFont(disabledFont);
            upLine0->setPalette(palette);
            upLine0->setContextMenuPolicy(Qt::NoContextMenu);
        }
        else
            upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->DocupTableWidget->setCellWidget(i,col,upLine0);
        col++; //2
        pItem1->setText(RemplirTableViewQuery.value(1).toString());                           // text
        ui->DocupTableWidget->setItem(i,col,pItem1);
        col++; //3
        pItem2->setText(RemplirTableViewQuery.value(2).toString());                           // idDocument
        ui->DocupTableWidget->setItem(i,col,pItem2);
        col++; //4
        pItem3->setText(RemplirTableViewQuery.value(3).toString());                           // DocPublic
        ui->DocupTableWidget->setItem(i,col,pItem3);
        col++; //5
        pItem4->setText(RemplirTableViewQuery.value(4).toString());                           // idUser
        ui->DocupTableWidget->setItem(i,col,pItem4);
        col++; //6
        pItem5->setText(RemplirTableViewQuery.value(5).toString());                           // Prescription
        ui->DocupTableWidget->setItem(i,col,pItem5);
        col++; //7
        UpLabel*lbl = new UpLabel(ui->DocupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        if (RemplirTableViewQuery.value(3).toInt()==1)
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
        ui->DocupTableWidget->setCellWidget(i,col,lbl);                                       // Public
        col++; //8
        pItem6->setText(RemplirTableViewQuery.value(6).toString());                           // Editable
        ui->DocupTableWidget->setItem(i,col,pItem6);
        col++; //9
        UpLabel*lbl1 = new UpLabel(ui->DocupTableWidget);
        lbl1->setAlignment(Qt::AlignCenter);
        if (RemplirTableViewQuery.value(6).toInt()==1)
            lbl1->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
        ui->DocupTableWidget->setCellWidget(i,col,lbl1);
        col++; //10
        pItem10->setText("1" + upLine0->text());                                              // Check+text  -> sert pour le tri de la table
        ui->DocupTableWidget->setItem(i,col,pItem10);

        ui->DocupTableWidget->setRowHeight(i,fm.height()*1.3);

        RemplirTableViewQuery.next();
    }


    //Remplissage Table Dossiers
    for (int i = 0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        upLine0 = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (upLine0){
            disconnect(upLine0, 0, 0, 0);
        }
    }
    ui->DossiersupTableWidget->clearContents();
    QString  RemplirtableDossiersrequete =
                "SELECT ResumeMetaDocument, idMetaDocument, idUser, Public"
                " FROM "  NOM_TABLE_METADOCUMENTS
                " WHERE idUser = " + QString::number(gidUser);
    RemplirtableDossiersrequete +=
                " UNION \n";
    RemplirtableDossiersrequete +=
                "select ResumeMetaDocument, idMetaDocument, idUser, Public from " NOM_TABLE_METADOCUMENTS
                " where idMetaDocument not in\n"
                " (select met.idMetaDocument from " NOM_TABLE_METADOCUMENTS " as met, "
                NOM_TABLE_JOINTURESDOCS " as joi, "
                NOM_TABLE_COURRIERS " as doc\n"
                " where joi.idmetadocument = met.idMetaDocument\n"
                " and joi.idDocument = doc.iddocument\n"
                " and doc.docpublic is null)\n";
    RemplirtableDossiersrequete +=
                " ORDER BY ResumeMetaDocument;";
    //UpMessageBox::Watch(this,RemplirtableDossiersrequete);
    QSqlQuery RemplirDossiersTableViewQuery (RemplirtableDossiersrequete,db);
    if (proc->TraiteErreurRequete(RemplirDossiersTableViewQuery, RemplirtableDossiersrequete,""))
        return;

    ui->DossiersupTableWidget->setRowCount(RemplirDossiersTableViewQuery.size());
    RemplirDossiersTableViewQuery.first();
    for (i = 0; i < RemplirDossiersTableViewQuery.size(); i++)
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
        upLine0->setText(RemplirDossiersTableViewQuery.value(0).toString());                           // resume
        upLine0->setRowTable(i);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::NoFocus);
        if (RemplirDossiersTableViewQuery.value(2).toInt() != gidUser)
        {
            upLine0->setFont(disabledFont);
            upLine0->setPalette(palette);
            upLine0->setContextMenuPolicy(Qt::NoContextMenu);
        }
        else
            upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->DossiersupTableWidget->setCellWidget(i,col,upLine0);

        col++; //2
        pItem1->setText(RemplirDossiersTableViewQuery.value(1).toString());                           // idMetaDocument
        ui->DossiersupTableWidget->setItem(i,col,pItem1);

        col++; //3
        pItem2->setText(RemplirDossiersTableViewQuery.value(2).toString());                           // idUser
        ui->DossiersupTableWidget->setItem(i,col,pItem2);

        col++; //4
        UpLabel*lbl = new UpLabel(ui->DossiersupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        if (RemplirDossiersTableViewQuery.value(3).toInt()==1)
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //TODO : icon scaled : pxLoupe 15,15
        ui->DossiersupTableWidget->setCellWidget(i,col,lbl);

        ui->DossiersupTableWidget->setRowHeight(i,fm.height()*1.3);
        RemplirDossiersTableViewQuery.next();
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
    msgbox.setText("Euuhh... " + proc->getDataUser()["UserLogin"].toString() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer le document"));
    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != NoBouton)
    {
        int iddoc = ui->DocupTableWidget->item(row,3)->text().toInt();
        QString requete = "DELETE FROM  " NOM_TABLE_COURRIERS " WHERE idDocument = " + QString::number(iddoc);
        Msg = tr("Impossible de supprimer le document\n") + line->text().toUpper() + tr("\n ... et je ne sais pas pourquoi...");
        QSqlQuery SupprimeDocQuery (requete,db);
        proc->TraiteErreurRequete(SupprimeDocQuery,requete, Msg);
        Remplir_TableView();
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
    msgbox.setText("Euuhh... " + proc->getDataUser()["UserLogin"].toString() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer le dosssier"));
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != NoBouton)
    {
        int iddos = ui->DossiersupTableWidget->item(row,2)->text().toInt();
        QSqlQuery ("SET AUTOCOMMIT = 0;", proc->getDataBase());
        QString lockrequete = "LOCK TABLES " NOM_TABLE_METADOCUMENTS " WRITE, " NOM_TABLE_JOINTURESDOCS " WRITE;";
        QSqlQuery lockquery (lockrequete, proc->getDataBase());
        if (!proc->TraiteErreurRequete(lockquery,lockrequete,"Impossible de verrouiller " NOM_TABLE_METADOCUMENTS " et " NOM_TABLE_JOINTURESDOCS))
        {
            QString requete = "DELETE FROM  " NOM_TABLE_METADOCUMENTS " WHERE idMetaDocument = " + QString::number(iddos);
            Msg = tr("Impossible de suppprimer le dossier") + "\n" + line->text().toUpper() + "!\n ... " + tr("et je ne sais pas pourquoi") + "...\nRufus";
            QSqlQuery SupprimeDossQuery (requete,db);
            if (!proc->TraiteErreurRequete(SupprimeDossQuery,requete, Msg))
            {
                requete = "delete from " NOM_TABLE_JOINTURESDOCS " WHERE idMetaDocument = " + QString::number(iddos);
                SupprimeDossQuery = QSqlQuery(requete,db);
                proc->TraiteErreurRequete(SupprimeDossQuery,requete, Msg);
            }
            proc->commit(db);
            Remplir_TableView();
        }
        else
            proc->rollback(db);
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
void dlg_documents::
UpdateDocument(int row)
{
    // recherche de l'enregistrement modifié
    // controle validate des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(row,1));
    line->setText(proc->MajusculePremiereLettre(line->text(), true, false, false));
    if (line->text().length() < 1)    {
        UpMessageBox::Watch(0,tr("Modification de document"), tr("Veuillez renseigner le champ Résumé, SVP !"));
        return;
    }
    if (line->text() == tr("Nouveau document"))
    {
        UpMessageBox::Watch(0,tr("Création de document"), tr("Votre document ne peut pas s'appeler \"Nouveau document\""));
        return;
    }
    if (ui->upTextEdit->document()->toPlainText().length() < 1)   {
        UpMessageBox::Watch(0,tr("Modification de document"), tr("Veuillez renseigner le champ Document, SVP !"));
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
            " SET TextDocument = '" + proc->CorrigeApostrophe(ui->upTextEdit->toHtml())     + "'"
            ", ResumeDocument = '"  + proc->CorrigeApostrophe(line->text().left(100)) + "'";
    if (ui->DocPubliccheckBox->isChecked())     req += " , DocPublic = 1";      else req += " , DocPublic = null";
    if (ui->PrescriptioncheckBox->isChecked())  req += " , Prescription = 1";   else req += " , Prescription = null";
    if (ui->DocEditcheckBox->isChecked())       req += " , Editable = 1";       else req += " , Editable = null";
    req += " WHERE  idDocument = " + idAmodifier;
    QSqlQuery ModifDocQuery (req,db);
    proc->TraiteErreurRequete(ModifDocQuery, req, tr("Erreur de mise à jour du document dans ") + NOM_TABLE_COURRIERS);
    Remplir_TableView();

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
    line->setText(proc->MajusculePremiereLettre(line->text(), true, false, false));
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
        UpMessageBox::Watch(0,tr("Modification de Dossier"), tr("Veuillez cocher au moins un document, SVP !"));
        return;
    }

    QString iddoss  = ui->DossiersupTableWidget->item(row,2)->text();

    if (ChercheDoublon(line->text(), row))
    {
        line->setFocus();
        line->selectAll();
        return;
    }

    QSqlQuery("delete from " NOM_TABLE_JOINTURESDOCS " where idmetadocument = " + iddoss, db);

    QString req     = "insert into " NOM_TABLE_JOINTURESDOCS " (idmetadocument,iddocument) Values ";
    for (int i=0; i<listid.size(); i++)
    {
        req += "(" + iddoss + ", " + listid.at(i) + ")";
        if (i<listid.size()-1)
            req += ",";
    }
    QSqlQuery(req,db);

    req =   "UPDATE " NOM_TABLE_METADOCUMENTS
            " SET ResumeMetaDocument = '"  + proc->CorrigeApostrophe(line->text().left(100)) + "'"
            " WHERE  idmetaDocument = " + iddoss;
    QSqlQuery ModifDossierQuery (req,db);
    proc->TraiteErreurRequete(ModifDossierQuery, req, tr("Erreur de mise à jour du dossier dans ") + NOM_TABLE_METADOCUMENTS);

    Remplir_TableView();
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
    int iddoc = ui->DocupTableWidget->item(row,3)->text().toInt();
    QString req = "select idmetadocument, resumemetadocument from " NOM_TABLE_METADOCUMENTS
                  " where idmetadocument in (select idmetadocument from " NOM_TABLE_JOINTURESDOCS " where iddocument = " + QString::number(iddoc) +
                  ") and public =1";
    QSqlQuery quer(req,db);
    if (quer.size()>0)
    {
        if (msg)
        {
            quer.first();
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas rendre privé ce document"), tr("Il est incorporé dans le dossier public\n- ") + quer.value(1).toString() +
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
    for (int j=0; j<ui->DossiersupTableWidget->rowCount(); j++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(j,0));
        if (Widg)
        {
            UpCheckBox *DossCheck = Widg->findChildren<UpCheckBox*>().at(0);
            if (DossCheck->isChecked())
            {
                QString requete = "select idDocument from " NOM_TABLE_JOINTURESDOCS
                                  " where idMetaDocument = " + ui->DossiersupTableWidget->item(j,2)->text();
                QSqlQuery docsquer(requete,db);
                proc->TraiteErreurRequete(docsquer,requete,"");
                if (docsquer.size() > 0)
                {
                    QStringList listid;
                    docsquer.first();
                    for (int i=0; i<docsquer.size(); i++)
                    {
                        listid << docsquer.value(0).toString();
                        docsquer.next();
                    }
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
    int iddossier = ui->DossiersupTableWidget->item(row,2)->text().toInt();
    QString req = "select iddocument, resumedocument from " NOM_TABLE_COURRIERS
                  " where iddocument in (select iddocument from " NOM_TABLE_JOINTURESDOCS " where idmetadocument = " + QString::number(iddossier) +
                  ") and docpublic is null";
    QSqlQuery quer(req,db);
    if (quer.size()>0)
    {
        if (msg)
        {
            quer.first();
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas rendre public ce dossier.\nIl incorpore le document\n- ") +
                             quer.value(1).toString() + tr(" -\nqui est un document privé!"));
        }
        return false;
    }
    else return true;
}
