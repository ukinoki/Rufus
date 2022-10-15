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

#include "dlg_impressions.h"
#include "ui_dlg_impressions.h"

dlg_impressions::dlg_impressions(Patient *pat, Intervention *intervention, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_impressions)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_currentpatient     = pat;
    if (intervention)
        m_currentintervention = intervention;
    if (!pat->isalloaded())
        Datas::I()->patients->loadAll(pat, Item::Update);
    QString nom         = pat->nom();
    QString prenom      = pat->prenom();
    setWindowTitle(tr("Préparer un document pour ") + nom + " " + prenom);
    setWindowIcon(Icons::icLoupe());


    restoreGeometry(proc->settings()->value(Position_Fiche "Documents").toByteArray());
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    ui->PrescriptioncheckBox->setVisible(currentuser()->isSoignant());
    wdg_docsbuttonframe     = new WidgetButtonFrame(ui->DocsupTableView);
    wdg_docsbuttonframe     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_docsbuttonframe     ->addSearchLine();
    wdg_dossiersbuttonframe = new WidgetButtonFrame(ui->DossiersupTableView);
    wdg_dossiersbuttonframe ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);

    ui->DocsPublicscheckBox->setImmediateToolTip(tr("Cocher cette case pour visualiser les dossiers et documents rendus publics par leurs créateurs\n"
                                                    "Ces dossiers et documents sont affichés en caractères italiques bleus"));

    ui->upTextEdit->disconnect(); // pour déconnecter la fonction MenuContextuel intrinsèque de la classe UpTextEdit

    connect (wdg_docsbuttonframe->searchline(), &QLineEdit::textEdited,                 this,   &dlg_impressions::FiltreListe);
    connect (ui->OKupPushButton,                &QPushButton::clicked,                  this,   &dlg_impressions::OKpushButtonClicked);
    connect (ui->AnnulupPushButton,             &QPushButton::clicked,                  this,   &dlg_impressions::Annulation);
    connect (ui->dateImpressiondateEdit,        &QDateEdit::dateChanged,                this,   [=] {
        if (m_currentdocument)
        {
            MetAJour(m_currentdocument->texte(),false);
            ui->upTextEdit                  ->setText(m_listtexts.at(0));
        }
    });
    connect (ui->DocPubliccheckBox,             &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocPubliccheckBox);});
    connect (ui->DocsPublicscheckBox,           &QCheckBox::clicked,                    this,   [=](bool a)
                                                                                                {
                                                                                                    ItemsList::update(currentuser(), CP_AFFICHEDOCSPUBLICS_USR,a);
                                                                                                    FiltreListe();
                                                                                                });
    connect (ui->DocEditcheckBox,               &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocEditcheckBox);});
    connect (ui->DocAdministratifcheckBox,      &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->DocAdministratifcheckBox);});
    connect (ui->PrescriptioncheckBox,          &QCheckBox::clicked,                    this,   [=] {CheckPublicEditablAdmin(ui->PrescriptioncheckBox);});
    connect (ui->upTextEdit,                    &QWidget::customContextMenuRequested,   this,   &dlg_impressions::MenuContextuelTexteDocument);
    connect (ui->upTextEdit,                    &QTextEdit::textChanged,                this,   [=] {EnableOKPushButton();});
    connect (ui->upTextEdit,                    &UpTextEdit::dblclick,                  this,   &dlg_impressions::dblClicktextEdit);
    connect (ui->DupliOrdocheckBox,             &QCheckBox::clicked,                    this,   [=] {OrdoAvecDupli(ui->DupliOrdocheckBox->isChecked());});
    connect (wdg_docsbuttonframe,               &WidgetButtonFrame::choix,              this,   [=] {ChoixButtonFrame(wdg_docsbuttonframe);});
    connect (wdg_dossiersbuttonframe,           &WidgetButtonFrame::choix,              this,   [=] {ChoixButtonFrame(wdg_dossiersbuttonframe);});

    // Mise en forme de la table Documents et de la table Dossiers
    QList<QTableView*> listtables;
    listtables << ui->DocsupTableView << ui->DossiersupTableView;
    foreach(QTableView* table, listtables)
    {
        table->setPalette(QPalette(Qt::white));
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setMouseTracking(true);
        table->setContextMenuPolicy(Qt::CustomContextMenu);
        table->horizontalHeader()->setVisible(true);
        table->horizontalHeader()->setIconSize(QSize(30,30));
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        table->verticalHeader()->setDefaultSectionSize(int(QFontMetrics(qApp->font()).height()*1.3));
        table->verticalHeader()->setVisible(false);
    }

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

    ui->DupliOrdocheckBox->setChecked(proc->settings()->value(Imprimante_OrdoAvecDupli).toString() == "YES");
    ui->DocsPublicscheckBox->setChecked(currentuser()->affichedocspublics());


    ui->textFrame->installEventFilter(this);
    m_opacityeffect  = new QGraphicsOpacityEffect();
    t_timerefface    = new QTimer(this);

    ui->ALDcheckBox->setChecked(m_currentpatient->isald());

    // supprime les jointures inutilisées
    db->NettoieJointuresDossiersImpressions();

    Remplir_TableView();
    if (m_docsmodel->rowCount() == 0)
        ConfigMode(CreationDOC);
    else
        ConfigMode(Selection);
    wdg_docsbuttonframe->searchline()->setFocus();

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

void dlg_impressions::AfficheDocsPublicsAutresUtilisateurs(bool affiche)
{
    wdg_docsbuttonframe->searchline()->setText("");
    for (int j=0; j<m_docsmodel->rowCount(); j++)
    {
        Impression *doc = getDocumentFromIndex(m_docsmodel->index(j,0));
        if (doc)
            if (doc->iduser() != currentuser()->id())
                ui->DocsupTableView->setRowHidden(m_docsmodel->getRowFromItem(doc),!affiche);
    }
    for (int j=0; j<m_dossiersmodel->rowCount(); j++)
    {
        DossierImpression *dossier = getDossierFromIndex(m_dossiersmodel->index(j,0));
        if (dossier)
            if (dossier->iduser() != currentuser()->id())
                ui->DossiersupTableView->setRowHidden(m_dossiersmodel->getRowFromItem(dossier),!affiche);
    }
}

void dlg_impressions::AfficheTexteDocument(Impression *doc)
{
    if (m_mode == Selection)
    {
        ui->textFrame                   ->setVisible(true);
        MetAJour(doc->texte(), false);
        ui->upTextEdit                  ->setText(m_listtexts.at(0));
        EffaceWidget(ui->textFrame);
        ui->DocPubliccheckBox           ->setChecked(doc->ispublic());
        ui->DocEditcheckBox             ->setChecked(doc->iseditable());
        ui->PrescriptioncheckBox        ->setChecked(doc->isprescription());
        ui->DocAdministratifcheckBox    ->setChecked(!doc->ismedical());
    }
}
// ----------------------------------------------------------------------------------
// Clic sur le bouton ANNULER.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_impressions::Annulation()
{
    switch (m_mode) {
    case CreationDOSS:
    case ModificationDOSS:{
        ui->DossiersupTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        int row = m_dossiersmodel->getRowFromItem(m_currentdossier);
        if (m_mode == ModificationDOSS && row < m_dossiersmodel->rowCount())
        {
            QModelIndex idx = m_dossiersmodel->index(row,1);
            ui->DossiersupTableView->closePersistentEditor(idx);
            m_dossiersmodel->setData(idx, m_currentdossier->resume());
            SetDossierToRow(m_currentdossier,row);
            EnableDossiersButtons(m_currentdossier);
        }
        else if (m_mode == CreationDOSS)
        {
            m_dossiersmodel->removeRow(row);
            if(m_dossiersmodel->rowCount() > 0 && row < m_dossiersmodel->rowCount())
                selectcurrentDossier(getDossierFromIndex(m_dossiersmodel->index(row,1)));
        }
        else
            Remplir_TableView();
        ConfigMode(Selection);
        AfficheDocsPublicsAutresUtilisateurs(currentuser()->affichedocspublics());
        break;
    }
    case Selection:
        reject();
        break;
    case CreationDOC:
    case ModificationDOC:{
        //qDebug() << m_currentdocument->resume();
        ui->DocsupTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        int row = m_docsmodel->getRowFromItem(m_currentdocument);
        if (m_mode == ModificationDOC && row < m_docsmodel->rowCount())
        {
            QModelIndex idx = m_docsmodel->index(row,1);
            ui->DocsupTableView->closePersistentEditor(idx);
            m_docsmodel->setData(idx, m_currentdocument->resume());
            SetDocumentToRow(m_currentdocument,row);
            EnableDocsButtons(m_currentdocument);
        }
        else if (m_mode == CreationDOC)
        {
            m_docsmodel->removeRow(row);
            if(m_docsmodel->rowCount() > 0 && row < m_docsmodel->rowCount())
                selectcurrentDocument(getDocumentFromIndex(m_docsmodel->index(row,1)));
        }
        else
            Remplir_TableView();
        ConfigMode(Selection);
    }
    }
}


void dlg_impressions::ChoixButtonFrame(WidgetButtonFrame *widgbutt)
{
    if (widgbutt== wdg_docsbuttonframe)
    {
        switch (wdg_docsbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            ConfigMode(CreationDOC);
            break;
        case WidgetButtonFrame::Modifier:
            ConfigMode(ModificationDOC);
            break;
        case WidgetButtonFrame::Moins:
            SupprimmeDocument(m_currentdocument);
            break;
        }
    }
    else if (widgbutt== wdg_dossiersbuttonframe)
    {
        switch (wdg_dossiersbuttonframe->Choix()) {
        case WidgetButtonFrame::Plus:
            ConfigMode(CreationDOSS);
            break;
        case WidgetButtonFrame::Modifier:
            ConfigMode(ModificationDOSS);
            break;
        case WidgetButtonFrame::Moins:
            SupprimmeDossier(m_currentdossier);
            break;
        }
    }
}

void dlg_impressions::CheckPublicEditablAdmin(QCheckBox *check)
{
    int row = m_docsmodel->getRowFromItem(m_currentdocument);
    if (row == -1)
        return;
    if (check == ui->DocPubliccheckBox)
    {
        QStandardItem *pitem = m_docsmodel->item(row, 2);
        if(pitem)
        {
            if (check->isChecked())
                pitem->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                pitem->setData(QPixmap(),Qt::DecorationRole);
        }
    }
    else if (check == ui->DocEditcheckBox)
    {
        QStandardItem *pitem = m_docsmodel->item(row, 3);
        if(pitem)
        {
            if (check->isChecked())
                pitem->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                pitem->setData(QPixmap(),Qt::DecorationRole);
        }
    }
    else if (check == ui->DocAdministratifcheckBox)
    {
        QStandardItem *pitem = m_docsmodel->item(row, 4);
        if (check->isChecked())
        {
            if(pitem)
                pitem->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            ui->PrescriptioncheckBox->setChecked(false);
        }
        else
            pitem->setData(QPixmap(),Qt::DecorationRole);
    }
    else if (check == ui->PrescriptioncheckBox)
    {
        if (check->isChecked())
        {
            QStandardItem *pitem = m_docsmodel->item(row, 4);
            if(pitem)
                pitem->setData(QPixmap(),Qt::DecorationRole);
            ui->DocAdministratifcheckBox->setChecked(false);
        }
    }
}

void dlg_impressions::dblClicktextEdit()
{
    if (!m_currentdocument)
        return;
    if (m_mode == Selection)
        ConfigMode(ModificationDOC);
}

QString dlg_impressions::DocumentToolTip(Impression *doc)
{
    if (!doc)
        return "";
    QTextEdit *text = new QTextEdit;
    MetAJour(doc->texte(),false);
    text->setText(m_listtexts.at(0));
    QString ab = text->toPlainText();
    ab.replace(QRegularExpression("\n\n[\n]*"),"\n");
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
    return ResumeItem;
}

