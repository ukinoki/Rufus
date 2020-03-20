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

#include "dlg_impressions.h"
#include "ui_dlg_impressions.h"

dlg_impressions::dlg_impressions(Patient *pat, Intervention *intervention, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_impressions)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    m_currentpatient     = pat;
    if (intervention != Q_NULLPTR)
        m_currentintervention = intervention;
    if (!pat->isalloaded())
        Datas::I()->patients->loadAll(pat, Item::Update);


    restoreGeometry(proc->settings()->value("PositionsFiches/PositionDocuments").toByteArray());
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    setWindowTitle(tr("Liste des documents prédéfinis"));
    ui->PrescriptioncheckBox->setVisible(currentuser()->isSoignant());
    wdg_docsbuttonframe     = new WidgetButtonFrame(ui->DocupTableWidget);
    wdg_docsbuttonframe     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    wdg_docsbuttonframe     ->layButtons()->insertWidget(0, ui->ChercheupLineEdit);
    wdg_docsbuttonframe     ->layButtons()->insertWidget(0, ui->label);
    wdg_dossiersbuttonframe = new WidgetButtonFrame(ui->DossiersupTableWidget);
    wdg_dossiersbuttonframe ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    ui->upTextEdit->disconnect(); // pour déconnecter la fonction MenuContextuel intrinsèque de la classe UpTextEdit

    connect (ui->ChercheupLineEdit,             &QLineEdit::textEdited,                 this,   [=] {FiltreListe();});
    connect (ui->OKupPushButton,                &QPushButton::clicked,                  this,   &dlg_impressions::Validation);
    connect (ui->AnnulupPushButton,             &QPushButton::clicked,                  this,   &dlg_impressions::Annulation);
    connect (ui->dateImpressiondateEdit,        &QDateEdit::dateChanged,                this,   [=] {
        if (currentdocument() != Q_NULLPTR)
        {
            MetAJour(currentdocument()->texte(),false);
            ui->upTextEdit                  ->setText(m_listtexts.at(0));
        }
    });
    connect (ui->DocPubliccheckBox,             &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocPubliccheckBox);});
    connect (ui->DocEditcheckBox,               &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocEditcheckBox);});
    connect (ui->DocAdministratifcheckBox,      &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocAdministratifcheckBox);});
    connect (ui->PrescriptioncheckBox,          &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->PrescriptioncheckBox);});
    connect (ui->upTextEdit,                    &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel(ui->upTextEdit);});
    connect (ui->upTextEdit,                    &QTextEdit::textChanged,                this,   [=] {EnableOKPushButton();});
    connect (ui->upTextEdit,                    &UpTextEdit::dblclick,                  this,   &dlg_impressions::dblClicktextEdit);
    connect (ui->DupliOrdocheckBox,             &QCheckBox::clicked,                    this,   [=] {OrdoAvecDupli(ui->DupliOrdocheckBox->isChecked());});
    connect (wdg_docsbuttonframe,               &WidgetButtonFrame::choix,              this,   [=] {ChoixButtonFrame(wdg_docsbuttonframe);});
    connect (wdg_dossiersbuttonframe,           &WidgetButtonFrame::choix,              this,   [=] {ChoixButtonFrame(wdg_dossiersbuttonframe);});

    // Mise en forme de la table Documents
    ui->DocupTableWidget->setPalette(QPalette(Qt::white));
    ui->DocupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->DocupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DocupTableWidget->verticalHeader()->setVisible(false);
    ui->DocupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->DocupTableWidget->setColumnCount(7);
    ui->DocupTableWidget->setColumnWidth(0,30);         // checkbox
    ui->DocupTableWidget->setColumnWidth(1,289);        // Resume
    ui->DocupTableWidget->setColumnHidden(2,true);      // idDocument
    ui->DocupTableWidget->setColumnWidth(3,30);         // Public   - affiche un check si document public
    ui->DocupTableWidget->setColumnWidth(4,30);         // Editable - affiche un check si document editable
    ui->DocupTableWidget->setColumnWidth(5,30);         // Medical  - affiche un check si document medical
    ui->DocupTableWidget->setColumnHidden(6,true);
    ui->DocupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Icons::icImprimer(),""));
    ui->DocupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("TITRES DES DOCUMENTS")));
    ui->DocupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    QTableWidgetItem *docpublic, *editable,*admin;
    docpublic   = new QTableWidgetItem(Icons::icFamily(),"");
    docpublic   ->setToolTip(tr("Document disponible pour tous les utilisateurs"));
    editable    = new QTableWidgetItem(Icons::icEditer(),"");
    editable    ->setToolTip(tr("Document éditable au moment de l'impression"));
    admin       = new QTableWidgetItem(Icons::icAdministratif(),"");
    admin       ->setToolTip(tr("Document administratif"));
    ui->DocupTableWidget->setHorizontalHeaderItem(3, docpublic);
    ui->DocupTableWidget->setHorizontalHeaderItem(4, editable);
    ui->DocupTableWidget->setHorizontalHeaderItem(5,admin);
    ui->DocupTableWidget->setHorizontalHeaderItem(6, new QTableWidgetItem(""));
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
    ui->DossiersupTableWidget->setColumnWidth(0,30);        // checkbox
    ui->DossiersupTableWidget->setColumnWidth(1,259);       // Resume
    ui->DossiersupTableWidget->setColumnWidth(2,30);        // Public
    ui->DossiersupTableWidget->setColumnHidden(3,true);     // idDocument
    ui->DossiersupTableWidget->setColumnHidden(4,true);     // Resume dans un QtableWidgetItem
    ui->DossiersupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Icons::icImprimer(),""));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("DOSSIERS")));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(Icons::icFamily(),""));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->DossiersupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(""));
    ui->DossiersupTableWidget->horizontalHeader()->setVisible(true);
    ui->DossiersupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->DossiersupTableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    ui->DossiersupTableWidget->horizontalHeader()->setIconSize(QSize(25,25));
    ui->DossiersupTableWidget->FixLargeurTotale();

    QHBoxLayout *doclay = new QHBoxLayout();
    doclay      ->addWidget(wdg_dossiersbuttonframe->widgButtonParent());
    doclay      ->addWidget(wdg_docsbuttonframe->widgButtonParent());
    doclay      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding));
    int marge   = 0;
    int space   = 5;
    doclay      ->setContentsMargins(marge,marge,marge,marge);
    doclay      ->setSpacing(space);
    ui->widget  ->setLayout(doclay);

    ui->AnnulupPushButton->setUpButtonStyle(UpPushButton::ANNULBUTTON, UpPushButton::Mid);
    ui->OKupPushButton->setUpButtonStyle(UpPushButton::OKBUTTON, UpPushButton::Mid);
    ui->AnnulupPushButton->setShortcut(QKeySequence("F12"));

    ui->OKupPushButton->setIcon(Icons::icImprimer());
    ui->OKupPushButton->setIconSize(QSize(30,30));

    ui->dateImpressiondateEdit->setDate(QDate::currentDate());
    ui->dateImpressiondateEdit->setMaximumDate(QDate::currentDate());

    ui->DupliOrdocheckBox->setChecked(proc->settings()->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES");
    ui->label->setPixmap(Icons::pxLoupe().scaled(30,30)); //WARNING : icon scaled : pxLoupe 20,20
    ui->ChercheupLineEdit->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 3px solid rgb(164, 205, 255);border-radius: 10px;}");


    ui->textFrame->installEventFilter(this);
    ui->DocupTableWidget->installEventFilter(this);
    ui->DossiersupTableWidget->installEventFilter(this);
    m_opacityeffect             = new QGraphicsOpacityEffect();
    t_timerefface    = new QTimer(this);

    ui->ALDcheckBox->setChecked(m_currentpatient->isald());

    //nettoyage de la table metadocs
    db->StandardSQL("delete from " TBL_JOINTURESIMPRESSIONS " where " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " not in (select " CP_ID_IMPRESSIONS " from " TBL_IMPRESSIONS ")");

    Remplir_TableWidget();
    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
        ConfigMode(Selection);
    ui->ChercheupLineEdit->setFocus();

    map_champs[TITRUSER]        = tr("Titre, nom et prénom de l'utilisateur");
    map_champs[NOMPAT]          = tr("Nom du patient");
    map_champs[DATEDOC]         = tr("Date du jour");
    map_champs[DDNPAT]          = tr("Date de naissance");
    map_champs[TITREPAT]        = tr("Titre du patient");
    map_champs[AGEPAT]          = tr("Âge du patient");
    map_champs[PRENOMPAT]       = tr("Prénom du patient");
    map_champs[MGPAT]           = tr("Médecin du patient");
    map_champs[MGPATTITRE]      = tr("Titre médecin du patient");
    map_champs[POLITESSEMG]     = tr("formule de politesse médecin patient");
    map_champs[PRENOMMG]        = tr("Prénom du médecin");
    map_champs[NOMMG]           = tr("Nom du médecin");
    map_champs[REFRACT]         = tr("Refraction du patient'");
    map_champs[KERATO]          = tr("Keratométrie du patient");
    map_champs[CORPAT]          = tr("Correspondant du patient");
    map_champs[POLITESSECOR]    = tr("formule de politesse correspondant");
    map_champs[PRENOMCOR]       = tr("Prénom du correspondant");
    map_champs[NOMCOR]          = tr("Nom du correspondant");
    map_champs[TELEPHONE]       = tr("Téléphone du patient");
    map_champs[SEXEPAT]         = tr("Sexe du patient");
}

dlg_impressions::~dlg_impressions()
{
}

QMap<int, QMap<dlg_impressions::DATASAIMPRIMER, QString> > dlg_impressions::mapdocsaimprimer() const
{
    return map_docsaimprimer;
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton ANNULER.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_impressions::Annulation()
{
    if (m_mode == CreationDOC || m_mode == ModificationDOC || m_mode == ModificationDOSS || m_mode == CreationDOSS)
    {
        QString TableAmodifier = "";
        int     row = -1;
        UpLineEdit *line;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->focusPolicy() == Qt::WheelFocus)
            {
                row = line->Row();
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
                    row = line->Row();
                    TableAmodifier = "Dossiers";
                    break;
                }
            }
        }
        if (m_mode == CreationDOC || m_mode == CreationDOSS)
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

void dlg_impressions::ChoixButtonFrame(WidgetButtonFrame *widgbutt)
{
    UpLineEdit *line = new UpLineEdit();
    int row = 0;
    if (widgbutt== wdg_docsbuttonframe)
    {
        switch (wdg_docsbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) {row = line->Row(); break;}
            }
            ConfigMode(CreationDOC, row);
            break;
        case WidgetButtonFrame::Modifier:
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            ConfigMode(ModificationDOC,line->Row());
            break;
        case WidgetButtonFrame::Moins:
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            DisableLines();
            SupprimmDocument(line->Row());
            break;
        }
    }
    else if (widgbutt== wdg_dossiersbuttonframe)
    {
        switch (wdg_dossiersbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) {row = line->Row(); break;}
            }
            ConfigMode(CreationDOSS, row);
            break;
        case WidgetButtonFrame::Modifier:
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            ConfigMode(ModificationDOSS,line->Row());
            break;
        case WidgetButtonFrame::Moins:
            for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
            {
                line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
                if (line->hasSelectedText()) break;
            }
            SupprimmDossier(line->Row());
            break;
        }
    }
}

void dlg_impressions::CheckPublicEditablAdmin(QCheckBox *check)
{
    UpLineEdit *line = new UpLineEdit(this);
    bool a = false;
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)  {
        line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line->isEnabled()) {a = true; break;}
    }
    if (!a)
        return;

    int colonnelbl (-1);
    if (check == ui->DocPubliccheckBox)
        colonnelbl = 3;
    else if (check == ui->DocEditcheckBox)
        colonnelbl = 4;
    else if (check == ui->DocAdministratifcheckBox || check == ui->PrescriptioncheckBox)
        colonnelbl = 5;
    UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->Row(),colonnelbl));
    if (check == ui->DocPubliccheckBox || check == ui->DocEditcheckBox)
    {
        if (check->isChecked())
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        else
            lbl->clear();
    }
    else if (check == ui->DocAdministratifcheckBox)
    {
        if (check->isChecked())
        {
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
            ui->PrescriptioncheckBox->setChecked(false);
        }
        else
            lbl->clear();
    }
    else if (check == ui->PrescriptioncheckBox)
    {
        if (check->isChecked())
        {
            lbl->clear();
            ui->DocAdministratifcheckBox->setChecked(false);
        }
    }
}

void dlg_impressions::dblClicktextEdit()
{
    if (m_mode == Selection)
    {
        UpLineEdit *line;
        int row = 0;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText())
            {
                row = line->Row();
                break;
            }
        }
        if (getDocumentFromRow(row)->iduser() == currentuser()->id())
            ConfigMode(ModificationDOC,row);
    }
}

