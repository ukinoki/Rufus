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

#include "dlg_programmationinterventions.h"

dlg_programmationinterventions::dlg_programmationinterventions(Patient *pat, QWidget *parent) : UpDialog(PATH_FILE_INI, "PositionsFiches/PositionProgramIntervention",parent)
{
    m_currentchirpatient = pat;
    setWindowTitle(tr("Programmer une intervention pour ") + m_currentchirpatient->prenom() + " " + m_currentchirpatient->nom());
    wdg_listmedecinscombo           = new QComboBox();
    QHBoxLayout *choixmedecinLay    = new QHBoxLayout();
    QHBoxLayout *programmLay        = new QHBoxLayout();

    QFont font  = qApp->font();
    font        .setPointSize(font.pointSize()+2);

    wdg_interventionstreeView   ->setFocusPolicy(Qt::NoFocus);
    wdg_interventionstreeView   ->setPalette(QPalette(Qt::white));
    wdg_interventionstreeView   ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_interventionstreeView   ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_interventionstreeView   ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_interventionstreeView   ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_interventionstreeView   ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_interventionstreeView   ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    wdg_sessionstreeView    ->setMaximumWidth(340);
    wdg_sessionstreeView    ->setColumnWidth(0,340);
    wdg_sessionstreeView    ->setFocusPolicy(Qt::StrongFocus);
    wdg_sessionstreeView    ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_sessionstreeView    ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_sessionstreeView    ->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_sessionstreeView    ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_sessionstreeView    ->setSelectionMode(QAbstractItemView::SingleSelection);

    UpLabel* lblmedecins = new UpLabel;
    lblmedecins         ->setText(tr("Programme opératoire de "));
    choixmedecinLay     ->addWidget(lblmedecins);
    choixmedecinLay     ->addWidget(wdg_listmedecinscombo);
    choixmedecinLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixmedecinLay     ->setSpacing(5);
    choixmedecinLay     ->setContentsMargins(0,0,0,0);

    wdg_buttonsessionsframe     = new WidgetButtonFrame(wdg_sessionstreeView);
    wdg_buttonsessionsframe     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect (wdg_buttonsessionsframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixSessionFrame);
    wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(false);
    wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(false);

    wdg_buttoninterventionframe     = new WidgetButtonFrame(wdg_interventionstreeView);
    wdg_buttoninterventionframe     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    UpLabel *lblinterventions = new UpLabel();
    lblinterventions->setText("Total interventions");
    wdg_buttoninterventionframe->layButtons()->insertWidget(0, lblinterventions);
    connect (wdg_buttoninterventionframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixInterventionFrame);

    programmLay     ->addWidget(wdg_buttonsessionsframe->widgButtonParent());
    programmLay     ->addWidget(wdg_buttoninterventionframe->widgButtonParent());
    programmLay     ->setSpacing(5);
    programmLay     ->setContentsMargins(0,0,0,0);

    dlglayout()     ->insertLayout(0, programmLay);
    dlglayout()     ->insertLayout(0, choixmedecinLay);

//    QVBoxLayout *lay0but            = new QVBoxLayout;
//    lay0but                         ->addWidget(incidentbutt);
//    lay0but                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
//    lay0but                         ->addWidget(commandeIOLbutt);
//    buttbox                         ->insertLayout(0, lay0but);

//    QVBoxLayout *lay1but            = new QVBoxLayout;
//    lay1but                         ->addWidget(manufacturerbutt);
//    lay1but                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
//    lay1but                         ->addWidget(IOLbutt);
//    buttbox                         ->insertLayout(0, lay1but);
//    buttbox                         ->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *buttbox            = new QHBoxLayout;
    UpPushButton *incidentbutt      = new UpPushButton(tr("Rapport d'incident"));
    UpPushButton *commandeIOLbutt   = new UpPushButton(tr("Commande d'implants"));
    UpPushButton *manufacturerbutt  = new UpPushButton(tr("Gestion fabricants"));
    UpPushButton *IOLbutt           = new UpPushButton(tr("Gestion implants"));
    QSize size = QSize(190,35);
    QSize sizeicon = QSize(20,20);
    incidentbutt                    ->setFixedSize(size);
    commandeIOLbutt                 ->setFixedSize(size);
    manufacturerbutt                ->setFixedSize(size);
    IOLbutt                         ->setFixedSize(size);
    incidentbutt                    ->setIcon(Icons::icOups());
    commandeIOLbutt                 ->setIcon(Icons::icNotepad());
    manufacturerbutt                ->setIcon(Icons::icFactory());
    IOLbutt                         ->setIcon(Icons::icIOL());
    incidentbutt                    ->setIconSize(sizeicon);
    commandeIOLbutt                 ->setIconSize(sizeicon);
    manufacturerbutt                ->setIconSize(sizeicon);
    IOLbutt                         ->setIconSize(sizeicon);
    buttbox                         ->addWidget(incidentbutt);
    buttbox                         ->addWidget(commandeIOLbutt);
    buttbox                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    buttbox                         ->addWidget(manufacturerbutt);
    buttbox                         ->addWidget(IOLbutt);
    buttbox                         ->setSpacing(5);
    buttbox                         ->setContentsMargins(0,0,0,0);
    buttbox                         ->setSizeConstraint(QLayout::SetFixedSize);
    buttonslayout()                 ->insertLayout(0, buttbox);

    connect(manufacturerbutt,       &QPushButton::clicked,                                  this,   &dlg_programmationinterventions::ListeManufacturers);
    connect(IOLbutt,                &QPushButton::clicked,                                  this,   &dlg_programmationinterventions::ListeIOLs);

    AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonOK);
    connect(OKButton,     &QPushButton::clicked,    this, &QDialog::close);
    setModal(true);
    dlglayout()->setStretch(0,1);
    dlglayout()->setStretch(1,15);
    setFixedWidth(1000);

    m_medecinsmodel = new QStandardItemModel(this);

    foreach (User* usr, *Datas::I()->users->actifs())
        if (usr->isMedecin())
        {
            QList<QStandardItem *> items;
            items << new QStandardItem(usr->login())
                  << new QStandardItem(QString::number(usr->id()));
            m_medecinsmodel->appendRow(items);
        }
    m_medecinsmodel->sort(0, Qt::AscendingOrder);
    for (int i=0; i< m_medecinsmodel->rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(m_medecinsmodel->item(i,0)->text());             //! le login
        wdg_listmedecinscombo->setItemData(i, m_medecinsmodel->item(i,1)->text());      //! l'id en data
    }
    if (Datas::I()->users->userconnected()->isMedecin())
    {
        wdg_listmedecinscombo->setCurrentIndex(wdg_listmedecinscombo->findData(Datas::I()->users->userconnected()->id()));
        wdg_listmedecinscombo->setEnabled(false);
        ChoixMedecin(wdg_listmedecinscombo->findData(Datas::I()->users->userconnected()->id()));
    }
    else
    {
        connect(wdg_listmedecinscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &dlg_programmationinterventions::ChoixMedecin);
        ChoixMedecin(0);
    }
    connect(wdg_sessionstreeView,       &QWidget::customContextMenuRequested,                   this, &dlg_programmationinterventions::MenuContextuelSessions);
    connect(wdg_interventionstreeView,  &QWidget::customContextMenuRequested,                   this, &dlg_programmationinterventions::MenuContextuelInterventionsions);
    Datas::I()->typesinterventions->initListe();
    ReconstruitListeTypeInterventions();
}

dlg_programmationinterventions::~dlg_programmationinterventions()
{
}