QString dlg_impressions::DossierToolTip(DossierImpression *dossier)
{
    if (!dossier)
        return "";
    if (!dossier->haslistdocsloaded())
        Datas::I()->metadocuments->loadlistedocs(dossier);
    QList<int> listiddocs = dossier->listiddocs();
    QString resume = "";
    for (int i=0; i<listiddocs.size(); i++)
    {
        Impression *impr = Datas::I()->impressions->getById(listiddocs.at(i));
        if (impr)
        {
            resume += ( i>0? "\n" : "");
            resume += impr->resume();
        }
    }
    return resume;
}

void dlg_impressions::EnableDocsButtons(Impression* doc)
{
//    qDebug() << "doc->iduser()" << doc->iduser();
//    qDebug() << "currentuser()->id()" << currentuser()->id();
    if (doc)
    {
        wdg_docsbuttonframe->wdg_moinsBouton->setEnabled(doc->iduser() == currentuser()->id());
        wdg_docsbuttonframe->wdg_modifBouton->setEnabled(doc->iduser() == currentuser()->id());
    }
}

void dlg_impressions::EnableDossiersButtons(DossierImpression *dossier)
{
    wdg_dossiersbuttonframe->wdg_modifBouton->setEnabled(dossier);
    if (dossier)
        wdg_dossiersbuttonframe->wdg_moinsBouton->setEnabled(dossier->iduser() == currentuser()->id());
    else
        wdg_dossiersbuttonframe->wdg_moinsBouton->setEnabled(false);
}

// ----------------------------------------------------------------------------------
// Enable OKpushbutton
// ----------------------------------------------------------------------------------
void dlg_impressions::EnableOKPushButton(QModelIndex idx)
{
    ui->OKupPushButton->setShortcut(QKeySequence());
    if (m_mode == CreationDOC || m_mode == ModificationDOC)
    {
        int row = m_docsmodel->getRowFromItem(m_currentdocument);
        QString resume = m_docsmodel->data(m_docsmodel->index(row,1),Qt::DisplayRole).toString();
        if (resume.size() == 0)
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
        bool a = false;
        int row = m_dossiersmodel->getRowFromItem(m_currentdossier);
        QString resume = m_dossiersmodel->data(m_dossiersmodel->index(row,1), Qt::EditRole).toString();
        if (resume.size() == 0)
        {
            ui->OKupPushButton->setEnabled(false);
            return;
        }
        a = false;
        for (int i=0 ; i<m_docsmodel->rowCount(); i++)
        {
            QStandardItem *itm = m_docsmodel->item(i,0);
            if (itm)
                if(itm->checkState() == Qt::Checked) {a = true;   break;}
        }
        ui->OKupPushButton->setEnabled(a);
    }
    else if (m_mode == Selection)
    {
        if (idx != QModelIndex())
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->itemFromIndex(idx));
            if (itm)
                if (itm->ischecked())
                    VerifDossiers();
            if (wdg_docsbuttonframe->searchline()->text() == "")
                m_listid.clear();
            for (int i=0; i<m_docsmodel->rowCount(); i++)
            {
                UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
                if (itm)
                {
                    UpStandardItem *titm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,1));
                    if (!titm)
                        continue;
                    Impression *doc = dynamic_cast<Impression*>(titm->item());
                    if (!doc)
                        continue;
                    if(itm->ischecked())
                           m_listid << QString::number(doc->id());
                    else
                        while (m_listid.contains(QString::number(doc->id())))
                                m_listid.removeOne(QString::number(doc->id()));
                }
            }
        }
        bool a = false;
        for (int i=0 ; i<m_docsmodel->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
            if (itm)
                if(itm->ischecked()) {a = true;   break;}
        }
        ui->OKupPushButton->setEnabled(a);
        ui->OKupPushButton->setShortcut(QKeySequence("Meta+Return"));
    }
}

void dlg_impressions::FiltreListe()
{
    EnableLines();
    bool affiche = currentuser()->affichedocspublics();
    for (int j=0; j<m_docsmodel->rowCount(); j++)
    {
        QString txt = m_docsmodel->item(j,1)->data(Qt::DisplayRole).toString().toUpper();
        QString txtachercher = wdg_docsbuttonframe->searchline()->text().toUpper();
        bool cache = !txt.contains(txtachercher);
        ui->DocsupTableView->setRowHidden(j, cache);
        if (!ui->DocsupTableView->isRowHidden(j))
        {
            Impression *doc = getDocumentFromIndex(m_docsmodel->index(j,0));
            if (doc)
                if (doc->iduser() != currentuser()->id())
                    ui->DocsupTableView->setRowHidden(m_docsmodel->getRowFromItem(doc),!affiche);
        }
    }
    for (int j=0; j<m_dossiersmodel->rowCount(); j++)
    {
        DossierImpression *dossier = getDossierFromIndex(m_dossiersmodel->index(j,0));
        if (dossier)
            if (dossier->iduser() != currentuser()->id())
                ui->DossiersupTableView->setRowHidden(m_dossiersmodel->getRowFromItem(dossier),!affiche);
    }
}

