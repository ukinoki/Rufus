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

    wdg_interventionstreeView  ->setFocusPolicy(Qt::NoFocus);
    wdg_interventionstreeView  ->setPalette(QPalette(Qt::white));
    wdg_interventionstreeView  ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_interventionstreeView  ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_interventionstreeView  ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_interventionstreeView  ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_interventionstreeView  ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    wdg_sessionstreeView->setMaximumWidth(340);
    wdg_sessionstreeView->setColumnWidth(0,340);
    wdg_sessionstreeView->setFocusPolicy(Qt::StrongFocus);
    wdg_sessionstreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_sessionstreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_sessionstreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_sessionstreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_sessionstreeView->setSelectionMode(QAbstractItemView::SingleSelection);

    UpLabel* lblmedecins = new UpLabel;
    lblmedecins->setText(tr("Programme opératoire de "));
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
            m_medecins.appendRow(items);
        }
    m_medecins.sort(0, Qt::AscendingOrder);
    for (int i=0; i< m_medecins.rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(m_medecins.item(i,0)->text());             //! le login
        wdg_listmedecinscombo->setItemData(i, m_medecins.item(i,1)->text());      //! l'id en data
    }
    if (Datas::I()->users->userconnected()->isMedecin())
    {
        wdg_listmedecinscombo->setCurrentIndex(wdg_listmedecinscombo->findData(Datas::I()->users->userconnected()->id()));
        wdg_listmedecinscombo->setEnabled(false);
        ChoixMedecin(Datas::I()->users->userconnected()->id());
    }
    else
        connect(wdg_listmedecinscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &dlg_programmationinterventions::ChoixMedecin);
    connect(wdg_sessionstreeView,       &QWidget::customContextMenuRequested,                   this, &dlg_programmationinterventions::MenuContextuelSessions);
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

void dlg_programmationinterventions::ChoixSession(QItemSelection select)
{
    QModelIndexList listindex = select.indexes();
    if (listindex.size() == 0)
        return;
    QModelIndex mdl = listindex.at(0);
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(mdl));
    if (upitem == Q_NULLPTR)
        return;
    m_currentsession = dynamic_cast<SessionOperatoire*>(upitem->item());
    wdg_buttoninterventionframe->wdg_moinsBouton->setEnabled(false);
    wdg_buttoninterventionframe->wdg_modifBouton->setEnabled(false);
    if (m_currentsession == Q_NULLPTR)
    {
        wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(false);
        wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(false);
        wdg_buttoninterventionframe->wdg_plusBouton->setEnabled(false);
        return;
    }
    //qDebug() << m_currentsession->date() << Datas::I()->sites->getById(m_currentsession->idlieu())->nom();
    Datas::I()->interventions->initListebySessionId(m_currentsession->id());
    RemplirTreeInterventions();
}

void dlg_programmationinterventions::RemplirTreeSessions(SessionOperatoire* session)
{
    disconnect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &dlg_programmationinterventions::ChoixSession);
    m_sessions.clear();
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
        m_sessions.appendRow(items);
    }
    m_sessions.sort(1, Qt::AscendingOrder);
    m_sessions.takeColumn(1);
    wdg_sessionstreeView->setModel(&m_sessions);
    m_sessions.setHeaderData(0, Qt::Horizontal, tr("Sessions"));
    wdg_sessionstreeView->expandAll();
    connect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &dlg_programmationinterventions::ChoixSession);
    if (m_sessions.rowCount() >0)
    {
        QModelIndex idx;
        if (session == Q_NULLPTR)
            idx = m_sessions.item(m_sessions.rowCount()-1)->index();        //! l'index de ce dernier item
        else for (int i=0; i<m_sessions.rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessions.item(i));
            if (itm->item() == session)
            {
                idx = itm->index();
                break;
            }
        }
        wdg_sessionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        wdg_sessionstreeView->setCurrentIndex(idx);
    }
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
        for (int i = 0; i < m_sessions.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.item(i));
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
        Datas::I()->sessionsoperatoires->CreationSessionOperatoire(listbinds);
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
        for (int i = 0; i < m_sessions.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.item(i));
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
    Datas::I()->sessionsoperatoires->SupprimeSessionOperatoire(m_currentsession);
    RemplirTreeSessions();
}

void dlg_programmationinterventions::MenuContextuelSessions()
{
    m_ctxtmenusessions = new QMenu(this);
    QModelIndex psortindx   = wdg_sessionstreeView->indexAt(wdg_sessionstreeView->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(psortindx));
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
        CreerIntervention();
        break;
    case WidgetButtonFrame::Modifier:
        proc->ab();
        break;
    case WidgetButtonFrame::Moins:
        SupprimeIntervention();
        break;
    }
}

