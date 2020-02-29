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
    m_currentpatient = pat;
    setWindowTitle(tr("Programmer une intervention pour ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());
    wdg_listmedecinscombo           = new QComboBox();
    QHBoxLayout *choixmedecinLay    = new QHBoxLayout();
    QHBoxLayout *programmLay        = new QHBoxLayout();

    wdg_interventionstableView  ->setFocusPolicy(Qt::NoFocus);
    wdg_interventionstableView  ->setPalette(QPalette(Qt::white));
    wdg_interventionstableView  ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_interventionstableView  ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_interventionstableView  ->verticalHeader()->setVisible(false);
    wdg_interventionstableView  ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_interventionstableView  ->setGridStyle(Qt::SolidLine);
    wdg_interventionstableView  ->verticalHeader()->setVisible(false);
    wdg_interventionstableView  ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_interventionstableView  ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    wdg_sessionstreeView->setMaximumWidth(340);
    wdg_sessionstreeView->setColumnWidth(0,340);
    wdg_sessionstreeView->setFocusPolicy(Qt::StrongFocus);
    wdg_sessionstreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_sessionstreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_sessionstreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_sessionstreeView->setContextMenuPolicy(Qt::CustomContextMenu);

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

    wdg_buttoninterventionframe     = new WidgetButtonFrame(wdg_interventionstableView);
    wdg_buttoninterventionframe     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect (wdg_buttoninterventionframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixInterventionFrame);

    programmLay     ->addWidget(wdg_buttonsessionsframe->widgButtonParent());
    programmLay     ->addWidget(wdg_buttoninterventionframe->widgButtonParent());
    programmLay     ->setSpacing(5);
    programmLay     ->setContentsMargins(0,0,0,0);

    dlglayout()     ->insertLayout(0, programmLay);
    dlglayout()     ->insertLayout(0, choixmedecinLay);

    AjouteLayButtons(UpDialog::ButtonOK);
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
    m_medecins.sort(0, Qt::DescendingOrder);
    for (int i=0; i< m_medecins.rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(m_medecins.item(i,0)->text());             //! le login
        wdg_listmedecinscombo->setItemData(i, m_medecins.item(i,1)->text());      //! l'id en data
    }
    if (Datas::I()->users->userconnected()->isMedecin())
    {
        wdg_listmedecinscombo->findData(Datas::I()->users->userconnected()->id());
        wdg_listmedecinscombo->setEnabled(false);
        ChoixMedecin(Datas::I()->users->userconnected()->id());
    }
    connect(wdg_listmedecinscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &dlg_programmationinterventions::ChoixMedecin);
    connect(wdg_sessionstreeView,   &QWidget::customContextMenuRequested,                   this, &dlg_programmationinterventions::MenuContextuelListeSessions);
}

dlg_programmationinterventions::~dlg_programmationinterventions()
{
}

void dlg_programmationinterventions::AfficheChoixIOL(int state)
{
    wdg_IOL->setVisible(state == Qt::Checked);
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
        proc->ab();
        break;
    }
}

void dlg_programmationinterventions::ChoixMedecin(int idx)
{
    m_currentuser = Datas::I()->users->getById(wdg_listmedecinscombo->itemData(idx).toInt());
    Datas::I()->sites->initListeByUser(m_currentuser->id());
    if (m_currentuser == Q_NULLPTR)
        return;
    Datas::I()->sessionsoperatoires->initListebyUserId(m_currentuser->id());
    RemplirTreeSessions();
}

void dlg_programmationinterventions::RemplirTreeSessions()
{
    wdg_sessionstreeView->selectionModel()->disconnect();
    disconnect(wdg_sessionstreeView, &QAbstractItemView::activated, this, &dlg_programmationinterventions::ChoixSession);
    m_sessions.clear();
    foreach (SessionOperatoire* session, *Datas::I()->sessionsoperatoires->sessions())
    {
        QList<QStandardItem *> items;
        QString nomsession = session->date().toString("dd-MMM-yy");
        UpStandardItem *item = new UpStandardItem(nomsession, session);
        items << item << new UpStandardItem(session->date().toString("yyyy-MM-dd"));
        m_sessions.appendRow(items);
        Site* site = Datas::I()->sites->getById(session->idlieu());
        if (site != Q_NULLPTR)
            item->appendRow(new UpStandardItem(site->nom(), session));
    }
    m_sessions.sort(1, Qt::AscendingOrder);
    m_sessions.takeColumn(1);
    wdg_sessionstreeView->setModel(&m_sessions);
    connect(wdg_sessionstreeView, &QAbstractItemView::activated, this, &dlg_programmationinterventions::ChoixSession);
    m_sessions.setHeaderData(0, Qt::Horizontal, tr("Sessions"));
    connect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]
                                {
                                    wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(true);
                                    wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(true);
                                });
    wdg_sessionstreeView->expandAll();
}

void dlg_programmationinterventions::ChoixSession(QModelIndex idx)
{
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(idx));
    if (itm == Q_NULLPTR)
        return;
    SessionOperatoire *session = dynamic_cast<SessionOperatoire*>(itm->item());
    if (session == Q_NULLPTR)
        return;
    Datas::I()->interventions->initListebySessionId(session->id());
    wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(Datas::I()->interventions->interventions()->size() == 0);
}