// ----------------------------------------------------------------------------------
// On entre sur une ligne de comm. On affiche le tooltip
// ----------------------------------------------------------------------------------
void dlg_impressions::DocCellEnter(UpLineEdit *line)
{
    QPoint pos = cursor().pos();
    QRect rect = QRect(pos,QSize(10,10));
    int row = line->Row();

    if (ui->DocupTableWidget->isAncestorOf(line))
    {
        //    QRect rect = QRect(itemselect->tableWidget()->pos(),itemselect->tableWidget()->size());
        QTextEdit *text = new QTextEdit;
        MetAJour(getDocumentFromRow(row)->texte(),false);
        text->setText(m_listtexts.at(0));
        QString ab = text->toPlainText();
        ab.replace(QRegExp("\n\n[\n]*"),"\n");
        while (ab.endsWith("\n"))
            ab = ab.left(ab.size()-1);
        if (ab.size()>300)
        {
            ab = ab.left(300);
            while (ab.endsWith("\n"))
                ab = ab.left(ab.size()-1);
            if (!ab.endsWith("."))
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
        QString req = "select " CP_RESUME_IMPRESSIONS " from " TBL_IMPRESSIONS
                " where " CP_ID_IMPRESSIONS " in (select " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " from " TBL_JOINTURESIMPRESSIONS
                " where " CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " = " + QString::number(getMetaDocumentFromRow(row)->id()) + ")";
        //UpMessageBox::Watch(this,req);
        QList<QVariantList> listdocs = db->StandardSelectSQL(req,ok);
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
void dlg_impressions::DocCellDblClick(UpLineEdit *line)
{
    if (ui->DocupTableWidget->isAncestorOf(line))
        ConfigMode(ModificationDOC, line->Row());
    else if (ui->DossiersupTableWidget->isAncestorOf(line))
        ConfigMode(ModificationDOSS, line->Row());
}

// ----------------------------------------------------------------------------------
// Enable OKpushbutton
// ----------------------------------------------------------------------------------
void dlg_impressions::EnableOKPushButton(UpCheckBox *Check)
{
    ui->OKupPushButton->setShortcut(QKeySequence());
    if (m_mode == CreationDOC || m_mode == ModificationDOC)
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
    else if (m_mode == CreationDOSS || m_mode == ModificationDOSS)
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
    else if (m_mode == Selection)
    {
        if (Check != Q_NULLPTR)
        {
            if (ui->DossiersupTableWidget->isAncestorOf(Check))
            {
                bool A = Check->isChecked();
               CocheLesDocs(getMetaDocumentFromRow(Check->rowTable())->id(),A);
            }
            if (ui->DocupTableWidget->isAncestorOf(Check))
            {
                QString nomdoc = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(Check->rowTable(),1))->text();
                if (!Check->isChecked())
                {
                    VerifDossiers();
                    ui->DocupTableWidget->item(Check->rowTable(),6)->setText("1" + nomdoc);
                }
                else
                    ui->DocupTableWidget->item(Check->rowTable(),6)->setText("0" + nomdoc);
            }
        }
        if (ui->ChercheupLineEdit->text() == "")
            m_listid.clear();
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                if(Check->isChecked())
                    m_listid << QString::number(getDocumentFromRow(i)->id());
                else
                {
                    for (int k=0; k<m_listid.size();k++){
                        if (m_listid.at(k) == QString::number(getDocumentFromRow(Check->rowTable())->id())){
                            m_listid.removeAt(k);
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
        ui->OKupPushButton->setShortcut(QKeySequence("Meta+Return"));
    }
}

void dlg_impressions::FiltreListe()
{
    Remplir_TableWidget();
    EnableLines();
    bool selectall = false;
    for (int j=0; j<ui->DocupTableWidget->rowCount(); j++)
    {
        UpLineEdit *line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(j,1));
        QString txt = line->text();
        QString txtachercher = ui->ChercheupLineEdit->text();
        int lgth = txtachercher.length();
        ui->DocupTableWidget->setRowHidden(j, txt.toUpper().left(lgth) != txtachercher.toUpper());
        if (!selectall && txt.toUpper().left(lgth) == txtachercher.toUpper())
        {
            dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(j,1))->selectAll();
            selectall= true;
        }
    }
}

void dlg_impressions::MenuContextuel(QWidget *widg)
{
    m_menucontextuel = new QMenu(this);
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
    QAction *pAction_InsInterroDate;
    QAction *pAction_InsInterroCote;
    QAction *pAction_InsInterroHeure;
    QAction *pAction_InsInterroMontant;
    QAction *pAction_InsInterroMedecin;
    QAction *pAction_InsInterroAnesthIntervention;
    QAction *pAction_InsInterroProvenance;
    QAction *pAction_InsInterroDateIntervention;
    QAction *pAction_InsInterroHeureIntervention;
    QAction *pAction_InsInterroSiteIntervention;
    QAction *pAction_InsInterroCoteIntervention;
    QAction *pAction_InsInterroTypeIntervention;
    QAction *pAction_InsInterroSejour;
    QAction *pAction_InsInterroSite;
    QAction *pAction_InsInterroText;
    QMenu *interro = new QMenu(this);
    UpLineEdit *line0;
    UpLineEdit *line = dynamic_cast<UpLineEdit*>(widg);

    if (line)
    {
        int row = line->Row();
        if (ui->DossiersupTableWidget->isAncestorOf(line))
        {
            LineSelect(ui->DossiersupTableWidget,row);

            pAction_CreerDossier            = m_menucontextuel->addAction(Icons::icCreer(), tr("Créer un dossier")) ;
            pAction_ModifDossier            = m_menucontextuel->addAction(Icons::icEditer(), tr("Modifier ce dossier")) ;
            pAction_SupprDossier            = m_menucontextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce dossier")) ;
            m_menucontextuel->addSeparator();
            UpLabel *lbl                    = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(line->Row(),4));
            if (lbl->pixmap() != Q_NULLPTR)
                pAction_PublicDossier       = m_menucontextuel->addAction(Icons::icBlackCheck(), tr("Public")) ;
            else
                pAction_PublicDossier       = m_menucontextuel->addAction(tr("Public")) ;

            connect (pAction_ModifDossier,  &QAction::triggered,    this, [=] {ChoixMenuContextuel("ModifierDossier");});
            connect (pAction_SupprDossier,  &QAction::triggered,    this, [=] {ChoixMenuContextuel("SupprimerDossier");});
            connect (pAction_CreerDossier,  &QAction::triggered,    this, [=] {ChoixMenuContextuel("CreerDossier");});
            connect (pAction_PublicDossier, &QAction::triggered,    this, [=] {ChoixMenuContextuel("PublicDossier");});
        }
        else if (ui->DocupTableWidget->isAncestorOf(line))
        {
            LineSelect(ui->DocupTableWidget,line->Row());

            pAction_ModifDoc                = m_menucontextuel->addAction(Icons::icEditer(), tr("Modifier ce document"));
            pAction_SupprDoc                = m_menucontextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce document"));
            pAction_CreerDoc                = m_menucontextuel->addAction(Icons::icCreer(), tr("Créer un document"));
            if (getDocumentFromRow(row)->ispublic())
                pAction_PublicDoc           = m_menucontextuel->addAction(Icons::icBlackCheck(), tr("Public"));
            else
                pAction_PublicDoc           = m_menucontextuel->addAction(tr("Public"));
            if (Datas::I()->users->userconnected()->isMedecin() || Datas::I()->users->userconnected()->isOrthoptist())
            {
                if (getDocumentFromRow(row)->isprescription())
                    pAction_PrescripDoc         = m_menucontextuel->addAction(Icons::icBlackCheck(), tr("Prescription"));
                else
                    pAction_PrescripDoc         = m_menucontextuel->addAction(tr("Prescription"));
                pAction_PrescripDoc ->setToolTip(tr("si cette option est cochée\nce document sera considéré comme une prescription"));
                connect (pAction_PrescripDoc,   &QAction::triggered,this, [=] {ChoixMenuContextuel("PrescripDoc");});
            }
            if (getDocumentFromRow(row)->iseditable())
                pAction_EditableDoc         = m_menucontextuel->addAction(Icons::icBlackCheck(), tr("Editable"));
            else
                pAction_EditableDoc         = m_menucontextuel->addAction(tr("Editable"));
            if (Datas::I()->users->userconnected()->isMedecin() || Datas::I()->users->userconnected()->isOrthoptist())
            {
                if (!getDocumentFromRow(row)->ismedical())
                    pAction_AdminDoc        = m_menucontextuel->addAction(Icons::icBlackCheck(), tr("Document administratif"));
                else
                    pAction_AdminDoc        = m_menucontextuel->addAction(tr("Document administratif"));
                pAction_AdminDoc    ->setToolTip(tr("si cette option est cochée\nle document est considéré comme un document non médical"));
                connect (pAction_AdminDoc,      &QAction::triggered,this, [=] {ChoixMenuContextuel("AdminDoc");});
            }

            pAction_PublicDoc   ->setToolTip(tr("si cette option est cochée\ntous les utilisateurs\nauront accès à ce document"));
            pAction_EditableDoc ->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));

            connect (pAction_ModifDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuel("ModifierDoc");});
            connect (pAction_SupprDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuel("SupprimerDoc");});
            connect (pAction_CreerDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuel("CreerDoc");});
            connect (pAction_PublicDoc,     &QAction::triggered,    this, [=] {ChoixMenuContextuel("PublicDoc");});
            connect (pAction_EditableDoc,   &QAction::triggered,    this, [=] {ChoixMenuContextuel("EditDoc");});
        }
    }

    else if (m_mode == Selection)
    {
        bool a = false;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line0 = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line0->hasSelectedText())
                if (getDocumentFromRow(line0->Row())->id() == currentuser()->id())
                {a =true; break;}
        }
        if (a)
        {
            pAction_ModifDoc       = m_menucontextuel->addAction(Icons::icEditer(), tr("Modifier ce document"));
            connect (pAction_ModifDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuel("ModifierDoc");});
        }
    }
    else if (widg == ui->upTextEdit)
    {
        pAction_InsertChamp                 = m_menucontextuel->addAction   (Icons::icAjouter(),    tr("Insérer un champ"));
        interro                             = m_menucontextuel->addMenu     (Icons::icAjouter(),    tr("Insérer une interrogation"));
        pAction_InsInterroDate              = interro->addAction            (Icons::icDate(),       tr("Date"));
        pAction_InsInterroHeure             = interro->addAction            (Icons::icClock(),      tr("Heure"));
        pAction_InsInterroCote              = interro->addAction            (Icons::icSide(),       tr("Côté"));
        pAction_InsInterroMontant           = interro->addAction            (Icons::icEuro(),       tr("Montant"));
        pAction_InsInterroMedecin           = interro->addAction            (Icons::icStetho(),     tr("Soignant"));
        pAction_InsInterroProvenance        = interro->addAction            (Icons::icFamily(),     tr("Provenance"));
        pAction_InsInterroSejour            = interro->addAction            (Icons::icInformation(),tr("Séjour"));
        pAction_InsInterroSite              = interro->addAction            (Icons::icClinic(),     tr("Centre"));
        pAction_InsInterroText              = interro->addAction            (Icons::icMedoc(),      tr("Texte libre"));
        pAction_InsInterroDateIntervention  = interro->addAction            (Icons::icDate(),       TITREDATEINTERVENTION);
        pAction_InsInterroAnesthIntervention= interro->addAction            (Icons::icStetho(),     TITREANESTHINTERVENTION);
        pAction_InsInterroHeureIntervention = interro->addAction            (Icons::icClock(),      TITREHEUREINTERVENTION);
        pAction_InsInterroCoteIntervention  = interro->addAction            (Icons::icSide(),       TITRECOTEINTERVENTION);
        pAction_InsInterroTypeIntervention  = interro->addAction            (Icons::icMedoc(),      TITRETYPEINTERVENTION);
        pAction_InsInterroSiteIntervention  = interro->addAction            (Icons::icClinic(),     TITRESITEINTERVENTION);

        m_menucontextuel->addSeparator();
        if (ui->upTextEdit->textCursor().selectedText().size() > 0)   {
            pAction_ModifPolice     = m_menucontextuel->addAction(Icons::icFont(),           tr("Modifier la police"));
            pAction_Fontbold        = m_menucontextuel->addAction(Icons::icFontbold(),       tr("Gras"));
            pAction_Fontitalic      = m_menucontextuel->addAction(Icons::icFontitalic(),     tr("Italique"));
            pAction_Fontunderline   = m_menucontextuel->addAction(Icons::icFontunderline(),  tr("Souligné"));
            pAction_Fontnormal      = m_menucontextuel->addAction(Icons::icFontnormal(),     tr("Normal"));

            connect (pAction_ModifPolice,       &QAction::triggered,    this, [=] {ChoixMenuContextuel("Police");});
            connect (pAction_Fontbold,          &QAction::triggered,    this, [=] {ChoixMenuContextuel("Gras");});
            connect (pAction_Fontitalic,        &QAction::triggered,    this, [=] {ChoixMenuContextuel("Italique");});
            connect (pAction_Fontunderline,     &QAction::triggered,    this, [=] {ChoixMenuContextuel("Souligne");});
            connect (pAction_Fontnormal,        &QAction::triggered,    this, [=] {ChoixMenuContextuel("Normal");});
            m_menucontextuel->addSeparator();
        }
        pAction_Blockleft       = m_menucontextuel->addAction(Icons::icBlockLeft(),          tr("Aligné à gauche"));
        pAction_Blockright      = m_menucontextuel->addAction(Icons::icBlockRight(),         tr("Aligné à droite"));
        pAction_Blockcentr      = m_menucontextuel->addAction(Icons::icBlockCenter(),        tr("Centré"));
        pAction_Blockjust       = m_menucontextuel->addAction(Icons::icBlockJustify(),       tr("Justifié"));
        m_menucontextuel->addSeparator();
        if (ui->upTextEdit->textCursor().selectedText().size() > 0)   {
            pAction_Copier          = m_menucontextuel->addAction(Icons::icCopy(),   tr("Copier"));
            pAction_Cut             = m_menucontextuel->addAction(Icons::icCut(),    tr("Couper"));
            connect (pAction_Copier,            &QAction::triggered,    this, [=] {ChoixMenuContextuel("Copier");});
            connect (pAction_Cut,               &QAction::triggered,    this, [=] {ChoixMenuContextuel("Couper");});
        }
        if (qApp->clipboard()->mimeData()->hasText()
                || qApp->clipboard()->mimeData()->hasUrls()
                || qApp->clipboard()->mimeData()->hasImage()
                || qApp->clipboard()->mimeData()->hasHtml())
        {
            QAction *pAction_Coller = m_menucontextuel->addAction(Icons::icPaste(),  tr("Coller"));
            connect (pAction_Coller,        &QAction::triggered,    this,    [=] {ChoixMenuContextuel("Coller");});
        }

        connect (pAction_InsertChamp,                   &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Inserer");});
        connect (pAction_InsInterroDate,                &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Date");});
        connect (pAction_InsInterroCote,                &QAction::triggered,    this,   [=] {ChoixMenuContextuel(COTE);});
        connect (pAction_InsInterroHeure,               &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Heure");});
        connect (pAction_InsInterroMontant,             &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Montant");});
        connect (pAction_InsInterroMedecin,             &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Soignant");});
        connect (pAction_InsInterroProvenance,          &QAction::triggered,    this,   [=] {ChoixMenuContextuel(PROVENANCE);});
        connect (pAction_InsInterroSejour,              &QAction::triggered,    this,   [=] {ChoixMenuContextuel(TYPESEJOUR);});
        connect (pAction_InsInterroSite,                &QAction::triggered,    this,   [=] {ChoixMenuContextuel(SITE);});
        connect (pAction_InsInterroText,                &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Texte");});
        connect (pAction_Blockcentr,                    &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Centre");});
        connect (pAction_Blockright,                    &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Droite");});
        connect (pAction_Blockleft,                     &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Gauche");});
        connect (pAction_Blockjust,                     &QAction::triggered,    this,   [=] {ChoixMenuContextuel("Justifie");});
        connect (pAction_InsInterroDateIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuel(DATEINTERVENTION);});
        connect (pAction_InsInterroHeureIntervention,   &QAction::triggered,    this,   [=] {ChoixMenuContextuel(HEUREINTERVENTION);});
        connect (pAction_InsInterroCoteIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuel(COTEINTERVENTION);});
        connect (pAction_InsInterroTypeIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuel(TYPEINTERVENTION);});
        connect (pAction_InsInterroSiteIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuel(SITEINTERVENTION);});
        connect (pAction_InsInterroAnesthIntervention,  &QAction::triggered,    this,   [=] {ChoixMenuContextuel(ANESTHINTERVENTION);});
    }

    // ouvrir le menu
    m_menucontextuel->exec(cursor().pos());
    delete m_menucontextuel;
}

void dlg_impressions::ChoixMenuContextuel(QString choix)
{
    bool a = false;
    QPoint pos = ui->DocupTableWidget->viewport()->mapFromGlobal(m_menucontextuel->pos());
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
        ConfigMode(ModificationDOC,line->Row());
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
        SupprimmDocument(line->Row());
    }
    else if (choix  == "ModifierDossier")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a = true; break;}
        }
        if (a == false) return;
        ConfigMode(ModificationDOSS,line->Row());
    }
    else if (choix  == "PublicDoc")
    {
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a = true; break;}
        }
        if (!a) return;
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->Row(),3));
        Impression *impr = getDocumentFromRow(line->Row());
        QString b = "null";
        if (lbl->pixmap() == Q_NULLPTR)
        {
            b = "1";
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        }
        else
        {
            if (!VerifDocumentPublic(line->Row())) return;
            lbl->clear();
        }
        ui->DocPubliccheckBox->toggle();
        if (m_mode == Selection)
        {
            db->StandardSQL("update " TBL_IMPRESSIONS " set " CP_DOCPUBLIC_IMPRESSIONS " = " + b + " where " CP_ID_IMPRESSIONS " = " +
                            QString::number(impr->id()));
            impr->setpublic(b == "1");
        }
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
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->Row(),4));
        Impression *impr = getDocumentFromRow(line->Row());
        QString b = "null";
        if (lbl->pixmap() == Q_NULLPTR)
        {
            b = "1";
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
        }
        else
            lbl->clear();
        ui->DocEditcheckBox->toggle();
        if (m_mode == Selection)
        {
            db->StandardSQL("update " TBL_IMPRESSIONS " set " CP_EDITABLE_IMPRESSIONS " = " + b + " where " CP_ID_IMPRESSIONS " = " +
                            QString::number(impr->id()));
            impr->seteditable(b == "1");
        }
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
        UpLabel *lbl = static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->Row(),5));
        Impression *impr = getDocumentFromRow(line->Row());
        impr->setmedical(!impr->ismedical());
        if (impr->ismedical())
            lbl->clear();                                           //! le document était administratif et devient médical
        else
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15));    //! le document était médical et devient administratif
        ui->DocAdministratifcheckBox->setChecked(!impr->ismedical());
        QString val = (impr->ismedical()? "1" : "null");
        db->StandardSQL("update " TBL_IMPRESSIONS " set " CP_MEDICAL_IMPRESSIONS " = " + val + " where " CP_ID_IMPRESSIONS " = " +  QString::number(impr->id()));
        if (impr->isprescription() && !impr->ismedical())
        {
            impr->setprescription(false);
            db->StandardSQL("update " TBL_IMPRESSIONS " set " CP_PRESCRIPTION_IMPRESSIONS " = null where " CP_ID_IMPRESSIONS " = " +
                            QString::number(impr->id()));
            ui->PrescriptioncheckBox->setChecked(false);
        }
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
        UpLabel *lbl = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(line->Row(),2));
        QString b = "null";
        if (lbl->pixmap() == Q_NULLPTR)
        {
            if (!VerifDossierPublic(line->Row())) return;
            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxLoupe 15,15
            b = "1";
        }
        else
            lbl->clear();
        if (m_mode == Selection)
            db->StandardSQL("update " TBL_DOSSIERSIMPRESSIONS " set " CP_PUBLIC_DOSSIERIMPRESSIONS " = " + b + " where idMetaDocument = " +
                       QString::number(getMetaDocumentFromRow(line->Row())->id()));
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
        Impression *impr = getDocumentFromRow(line->Row());
        ui->PrescriptioncheckBox->toggle();
        QString b = "null";
        if (ui->PrescriptioncheckBox->isChecked())
            b = "1";
        if (m_mode == Selection)
        {
            db->StandardSQL("update " TBL_IMPRESSIONS " set " CP_PRESCRIPTION_IMPRESSIONS " = " + b + " where " CP_ID_IMPRESSIONS " = " +
                            QString::number(impr->id()));
            impr->setprescription(ui->PrescriptioncheckBox->isChecked());
            if (impr->isprescription() && !impr->ismedical())
            {
                impr->setmedical(true);
                db->StandardSQL("update " TBL_IMPRESSIONS " set " CP_MEDICAL_IMPRESSIONS " = 1 where " CP_ID_IMPRESSIONS " = " +
                                QString::number(impr->id()));
                ui->DocAdministratifcheckBox->setChecked(false);
                static_cast<UpLabel*>(ui->DocupTableWidget->cellWidget(line->Row(),5))->clear();
            }
        }
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
        SupprimmDossier(line->Row());
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
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignLeft);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Justifie")    {
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignJustify);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Droite")    {
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignRight);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Centre")    {
        QTextBlockFormat blockformat  = ui->upTextEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignCenter);
        ui->upTextEdit->textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Inserer")   {
        UpDialog        *ListChamps     = new UpDialog(this);
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
        for(auto it = map_champs.cbegin(); it!=map_champs.cend(); ++it)
        {
            tabChamps   ->insertRow(i);
            pitem0       = new QTableWidgetItem;
            pitem0       ->setText(map_champs[it.key()]);
            tabChamps   ->setItem(i,0,pitem0);
            pitem1       = new QTableWidgetItem;
            pitem1      ->setText(it.key());
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
        ListChamps->dlglayout()->insertWidget(0,tabChamps);

        ListChamps->setModal(true);
        ListChamps->move(QPoint(x()+width()/2,y()+height()/2));

        connect(ListChamps->OKButton,   &QPushButton::clicked,          [=] {ListChamps->accept();});
        ListChamps->setFixedWidth(tabChamps->width() + ListChamps->dlglayout()->contentsMargins().left()*2);
        connect(tabChamps,              &QTableWidget::doubleClicked,   [=] {ListChamps->accept();});
        ListChamps->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

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
            ui->upTextEdit->textCursor().insertHtml("((" + dlg_askdialog->findChildren<UpLineEdit*>().at(0)->text() + "//DATE))");
        delete dlg_askdialog;
    }
    else if (choix == "Texte")
    {
        if (AskDialog("Choix de texte")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + dlg_askdialog->findChildren<UpLineEdit*>().at(0)->text() + "//TEXTE))");
        delete dlg_askdialog;
    }
    else if (choix == "Soignant")
    {
        ui->upTextEdit->textCursor().insertHtml("((" + tr("Quel soignant?") + "//SOIGNANT))");
    }
    else if (choix == ANESTHINTERVENTION)
    {
        QString txt = "((" + TITREANESTHINTERVENTION + "//" + ANESTHINTERVENTION + "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
     }
    else if (choix == PROVENANCE)
    {
        QString txt = "((" + tr("Provenance") + "//";
        txt += PROVENANCE;
        txt += "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == DATEINTERVENTION)
    {
        QString txt = "((" + TITREDATEINTERVENTION + "//" + DATEINTERVENTION + "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == HEUREINTERVENTION)
    {
        QString txt = "((" + TITREHEUREINTERVENTION + "//" + HEUREINTERVENTION + "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == COTEINTERVENTION)
    {
        QString txt = "((" + TITRECOTEINTERVENTION + "//" + COTEINTERVENTION + "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == TYPEINTERVENTION)
    {
        QString txt = "((" + TITRETYPEINTERVENTION + "//" + TYPEINTERVENTION + "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == SITEINTERVENTION)
    {
        QString txt = "((" + TITRESITEINTERVENTION + "//" + SITEINTERVENTION + "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == TYPESEJOUR)
    {
        QString txt = "((" + tr("Sejour") + "//";
        txt += TYPESEJOUR;
        txt += "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == SITE)
    {
        QString txt = "((" + tr("Centre") + "//";
        txt += SITE;
        txt += "))";
        ui->upTextEdit->textCursor().insertHtml(txt);
    }
    else if (choix == "Montant")
    {
        if (AskDialog("Choix de texte")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + dlg_askdialog->findChildren<UpLineEdit*>().at(0)->text() + "//MONTANT))");
        delete dlg_askdialog;
    }
    else if (choix == "Heure")
    {
        if (AskDialog("Choix d'une heure")>0)
            ui->upTextEdit->textCursor().insertHtml("((" + dlg_askdialog->findChildren<UpLineEdit*>().at(0)->text() + "//HEURE))");
        delete dlg_askdialog;
    }
    else if (choix == COTE)
    {
        if (AskDialog("Choix d'un côté")>0)
        {
            QString txt = "((" + dlg_askdialog->findChildren<UpLineEdit*>().at(0)->text() + "//";
            txt += COTE;
            txt += "))";
            ui->upTextEdit->textCursor().insertHtml(txt);
        }
        delete dlg_askdialog;
    }
}


// ----------------------------------------------------------------------------------
// Clic sur le bouton OK.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_impressions::Validation()
{
    UpLineEdit *line = new UpLineEdit();
    int         c = 0;
    QStringList listQuestions, listtypeQuestions;
    QStringList ExpARemplacer, Rempla;
    QString listusers = "ListUsers";
    QString listsoignants = "ListSoignants";
    m_userentete = Q_NULLPTR;
    if (m_currentintervention == Q_NULLPTR)
    {
        if (currentuser()->ishisownsupervisor())
            m_userentete = currentuser();
    }
    else
    {
        SessionOperatoire *session = Datas::I()->sessionsoperatoires->getById(m_currentintervention->idsession());
        if (session != Q_NULLPTR)
            m_userentete = Datas::I()->users->getById(session->iduser());
    }
    int ndocs = 0;
    switch (m_mode) {
    case CreationDOC:
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        InsertDocument(line->Row());
        break;
    case CreationDOSS:
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        InsertDossier(line->Row());
        break;
    case ModificationDOC:
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        UpdateDocument(line->Row());
        break;
    case ModificationDOSS:
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) break;
        }
        UpdateDossier(line->Row());
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
            UpMessageBox::Watch(this,"Euuhh... " + Datas::I()->users->userconnected()->login() + ", " + tr("il doit y avoir une erreur..."), tr("Vous n'avez sélectionné aucun document."));
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
                    QString text = getDocumentFromRow(i)->texte();
                    QString quest = "([(][(][éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°?, -]*//(DATE|TEXTE|HEURE|MONTANT|SOIGNANT";
                    quest+= "|" + COTE + "|" + PROVENANCE + "|" + TYPESEJOUR + "|" + SITE;
                    if (m_currentintervention == Q_NULLPTR)
                        quest+= "|" + DATEINTERVENTION + "|" + HEUREINTERVENTION + "|" + COTEINTERVENTION + "|" + TYPEINTERVENTION + "|" + SITEINTERVENTION + "|" + ANESTHINTERVENTION;
                    quest += ")[)][)])";
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
        if (listQuestions.size()>0 || m_userentete == Q_NULLPTR)
        {
            dlg_ask = new UpDialog(this);
            dlg_ask->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
            dlg_ask->setModal(true);
            dlg_ask->setSizeGripEnabled(false);
            dlg_ask->move(QPoint(x()+width()/2,y()+height()/2));
            dlg_ask->setWindowTitle(tr("Complétez la fiche"));

            QVBoxLayout *layWidg = new QVBoxLayout();
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
                    UpLineEdit *Line = new UpLineEdit(dlg_ask);
                    QDoubleValidator *val= new QDoubleValidator(this);
                    val->setDecimals(2);
                    Line->setValidator(val);
                    Line->setMaxLength(15);
                    Line->setFixedHeight(23);
                    Line->installEventFilter(this);
                    lay->addWidget(Line);
                }
                else if (listtypeQuestions.at(m)  == "SOIGNANT")
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    Combo->setAccessibleDescription(listsoignants);
                    foreach (User* usr, *Datas::I()->users->actifs())
                        if (usr->isSoignant())
                            Combo->addItem(usr->login(), usr->id());
                    lay->addWidget(Combo);
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
                else if (listtypeQuestions.at(m)  == COTE)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    Combo->setAccessibleDescription(COTE);
                    QStringList listcote;
                    listcote << tr("chaque oeil") << tr("l'oeil droit") << tr("l'oeil gauche");
                    Combo->addItems(listcote);
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m) == ANESTHINTERVENTION)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    Combo->setAccessibleDescription(ANESTHINTERVENTION);
                    Combo->addItem(tr("Locale"), "L");
                    Combo->addItem(tr("LocoRegionale"), "R");
                    Combo->addItem(tr("Générale"), "G");
                    Combo->addItem(tr("Sans objet", ""));
                    Combo->setCurrentIndex(0);
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m) == PROVENANCE)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    Combo->setAccessibleDescription(PROVENANCE);
                    Combo->addItems(QStringList() << tr("Domicile") << tr("Institution") << tr("Transfert"));
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m)  == TYPESEJOUR)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    Combo->setAccessibleDescription(TYPESEJOUR);
                    Combo->addItems(QStringList() << tr("Ambulatoire") << tr("Hospitalisation") << tr("Urgence"));
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m)  == SITE)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    Combo->setAccessibleDescription(SITE);
                    foreach (Site* sit, *Datas::I()->sites->sites())
                        Combo->addItem(sit->nom(), QString::number(sit->id()) );
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m)  == DATEINTERVENTION)
                {
                    QDateEdit *Date = new QDateEdit();
                    Date->setCalendarPopup(false); //si on met ce paramètre à true et qu'on utilise le calendarpopup,
                                                   //la fonction QLayout::count() utilisée un peu plus loin plante le programme..(?)..
                    Date->setContentsMargins(0,0,0,0);
                    Date->setFixedSize(110,30);
                    Date->setDate(QDate::currentDate());
                    Date->setAccessibleDescription(DATEINTERVENTION);
                    lay->addWidget(Date);
                }
                else if (listtypeQuestions.at(m)  == HEUREINTERVENTION)
                {
                    QTimeEdit *Time = new QTimeEdit();
                    Time->setContentsMargins(0,0,0,0);
                    Time->setFixedSize(70,30);
                    Time->setTime(QTime::currentTime());
                    Time->setTimeSpec(Qt::LocalTime);
                    Time->setAccessibleDescription(HEUREINTERVENTION);
                    lay->addWidget(Time);
                }
                else if (listtypeQuestions.at(m)  == COTEINTERVENTION)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    QStringList listcote;
                    listcote << tr("chaque oeil") << tr("l'oeil droit") << tr("l'oeil gauche");
                    Combo->addItems(listcote);
                    Combo->setAccessibleDescription(COTEINTERVENTION);
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m)  == TYPEINTERVENTION)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    foreach (TypeIntervention* typ, *Datas::I()->typesinterventions->typeinterventions())
                        Combo->addItem(typ->typeintervention(), QString::number(typ->id()) );
                    Combo->setAccessibleDescription(TYPEINTERVENTION);
                    lay->addWidget(Combo);
                }
                else if (listtypeQuestions.at(m)  == SITEINTERVENTION)
                {
                    UpComboBox *Combo = new UpComboBox();
                    Combo->setContentsMargins(0,0,0,0);
                    Combo->setFixedHeight(34);
                    Combo->setEditable(false);
                    foreach (Site* sit, *Datas::I()->sites->sites())
                        Combo->addItem(sit->nom(), QString::number(sit->id()) );
                    Combo->setAccessibleDescription(SITEINTERVENTION);
                    lay->addWidget(Combo);
                }
            }
            if (listQuestions.size()>0 && !currentuser()->ishisownsupervisor())
            {
                QFrame *line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                layWidg->addWidget(line);
            }
            if (m_userentete == Q_NULLPTR)
            {
                QHBoxLayout *lay = new QHBoxLayout();
                lay->setContentsMargins(5,0,5,0);
                layWidg->addLayout(lay);
                UpLabel *label = new UpLabel();
                label->setText(tr("Quel entête?"));
                label->setFixedSize(150,25);
                lay->addWidget(label);
                QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Expanding);
                lay->addSpacerItem(spacer);
                UpComboBox *Combo = new UpComboBox();
                Combo->setContentsMargins(0,0,0,0);
                Combo->setFixedHeight(34);
                Combo->setEditable(false);
                foreach (User* usr, *Datas::I()->users->superviseurs())
                    Combo->addItem(usr->login(), QString::number(usr->id()));
                Combo->setAccessibleDescription(listusers);
                lay->addWidget(Combo);
            }
            dlg_ask->dlglayout()   ->setContentsMargins(5,5,5,5);
            layWidg     ->setContentsMargins(0,0,0,0);
            layWidg     ->setSpacing(0);
            dlg_ask->dlglayout()   ->setSpacing(5);
            dlg_ask->dlglayout()   ->insertLayout(0,layWidg);

            dlg_ask->AjouteLayButtons();
            dlg_ask->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
            connect(dlg_ask->OKButton,     &QPushButton::clicked,   [=] {VerifCoherencedlg_ask();});

            if (dlg_ask->exec() == 0)
            {
                delete dlg_ask;
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
                            w = Q_NULLPTR;
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
                                }
                                else if (listwidg.at(p)->inherits("QDateEdit"))
                                {
                                    QDateEdit *linedate = static_cast<QDateEdit*>(listwidg.at(p));
                                    Rempla          << linedate->date().toString(tr("d MMMM yyyy"));
                                    if (linedate->accessibleDescription() == DATEINTERVENTION)
                                        ExpARemplacer   << minidou + "//" + DATEINTERVENTION + "))";
                                    else
                                        ExpARemplacer   << minidou + "//DATE))";
                                }
                                else if (listwidg.at(p)->inherits("QTimeEdit"))
                                {
                                    QTimeEdit *linetime = static_cast<QTimeEdit*>(listwidg.at(p));
                                    Rempla          << linetime->time().toString("H'H'mm");
                                    if (linetime->accessibleDescription() == HEUREINTERVENTION)
                                        ExpARemplacer   << minidou + "//" + HEUREINTERVENTION + "))";
                                    else
                                        ExpARemplacer   << minidou + "//HEURE))";
                                }
                                else if (listwidg.at(p)->inherits("UpComboBox"))
                                {
                                    UpComboBox *linecombo = static_cast<UpComboBox*>(listwidg.at(p));
                                    if (linecombo->accessibleDescription() == listsoignants)
                                    {
                                        int idusr = linecombo->currentData().toInt();
                                        User* usr = Datas::I()->users->getById(idusr);
                                        QString babar = (usr != Q_NULLPTR? (usr->isMedecin()? usr->titre() + " " : "") + usr->prenom() + " " + usr->nom() : "");
                                        Rempla          << babar;
                                        ExpARemplacer   << minidou + "//SOIGNANT))";
                                    }
                                    if (linecombo->accessibleDescription() == COTEINTERVENTION
                                            || linecombo->accessibleDescription() == SITEINTERVENTION
                                            || linecombo->accessibleDescription() == ANESTHINTERVENTION
                                            || linecombo->accessibleDescription() == TYPEINTERVENTION)
                                    {
                                        Rempla          << linecombo->currentText();
                                        ExpARemplacer   << minidou + "//" + linecombo->accessibleDescription() + "))";
                                    }
                                    else if (linecombo->accessibleDescription() != listusers)
                                    {
                                        Rempla          << linecombo->currentText();
                                        ExpARemplacer   << minidou + "//" + linecombo->accessibleDescription() + "))";
                                    }
                                    else
                                    {
                                        int idusr = linecombo->currentData().toInt();
                                        m_userentete = Datas::I()->users->getById(idusr);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            delete dlg_ask;
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
                    QMap<DATASAIMPRIMER,QString>  datasdocaimprimer;
                    QString textAimprimer = getDocumentFromRow(i)->texte();
                    if (ExpARemplacer.size() > 0)
                        for (int y=0; y<ExpARemplacer.size(); y++)
                            textAimprimer.replace(ExpARemplacer.at(y),Rempla.at(y));
                    MetAJour(textAimprimer,true);
                    for (int j=0; j<m_listtexts.size();j++)
                    {
                        QString txtdoc = m_listtexts.at(j);
                        int row = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1))->Row();
                        Impression *impr = getDocumentFromRow(row);

                        // on détermine le titre du document à inscrire en conclusion et le statut de prescription (si prescription => impression d'un dupli)

                        QString titre                   = static_cast<UpLineEdit *>(ui->DocupTableWidget->cellWidget(i,1))->text();
                        datasdocaimprimer.insert(Titre, titre);
                        datasdocaimprimer.insert(Prescription, (impr->isprescription()? "1": ""));
                        datasdocaimprimer.insert(Administratif, (impr->ismedical()? "": "1"));
                        datasdocaimprimer.insert(Dupli, ((impr->isprescription() && ui->DupliOrdocheckBox->isChecked())? "1": ""));
                        // on visualise le document pour correction s'il est éditable
                        txtdoc                          = (impr->iseditable()? proc->Edit(txtdoc, titre): txtdoc);
                        if (txtdoc != "")               // si le texte du document est vide, on annule l'impression de cette itération
                        {
                            datasdocaimprimer.insert(Texte, txtdoc);
                            map_docsaimprimer.insert(ndocs, datasdocaimprimer);
                            ++ndocs;
                        }
                    }
                }
            }
        }
        if (map_docsaimprimer.size() > 0)
            accept();
        break;
    default:
        break;
    }
}