void dlg_programmationinterventions::ChoixIntervention(QItemSelection select)
{
    QModelIndexList listindex = select.indexes();
    if (listindex.size() == 0)
        return;
    QModelIndex mdl = listindex.at(0);
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_interventions.itemFromIndex(mdl));
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
    disconnect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    m_interventions.clear();
    m_currentintervention = Q_NULLPTR;
    foreach (Intervention* interv, *Datas::I()->interventions->interventions())
    {
        QString nompatient = "";
        Patient * pat = Datas::I()->patients->getById(interv->idpatient(), Item::LoadDetails);
        if (!pat)
            return;
        nompatient = pat->nom().toUpper() + " " + pat->prenom();
        UpStandardItem *item    = new UpStandardItem(interv->heure().toString("HH:mm") + " - " + nompatient, interv);
        item->setForeground(QBrush(QColor(Qt::blue)));
        UpStandardItem *itemddn = new UpStandardItem((pat->sexe() == "M"? tr("Né le") : tr("Née le"))
                                                     + " " + pat->datedenaissance().toString("dd-MM-yyyy")
                                                     + " - " + Utils::CalculAge(pat->datedenaissance())["toString"].toString(),
                                                     interv);        
        itemddn->setForeground(QBrush(QColor(Qt::gray)));
        item->appendRow(itemddn);
        if (pat->telephone() != "" || pat->portable() != "")
        {
            QString tel = tr("Tel") + " " + pat->telephone();
            if (tel != "" && pat->portable() != "")
                tel += " - " + pat->portable();
            else
                tel = pat->portable();
            UpStandardItem *itemtel = new UpStandardItem(tel, interv);
            itemtel->setForeground(QBrush(QColor(Qt::gray)));
            item->appendRow(itemtel);
        }
        TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());
        if (typ)
        {
            QString typinterv = typ->typeintervention().toUpper();
            if (interv->cote() != Utils::NoLoSo)
                typinterv += " - " + tr("Côté") + " " +  Utils::TraduitCote(interv->cote()).toLower();
            UpStandardItem *itemtyp = new UpStandardItem(typinterv, interv);
            itemtyp->setForeground(QBrush(QColor(Qt::darkBlue)));
            item->appendRow(itemtyp);
        }
        m_interventions.appendRow(item);
    }
    m_interventions.sort(0, Qt::AscendingOrder);
    wdg_interventionstreeView->setModel(&m_interventions);
    m_interventions.setHeaderData(0, Qt::Horizontal, tr("interventions"));
    wdg_interventionstreeView->expandAll();
    connect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    if (m_interventions.rowCount() >0)
    {
        QModelIndex idx;
        if (intervention == Q_NULLPTR)
            idx = m_interventions.item(m_interventions.rowCount()-1)->index();        //! l'index du dernier item
        else for (int i=0; i<m_interventions.rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_interventions.item(i));
            if (itm->item() == intervention)
            {
                idx = itm->index();
                break;
            }
        }
        wdg_interventionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        wdg_interventionstreeView->setCurrentIndex(idx);
    }
    connect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &dlg_programmationinterventions::ChoixIntervention);
}