/*! les sessions ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ChoixMedecin(int idx)
{
    m_currentchiruser = Datas::I()->users->getById(wdg_listmedecinscombo->itemData(idx).toInt());
    if (m_currentchiruser == Q_NULLPTR)
        return;
    Datas::I()->sites->initListeByUser(m_currentchiruser->id());
    Datas::I()->sessionsoperatoires->initListebyUserId(m_currentchiruser->id());
    RemplirTreeSessions();
}

void dlg_programmationinterventions::ChoixSessionFrame()
{
    switch (wdg_buttonsessionsframe->Choix()) {
    case WidgetButtonFrame::Plus:
        CreerSession();
        break;
    case WidgetButtonFrame::Modifier:
        EditSession();
        break;
    case WidgetButtonFrame::Moins:
        SupprimeSession();
        break;
    }
}

void dlg_programmationinterventions::AfficheInterventionsSession(QModelIndex idx)
{
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel->itemFromIndex(idx));
    if (upitem == Q_NULLPTR)
        return;
    m_currentsession = dynamic_cast<SessionOperatoire*>(upitem->item());
    wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(m_currentsession != Q_NULLPTR);
    wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(m_currentsession != Q_NULLPTR);
    wdg_buttoninterventionframe->wdg_plusBouton->setEnabled(m_currentsession != Q_NULLPTR);
    Datas::I()->interventions->initListebySessionId(m_currentsession->id());
    RemplirTreeInterventions();
}

void dlg_programmationinterventions::RemplirTreeSessions(SessionOperatoire* session)
{
    disconnect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::AfficheInterventionsSession);
    if (m_sessionsmodel == Q_NULLPTR)
        delete m_sessionsmodel;
    m_sessionsmodel = new QStandardItemModel(this);
    m_currentsession = Q_NULLPTR;
    foreach (SessionOperatoire* session, *Datas::I()->sessionsoperatoires->sessions())
    {
        QList<QStandardItem *> items;
        QString nomsession = session->date().toString("dd-MMM-yy");
        Site* site = Datas::I()->sites->getById(session->idlieu());
        if (site != Q_NULLPTR)
            nomsession += " - " + site->nom();
        UpStandardItem *item = new UpStandardItem(nomsession, session);
        if (site != Q_NULLPTR)
            item->setForeground(QBrush(QColor("#" + Datas::I()->sites->getById(session->idlieu())->couleur())));
        items << item << new UpStandardItem(session->date().toString("yyyy-MM-dd"));
        m_sessionsmodel->appendRow(items);
    }
    m_sessionsmodel->sort(1, Qt::AscendingOrder);
    m_sessionsmodel->takeColumn(1);
    for (int i=0; i< m_sessionsmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(0));
        if (itm != Q_NULLPTR)
        {
            SessionOperatoire* sess = dynamic_cast<SessionOperatoire*>(itm->item());
            if (sess)
                if (sess->incident() != "")
                {
                    UpStandardItem *itminc = new UpStandardItem(tr("Incident") + " : " + sess->incident(), session);
                    itm->appendRow(itminc);
                }
        }
    }
    wdg_sessionstreeView->setModel(m_sessionsmodel);
    m_sessionsmodel->setHeaderData(0, Qt::Horizontal, tr("Sessions"));
    wdg_sessionstreeView->expandAll();
    QModelIndex idx;
    if (m_sessionsmodel->rowCount() >0)
    {
        if (session == Q_NULLPTR)
            idx = m_sessionsmodel->item(m_sessionsmodel->rowCount()-1)->index();        //! l'index de ce dernier item
        else for (int i=0; i<m_sessionsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
            if (itm->item() == session)
            {
                idx = itm->index();
                break;
            }
        }
        wdg_sessionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        wdg_sessionstreeView->setCurrentIndex(idx);
    }
    connect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::AfficheInterventionsSession);
    AfficheInterventionsSession(idx);
}

void dlg_programmationinterventions::CreerSession()
{
    UpDialog            *dlg_session = new UpDialog(this);
    dlg_session->setAttribute(Qt::WA_DeleteOnClose);
    dlg_session->setWindowTitle(tr("créer une session opératoire pour ") + m_currentchirpatient->prenom() + " " + m_currentchirpatient->nom());

    QHBoxLayout *choixdateLay    = new QHBoxLayout();
    UpLabel* lbldate = new UpLabel;
    lbldate         ->setText(tr("Date"));
    QDateEdit *dateedit = new QDateEdit(QDate::currentDate());
    dateedit        ->setFixedSize(QSize(120,24));
    dateedit        ->setCalendarPopup(true);
    choixdateLay    ->addWidget(lbldate);
    choixdateLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixdateLay    ->addWidget(dateedit);
    choixdateLay    ->setSpacing(5);
    choixdateLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixsiteLay    = new QHBoxLayout();
    UpLabel* lblsite = new UpLabel;
    lblsite               ->setText(tr("Site"));
    QComboBox *sitecombo = new QComboBox();
    sitecombo       ->setEditable(true);
    sitecombo       ->lineEdit()->setReadOnly(true);
    sitecombo       ->lineEdit()->setAlignment(Qt::AlignRight);
    choixsiteLay    ->addWidget(lblsite);
    choixsiteLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixsiteLay    ->addWidget(sitecombo);
    choixsiteLay    ->setSpacing(5);
    choixsiteLay    ->setContentsMargins(0,0,0,0);

    foreach (Site* site, *Datas::I()->sites->sites())
        sitecombo->addItem(site->nom(), site->id());

    dlg_session->dlglayout()   ->insertLayout(0, choixsiteLay);
    dlg_session->dlglayout()   ->insertLayout(0, choixdateLay);
    dlg_session->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_session->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_session->OKButton, &QPushButton::clicked, dlg_session, [&]
    {
        QDate date = dateedit->date();
        int idsite = sitecombo->currentData().toInt();
        for (int i = 0; i < m_sessionsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
            SessionOperatoire *session = dynamic_cast<SessionOperatoire*>(upitem->item());
            if (session->date() ==  date && session->idlieu() == idsite)
            {
                UpMessageBox::Watch(this, tr("Cette session existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_DATE_SESSIONOPERATOIRE]    = date.toString("yyyy-MM-dd");
        listbinds[CP_IDLIEU_SESSIONOPERATOIRE]  = idsite;
        listbinds[CP_IDUSER_SESSIONOPERATOIRE]  = m_currentchiruser->id();
        m_currentsession = Datas::I()->sessionsoperatoires->CreationSessionOperatoire(listbinds);
        RemplirTreeSessions();
        dlg_session->close();
    });
    dlg_session->exec();
}

void dlg_programmationinterventions::EditSession()
{
    if (m_currentsession == Q_NULLPTR)
        return;
    UpDialog            *dlg_session = new UpDialog(this);
    dlg_session->setAttribute(Qt::WA_DeleteOnClose);
    dlg_session->setWindowTitle(tr("Modifier une session opératoire pour ") + m_currentchirpatient->prenom() + " " + m_currentchirpatient->nom());

    QHBoxLayout *choixdateLay    = new QHBoxLayout();
    UpLabel* lbldate = new UpLabel;
    lbldate         ->setText(tr("Date"));
    QDateEdit *dateedit = new QDateEdit(QDate::currentDate());
    dateedit        ->setFixedSize(QSize(120,24));
    dateedit        ->setDate(m_currentsession->date());
    dateedit        ->setCalendarPopup(true);
    choixdateLay    ->addWidget(lbldate);
    choixdateLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixdateLay    ->addWidget(dateedit);
    choixdateLay    ->setSpacing(5);
    choixdateLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixsiteLay    = new QHBoxLayout();
    UpLabel* lblsite = new UpLabel;
    lblsite               ->setText(tr("Site"));
    QComboBox *sitecombo = new QComboBox();
    choixsiteLay    ->addWidget(lblsite);
    choixsiteLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixsiteLay    ->addWidget(sitecombo);
    choixsiteLay    ->setSpacing(5);
    choixsiteLay    ->setContentsMargins(0,0,0,0);

    foreach (Site* site, *Datas::I()->sites->sites())
        sitecombo->addItem(site->nom(), site->id());
    sitecombo->setCurrentIndex(sitecombo->findData(m_currentsession->idlieu()));

    dlg_session->dlglayout()   ->insertLayout(0, choixsiteLay);
    dlg_session->dlglayout()   ->insertLayout(0, choixdateLay);
    dlg_session->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_session->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_session->OKButton, &QPushButton::clicked, dlg_session, [&]
    {
        QDate date = dateedit->date();
        int idsite = sitecombo->currentData().toInt();
        for (int i = 0; i < m_sessionsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
            SessionOperatoire *rechsession = dynamic_cast<SessionOperatoire*>(upitem->item());
            if (rechsession->date() ==  date && rechsession->idlieu() == idsite)
            {
                UpMessageBox::Watch(this, tr("Cette session existe déjà!"));
                return;
            }
        }
        ItemsList::update(m_currentsession, CP_DATE_SESSIONOPERATOIRE, date);
        ItemsList::update(m_currentsession, CP_IDLIEU_SESSIONOPERATOIRE, idsite);
        RemplirTreeSessions();
        dlg_session->close();
    });
    dlg_session->exec();
}

void dlg_programmationinterventions::SupprimeSession()
{
    if (m_currentsession == Q_NULLPTR)
        return;
    QString nomsession = m_currentsession->date().toString("dd-MMM-yy");
    Site* site = Datas::I()->sites->getById(m_currentsession->idlieu());
    if (site != Q_NULLPTR)
        nomsession += " - " + site->nom();
    if (UpMessageBox::Question(this, tr("Voulez-vous supprimer la session"), nomsession + " ?") != UpSmallButton::STARTBUTTON)
        return;
    foreach (Intervention* interv, *Datas::I()->interventions->interventions())
    {
        m_currentintervention = interv;
        SupprimeIntervention();
    }
    Datas::I()->sessionsoperatoires->SupprimeSessionOperatoire(m_currentsession);
    RemplirTreeSessions();
}

void dlg_programmationinterventions::MenuContextuelSessions()
{
    m_ctxtmenusessions = new QMenu(this);
    QModelIndex psortindx   = wdg_sessionstreeView->indexAt(wdg_sessionstreeView->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel->itemFromIndex(psortindx));
    if (upitem == Q_NULLPTR)
    {
        QAction *pAction_CreerSession = m_ctxtmenusessions->addAction(tr("Créer une session"));
        connect (pAction_CreerSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::CreerSession);
    }
    else
    {
        SessionOperatoire *session = dynamic_cast<SessionOperatoire*>(upitem->item());
        if (session == Q_NULLPTR)
        {
            delete m_ctxtmenusessions;
            return;
        }
        QAction *pAction_ModifSession = m_ctxtmenusessions->addAction(tr("Modifier la session"));
        connect (pAction_ModifSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::EditSession);
        QAction *pAction_SupprSession = m_ctxtmenusessions->addAction(tr("Supprimer la session"));
        connect (pAction_SupprSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::SupprimeSession);
        if (Datas::I()->users->userconnected()->isMedecin())
        {
            QAction *pAction_IncidentSession = m_ctxtmenusessions->addAction(tr("Enregistrer un incident sur cette session"));
            connect (pAction_IncidentSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::EnregistreIncidentSession);
        }
    }
    // ouvrir le menu
    m_ctxtmenusessions->exec(cursor().pos());
    delete m_ctxtmenusessions;
}


/*! les interventions ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ChoixInterventionFrame()
{
    switch (wdg_buttoninterventionframe->Choix()) {
    case WidgetButtonFrame::Plus:
        CreerFicheIntervention();
        break;
    case WidgetButtonFrame::Modifier:
        ModifFicheIntervention();
        break;
    case WidgetButtonFrame::Moins:
        SupprimeIntervention();
        break;
    }
}

void dlg_programmationinterventions::ChoixIntervention(QModelIndex idx)
{
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_interventionsmodel->itemFromIndex(idx));
    if (upitem == Q_NULLPTR)
        return;
    m_currentintervention = dynamic_cast<Intervention*>(upitem->item());
    if (m_currentintervention == Q_NULLPTR)
    {
        wdg_buttoninterventionframe->wdg_moinsBouton->setEnabled(false);
        wdg_buttoninterventionframe->wdg_modifBouton->setEnabled(false);
        return;
    }
    //qDebug() << m_currentsession->date() << Datas::I()->sites->getById(m_currentsession->idlieu())->nom();
    wdg_buttoninterventionframe->wdg_moinsBouton->setEnabled(true);
    wdg_buttoninterventionframe->wdg_modifBouton->setEnabled(true);
}

void dlg_programmationinterventions::RemplirTreeInterventions(Intervention* intervention)
{
    disconnect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    if (m_interventionsmodel == Q_NULLPTR)
        delete m_interventionsmodel;
    m_interventionsmodel = new QStandardItemModel(this);

    m_currentintervention = Q_NULLPTR;
    QStandardItem * rootNodeDate = m_interventionsmodel->invisibleRootItem();
    QList<QTime> listheures;

    foreach (Intervention *interv, *Datas::I()->interventions->interventions())
    {
        // créations des entêtes par heure
        if (!listheures.contains(interv->heure()))
            listheures << interv->heure();
    }
    UpStandardItem *itempat = Q_NULLPTR;
    UpStandardItem *itemobs = Q_NULLPTR;
    UpStandardItem *itemiol = Q_NULLPTR;
    UpStandardItem *itemddn = Q_NULLPTR;
    UpStandardItem *itemtyp = Q_NULLPTR;
    UpStandardItem *iteminc = Q_NULLPTR;
    UpStandardItem *heureitem = Q_NULLPTR;
    // Tri par date
    std::sort(listheures.begin(), listheures.end());
    for (int i=0; i<listheures.size(); ++i)
    {
        QString heurestring = listheures.at(i).toString("HH:mm");
        heureitem    = new UpStandardItem("- " + heurestring + " -");
        heureitem    ->setForeground(QBrush(QColor(Qt::darkGreen)));
        heureitem    ->setEditable(false);
        heureitem    ->setIcon(Icons::icDate());
        rootNodeDate ->appendRow(heureitem);
    }
    int a = 0;
    foreach (Intervention *interv, *Datas::I()->interventions->interventions())
    {
        QString heure = "- " + interv->heure().toString("HH:mm") + " -";
        QList<QStandardItem *> listitemsheure = m_interventionsmodel->findItems(heure);
        if (listitemsheure.size()>0)
        {
            QString nompatient = "";
            Patient * pat = Datas::I()->patients->getById(interv->idpatient(), Item::LoadDetails);
            if (!pat)
                return;
            nompatient  = pat->nom().toUpper() + " " + pat->prenom();
            itempat     = new UpStandardItem(nompatient, interv);
            QFont fontitem = m_font;
            fontitem    .setBold(true);
            itempat     ->setFont(fontitem);
            itempat     ->setForeground(QBrush(QColor(Qt::darkBlue)));
            itempat     ->setEditable(false);
            listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itempat << new QStandardItem(QString::number(a) + "a"));         //! nom du patient

            TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());                              //! type d'intervention et anesthésie
            if (typ)
            {
                QString typinterv = typ->typeintervention().toUpper();
                if (interv->cote() != Utils::NoLoSo)
                    typinterv += " - " + tr("Côté") + " " +  Utils::TraduitCote(interv->cote()).toLower();
                itemtyp = new UpStandardItem();
                itemtyp ->setForeground(QBrush(QColor(Qt::darkBlue)));
                if (interv->anesthesie() != Intervention::NoLoSo)                                                                           //! type d'anesthésie
                {
                    QString anesth = "";
                    switch (interv->anesthesie()) {
                    case Intervention::Locale:          anesth = tr("Anesthésie locale");           break;
                    case Intervention::LocoRegionale:   anesth = tr("Anesthésie locoregionale");    break;
                    case Intervention::Generale:        anesth = tr("Anesthésie générale");         break;
                    default: break;
                    }
                    typinterv += " - " + anesth;
                   if (interv->anesthesie() == Intervention::Generale)
                        itemtyp ->setForeground(QBrush(QColor(Qt::red)));
                }
                itemtyp->setText("\t" + typinterv);
                itemtyp->setitem(interv);
                itemtyp ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemtyp << new QStandardItem(QString::number(a) + "b"));
            }
            QString sexeddntel = (pat->sexe() == "M"? tr("Né le") : tr("Née le"))                                                           //! date de naissance - sexe - telephone
                    + " " + pat->datedenaissance().toString("dd-MM-yyyy")
                    + " - " + Utils::CalculAge(pat->datedenaissance())["toString"].toString();
            if (pat->telephone() != "" || pat->portable() != "")                                                                            //! telephone
            {
                QString tel = tr("Tel") + " ";
                if (pat->telephone() != "")
                {
                    tel += pat->telephone();
                    if (pat->portable() != "")
                        tel += " - " + pat->portable();
                }
                else
                    tel += pat->portable();
                sexeddntel += "- " + tel;
            }
            itemddn = new UpStandardItem("\t" + sexeddntel, interv);
            itemddn ->setForeground(QBrush(QColor(Qt::darkGray)));
            itemddn ->setEditable(false);
            listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemddn << new QStandardItem(QString::number(a) + "c"));

            if (interv->idIOL()>0)                                                                                                          //! IOL
            {
                QString ioltxt = "";
                IOL *iol = Datas::I()->iols->getById(interv->idIOL());
                if (iol)
                {
                    Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                    if (man != Q_NULLPTR)
                        ioltxt += man->nom().toUpper() + " " + iol->modele() + " ";
                }
                QString pwriol = QString::number(interv->puissanceIOL(), 'f', 2);
                if (interv->puissanceIOL() > 0)
                    pwriol = "+" + pwriol;
                ioltxt += pwriol;
                if (interv->cylindreIOL() != 0.0)
                {
                    QString cyliol = QString::number(interv->cylindreIOL(), 'f', 2);
                    if (interv->cylindreIOL() > 0)
                        cyliol = "+" + cyliol;
                    ioltxt += " Cyl. " + cyliol;
                }
                itemiol = new UpStandardItem("\t" + tr("Implant") + " : " + ioltxt, interv);
                itemiol ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemiol << new QStandardItem(QString::number(a) + "d"));
            }
            if (interv->observation() != "")                                                                                            //! observation
            {
                itemobs = new UpStandardItem("\t" + tr("Remarque") + " : " + interv->observation(), interv);
                itemobs ->setForeground(QBrush(QColor(Qt::red)));
                itemobs ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemobs << new QStandardItem(QString::number(a) + "e"));
            }
            if (interv->incident() != "")                                                                                               //! incident
            {
                iteminc = new UpStandardItem("\t" + tr("Incident") + " : " + interv->incident(), interv);
                iteminc ->setForeground(QBrush(QColor(Qt::red)));
                iteminc ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << iteminc << new QStandardItem(QString::number(a) + "f"));
            }
        }
        listitemsheure.at(0)->sortChildren(1);
        ++a;
    }
    wdg_interventionstreeView->setModel(m_interventionsmodel);
    m_interventionsmodel->setHeaderData(0, Qt::Horizontal, tr("Interventions"));
    m_interventionsmodel->setHeaderData(1, Qt::Horizontal, "");
    m_interventionsmodel->setHeaderData(2, Qt::Horizontal, "");
    wdg_interventionstreeView->expandAll();
    wdg_interventionstreeView   ->setColumnWidth(0,340);
    wdg_interventionstreeView   ->header()->setSectionResizeMode(QHeaderView::Fixed);
    wdg_interventionstreeView   ->setSortingEnabled(false);
    connect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    QModelIndex idx;
    if (m_interventionsmodel->rowCount() >0)
    {
        if (intervention == Q_NULLPTR)
            idx = m_interventionsmodel->item(m_interventionsmodel->rowCount()-1)->index();        //! l'index du dernier item
        else for (int i=0; i<m_interventionsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_interventionsmodel->item(i));
            if (itm->item() == intervention)
            {
                idx = itm->index();
                break;
            }
        }
        wdg_interventionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        wdg_interventionstreeView->setCurrentIndex(idx);
    }
    connect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    ChoixIntervention(idx);
}

void dlg_programmationinterventions::EnregistreIncident(Item *itm)
{
    if( itm == Q_NULLPTR)
        return;
    QString mode = "";
    QString table = "";
    QString champ = "";
    QString idchamp = "";
    QString incident = "";
    Intervention * interv = dynamic_cast<Intervention*>(itm);
    if (interv)
    {
        table = TBL_LIGNESPRGOPERATOIRES;
        champ = CP_INCIDENT_LIGNPRGOPERATOIRE;
        idchamp = CP_ID_LIGNPRGOPERATOIRE;
        mode = "intervention";
        if (interv->incident() != "")
            incident = interv->incident();
    }
    else
    {
        SessionOperatoire * session = dynamic_cast<SessionOperatoire*>(itm);
        if (session)
        {
            table = TBL_LIGNESPRGOPERATOIRES;
            champ = CP_INCIDENT_LIGNPRGOPERATOIRE;
            idchamp = CP_ID_LIGNPRGOPERATOIRE;
            mode = "session";
            if (session->incident() != "")
                incident = session->incident();
        }
        else
            return;
    }

    UpDialog            *dlg_incident = new UpDialog(this);
    dlg_incident->setAttribute(Qt::WA_DeleteOnClose);
    dlg_incident->setWindowTitle(tr("Rapport d'incident"));

    QTextEdit *incidenttxtedit  = new QTextEdit();
    incidenttxtedit             ->setFixedSize(QSize(250,100));
    incidenttxtedit             ->setText(incident);

    dlg_incident->dlglayout()   ->insertWidget(0, incidenttxtedit);
    dlg_incident->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_incident->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_incident->OKButton, &QPushButton::clicked, dlg_incident, [&]
    {
        QString incident = incidenttxtedit->toPlainText();
        QHash<QString, QVariant> listbinds;
        listbinds[champ] = incident;
        DataBase::I()->UpdateTable(table, listbinds, "where " + idchamp + " = " + QString::number(itm->id()));
        if (mode == "intervention")
        {
            m_currentintervention->setincident(incident);
            RemplirTreeInterventions(m_currentintervention);
        }
        else if (mode == "session")
        {
            m_currentsession->setincident(incident);
            RemplirTreeSessions(m_currentsession);
        }
        dlg_incident->close();
    });
    dlg_incident->exec();
}


void dlg_programmationinterventions::FicheIntervention()
{

    Intervention *interv = m_currentintervention;
    bool verifencours = false;
    bool verifmanufacturerencours = false;
    UpDialog *dlg_intervention = new UpDialog(this);
    Patient *pat = (interv == Q_NULLPTR? m_currentchirpatient : Datas::I()->patients->getById(interv->idpatient()));
    if (pat != Q_NULLPTR)
            dlg_intervention->setWindowTitle(pat->prenom() + " " + pat->nom());

    QHBoxLayout *choixsessionLay    = new QHBoxLayout();
    UpLabel* lblsession = new UpLabel;
    lblsession          ->setText(tr("Session"));
    UpComboBox *sessioncombo = new UpComboBox();
    sessioncombo        ->setModel(m_sessionsmodel);
    for (int i=0; i< m_sessionsmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i,0));
        if (itm)
            if (itm->item())
                if (itm->item()->id() == m_currentsession->id())
                {
                    sessioncombo  ->setCurrentIndex(i);
                    break;
                }
    }
    sessioncombo        ->setFixedSize(QSize(300,28));
    sessioncombo        ->setEditable(true);
    sessioncombo        ->lineEdit()->setReadOnly(true);
    sessioncombo        ->lineEdit()->setAlignment(Qt::AlignRight);
    choixsessionLay     ->addWidget(lblsession);
    choixsessionLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixsessionLay     ->addWidget(sessioncombo);
    choixsessionLay     ->setSpacing(5);
    choixsessionLay     ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixheureLay    = new QHBoxLayout();
    UpLabel* lblheure = new UpLabel;
    lblheure            ->setText(tr("Heure"));
    QTimeEdit *timeedit = new QTimeEdit();
    choixheureLay       ->addWidget(lblheure);
    choixheureLay       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixheureLay       ->addWidget(timeedit);
    choixheureLay       ->setSpacing(5);
    choixheureLay       ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixinterventionLay    = new QHBoxLayout();
    UpLabel* lblinterv = new UpLabel;
    lblinterv               ->setText(tr("Type d'intervention"));
    UpComboBox *interventioncombo = new UpComboBox();
    interventioncombo       ->setFixedSize(QSize(150,28));
    interventioncombo       ->setEditable(true);
    interventioncombo       ->setModel(m_typeinterventionsmodel);
    interventioncombo       ->setCurrentIndex(-1);
    interventioncombo       ->setInsertPolicy(QComboBox::NoInsert);
    choixinterventionLay    ->addWidget(lblinterv);
    choixinterventionLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixinterventionLay    ->addWidget(interventioncombo);
    choixinterventionLay    ->setSpacing(5);
    choixinterventionLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixanesthLay = new QHBoxLayout();
    UpLabel* lblanesth          = new UpLabel;
    lblanesth                   ->setText(tr("Anesthésie"));
    QComboBox *anesthcombo      = new QComboBox();
    anesthcombo                 ->setFixedSize(QSize(100,28));
    anesthcombo                 ->setEditable(false);
    anesthcombo                 ->addItem(tr("Locale"), "L");
    anesthcombo                 ->addItem(tr("LocoRegionale"), "R");
    anesthcombo                 ->addItem(tr("Générale"), "G");
    anesthcombo                 ->addItem(tr("Sans objet", ""));
    anesthcombo                 ->setCurrentIndex(0);
    choixanesthLay              ->addWidget(lblanesth);
    choixanesthLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixanesthLay              ->addWidget(anesthcombo);
    choixanesthLay              ->setSpacing(5);
    choixanesthLay              ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixcoteLay   = new QHBoxLayout();
    UpLabel* lblcote            = new UpLabel;
    lblcote                     ->setText(tr("Côté"));
    QComboBox *cotecombo        = new QComboBox();
    cotecombo                   ->setFixedSize(QSize(100,28));
    cotecombo                   ->setEditable(false);
    cotecombo                   ->addItem(tr("Droit"), "D");
    cotecombo                   ->addItem(tr("Gauche"), "G");
    cotecombo                   ->addItem(tr("Les 2"), "2");
    cotecombo                   ->addItem(tr("Sans objet", ""));
    choixcoteLay                ->addWidget(lblcote);
    choixcoteLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixcoteLay                ->addWidget(cotecombo);
    choixcoteLay                ->setSpacing(5);
    choixcoteLay                ->setContentsMargins(0,0,0,0);


    //! -----------------------------------------------------------------------------------------     CHOIX DE L'IOL
    wdg_manufacturercombo       = new QComboBox();
    wdg_IOLcombo                = new QComboBox();
    QHBoxLayout *checkIOLLay    = new QHBoxLayout();
    QCheckBox *IOLchk = new QCheckBox(tr("Utiliser un implant"));
    IOLchk          ->setCheckState(Qt::Unchecked);
    checkIOLLay     ->addWidget(IOLchk);
    checkIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkIOLLay     ->setSpacing(5);
    checkIOLLay     ->setContentsMargins(0,0,0,0);

    QWidget *wdg_IOL            = new QWidget();
    QVBoxLayout *box_IOLlay     = new QVBoxLayout();
    box_IOLlay->setSpacing(5);

            //! FABRICANT
    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    wdg_manufacturercombo       ->setFixedSize(QSize(150,28));
    wdg_manufacturercombo       ->setEditable(true);
    ReconstruitListeManufacturers();
    wdg_manufacturercombo       ->setInsertPolicy(QComboBox::NoInsert);
    choixManufacturerIOLLay     ->addWidget(lblManufacturerIOL);
    choixManufacturerIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerIOLLay     ->addWidget(wdg_manufacturercombo);
    choixManufacturerIOLLay     ->setSpacing(5);
    choixManufacturerIOLLay     ->setContentsMargins(0,0,0,0);

            //! MODELE
    QHBoxLayout *choixIOLLay    = new QHBoxLayout();
    UpLabel* lblIOL             = new UpLabel;
    lblIOL                      ->setText(tr("Implant"));
    wdg_IOLcombo                ->setFixedSize(QSize(150,28));
    wdg_IOLcombo                ->setEditable(true);
    wdg_IOLcombo                ->setInsertPolicy(QComboBox::NoInsert);
    choixIOLLay                 ->addWidget(lblIOL);
    choixIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay                 ->addWidget(wdg_IOLcombo);
    choixIOLLay                 ->setSpacing(5);
    choixIOLLay                 ->setContentsMargins(0,0,0,0);

            //! PUISSANCE
    QHBoxLayout *choixPwrIOLLay     = new QHBoxLayout();
    UpLabel* lblPwrIOL              = new UpLabel;
    lblPwrIOL                       ->setText(tr("Puissance"));
    UpDoubleSpinBox *PwrIOLspinbox  = new UpDoubleSpinBox();
    PwrIOLspinbox                   ->setRange(-10.00, 35.00);
    PwrIOLspinbox                   ->setSingleStep(0.50);
    PwrIOLspinbox                   ->setValue(21.00);
    choixPwrIOLLay                  ->addWidget(lblPwrIOL);
    choixPwrIOLLay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixPwrIOLLay                  ->addWidget(PwrIOLspinbox);
    choixPwrIOLLay                  ->setSpacing(5);
    choixPwrIOLLay                  ->setContentsMargins(0,0,0,0);

            //! CYLINDRE
    QHBoxLayout *choixCylIOLLay     = new QHBoxLayout();
    UpLabel* lblCylIOL              = new UpLabel;
    lblCylIOL                       ->setText(tr("Cylindre"));
    UpDoubleSpinBox *CylIOLspinbox  = new UpDoubleSpinBox();
    CylIOLspinbox                   ->setRange(0.00, 10.00);
    CylIOLspinbox                   ->setSingleStep(0.50);
    CylIOLspinbox                   ->setValue(0.00);
    choixCylIOLLay                  ->addWidget(lblCylIOL);
    choixCylIOLLay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixCylIOLLay                  ->addWidget(CylIOLspinbox);
    choixCylIOLLay                  ->setSpacing(5);
    choixCylIOLLay                  ->setContentsMargins(0,0,0,0);

    connect(IOLchk, &QCheckBox::stateChanged, dlg_intervention, [&] { wdg_IOL->setVisible(IOLchk->isChecked()); });

    box_IOLlay      ->insertLayout(0, choixCylIOLLay);
    box_IOLlay      ->insertLayout(0, choixPwrIOLLay);
    box_IOLlay      ->insertLayout(0, choixIOLLay);
    box_IOLlay      ->insertLayout(0, choixManufacturerIOLLay);
    box_IOLlay      ->setContentsMargins(0,0,0,0);
    wdg_IOL         ->setLayout(box_IOLlay);
    wdg_IOL         ->setVisible(false);

    QVBoxLayout *lblObservLay = new QVBoxLayout();
    QHBoxLayout *ObservLay    = new QHBoxLayout();
    UpLabel* lblObserv = new UpLabel;
    lblObserv       ->setText(tr("Remarque"));
    lblObservLay    ->addWidget(lblObserv);
    lblObservLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    UpTextEdit *ObservtextEdit = new UpTextEdit;
    ObservtextEdit  ->setFixedHeight(50);
    ObservLay       ->addLayout(lblObservLay);
    ObservLay       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    ObservLay       ->addWidget(ObservtextEdit);
    ObservLay       ->setSpacing(5);
    ObservLay       ->setContentsMargins(0,0,0,0);

    if (interv != Q_NULLPTR)
    {
        timeedit->setTime(interv->heure());
        QString type = "";
        TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());
        if (typ)
            interventioncombo->setCurrentIndex(interventioncombo->findText(typ->typeintervention()));
        cotecombo->setCurrentIndex(cotecombo->findData(Utils::ConvertCote(interv->cote())));
        anesthcombo->setCurrentIndex(anesthcombo->findData(Intervention::ConvertModeAnesthesie(interv->anesthesie())));
        ObservtextEdit->setText(interv->observation());
        if (interv->idIOL() > 0)
        {
            wdg_IOL->setVisible(true);
            IOLchk->setChecked(true);
            IOL *iol = Datas::I()->iols->getById(interv->idIOL());
            Manufacturer *man = Q_NULLPTR;
            if (iol != Q_NULLPTR)
            {
                man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                wdg_IOLcombo->setCurrentIndex(wdg_IOLcombo->findData(iol->id()));
            }
            if (man != Q_NULLPTR)
                wdg_manufacturercombo->setCurrentIndex(wdg_manufacturercombo->findData(man->id()));
            PwrIOLspinbox->setValue(interv->puissanceIOL());
            CylIOLspinbox->setValue(interv->cylindreIOL());
        }
    }


    dlg_intervention->dlglayout()   ->insertLayout(0, ObservLay);
    dlg_intervention->dlglayout()   ->insertWidget(0, wdg_IOL);
    dlg_intervention->dlglayout()   ->insertLayout(0, checkIOLLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixanesthLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixinterventionLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixcoteLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixheureLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixsessionLay);
    dlg_intervention->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_intervention->dlglayout()   ->setSpacing(5);

    dlg_intervention->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    disconnect(dlg_intervention->CancelButton,   &QPushButton::clicked, dlg_intervention, &UpDialog::reject);
    connect(dlg_intervention->OKButton, &QPushButton::clicked, dlg_intervention, [&]
    {
        if (!timeedit->time().isValid())
        {
            UpMessageBox::Watch(this, tr("Vous n'avez pas spécifié une heure valide"));
            return;
        }
        if (interventioncombo->currentText() == "" || interventioncombo->currentIndex() == -1)
        {
            UpMessageBox::Watch(this, tr("Vous n'avez pas spécifié le type d'intervention"));
            return;
        }
        if (anesthcombo->currentText() == "" || anesthcombo->currentIndex() == -1)
        {
            UpMessageBox::Watch(this, tr("Vous n'avez pas spécifié le type d'anesthésie"));
            return;
        }
        if (IOLchk->isChecked())
        {
            if (Datas::I()->manufacturers->getById(wdg_manufacturercombo->currentData().toInt()) == Q_NULLPTR)
            {
                UpMessageBox::Watch(this, tr("Ce farbicant n'est pas retrouvé"));
                return;
            }
            if (Datas::I()->iols->getById(wdg_IOLcombo->currentData().toInt()) == Q_NULLPTR)
            {
                UpMessageBox::Watch(this, tr("Cet implant n'est pas retrouvé"));
                return;
            }
        }
        if (pat != Q_NULLPTR)
            if (pat->telephone() == "" && pat->portable() == "")
                if (!Patients::veriftelephone(pat))
                    return;
        QTime heure = timeedit->time();
        QStandardItem *itm = m_sessionsmodel->itemFromIndex(sessioncombo->model()->index(sessioncombo->currentIndex(),0));
        int idsession = dynamic_cast<UpStandardItem*>(itm)->item()->id();
        int idpat = pat->id();
        int idtype = 0;
        QString cote = cotecombo->currentData().toString();
        QString anesth = anesthcombo->currentData().toString();
        UpStandardItem *itmitv = dynamic_cast<UpStandardItem*>(m_typeinterventionsmodel->item(interventioncombo->currentIndex()));
        if (itm)
            idtype = itmitv->item()->id();
        QHash<QString, QVariant> listbinds;
        listbinds[CP_HEURE_LIGNPRGOPERATOIRE]    = heure.toString("HH:mm:ss");
        listbinds[CP_IDSESSION_LIGNPRGOPERATOIRE]  = idsession;
        listbinds[CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE]  = idtype;
        listbinds[CP_COTE_LIGNPRGOPERATOIRE]  = cote;
        listbinds[CP_TYPEANESTH_LIGNPRGOPERATOIRE]  = anesth;
        listbinds[CP_OBSERV_LIGNPRGOPERATOIRE]  = ObservtextEdit->toPlainText();
        if (IOLchk->isChecked())
        {
            listbinds[CP_IDIOL_LIGNPRGOPERATOIRE] = m_currentIOL->id();
            listbinds[CP_PWRIOL_LIGNPRGOPERATOIRE] = PwrIOLspinbox->value();
            listbinds[CP_CYLIOL_LIGNPRGOPERATOIRE] = CylIOLspinbox->value();
        }
        if (interv == Q_NULLPTR)                                                                                        //! il s'agit d'une création parce qu'aucune intervention n'a été passée en paramètre de la fonction
        {
            for (int i = 0; i < m_interventionsmodel->rowCount(); ++i)
            {
                UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_interventionsmodel->item(i));
                if(upitem->hasChildren())
                {
                    UpStandardItem * itm = dynamic_cast<UpStandardItem*>(upitem->child(0));
                    if (itm->hasitem())
                    {
                        Intervention *intervention = dynamic_cast<Intervention*>(itm->item());
                        if (intervention)
                        {
                            //qDebug() << intervention->heure() << Datas::I()->patients->getById(intervention->idpatient())->nomcomplet();
                            if (intervention->heure() == heure && intervention->idpatient() == idpat)
                            {
                                UpMessageBox::Watch(this, tr("Cette intervention existe déjà!"));
                                return;
                            }
                        }
                    }
                }
            }
            listbinds[CP_IDPATIENT_LIGNPRGOPERATOIRE]  = idpat;
            Datas::I()->interventions->CreationIntervention(listbinds);
            RemplirTreeInterventions();
        }
        else                                                                                                            //! il s'agit de modifier l'intervention passée en paramètre de la fonction
        {
            int oldidsession = interv->idsession();
            DataBase::I()->UpdateTable(TBL_LIGNESPRGOPERATOIRES, listbinds, "where " CP_ID_LIGNPRGOPERATOIRE " = " + QString::number(interv->id()));
            interv->setheure(heure);
            interv->setidsession(idsession);
            interv->setidtypeintervention(idtype);
            interv->setcote(Utils::ConvertCote(cote));
            interv->setanesthesie(Intervention::ConvertModeAnesthesie(anesth));
            interv->setobservation(ObservtextEdit->toPlainText());
            Datas::I()->interventions->initListebySessionId(idsession);
            if (idsession != oldidsession) // on a changé de session, on change la session active
            {
                QModelIndex idx;
                if (m_sessionsmodel->rowCount() >0)
                {
                    for (int i=0; i<m_sessionsmodel->rowCount(); ++i)
                    {
                        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
                        if (itm->item()->id() == idsession)
                        {
                            idx = itm->index();
                            break;
                        }
                    }
                    wdg_sessionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                    wdg_sessionstreeView->setCurrentIndex(idx);
                }
                AfficheInterventionsSession(idx);
            }
            else
                RemplirTreeInterventions();
        }
        dlg_intervention->close();
    });
    connect(interventioncombo->lineEdit(),      &QLineEdit::editingFinished,    dlg_intervention,   [&] { VerifExistIntervention(verifencours, interventioncombo); });
    connect(wdg_manufacturercombo->lineEdit(),  &QLineEdit::editingFinished,    dlg_intervention,   [&] { VerifExistManufacturer(verifmanufacturerencours); });
    connect(dlg_intervention->CancelButton,     &QPushButton::clicked,          dlg_intervention,   [&]
                                                                                                    {
                                                                                                        interventioncombo->lineEdit()->disconnect();
                                                                                                        dlg_intervention->reject();
                                                                                                    });
    connect(wdg_IOLcombo->lineEdit(),           &QLineEdit::editingFinished,    dlg_intervention,   [&] { VerifExistIOL(verifencours); });
    timeedit->setFocus();
    dlg_intervention->exec();
}

void dlg_programmationinterventions::ImprimeDoc(Patient *pat, Intervention *interv)
{
    if (pat == Q_NULLPTR || interv == Q_NULLPTR)
        return;

    QString nom         = pat->nom();
    QString prenom      = pat->prenom();
    Dlg_Imprs   = new dlg_impressions(pat, interv);
    Dlg_Imprs->setWindowTitle(tr("Préparer un document pour ") + nom + " " + prenom);
    Dlg_Imprs->setWindowIcon(Icons::icLoupe());
    m_docimprime = false;
    if (Dlg_Imprs->exec() > 0)
    {
        User *userEntete = Dlg_Imprs->userentete();
        if (userEntete == Q_NULLPTR)
            return;

        QString     Entete;
        QDate DateDoc = Dlg_Imprs->ui->dateImpressiondateEdit->date();
        //création de l'entête
        QMap<QString,QString> EnteteMap = proc->CalcEnteteImpression(DateDoc, userEntete);
        if (EnteteMap.value("Norm") == "") return;

        bool ALD;
        QString imprimante = "";
        QMap<dlg_impressions::DATASAIMPRIMER, QString> mapdoc;
        foreach (mapdoc, Dlg_Imprs->mapdocsaimprimer())
        {
            bool Prescription           = (mapdoc.find(dlg_impressions::Prescription).value() == "1");
                bool AvecDupli              = (mapdoc.find(dlg_impressions::Dupli).value() == "1");
                bool Administratif          = (mapdoc.find(dlg_impressions::Administratif).value() == "1");
                QString Titre               =  mapdoc.find(dlg_impressions::Titre).value();
                QString TxtDocument         =  mapdoc.find(dlg_impressions::Texte).value();

                bool AvecChoixImprimante    = (mapdoc == Dlg_Imprs->mapdocsaimprimer().first());            // s'il y a plusieurs documents à imprimer on détermine l'imprimante pour le premier et on garde ce choix pour les autres
                bool AvecPrevisu            = proc->ApercuAvantImpression();
                ALD                         = Dlg_Imprs->ui->ALDcheckBox->checkState() == Qt::Checked && Prescription;
                Entete                      = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
                if (Entete == "") return;
                Entete.replace("{{TITRE1}}"        , "");
                Entete.replace("{{TITRE}}"         , "");
                Entete.replace("{{DDN}}"           , "");
                proc                        ->setNomImprimante(imprimante);
                m_docimprime                = Imprimer_Document(pat, userEntete, Titre, Entete, TxtDocument, DateDoc, nom, prenom, Prescription, ALD, AvecPrevisu, AvecDupli, AvecChoixImprimante, Administratif);
                if (!m_docimprime)
                    break;
                imprimante = proc->nomImprimante();
            }
        }
        delete Dlg_Imprs;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Ouvrir la fiche documents ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool   dlg_programmationinterventions::Imprimer_Document(Patient *pat, User * user, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                 bool Prescription, bool ALD, bool AvecPrevisu, bool AvecDupli, bool AvecChoixImprimante, bool Administratif)
{
    QString     Corps, Pied;
    QTextEdit   *Etat_textEdit = new QTextEdit;
    bool        AvecNumPage = false;
    bool        aa;

    Entete.replace("{{PRENOM PATIENT}}", (Prescription? prenom        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? nom.toUpper() : ""));

    //création du pied
    Pied = proc->CalcPiedImpression(user, false, ALD);
    if (Pied == "") return false;

    // creation du corps
    Corps = proc->CalcCorpsImpression(text, ALD);
    if (Corps == "") return false;
    Etat_textEdit->setHtml(Corps);
    if (Etat_textEdit->toPlainText() == "") return false;

    int TailleEnTete = proc->TailleEnTete();
    if (ALD) TailleEnTete = proc->TailleEnTeteALD();
    aa = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                       proc->TaillePieddePage(), TailleEnTete, proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage, AvecChoixImprimante);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        Utils::nettoieHTML(Corps);

        int idpat = 0;
        idpat = pat->id();

        QHash<QString, QVariant> listbinds;
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        listbinds[CP_IDUSER_DOCSEXTERNES]        = user->id();
        listbinds[CP_IDPAT_DOCSEXTERNES]         = idpat;
        listbinds[CP_TYPEDOC_DOCSEXTERNES]       = (Prescription? "Prescription" : "Courrier");
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]   = titre;
        listbinds[CP_TITRE_DOCSEXTERNES]         = titre;
        listbinds[CP_TEXTENTETE_DOCSEXTERNES]    = Entete;
        listbinds[CP_TEXTCORPS_DOCSEXTERNES]     = Corps;
        listbinds[CP_TEXTORIGINE_DOCSEXTERNES]   = text;
        listbinds[CP_TEXTPIED_DOCSEXTERNES]      = Pied;
        listbinds[CP_DATE_DOCSEXTERNES]          = date.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES]    = Datas::I()->users->userconnected()->id();
        listbinds[CP_ALD_DOCSEXTERNES]           = (ALD? "1": QVariant(QVariant::String));
        listbinds[CP_EMISORRECU_DOCSEXTERNES]    = "0";
        listbinds[CP_FORMATDOC_DOCSEXTERNES]     = (Prescription? PRESCRIPTION : (Administratif? COURRIERADMINISTRATIF : COURRIER));
        listbinds[CP_IDLIEU_DOCSEXTERNES]        = Datas::I()->sites->idcurrentsite();
        listbinds[CP_IMPORTANCE_DOCSEXTERNES]    = (Administratif? "0" : "1");
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if(doc != Q_NULLPTR)
            delete doc;
    }
    delete Etat_textEdit;
    return aa;
}

void dlg_programmationinterventions::CreerFicheIntervention()
{
    m_currentintervention = Q_NULLPTR;
    FicheIntervention();
}

void dlg_programmationinterventions::ModifFicheIntervention()
{
    if (m_currentintervention != Q_NULLPTR)
        FicheIntervention();
}

void dlg_programmationinterventions::SupprimeIntervention()
{
    if (m_currentintervention == Q_NULLPTR)
        return;
    QString nomintervention = "";
    TypeIntervention *typ = Datas::I()->typesinterventions->getById(m_currentintervention->idtypeintervention());
    if (typ)
        nomintervention += typ->typeintervention();
    Patient * pat = Datas::I()->patients->getById(m_currentintervention->idpatient());
    if (pat)
        nomintervention += " - " + pat->nom() + " " + pat->prenom();
    QString nomsession = m_currentsession->date().toString("dd-MMM-yy");
    Site* site = Datas::I()->sites->getById(m_currentsession->idlieu());
    if (site != Q_NULLPTR)
        nomsession += " - " + site->nom();
    if (UpMessageBox::Question(this, tr("Voulez-vous supprimer l'intervention"), nomintervention + "\n" + nomsession + " ?") != UpSmallButton::STARTBUTTON)
        return;
    Datas::I()->interventions->SupprimeIntervention(m_currentintervention);
    RemplirTreeInterventions();
}

void dlg_programmationinterventions::VerifExistIntervention(bool &ok, QComboBox *box)
{
    if (ok) return; // c'est de la bidouille, je sais... mais pas trouvé autre chose sinon, le editingFinished est émis 2 fois en cas d'appui sur les touches Enter ou Return du combobox
    ok = true;
    QString txt = box->lineEdit()->text();
    if (m_typeinterventionsmodel->findItems(txt).size() ==0 && txt !="")
    {
        if (UpMessageBox::Question(this, tr("Intervention non référencée!"), tr("Voulez-vous l'enregistrer?")) != UpSmallButton::STARTBUTTON)
            return;
        else
        {
            if (m_currenttypeintervention != Q_NULLPTR)
            {
                delete m_currenttypeintervention;
                m_currenttypeintervention = Q_NULLPTR;
            }
            CreerTypeIntervention(Utils::trimcapitilize(txt));
            box->setModel(m_typeinterventionsmodel);
            if (m_currenttypeintervention != Q_NULLPTR)
            {
                int id = m_currenttypeintervention->id();
                int row = m_typeinterventionsmodel->findItems(QString::number(id), Qt::MatchExactly, 2).at(0)->row();
                box->setCurrentIndex(row);
            }
        }
    }
    ok = false;
};

void dlg_programmationinterventions::VerifFicheIntervention(bool &ok, QTimeEdit *timeedit, QComboBox *box, Patient *pat)
{
    ok = true;
    if (!timeedit->time().isValid())
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas spécifié une heure valide"));
        ok = false;
        return;
    }
    if (box->currentText() == "" || box->currentIndex() == -1)
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas spécifié le type d'intervention"));
        ok = false;
        return;
    }
    if (pat != Q_NULLPTR)
        if (pat->telephone() == "" && pat->portable() == "")
            if (!Patients::veriftelephone(pat))
            {
                ok = false;
                return;
            }
};

void dlg_programmationinterventions::MenuContextuelInterventionsions()
{
    m_ctxtmenuinterventions = new QMenu(this);
    QModelIndex psortindx   = wdg_interventionstreeView->indexAt(wdg_interventionstreeView->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_interventionsmodel->itemFromIndex(psortindx));
    if (upitem == Q_NULLPTR)
    {
        QAction *pAction_CreerSession = m_ctxtmenuinterventions->addAction(tr("Créer une intervention"));
        connect (pAction_CreerSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::CreerFicheIntervention);
    }
    else
    {
        Intervention *interv = dynamic_cast<Intervention*>(upitem->item());
        if (interv == Q_NULLPTR)
        {
            delete m_ctxtmenuinterventions;
            return;
        }
        QAction *pAction_ModifIntervention = m_ctxtmenuinterventions->addAction(tr("Modifier cette intervention"));
        connect (pAction_ModifIntervention,         &QAction::triggered,    this,    &dlg_programmationinterventions::ModifFicheIntervention);
        QAction *pAction_SupprIntervention = m_ctxtmenuinterventions->addAction(tr("Supprimer cette intervention"));
        connect (pAction_SupprIntervention,         &QAction::triggered,    this,    &dlg_programmationinterventions::SupprimeIntervention);
        QAction *pAction_ImprIntervention = m_ctxtmenuinterventions->addAction(tr("Imprimer un document"));
        connect (pAction_ImprIntervention,          &QAction::triggered,    this,    [&] {ImprimeDoc(Datas::I()->patients->getById(interv->idpatient()), interv);});
        if (Datas::I()->users->userconnected()->isMedecin())
        {
            QString txt = (interv->incident() != ""? tr("Modifier le rapport d'incident") : tr ("Enregistrer un incident sur cette intervention"));
            QAction *pAction_IncidentSession = m_ctxtmenuinterventions->addAction(txt);
            connect (pAction_IncidentSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::EnregistreIncidentIntervention);
        }
    }
    // ouvrir le menu
    m_ctxtmenuinterventions->exec(cursor().pos());
    delete m_ctxtmenuinterventions;
}


/*! les types d'intervention ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ReconstruitListeTypeInterventions()
{
    if (m_typeinterventionsmodel == Q_NULLPTR)
        delete m_typeinterventionsmodel;
    m_typeinterventionsmodel = new QStandardItemModel(this);
    foreach (TypeIntervention* typ, *Datas::I()->typesinterventions->typeinterventions())
    {
        QList<QStandardItem *> items;
        QString nomtype = typ->typeintervention();
        UpStandardItem *itemtyp = new UpStandardItem(typ->typeintervention(), typ);
        UpStandardItem *itemccam = new UpStandardItem(typ->codeCCAM(), typ);
        UpStandardItem *itemid = new UpStandardItem(QString::number(typ->id()), typ);
        items << itemtyp << itemccam << itemid;
        m_typeinterventionsmodel->appendRow(items);
    }
    m_typeinterventionsmodel->sort(0, Qt::AscendingOrder);
}

void dlg_programmationinterventions::CreerTypeIntervention(QString txt)
{
    UpDialog            *dlg_typintervention = new UpDialog(this);
    dlg_typintervention->setAttribute(Qt::WA_DeleteOnClose);
    dlg_typintervention->setWindowTitle(tr("créer un type d'intervention"));

    QHBoxLayout *nomLay    = new QHBoxLayout();
    UpLabel* lblnom = new UpLabel;
    lblnom         ->setText(tr("Type"));
    QLineEdit *linenom = new QLineEdit();
    linenom        ->setFixedSize(QSize(200,24));
    linenom        ->setText(txt);
    nomLay    ->addWidget(lblnom);
    nomLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    nomLay    ->addWidget(linenom);
    nomLay    ->setSpacing(5);
    nomLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *ccamLay    = new QHBoxLayout();
    UpLabel* lblccam = new UpLabel;
    lblccam         ->setText(tr("Code CCAM"));
    QLineEdit *lineccam = new QLineEdit();
    lineccam        ->setFixedSize(QSize(100,24));
    ccamLay    ->addWidget(lblccam);
    ccamLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    ccamLay    ->addWidget(lineccam);
    ccamLay    ->setSpacing(5);
    ccamLay    ->setContentsMargins(0,0,0,0);

    dlg_typintervention->dlglayout()   ->insertLayout(0, ccamLay);
    dlg_typintervention->dlglayout()   ->insertLayout(0, nomLay);
    dlg_typintervention->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_typintervention->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_typintervention->OKButton, &QPushButton::clicked, dlg_typintervention, [&]
    {
        if (linenom->text() == "")
            return;
        for (int i = 0; i < m_typeinterventionsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_typeinterventionsmodel->item(i));
            TypeIntervention *typ = dynamic_cast<TypeIntervention*>(upitem->item());
            if (typ->typeintervention() == Utils::trimcapitilize(linenom->text()))
            {
                UpMessageBox::Watch(this, tr("Ce type d'intervention existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_TYPEINTERVENTION_TYPINTERVENTION] = Utils::trimcapitilize(linenom->text());
        listbinds[CP_CODECCAM_TYPINTERVENTION]  = lineccam->text().toUpper();
        if (m_currenttypeintervention != Q_NULLPTR)
            delete m_currenttypeintervention;
        m_currenttypeintervention = Datas::I()->typesinterventions->CreationTypeIntervention(listbinds);
        ReconstruitListeTypeInterventions();
        dlg_typintervention->close();
    });
    dlg_typintervention->exec();
}


/*! les IOLs ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::CreerIOL(QString nomiol)
{
    UpDialog            *dlg_IOL = new UpDialog(this);
    dlg_IOL->setAttribute(Qt::WA_DeleteOnClose);
    dlg_IOL->setWindowTitle(tr("créer un IOL"));

    //! FABRICANT
    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    QComboBox *manufacturercombo = new QComboBox();
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        manufacturercombo->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
        manufacturercombo->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
    }
    if (m_currentmanufacturer)
    {
        manufacturercombo           ->setCurrentIndex(manufacturercombo->findData(m_currentmanufacturer->id()));
        manufacturercombo           ->setEnabled(false);
    }
    choixManufacturerIOLLay     ->addWidget(lblManufacturerIOL);
    choixManufacturerIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerIOLLay     ->addWidget(manufacturercombo);
    choixManufacturerIOLLay     ->setSpacing(5);
    choixManufacturerIOLLay     ->setContentsMargins(0,0,0,0);

    //! MODELE
    QHBoxLayout *choixIOLLay    = new QHBoxLayout();
    UpLabel* lblIOL             = new UpLabel;
    lblIOL                      ->setText(tr("Nom du modèle"));
    QLineEdit *IOLline          = new QLineEdit(nomiol);
    IOLline                     ->setFixedSize(QSize(150,28));
    IOLline                     ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric));
    choixIOLLay                 ->addWidget(lblIOL);
    choixIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay                 ->addWidget(IOLline);
    choixIOLLay                 ->setSpacing(5);
    choixIOLLay                 ->setContentsMargins(0,0,0,0);


    dlg_IOL->dlglayout()   ->insertLayout(0, choixIOLLay);
    dlg_IOL->dlglayout()   ->insertLayout(0, choixManufacturerIOLLay);
    dlg_IOL->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_IOL->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_IOL->OKButton, &QPushButton::clicked, dlg_IOL, [&]
    {
        QString modele = IOLline->text();
        int idmanufacturer = m_currentmanufacturer->id();
        for (int i = 0; i < m_IOLsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_IOLsmodel->item(i));
            IOL *iol = dynamic_cast<IOL*>(upitem->item());
            if (iol->modele() == modele)
            {
                UpMessageBox::Watch(this, tr("Cet implant existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_MODELNAME_IOLS]    = modele;
        listbinds[CP_IDMANUFACTURER_IOLS]  = idmanufacturer;
        listbinds[CP_INACTIF_IOLS]  = QVariant();
        m_currentIOL = Datas::I()->iols->CreationIOL(listbinds);
        int id = (m_currentIOL != Q_NULLPTR? m_currentIOL->id() : 0);
        ReconstruitListeIOLs(idmanufacturer, id);
        dlg_IOL->close();
    });
    dlg_IOL->exec();
}

void dlg_programmationinterventions::ListeIOLs()
{
    Dlg_ListIOLs = new dlg_listeiols(this);
    Dlg_ListIOLs->exec();
    if (Dlg_ListIOLs->listeIOLsmodifiee())
        RemplirTreeInterventions();
    delete Dlg_ListIOLs;
}

void dlg_programmationinterventions::ReconstruitListeIOLs(int idmanufacturer, int idiol)
{
    m_currentIOL = Q_NULLPTR;
    wdg_IOLcombo->disconnect();
    wdg_IOLcombo->clear();
    Datas::I()->iols->initListeByManufacturerId(idmanufacturer);
    if (m_IOLsmodel == Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);
    foreach (IOL* iol, *Datas::I()->iols->iols())
    {
        QList<QStandardItem *> items;
        if (iol->idmanufacturer() == idmanufacturer && iol->isactif())
        {
            UpStandardItem *itemiol = new UpStandardItem(iol->modele(), iol);
            UpStandardItem *itemid = new UpStandardItem(QString::number(iol->id()), iol);
            items << itemiol << itemid;
            m_IOLsmodel->appendRow(items);
        }
    }
    if (m_IOLsmodel->rowCount() > 0)
    {
        if (m_IOLsmodel->rowCount() > 1)
            m_IOLsmodel->sort(0, Qt::AscendingOrder);
        for (int i=0; i< m_IOLsmodel->rowCount(); ++i)
        {
            wdg_IOLcombo->addItem(m_IOLsmodel->item(i,0)->text());              //! le modèle de l'IOL
            wdg_IOLcombo->setItemData(i, m_IOLsmodel->item(i,1)->text());       //! l'id en data
        }
        if (idiol > 0)
            m_currentIOL = Datas::I()->iols->getById(idiol);
        if (m_currentIOL == Q_NULLPTR)
        {
            wdg_IOLcombo->setCurrentIndex(0);
            m_currentIOL = Datas::I()->iols->getById(wdg_IOLcombo->currentData().toInt());
        }
        else
        {
            //qDebug() << "m_currentIOL->id() " << m_currentIOL->id();
            int id = m_currentIOL->id();
            //qDebug() << "wdg_IOLcombo->findData(id) " << wdg_IOLcombo->findData(id);
            int idx = wdg_IOLcombo->findData(id);
            wdg_IOLcombo->setCurrentIndex(idx);
        }
        connect(wdg_IOLcombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [&] (int idx) { m_currentIOL = Datas::I()->iols->getById(wdg_IOLcombo->itemData(idx).toInt()); });
    }
}

void dlg_programmationinterventions::VerifExistIOL(bool &ok)
{
    if (ok) return; // c'est de la bidouille, je sais... mais pas trouvé autre chose sinon, le editingFinished est émis 2 fois en cas d'appui sur les touches Enter ou Return du combobox
    ok = true;
    QString txt = wdg_IOLcombo->lineEdit()->text();
    if (m_IOLsmodel->findItems(txt).size() == 0 && txt !="")
    {
        if (UpMessageBox::Question(this, tr("Implant non référencé!"), tr("Voulez-vous l'enregistrer?")) != UpSmallButton::STARTBUTTON)
            return;
        else
            CreerIOL(txt);
    }
    ok = false;
};

/*! les fabricants ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ChoixManufacturer(int idx)
{
    int id = wdg_manufacturercombo->itemData(idx).toInt();
    m_currentmanufacturer = Datas::I()->manufacturers->getById(id);
    if (m_currentmanufacturer != Q_NULLPTR)
        ReconstruitListeIOLs(m_currentmanufacturer->id());
}

void dlg_programmationinterventions::ListeManufacturers()
{
    if (Datas::I()->manufacturers->manufacturers()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de fournisseur enregistré") );
        Dlg_IdentManufacturer    = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Creation);
        Dlg_IdentManufacturer->exec();
        delete Dlg_IdentManufacturer;
        return;
    }
    Dlg_ListManufacturers = new dlg_listemanufacturers(this);
    Dlg_ListManufacturers->exec();
    if (Dlg_ListManufacturers->listemanufacturersmodifiee())
        AfficheInterventionsSession(wdg_sessionstreeView->currentIndex());
    delete Dlg_ListManufacturers;
}

void dlg_programmationinterventions::ReconstruitListeManufacturers(int idmanufacturer)
{
    m_currentmanufacturer = Q_NULLPTR;
    wdg_manufacturercombo->disconnect();
    wdg_manufacturercombo->clear();
    if (m_manufacturersmodel == Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);
    foreach (Manufacturer *man, *Datas::I()->manufacturers->manufacturers())
        if (man->isactif()) {
            QList<QStandardItem *> items;
            //qDebug() << man->nom() << man->id();
            UpStandardItem *itemman = new UpStandardItem(man->nom(), man);
            UpStandardItem *itemid = new UpStandardItem(QString::number(man->id()), man);
            items << itemman << itemid;
            m_manufacturersmodel->appendRow(items);
        }

    if (m_manufacturersmodel->rowCount() > 0)
    {
        if (m_manufacturersmodel->rowCount() > 1)
            m_manufacturersmodel->sort(0, Qt::AscendingOrder);
        for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
        {
            wdg_manufacturercombo->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
            wdg_manufacturercombo->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
        }
        if (idmanufacturer > 0)
            m_currentmanufacturer = Datas::I()->manufacturers->getById(idmanufacturer);
        if (m_currentmanufacturer == Q_NULLPTR)
        {
            wdg_manufacturercombo->setCurrentIndex(0);
            m_currentmanufacturer = Datas::I()->manufacturers->getById(wdg_manufacturercombo->currentData().toInt());
        }
        else
            wdg_manufacturercombo->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
        ReconstruitListeIOLs(wdg_manufacturercombo->currentData().toInt());
        connect(wdg_manufacturercombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [&] (int idx) { ChoixManufacturer(idx); });
    }
}

void dlg_programmationinterventions::VerifExistManufacturer(bool &ok)
{
    if (ok) return; // c'est de la bidouille, je sais... mais pas trouvé autre chose sinon, le editingFinished est émis 2 fois en cas d'appui sur les touches Enter ou Return du combobox
    ok = true;
    QString txt = wdg_manufacturercombo->lineEdit()->text();
    if (m_manufacturersmodel->findItems(txt).size() == 0 && txt !="")
    {
        if (UpMessageBox::Question(this, tr("Fabricant non référencé!"), tr("Voulez-vous l'enregistrer?")) != UpSmallButton::STARTBUTTON)
            return;
        else
        {
            if (m_currentmanufacturer != Q_NULLPTR)
            {
                delete m_currentmanufacturer;
                m_currentmanufacturer = Q_NULLPTR;
            }
            Dlg_IdentManufacturer    = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Creation);
            Dlg_IdentManufacturer->ui->NomlineEdit->setText(txt.toUpper());
            Dlg_IdentManufacturer->ui->ActifcheckBox->setChecked(true);
            Dlg_IdentManufacturer->ui->ActifcheckBox->setEnabled(false);
            Dlg_IdentManufacturer->OKButton->setEnabled(true);
            if (Dlg_IdentManufacturer->exec() >0)
                m_currentmanufacturer = Datas::I()->manufacturers->CreationManufacturer(Dlg_IdentManufacturer->Listbinds());
            delete Dlg_IdentManufacturer;
            int id = (m_currentmanufacturer != Q_NULLPTR? m_currentmanufacturer->id() : 0);
            ReconstruitListeManufacturers(id);
        }
    }
    ok = false;
};