void dlg_impressions::OrdoAvecDupli(bool a)
{
    proc->settings()->setValue("Param_Imprimante/OrdoAvecDupli",(a? "YES" : "NO"));
}


/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Vérifie que les champs sont remplis avant la fermeture de dlg_ask ------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_impressions::VerifCoherencedlg_ask()
{
    QList<UpLineEdit*> listUpline = dlg_ask->findChildren<UpLineEdit*>();
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
        QList<UpComboBox*> listCombo = dlg_ask->findChildren<UpComboBox*>();
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
    if (a) dlg_ask->accept();
}

bool dlg_impressions::event(QEvent *event)
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
void dlg_impressions::closeEvent(QCloseEvent *event)
{
    proc->settings()->setValue("PositionsFiches/PositionDocuments",saveGeometry());
    event->accept();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_impressions::eventFilter(QObject *obj, QEvent *event)
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
            if (m_mode == Selection)
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
                        if (line->Row() > 0){
                            LineSelect(table, line->Row()-1);
                            QPoint posdebut = mapFrom(this, table->cellWidget(0,0)->pos());
                            QPoint poscell  = mapFrom(this, table->cellWidget(line->Row(),0)->pos());
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
                        if (line->Row() < table->rowCount()-1){
                            LineSelect(table, line->Row()+1);
                            QPoint poscell = mapFrom(this, table->cellWidget(line->Row(),0)->pos());
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

void dlg_impressions::keyPressEvent(QKeyEvent * event )
{
    switch (event->key()) {
    case Qt::Key_Escape:
    {
        Annulation();
        break;
    }
    case Qt::Key_F5:
    {
        int row = -1;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            UpLineEdit *line = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) { row = line->Row(); break; }
        }
        if (row > -1)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(row,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setChecked(!Check->isChecked());
                EnableOKPushButton(Check);
            }
        }
        break;
    }
    default:
        break;
    }
}