void dlg_programmationinterventions::CreerIntervention()
{
    UpDialog *dlg_intervention = new UpDialog(this);
    dlg_intervention->setWindowTitle(tr("programmer une intervention pour ") + m_currentchirpatient->prenom() + " " + m_currentchirpatient->nom());

    QHBoxLayout *choixsessionLay    = new QHBoxLayout();
    UpLabel* lblsession = new UpLabel;
    lblsession          ->setText(tr("Session"));
    UpComboBox *sessioncombo = new UpComboBox();
    sessioncombo        ->setModel(&m_sessions);
    for (int i=0; i< m_sessions.rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessions.item(i,0));
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
    QComboBox *interventioncombo = new QComboBox();
    interventioncombo       ->setFixedSize(QSize(250,28));
    interventioncombo       ->setEditable(true);
    interventioncombo       ->setModel(&m_typeinterventions);
    interventioncombo       ->setCompleter(new QCompleter(&m_typeinterventions));
    choixinterventionLay    ->addWidget(lblinterv);
    choixinterventionLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixinterventionLay    ->addWidget(interventioncombo);
    choixinterventionLay    ->setSpacing(5);
    choixinterventionLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixcoteLay    = new QHBoxLayout();
    UpLabel* lblcote        = new UpLabel;
    lblcote                 ->setText(tr("Côté"));
    QComboBox *cotecombo    = new QComboBox();
    cotecombo               ->setFixedSize(QSize(100,28));
    cotecombo               ->setEditable(true);
    cotecombo               ->addItem(tr("Droit"), "D");
    cotecombo               ->addItem(tr("Gauche"), "G");
    cotecombo               ->addItem(tr("Les 2"), "2");
    cotecombo               ->addItem(tr("Sans objet"));
    cotecombo               ->setCompleter(new QCompleter(&m_typeinterventions));
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

    wdg_IOL         = new QWidget();
    QVBoxLayout *box_IOLlay  = new QVBoxLayout();
    box_IOLlay->setSpacing(0);

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

    connect(IOLchk, &QCheckBox::stateChanged, this, &dlg_programmationinterventions::AfficheChoixIOL);

    box_IOLlay      ->insertLayout(0, choixCylIOLLay);
    box_IOLlay      ->insertLayout(0, choixPwrIOLLay);
    box_IOLlay      ->insertLayout(0, choixIOLLay);
    wdg_IOL         ->setLayout(box_IOLlay);
    wdg_IOL         ->setVisible(false);

    dlg_intervention->dlglayout()   ->insertWidget(0, wdg_IOL);
    dlg_intervention->dlglayout()   ->insertLayout(0, checkIOLLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixinterventionLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixcoteLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixheureLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixsessionLay);
    dlg_intervention->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_intervention->dlglayout()   ->setSpacing(5);
    dlg_intervention->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_intervention->OKButton, &QPushButton::clicked, dlg_intervention, [=]
    {
        QTime heure = timeedit->time();
        QStandardItem *itm = m_sessions.itemFromIndex(sessioncombo->model()->index(sessioncombo->currentIndex(),0));
        int idsession = dynamic_cast<UpStandardItem*>(itm)->item()->id();
        int idpat = m_currentchirpatient->id();
        int idtype = 0;
        QString cote = cotecombo->currentData().toString();
        UpStandardItem *itmitv = dynamic_cast<UpStandardItem*>(m_typeinterventions.item(interventioncombo->currentIndex()));
        if (itm)
            idtype = itmitv->item()->id();
        for (int i = 0; i < m_interventions.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_interventions.item(i));
            Intervention *intervention = dynamic_cast<Intervention*>(upitem->item());
            if (intervention->heure() == heure && intervention->idpatient() == idpat)
            {
                UpMessageBox::Watch(this, tr("Cette intervention existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_HEURE_LIGNPRGOPERATOIRE]    = heure.toString("HH:mm:ss");
        listbinds[CP_IDPATIENT_LIGNPRGOPERATOIRE]  = idpat;
        listbinds[CP_IDSESSION_LIGNPRGOPERATOIRE]  = idsession;
        listbinds[CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE]  = idtype;
        listbinds[CP_COTE_LIGNPRGOPERATOIRE]  = cote;
        Datas::I()->interventions->CreationIntervention(listbinds);
        RemplirTreeInterventions();
        dlg_intervention->close();
    });
    connect(interventioncombo->lineEdit(), &QLineEdit::editingFinished, dlg_intervention, [=]
    {
        QString txt = interventioncombo->lineEdit()->text();
        if (m_typeinterventions.findItems(txt).size()==0)
        {
            if (UpMessageBox::Question(this, tr("Intervention non référencée!"), tr("Voulez-vous l'enregistrer?")) != UpSmallButton::STARTBUTTON)
                return;
        }
    });
    dlg_intervention->exec();
}

void dlg_programmationinterventions::EditIntervention()
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
        for (int i = 0; i < m_sessions.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.item(i));
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

void dlg_programmationinterventions::MenuContextuelInterventionsions()
{
    m_ctxtmenusessions = new QMenu(this);
    QModelIndex psortindx   = wdg_sessionstreeView->indexAt(wdg_sessionstreeView->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(psortindx));
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


/*! les types d'intervention ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ReconstruitListeTypeInterventions()
{
    m_typeinterventions.clear();
    foreach (TypeIntervention* typ, *Datas::I()->typesinterventions->typeinterventions())
    {
        QList<QStandardItem *> items;
        QString nomtype = typ->typeintervention();
        UpStandardItem *itemtyp = new UpStandardItem(typ->typeintervention(), typ);
        UpStandardItem *itemccam = new UpStandardItem(typ->codeCCAM(), typ);
        items << itemtyp << itemccam;
        m_typeinterventions.appendRow(items);
    }
    m_typeinterventions.sort(0, Qt::AscendingOrder);
}

void dlg_programmationinterventions::CreerTypeIntervention()
{
    UpDialog            *dlg_typintervention = new UpDialog(this);
    dlg_typintervention->setAttribute(Qt::WA_DeleteOnClose);
    dlg_typintervention->setWindowTitle(tr("créer un type d'intervention"));

    QHBoxLayout *nomLay    = new QHBoxLayout();
    UpLabel* lblnom = new UpLabel;
    lblnom         ->setText(tr("Type"));
    QLineEdit *linenom = new QLineEdit();
    linenom        ->setFixedSize(QSize(200,24));
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
        for (int i = 0; i < m_typeinterventions.rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_typeinterventions.item(i));
            TypeIntervention *typ = dynamic_cast<TypeIntervention*>(upitem->item());
            if (typ->typeintervention() == Utils::trimcapitilize(linenom->text()))
            {
                UpMessageBox::Watch(this, tr("Ce type d'intervention existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_TYPEINTERVENTION_TYPINTERVENTION]    = Utils::trimcapitilize(linenom->text());
        listbinds[CP_CODECCAM_TYPINTERVENTION]  = lineccam->text().toUpper();
        listbinds[CP_IDUSER_SESSIONOPERATOIRE]  = m_currentchiruser->id();
        Datas::I()->typesinterventions->CreationTypeIntervention(listbinds);
        ReconstruitListeTypeInterventions();
        dlg_typintervention->close();
    });
    dlg_typintervention->exec();
}


/*! les IOLs ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::AfficheChoixIOL(int state)
{
    wdg_IOL->setVisible(state == Qt::Checked);
}