void dlg_impressions::MenuContextuelTexteDocument()
{
    QMenu menucontextuel;
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
    pAction_InsertChamp                 = menucontextuel.addAction   (Icons::icAjouter(),    tr("Insérer un champ"));
    QMenu *interro                      = menucontextuel.addMenu     (Icons::icAjouter(),    tr("Insérer une interrogation"));
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

    menucontextuel.addSeparator();
    if (ui->upTextEdit->textCursor().selectedText().size() > 0)   {
        pAction_ModifPolice     = menucontextuel.addAction(Icons::icFont(),           tr("Modifier la police"));
        pAction_Fontbold        = menucontextuel.addAction(Icons::icFontbold(),       tr("Gras"));
        pAction_Fontitalic      = menucontextuel.addAction(Icons::icFontitalic(),     tr("Italique"));
        pAction_Fontunderline   = menucontextuel.addAction(Icons::icFontunderline(),  tr("Souligné"));
        pAction_Fontnormal      = menucontextuel.addAction(Icons::icFontnormal(),     tr("Normal"));

        connect (pAction_ModifPolice,       &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Police");});
        connect (pAction_Fontbold,          &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Gras");});
        connect (pAction_Fontitalic,        &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Italique");});
        connect (pAction_Fontunderline,     &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Souligne");});
        connect (pAction_Fontnormal,        &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Normal");});
        menucontextuel.addSeparator();
    }
    pAction_Blockleft       = menucontextuel.addAction(Icons::icBlockLeft(),          tr("Aligné à gauche"));
    pAction_Blockright      = menucontextuel.addAction(Icons::icBlockRight(),         tr("Aligné à droite"));
    pAction_Blockcentr      = menucontextuel.addAction(Icons::icBlockCenter(),        tr("Centré"));
    pAction_Blockjust       = menucontextuel.addAction(Icons::icBlockJustify(),       tr("Justifié"));
    menucontextuel.addSeparator();
    if (ui->upTextEdit->textCursor().selectedText().size() > 0)   {
        pAction_Copier          = menucontextuel.addAction(Icons::icCopy(),   tr("Copier"));
        pAction_Cut             = menucontextuel.addAction(Icons::icCut(),    tr("Couper"));
        connect (pAction_Copier,            &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Copier");});
        connect (pAction_Cut,               &QAction::triggered,    this, [=] {ChoixMenuContextuelTexteDocument("Couper");});
    }
    if (qApp->clipboard()->mimeData()->hasText()
            || qApp->clipboard()->mimeData()->hasUrls()
            || qApp->clipboard()->mimeData()->hasImage()
            || qApp->clipboard()->mimeData()->hasHtml())
    {
        QAction *pAction_Coller = menucontextuel.addAction(Icons::icPaste(),  tr("Coller"));
        connect (pAction_Coller,        &QAction::triggered,    this,    [=] {ChoixMenuContextuelTexteDocument("Coller");});
    }

    connect (pAction_InsertChamp,                   &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Inserer");});
    connect (pAction_InsInterroDate,                &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Date");});
    connect (pAction_InsInterroCote,                &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(COTE);});
    connect (pAction_InsInterroHeure,               &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Heure");});
    connect (pAction_InsInterroMontant,             &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Montant");});
    connect (pAction_InsInterroMedecin,             &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Soignant");});
    connect (pAction_InsInterroProvenance,          &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(PROVENANCE);});
    connect (pAction_InsInterroSejour,              &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(TYPESEJOUR);});
    connect (pAction_InsInterroSite,                &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(SITE);});
    connect (pAction_InsInterroText,                &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Texte");});
    connect (pAction_Blockcentr,                    &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Centre");});
    connect (pAction_Blockright,                    &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Droite");});
    connect (pAction_Blockleft,                     &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Gauche");});
    connect (pAction_Blockjust,                     &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument("Justifie");});
    connect (pAction_InsInterroDateIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(DATEINTERVENTION);});
    connect (pAction_InsInterroHeureIntervention,   &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(HEUREINTERVENTION);});
    connect (pAction_InsInterroCoteIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(COTEINTERVENTION);});
    connect (pAction_InsInterroTypeIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(TYPEINTERVENTION);});
    connect (pAction_InsInterroSiteIntervention,    &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(SITEINTERVENTION);});
    connect (pAction_InsInterroAnesthIntervention,  &QAction::triggered,    this,   [=] {ChoixMenuContextuelTexteDocument(ANESTHINTERVENTION);});

    // ouvrir le menu
    menucontextuel.exec(cursor().pos());
}

void dlg_impressions::MenuContextuelDocuments()
{
    QModelIndex idx   = ui->DocsupTableView->indexAt(ui->DocsupTableView->viewport()->mapFromGlobal(cursor().pos()));
    m_currentdocument = getDocumentFromIndex(idx);
    if (!m_currentdocument)
        return;
    if (m_menucontextuel_doc == Q_NULLPTR)
        m_menucontextuel_doc= new QMenu(this);
    else
        m_menucontextuel_doc->clear();
    QAction *pAction_ModifDoc;
    QAction *pAction_SupprDoc;
    QAction *pAction_CreerDoc;
    QAction *pAction_PublicDoc = Q_NULLPTR;
    QAction *pAction_EditableDoc;
    QAction *pAction_AdminDoc;
    QAction *pAction_RecopierDoc;
    if (m_currentdocument->iduser() == currentuser()->id())
    {
        pAction_ModifDoc                = m_menucontextuel_doc->addAction(Icons::icEditer(), tr("Modifier ce document"));
        pAction_SupprDoc                = m_menucontextuel_doc->addAction(Icons::icPoubelle(), tr("Supprimer ce document"));
        connect (pAction_ModifDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuelDocument("Modifier");});
        connect (pAction_SupprDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuelDocument("Supprimer");});
    }
    pAction_CreerDoc                = m_menucontextuel_doc->addAction(Icons::icCreer(), tr("Créer un document"));
    if (m_currentdocument->iduser() == currentuser()->id())
    {
        if (m_currentdocument->ispublic())
        {
            pAction_PublicDoc       = m_menucontextuel_doc->addAction(tr("Privé"));
            pAction_PublicDoc       ->setEnabled(VerifDocumentPublic(m_currentdocument,false));
        }
        else
            pAction_PublicDoc       = m_menucontextuel_doc->addAction(tr("Public"));
        connect (pAction_PublicDoc,     &QAction::triggered,    this, [=] {ChoixMenuContextuelDocument("Public");});
        pAction_PublicDoc       ->setToolTip(tr("si cette option est cochée\ntous les utilisateurs\nauront accès à ce document"));
        if (!m_currentdocument->iseditable())
            pAction_EditableDoc         = m_menucontextuel_doc->addAction(tr("Editable"));
        else
            pAction_EditableDoc         = m_menucontextuel_doc->addAction(tr("Non modifiable"));
        pAction_EditableDoc ->setToolTip(tr("si cette option est cochée\nle document sera édité dans une fenêtre\navant son impression"));
        connect (pAction_EditableDoc,   &QAction::triggered,    this, [=] {ChoixMenuContextuelDocument("Editable");});
        if (Datas::I()->users->userconnected()->isMedecin() || Datas::I()->users->userconnected()->isOrthoptist())
        {
            /*!
        QAction *pAction_PrescripDoc;
        if (m_currentdocument->isprescription())
            pAction_PrescripDoc         = m_menucontextuel_doc->addAction(tr("Prescription"));
        else
            pAction_PrescripDoc         = m_menucontextuel_doc->addAction(Icons::icBlackCheck(), tr("Prescription"));
        connect (pAction_PrescripDoc,   &QAction::triggered,this, [=] {ChoixMenuContextuelDocument("Prescription");});
        pAction_PrescripDoc ->setToolTip(tr("si cette option est cochée\nce document sera considéré comme une prescription"));
        */
            if (!m_currentdocument->ismedical())
                pAction_AdminDoc            = m_menucontextuel_doc->addAction(tr("Document médical"));
            else
                pAction_AdminDoc            = m_menucontextuel_doc->addAction(tr("Document administratif"));
            connect (pAction_AdminDoc,  &QAction::triggered,this, [=] {ChoixMenuContextuelDocument("Administratif");});
        }
    }
    if (m_currentdocument->iduser() != currentuser()->id())
    {
        pAction_RecopierDoc           = m_menucontextuel_doc->addAction(Icons::icCopy(), tr("Recopier ce document"));
        pAction_RecopierDoc ->setToolTip(tr("Recopier ce document dans sa propre collection de documents"));
        connect (pAction_RecopierDoc,     &QAction::triggered,    this, [=] {ChoixMenuContextuelDocument("Recopier");});
    }
    connect (pAction_CreerDoc,      &QAction::triggered,    this, [=] {ChoixMenuContextuelDocument("Creer");});

    // ouvrir le menu
    m_menucontextuel_doc->exec(cursor().pos());
}

void dlg_impressions::MenuContextuelDossiers()
{
    QModelIndex idx   = ui->DossiersupTableView->indexAt(ui->DossiersupTableView->viewport()->mapFromGlobal(cursor().pos()));
    m_currentdossier = getDossierFromIndex(idx);
    if (!m_currentdossier)
        return;
    QMenu menucontextuel;
    QAction *pAction_ModifDossier;
    QAction *pAction_SupprDossier;
    QAction *pAction_CreerDossier;
    QAction *pAction_PublicDossier;

    pAction_CreerDossier            = menucontextuel.addAction(Icons::icCreer(), tr("Créer un dossier")) ;
    pAction_ModifDossier            = menucontextuel.addAction(Icons::icEditer(), tr("Modifier ce dossier")) ;
    pAction_SupprDossier            = menucontextuel.addAction(Icons::icPoubelle(), tr("Supprimer ce dossier")) ;
    menucontextuel.addSeparator();
    if (!m_currentdossier->ispublic())
        pAction_PublicDossier       = menucontextuel.addAction(Icons::icBlackCheck(), tr("Public")) ;
    else
        pAction_PublicDossier       = menucontextuel.addAction(tr("Privé")) ;

    connect (pAction_ModifDossier,  &QAction::triggered,    this, [=] {ChoixMenuContextuelDossier("Modifier");});
    connect (pAction_SupprDossier,  &QAction::triggered,    this, [=] {ChoixMenuContextuelDossier("Supprimer");});
    connect (pAction_CreerDossier,  &QAction::triggered,    this, [=] {ChoixMenuContextuelDossier("Creer");});
    connect (pAction_PublicDossier, &QAction::triggered,    this, [=] {ChoixMenuContextuelDossier("Public");});

    // ouvrir le menu
    menucontextuel.exec(cursor().pos());
}

void dlg_impressions::ChoixMenuContextuelTexteDocument(QString choix)
{
    if (choix       == "Coller")    ui->upTextEdit->paste();

    else if (choix  == "Copier")    ui->upTextEdit->copy();

    else if (choix  == "Couper")    ui->upTextEdit->cut();

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

        connect(ListChamps->OKButton,   &QPushButton::clicked, ListChamps,         [=] {ListChamps->accept();});
        ListChamps->setFixedWidth(tabChamps->width() + ListChamps->dlglayout()->contentsMargins().left()*2);
        connect(tabChamps,              &QTableWidget::doubleClicked, ListChamps,  [=] {ListChamps->accept();});
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

void dlg_impressions::ChoixMenuContextuelDocument(QString choix)
{
    if (choix  == "Modifier")
        ConfigMode(ModificationDOC);
    else if (choix  == "Supprimer")
        SupprimmeDocument(m_currentdocument);
    else if (choix  == "Public")
    {
        int row = m_docsmodel->getRowFromItem(m_currentdocument);
        if (row== -1)
            return;
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,2));
        if (itm)
        {
            ItemsList::update(m_currentdocument, CP_DOCPUBLIC_IMPRESSIONS,m_currentdocument->ispublic()? false:true);
            if(m_currentdocument->ispublic())
            {
                itm->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
                if (m_menucontextuel_doc != Q_NULLPTR)
                    if (m_menucontextuel_doc->activeAction() != Q_NULLPTR)
                        m_menucontextuel_doc->activeAction()->setEnabled(VerifDocumentPublic(m_currentdocument,false));
            }
            else
                itm->setData(QPixmap(),Qt::DecorationRole);
        }
        ui->DocPubliccheckBox->setChecked(m_currentdocument->ispublic());
    }
    else if (choix  == "Editable")
    {
        int row = m_docsmodel->getRowFromItem(m_currentdocument);
        if (row== -1)
            return;
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,3));
        if (itm)
        {
            if(itm->data(Qt::DecorationRole) == QPixmap())
                itm->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                itm->setData(QPixmap(),Qt::DecorationRole);
            if (m_mode == Selection)
                ItemsList::update(m_currentdocument, CP_EDITABLE_IMPRESSIONS,itm->data(Qt::DecorationRole) != QPixmap());
        }
        ui->DocEditcheckBox->toggle();
    }
    else if (choix  == "Administratif")
    {
        int row = m_docsmodel->getRowFromItem(m_currentdocument);
        if (row== -1)
            return;
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,4));
        if (itm)
        {
            if(itm->data(Qt::DecorationRole) == QPixmap())
                itm->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                itm->setData(QPixmap(),Qt::DecorationRole);
            ui->DocAdministratifcheckBox->setChecked(itm->data(Qt::DecorationRole) != QPixmap());
            if (m_mode == Selection)
                ItemsList::update(m_currentdocument, CP_MEDICAL_IMPRESSIONS, itm->data(Qt::DecorationRole) == QPixmap());
        }
        if (ui->PrescriptioncheckBox->isChecked() && ui->DocAdministratifcheckBox->isChecked())
        {
            ui->PrescriptioncheckBox->setChecked(false);
            if (m_mode == Selection)
                ItemsList::update(m_currentdocument, CP_PRESCRIPTION_IMPRESSIONS,false);
        }
    }
    else if (choix  == "Recopier")
    {
        RecopieDocument(m_currentdocument);
    }
    /*!
    else if (choix  == "Prescription")
    {
        int row = getRowFromDocument(m_currentdocument);
        if (row== -1)
            return;
        ui->PrescriptioncheckBox->setChecked(!ui->PrescriptioncheckBox->isChecked());
        if (ui->PrescriptioncheckBox->isChecked() && ui->DocAdministratifcheckBox->isChecked())
        {
            ui->DocAdministratifcheckBox->setChecked(false);
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,4));
            if (itm)
                itm->setData(QPixmap(),Qt::DecorationRole);
            if (m_mode == Selection)
                ItemsList::update(m_currentdocument, CP_MEDICAL_IMPRESSIONS,true);
        }
        if (m_mode == Selection)
            ItemsList::update(m_currentdocument, CP_PRESCRIPTION_IMPRESSIONS,!m_currentdocument->isprescription());
    }
    */
    else if (choix  == "Creer")
        ConfigMode(CreationDOSS);
}

void dlg_impressions::ChoixMenuContextuelDossier(QString choix)
{
    if (choix  == "Modifier")
        ConfigMode(ModificationDOSS);
    else if (choix  == "Supprimer")
        SupprimmeDossier(m_currentdossier);
    else if (choix  == "Public")
    {
        if (!m_currentdossier->ispublic() && hasDocumentPrive(m_currentdossier))
            return;
        int row = m_dossiersmodel->getRowFromItem(m_currentdossier);
        if (row== -1)
            return;
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(row,2));
        if (itm)
        {
            if(itm->data(Qt::DecorationRole) == QPixmap())
                itm->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
            else
                itm->setData(QPixmap(),Qt::DecorationRole);
            if (m_mode == Selection)
                ItemsList::update(m_currentdossier, CP_PUBLIC_DOSSIERIMPRESSIONS, itm->data(Qt::DecorationRole) != QPixmap());
        }
    }
    else if (choix  == "Creer")
    {
        ConfigMode(CreationDOSS);
    }
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton OK.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_impressions::OKpushButtonClicked()
{
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
        //qDebug() << "idsession = " << session->id() << " - iduser = " << session->iduser();
        if (session != Q_NULLPTR)
            m_userentete = Datas::I()->users->getById(session->iduser());
    }
    int ndocs = 0;
    switch (m_mode) {
    case CreationDOC:
    case ModificationDOC:
        if (EnregistreDocument(m_currentdocument))
        {
            ConfigMode(Selection);
            selectcurrentDocument(m_currentdocument);
        }
        break;
    case CreationDOSS:
    case ModificationDOSS:
        if (EnregistreDossier(m_currentdossier))
        {
            ConfigMode(Selection);
            selectcurrentDossier(m_currentdossier);
        }
        AfficheDocsPublicsAutresUtilisateurs(currentuser()->affichedocspublics());
        break;
    case Selection:                                                         // -> On imprime le
        for (int i =0 ; i < m_docsmodel->rowCount(); i++)
        {
            QStandardItem *itm = m_docsmodel->item(i,0);
            if (itm)
                if (itm->checkState() == Qt::Checked)
                    c++;
        }
        if (c == 0)
        {
            UpMessageBox::Watch(this,"Euuhh... " + Datas::I()->users->userconnected()->login() + ", " + tr("il doit y avoir une erreur..."), tr("Vous n'avez sélectionné aucun document."));
            break;
        }

        for (int i =0 ; i < m_docsmodel->rowCount(); i++)
        {
            // on établit d'abord la liste de questions à poser
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
            if (itm)
            {
                if (itm->checkState() == Qt::Checked)
                {
                    Impression *doc = getDocumentFromIndex(m_docsmodel->index(i,0));
                    QString text =  doc->texte();
                    QString quest = "([(][(][éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°?, -]*//(DATE|TEXTE|HEURE|MONTANT|SOIGNANT";
                    quest+= "|" + COTE + "|" + PROVENANCE + "|" + TYPESEJOUR + "|" + SITE;
                    if (m_currentintervention == Q_NULLPTR)
                        quest+= "|" + DATEINTERVENTION + "|" + HEUREINTERVENTION + "|" + COTEINTERVENTION + "|" + TYPEINTERVENTION + "|" + SITEINTERVENTION + "|" + ANESTHINTERVENTION;
                    quest += ")[)][)])";
                    QRegularExpression reg;
                    reg.setPattern(quest);
                    int count = 0;
                    int pos = 0;



                    while (reg.match(text, pos).hasMatch())
                    {
                        QString txt = reg.match(text, pos).captured(0);
                        ++count;
                        int fin = txt.indexOf("//");
                        int lengthquest = fin-2;
                        int lengthtype = txt.length() - fin;
                        bool a = true;
                        if (listQuestions.size()>0)
                            for (int j=0; j<listQuestions.size();j++)
                            {
                                if (listQuestions.at(j) == txt.mid(2,lengthquest))
                                {
                                    a = false;
                                    break;
                                }
                            }
                        if (a){
                            listQuestions << txt.mid(2,lengthquest);
                            listtypeQuestions << txt.mid(fin+2,lengthtype-4);
                        }
                    }


//                    while (reg.indexIn(text, pos) != -1)
//                    {
//                        pos = reg.indexIn(text, pos);
//                        ++count;
//                        pos += reg.matchedLength();
//                        int fin = reg.cap(1).indexOf("//");
//                        int lengthquest = fin-2;
//                        int lengthtype = reg.cap(1).length() - fin;
//                        bool a = true;
//                        if (listQuestions.size()>0)
//                            for (int j=0; j<listQuestions.size();j++)
//                            {
//                                if (listQuestions.at(j) == reg.cap(1).mid(2,lengthquest))
//                                {
//                                    a = false;
//                                    break;
//                                }
//                            }
//                        if (a){
//                            listQuestions << reg.cap(1).mid(2,lengthquest);
//                            listtypeQuestions << reg.cap(1).mid(fin+2,lengthtype-4);
//                        }
//                    }


                }
            }
        }
        // On a établi la liste de questions - on prépare la fiche qui va les poser
        if (listQuestions.size()>0 || m_userentete == Q_NULLPTR)
        {
            dlg_ask = new UpDialog(this);
            dlg_ask->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
            dlg_ask->setWindowModality(Qt::WindowModal);
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
                    Line->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse,this));
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
            connect(dlg_ask->OKButton,     &QPushButton::clicked, this,  [=] {VerifCoherencedlg_ask();});

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
        for (int i =0 ; i < m_docsmodel->rowCount(); i++)
        {
            /* On alimente 5 QStringList. Chaque iteration correspond à la description d'un document
             * TitreDocumentAImprimerList       -> le titre qui sera inséré dans la fiche docsexternes et dans la conclusion
             * prescriptionAImprimerList        -> précise si le document est une prescription - le formatage n'est pas le même
             * DupliAImprimerList               -> la nécessité ou non d'mprimer un dupli
             * TextDocumentsAImprimerList       -> le corps du document à imprimer
             * AdministratifAImprimerList       -> document administratif ou pas
            */
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
            if (itm)
            {
                if (itm->ischecked())
                {
                    // on effectue les corrections de chacun des documents
                    QMap<DATASAIMPRIMER,QString>  datasdocaimprimer;
                    Impression *doc = dynamic_cast<Impression*>(itm->item());
                    if (!itm)
                        continue;
                    QString textAimprimer = doc->texte();
                    if (ExpARemplacer.size() > 0)
                        for (int y=0; y<ExpARemplacer.size(); y++)
                            textAimprimer.replace(ExpARemplacer.at(y),Rempla.at(y));
                    MetAJour(textAimprimer,true);
                    for (int j=0; j<m_listtexts.size();j++)
                    {
                        QString txtdoc = m_listtexts.at(j);
                        Impression *impr = getDocumentFromIndex(m_docsmodel->index(i,0));

                        // on détermine le titre du document à inscrire en conclusion et le statut de prescription (si prescription => impression d'un dupli)

                        QString titre = impr->resume();
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
    proc->settings()->setValue(Imprimante_OrdoAvecDupli,(a? "YES" : "NO"));
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
            Utils::playAlarm();
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
                Utils::playAlarm();
            }
        }
    }
    if (a) dlg_ask->accept();
}