int dlg_impressions::AskDialog(QString titre)
{
    dlg_askdialog               = new UpDialog(this);
    dlg_askdialog               ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
    UpLineEdit  *Line           = new UpLineEdit(dlg_askdialog);
    UpLabel     *label          = new UpLabel(dlg_askdialog);
    QString question            = tr("Entrez la question que vous voulez poser.");

    label   ->setText(question);
    label   ->setFixedSize(Utils::CalcSize(question));
    Line    ->setFixedSize(Utils::CalcSize(question));

    dlg_askdialog->dlglayout()->setSpacing(4);
    dlg_askdialog->dlglayout()->insertWidget(0,Line);
    dlg_askdialog->dlglayout()->insertWidget(0,label);

    dlg_askdialog->setModal(true);
    dlg_askdialog->setSizeGripEnabled(false);
    //dlg_askdialog->setFixedSize(270,100);
    dlg_askdialog->move(QPoint(x()+width()/2,y()+height()/2));
    dlg_askdialog->setWindowTitle(titre);
    dlg_askdialog->AjouteLayButtons();
    dlg_askdialog->TuneSize();

    connect(dlg_askdialog->OKButton,   &QPushButton::clicked,   [=] {dlg_askdialog->accept();});

    Line->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    Line->setMaxLength(60);
    return dlg_askdialog->exec();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de document
// ----------------------------------------------------------------------------------
bool dlg_impressions::ChercheDoublon(QString str, int row)
{
    QString req, nom;
    switch (m_mode) {
    case CreationDOC:
        req = "select " CP_RESUME_IMPRESSIONS ", " CP_IDUSER_IMPRESSIONS " from " TBL_IMPRESSIONS;
        nom = tr("document");
        break;
    case ModificationDOC:
        req = "select " CP_RESUME_IMPRESSIONS ", " CP_IDUSER_IMPRESSIONS " from " TBL_IMPRESSIONS " where " CP_ID_IMPRESSIONS " <> " + QString::number(getDocumentFromRow(row)->id());
        nom = tr("document");
        break;
    case CreationDOSS:
        req = "select " CP_RESUME_DOSSIERIMPRESSIONS ", " CP_IDUSER_DOSSIERIMPRESSIONS " from " TBL_DOSSIERSIMPRESSIONS;
        nom = tr("dossier");
        break;
    case ModificationDOSS:
        req = "select " CP_RESUME_DOSSIERIMPRESSIONS ", " CP_IDUSER_DOSSIERIMPRESSIONS " from " TBL_DOSSIERSIMPRESSIONS " where idmetadocument <> " + QString::number(getMetaDocumentFromRow(row)->id());
        nom = tr("dossier");
        break;
    default:
        return false;
    }
    bool a = false;
    QList<QVariantList> listdocs;
    bool ok;
    listdocs = db->StandardSelectSQL(req,ok);
    if (listdocs.size() > 0)
    {
        for (int i=0; i<listdocs.size() ; i++)
        {
            if (listdocs.at(i).at(0).toString().toUpper() == str.toUpper())
            {
                a = true;
                QString b = " " + tr("créé par vous");
                if (listdocs.at(i).at(1).toInt() != currentuser()->id())
                {
                    if (Datas::I()->users->getById(listdocs.at(i).at(1).toInt()) != Q_NULLPTR)
                        b =  " " + tr("créé par") + " " + Datas::I()->users->getById(listdocs.at(i).at(1).toInt())->login();
                    else
                        b = "";
                }
                UpMessageBox::Watch(this,tr("Il existe déjà un") + " " + nom + " " + tr("portant ce nom") + b);
                break;
            }
        }
    }
    return a;
}

// ----------------------------------------------------------------------------------
// On a clique sur une ligne de dossiers . on coche-décoche les docs correspondants
// ----------------------------------------------------------------------------------
void dlg_impressions::CocheLesDocs(int iddoss, bool A)
{
    bool ok;
    QString req = "select " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " from " TBL_JOINTURESIMPRESSIONS " where " CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " = " + QString::number(iddoss);
    QList<QVariantList> listdocmts = db->StandardSelectSQL(req,ok);
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
                if (listiddocs.contains(QString::number(getDocumentFromRow(k)->id())))
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
                                if (QString::number(getMetaDocumentFromRow(j)->id()) != iddoss)
                                {
                                    if (DossCheck->isChecked())
                                    {
                                        req = "select " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " from " TBL_JOINTURESIMPRESSIONS
                                                " where " CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " = " + QString::number(getMetaDocumentFromRow(j)->id());
                                        QList<QVariantList> listdocmts2 = db->StandardSelectSQL(req,ok);
                                        if (listdocmts2.size() > 0)
                                        {
                                            QStringList listid;
                                            for (int i=0; i<listdocmts2.size(); i++)
                                                listid << listdocmts2.at(i).at(0).toString();
                                            if (listid.contains(QString::number(getDocumentFromRow(k)->id())))
                                            {
                                                a = true;
                                                break;
                        }   }   }   }   }   }
                        DocCheck->setChecked(a);
                    }
                    QString nomdoc = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(DocCheck->rowTable(),1))->text();
                    if (!DocCheck->isChecked())
                        ui->DocupTableWidget->item(DocCheck->rowTable(),6)->setText("1" + nomdoc);
                    else
                        ui->DocupTableWidget->item(DocCheck->rowTable(),6)->setText("0" + nomdoc);
                }

            }   }   }
    // tri de la table docuptablewidget
    TriDocupTableWidget();
    ui->DocupTableWidget->scrollToTop();

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
void dlg_impressions::ConfigMode(Mode mode, int row)
{
    m_mode = mode;
    ui->ChercheupLineEdit->setEnabled       (m_mode == Selection);
    ui->dateImpressiondateEdit->setEnabled                (m_mode == Selection);
    ui->ALDcheckBox->setVisible             (m_mode == Selection);
    wdg_dossiersbuttonframe->setEnabled         (m_mode == Selection);
    ui->DossiersupTableWidget->setEnabled   (m_mode == Selection);
    ui->OKupPushButton->setEnabled          (false);
    ui->textFrame->setVisible               (m_mode != CreationDOSS && m_mode!= ModificationDOSS && m_mode != Selection);

    if (m_mode != Selection) {
        t_timerefface->disconnect();
        ui->textFrame->setGraphicsEffect(new QGraphicsOpacityEffect());
    }
    else
    {
        m_opacityeffect = new QGraphicsOpacityEffect();
        m_opacityeffect->setOpacity(0.1);
        ui->textFrame->setGraphicsEffect(m_opacityeffect);
    }

    if (mode == Selection)
    {
        EnableLines();
        wdg_docsbuttonframe                 ->setEnabled(true);
        ui->DocPubliccheckBox           ->setChecked(false);
        ui->DocPubliccheckBox           ->setEnabled(false);
        ui->DocPubliccheckBox           ->setToolTip("");
        ui->DocupTableWidget            ->setEnabled(true);
        ui->DocupTableWidget            ->setFocus();
        ui->DocupTableWidget            ->setStyleSheet("");
        wdg_dossiersbuttonframe             ->setEnabled(true);
        ui->DossiersupTableWidget       ->setEnabled(true);
        ui->DocEditcheckBox             ->setChecked(false);
        ui->DocEditcheckBox             ->setEnabled(false);
        ui->DocEditcheckBox             ->setToolTip("");
        ui->DocAdministratifcheckBox    ->setChecked(false);
        ui->DocAdministratifcheckBox    ->setEnabled(false);
        ui->DocAdministratifcheckBox    ->setToolTip("");
        ui->Expliclabel                 ->setText(tr("SELECTION - Cochez les dossiers ou les documents que vous voulez imprimer")
                                                     + "\n" + tr("clic souris ou touche F5 pour sélectionner/déselectionner"));
        wdg_docsbuttonframe->wdg_modifBouton    ->setEnabled(false);
        wdg_dossiersbuttonframe->wdg_modifBouton->setEnabled(false);
        ui->PrescriptioncheckBox        ->setEnabled(false);
        wdg_docsbuttonframe->wdg_moinsBouton    ->setEnabled(false);
        wdg_dossiersbuttonframe->wdg_moinsBouton->setEnabled(false);
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
        int iddoc = getDocumentFromRow(row)->id();
        DisableLines();
        row = -1;
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            if (getDocumentFromRow(i)->id() == iddoc)
            {
                row = i;
                break;
            }
        if (row == -1)
            return;
        UpLineEdit *line = static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(row,1));
        line->setEnabled(true);
        line->setFocusPolicy(Qt::WheelFocus);
        line->setFocus();
        line->selectAll();
        connect(line,   &QLineEdit::textEdited, this, [=] {EnableOKPushButton();});

        ui->upTextEdit->setText(getDocumentFromRow(row)->texte());

        ui->DocPubliccheckBox->setEnabled(VerifDocumentPublic(row,false));
        ui->DocPubliccheckBox->setToolTip(tr("Cochez cette case si vous souhaitez\nque ce document soit visible par tous les utilisateurs"));
        ui->DocupTableWidget->setEnabled(true);
        ui->DocupTableWidget->setStyleSheet("");
        wdg_docsbuttonframe->setEnabled(false);
        ui->DossiersupTableWidget->setEnabled(true);
        wdg_dossiersbuttonframe->setEnabled(false);
        ui->textFrame->setEnabled(true);
        ui->Expliclabel->setText(tr("DOCUMENTS - MODIFICATION"));
        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));
        ui->DocAdministratifcheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setToolTip(tr("si cette option est cochée\nle document est considéré comme purement administratif"));
        ui->PrescriptioncheckBox    ->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");
        if (!Datas::I()->users->userconnected()->isMedecin() && !Datas::I()->users->userconnected()->isOrthoptist())
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
        int iddossier = getMetaDocumentFromRow(row)->id();
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
        ui->ChercheupLineEdit->clear();
    }
    else if (mode == CreationDOC)
    {
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
        upLine0->setMaxLength(50);
        upLine0->setRow(row);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::WheelFocus);
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        upLine0->selectAll();
        connect(upLine0,   &QLineEdit::textEdited, this, [=] {EnableOKPushButton();});
        ui->DocupTableWidget->setCellWidget(row,1,upLine0);
        QTableWidgetItem    *pItem1 = new QTableWidgetItem;
        QTableWidgetItem    *pItem2 = new QTableWidgetItem;
        int col = 2;
        pItem1->setText("0");                           // idDocument
        ui->DocupTableWidget->setItem(row,col,pItem1);
        col++; //3
        UpLabel*lbl = new UpLabel(ui->DocupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        ui->DocupTableWidget->setCellWidget(row,col,lbl);
        col++; //4
        UpLabel*lbl1 = new UpLabel(ui->DocupTableWidget);
        lbl1->setAlignment(Qt::AlignCenter);
        ui->DocupTableWidget->setCellWidget(row,col,lbl1);
        col++; //5
        UpLabel*lbl2 = new UpLabel(ui->DocupTableWidget);
        lbl1->setAlignment(Qt::AlignCenter);
        ui->DocupTableWidget->setCellWidget(row,col,lbl2);
        col++; //6
        pItem2->setText("1");                           // Check+text   -> sert pour le tri de la table
        ui->DocupTableWidget->setItem(row,col,pItem2);
        ui->DocupTableWidget->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));

        ui->DocPubliccheckBox->setChecked(false);
        ui->DocPubliccheckBox->setEnabled(true);
        ui->DocPubliccheckBox->setToolTip(tr("Cochez cette case si vous souhaitez\nque ce document soit visible par tous les utilisateurs"));
        ui->DossiersupTableWidget->setEnabled(false);
        wdg_dossiersbuttonframe->setEnabled(false);
        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setChecked(false);
        ui->DocEditcheckBox->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));
        ui->Expliclabel->setText(tr("DOCUMENTS - CREATION - Remplissez les champs définissant le document que vous voulez créer"));
        ui->PrescriptioncheckBox->setChecked(Qt::Unchecked);
        ui->PrescriptioncheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setChecked(false);
        ui->DocAdministratifcheckBox->setToolTip(tr("si cette option est cochée\nle document est considéré comme purement administratif"));
        wdg_docsbuttonframe->wdg_moinsBouton->setEnabled(false);
        ui->upTextEdit->clear();
        ui->upTextEdit->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");
        if (!Datas::I()->users->userconnected()->isMedecin() && !Datas::I()->users->userconnected()->isOrthoptist())
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
        ui->ChercheupLineEdit->clear();
        FiltreListe();
        DisableLines();
        for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        {
            QWidget *Widg = dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(true);
                Check->setChecked(false);
            }
            UpLineEdit *line0 = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
            if (line0) {
                line0->deselect();
                line0->setEnabled(false);
            }
        }
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            QWidget *Widg = dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
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
        upLine0->setMaxLength(80);
        upLine0->setRow(row);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::WheelFocus);
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        upLine0->setFocus();
        upLine0->selectAll();
        connect(upLine0,   &QLineEdit::textEdited, [=] {EnableOKPushButton();});
        ui->DossiersupTableWidget->setCellWidget(row,1,upLine0);
        QTableWidgetItem    *pItem1 = new QTableWidgetItem;
        int col = 2;
        UpLabel*lbl = new UpLabel(ui->DossiersupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        ui->DossiersupTableWidget->setCellWidget(row,col,lbl);
        col++; //3
        pItem1->setText("");                           // idMetaDocument
        ui->DossiersupTableWidget->setItem(row,col,pItem1);

        ui->DossiersupTableWidget->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));

        wdg_docsbuttonframe->setEnabled(false);
        ui->DocupTableWidget->setEnabled(true);
        ui->DocupTableWidget->setStyleSheet("UpTableWidget {border: 2px solid rgb(251, 51, 61);}");
        ui->DossiersupTableWidget->setEnabled(true);
        wdg_dossiersbuttonframe->setEnabled(false);
        ui->Expliclabel->setText(tr("DOSSIER - CREATION - Cochez les cases correspondants au dossier que vous voulez créer"));

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setEnabled(true);
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setText(tr("Enregistrer\nle dossier"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
    }
    if (!Datas::I()->users->userconnected()->isMedecin() && !Datas::I()->users->userconnected()->isOrthoptist())
    {
        ui->PrescriptioncheckBox->setVisible(false);
        ui->DocAdministratifcheckBox->setVisible(false);
    }
}

