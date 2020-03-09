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
    connect (wdg_buttoninterventionframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixInterventionFrame);

    programmLay     ->addWidget(wdg_buttonsessionsframe->widgButtonParent());
    programmLay     ->addWidget(wdg_buttoninterventionframe->widgButtonParent());
    programmLay     ->setSpacing(5);
    programmLay     ->setContentsMargins(0,0,0,0);

    dlglayout()     ->insertLayout(0, programmLay);
    dlglayout()     ->insertLayout(0, choixmedecinLay);

    AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonOK);
    connect(OKButton,     &QPushButton::clicked,    this, [=] {close();});
    setModal(true);
    dlglayout()->setStretch(0,1);
    dlglayout()->setStretch(1,15);
    setFixedWidth(1000);

    foreach (User* usr, *Datas::I()->users->actifs())
        if (usr->isMedecin())
        {
            QList<QStandardItem *> items;
            items << new QStandardItem(usr->login())
                  << new QStandardItem(QString::number(usr->id()));
            m_medecinsmodel.appendRow(items);
        }
    m_medecinsmodel.sort(0, Qt::AscendingOrder);
    for (int i=0; i< m_medecinsmodel.rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(m_medecinsmodel.item(i,0)->text());             //! le login
        wdg_listmedecinscombo->setItemData(i, m_medecinsmodel.item(i,1)->text());      //! l'id en data
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
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel.itemFromIndex(idx));
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
    m_sessionsmodel.clear();
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
        m_sessionsmodel.appendRow(items);
    }
    m_sessionsmodel.sort(1, Qt::AscendingOrder);
    m_sessionsmodel.takeColumn(1);
    wdg_sessionstreeView->setModel(&m_sessionsmodel);
    m_sessionsmodel.setHeaderData(0, Qt::Horizontal, tr("Sessions"));
    wdg_sessionstreeView->expandAll();
    QModelIndex idx;
    if (m_sessionsmodel.rowCount() >0)
    {
        if (session == Q_NULLPTR)
            idx = m_sessionsmodel.item(m_sessionsmodel.rowCount()-1)->index();        //! l'index de ce dernier item
        else for (int i=0; i<m_sessionsmodel.rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel.item(i));
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
    connect(dlg_session->OKButton, &QPushButton::clicked, dlg_session, [=]
    {
        QDate date = dateedit->date();
        int idsite = sitecombo->currentData().toInt();
        for (int i = 0; i < m_sessionsmodel.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel.item(i));
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
    connect(dlg_session->OKButton, &QPushButton::clicked, dlg_session, [=]
    {
        QDate date = dateedit->date();
        int idsite = sitecombo->currentData().toInt();
        for (int i = 0; i < m_sessionsmodel.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel.item(i));
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
        SupprimeIntervention(interv);
    }
    Datas::I()->sessionsoperatoires->SupprimeSessionOperatoire(m_currentsession);
    RemplirTreeSessions();
}

void dlg_programmationinterventions::MenuContextuelSessions()
{
    m_ctxtmenusessions = new QMenu(this);
    QModelIndex psortindx   = wdg_sessionstreeView->indexAt(wdg_sessionstreeView->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel.itemFromIndex(psortindx));
    if (upitem == Q_NULLPTR)
    {
        QAction *pAction_CreerSession = m_ctxtmenusessions->addAction(tr("Créer une session"));
        connect (pAction_CreerSession,        &QAction::triggered,    this,    [=] {CreerSession();});
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
        connect (pAction_ModifSession,        &QAction::triggered,    this,    [=] {EditSession();});
        QAction *pAction_SupprSession = m_ctxtmenusessions->addAction(tr("Supprimer la session"));
        connect (pAction_SupprSession,        &QAction::triggered,    this,    [=] {SupprimeSession();});
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
        CreerFicheIntervention(m_currentintervention);
        break;
    case WidgetButtonFrame::Moins:
        SupprimeIntervention(m_currentintervention);
        break;
    }
}

void dlg_programmationinterventions::ChoixIntervention(QModelIndex idx)
{
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_interventionsmodel.itemFromIndex(idx));
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
    m_interventionsmodel.clear();
    m_currentintervention = Q_NULLPTR;
    QStandardItem * rootNodeDate = m_interventionsmodel.invisibleRootItem();
    QList<QTime> listheures;

    foreach (Intervention *interv, *Datas::I()->interventions->interventions())
    {
        // créations des entêtes par heure
        if (!listheures.contains(interv->heure()))
            listheures << interv->heure();
    }
    UpStandardItem *itempat = Q_NULLPTR;
    UpStandardItem *itemobs = Q_NULLPTR;
    UpStandardItem *itemddn = Q_NULLPTR;
    UpStandardItem *itemtel = Q_NULLPTR;
    UpStandardItem *itemtyp = Q_NULLPTR;
    UpStandardItem *itemane = Q_NULLPTR;
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
        QList<QStandardItem *> listitemsheure = m_interventionsmodel.findItems(heure);
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

            TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());                              //! type d'intervention
            if (typ)
            {
                QString typinterv = typ->typeintervention().toUpper();
                if (interv->cote() != Utils::NoLoSo)
                    typinterv += " - " + tr("Côté") + " " +  Utils::TraduitCote(interv->cote()).toLower();
                itemtyp = new UpStandardItem("\t" + typinterv, interv);
                itemtyp ->setForeground(QBrush(QColor(Qt::darkBlue)));
                itemtyp ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemtyp << new QStandardItem(QString::number(a) + "b"));
            }
            if (interv->anesthesie() != Intervention::NoLoSo)                                                                           //! type d'anesthésie
            {
                QString anesth = "";
                switch (interv->anesthesie()) {
                case Intervention::Locale:          anesth = tr("Anesthésie locale");           break;
                case Intervention::LocoRegionale:   anesth = tr("Anesthésie locoregionale");    break;
                case Intervention::Generale:        anesth = tr("Anesthésie générale");         break;
                default: break;
                }
                itemane = new UpStandardItem("\t" + anesth, interv);
                if (interv->anesthesie() == Intervention::Generale)
                    itemane ->setForeground(QBrush(QColor(Qt::red)));
                else
                    itemane ->setForeground(QBrush(QColor(Qt::darkGray)));
                itemane ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemane << new QStandardItem(QString::number(a) + "c"));
            }
            QString sexeddn = (pat->sexe() == "M"? tr("Né le") : tr("Née le"))                                                          //! date de naissance - sexe
                    + " " + pat->datedenaissance().toString("dd-MM-yyyy")
                    + " - " + Utils::CalculAge(pat->datedenaissance())["toString"].toString();
            itemddn = new UpStandardItem("\t" + sexeddn, interv);
            itemddn ->setForeground(QBrush(QColor(Qt::darkGray)));
            itemddn ->setEditable(false);
            listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemddn << new QStandardItem(QString::number(a) + "d"));

            if (pat->telephone() != "" || pat->portable() != "")                                                                        //! telephone
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
                itemtel = new UpStandardItem("\t" + tel, interv);
                itemtel ->setForeground(QBrush(QColor(Qt::darkGray)));
                itemtel ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemtel << new QStandardItem(QString::number(a) + "e"));
            }

            if (interv->observation() != "")                                                                                            //! observation
            {
                itemobs = new UpStandardItem("\t" + tr("Remarque") + " : " + interv->observation(), interv);
                itemobs ->setForeground(QBrush(QColor(Qt::red)));
                itemobs ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemobs << new QStandardItem(QString::number(a) + "f"));
            }
        }
        listitemsheure.at(0)->sortChildren(1);
        ++a;
    }
    wdg_interventionstreeView->setModel(&m_interventionsmodel);
    m_interventionsmodel.setHeaderData(0, Qt::Horizontal, tr("Interventions"));
    m_interventionsmodel.setHeaderData(1, Qt::Horizontal, "");
    m_interventionsmodel.setHeaderData(2, Qt::Horizontal, "");
    wdg_interventionstreeView->expandAll();
    wdg_interventionstreeView   ->setColumnWidth(0,340);
    wdg_interventionstreeView   ->header()->setSectionResizeMode(QHeaderView::Fixed);
    wdg_interventionstreeView   ->setSortingEnabled(false);
    connect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    QModelIndex idx;
    if (m_interventionsmodel.rowCount() >0)
    {
        if (intervention == Q_NULLPTR)
            idx = m_interventionsmodel.item(m_interventionsmodel.rowCount()-1)->index();        //! l'index du dernier item
        else for (int i=0; i<m_interventionsmodel.rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_interventionsmodel.item(i));
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

void dlg_programmationinterventions::CreerFicheIntervention(Intervention* interv)
{
    bool verifencours = false;
    UpDialog *dlg_intervention = new UpDialog(this);
    Patient *pat = (interv == Q_NULLPTR? m_currentchirpatient : Datas::I()->patients->getById(interv->idpatient()));
    if (pat != Q_NULLPTR)
            dlg_intervention->setWindowTitle(pat->prenom() + " " + pat->nom());

    QHBoxLayout *choixsessionLay    = new QHBoxLayout();
    UpLabel* lblsession = new UpLabel;
    lblsession          ->setText(tr("Session"));
    UpComboBox *sessioncombo = new UpComboBox();
    sessioncombo        ->setModel(&m_sessionsmodel);
    for (int i=0; i< m_sessionsmodel.rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel.item(i,0));
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
    interventioncombo       ->setModel(&m_typeinterventionsmodel);
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

    QHBoxLayout *choixcoteLay    = new QHBoxLayout();
    UpLabel* lblcote        = new UpLabel;
    lblcote                 ->setText(tr("Côté"));
    QComboBox *cotecombo    = new QComboBox();
    cotecombo               ->setFixedSize(QSize(100,28));
    cotecombo               ->setEditable(false);
    cotecombo               ->addItem(tr("Droit"), "D");
    cotecombo               ->addItem(tr("Gauche"), "G");
    cotecombo               ->addItem(tr("Les 2"), "2");
    cotecombo               ->addItem(tr("Sans objet", ""));
    choixcoteLay            ->addWidget(lblcote);
    choixcoteLay            ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixcoteLay            ->addWidget(cotecombo);
    choixcoteLay            ->setSpacing(5);
    choixcoteLay            ->setContentsMargins(0,0,0,0);

    QHBoxLayout *checkIOLLay    = new QHBoxLayout();
    QCheckBox *IOLchk = new QCheckBox(tr("Utiliser un implant"));
    IOLchk          ->setCheckState(Qt::Unchecked);
    checkIOLLay     ->addWidget(IOLchk);
    checkIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkIOLLay     ->setSpacing(5);
    checkIOLLay     ->setContentsMargins(0,0,0,0);

    QWidget *wdg_IOL         = new QWidget();
    QVBoxLayout *box_IOLlay  = new QVBoxLayout();
    box_IOLlay->setSpacing(5);

    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    QComboBox *ManufacturerIOLcombo = new QComboBox();
    choixManufacturerIOLLay     ->addWidget(lblManufacturerIOL);
    choixManufacturerIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerIOLLay     ->addWidget(ManufacturerIOLcombo);
    choixManufacturerIOLLay     ->setSpacing(5);
    choixManufacturerIOLLay     ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixIOLLay    = new QHBoxLayout();
    UpLabel* lblIOL = new UpLabel;
    lblIOL          ->setText(tr("Implant"));
    QComboBox *IOLcombo = new QComboBox();
    choixIOLLay     ->addWidget(lblIOL);
    choixIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay     ->addWidget(IOLcombo);
    choixIOLLay     ->setSpacing(5);
    choixIOLLay     ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixPwrIOLLay    = new QHBoxLayout();
    UpLabel* lblPwrIOL = new UpLabel;
    lblPwrIOL       ->setText(tr("Puissance"));
    UpDoubleSpinBox *PwrIOLspinbox = new UpDoubleSpinBox();
    PwrIOLspinbox   ->setRange(-10.00, 35.00);
    PwrIOLspinbox   ->setSingleStep(0.50);
    PwrIOLspinbox   ->setValue(21.00);
    choixPwrIOLLay  ->addWidget(lblPwrIOL);
    choixPwrIOLLay  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixPwrIOLLay  ->addWidget(PwrIOLspinbox);
    choixPwrIOLLay  ->setSpacing(5);
    choixPwrIOLLay  ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixCylIOLLay    = new QHBoxLayout();
    UpLabel* lblCylIOL = new UpLabel;
    lblCylIOL       ->setText(tr("Cylindre"));
    UpDoubleSpinBox *CylIOLspinbox = new UpDoubleSpinBox();
    CylIOLspinbox   ->setRange(0.00, 10.00);
    CylIOLspinbox   ->setSingleStep(0.50);
    CylIOLspinbox   ->setValue(0.00);
    choixCylIOLLay  ->addWidget(lblCylIOL);
    choixCylIOLLay  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixCylIOLLay  ->addWidget(CylIOLspinbox);
    choixCylIOLLay  ->setSpacing(5);
    choixCylIOLLay  ->setContentsMargins(0,0,0,0);

    connect(IOLchk, &QCheckBox::stateChanged, dlg_intervention, [=]
    {
        wdg_IOL->setVisible(IOLchk->isChecked());
    });

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
    connect(dlg_intervention->OKButton, &QPushButton::clicked, dlg_intervention, [=]
    {
        bool ok;
        VerifFicheIntervention(ok, timeedit, interventioncombo, pat);
        if (!ok)
            return;
        QTime heure = timeedit->time();
        QStandardItem *itm = m_sessionsmodel.itemFromIndex(sessioncombo->model()->index(sessioncombo->currentIndex(),0));
        int idsession = dynamic_cast<UpStandardItem*>(itm)->item()->id();
        int idpat = pat->id();
        int idtype = 0;
        QString cote = cotecombo->currentData().toString();
        QString anesth = anesthcombo->currentData().toString();
        UpStandardItem *itmitv = dynamic_cast<UpStandardItem*>(m_typeinterventionsmodel.item(interventioncombo->currentIndex()));
        if (itm)
            idtype = itmitv->item()->id();
        QHash<QString, QVariant> listbinds;
        listbinds[CP_HEURE_LIGNPRGOPERATOIRE]    = heure.toString("HH:mm:ss");
        listbinds[CP_IDSESSION_LIGNPRGOPERATOIRE]  = idsession;
        listbinds[CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE]  = idtype;
        listbinds[CP_COTE_LIGNPRGOPERATOIRE]  = cote;
        listbinds[CP_TYPEANESTH_LIGNPRGOPERATOIRE]  = anesth;
        listbinds[CP_OBSERV_LIGNPRGOPERATOIRE]  = ObservtextEdit->toPlainText();
        if (interv == Q_NULLPTR)                                                                                        //! il s'agit d'une création parce qu'aucune intervention n'a été passée en paramètre de la fonction
        {
            for (int i = 0; i < m_interventionsmodel.rowCount(); ++i)
            {
                UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_interventionsmodel.item(i));
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
                if (m_sessionsmodel.rowCount() >0)
                {
                    for (int i=0; i<m_sessionsmodel.rowCount(); ++i)
                    {
                        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel.item(i));
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
    connect(interventioncombo->lineEdit(), &QLineEdit::editingFinished, dlg_intervention, [&] { VerifExistIntervention(verifencours, interventioncombo); });
    connect(dlg_intervention->CancelButton, &QPushButton::clicked, dlg_intervention, [=]
    {
        interventioncombo->lineEdit()->disconnect();
        dlg_intervention->reject();
    });
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
        qDebug() << Datas::I()->patients->getById(idpat)->nomcomplet();

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

void dlg_programmationinterventions::SupprimeIntervention(Intervention* intervention)
{
    if (intervention == Q_NULLPTR)
        return;
    QString nomintervention = "";
    TypeIntervention *typ = Datas::I()->typesinterventions->getById(intervention->idtypeintervention());
    if (typ)
        nomintervention += typ->typeintervention();
    Patient * pat = Datas::I()->patients->getById(intervention->idpatient());
    if (pat)
        nomintervention += " - " + pat->nom() + " " + pat->prenom();
    QString nomsession = m_currentsession->date().toString("dd-MMM-yy");
    Site* site = Datas::I()->sites->getById(m_currentsession->idlieu());
    if (site != Q_NULLPTR)
        nomsession += " - " + site->nom();
    if (UpMessageBox::Question(this, tr("Voulez-vous supprimer l'intervention"), nomintervention + "\n" + nomsession + " ?") != UpSmallButton::STARTBUTTON)
        return;
    Datas::I()->interventions->SupprimeIntervention(intervention);
    RemplirTreeInterventions();
}

void dlg_programmationinterventions::VerifExistIntervention(bool &ok, QComboBox *box)
{
    if (ok) return; // c'est de la bidouille, je sais... mais pas trouvé autre chose sinon, le editingFinished est émis 2 fois en cas d'appui sur les touches Enter ou Return du combobox
    ok = true;
    QString txt = box->lineEdit()->text();
    if (m_typeinterventionsmodel.findItems(txt).size() ==0 && txt !="")
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
            box->setModel(&m_typeinterventionsmodel);
            if (m_currenttypeintervention != Q_NULLPTR)
            {
                int id = m_currenttypeintervention->id();
                int row = m_typeinterventionsmodel.findItems(QString::number(id), Qt::MatchExactly, 2).at(0)->row();
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
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_interventionsmodel.itemFromIndex(psortindx));
    if (upitem == Q_NULLPTR)
    {
        QAction *pAction_CreerSession = m_ctxtmenuinterventions->addAction(tr("Créer une intervention"));
        connect (pAction_CreerSession,        &QAction::triggered,    this,    [=] {CreerFicheIntervention();});
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
        connect (pAction_ModifIntervention,        &QAction::triggered,    this,    [=] {CreerFicheIntervention(interv);});
        QAction *pAction_SupprIntervention = m_ctxtmenuinterventions->addAction(tr("Supprimer cette intervention"));
        connect (pAction_SupprIntervention,        &QAction::triggered,    this,    [=] {SupprimeIntervention(interv);});
        QAction *pAction_ImprIntervention = m_ctxtmenuinterventions->addAction(tr("Imprimer un document"));
        connect (pAction_ImprIntervention,        &QAction::triggered,    this,    [=] {ImprimeDoc(Datas::I()->patients->getById(interv->idpatient()), interv);});
    }
    // ouvrir le menu
    m_ctxtmenuinterventions->exec(cursor().pos());
    delete m_ctxtmenuinterventions;
}


/*! les types d'intervention ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ReconstruitListeTypeInterventions()
{
    m_typeinterventionsmodel.clear();
    foreach (TypeIntervention* typ, *Datas::I()->typesinterventions->typeinterventions())
    {
        QList<QStandardItem *> items;
        QString nomtype = typ->typeintervention();
        UpStandardItem *itemtyp = new UpStandardItem(typ->typeintervention(), typ);
        UpStandardItem *itemccam = new UpStandardItem(typ->codeCCAM(), typ);
        UpStandardItem *itemid = new UpStandardItem(QString::number(typ->id()), typ);
        items << itemtyp << itemccam << itemid;
        m_typeinterventionsmodel.appendRow(items);
    }
    m_typeinterventionsmodel.sort(0, Qt::AscendingOrder);
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
    connect(dlg_typintervention->OKButton, &QPushButton::clicked, dlg_typintervention, [=]
    {
        if (linenom->text() == "")
            return;
        for (int i = 0; i < m_typeinterventionsmodel.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_typeinterventionsmodel.item(i));
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

void dlg_programmationinterventions::AfficheChoixIOL(int state)
{

}