void dlg_programmationinterventions::CreerIntervention()
{
    UpDialog *dlg_intervention = new UpDialog(this);
    dlg_intervention->setWindowTitle(tr("programmer une intervention pour ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());

    QHBoxLayout *choixdateLay    = new QHBoxLayout();
    UpLabel* lbldate = new UpLabel;
    lbldate         ->setText(tr("Date"));
    QDateEdit *dateedit = new QDateEdit(m_currentdate);
    dateedit        ->setFixedSize(QSize(105,24));
    dateedit        ->setCalendarPopup(true);
    choixdateLay    ->addWidget(lbldate);
    choixdateLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixdateLay    ->addWidget(dateedit);
    choixdateLay    ->setSpacing(5);
    choixdateLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixheureLay    = new QHBoxLayout();
    UpLabel* lblheure = new UpLabel;
    lblheure        ->setText(tr("Heure"));
    QTimeEdit *timeedit = new QTimeEdit();
    choixheureLay   ->addWidget(lblheure);
    choixheureLay   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixheureLay   ->addWidget(timeedit);
    choixheureLay   ->setSpacing(5);
    choixheureLay   ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixinterventionLay    = new QHBoxLayout();
    UpLabel* lblinterv = new UpLabel;
    lblinterv               ->setText(tr("Type d'intervention"));
    QComboBox *intercombo = new QComboBox();
    choixinterventionLay    ->addWidget(lblinterv);
    choixinterventionLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixinterventionLay    ->addWidget(intercombo);
    choixinterventionLay    ->setSpacing(5);
    choixinterventionLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *checkIOLLay    = new QHBoxLayout();
    QCheckBox *IOLchk = new QCheckBox(tr("Utiliser un implamt"));
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
    dlg_intervention->dlglayout()   ->insertLayout(0, choixheureLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixdateLay);
    dlg_intervention->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_intervention->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    dlg_intervention->exec();
}

void dlg_programmationinterventions::CreerSession()
{
    UpDialog            *dlg_session = new UpDialog(this);
    dlg_session->setAttribute(Qt::WA_DeleteOnClose);
    dlg_session->setWindowTitle(tr("créer une session opératoire pour ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());

    QHBoxLayout *choixdateLay    = new QHBoxLayout();
    UpLabel* lbldate = new UpLabel;
    lbldate         ->setText(tr("Date"));
    QDateEdit *dateedit = new QDateEdit(m_currentdate);
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
        listbinds[CP_IDUSER_SESSIONOPERATOIRE]  = m_currentuser->id();
        Datas::I()->sessionsoperatoires->CreationSessionOperatoire(listbinds);
        RemplirTreeSessions();
        dlg_session->close();
    });
    dlg_session->exec();
}

void dlg_programmationinterventions::EnregistreNouvelleSession()
{
    proc->ab();
}

void dlg_programmationinterventions::EditSession()
{
    UpStandardItem      *upitem = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(wdg_sessionstreeView->selectionModel()->selectedIndexes().at(0)));
    SessionOperatoire   *session = dynamic_cast<SessionOperatoire*>(upitem->item());
    UpDialog            *dlg_session = new UpDialog(this);
    dlg_session->setAttribute(Qt::WA_DeleteOnClose);
    dlg_session->setWindowTitle(tr("Modifier une session opératoire pour ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());

    QHBoxLayout *choixdateLay    = new QHBoxLayout();
    UpLabel* lbldate = new UpLabel;
    lbldate         ->setText(tr("Date"));
    QDateEdit *dateedit = new QDateEdit(m_currentdate);
    dateedit        ->setFixedSize(QSize(120,24));
    dateedit        ->setDate(session->date());
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
    sitecombo->setCurrentIndex(sitecombo->findData(session->idlieu()));

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
        ItemsList::update(session, CP_DATE_SESSIONOPERATOIRE, date);
        ItemsList::update(session, CP_IDLIEU_SESSIONOPERATOIRE, idsite);
        RemplirTreeSessions();
        dlg_session->close();
    });
    dlg_session->exec();
}

void dlg_programmationinterventions::SupprimeSession()
{
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(wdg_sessionstreeView->selectionModel()->selectedIndexes().at(0)));
    SessionOperatoire *session = dynamic_cast<SessionOperatoire*>(upitem->item());
    QString nomsession = session->date().toString("dd-MMM-yy");
    Site* site = Datas::I()->sites->getById(session->idlieu());
    if (site != Q_NULLPTR)
        nomsession += " - " + site->nom();
    if (UpMessageBox::Question(this, tr("Voulez-vous supprimer la session") + "\n" + nomsession + " ?") != UpSmallButton::STARTBUTTON)
        return;
    Datas::I()->sessionsoperatoires->SupprimeSessionOperatoire(session);
    RemplirTreeSessions();
}

void dlg_programmationinterventions::MenuContextuelListeSessions()
{
    m_menucontextlistsessions = new QMenu(this);
    QModelIndex psortindx   = wdg_sessionstreeView->indexAt(wdg_sessionstreeView->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessions.itemFromIndex(psortindx));
    if (upitem == Q_NULLPTR)
    {
        QAction *pAction_CreerSession = m_menucontextlistsessions->addAction(tr("Créer une session"));
        connect (pAction_CreerSession,        &QAction::triggered,    this,    [=] {CreerSession();});
    }
    else
    {
        SessionOperatoire *session = dynamic_cast<SessionOperatoire*>(upitem->item());
        if (session == Q_NULLPTR)
        {
            delete m_menucontextlistsessions;
            return;
        }
        QAction *pAction_ModifSession = m_menucontextlistsessions->addAction(tr("Modifier la session"));
        connect (pAction_ModifSession,        &QAction::triggered,    this,    [=] {EditSession();});
        QAction *pAction_SupprSession = m_menucontextlistsessions->addAction(tr("Supprimer la session"));
        connect (pAction_SupprSession,        &QAction::triggered,    this,    [=] {SupprimeSession();});
    }
    // ouvrir le menu
    m_menucontextlistsessions->exec(cursor().pos());
    delete m_menucontextlistsessions;
}