// --------------------------------------------------------------------------------------------------
// SetEnabled = false et disconnect toutes les lignes des UpTableWidget - SetEnabled = false checkBox
// --------------------------------------------------------------------------------------------------
void dlg_impressions::DisableLines()
{
    ui->ChercheupLineEdit->clear();
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
        ui->DocupTableWidget->setRowHidden(i,false);
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        ui->DossiersupTableWidget->setRowHidden(i,false);

    wdg_dossiersbuttonframe->setEnabled(false);
    wdg_docsbuttonframe->setEnabled(false);
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(false);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line0 != Q_NULLPTR) {
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
        if (line0 != Q_NULLPTR) {
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
void dlg_impressions::EnableLines()
{
    wdg_dossiersbuttonframe->setEnabled(true);
    wdg_docsbuttonframe->setEnabled(true);
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
        if (Widg != Q_NULLPTR)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(true);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line0 != Q_NULLPTR) {
            line0->deselect();
            line0->setEnabled(true);
            line0->setFocusPolicy(Qt::NoFocus);
            if (getDocumentFromRow(i)->iduser() == currentuser()->id())
            {
                connect(line0,          &UpLineEdit::mouseDoubleClick,          [=] {DocCellDblClick(line0);});
                connect(line0,          &QWidget::customContextMenuRequested,   [=] {MenuContextuel(line0);});
                connect(line0,          &QLineEdit::textEdited,                 [=] {EnableOKPushButton();});
            }
            connect(line0,              &UpLineEdit::mouseEnter,                [=] {DocCellEnter(line0);});
            connect(line0,              &UpLineEdit::mouseRelease,              [=] {LineSelect(ui->DocupTableWidget, line0->Row());});
        }
    }
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0));
        if (Widg != Q_NULLPTR)
        {
            UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(true);
        }
        UpLineEdit *line0        = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (line0 != Q_NULLPTR) {
            line0->deselect();
            line0->setEnabled(true);
            line0->setFocusPolicy(Qt::NoFocus);
            if (getMetaDocumentFromRow(i)->iduser() == currentuser()->id())
            {
                connect(line0,          &UpLineEdit::mouseDoubleClick,          [=] {DocCellDblClick(line0);});
                connect(line0,          &QWidget::customContextMenuRequested,   [=] {MenuContextuel(line0);});
                connect(line0,          &QLineEdit::textEdited,                 [=] {EnableOKPushButton();});
            }
            connect(line0,              &UpLineEdit::mouseEnter,                [=] {DocCellEnter(line0);});
            connect(line0,              &UpLineEdit::mouseRelease,              [=] {LineSelect(ui->DossiersupTableWidget, line0->Row());});
        }
    }
}