bool dlg_impressions::event(QEvent *event)
{
/*    if (event->type() == QEvent::ToolTip)
    {
        QWidget* rec            = ui->DocupTableView;
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
        itemM = ui->DocupTableView->itemAt(rec->mapFrom(this,pos));
        if (itemM == 0) return false;
        if (itemM->row() > -1) {
            QTextEdit *text = new QTextEdit;
            text->setText(MetAJour(ui->DocupTableView->item(itemM->row(),2)->text()));
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
    proc->settings()->setValue(Position_Fiche "Documents",saveGeometry());
    event->accept();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_impressions::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::FocusIn)
    {
        if (dynamic_cast<UpLineEdit*>(obj))
        {
            UpLineEdit *linetext = static_cast<UpLineEdit*>(obj);
            linetext->selectAll();
        }
    }
    if(event->type() == QEvent::FocusOut)
    {
        if (dynamic_cast<UpLineEdit*>(obj))
        {
            UpLineEdit *linetext = static_cast<UpLineEdit*>(obj);
            const QDoubleValidator *val = dynamic_cast<const QDoubleValidator*>(linetext->validator());
            if (val)
                linetext->setText(QLocale().toString(QLocale().toDouble(linetext->text()),'f',2));
        }
    }
    if(event->type() == QEvent::MouseMove)
    {
        if (obj == ui->textFrame)
            if (m_mode == Selection)
            {
                QRect rect = QRect(ui->textFrame->pos(),ui->textFrame->size());
                QPoint pos = mapFromParent(cursor().pos());
                if (rect.contains(pos) && ui->upTextEdit->toPlainText() != "")
                    EffaceWidget(ui->textFrame, false);
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
        if (m_currentdocument)
        {
            int row = m_docsmodel->getRowFromItem(m_currentdocument);
            if (row > -1 && row < m_docsmodel->rowCount())
            {
                UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,0));
            if (itm)
                itm->setCheckState(itm->checkState()==Qt::Checked? Qt::Unchecked : Qt:: Checked);
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

    connect(dlg_askdialog->OKButton,   &QPushButton::clicked, dlg_askdialog,  [=] {dlg_askdialog->accept();});

    Line->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse,this));
    Line->setMaxLength(60);
    return dlg_askdialog->exec();
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de document
// ----------------------------------------------------------------------------------
bool dlg_impressions::ChercheDoublon(QString str, int row)
{
    Impression *doc = Q_NULLPTR;
    DossierImpression *dossier = Q_NULLPTR;

    if (m_mode == CreationDOC || m_mode == ModificationDOC)  {
        doc = getDocumentFromIndex(m_docsmodel->index(row,0));
        if (doc)
        {
            for (auto itimpr = Datas::I()->impressions->impressions()->begin(); itimpr != Datas::I()->impressions->impressions()->end(); ++itimpr)
            {
                if (itimpr.value()->resume().toUpper() == str.toUpper() && itimpr.value()->iduser()  == currentuser()->id() && itimpr.value()->id() != doc->id())
                {
                    UpMessageBox::Watch(this,tr("Vous avez déjà créé un document portant ce nom"));
                    return true;
                }
            }
        }
    }
    else if (m_mode == CreationDOSS || m_mode == ModificationDOSS)  {
        dossier = getDossierFromIndex(m_dossiersmodel->index(row,0));
        if (dossier)
        {
            for (auto itdoss = Datas::I()->metadocuments->dossiersimpressions()->begin(); itdoss != Datas::I()->metadocuments->dossiersimpressions()->end(); ++itdoss)
            {
                if (itdoss.value()->resume().toUpper() == str.toUpper() && itdoss.value()->iduser()  == currentuser()->id() && itdoss.value()->id() != dossier->id())
                {
                    UpMessageBox::Watch(this,tr("Vous avez déjà créé un dossier portant ce nom"));
                    return true;
                }
            }
        }
    }
    return false;
}

// ----------------------------------------------------------------------------------
// On a clique sur une ligne de dossiers . on coche-décoche les docs correspondants
// ----------------------------------------------------------------------------------
void dlg_impressions::CocheLesDocs(int iddoss, bool A)
{
    DossierImpression *dossier = Datas::I()->metadocuments->getById(iddoss);
    if (!dossier)
        return;
    if (!dossier->haslistdocsloaded())
        Datas::I()->metadocuments->loadlistedocs(dossier);
    QList<int> listiddocs = dossier->listiddocs();

    for (int k=0; k<m_docsmodel->rowCount(); k++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(k,0));
        if (itm)
        {
            Impression *doc = dynamic_cast<Impression*>(itm->item());
            if (doc)
            {
                if (listiddocs.contains(doc->id()))
                {
                    if (A)
                    {
                        itm->setCheckState(Qt::Checked);
                        m_docsmodel->item(k,5)->setText("0" + doc->resume());
                    }
                    else                 // on vérifie qu'on peut décocher un doc et qu'il n'est pas nécessité par un autre dossier coché
                    {
                        bool a = false;
                        for (int j=0; j<m_dossiersmodel->rowCount(); j++)
                        {
                            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(j,0));
                            if (itm)
                            {
                                DossierImpression * dossieraverifier = getDossierFromIndex(itm->index());
                                if (dossieraverifier)
                                    if (dossieraverifier->id() != iddoss)
                                    {
                                        if (itm->checkState()==Qt::Checked)
                                        {
                                            if (!dossieraverifier->haslistdocsloaded())
                                                Datas::I()->metadocuments->loadlistedocs(dossieraverifier);
                                            if (dossieraverifier->listiddocs().contains(doc->id()))
                                            {
                                                a = true;
                                                break;
                                            }   }   }   }   }
                                            itm->setCheckState(a? Qt::Checked : Qt::Unchecked);
                        m_docsmodel->item(k,5)->setText((a?"0":"1") + doc->resume());
                    }
                }
            }

        }   }
    // tri de la table DocupTableView
    m_docsmodel->sort(5);
    ui->DocsupTableView->scrollToTop();

    // enable okpushbutton
    for (int l=0; l<m_docsmodel->rowCount(); l++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(l,0));
        if (itm)
        {
            if (itm->ischecked())
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
void dlg_impressions::ConfigMode(Mode mode)
{
    m_mode = mode;
    wdg_docsbuttonframe->searchline()   ->setEnabled(m_mode == Selection);
    ui->dateImpressiondateEdit          ->setEnabled(m_mode == Selection);
    ui->ALDcheckBox                     ->setVisible(m_mode == Selection);
    wdg_dossiersbuttonframe             ->setEnabled(m_mode == Selection);
    ui->DossiersupTableView             ->setEnabled(m_mode == Selection);
    ui->OKupPushButton                  ->setEnabled(false);
    ui->textFrame                       ->setVisible(m_mode != CreationDOSS && m_mode!= ModificationDOSS && m_mode != Selection);

    if (m_mode != Selection) {
        t_timerefface->disconnect();
        ui->textFrame->setGraphicsEffect(new QGraphicsOpacityEffect());
        ui->DossiersupTableView->setContextMenuPolicy(Qt::NoContextMenu);
    }
    else
    {
        m_opacityeffect = new QGraphicsOpacityEffect();
        m_opacityeffect->setOpacity(0.1);
        ui->textFrame->setGraphicsEffect(m_opacityeffect);
        ui->DossiersupTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    if (mode == Selection)
    {
        EnableLines();
        wdg_docsbuttonframe             ->setEnabled(true);
        ui->DocPubliccheckBox           ->setChecked(false);
        ui->DocPubliccheckBox           ->setEnabled(false);
        ui->DocPubliccheckBox           ->setImmediateToolTip("");
        ui->DocsupTableView             ->setEnabled(true);
        ui->DocsupTableView             ->setStyleSheet("");
        wdg_dossiersbuttonframe         ->setEnabled(true);
        ui->DossiersupTableView         ->setEnabled(true);
        ui->DocEditcheckBox             ->setChecked(false);
        ui->DocEditcheckBox             ->setEnabled(false);
        ui->DocEditcheckBox             ->setImmediateToolTip("");
        ui->DocAdministratifcheckBox    ->setChecked(false);
        ui->DocAdministratifcheckBox    ->setEnabled(false);
        ui->DocAdministratifcheckBox    ->setImmediateToolTip("");
        ui->PrescriptioncheckBox        ->setImmediateToolTip("");
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
        for (int i =0 ; i < m_docsmodel->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
            if (itm)
                if(itm->ischecked())
                    ++nbCheck;
        }
        ui->OKupPushButton->setEnabled(nbCheck>0);
        wdg_docsbuttonframe->searchline()->setFocus();
    }

    else if (mode == ModificationDOC)
    {
        if (!m_currentdocument)
            return;
        DisableLines();
        ui->upTextEdit->setText(m_currentdocument->texte());

        ui->DocsupTableView->setEnabled(true);
        ui->DocsupTableView->setStyleSheet("");
        wdg_docsbuttonframe->setEnabled(false);
        ui->DossiersupTableView->setEnabled(true);
        wdg_dossiersbuttonframe->setEnabled(false);
        ui->textFrame->setEnabled(true);
        ui->Expliclabel->setText(tr("DOCUMENTS - MODIFICATION"));
        if (!m_currentdocument->ispublic())
            ui->DocPubliccheckBox->setEnabled(VerifDocumentPublic(m_currentdocument,false));
        ui->DocPubliccheckBox->setImmediateToolTip(tr("Cocher cette case pour que le document soit visible par tous les utilisateurs"));

        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setImmediateToolTip(tr("Cocher cette case pour que le document soit affiché pour pouvoir être modifié avant son impression"));

        ui->DocAdministratifcheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setImmediateToolTip(tr("Cocher cette case si ce document est administratif"));

        ui->PrescriptioncheckBox->setEnabled(true);
        ui->PrescriptioncheckBox->setImmediateToolTip(tr("Cocher cette case pour que le documment soit considéré comme une prescription\n"
                                                      "Il ne sera accessible qu'au personnel soignant\n"
                                                      "Un duplicata sera imprimé\n"
                                                      "le nom du patient apparaîtra dans l'en-tête"));

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
        int row = ui->DocsupTableView->currentIndex().row();
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,0));
        if (itm)
        {
            m_docsmodel->item(row,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            ui->DocsupTableView->setFocus();
            ui->DocsupTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
            ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(row,1));
         }
        //qDebug() << m_currentdocument->resume();
    }

    else if (mode == ModificationDOSS)
    {
        if (!m_currentdossier)
            return;
        DisableLines();
        int row = ui->DossiersupTableView->currentIndex().row();
        UpStandardItem *itmdef = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(row,2));
        if (!itmdef)
            return;
        bool publicdossier = (itmdef->data(Qt::DecorationRole) != QPixmap());
        for (int i=0; i<m_dossiersmodel->rowCount(); i++)
        {
            QStandardItem *itm = m_dossiersmodel->item(i);
            if (itm)
                itm->setCheckState(Qt::Unchecked);
        }
        for (int i=0; i<m_docsmodel->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
            if (itm)
            {
                if (!m_currentdossier->haslistdocsloaded())
                    Datas::I()->metadocuments->loadlistedocs(m_currentdossier);
                Impression *doc = getDocumentFromIndex(m_docsmodel->index(i,0));
                if (doc)
                {
                    bool a = m_currentdossier->listiddocs().contains(doc->id());
                    itm->setCheckState(a? Qt::Checked : Qt::Unchecked);
                    itm->setFlags(publicdossier && ! doc->ispublic()? Qt::ItemIsEnabled : Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                    m_docsmodel->item(i,5)->setText((a?"0":"1") + doc->resume());
                }
            }
        }
        m_docsmodel->sort(5);

        ui->DocsupTableView->setEnabled(true);
        ui->DocsupTableView->setStyleSheet("UpTableWidget {border: 2px solid rgb(251, 51, 61);}");
        ui->DossiersupTableView->setEnabled(true);
        ui->Expliclabel->setText(tr("DOSSIERS - MODIFICATION"));

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
        ui->OKupPushButton->setText(tr("Enregistrer"));
        ui->OKupPushButton->setEnabled(false);
        wdg_docsbuttonframe->searchline()->clear();
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(row,0));
        if (itm)
        {
            m_dossiersmodel->item(row,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            ui->DossiersupTableView->setFocus();
            ui->DossiersupTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
            ui->DossiersupTableView->openPersistentEditor(m_dossiersmodel->index(row,1));
        }
        AfficheDocsPublicsAutresUtilisateurs(false);
    }
    else if (mode == CreationDOC)
    {
        wdg_docsbuttonframe->searchline()->clear();
        FiltreListe();
        DisableLines();
        for (int i=0; i<m_dossiersmodel->rowCount(); i++)
        {
            QStandardItem *itm = m_dossiersmodel->item(i);
            if (itm)
                itm->setCheckState(Qt::Unchecked);
        }
        for (int i=0; i<m_docsmodel->rowCount(); i++)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
            if (itm)
            {
                itm->setCheckState(Qt::Unchecked);
                Impression *doc = getDocumentFromIndex(m_docsmodel->index(i,0));
                if (doc)
                    m_docsmodel->item(i,5)->setText("1" + doc->resume());
            }
        }
        ui->DossiersupTableView->setEnabled(false);
        wdg_dossiersbuttonframe->setEnabled(false);

        ui->Expliclabel->setText(tr("DOCUMENTS - CREATION - Remplissez les champs définissant le document que vous voulez créer"));

        ui->DocEditcheckBox->setEnabled(true);
        ui->DocEditcheckBox->setChecked(false);
        ui->DocEditcheckBox->setImmediateToolTip(tr("Cocher cette case pour que le document soit affiché pour pouvoir être modifié avant son impression"));

        ui->DocPubliccheckBox->setChecked(true);
        ui->DocPubliccheckBox->setEnabled(true);
        ui->DocPubliccheckBox->setImmediateToolTip(tr("Cocher cette case pour que le document soit visible par tous les utilisateurs"));

        ui->PrescriptioncheckBox->setChecked(Datas::I()->users->userconnected()->isMedecin() || Datas::I()->users->userconnected()->isOrthoptist());
        ui->PrescriptioncheckBox->setEnabled(true);
        ui->PrescriptioncheckBox->setImmediateToolTip(tr("Cocher cette case pour que le documment soit considéré comme une prescription\n"
                                                      "Il ne sera accessible qu'au personnel soignant\n"
                                                      "Un duplicata sera imprimé\n"
                                                      "le nom du patient apparaîtra dans l'en-tête"));

        ui->DocAdministratifcheckBox->setEnabled(true);
        ui->DocAdministratifcheckBox->setChecked(!Datas::I()->users->userconnected()->isMedecin() && !Datas::I()->users->userconnected()->isOrthoptist());
        ui->DocAdministratifcheckBox->setImmediateToolTip(tr("Cocher cette case si le document est un document administratif, accessible à tous les utilisateurs"));
        wdg_docsbuttonframe->wdg_moinsBouton->setEnabled(false);
        ui->upTextEdit->clear();
        ui->upTextEdit->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setEnabled(true);
        ui->AnnulupPushButton->setImmediateToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setText(tr("Enregistrer\nle document"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));
        m_docsmodel->insertRow(0);
        m_currentdocument = new Impression;
        m_currentdocument->setresume(tr("Nouveau document"));
        m_currentdocument->setiduser(currentuser()->id());
        m_currentdocument->setmedical(Datas::I()->users->userconnected()->isMedecin() || Datas::I()->users->userconnected()->isOrthoptist());
        m_currentdocument->setprescription(Datas::I()->users->userconnected()->isMedecin() || Datas::I()->users->userconnected()->isOrthoptist());
        m_currentdocument->setpublic(true);
        m_currentdocument->seteditable(false);
        SetDocumentToRow(m_currentdocument, 0);
        m_docsmodel->item(0,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_docsmodel->item(0,0)->setFlags(Qt::NoItemFlags);

        ui->DocsupTableView->setFocus();
        ui->DocsupTableView->scrollTo(m_docsmodel->index(0,1), QAbstractItemView::EnsureVisible);
        ui->DocsupTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(0,1));
    }

    else if (mode == CreationDOSS)
    {
        wdg_docsbuttonframe->searchline()->clear();
        FiltreListe();
        DisableLines();
        bool publicdossier = (m_currentdossier? m_currentdossier->ispublic() : false);
        for (int i=0; i<m_docsmodel->rowCount(); i++) // on ne peut pas rendre public un dossier qui contient des documents non publics
        {
            QStandardItem *itm = m_docsmodel->item(i,0);
            if (itm)
            {
                itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                itm->setCheckState(Qt::Unchecked);
                Impression *doc = getDocumentFromIndex(m_docsmodel->index(i,0));
                if (doc)
                {
                    itm->setFlags(publicdossier && !doc->ispublic()? Qt::ItemIsEnabled : Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                    m_docsmodel->item(i,5)->setText("1" + doc->resume());
                }
            }
        }
        m_docsmodel->sort(5);
        for (int i=0; i<m_dossiersmodel->rowCount(); i++)
        {
            QStandardItem *itm = m_dossiersmodel->item(i);
            if (itm)
                itm->setCheckState(Qt::Unchecked);
        }

        wdg_docsbuttonframe->setEnabled(false);
        ui->DocsupTableView->setEnabled(true);
        ui->DocsupTableView->setStyleSheet("UpTableWidget {border: 2px solid rgb(251, 51, 61);}");
        ui->DossiersupTableView->setEnabled(true);
        wdg_dossiersbuttonframe->setEnabled(false);
        ui->Expliclabel->setText(tr("DOSSIER - CREATION - Cochez les cases correspondants au dossier que vous voulez créer"));

        ui->AnnulupPushButton->setIcon(Icons::icBack());
        ui->AnnulupPushButton->setEnabled(true);
        ui->AnnulupPushButton->setToolTip(tr("Revenir au mode\nsélection de document"));
        ui->OKupPushButton->setText(tr("Enregistrer\nle dossier"));
        ui->OKupPushButton->setIcon(Icons::icValide());
        ui->OKupPushButton->setIconSize(QSize(25,25));

        m_dossiersmodel->insertRow(0);
        m_currentdossier = new DossierImpression;
        m_currentdossier->setresume(tr("Nouveau Dossier"));
        m_currentdossier->setiduser(currentuser()->id());
        SetDossierToRow(m_currentdossier, 0);
        m_dossiersmodel->item(0,1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_dossiersmodel->item(0,0)->setFlags(Qt::NoItemFlags);
        ui->DossiersupTableView->setFocus();
        ui->DossiersupTableView->scrollTo(m_dossiersmodel->index(0,1), QAbstractItemView::EnsureVisible);
        ui->DossiersupTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        ui->DossiersupTableView->openPersistentEditor(m_dossiersmodel->index(0,1));
        AfficheDocsPublicsAutresUtilisateurs(false);
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
    wdg_docsbuttonframe->searchline()->clear();
    for (int i=0; i<m_docsmodel->rowCount(); i++)
        ui->DocsupTableView->setRowHidden(i,false);
    for (int i=0; i<m_dossiersmodel->rowCount(); i++)
        ui->DossiersupTableView->setRowHidden(i,false);

    wdg_dossiersbuttonframe->setEnabled(false);
    wdg_docsbuttonframe->setEnabled(false);

    for (int i=0; i<m_docsmodel->rowCount(); i++)
    {
        QStandardItem *itm = m_docsmodel->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable);
        QStandardItem *itm1 = m_docsmodel->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::NoItemFlags);
    }
    for (int i=0; i<m_dossiersmodel->rowCount(); i++)
    {
        QStandardItem *itm = m_dossiersmodel->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable);
        QStandardItem *itm1 = m_dossiersmodel->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::NoItemFlags);
    }
}

// -------------------------------------------------------------------------------------------
// SetEnabled = true, connect toutes les lignes des UpTableWidget - SetEnabled = true checkBox
// -------------------------------------------------------------------------------------------
void dlg_impressions::EnableLines()
{
    wdg_dossiersbuttonframe->setEnabled(true);
    wdg_docsbuttonframe->setEnabled(true);
    for (int i=0; i<m_docsmodel->rowCount(); i++)
    {
        QStandardItem *itm = m_docsmodel->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        QStandardItem *itm1 = m_docsmodel->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
    for (int i=0; i<m_dossiersmodel->rowCount(); i++)
    {
        QStandardItem *itm = m_dossiersmodel->item(i,0);
        if (itm)
            itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        QStandardItem *itm1 = m_dossiersmodel->item(i,1);
        if (itm1)
            itm1->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
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
    connect(t_timerefface, &QTimer::timeout, this, [=]
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

Impression* dlg_impressions::getDocumentFromIndex(QModelIndex idx)
{
    int row = idx.row();
    if (row < 0)
        return Q_NULLPTR;
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row,0));
    if (itm != Q_NULLPTR)
        return dynamic_cast<Impression*>(itm->item());
    else
        return Q_NULLPTR;
}

DossierImpression* dlg_impressions::getDossierFromIndex(QModelIndex idx)
{
    int row = idx.row();
    if (row < 0)
        return Q_NULLPTR;
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(row,0));
    if (itm != Q_NULLPTR)
        return dynamic_cast<DossierImpression*>(itm->item());
    else
        return Q_NULLPTR;
}

User* dlg_impressions::userentete() const
{
    return m_userentete;
}

// ----------------------------------------------------------------------------------
// Verifie si un dossier peut être rendu public
// ----------------------------------------------------------------------------------
bool dlg_impressions::hasDocumentPrive(DossierImpression *dossier)
{
    if (!m_currentdossier)
        return false;
    if (!m_currentdossier->haslistdocsloaded())
        Datas::I()->metadocuments->loadlistedocs(m_currentdossier);
    for (int i=0; i<dossier->listiddocs().size(); i++)
    {
        Impression *impr = Datas::I()->impressions->getById(dossier->listiddocs().at(i));
        if (impr)
        {
            if (!impr->ispublic())
            {
                UpMessageBox::Watch(this,tr("Vous ne pouvez pas rendre public ce dossier.\nIl incorpore le document\n- ") +
                                    impr->resume() + tr(" -\nqui est un document privé!"));
                return true;
            }
        }
    }
    return false;
}

// ----------------------------------------------------------------------------------
// Creation du Document dans la base.
// ----------------------------------------------------------------------------------
bool dlg_impressions::EnregistreDocument(Impression *doc)
{
    // controle validate des champs
    if (!doc)
        return false;
    int row = m_docsmodel->getRowFromItem(doc);
    ui->DocsupTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(row));
    if (!itm)
        return false;
    ui->DocsupTableView->closePersistentEditor(m_docsmodel->index(row,1));
    qApp->focusWidget()->clearFocus();
    QString resume = Utils::trimcapitilize(m_textdocdelegate, true, false, false).left(50);
    resume = Utils::capitilize(resume, true);
    QString titre = tr(m_mode == CreationDOC? "Creation de document" : "Modification de document");
    if (resume.length() < 1)
    {
        UpMessageBox::Watch(Q_NULLPTR, titre, tr("Veuillez renseigner le champ Résumé, SVP !"));
        ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(row,1));
        return false;
    }
    if (resume == tr("Nouveau document"))
    {
        UpMessageBox::Watch(Q_NULLPTR, titre, tr("Votre document ne peut pas s'appeler \"Nouveau document\""));
        ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(row,1));
        return false;
    }
    if (ui->upTextEdit->document()->toPlainText().length() < 1)
    {
        UpMessageBox::Watch(Q_NULLPTR, titre, tr("Veuillez renseigner le champ Document, SVP !"));
        ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(row,1));        return false;
    }
    // Creation du Document dans la table
    if (ChercheDoublon(resume, row))
    {
        ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(row,1));
        return false;
    }

    m_docslistbinds[CP_TEXTE_IMPRESSIONS]           = ui->upTextEdit->document()->toHtml();
    m_docslistbinds[CP_RESUME_IMPRESSIONS]          = resume;
    m_docslistbinds[CP_IDUSER_IMPRESSIONS]          = currentuser()->id();
    m_docslistbinds[CP_DOCPUBLIC_IMPRESSIONS]       = (ui->DocPubliccheckBox->isChecked()?          "1" : QVariant());
    m_docslistbinds[CP_PRESCRIPTION_IMPRESSIONS]    = (ui->PrescriptioncheckBox->isChecked()?       "1" : QVariant());
    m_docslistbinds[CP_EDITABLE_IMPRESSIONS]        = (ui->DocEditcheckBox->isChecked()?            "1" : QVariant());
    m_docslistbinds[CP_MEDICAL_IMPRESSIONS]         = (ui->DocAdministratifcheckBox->isChecked()?    QVariant() : "1");
    if (m_mode == CreationDOC)
    {
        delete m_currentdocument;
        m_currentdocument = Datas::I()->impressions->CreationImpression(m_docslistbinds);
    }
    else if (m_mode == ModificationDOC)
    {
        DataBase::I()->UpdateTable(TBL_IMPRESSIONS, m_docslistbinds, " where " CP_ID_IMPRESSIONS " = " + QString::number(m_currentdocument->id()),tr("Impossible de modifier le site"));
        m_currentdocument = Datas::I()->impressions->getById(m_currentdocument->id(), true);
        row = m_docsmodel->getRowFromItem(m_currentdocument);
    }
    if(m_currentdocument)
    {
        SetDocumentToRow(m_currentdocument,row);
        m_docsmodel->sort(5);
    }
    return true;
}

// ----------------------------------------------------------------------------------
// Creation du Document dans la base.
// ----------------------------------------------------------------------------------
bool dlg_impressions::RecopieDocument(Impression *doc)
{
    int row = m_docsmodel->getRowFromItem(doc);
    // Creation du Document dans la table
    if (ChercheDoublon(m_currentdocument->resume(), row))
    {
        ui->DocsupTableView->openPersistentEditor(m_docsmodel->index(row,1));
        return false;
    }

    m_docslistbinds[CP_TEXTE_IMPRESSIONS]           = m_currentdocument->texte();
    m_docslistbinds[CP_RESUME_IMPRESSIONS]          = m_currentdocument->resume();
    m_docslistbinds[CP_IDUSER_IMPRESSIONS]          = currentuser()->id();
    m_docslistbinds[CP_DOCPUBLIC_IMPRESSIONS]       = QVariant();
    m_docslistbinds[CP_PRESCRIPTION_IMPRESSIONS]    = (m_currentdocument->isprescription()? "1" : QVariant());
    m_docslistbinds[CP_EDITABLE_IMPRESSIONS]        = (m_currentdocument->iseditable()?     "1" : QVariant());
    m_docslistbinds[CP_MEDICAL_IMPRESSIONS]         = (m_currentdocument->ismedical()?      "1" : QVariant());
    m_currentdocument = Q_NULLPTR;
    m_currentdocument = Datas::I()->impressions->CreationImpression(m_docslistbinds);
    Remplir_TableView();
    FiltreListe();
    m_docsmodel->sort(5);
    return true;
}

// ----------------------------------------------------------------------------------
// Creation du Dossier dans la base.
// ----------------------------------------------------------------------------------
bool dlg_impressions::EnregistreDossier(DossierImpression  *dossier)
{
    // controle validité des champs
    if (!dossier)
        return false;
    int row = m_dossiersmodel->getRowFromItem(dossier);
    ui->DossiersupTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(row));
    if (!itm)
    {
        return false;
    }
    UpStandardItem *itmdef = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(row,2));
    if (!itmdef)
    {
        return false;
    }
    ui->DossiersupTableView->closePersistentEditor(m_dossiersmodel->index(row,1));
    qApp->focusWidget()->clearFocus();
    bool publicdossier = (itmdef->data(Qt::DecorationRole) != QPixmap());
    QString resume = Utils::trimcapitilize(m_textdossierdelegate).left(100);
    if (resume == tr("Nouveau dossier"))
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Creation de dossier"), tr("Votre dossier ne peut pas s'appeler \"Nouveau dossier\""));
        ui->DossiersupTableView->openPersistentEditor(m_dossiersmodel->index(row,1));
        return false;
    }
    if (ChercheDoublon(resume, row))
    {
        ui->DossiersupTableView->openPersistentEditor(m_dossiersmodel->index(row,1));
        return false;
    }
    QList<int> listid;
    for (int l=0; l<m_docsmodel->rowCount(); l++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(l,0));
        if (itm)
            if (itm->ischecked())
            {
                Impression *doc= dynamic_cast<Impression*>(itm->item());
                if (doc)
                {
                    if (publicdossier && !doc->ispublic())
                    {
                        UpMessageBox::Watch(Q_NULLPTR,tr("Creation de dossier"), tr("Vous ne pouvez pas enregistre le document") + " " + doc->resume()
                                            + " " + tr("dans ce dossier car ce dossier est public et pas le document"));
                        ui->DossiersupTableView->openPersistentEditor(m_dossiersmodel->index(row,1));
                        return false;
                    }
                    listid << doc->id();
                }
            }
    }
    if (listid.size() == 0)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Enregistrement de Dossier"), tr("Veuillez cocher au moins un document, SVP !"));
        ui->DossiersupTableView->openPersistentEditor(m_dossiersmodel->index(row,1));
        return false;
    }
    if (m_mode == ModificationDOSS)
        Datas::I()->metadocuments->setlistedocs(m_currentdossier,listid);

    // Enregistrement du Dossier dans la table

    m_dossierlistbinds[CP_RESUME_DOSSIERIMPRESSIONS]   = resume.left(100);
    m_dossierlistbinds[CP_IDUSER_DOSSIERIMPRESSIONS]   = currentuser()->id();
    m_dossierlistbinds[CP_PUBLIC_DOSSIERIMPRESSIONS]   = (publicdossier? "1" : QVariant());
    if (m_mode == ModificationDOSS)
    {
        DataBase::I()->UpdateTable(TBL_DOSSIERSIMPRESSIONS, m_dossierlistbinds, " where " CP_ID_DOSSIERIMPRESSIONS " = " + QString::number(m_currentdossier->id()),tr("Impossible d'enregistrer le dossier"));
        m_currentdossier = Datas::I()->metadocuments->getById(m_currentdossier->id(), true);
    }
    else if (m_mode == CreationDOSS)
    {
        delete m_currentdossier;
        m_currentdossier = Datas::I()->metadocuments->CreationDossierImpression(m_dossierlistbinds);
        SetDossierToRow(m_currentdossier, row);
    }
    if (m_currentdossier)
        if (listid.size()>0)
        {
            QString req = "insert into " TBL_JOINTURESIMPRESSIONS " (" CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS ", " CP_IDDOCUMENT_JOINTURESIMPRESSIONS ") VALUES ";
            for (int k=0; k<listid.size(); k++)
            {
                req += "(" + QString::number(m_currentdossier->id()) + ", " + QString::number(listid.at(k)) + ")";
                if (k<listid.size()-1)    req += ",";
            }
            db->StandardSQL(req);
            for (int i=0; i<m_docsmodel->rowCount(); i++)
            {
                UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i,0));
                if (itm)
                {
                    Impression *doc = getDocumentFromIndex(m_docsmodel->index(i,0));
                    bool a = itm->checkState() == Qt::Checked;
                    m_docsmodel->item(i,5)->setText((a?"0":"1") + doc->resume());
                }
            }
            m_docsmodel->sort(5);
            m_dossiersmodel->sort(1);
            row = m_dossiersmodel->getRowFromItem(m_currentdossier);
            if (row > -1 && row < m_dossiersmodel->rowCount())
                m_dossiersmodel->item(row)->setCheckState(Qt::Checked);
        }
    return true;
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
        texte.replace(txt, (sit? sit->nom() : "null"));

        txt = "((" + TITRETYPEINTERVENTION + "//" + TYPEINTERVENTION + "))";
        texte.replace(txt, (typ? typ->typeintervention() : "null"));
    }
    int pos = 0;
    QRegularExpression reg;
    reg.setPattern("([{][{].*CORRESPONDANT.*[}][}])");
    if (reg.match(texte, pos).hasMatch())
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
    int largfinal               = fm.horizontalAdvance(lbltxt);
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
        if (fm.horizontalAdvance(pitem->text()) > largeurcolonne)
            largeurcolonne = fm.horizontalAdvance(pitem->text());
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

    connect(dlg_askcorrespondant->OKButton,   &QPushButton::clicked, dlg_askcorrespondant, [=] {ListidCor();});

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
void dlg_impressions::Remplir_TableView()
{
    //Remplissage Table Documents
    if (!Datas::I()->impressions->isfull())
        Datas::I()->impressions->initListe();
    ui->DocsupTableView->disconnect();
    ui->DocsupTableView->selectionModel()->disconnect();
    ui->DocsupTableView->horizontalHeader()->disconnect();
    UpLineDelegate *linedoc = new UpLineDelegate();
    connect(linedoc,   &UpLineDelegate::textEdited, this, [=] {ui->OKupPushButton->setEnabled(true);});
    connect(linedoc,   &UpLineDelegate::commitData, this, [=](QWidget *editor) {
                                                                            UpLineEdit *line = qobject_cast<UpLineEdit*>(editor);
                                                                            m_textdocdelegate = line->text();
                                                                         });
    ui->DocsupTableView->setItemDelegateForColumn(1,linedoc);
    if (m_docsmodel == Q_NULLPTR)
        delete m_docsmodel;
    m_docsmodel = new UpStandardItemModel(this);

    QStandardItem *ditem0   = new QStandardItem(Icons::icunBlackCheck(),"");
    ditem0->setEditable(false);
    ditem0->setTextAlignment(Qt::AlignCenter);
    m_docsmodel->setHorizontalHeaderItem(0,ditem0);
    QStandardItem *ditem1   = new QStandardItem(tr("TITRES DES DOCUMENTS"));
    ditem1->setEditable(false);
    ditem1->setTextAlignment(Qt::AlignLeft);
    m_docsmodel->setHorizontalHeaderItem(1,ditem1);
    QStandardItem *ditem2   = new QStandardItem(Icons::icFamily(),"");
    ditem2->setEditable(false);
    ditem2->setTextAlignment(Qt::AlignCenter);
    ditem2->setToolTip(tr("Document disponible pour tous les utilisateurs"));
    m_docsmodel->setHorizontalHeaderItem(2,ditem2);
    QStandardItem *ditem3   = new QStandardItem(Icons::icEditer(),"");
    ditem3->setEditable(false);
    ditem3->setTextAlignment(Qt::AlignCenter);
    ditem3->setToolTip(tr("Document éditable au moment de l'impression"));
    m_docsmodel->setHorizontalHeaderItem(3,ditem3);
    QStandardItem *ditem4   = new QStandardItem(Icons::icAdministratif(),"");
    ditem4->setEditable(false);
    ditem4->setTextAlignment(Qt::AlignCenter);
    ditem4->setToolTip(tr("Document administratif"));
    m_docsmodel->setHorizontalHeaderItem(4,ditem4);
    QStandardItem *ditem5   = new QStandardItem("");
    ditem5->setEditable(false);
    m_docsmodel->setHorizontalHeaderItem(5,ditem5);
    m_docsmodel->setRowCount(Datas::I()->impressions->impressions()->size());
    m_docsmodel->setColumnCount(6);

    int         i = 0;
    foreach (Impression *doc, *Datas::I()->impressions->impressions())
    {
        SetDocumentToRow(doc, i);
        i++;
    }
    if (m_docsmodel->rowCount()>0)
    {
        m_docsmodel->sort(5);
        QItemSelectionModel *m = ui->DocsupTableView->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
        ui->DocsupTableView->setModel(m_docsmodel);
        delete m;
        ui->DocsupTableView->setColumnWidth(0,30);        // checkbox
        ui->DocsupTableView->setColumnWidth(1,289);       // Resume
        ui->DocsupTableView->setColumnWidth(2,30);        // Public   - affiche un check si document public
        ui->DocsupTableView->setColumnWidth(3,30);        // Editable - affiche un check si document editable
        ui->DocsupTableView->setColumnWidth(4,30);        // Medical  - affiche un check si document medical
        ui->DocsupTableView->setColumnWidth(5,0);         // 0 ou 1 suivant que l'item est coché ou pas, suivi du résumé - sert au tri des documents
        ui->DocsupTableView->setColumnHidden(5,true);
        ui->DocsupTableView->FixLargeurTotale();
        wdg_docsbuttonframe->widgButtonParent()->setFixedWidth(ui->DocsupTableView->width());
        ui->DocsupTableView->setGridStyle(Qt::DotLine);

//        connect(ui->DocsupTableView,    &QAbstractItemView::entered,               this,   [&] (QModelIndex idx) {
//                                                                                                        Impression *doc = getDocumentFromIndex(idx);
//                                                                                                        if (doc && idx.column() == 1)
//                                                                                                            QToolTip::showText(cursor().pos(),DocumentToolTip(doc));
//                                                                                                        } );
        connect(ui->DocsupTableView,    &QAbstractItemView::doubleClicked,         this,   [&] (QModelIndex idx) {
                                                                                                        Impression *doc = getDocumentFromIndex(idx);
                                                                                                        if (doc && idx.column() == 1)
                                                                                                        {
                                                                                                            m_currentdocument = doc;
                                                                                                            if(doc->iduser() == currentuser()->id() && m_mode == Selection)
                                                                                                                ConfigMode(ModificationDOC);
                                                                                                        }
                                                                                                      });
        connect (ui->DocsupTableView,   &QWidget::customContextMenuRequested,      this,   &dlg_impressions::MenuContextuelDocuments);
        connect (ui->DocsupTableView->selectionModel(), &QItemSelectionModel::currentRowChanged,          this,   [&] (QModelIndex idx) {
                                                                                                                    m_currentdocument = getDocumentFromIndex(idx);
                                                                                                                    if (m_currentdocument)
                                                                                                                    {
                                                                                                                        AfficheTexteDocument(m_currentdocument);
                                                                                                                        ui->OKupPushButton->setEnabled(true);
                                                                                                                        EnableDocsButtons(m_currentdocument);
                                                                                                                    }
                                                                                                                });
        connect(ui->DocsupTableView,    &QAbstractItemView::clicked,               this,   [&] (QModelIndex idx)
                                                                                                    {// le bouton OK est enabled quand une case est cochée
                                                                                                        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->itemFromIndex(idx));
                                                                                                        if (itm)
                                                                                                            if(itm->isCheckable())
                                                                                                            {
                                                                                                                bool a = itm->ischecked();
                                                                                                                int row = itm->row();
                                                                                                                Impression *doc = dynamic_cast<Impression*>(itm->item());
                                                                                                                if(doc)
                                                                                                                    m_docsmodel->item(row,5)->setText((a?"0":"1") + doc->resume());
                                                                                                                EnableOKPushButton(idx);
                                                                                                            }
                                                                                                    });
        connect(ui->DocsupTableView->horizontalHeader(),    &QHeaderView::sectionClicked,   this,  [&] (int id)  {if(id == 0)  selectAllDocuments();});
        m_currentdocument = Q_NULLPTR;
    }


    //Remplissage Table Dossiers
    if (!Datas::I()->metadocuments->isfull())
        Datas::I()->metadocuments->initListe();
    ui->DossiersupTableView->disconnect();
    ui->DossiersupTableView->selectionModel()->disconnect();
    UpLineDelegate *line = new UpLineDelegate();
    connect(line,   &UpLineDelegate::textEdited, this, [=] {ui->OKupPushButton->setEnabled(true);});
    connect(line,   &UpLineDelegate::commitData, this, [=](QWidget *editor) {
                                                                         UpLineEdit *line = qobject_cast<UpLineEdit*>(editor);
                                                                         m_textdossierdelegate = line->text();
                                                                      });
    ui->DossiersupTableView->setItemDelegateForColumn(1,line);
    if (m_dossiersmodel == Q_NULLPTR)
        delete m_dossiersmodel;
    m_dossiersmodel = new UpStandardItemModel(this);

    QStandardItem *pitem0   = new QStandardItem(Icons::icCheck(),"");
    pitem0->setEditable(false);
    pitem0->setTextAlignment(Qt::AlignCenter);
    m_dossiersmodel->setHorizontalHeaderItem(0,pitem0);
    QStandardItem *pitem1   = new QStandardItem(tr("DOSSIERS"));
    pitem1->setEditable(false);
    pitem1->setTextAlignment(Qt::AlignLeft);
    m_dossiersmodel->setHorizontalHeaderItem(1,pitem1);
    QStandardItem *pitem2   = new QStandardItem(Icons::icFamily(),"");
    pitem2->setEditable(false);
    pitem2->setTextAlignment(Qt::AlignCenter);
    m_dossiersmodel->setHorizontalHeaderItem(2,pitem2);
    m_dossiersmodel->setRowCount(Datas::I()->metadocuments->dossiersimpressions()->size());
    m_dossiersmodel->setColumnCount(3);
    i = 0;
    foreach (DossierImpression *dossier, *Datas::I()->metadocuments->dossiersimpressions())
    {
        SetDossierToRow(dossier, i, false);
        i++;
    }
    if (m_dossiersmodel->rowCount()>0)
    {
        m_dossiersmodel->sort(1);
        QItemSelectionModel *m = ui->DossiersupTableView->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
        ui->DossiersupTableView->setModel(m_dossiersmodel);
        delete m;
        ui->DossiersupTableView->setColumnWidth(0,30);        // checkbox
        ui->DossiersupTableView->setColumnWidth(1,259);       // Resume
        ui->DossiersupTableView->setColumnWidth(2,30);        // Public
        ui->DossiersupTableView->FixLargeurTotale();
        wdg_dossiersbuttonframe->widgButtonParent()->setFixedWidth(ui->DossiersupTableView->width());
        ui->DossiersupTableView->setGridStyle(Qt::NoPen);
        connect(ui->DossiersupTableView,    &QAbstractItemView::entered,               this,   [&] (QModelIndex idx) {
                                                                                                        DossierImpression *dossier = getDossierFromIndex(idx);
                                                                                                        if (dossier && idx.column() == 1)
                                                                                                            QToolTip::showText(cursor().pos(),DossierToolTip(dossier));
                                                                                                        } );
        connect(ui->DossiersupTableView,    &QAbstractItemView::doubleClicked,         this,   [&] (QModelIndex idx) {
                                                                                                        DossierImpression *dossier = getDossierFromIndex(idx);
                                                                                                        if (dossier && idx.column() == 1)
                                                                                                        {
                                                                                                            m_currentdossier = dossier;
                                                                                                            if(dossier->iduser() == currentuser()->id() && m_mode == Selection)
                                                                                                                ConfigMode(ModificationDOSS);
                                                                                                        }
                                                                                                      });
        connect (ui->DossiersupTableView,   &QWidget::customContextMenuRequested,      this,   &dlg_impressions::MenuContextuelDossiers);

        connect (ui->DossiersupTableView->selectionModel(), &QItemSelectionModel::currentRowChanged,          this,   [&] (QModelIndex idx) {
                                                                                                                    m_currentdossier = getDossierFromIndex(idx);
                                                                                                                    if (m_currentdossier)
                                                                                                                    {
                                                                                                                        ui->OKupPushButton->setEnabled(true);
                                                                                                                        EnableDossiersButtons(m_currentdossier);
                                                                                                                    }
                                                                                                                });
        connect(ui->DossiersupTableView,    &QAbstractItemView::clicked,               this,   [&] (QModelIndex idx)
                                                                                                    {// le bouton OK est enabled quand une case est cochée
                                                                                                        QStandardItem *itm = m_dossiersmodel->itemFromIndex(idx);
                                                                                                        if (itm)
                                                                                                            if(itm->isCheckable())
                                                                                                            {
                                                                                                                bool A = (itm->checkState() == Qt::Checked);
                                                                                                                DossierImpression *dossier = getDossierFromIndex(idx);
                                                                                                                if (dossier)
                                                                                                                    CocheLesDocs(dossier->id(),A);
                                                                                                                EnableOKPushButton();
                                                                                                            }
                                                                                                    });
        m_currentdossier = Q_NULLPTR;
    }
}

void dlg_impressions::selectAllDocuments()
{
    bool a = Icons::CompareQIcon(m_docsmodel->horizontalHeaderItem(0)->icon(), Icons::icunBlackCheck());
    m_docsmodel->horizontalHeaderItem(0)->setIcon(a? Icons::icBlockCheck() : Icons::icunBlackCheck());
    for (int i=0; i< m_docsmodel->rowCount(); ++i)
    {
         QStandardItem *itm = m_docsmodel->item(i,0);
         if (itm)
             itm->setCheckState(a? Qt::Checked : Qt::Unchecked);
    }
}

void dlg_impressions::selectcurrentDocument(Impression *doc, QAbstractItemView::ScrollHint hint)
{
    ui->DocsupTableView->selectionModel()->reset();
    m_currentdocument = doc;
    if (!m_currentdocument)
        EnableDocsButtons();
    else for (int i=0; i<m_docsmodel->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(i));
        if (itm)
        {
            Impression *sdoc = dynamic_cast<Impression*>(itm->item());
            if (sdoc)
                if (m_currentdocument == sdoc)
                {
                    QModelIndex idx = m_docsmodel->index(i,1);
                    ui->DocsupTableView->selectionModel()->select(idx,QItemSelectionModel::SelectCurrent);
                    ui->DocsupTableView->scrollTo(idx, hint);
                    ui->OKupPushButton->setEnabled(true);
                    EnableDocsButtons(m_currentdocument);
                    AfficheTexteDocument(m_currentdocument);
                    break;
                }
        }
    }
}

void dlg_impressions::selectcurrentDossier(DossierImpression *dossier, QAbstractItemView::ScrollHint hint)
{
    ui->DossiersupTableView->selectionModel()->reset();
    m_currentdossier = dossier;
    if (!m_currentdossier)
        EnableDossiersButtons();
    else for (int i=0; i<m_dossiersmodel->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(i));
        if (itm)
        {
            DossierImpression *sdossier = dynamic_cast<DossierImpression*>(itm->item());
            if (sdossier)
                if (m_currentdossier == sdossier)
                {
                    QModelIndex idx = m_dossiersmodel->index(i,1);
                    ui->DossiersupTableView->selectionModel()->select(idx,QItemSelectionModel::SelectCurrent);
                    ui->DossiersupTableView->scrollTo(idx, hint);
                    ui->OKupPushButton->setEnabled(true);
                    EnableDossiersButtons(m_currentdossier);
                    break;
                }
        }
    }
}

void dlg_impressions::SetDocumentToRow(Impression*doc, int row, bool resizecolumn)
{
    if (!doc)
        return;
    if (row < 0 || row > m_docsmodel->rowCount()-1)
        return;

    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);

    UpStandardItem *pitem0 = new UpStandardItem("", doc);
    pitem0->setCheckable(true);
    m_docsmodel->setItem(row, 0, pitem0);
    QModelIndex index = m_docsmodel->index(row, 1, QModelIndex());
    m_docsmodel->setData(index, doc->resume());
        if (doc->iduser() != currentuser()->id())
        {
            m_docsmodel->itemFromIndex(index)->setFont(disabledFont);
            m_docsmodel->itemFromIndex(index)->setForeground(QBrush(QColor(0,0,140)));
        }
    UpStandardItem *pitem1 = new UpStandardItem("", doc);
    if (doc->ispublic())
        pitem1->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem1->setData(QPixmap(),Qt::DecorationRole);
    pitem1->setFlags(Qt::NoItemFlags);
    m_docsmodel->setItem(row, 2, pitem1);
    UpStandardItem *pitem2 = new UpStandardItem("", doc);
    if (doc->iseditable())
        pitem2->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem2->setData(QPixmap(),Qt::DecorationRole);
    pitem2->setFlags(Qt::NoItemFlags);
    m_docsmodel->setItem(row, 3, pitem2);
    UpStandardItem *pitem3 = new UpStandardItem("", doc);
    if (!doc->ismedical())
        pitem3->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem3->setData(QPixmap(),Qt::DecorationRole);
    pitem3->setFlags(Qt::NoItemFlags);
    m_docsmodel->setItem(row, 4, pitem3);
    UpStandardItem *pitem4 = new UpStandardItem("1" + doc->resume(), doc);      //! cette colonne servira à trier la table en mettent en avant les items cochés
                                                                                //! quand un item est coché, le 1 du début est remplacé par 0
    pitem4->setFlags(Qt::NoItemFlags);
    m_docsmodel->setItem(row, 5, pitem4);
    if(!resizecolumn)
        return;

    //! la suite est obligatoire poiur contourner un bug d'affichage sous MacOS
    ui->DocsupTableView->setColumnWidth(0,30);        // checkbox
    ui->DocsupTableView->setColumnWidth(1,289);       // Resume
    ui->DocsupTableView->setColumnWidth(2,30);        // Public   - affiche un check si document public
    ui->DocsupTableView->setColumnWidth(3,30);        // Editable - affiche un check si document editable
    ui->DocsupTableView->setColumnWidth(4,30);        // Medical  - affiche un check si document medical
    ui->DocsupTableView->setColumnWidth(5,0);         // 0 ou 1 suivant que l'item est coché ou pas, suivi du résumé - sert au tri des documents
}

void dlg_impressions::SetDossierToRow(DossierImpression*dossier, int row, bool resizecolumn)
{
    if(!dossier)
        return;
    if (row < 0 || row > m_dossiersmodel->rowCount()-1)
        return;
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    QPalette palette;
    palette.setColor(QPalette::Text,QColor(0,0,140));

    UpStandardItem *pitem0 = new UpStandardItem("", dossier);
    pitem0->setCheckable(true);
    m_dossiersmodel->setItem(row,0,pitem0);
    QModelIndex index = m_dossiersmodel->index(row, 1, QModelIndex());
    m_dossiersmodel->setData(index, dossier->resume());
        if (dossier->iduser() != currentuser()->id())
        {
            m_dossiersmodel->itemFromIndex(index)->setFont(disabledFont);
            m_dossiersmodel->itemFromIndex(index)->setForeground(QBrush(QColor(0,0,140)));
        }
    UpStandardItem *pitem1 = new UpStandardItem("", dossier);
    if (dossier->ispublic())
        pitem1->setData(Icons::pxBlackCheck().scaled(15,15),Qt::DecorationRole);
    else
        pitem1->setData(QPixmap(),Qt::DecorationRole);
    pitem1->setFlags(Qt::NoItemFlags);
    m_dossiersmodel->setItem(row,2, pitem1);
    if(!resizecolumn)
        return;

    //! la suite est obligatoire poiur contourner un bug d'affichage
    ui->DossiersupTableView->setColumnWidth(0,30);        // checkbox
    ui->DossiersupTableView->setColumnWidth(1,259);       // Resume
    ui->DossiersupTableView->setColumnWidth(2,30);        // Public
}

// ----------------------------------------------------------------------------------
// Supprime Document
// ----------------------------------------------------------------------------------
void dlg_impressions::SupprimmeDocument(Impression *doc)
{
    if(!doc)
        return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le  document\n") + doc->resume() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton(tr("Supprimer le document"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()  != &NoBouton)
    {
        m_currentdocument = Q_NULLPTR;
        int row = m_docsmodel->getRowFromItem(doc);
        //nettoyage de la table metadocs
        db->SupprRecordFromTable(doc->id(), CP_IDDOCUMENT_JOINTURESIMPRESSIONS , TBL_JOINTURESIMPRESSIONS);
        Datas::I()->impressions->SupprimeImpression(doc);
        if (row>-1 && row < m_docsmodel->rowCount())
            m_docsmodel->removeRow(row); //! declenche le signal currentrowchanged si la row current est la row supprimée et mcurrentdocument devient le document suivant ou qnullptre s'il n'y a pas de suivant
        if (m_docsmodel->rowCount() == 0)
            ConfigMode(CreationDOC);
        else
        {
            ConfigMode(Selection);
            if (row >= m_docsmodel->rowCount())
                row = m_docsmodel->rowCount()-1;
            if(m_docsmodel->rowCount()>0)
            {
                Remplir_TableView();
                doc = getDocumentFromIndex(m_docsmodel->index(row,0));
                if (doc)
                    selectcurrentDocument(doc);
            }
        }
    }
}

// ----------------------------------------------------------------------------------
// Supprime Dossier
// ----------------------------------------------------------------------------------
void dlg_impressions::SupprimmeDossier(DossierImpression *dossier)
{
    if(!dossier)
        return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le  dossier\n") + dossier->resume() + "?";
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
        int row = m_dossiersmodel->getRowFromItem(dossier);
        db->SupprRecordFromTable(dossier->id(), CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS , TBL_JOINTURESIMPRESSIONS);
        Datas::I()->metadocuments->SupprimeDossierImpression(dossier);
        if (row > -1 && row < m_dossiersmodel->rowCount())
            m_dossiersmodel->removeRow(row); //! declenche le signal currentrowchanged et mcurrentdosier devient le dossier suivanr ou qnullptre s'il n'y a pas de suivant
        if (m_docsmodel->rowCount() == 0)
            ConfigMode(CreationDOC);
        else
        {
            ConfigMode(Selection);
            if (row >= m_dossiersmodel->rowCount())
                row = m_dossiersmodel->rowCount()-1;
            dossier = getDossierFromIndex(m_dossiersmodel->index(row,0));
            if (dossier)
                selectcurrentDossier(dossier);
        }
    }
}


// ----------------------------------------------------------------------------------
// Verifie qu'un document peut devenir privé
// ----------------------------------------------------------------------------------
bool dlg_impressions::VerifDocumentPublic(Impression *doc, bool msg)
{
    if (!doc)
        return false;
    for (auto itdossier = Datas::I()->metadocuments->dossiersimpressions()->begin(); itdossier != Datas::I()->metadocuments->dossiersimpressions()->end(); ++itdossier)
    {
        DossierImpression *dossier = itdossier.value();
        if (dossier)
        {
            if (dossier->ispublic() && dossier->iduser() == currentuser()->id())
            {
                if (!dossier->haslistdocsloaded())
                    Datas::I()->metadocuments->loadlistedocs(dossier);
                if (dossier->listiddocs().contains(doc->id()))
                {
//                    for (int i=0; i<dossier->listiddocs().size(); ++i) {
//                        Impression *  impr = Datas::I()->impressions->getById(dossier->listiddocs().at(i));
//                        if (impr)
//                            qDebug() << impr->id() << impr->resume() << dossier->resume();
//                    }
                    if (msg)
                        UpMessageBox::Watch(this,tr("Vous ne pouvez pas rendre privé ce document"), tr("Il est incorporé dans le dossier public\n- ") + dossier->resume() +
                                            tr(" -\nVous devez d'abord rendre ce dossier privé!"));
                    return false;
                }
            }
        }
    }
    return true;
}

// ----------------------------------------------------------------------------------
// Verifie qu'un dossier ne reste pas coché pour rien
// ----------------------------------------------------------------------------------
void dlg_impressions::VerifDossiers()
{
    for (int j=0; j<m_dossiersmodel->rowCount(); j++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_dossiersmodel->item(j,0));
        if (itm)
        {
            if (itm->checkState() == Qt::Checked)
            {
                DossierImpression *dossier = dynamic_cast<DossierImpression*>(itm->item());
                if (dossier)
                {
                    bool a = false;
                    for (int k=0; k<dossier->listiddocs().size(); k++)
                    {
                        for (int l=0; l<m_docsmodel->rowCount(); l++)
                        {
                            UpStandardItem *sitm = dynamic_cast<UpStandardItem*>(m_docsmodel->item(l,0));
                            if (sitm->checkState() == Qt::Checked)
                            {a = true;  break;}
                        }
                        if (a) break;
                    }
                    if (!a) itm->setCheckState(Qt::Unchecked);
                }
            }
        }
    }
}