// ----------------------------------------------------------------------------------
// Effacement progressif d'un textEdit.
// ----------------------------------------------------------------------------------
void dlg_impressions::EffaceWidget(QWidget* widg, bool AvecOuSansPause)
{
    QTime DebutTimer     = QTime::currentTime();
    m_opacity = 1;
    widg->setVisible(true);
    widg->setAutoFillBackground(true);
    t_timerefface->disconnect();
    t_timerefface->start(70);
    connect(t_timerefface, &QTimer::timeout, [=]
    {
        QRect rect = QRect(widg->pos(),widg->size());
        QPoint pos = mapFromParent(cursor().pos());
        int Pause = (AvecOuSansPause? 8000: 0);
        if (DebutTimer.msecsTo(QTime::currentTime()) > Pause  && !rect.contains(pos))
        {
            m_opacity = m_opacity*0.9;
            m_opacityeffect->setOpacity(m_opacity);
            widg->setGraphicsEffect(m_opacityeffect);
            if (m_opacity < 0.10)
                t_timerefface->disconnect();
        }
        else
        {
            m_opacityeffect->setOpacity(1);
            widg->setGraphicsEffect(m_opacityeffect);
        }
    });
}

Impression* dlg_impressions::getDocumentFromRow(int row)
{
    return Datas::I()->impressions->getById(ui->DocupTableWidget->item(row,2)->text().toInt());
}

DossierImpression* dlg_impressions::getMetaDocumentFromRow(int row)
{
    return Datas::I()->metadocuments->getById(ui->DossiersupTableWidget->item(row,3)->text().toInt());
}

User* dlg_impressions::userentete() const
{
    return m_userentete;
}
// ----------------------------------------------------------------------------------
// Creation du Document dans la base.
// ----------------------------------------------------------------------------------
void dlg_impressions::InsertDocument(int row)
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

    QString requete = "INSERT INTO " TBL_IMPRESSIONS
            " (" CP_TEXTE_IMPRESSIONS ", " CP_RESUME_IMPRESSIONS ", " CP_IDUSER_IMPRESSIONS ", " CP_DOCPUBLIC_IMPRESSIONS ", " CP_PRESCRIPTION_IMPRESSIONS ", " CP_EDITABLE_IMPRESSIONS ", " CP_MEDICAL_IMPRESSIONS ") "
            " VALUES ('" + Utils::correctquoteSQL(ui->upTextEdit->document()->toHtml()) +
            "', '" + Utils::correctquoteSQL(line->text().left(100)) +
            "', " + QString::number(currentuser()->id());
    QString Public          = (ui->DocPubliccheckBox->isChecked()?          "1" : "null");
    QString Prescription    = (ui->PrescriptioncheckBox->isChecked()?       "1" : "null");
    QString Editable        = (ui->DocEditcheckBox->isChecked()?            "1" : "null");
    QString Admin           = (ui->DocAdministratifcheckBox->isChecked()?   "null" : "1");
    requete += ", " + Public;
    requete += ", " + Prescription;
    requete += ", " + Editable;
    requete += ", " + Admin;
    requete += ")";
    db->StandardSQL(requete,tr("Erreur d'enregistrement du document dans ") + TBL_IMPRESSIONS);

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
                LineSelect(ui->DocupTableWidget,line->Row());
                QModelIndex index = ui->DocupTableWidget->model()->index(line->Row(),1);
                ui->DocupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                break;
             }
         }
    }
}

// ----------------------------------------------------------------------------------
// Creation du Dossier dans la base.
// ----------------------------------------------------------------------------------
void dlg_impressions::InsertDossier(int row)
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

    QString requete = "INSERT INTO " TBL_DOSSIERSIMPRESSIONS
            " (" CP_RESUME_DOSSIERIMPRESSIONS ", " CP_IDUSER_DOSSIERIMPRESSIONS ", " CP_PUBLIC_DOSSIERIMPRESSIONS ") "
            " VALUES ('" + Utils::correctquoteSQL(line->text().left(100)) +
            "'," + QString::number(currentuser()->id());
    UpLabel *lbl = static_cast<UpLabel*>(ui->DossiersupTableWidget->cellWidget(row,2));
    QString a = "null";
    if (lbl->pixmap() != Q_NULLPTR)
        a = "1";
    requete += "," + a + ")";
    if (db->StandardSQL(requete, tr("Erreur d'enregistrement du dossier dans ") +  TBL_DOSSIERSIMPRESSIONS))
    {
        QStringList listdocs;
        QString idmetadoc;
        requete = "select " CP_ID_DOSSIERIMPRESSIONS " from " TBL_DOSSIERSIMPRESSIONS " where " CP_RESUME_DOSSIERIMPRESSIONS " = '" + Utils::correctquoteSQL(line->text().left(100)) + "'";
        bool ok;
        QList<QVariantList> listdocmts = db->StandardSelectSQL(requete,ok);
        if (listdocmts.size()>0)
        {
            idmetadoc = listdocmts.at(0).at(0).toString();
            for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
            {
                QWidget * w = static_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0));
                QHBoxLayout* boxlay = w->findChildren<QHBoxLayout*>().at(0);
                UpCheckBox* Check = static_cast<UpCheckBox*>(boxlay->itemAt(0)->widget());
                if (Check->isChecked())
                    listdocs << QString::number(getDocumentFromRow(i)->id());
            }
            if (listdocs.size()>0)
            {
                requete = "insert into " TBL_JOINTURESIMPRESSIONS " (" CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS ", " CP_IDDOCUMENT_JOINTURESIMPRESSIONS ") VALUES ";
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
                LineSelect(ui->DossiersupTableWidget,line->Row());
                QModelIndex index = ui->DossiersupTableWidget->model()->index(line->Row(),1);
                ui->DocupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                break;
             }
         }
    }
}

// ----------------------------------------------------------------------------------
// On sélectionne une ligne. On affiche le détail et on met en édition
// ----------------------------------------------------------------------------------
void dlg_impressions::LineSelect(UpTableWidget *table, int row)
{
    if (table->rowCount() == 0) return;
    if (row < 0) row = 0;
    if (row > table->rowCount()-1) row = table->rowCount()-1;

    UpLineEdit *line        = dynamic_cast<UpLineEdit*>(table->cellWidget(row,1));
    if (!line) return;

    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        UpLineEdit *line0   = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (line0) line0->deselect();
    }
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        UpLineEdit *line0   = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (line0) line0->deselect();
    }
    if (table == ui->DocupTableWidget)
    {
        wdg_docsbuttonframe->wdg_modifBouton        ->setEnabled(getDocumentFromRow(row)->iduser() == currentuser()->id());
        wdg_docsbuttonframe->wdg_moinsBouton        ->setEnabled(getDocumentFromRow(row)->iduser() == currentuser()->id());
        wdg_dossiersbuttonframe->wdg_modifBouton    ->setEnabled(false);
        wdg_dossiersbuttonframe->wdg_moinsBouton    ->setEnabled(false);
        if (m_mode == Selection)
        {
            ui->textFrame                   ->setVisible(true);
            m_currentdocument               = getDocumentFromRow(row);
            MetAJour(getDocumentFromRow(row)->texte(), false);
            ui->upTextEdit                  ->setText(m_listtexts.at(0));
            EffaceWidget(ui->textFrame);
            ui->DocPubliccheckBox           ->setChecked(getDocumentFromRow(row)->ispublic());
            ui->DocEditcheckBox             ->setChecked(getDocumentFromRow(row)->iseditable());
            ui->PrescriptioncheckBox        ->setChecked(getDocumentFromRow(row)->isprescription());
            ui->DocAdministratifcheckBox    ->setChecked(!getDocumentFromRow(row)->ismedical());
        }
    }
    else if (table == ui->DossiersupTableWidget)
    {
        ui->textFrame                       ->setVisible(false);
        wdg_docsbuttonframe->wdg_modifBouton        ->setEnabled(false);
        wdg_dossiersbuttonframe->wdg_modifBouton    ->setEnabled(getMetaDocumentFromRow(row)->iduser() == currentuser()->id());
        wdg_docsbuttonframe->wdg_moinsBouton        ->setEnabled(false);
        wdg_dossiersbuttonframe->wdg_moinsBouton    ->setEnabled(getMetaDocumentFromRow(row)->iduser() == currentuser()->id());
    }
    line->selectAll();
}

// ----------------------------------------------------------------------------------
// Met à jour les champs du texte à afficher
// ----------------------------------------------------------------------------------
void dlg_impressions::MetAJour(QString texte, bool pourVisu)
{
    m_listedestinataires.clear();
    m_listtexts.clear();

    User *userEntete = (Datas::I()->users->getById(currentuser()->idsuperviseur()) == Q_NULLPTR? Datas::I()->users->superviseurs()->first() : Datas::I()->users->getById(currentuser()->idsuperviseur()));
    if (userEntete == Q_NULLPTR)
        return;
    QMap<QString,QVariant>  AgeTotal    = Utils::CalculAge(m_currentpatient->datedenaissance(), m_currentpatient->sexe(), ui->dateImpressiondateEdit->date());
    QString age                         = AgeTotal["toString"].toString();
    QString formule                     = AgeTotal["formule"].toString();

    texte.replace("{{" + DATEDOC + "}}"         , QDate::currentDate().toString(tr("d MMMM yyyy")));
    texte.replace("{{" + NOMPAT + "}},"         , m_currentpatient->nom() + ",");
    texte.replace("{{" + NOMPAT + "}} "         , m_currentpatient->nom() + " ");
    texte.replace("{{" + NOMPAT + "}}"          , m_currentpatient->nom());
    texte.replace("{{" + PRENOMPAT + "}},"      , m_currentpatient->prenom() + ",");
    texte.replace("{{" + PRENOMPAT + "}} "      , m_currentpatient->prenom() + " ");
    texte.replace("{{" + PRENOMPAT + "}}"       , m_currentpatient->prenom());
    if (userEntete->titre().size())
        texte.replace("{{" + TITRUSER + "}}"    , userEntete->titre() + " " + userEntete->prenom() + " " + userEntete->nom());
    else
        texte.replace("{{" + TITRUSER + "}}"    , userEntete->prenom() + " " + userEntete->nom());
    texte.replace("{{" + DDNPAT + "}}"          , m_currentpatient->datedenaissance().toString((tr("d MMMM yyyy"))));
    texte.replace("{{" + TITREPAT + "}} "       , formule + " ");
    texte.replace("{{" + TITREPAT + "}}"        , formule);
    texte.replace("{{" + AGEPAT + "}}"          , age);
    QString telephone = "";
    if (m_currentpatient->telephone() != "")
        telephone= m_currentpatient->telephone();
    if (m_currentpatient->portable() != "")
        telephone = m_currentpatient->portable();
    texte.replace("{{" + TELEPHONE + "}}"       , telephone);
    texte.replace("{{" + SEXEPAT + "}}"         , m_currentpatient->sexe());
    if (m_currentpatient->sexe() == "F")
        texte.replace("(e)" ,"e");
    else if (m_currentpatient->sexe() == "M")
        texte.replace("(e)" ,"");

    QString form = NOCOR, form2 = NOCOR;
    Correspondant * cor = Datas::I()->correspondants->getById(m_currentpatient->idmg());
    if (cor != Q_NULLPTR)
    {
        if (cor->sexe() == "F")
        {
            form = tr("Madame le docteur ");
            form2 = tr("Ma chère consoeur");
        }
        else if (cor->sexe() == "M")
        {
            form = tr("Monsieur le docteur ");
            form2 = tr("Mon cher confrère");
        }
        else
            form2 = tr("Ma chère consoeur, mon cher confrère");
        form += cor->prenom() + " " + cor->nom();
    }
    texte.replace("{{" + MGPAT + "}}"               , form);
    texte.replace("{{" + MGPATTITRE + "}}"          , (cor != Q_NULLPTR? (cor->ismedecin()? tr("Docteur") : "") : ""));
    texte.replace("{{" + POLITESSEMG + "}}"         , form2);
    texte.replace("{{" + NOMMG + "}}"               , (cor != Q_NULLPTR? cor->nom(): ""));
    texte.replace("{{" + PRENOMMG + "}}"            , (cor != Q_NULLPTR? cor->prenom(): ""));

    if (texte.contains("{{" + KERATO + "}}"))
    {
        if (DataBase::I()->donneesOphtaPatient()->ismesurekerato())
        {
            QString kerato = "";
            if (DataBase::I()->donneesOphtaPatient()->K1OD() >0)
            {
                if (DataBase::I()->donneesOphtaPatient()->dioptriesK1OD()!=0.0)
                    kerato += "<font color = " COULEUR_TITRES "><b>" + tr("KOD:") + "</b></font> "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K1OD(),'f',2)
                            + "/"
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K2OD(),'f',2)
                            + " Km = "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->KMOD(),'f',2)
                            + " - "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesK1OD(),'f',2)
                            + "/"
                            + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesK2OD(),'f',2)
                            + " "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesKOD(),'f',2)
                            +  " à "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->axeKOD()) + "°";
                else
                    kerato += "<font color = " COULEUR_TITRES "><b>" + tr("KOD:") + "</b></font> "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K1OD(),'f',2)
                            + " à "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->axeKOD())
                            + "°/"
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K2OD(),'f',2)
                            + " Km = "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->KMOD(),'f',2) ;
            }
            if (DataBase::I()->donneesOphtaPatient()->dioptriesK1OD()!=0.0 && DataBase::I()->donneesOphtaPatient()->dioptriesK1OG()!=0.0)
                kerato += "<br/>";
            if (DataBase::I()->donneesOphtaPatient()->K1OG() >0)
            {
                if (DataBase::I()->donneesOphtaPatient()->dioptriesK1OG()!=0.0)
                    kerato += "<font color = " COULEUR_TITRES "><b>" + tr("KOG:") + "</b></font> "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K1OG(),'f',2)
                            + "/"
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K2OG(),'f',2)
                            + " Km = "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->KMOG(),'f',2)
                            + " - "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesK1OG(),'f',2)
                            + "/"
                            + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesK2OG(),'f',2)
                            + " "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesKOG(),'f',2)
                            +  " à "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->axeKOG()) + "°";
                else
                    kerato += "<font color = " COULEUR_TITRES "><b>" + tr("KOG:") + "</b></font> "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K1OG(),'f',2)
                            + " à "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->axeKOG())
                            + "°/"
                            + QString::number(DataBase::I()->donneesOphtaPatient()->K2OG(),'f',2)
                            + " Km = "
                            + QString::number(DataBase::I()->donneesOphtaPatient()->KMOG(),'f',2) ;
            }
            texte.replace("{{" + KERATO + "}}",kerato);
        }
        else
            texte.replace("{{" + KERATO + "}}",tr("pas de données de keratométrie retrouvées"));
    }
    if (texte.contains("{{" + REFRACT + "}}"))
    {
        QString formuleOD (""), formuleOG(""), refract ("");
        bool okOD (false), okOG (false);
        QMapIterator<int, Refraction*> itref(*Datas::I()->refractions->refractions());
        itref.toBack();
        while (itref.hasPrevious() && !okOD && !okOG) {
            itref.previous();
            if (itref.value()->typemesure() == Refraction::Acuite)
            {
                if (!okOD)
                {
                    formuleOD = itref.value()->formuleOD();
                    okOD = true;
                }
                if (!okOG)
                {
                    formuleOG = itref.value()->formuleOG();
                    okOG = true;
                }
            }
        }
        if (okOD || okOG)
        {
            if (okOD)
                refract += "<font color = " COULEUR_TITRES "><b>" + tr("OD:") + "</b></font> " + formuleOD;
            if (okOD && okOG)
                refract += "<br/>";
            if (okOG)
                refract += "<font color = " COULEUR_TITRES "><b>" + tr("OG:") + "</b></font> " + formuleOG;
            texte.replace("{{" + REFRACT + "}}",refract);
        }
        else
            texte.replace("{{" + REFRACT + "}}",tr("pas de données de refraction retrouvées"));
    }
    if (m_currentintervention != Q_NULLPTR)
    {
        QString txt = "";
        SessionOperatoire * session = Datas::I()->sessionsoperatoires->getById(m_currentintervention->idsession());
        Site * sit = Q_NULLPTR;
        if (session != Q_NULLPTR)
            sit = Datas::I()->sites->getById(session->idlieu());
        TypeIntervention *typ = Datas::I()->typesinterventions->getById(m_currentintervention->idtypeintervention());

        txt = "((" + TITREDATEINTERVENTION + "//" + DATEINTERVENTION + "))";
        texte.replace(txt,  (session != Q_NULLPTR? session->date().toString(tr("d MMMM yyyy")) : "null"));

        txt = "((" + TITREHEUREINTERVENTION + "//" + HEUREINTERVENTION + "))";
        texte.replace(txt, m_currentintervention->heure().toString(tr("HH:mm")));

        txt = "((" + TITRECOTEINTERVENTION + "//" + COTEINTERVENTION + "))";
        {
            QString cote ="";
            if (m_currentintervention->cote() == Utils::Droit)
                cote = tr("l'oeil droit");
            else if (m_currentintervention->cote() == Utils::Gauche)
                cote = tr("l'oeil gauche");
            else if (m_currentintervention->cote() == Utils::Les2)
                cote = tr("des deux yeux");
            texte.replace(txt, cote);
        }

        txt = "((" + TITREANESTHINTERVENTION + "//" + ANESTHINTERVENTION + "))";
        {
            QString anesth ="";
            if (m_currentintervention->anesthesie() == Intervention::Locale)
                anesth = tr("Locale");
            else if (m_currentintervention->anesthesie() == Intervention::LocoRegionale)
                anesth = tr("LocoRegionale");
            else if (m_currentintervention->anesthesie() == Intervention::Generale)
                anesth = tr("Générale");
            texte.replace(txt, anesth);
        }

        txt = "((" + TITRESITEINTERVENTION + "//" + SITEINTERVENTION + "))";
        texte.replace(txt, (sit != Q_NULLPTR? sit->nom() : "null"));

        txt = "((" + TITRETYPEINTERVENTION + "//" + TYPEINTERVENTION + "))";
        texte.replace(txt, (typ != Q_NULLPTR? typ->typeintervention() : "null"));
    }
    int pos = 0;
    QRegExp reg;
    reg.setPattern("([{][{].*CORRESPONDANT.*[}][}])");
    if (reg.indexIn(texte, pos) != -1)
    {
        QList<Correspondant*> listcor;
        Correspondant * cor = Datas::I()->correspondants->getById(m_currentpatient->idmg());
        if (cor != Q_NULLPTR)
            listcor << cor;
        Correspondant * spe1 = Datas::I()->correspondants->getById(m_currentpatient->idspe1());
        if (spe1 != Q_NULLPTR)
            listcor << spe1;
        Correspondant * spe2 = Datas::I()->correspondants->getById(m_currentpatient->idspe2());
        if (spe2 != Q_NULLPTR)
            listcor << spe2;
        Correspondant * spe3 = Datas::I()->correspondants->getById(m_currentpatient->idspe3());
        if (spe3 != Q_NULLPTR)
            listcor << spe3;
        if (listcor.size()==0)
            texte.replace(reg, NOCOR);
        else if (listcor.size()==1)
        {
            QString form = "", form2 = "";
            if (listcor.at(0)->sexe() == "F")
            {
                if (listcor.at(0)->ismedecin() == 1)
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
            else if (listcor.at(0)->sexe() == "M")
            {
                if (listcor.at(0)->ismedecin() == 1)
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

            form += listcor.at(0)->prenom() + " " + listcor.at(0)->nom();
            texte.replace("{{" + CORPAT + "}}"         ,form);
            texte.replace("{{" + POLITESSECOR + "}}"   ,form2);
            texte.replace("{{" + NOMCOR + "}}}"        ,listcor.at(0)->nom());
            texte.replace("{{" + PRENOMCOR + "}}"      ,listcor.at(0)->prenom());
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
            if (m_listedestinataires.size()>0)
            {
                for (int j=0; j<m_listedestinataires.size(); j++)
                {
                    Correspondant * cor = m_listedestinataires.at(j);
                    if (cor == Q_NULLPTR)
                        continue;
                    QString txtdef = texte;
                    QString form = "", form2 = "";
                    if (cor->sexe() == "F")
                    {
                        if (cor->ismedecin() == 1)
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
                    else if (cor->sexe() == "M")
                    {
                        if (cor->ismedecin() == 1)
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

                    form += cor->prenom() + " " + cor->nom();
                    txtdef.replace("{{" + CORPAT + "}}"         , form);
                    txtdef.replace("{{" + POLITESSECOR + "}}"   , form2);
                    txtdef.replace("{{" + NOMCOR + "}}}"        , cor->nom());
                    txtdef.replace("{{" + PRENOMCOR + "}}"      , cor->prenom());
                    m_listtexts << txtdef;
                }
            }
        }
    }
    if (m_listedestinataires.size() == 0)
        m_listtexts << texte;
}

void dlg_impressions::ChoixCorrespondant(QList<Correspondant *> listcor)
{
    m_listedestinataires.clear();
    dlg_askcorrespondant                 = new UpDialog(this);
    dlg_askcorrespondant                 ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
    dlg_askcorrespondant                 ->setAttribute(Qt::WA_DeleteOnClose);
    dlg_askcorrespondant                 ->AjouteLayButtons();
    QTableView  *tblCorresp     = new QTableView(dlg_askcorrespondant);
    QStandardItemModel *m_modele = new QStandardItemModel;
    QStandardItem *pitem;
    UpLabel     *label          = new UpLabel(dlg_askcorrespondant);
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
        pitem       = new QStandardItem(listcor.at(i)->prenom() + " " + listcor.at(i)->nom());
        pitem       ->setAccessibleDescription(QString::number(listcor.at(i)->id()));
        pitem       ->setEditable(false);
        pitem       ->setCheckable(true);
        pitem       ->setCheckState(Qt::Unchecked);
        if (fm.width(pitem->text()) > largeurcolonne)
            largeurcolonne = fm.width(pitem->text());
        m_modele     ->appendRow(pitem);
    }
    tblCorresp  ->setModel(m_modele);
    int nrows   = m_modele->rowCount();
    int haut    = nrows*30 + 2;             //la valeur 30 correpsond à la hauteur figée de la ligne par la présence du checkbox
    tblCorresp  ->setFixedHeight(haut);
    if ((largeurcolonne + 40 + 2) > largfinal)
        largfinal = largeurcolonne + 40 + 2;
    tblCorresp  ->setColumnWidth(0, largfinal - 2);
    tblCorresp  ->setFixedWidth(largfinal);
    label       ->setFixedWidth(largfinal);
    label       ->setFixedHeight(hauteurligne + 2);
    dlg_askcorrespondant->dlglayout()   ->insertWidget(0,tblCorresp);
    dlg_askcorrespondant->dlglayout()   ->insertWidget(0,label);

    dlg_askcorrespondant ->setModal(true);
    dlg_askcorrespondant->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);

    connect(dlg_askcorrespondant->OKButton,   &QPushButton::clicked, [=] {ListidCor();});

    dlg_askcorrespondant ->exec();
}

void dlg_impressions::ListidCor()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(dlg_askcorrespondant->findChildren<QTableView *>().at(0)->model());
    for (int i=0; i< model->rowCount(); i++)
        if (model->item(i)->checkState() == Qt::Checked)
        {
            Correspondant * cor = Datas::I()->correspondants->getById(model->item(i)->accessibleDescription().toInt());
            if (cor != Q_NULLPTR)
                m_listedestinataires << cor;
        }
    if (m_listedestinataires.size() > 0)
        dlg_askcorrespondant->accept();
}

// ----------------------------------------------------------------------------------
// Remplissage des UpTableWidget avec les documents de la base.
// ----------------------------------------------------------------------------------
void dlg_impressions::Remplir_TableWidget()
{
    UpLineEdit  *upLine0 = Q_NULLPTR;
    int         i = 0;

    //Remplissage Table Documents
    Datas::I()->impressions->initListe();
    for (int i = 0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        upLine0 = dynamic_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1));
        if (upLine0 != Q_NULLPTR)
            upLine0->disconnect();
    }
    ui->DocupTableWidget->clearContents();
    ui->DocupTableWidget->setRowCount(Datas::I()->impressions->impressions()->size());

    foreach (Impression *doc, *Datas::I()->impressions->impressions())
    {
        SetDocumentToRow(doc, i);
        i++;
    }
    TriDocupTableWidget();


    //Remplissage Table Dossiers
    i = 0;
    Datas::I()->metadocuments->initListe();
    for (int i = 0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        upLine0 = dynamic_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1));
        if (upLine0)
            upLine0->disconnect();
    }
    ui->DossiersupTableWidget->clearContents();
    ui->DossiersupTableWidget->setRowCount(Datas::I()->metadocuments->dossiersimpressions()->size());
    foreach (DossierImpression *metadoc, *Datas::I()->metadocuments->dossiersimpressions())
    {
        SetMetaDocumentToRow(metadoc, i);
        i++;
    }
    TriDossiersupTableWidget();
}

void dlg_impressions::SetDocumentToRow(Impression*doc, int row)
{
    UpLineEdit          *upLine0;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem2;

    QFontMetrics fm(qApp->font());
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    QPalette palette;
    palette.setColor(QPalette::Text,QColor(0,0,140));

    upLine0 = new UpLineEdit() ;
    pItem1  = new QTableWidgetItem() ;
    pItem2  = new QTableWidgetItem() ;

    int col = 0;
    QWidget * w = new QWidget(ui->DocupTableWidget);
    UpCheckBox *Check = new UpCheckBox(w);
    Check->setCheckState(Qt::Unchecked);
    Check->setRowTable(row);
    Check->setFocusPolicy(Qt::NoFocus);
    connect(Check, &QCheckBox::clicked, this, [=] {EnableOKPushButton(Check);});
    QHBoxLayout *l = new QHBoxLayout();
    l->setAlignment( Qt::AlignCenter );
    l->addWidget(Check);
    l->setContentsMargins(0,0,0,0);
    w->setLayout(l);
    ui->DocupTableWidget->setCellWidget(row,col,w);
    col++; //1
    upLine0->setText(doc->resume());                                // resume
    upLine0->setMaxLength(50);
    upLine0->setRow(row);
    upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                           "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
    upLine0->setFocusPolicy(Qt::NoFocus);
    if (doc->iduser() != currentuser()->id())
    {
        upLine0->setFont(disabledFont);
        upLine0->setPalette(palette);
        upLine0->setContextMenuPolicy(Qt::NoContextMenu);
    }
    else
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->DocupTableWidget->setCellWidget(row,col,upLine0);
    col++; //2
    pItem1->setText(QString::number(doc->id()));                    // idDocument
    ui->DocupTableWidget->setItem(row,col,pItem1);
    col++; //3
    UpLabel*lbl = new UpLabel(ui->DocupTableWidget);
    lbl->setAlignment(Qt::AlignCenter);
    if (doc->ispublic())
        lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15));        //WARNING : icon scaled : pxLoupe 15,15
    ui->DocupTableWidget->setCellWidget(row,col,lbl);               // Public
    col++; //4
    UpLabel*lbl1 = new UpLabel(ui->DocupTableWidget);
    lbl1->setAlignment(Qt::AlignCenter);
    if (doc->iseditable())
        lbl1->setPixmap(Icons::pxBlackCheck().scaled(15,15));       //WARNING : icon scaled : pxLoupe 15,15
    ui->DocupTableWidget->setCellWidget(row,col,lbl1);              //Editable
    col++; //5
    UpLabel*lbl11 = new UpLabel(ui->DocupTableWidget);
    lbl11->setAlignment(Qt::AlignCenter);
    if (!doc->ismedical())
        lbl11->setPixmap(Icons::pxBlackCheck().scaled(15,15));      //WARNING : icon scaled : pxLoupe 15,15
    ui->DocupTableWidget->setCellWidget(row,col,lbl11);             // Administratif
    col++; //6
    pItem2->setText("1" + upLine0->text());                         // Check+text   -> sert pour le tri de la table
    ui->DocupTableWidget->setItem(row,col,pItem2);

    ui->DocupTableWidget->setRowHeight(row,int(fm.height()*1.3));
}

void dlg_impressions::SetMetaDocumentToRow(DossierImpression*dossier, int row)
{
    UpLineEdit          *upLine0;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem2;

    QFontMetrics fm(qApp->font());
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    QPalette palette;
    palette.setColor(QPalette::Text,QColor(0,0,140));

    pItem1  = new QTableWidgetItem() ;
    upLine0 = new UpLineEdit() ;
    pItem2  = new QTableWidgetItem() ;

    int col = 0;
    QWidget * w = new QWidget(ui->DossiersupTableWidget);
    UpCheckBox *Check = new UpCheckBox(w);
    Check->setCheckState(Qt::Unchecked);
    Check->setRowTable(row);
    Check->setFocusPolicy(Qt::NoFocus);
    connect(Check, &QCheckBox::clicked, this, [=] {EnableOKPushButton(Check);});
    QHBoxLayout *l = new QHBoxLayout();
    l->setAlignment( Qt::AlignCenter );
    l->addWidget(Check);
    l->setContentsMargins(0,0,0,0);
    w->setLayout(l);
    ui->DossiersupTableWidget->setCellWidget(row,col,w);

    col++; //1
    upLine0->setText(dossier->resume());                           // resume
    upLine0->setMaxLength(80);
    upLine0->setRow(row);
    upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                           "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
    upLine0->setFocusPolicy(Qt::NoFocus);
    if (dossier->iduser() != currentuser()->id())
    {
        upLine0->setFont(disabledFont);
        upLine0->setPalette(palette);
        upLine0->setContextMenuPolicy(Qt::NoContextMenu);
    }
    else
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->DossiersupTableWidget->setCellWidget(row,col,upLine0);

    col++; //2
    UpLabel*lbl = new UpLabel(ui->DossiersupTableWidget);
    lbl->setAlignment(Qt::AlignCenter);
    if (dossier->ispublic())
        lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15));        //WARNING : icon scaled : pxLoupe 15,15
    ui->DossiersupTableWidget->setCellWidget(row,col,lbl);

    col++; //3
    pItem1->setText(QString::number(dossier->id()));                // idMetaDocument
    ui->DossiersupTableWidget->setItem(row,col,pItem1);

    col++; //4
    pItem2->setText(dossier->resume());                             // Resume dans un QTableWidgetItem
    ui->DossiersupTableWidget->setItem(row,col,pItem2);


    ui->DossiersupTableWidget->setRowHeight(row,int(fm.height()*1.3));
}

// ----------------------------------------------------------------------------------
// Supprime Document
// ----------------------------------------------------------------------------------
void dlg_impressions::SupprimmDocument(int row)
{
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le document\n") + getDocumentFromRow(row)->resume() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton NoBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer le document"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != &NoBouton)
    {
        db->SupprRecordFromTable(getDocumentFromRow(row)->id(),
                                 CP_ID_IMPRESSIONS,
                                 TBL_IMPRESSIONS,
                                 tr("Impossible de supprimer le document\n") + getDocumentFromRow(row)->resume() + tr("\n ... et je ne sais pas pourquoi..."));
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
void dlg_impressions::SupprimmDossier(int row)
{
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le  dossier\n") + getMetaDocumentFromRow(row)->resume() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton(tr("Supprimer le dosssier"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != &NoBouton)
    {
        QStringList locklist;
        locklist << TBL_DOSSIERSIMPRESSIONS << TBL_JOINTURESIMPRESSIONS ;
        if (db->SupprRecordFromTable( getMetaDocumentFromRow(row)->id(),
                                      CP_ID_DOSSIERIMPRESSIONS,
                                      TBL_DOSSIERSIMPRESSIONS,
                                      tr("Impossible de suppprimer le dossier") + "\n" + getMetaDocumentFromRow(row)->resume() + "!\n ... " + tr("et je ne sais pas pourquoi") + "...\nRufus"))
            db->SupprRecordFromTable(getMetaDocumentFromRow(row)->id(), CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS , TBL_JOINTURESIMPRESSIONS);
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
void dlg_impressions::TriDocupTableWidget()
{
    ui->DocupTableWidget->sortByColumn(6,Qt::AscendingOrder);
    for (int i=0; i<ui->DocupTableWidget->rowCount(); i++)
    {
        static_cast<UpLineEdit*>(ui->DocupTableWidget->cellWidget(i,1))->setRow(i);
        static_cast<QWidget*>(ui->DocupTableWidget->cellWidget(i,0))->findChildren<UpCheckBox*>().at(0)->setRowTable(i);
    }
}

// ----------------------------------------------------------------------------------
// Tri de la table dossiers.
// ----------------------------------------------------------------------------------
void dlg_impressions::TriDossiersupTableWidget()
{
    ui->DossiersupTableWidget->sortByColumn(4,Qt::AscendingOrder);
    for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
    {
        static_cast<UpLineEdit*>(ui->DossiersupTableWidget->cellWidget(i,1))->setRow(i);
        static_cast<QWidget*>(ui->DossiersupTableWidget->cellWidget(i,0))->findChildren<UpCheckBox*>().at(0)->setRowTable(i);
    }
}

// ----------------------------------------------------------------------------------
// Modification du Document dans la base.
// ----------------------------------------------------------------------------------
void dlg_impressions::UpdateDocument(int row)
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

    QString req =   "UPDATE " TBL_IMPRESSIONS
            " SET " CP_TEXTE_IMPRESSIONS " = '" + Utils::correctquoteSQL(ui->upTextEdit->toHtml())     + "'"
            ", " CP_RESUME_IMPRESSIONS " = '"  + Utils::correctquoteSQL(line->text().left(100)) + "'";
    req += ", " CP_DOCPUBLIC_IMPRESSIONS " = ";
    req += (ui->DocPubliccheckBox->isChecked()?  "1" : "null");
    req += ", " CP_PRESCRIPTION_IMPRESSIONS " = ";
    req += (ui->PrescriptioncheckBox->isChecked()?  "1" : "null");
    req += ", " CP_EDITABLE_IMPRESSIONS " = ";
    req += (ui->DocEditcheckBox->isChecked()?  "1" : "null");
    req += ", " CP_MEDICAL_IMPRESSIONS " = ";
    req += (ui->DocAdministratifcheckBox->isChecked()?  "null" : "1");
    req += " WHERE  " CP_ID_IMPRESSIONS " = " + QString::number(getDocumentFromRow(line->Row())->id());
    db->StandardSQL(req, tr("Erreur de mise à jour du document dans ") + TBL_IMPRESSIONS);

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
                LineSelect(ui->DocupTableWidget,line->Row());
                QModelIndex index = ui->DocupTableWidget->model()->index(line->Row(),1);
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
void dlg_impressions::UpdateDossier(int row)
{
    QStringList listid;
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->DossiersupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));
    for (int l=0; l<ui->DocupTableWidget->rowCount(); l++)
    {
        QWidget *Widg =  dynamic_cast<QWidget*>(ui->DocupTableWidget->cellWidget(l,0));
        if (Widg != Q_NULLPTR)
        {
            UpCheckBox *DocCheck = Widg->findChildren<UpCheckBox*>().at(0);
            if (DocCheck->isChecked())
                listid << QString::number(getDocumentFromRow(l)->id());
        }
    }
    if (listid.size() == 0)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Modification de Dossier"), tr("Veuillez cocher au moins un document, SVP !"));
        return;
    }

    QString iddoss  = QString::number(getMetaDocumentFromRow(row)->id());

    if (ChercheDoublon(line->text(), row))
    {
        line->setFocus();
        line->selectAll();
        return;
    }

    db->SupprRecordFromTable(iddoss.toInt(), "idMetaDocument", TBL_JOINTURESIMPRESSIONS);

    QString req     = "insert into " TBL_JOINTURESIMPRESSIONS " (" CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS "," CP_IDDOCUMENT_JOINTURESIMPRESSIONS ") Values ";
    for (int i=0; i<listid.size(); i++)
    {
        req += "(" + iddoss + ", " + listid.at(i) + ")";
        if (i<listid.size()-1)
            req += ",";
    }
    db->StandardSQL(req);

    req =   "UPDATE " TBL_DOSSIERSIMPRESSIONS
            " SET " CP_RESUME_DOSSIERIMPRESSIONS " = '"  + Utils::correctquoteSQL(line->text().left(100)) + "'"
            " WHERE " CP_ID_DOSSIERIMPRESSIONS " = " + iddoss;
    db->StandardSQL(req, tr("Erreur de mise à jour du dossier dans ") + TBL_DOSSIERSIMPRESSIONS);

    Remplir_TableWidget();
    if (ui->DocupTableWidget->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
    {
        ConfigMode(Selection);
        for (int i=0; i<ui->DossiersupTableWidget->rowCount(); i++)
        {
            if (QString::number(getMetaDocumentFromRow(i)->id()) == iddoss)
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
bool dlg_impressions::VerifDocumentPublic(int row, bool msg)
{
    bool ok;
    int iddoc = getDocumentFromRow(row)->id();
    QString req = "select " CP_ID_DOSSIERIMPRESSIONS ", " CP_RESUME_DOSSIERIMPRESSIONS " from " TBL_DOSSIERSIMPRESSIONS
                  " where " CP_ID_DOSSIERIMPRESSIONS " in (select " CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " from " TBL_JOINTURESIMPRESSIONS " where " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " = " + QString::number(iddoc) +
                  ") and " CP_PUBLIC_DOSSIERIMPRESSIONS " =1";
    QList<QVariantList> listdossiers = db->StandardSelectSQL(req,ok);
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
void dlg_impressions::VerifDossiers()
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
                QString req = "select " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " from " TBL_JOINTURESIMPRESSIONS
                              " where " CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " = " + QString::number(getMetaDocumentFromRow(j)->id());
                QList<QVariantList> listdocs = db->StandardSelectSQL(req,ok);
                if (listdocs.size() > 0)
                {
                    QStringList listid;
                    for (int i=0; i<listdocs.size(); i++)
                        listid << listdocs.at(i).at(0).toString();
                    bool a = false;
                    for (int k=0; k<listid.size(); k++)
                    {
                        for (int l=0; l<ui->DocupTableWidget->rowCount(); l++)
                            if (listid.contains(QString::number(getDocumentFromRow(l)->id())))
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
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------------
// Verifie qu'un dossier peut être rendu public
// ----------------------------------------------------------------------------------
bool dlg_impressions::VerifDossierPublic(int row, bool msg)
{
    bool ok;
    int iddossier = getMetaDocumentFromRow(row)->id();
    QString req = "select " CP_ID_IMPRESSIONS ", " CP_RESUME_IMPRESSIONS " from " TBL_IMPRESSIONS
                  " where " CP_ID_IMPRESSIONS " in (select " CP_IDDOCUMENT_JOINTURESIMPRESSIONS " from " TBL_JOINTURESIMPRESSIONS " where " CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " = " + QString::number(iddossier) +
                  ") and " CP_DOCPUBLIC_IMPRESSIONS " is null";
    QList<QVariantList> listdocs = db->StandardSelectSQL(req,ok);
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
