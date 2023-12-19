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

#include "dlg_programmationinterventions.h"

dlg_programmationinterventions::dlg_programmationinterventions(Patient *pat, Acte *act, QWidget *parent) : UpDialog(PATH_FILE_INI, "PositionsFiches/PositionProgramIntervention",parent)
{
    setcurrentsession(Q_NULLPTR);
    setcurrentintervention(Q_NULLPTR);
    m_currentchirpatient    = pat;
    m_currentchiracte       = act;
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
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
    wdg_interventionstreeView   ->setMouseTracking(true);

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
    wdg_buttonsessionsframe     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    connect (wdg_buttonsessionsframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixSessionFrame);
    wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(false);
    wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(false);

    wdg_buttoninterventionframe     = new WidgetButtonFrame(wdg_interventionstreeView);
    wdg_buttoninterventionframe     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_lblinterventions = new UpLabel();
    wdg_buttoninterventionframe->layButtons()->insertWidget(0, wdg_lblinterventions);
    connect (wdg_buttoninterventionframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixInterventionFrame);

    programmLay     ->addWidget(wdg_buttonsessionsframe->widgButtonParent());
    programmLay     ->addWidget(wdg_buttoninterventionframe->widgButtonParent());
    programmLay     ->setSpacing(5);
    programmLay     ->setContentsMargins(0,0,0,0);

    dlglayout()     ->insertLayout(0, programmLay);
    dlglayout()     ->insertLayout(0, choixmedecinLay);

    QHBoxLayout *buttbox            = new QHBoxLayout;
    wdg_incidentbutt      = new UpPushButton(tr("Rapport d'incident"));
    wdg_commandeIOLbutt   = new UpPushButton(tr("Commande d'implants"));
    wdg_manufacturerbutt  = new UpPushButton(tr("Gestion fabricants"));
    wdg_IOLbutt           = new UpPushButton(tr("Gestion implants"));
    QSize size = QSize(190,35);
    QSize sizeicon = QSize(20,20);
    wdg_incidentbutt                    ->setFixedSize(size);
    wdg_commandeIOLbutt                 ->setFixedSize(size);
    wdg_manufacturerbutt                ->setFixedSize(size);
    wdg_IOLbutt                         ->setFixedSize(size);
    wdg_incidentbutt                    ->setIcon(Icons::icOups());
    wdg_commandeIOLbutt                 ->setIcon(Icons::icNotepad());
    wdg_manufacturerbutt                ->setIcon(Icons::icFactory());
    wdg_IOLbutt                         ->setIcon(Icons::icIOL());
    wdg_incidentbutt                    ->setIconSize(sizeicon);
    wdg_commandeIOLbutt                 ->setIconSize(sizeicon);
    wdg_manufacturerbutt                ->setIconSize(sizeicon);
    wdg_IOLbutt                         ->setIconSize(sizeicon);
    buttbox                         ->addWidget(wdg_incidentbutt);
    buttbox                         ->addWidget(wdg_commandeIOLbutt);
    buttbox                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    buttbox                         ->addWidget(wdg_manufacturerbutt);
    buttbox                         ->addWidget(wdg_IOLbutt);
    buttbox                         ->setSpacing(5);
    buttbox                         ->setContentsMargins(0,0,0,0);
    buttbox                         ->setSizeConstraint(QLayout::SetFixedSize);
    buttonslayout()                 ->insertLayout(0, buttbox);

    connect(wdg_manufacturerbutt,       &QPushButton::clicked,  this,   &dlg_programmationinterventions::FicheListeManufacturers);
    connect(wdg_IOLbutt,                &QPushButton::clicked,  this,   &dlg_programmationinterventions::FicheListeIOLs);
    connect(wdg_incidentbutt,           &QPushButton::clicked,  this,   &dlg_programmationinterventions::ImprimeRapportIncident);
    connect(wdg_commandeIOLbutt,        &QPushButton::clicked,  this,   &dlg_programmationinterventions::ImprimeListeIOLsSession);

    wdg_IOLbutt->setEnabled(Datas::I()->users->userconnected()->isMedecin());

    AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(OKButton,     &QPushButton::clicked,    this, &QDialog::close);
    connect(PrintButton,  &QPushButton::clicked,    this, &dlg_programmationinterventions::ImprimeSession);
    dlglayout()->setStretch(0,1);
    dlglayout()->setStretch(1,15);
    setFixedWidth(1000);
    if (!Datas::I()->iols->isfull())
        Datas::I()->iols->initListe();

    m_medecinsmodel = new QStandardItemModel(this);

    foreach (User* usr, *Datas::I()->users->actifs())
        if (usr->isMedecin())
        {
            QList<QStandardItem *> items;
            items << new QStandardItem(usr->login())
                  << new QStandardItem(QString::number(usr->id()));
            m_medecinsmodel->appendRow(items);
        }
    m_medecinsmodel->sort(1, Qt::AscendingOrder);
    for (int i=0; i< m_medecinsmodel->rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(m_medecinsmodel->item(i,0)->text());             //! le login
        wdg_listmedecinscombo->setItemData(i, m_medecinsmodel->item(i,1)->text());      //! l'id en data
    }
    if (Datas::I()->users->userconnected()->isMedecin())
    {
        User *usr = Datas::I()->users->userconnected();
        if (m_currentchiracte)
            if (m_currentchiracte->isintervention())
            {
                usr = Datas::I()->users->getById(m_currentchiracte->idUserSuperviseur());
                if (!usr)
                    usr = Datas::I()->users->userconnected();
            }
        wdg_listmedecinscombo->setCurrentIndex(wdg_listmedecinscombo->findData(usr->id()));
        //wdg_listmedecinscombo->setEnabled(false);
        connect(wdg_listmedecinscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &dlg_programmationinterventions::ChoixMedecin);
        ChoixMedecin(wdg_listmedecinscombo->findData(usr->id()));
    }
    else
    {
        connect(wdg_listmedecinscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this, &dlg_programmationinterventions::ChoixMedecin);
        ChoixMedecin(0);
    }
    connect(wdg_sessionstreeView,       &QWidget::customContextMenuRequested,                   this, &dlg_programmationinterventions::MenuContextuelSessions);
    connect(wdg_interventionstreeView,  &QWidget::customContextMenuRequested,                   this, &dlg_programmationinterventions::MenuContextuelInterventionsions);
    Datas::I()->typesinterventions->initListe();
    m_typeinterventionsmodel = Datas::I()->typesinterventions->listetypesinterventionsmodel(true);
}

dlg_programmationinterventions::~dlg_programmationinterventions()
{
}

bool dlg_programmationinterventions::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        UpDoubleSpinBox* objUpdSpin = qobject_cast<UpDoubleSpinBox*>(obj);
        if (objUpdSpin != Q_NULLPTR)   {
            objUpdSpin->setPrefix("");
            objUpdSpin->selectAll();
            return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

/*! les sessions ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ChoixMedecin(int idx)
{
    m_currentchiruser = Datas::I()->users->getById(wdg_listmedecinscombo->itemData(idx).toInt());
    if (m_currentchiruser == Q_NULLPTR)
        return;
    Datas::I()->sites->initListeByUser(m_currentchiruser->id());
    Datas::I()->sessionsoperatoires->initListebyUserId(m_currentchiruser->id());
    /*! si un acte chirurgical est défini à l'ouverture de la fiche,
     *  on regarde s'il est lié à l'utilisateur chirurgical en cours
     *  et si oui, on se positionne dessus
     */
    if (m_currentchiracte)
    {
        Intervention *interv = Datas::I()->interventions->getById(m_currentchiracte->idIntervention());
        if (interv)
        {
            auto it = Datas::I()->sessionsoperatoires->sessions()->constFind(interv->idsession());
            if (it != Datas::I()->sessionsoperatoires->sessions()->cend())
            {
                setcurrentsession(const_cast<SessionOperatoire*>(it.value()));
                setcurrentintervention(interv);
            }
            else
                delete interv;
        }
    }
    /*! si on n'a pas défini d'intervention en cours,
     * on fait la liste des interventions du patient en cours
     * on définit comme intervention en cours
     * la dernière intervention de ce patient qui a été faite par ce chirurgien
     */
    if (!currentintervention())
    {
        Datas::I()->patients->initListeIdInterventions(m_currentchirpatient);
        if (m_currentchirpatient->listidinterventions().size()>0)
            for (int i=0; i < m_currentchirpatient->listidinterventions().size(); ++i)
            {
                int iduser = m_currentchirpatient->listidinterventions().at(i).second;
                if (iduser == m_currentchiruser->id())
                {
                    Intervention *interv = Datas::I()->interventions->getById(m_currentchirpatient->listidinterventions().at(i).first);
                    if (interv)
                    {
                        auto it = Datas::I()->sessionsoperatoires->sessions()->constFind(interv->idsession());
                        if (it != Datas::I()->sessionsoperatoires->sessions()->cend())
                        {
                            if (currentsession() == Q_NULLPTR)
                            {
                                setcurrentsession(const_cast<SessionOperatoire*>(it.value()));
                                setcurrentintervention(interv);
                            }
                            else if (currentsession()->date() < const_cast<SessionOperatoire*>(it.value())->date())
                            {
                                setcurrentsession(const_cast<SessionOperatoire*>(it.value()));
                                setcurrentintervention(interv);
                            }
                        }
                        else
                            delete interv;
                       break;
                    }
                }
            }
    }
    /*! si aucune intervention pour le patient en cours n'a été effectuée par le chirurgien en cours, on se positionne sur la prochaine session de ce chirurgien */
    if (!currentsession() && Datas::I()->sessionsoperatoires->sessions()->size() > 0)
    {
        SessionOperatoire *lastsession = Q_NULLPTR;
        for (auto it = Datas::I()->sessionsoperatoires->sessions()->constBegin(); it != Datas::I()->sessionsoperatoires->sessions()->constEnd();++it)
        {
            SessionOperatoire *session = it.value();
            if (session != Q_NULLPTR)
            {
                if (lastsession == Q_NULLPTR)
                    lastsession = session;
                else if (session->date() > lastsession->date())
                    lastsession = session;
                if (session->date() > m_currentdate)
                {
                    if (currentsession() == Q_NULLPTR)
                        setcurrentsession(session);
                    else if (session->date() < currentsession()->date())
                        setcurrentsession(session);
                }
            }
        }
        if (currentsession() == Q_NULLPTR && lastsession != Q_NULLPTR)
            setcurrentsession(lastsession);
    }
    RemplirTreeSessions();
}

void dlg_programmationinterventions::ChoixSessionFrame()
{
    switch (wdg_buttonsessionsframe->Choix()) {
    case WidgetButtonFrame::Plus:
        FicheSession();
        break;
    case WidgetButtonFrame::Modifier:
        ModifSession();
        break;
    case WidgetButtonFrame::Moins:
        SupprimeSession();
        break;
    }
}

void dlg_programmationinterventions::AfficheInterventionsSession(QModelIndex idx)
{
    if (Datas::I()->sessionsoperatoires->sessions()->size() == 0)
    {
        ItemsList::clearAll(Datas::I()->interventions->interventions());
        RemplirTreeInterventions();
        return;
    }
    UpStandardItem *upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel->itemFromIndex(idx));
    if (upitem == Q_NULLPTR)
    {
        RemplirTreeInterventions();
        return;
    }
    setcurrentsession(qobject_cast<SessionOperatoire*>(upitem->item()));
    wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(currentsession() != Q_NULLPTR);
    wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(currentsession() != Q_NULLPTR);
    wdg_buttoninterventionframe->wdg_plusBouton->setEnabled(currentsession() != Q_NULLPTR);
    if (currentsession() != Q_NULLPTR)
    {
        Datas::I()->interventions->initListebySessionId(currentsession()->id());
        wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(Datas::I()->interventions->interventions()->size()==0);
    }
    if (currentintervention() != Q_NULLPTR)
        if (currentintervention()->idsession() == currentsession()->id())
        {
            RemplirTreeInterventions(currentintervention());
            return;
        }
    RemplirTreeInterventions();
}

void dlg_programmationinterventions::RemplirTreeSessions()
{
    disconnect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::AfficheInterventionsSession);
    if (m_sessionsmodel != Q_NULLPTR)
        delete m_sessionsmodel;
    m_sessionsmodel = new QStandardItemModel(this);
    QModelIndex idx = QModelIndex();
    wdg_sessionstreeView->setModel(m_sessionsmodel);
    m_sessionsmodel->setColumnCount(2);
    m_sessionsmodel->setHeaderData(0, Qt::Horizontal, tr("Sessions"));
    m_sessionsmodel->setHeaderData(1, Qt::Horizontal, "");
    if (Datas::I()->sessionsoperatoires->sessions()->size() >0)
    {
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
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
            if (itm != Q_NULLPTR)
            {
                SessionOperatoire* sess = qobject_cast<SessionOperatoire*>(itm->item());
                if (sess)
                    if (sess->incident() != "")
                    {
                        UpStandardItem *itminc = new UpStandardItem(tr("Incident") + " : " + sess->incident(), sess);
                        itminc ->setForeground(QBrush(QColor(Qt::red)));
                        itminc ->setEditable(false);
                        itm->appendRow(itminc);
                    }
            }
        }
        wdg_sessionstreeView->expandAll();
        if (m_sessionsmodel->rowCount() >0)
        {
            if (currentsession() == Q_NULLPTR)
                idx = m_sessionsmodel->item(m_sessionsmodel->rowCount()-1)->index();        //! l'index de ce dernier item
            else {
            for (int i=0; i<m_sessionsmodel->rowCount(); ++i)
            {
                UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
                if (itm)
                {
                    SessionOperatoire *session = qobject_cast<SessionOperatoire*>(itm->item());
                    if (session)
                        if (session->id() == currentsession()->id())
                        {
                            idx = itm->index();
                            break;
                        }
                }
            }
            }
            wdg_sessionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            wdg_sessionstreeView->selectionModel()->select(idx,QItemSelectionModel::Rows | QItemSelectionModel::Select);
            wdg_sessionstreeView->setCurrentIndex(idx);
        }
        connect(wdg_sessionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::AfficheInterventionsSession);
        AfficheInterventionsSession(idx);
    }
    else
    {
        setcurrentsession(Q_NULLPTR);
        wdg_buttonsessionsframe->wdg_moinsBouton->setEnabled(false);
        wdg_buttonsessionsframe->wdg_modifBouton->setEnabled(false);
    }
    wdg_interventionstreeView->setVisible(Datas::I()->sessionsoperatoires->sessions()->size() >0);
    wdg_buttoninterventionframe->setVisible(Datas::I()->sessionsoperatoires->sessions()->size() >0);
}

void dlg_programmationinterventions::FicheSession(SessionOperatoire *session)
{
    UpDialog            *dlg_session = new UpDialog(this);

    dlg_session->setWindowTitle(tr("créer une session opératoire pour ") + m_currentchirpatient->prenom() + " " + m_currentchirpatient->nom());
    dlg_session->setWindowModality(Qt::WindowModal);

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

    QWidget *wdg_incident       = new QWidget();
    QHBoxLayout *incidentLay    = new QHBoxLayout();
    UpLabel* lblincident = new UpLabel;
    lblincident               ->setText(tr("Incident"));
    QTextEdit *incidenttxtedit  = new QTextEdit();
    incidenttxtedit             ->setFixedSize(QSize(250,100));
    incidentLay    ->addWidget(lblincident);
    incidentLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    incidentLay    ->addWidget(incidenttxtedit);
    incidentLay    ->setSpacing(5);
    incidentLay    ->setContentsMargins(0,0,0,0);
    wdg_incident   ->setLayout(incidentLay);
    wdg_incident   ->setVisible(session != Q_NULLPTR);


    dlg_session->dlglayout()   ->insertWidget(0, wdg_incident);
    dlg_session->dlglayout()   ->insertLayout(0, choixsiteLay);
    dlg_session->dlglayout()   ->insertLayout(0, choixdateLay);
    dlg_session->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_session->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    if (session != Q_NULLPTR)
    {
        dateedit        ->setDate(session->date());
        sitecombo       ->setCurrentIndex(sitecombo->findData(session->idlieu()));
        incidenttxtedit ->setText(session->incident());
    }
    connect(dlg_session->OKButton, &QPushButton::clicked, dlg_session, [&]
    {
        QDate date = dateedit->date();
        int idsite = sitecombo->currentData().toInt();
        for (int i = 0; i < m_sessionsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i));
            SessionOperatoire *session = qobject_cast<SessionOperatoire*>(upitem->item());
            if (session->date() ==  date && session->idlieu() == idsite)
            {
                UpMessageBox::Watch(dlg_session, tr("Cette session existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_DATE_SESSIONOPERATOIRE]    = date.toString("yyyy-MM-dd");
        listbinds[CP_IDLIEU_SESSIONOPERATOIRE]  = idsite;
        listbinds[CP_IDUSER_SESSIONOPERATOIRE]  = m_currentchiruser->id();
        if (session == Q_NULLPTR)
            setcurrentsession(Datas::I()->sessionsoperatoires->CreationSessionOperatoire(listbinds));
        else
        {
            ItemsList::update(session, CP_DATE_SESSIONOPERATOIRE, date);
            ItemsList::update(session, CP_IDLIEU_SESSIONOPERATOIRE, idsite);
            ItemsList::update(session, CP_INCIDENT_SESSIONOPERATOIRE, incidenttxtedit->toPlainText());
        }
        RemplirTreeSessions();
        dlg_session->accept();
    });
    dlg_session->exec();
    delete dlg_session;
}

void dlg_programmationinterventions::ModifSession()
{
    FicheSession(currentsession());
}

void dlg_programmationinterventions::ImprimeRapportIncident()
{
    if (currentsession() == Q_NULLPTR)
        return;
    bool AvecDupli   = false;
    bool AvecPrevisu = proc->ApercuAvantImpression();
    bool AvecNumPage = true;

    //--------------------------------------------------------------------
    // Préparation de l'état "session" dans un QplainTextEdit
    //--------------------------------------------------------------------

    int iduser = currentsession()->iduser();

//    //création de l'entête
    QString EnTete;
    User *userEntete = Datas::I()->users->getById(iduser);
    if(userEntete == Q_NULLPTR)
        return;
    EnTete = proc->CalcEnteteImpression(m_currentdate, userEntete).value("Norm");
    if (EnTete == "") return;
    Site *sit = Datas::I()->sites->getById(currentsession()->idlieu());

    EnTete.replace("{{TITRE1}}"            , "<b>" + tr("RAPPORT D'INCIDENTS OPÉRATOIRES") +"</b>");
    EnTete.replace("{{PRENOM PATIENT}}"    , "");
    EnTete.replace("{{NOM PATIENT}}"       , "");
    EnTete.replace("{{TITRE}}"             , "<b>" + wdg_lblinterventions->text() +"</b>");
    EnTete.replace("{{DDN}}"               , "");

    // création du pied
    QString Pied = proc->CalcPiedImpression(userEntete);
    if (Pied == "") return;

    // creation du corps
    double c = CORRECTION_td_width;

    QString lign = "";
    QString cor = sit->nom() + "<br>" + sit->coordonnees().replace("\n", "<br>");
    lign =  HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b>" + cor + "</b></span></td>" ;
    lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b></b></span></td>";
    lign += "<p align=\"center\"><font color = " COULEUR_TITRES "><span style=\"font-size:10pt;\"><b>" + tr("RAPPORTS D'INCIDENTS SUR LA SESSION CHIRURGICALE DU") + " " + currentsession()->date().toString("dddd dd MMMM yyyy") + "</b></span></font>" ;
    lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b></b></span></td>";
    if (currentsession()->incident() != "")
    {
        lign += "<p align=\"left\"><font color = " COULEUR_TITRES "><span style=\"font-size:10pt;\"><b>" + tr("INCIDENTS GÉNÉRAUX SUR LA SESSION") + "</b></span></font>" ;
        lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*30)) + "\"></td><td width=\"" + QString::number(int(c*500)) + "\"><span style=\"font-size:8pt;\">" + currentsession()->incident() + "</span></td>" ;
        lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b></b></span></td>";
    }
    bool incidents = false;
    foreach (Intervention* interv, *Datas::I()->interventions->interventions())
    {
        if (interv->incident() != "")
            incidents = true;
    }
    if (incidents)
    {
        lign += "<p align=\"left\"><font color = " COULEUR_TITRES "><span style=\"font-size:10pt;\"><b>" + tr("INCIDENTS PAR INTERVENTION") + "</b></span></font>" ;
        foreach (Intervention* interv, *Datas::I()->interventions->interventions())
        {
            if (interv->incident() != "")
            {
                QString entete = tr("Intervention") + " " + QString::number(interv->id());
                TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());
                if (typ)
                    entete += " - " + typ->typeintervention();
                lign +=  HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*30)) + "\"><td width=\"" + QString::number(int(c*400)) + "\"><span style=\"font-size:8pt;\"><b>" + entete + "</b></span></td>" ;
                QString inc = interv->incident();
                lign +=  HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*60)) + "\"></td><td width=\"" + QString::number(int(c*400)) + "\"><font color = gray><span style=\"font-size:8pt;\"><b>" + inc + "</b></span></font></td>" ;
            }
        }
    }
    QTextEdit textEdit;
    textEdit.setHtml(lign);
    proc->Imprime_Etat(this, &textEdit, EnTete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
}

void dlg_programmationinterventions::ImprimeSession()
{
    if (currentsession() == Q_NULLPTR)
        return;
    bool AvecDupli   = false;
    bool AvecPrevisu = proc->ApercuAvantImpression();
    bool AvecNumPage = true;

    //--------------------------------------------------------------------
    // Préparation de l'état "session" dans un QplainTextEdit
    //--------------------------------------------------------------------

    int iduser = currentsession()->iduser();

    //création de l'entête
    QString EnTete;
    User *userEntete = Datas::I()->users->getById(iduser);
    if(userEntete == Q_NULLPTR)
        return;
    EnTete = proc->CalcEnteteImpression(m_currentdate, userEntete).value("Norm");
    if (EnTete == "") return;
    Site *sit = Datas::I()->sites->getById(currentsession()->idlieu());

    EnTete.replace("{{TITRE1}}"            , "<b>" + tr("PROGRAMME OPÉRATOIRE") +"</b>");
    EnTete.replace("{{PRENOM PATIENT}}"    , (sit? sit->nom() + " - " + sit->ville() : ""));
    EnTete.replace("{{NOM PATIENT}}"       , "");
    EnTete.replace("{{TITRE}}"             , "<b>" + wdg_lblinterventions->text() +"</b>");
    EnTete.replace("{{DDN}}"               , "<font color = \"" COULEUR_TITRES "\">" + currentsession()->date().toString("dddd dd MMMM yyyy") + "</font>");

    // création du pied
    QString Pied = proc->CalcPiedImpression(userEntete);
    if (Pied == "") return;

    // creation du corps
    double c = CORRECTION_td_width;

    QString lign = "";
    if (currentsession()->incident() != "")
    {
        QString incident = (currentsession()->incident() ==""? " : " + tr("NEANT") : "");
        lign +=  HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*200)) + "\"><font color = " COULEUR_TITRES "><span style=\"font-size:8pt;\"><b>" + tr("INCIDENTS GÉNÉRAUX SUR LA SESSION") + incident + "</b></span></font></td>" ;
        if (currentsession()->incident() != "")
            lign +=  HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*30)) + "\"></td><td width=\"" + QString::number(int(c*200)) + "\"><span style=\"font-size:8pt;\">" + currentsession()->incident() + "</span></td>" ;
        lign += HTML_RETOURLIGNE;
    }
    for (int i=0; i< m_interventionsmodel->rowCount(); ++i)
    {
        QStandardItem *itm = m_interventionsmodel->item(i);
        if (itm != Q_NULLPTR)
        {
             if (itm->hasChildren())
            {
                lign +=  HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*200)) + "\"><font color = " COULEUR_TITRES "><span style=\"font-size:8pt;\"><b>" + itm->text() + "</b></span></font></td>" ;
                QTime time = QTime::fromString(itm->text(),"- HH:mm -");
                foreach (Intervention* interv, *Datas::I()->interventions->interventions())
                {
                    if (interv->heure() == time)
                    {
                        Patient * pat = Datas::I()->patients->getById(interv->idpatient(), Item::LoadDetails);
                        if (pat != Q_NULLPTR)
                        {
                            QString nompatient = "";
                            nompatient  = pat->nom().toUpper() + " " + pat->prenom();
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*30)) + "\"></td><td width=\"350\"><font color = darkgreen><span style=\"font-size:8pt;\"><b>" + nompatient + "</b></span></font></td>" ;
                        }
                        TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());
                        if (typ)                                                                                                                    //! type d'intervention et anesthésie
                        {
                            QString color = "black";
                            QString typinterv = typ->typeintervention().toUpper();
                            if (interv->cote() != Utils::NoLoSo)
                                typinterv += " - " + tr("Côté") + " " +  Utils::TraduitCote(interv->cote()).toLower();
                            if (interv->anesthesie() != Intervention::NoLoSo)                                                                       //! type d'anesthésie
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
                                    color = "red";
                            }
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*60)) + "\"></td><td width=\"350\"><font color = " + color + "><span style=\"font-size:8pt;\">" + typinterv + "</span></font></td>" ;                        }
                        if (pat != Q_NULLPTR)
                        {
                            QMap<QString,QVariant> mapage = Utils::CalculAge(pat->datedenaissance(), db->ServerDate());
                            QString sexeddntel = (pat->sexe() == "M"? tr("Né le") : tr("Née le"))                                                           //! date de naissance - sexe - telephone
                                    + " " + pat->datedenaissance().toString("dd-MM-yyyy")
                                    + " - " + mapage["toString"].toString();
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
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*60)) + "\"></td><td width=\"350\"><font color = gray><span style=\"font-size:8pt;\">" + sexeddntel + "</span></font></td>" ;
                        }
                        if (interv->idIOL()>0)                                                                                                      //! IOL
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
                            ioltxt = tr("Implant") + " : " + ioltxt;
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*60)) + "\"></td><td width=\"250\"><span style=\"font-size:8pt;\">" + ioltxt + "</span></td>" ;
                        }
                        if (interv->observation() != "")                                                                                            //! observation
                        {
                            QString obs = tr("Remarque") + " : " + interv->observation();
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*60)) + "\"></td><td width=\"250\"><font color = gray><span style=\"font-size:8pt;\">" + obs + "</span></font></td>" ;
                        }
                        if (interv->incident() != "")                                                                                               //! incident
                        {
                            QString inc = tr("Incident") + " : " + interv->incident();
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*60)) + "\"></td><td width=\"250\"><font color = gray><span style=\"font-size:8pt;\"><b>" + inc + "</b></span></font></td>" ;
                        }
                    }
                }
            }
        }
    }
    QTextEdit textEdit;
    textEdit.setHtml(lign);
    proc->Imprime_Etat(this, &textEdit, EnTete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
}

void dlg_programmationinterventions::SupprimeSession()
{
    if (currentsession() == Q_NULLPTR)
        return;
    QString nomsession = currentsession()->date().toString("dd-MMM-yy");
    Site* site = Datas::I()->sites->getById(currentsession()->idlieu());
    if (site != Q_NULLPTR)
        nomsession += " - " + site->nom();
    if (UpMessageBox::Question(this, tr("Voulez-vous supprimer la session"), nomsession + " ?") != UpSmallButton::STARTBUTTON)
        return;
    foreach (Intervention* interv, *Datas::I()->interventions->interventions())
    {
        setcurrentintervention(interv);
        SupprimeIntervention();
    }
    SessionOperatoire* session = Datas::I()->sessionsoperatoires->getById(currentsession()->id());
    if (session)
        Datas::I()->sessionsoperatoires->SupprimeSessionOperatoire(session);
    setcurrentsession(Q_NULLPTR);
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
        connect (pAction_CreerSession,        &QAction::triggered,    this,    [&] {FicheSession();});
    }
    else
    {
        SessionOperatoire *session = qobject_cast<SessionOperatoire*>(upitem->item());
        if (session == Q_NULLPTR)
        {
            delete m_ctxtmenusessions;
            return;
        }
        QAction *pAction_ModifSession = m_ctxtmenusessions->addAction(tr("Modifier la session"));
        connect (pAction_ModifSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::ModifSession);
        QAction *pAction_SupprSession = m_ctxtmenusessions->addAction(tr("Supprimer la session"));
        connect (pAction_SupprSession,        &QAction::triggered,    this,    &dlg_programmationinterventions::SupprimeSession);
        if (Datas::I()->users->userconnected()->isMedecin())
        {
            QString txt = (session->incident() != ""? tr("Modifier le rapport d'incident") : tr ("Enregistrer un incident sur cette session"));
            QAction *pAction_IncidentSession = m_ctxtmenusessions->addAction(txt);
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
        ModifIntervention();
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
    setcurrentintervention(qobject_cast<Intervention*>(upitem->item()));
    if (currentintervention() == Q_NULLPTR)
    {
        wdg_buttoninterventionframe->wdg_moinsBouton->setEnabled(false);
        wdg_buttoninterventionframe->wdg_modifBouton->setEnabled(false);
        return;
    }
    wdg_buttoninterventionframe->wdg_moinsBouton->setEnabled(true);
    wdg_buttoninterventionframe->wdg_modifBouton->setEnabled(true);
}

void dlg_programmationinterventions::RemplirTreeInterventions(Intervention* intervention)
{
    disconnect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    if (m_interventionsmodel == Q_NULLPTR)
        delete m_interventionsmodel;
    m_interventionsmodel = new QStandardItemModel(this);
    wdg_lblinterventions-> setText(QString::number(Datas::I()->interventions->interventions()->size()) + " " + (Datas::I()->interventions->interventions()->size()>1? tr("interventions") : tr("intervention")));

    bool incident = false;          //! va servir à indiquer si des incidents sont notés sur la session
    if (currentsession() != Q_NULLPTR)
        if (currentsession()->incident() != "")
            incident = true;
    bool iollist = false;           //! va servir à indiquer si des implants sont prévus sur la session
    QStandardItem *rootNodeDate = m_interventionsmodel->invisibleRootItem();
    QList<QTime> listheures;
    PrintButton         ->setEnabled(Datas::I()->interventions->interventions()->size() >0);
    wdg_interventionstreeView->setModel(m_interventionsmodel);
    if (Datas::I()->interventions->interventions()->size()>0)
    {
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
                    continue;
                nompatient  = pat->nom().toUpper() + " " + pat->prenom();
                itempat     = new UpStandardItem(nompatient, interv);
                QFont fontitem = m_font;
                fontitem    .setBold(true);
                itempat     ->setFont(fontitem);
                itempat     ->setForeground(QBrush(QColor(Qt::darkBlue)));
                itempat     ->setEditable(false);
                listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itempat << new QStandardItem(QString::number(a) + "a"));             //! nom du patient

                TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());                                  //! type d'intervention et anesthésie
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
                QMap<QString,QVariant> mapage = Utils::CalculAge(pat->datedenaissance(), db->ServerDate());
                QString sexeddntel = (pat->sexe() == "M"? tr("Né le") : tr("Née le"))                                                           //! date de naissance - sexe - telephone
                        + " " + pat->datedenaissance().toString("dd-MM-yyyy")
                        + " - " + mapage["toString"].toString();
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
                    iollist = true;
                    QString ioltxt = "";
                    IOL *iol = Datas::I()->iols->getById(interv->idIOL());
                    if (iol != Q_NULLPTR)
                    {
                        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                        if (man != Q_NULLPTR)
                            ioltxt += man->nom().toUpper() + " " + iol->modele() + " ";
                    }
                    ioltxt += Utils::PrefixePlus(interv->puissanceIOL());
                    if (interv->cylindreIOL() != 0.0)
                        ioltxt += " Cyl. " + Utils::PrefixePlus(interv->cylindreIOL());
                    itemiol = new UpStandardItem("\t" + tr("Implant") + " : " + ioltxt, interv);
                    itemiol ->setEditable(false);
                    if (iol)
                    {
                        QString ttip = iol->tooltip(true);
                        itemiol->setData(ttip);
                    }
                    listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemiol << new QStandardItem(QString::number(a) + "d"));
                }
                if (interv->observation() != "")                                                                                                //! observation
                {
                    itemobs = new UpStandardItem("\t" + tr("Remarque") + " : " + interv->observation(), interv);
                    itemobs ->setForeground(QBrush(QColor(Qt::red)));
                    itemobs ->setEditable(false);
                    listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << itemobs << new QStandardItem(QString::number(a) + "e"));
                }
                if (interv->incident() != "")                                                                                                   //! incident
                {
                    incident = true;
                    iteminc = new UpStandardItem("\t" + tr("Incident") + " : " + interv->incident(), interv);
                    iteminc ->setForeground(QBrush(QColor(Qt::red)));
                    iteminc ->setEditable(false);
                    listitemsheure.at(0)->appendRow(QList<QStandardItem*>() << iteminc << new QStandardItem(QString::number(a) + "f"));
                }
            }
            listitemsheure.at(0)->sortChildren(1);
            ++a;
        }
        connect(wdg_interventionstreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &dlg_programmationinterventions::ChoixIntervention);
    }
    else
    {
        setcurrentintervention(Q_NULLPTR);
    }
    m_interventionsmodel->setHeaderData(0, Qt::Horizontal, tr("Interventions"));
    m_interventionsmodel->setHeaderData(1, Qt::Horizontal, "");
    m_interventionsmodel->setHeaderData(2, Qt::Horizontal, "");
    wdg_interventionstreeView->expandAll();
    wdg_interventionstreeView   ->setColumnWidth(0,340);
    wdg_interventionstreeView   ->header()->setSectionResizeMode(QHeaderView::Fixed);
    wdg_interventionstreeView   ->setSortingEnabled(false);
    if (Datas::I()->interventions->interventions()->size()>0)
        PositionneTreeInterventionsSurIntervention(intervention);
    wdg_incidentbutt    ->setEnabled(incident);
    wdg_commandeIOLbutt ->setEnabled(iollist);
    wdg_buttoninterventionframe->wdg_moinsBouton->setEnabled(currentintervention() != Q_NULLPTR);
    wdg_buttoninterventionframe->wdg_modifBouton->setEnabled(currentintervention() != Q_NULLPTR);

//    connect(wdg_interventionstreeView,  &QAbstractItemView::entered,    this,   [&](QModelIndex idx)
//    {
//        QStandardItem *itm = dynamic_cast<QStandardItem*>(m_interventionsmodel->itemFromIndex(idx));
//        if (itm)
//            QToolTip::showText(cursor().pos(), itm->data().toString());
//    });
}

void dlg_programmationinterventions::PositionneTreeInterventionsSurIntervention(Intervention* interv)
{
    QModelIndex idx;
    bool found = false;
    if (m_interventionsmodel->rowCount() >0)
    {
        if (interv == Q_NULLPTR)
        {
            idx = m_interventionsmodel->item(m_interventionsmodel->rowCount()-1)->index();        //! l'index du dernier item
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_interventionsmodel->item(m_interventionsmodel->rowCount()-1));
            if (itm->rowCount() > 0)
            {
                UpStandardItem *childitm = dynamic_cast<UpStandardItem*>(itm->child(0,0));
                idx = childitm->index();
                found = true;
            }
        }
        else for (int i=0; i<m_interventionsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_interventionsmodel->item(i));
            QString heurestring = "- " + interv->heure().toString("HH:mm") + " -";
            if (itm->text() == heurestring)
            {
                for (int j=0; j<itm->rowCount(); ++j)
                {
                    UpStandardItem *childitm = dynamic_cast<UpStandardItem*>(itm->child(j,0));
                    if (childitm)
                    {
                        if (childitm->item())
                        {
                            if (childitm->item()->id() == interv->id())
                            {
                                idx = childitm->index();
                                found = true;
                                break;
                            }
                        }
                    }
                }
                if (found)
                    break;
            }
        }
        if (found)
        {
            wdg_interventionstreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            wdg_interventionstreeView->setCurrentIndex(idx);
            wdg_interventionstreeView->selectionModel()->select(idx,QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
    ChoixIntervention(idx);
}

void dlg_programmationinterventions::EnregistreIncident(Item *itm)
{
    if( itm == Q_NULLPTR)
        return;
    QString mode = "";
    QString incident = "";
    Intervention * interv = qobject_cast<Intervention*>(itm);
    if (interv)
    {
        mode = "intervention";
        if (interv->incident() != "")
            incident = interv->incident();
    }
    else
    {
        SessionOperatoire * session = qobject_cast<SessionOperatoire*>(itm);
        if (session)
        {
            mode = "session";
            if (session->incident() != "")
                incident = session->incident();
        }
        else
            return;
    }

    UpDialog            *dlg_incident = new UpDialog(this);

    dlg_incident->setWindowModality(Qt::WindowModal);
    dlg_incident->setWindowTitle(tr("Rapport d'incident"));

    QLineEdit *incidenttxtedit  = new QLineEdit();
    incidenttxtedit             ->setFixedSize(QSize(450,30));
    incidenttxtedit             ->setText(incident);

    dlg_incident->dlglayout()   ->insertWidget(0, incidenttxtedit);
    dlg_incident->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_incident->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_incident->OKButton, &QPushButton::clicked, dlg_incident, [&]
    {
        QString incident = incidenttxtedit->text();
        if (mode == "intervention")
        {
            ItemsList::update(currentintervention(), CP_INCIDENT_LIGNPRGOPERATOIRE, incident);
            RemplirTreeInterventions(currentintervention());
        }
        else if (mode == "session")
        {
            ItemsList::update(currentsession(), CP_INCIDENT_SESSIONOPERATOIRE, incident);
            RemplirTreeSessions();
        }
        dlg_incident->accept();
    });
    dlg_incident->exec();
    delete dlg_incident;
}

void dlg_programmationinterventions::ModifStatutActeCorrespondant(int idacte)
{
    if (idacte >0)
    {
        Acte * act = Datas::I()->actes->getById(idacte, Item::NoLoadDetails);
        if (act != Q_NULLPTR)
            act->setidintervention(idacte);
    }
}

void dlg_programmationinterventions::FicheIntervention(Intervention *interv)
{
    bool verifencours = false;
    UpDialog *dlg_intervention = new UpDialog(this);
    dlg_intervention->setWindowModality(Qt::WindowModal);
    Patient *pat = (interv == Q_NULLPTR? m_currentchirpatient : Datas::I()->patients->getById(interv->idpatient()));
    if (pat != Q_NULLPTR)
            dlg_intervention->setWindowTitle(pat->prenom() + " " + pat->nom());

    QHBoxLayout *titreLay       = new QHBoxLayout();
    UpLabel* lbltitre           = new UpLabel;
    lbltitre                    ->setText(m_currentchirpatient->nom().toUpper() + " " + m_currentchirpatient->prenom());
    titreLay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    titreLay                    ->addWidget(lbltitre);
    titreLay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    titreLay                    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixsessionLay = new QHBoxLayout();
    UpLabel* lblsession         = new UpLabel;
    lblsession                  ->setText(tr("Session"));
    UpComboBox *sessioncombo    = new UpComboBox();
    sessioncombo                ->setModel(m_sessionsmodel);
    for (int i=0; i< m_sessionsmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_sessionsmodel->item(i,0));
        if (itm)
            if (itm->item())
                if (itm->item()->id() == currentsession()->id())
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

    QHBoxLayout *choixheureLay = new QHBoxLayout();
    UpLabel* lblheure   = new UpLabel;
    lblheure            ->setText(tr("Heure"));
    QTimeEdit *timeedit = new QTimeEdit();
    choixheureLay       ->addWidget(lblheure);
    choixheureLay       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixheureLay       ->addWidget(timeedit);
    choixheureLay       ->setSpacing(5);
    choixheureLay       ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixinterventionLay = new QHBoxLayout();
    UpLabel* lblinterv = new UpLabel;
    lblinterv               ->setText(tr("Type d'intervention"));

    UpComboBox *interventioncombo = new UpComboBox();
    interventioncombo       ->setFixedSize(QSize(150,28));
    interventioncombo       ->setEditable(true);
    interventioncombo       ->setModel(m_typeinterventionsmodel);
    interventioncombo       ->setCurrentIndex(-1);
    interventioncombo       ->setInsertPolicy(QComboBox::NoInsert);
    interventioncombo       ->setCompleter(Datas::I()->typesinterventions->completer());

    UpSmallButton *gestionTypIntervButton = new UpSmallButton(dlg_intervention);
    gestionTypIntervButton  ->setImmediateToolTip(tr("gérer les types d'intervention"));
    gestionTypIntervButton  ->setIcon(Icons::icEditer());
    int szicon  = 20;
    int geo     = szicon + 4;
    gestionTypIntervButton  ->setIconSize(QSize(szicon,szicon));
    gestionTypIntervButton  ->setFlat(true);
    gestionTypIntervButton  ->setFixedSize(geo,geo);
    gestionTypIntervButton  ->setFocusPolicy(Qt::NoFocus);
    gestionTypIntervButton  ->setContextMenuPolicy(Qt::NoContextMenu);
    gestionTypIntervButton  ->setStyleSheet(QStringLiteral("border: 0px"));
    connect(gestionTypIntervButton, &QPushButton::clicked, dlg_intervention,   [=]
    {
        TypeIntervention *typ = Q_NULLPTR;
        UpStandardItem *itmitv = dynamic_cast<UpStandardItem*>(m_typeinterventionsmodel->item(interventioncombo->currentIndex()));
        if (itmitv)
            if (itmitv->item())
                typ = qobject_cast<TypeIntervention*>(itmitv->item());
        dlg_listetypesinterventions *dlgtyp = new dlg_listetypesinterventions(typ, dlg_intervention);
        if (dlgtyp->exec() == QDialog::Accepted)
        {
            m_typeinterventionsmodel = Datas::I()->typesinterventions->listetypesinterventionsmodel(true);
            interventioncombo       ->setModel(m_typeinterventionsmodel);
            interventioncombo       ->setCompleter(Datas::I()->typesinterventions->completer());
            typ = dlgtyp->currenttype();
            if (typ)
                interventioncombo->setCurrentIndex(interventioncombo->findText(typ->typeintervention()));
        }
        delete dlgtyp;
    });

    choixinterventionLay    ->addWidget(lblinterv);
    choixinterventionLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixinterventionLay    ->addWidget(gestionTypIntervButton);
    choixinterventionLay    ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Fixed,QSizePolicy::Fixed));
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
    wdg_IOLchk                  = new QCheckBox(tr("Utiliser un implant"));
    wdg_IOLchk                  ->setCheckState(Qt::Unchecked);
    wdg_choixIOLbutt            = new UpPushButton(tr("Choisir un implant"));
    checkIOLLay                 ->addWidget(wdg_IOLchk);
    checkIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkIOLLay                 ->addWidget(wdg_choixIOLbutt);
    wdg_choixIOLbutt            ->setVisible(false);
    checkIOLLay                 ->setSpacing(5);
    checkIOLLay                 ->setContentsMargins(0,0,0,0);

    QWidget *wdg_IOL            = new QWidget();
    QVBoxLayout *box_IOLlay     = new QVBoxLayout();
    box_IOLlay->setSpacing(5);

            //! FABRICANT
    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    wdg_manufacturercombo       ->setFixedSize(QSize(200,28));
    wdg_manufacturercombo       ->setEditable(true);
    wdg_manufacturercombo       ->setInsertPolicy(QComboBox::NoInsert);
    choixManufacturerIOLLay     ->addWidget(lblManufacturerIOL);
    choixManufacturerIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerIOLLay     ->addWidget(wdg_manufacturercombo);
    choixManufacturerIOLLay     ->setSpacing(5);
    choixManufacturerIOLLay     ->setContentsMargins(0,0,0,0);

    connect(wdg_IOLchk, &QCheckBox::stateChanged,       dlg_intervention,   [&](int state) {if (state == Qt::Checked) ReconstruitListeManufacturers();});
    connect(wdg_choixIOLbutt,   &QPushButton::clicked,  this,               [&]{
                int idiol = 0;
                dlg_listeiols *Dlg_ListIOLs = new dlg_listeiols(true, dlg_intervention);
                if (Dlg_ListIOLs->exec() == QDialog::Accepted)
                {
                    idiol = Dlg_ListIOLs->idcurrentIOL();
                    if (Dlg_ListIOLs->listeIOLsmodifiee())
                        RemplirTreeInterventions();
                }
                if (idiol > 0)
                {
                    IOL *iol = Datas::I()->iols->getById(idiol);
                    if (iol)
                    {
                        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                        if (man != Q_NULLPTR)
                            wdg_manufacturercombo->setCurrentIndex(wdg_manufacturercombo->findData(man->id()));
                        wdg_IOLcombo->setCurrentIndex(wdg_IOLcombo->findData(idiol));
                    }
                }
                delete Dlg_ListIOLs;
            });

            //! MODELE
    QHBoxLayout *choixIOLLay    = new QHBoxLayout();
    UpLabel* lblIOL             = new UpLabel;
    lblIOL                      ->setText(tr("Implant"));
    wdg_IOLcombo                ->setFixedSize(QSize(200,28));
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
    wdg_pwrIOLspinbox               = new UpDoubleSpinBox();
    wdg_pwrIOLspinbox               ->setSingleStep(0.50);
    wdg_pwrIOLspinbox               ->setValuewithPrefix(21.00);
    wdg_pwrIOLspinbox               ->setFixedSize(QSize(70,28));
    wdg_pwrIOLspinbox               ->installEventFilter(this);
    choixPwrIOLLay                  ->addWidget(lblPwrIOL);
    choixPwrIOLLay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixPwrIOLLay                  ->addWidget(wdg_pwrIOLspinbox);
    choixPwrIOLLay                  ->setSpacing(5);
    choixPwrIOLLay                  ->setContentsMargins(0,0,0,0);

            //! CYLINDRE
    wdg_choixcylwdg                 = new QWidget();
    QHBoxLayout *choixCylIOLLay     = new QHBoxLayout();
    UpLabel* lblCylIOL              = new UpLabel;
    lblCylIOL                       ->setText(tr("Cylindre"));
    wdg_cylIOLspinbox               = new UpDoubleSpinBox();
    wdg_cylIOLspinbox               ->setSingleStep(0.50);
    wdg_cylIOLspinbox               ->setValue(0.00);
    wdg_cylIOLspinbox               ->setFixedSize(QSize(70,28));
    wdg_cylIOLspinbox               ->installEventFilter(this);
    choixCylIOLLay                  ->addWidget(lblCylIOL);
    choixCylIOLLay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixCylIOLLay                  ->addWidget(wdg_cylIOLspinbox);
    choixCylIOLLay                  ->setSpacing(5);
    choixCylIOLLay                  ->setContentsMargins(0,0,0,0);
    wdg_choixcylwdg                 ->setLayout(choixCylIOLLay);


    connect(wdg_IOLchk, &QCheckBox::stateChanged, dlg_intervention, [&] { wdg_IOL->setVisible(wdg_IOLchk->isChecked()); wdg_choixIOLbutt->setVisible(wdg_IOLchk->isChecked());});
    CalcRangeBox(m_currentIOL);

    box_IOLlay      ->insertWidget(0, wdg_choixcylwdg);
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

    if (interv != Q_NULLPTR)                        /*! On modidife une intervention */
    {
        timeedit->setTime(interv->heure());
        TypeIntervention *typ = Datas::I()->typesinterventions->getById(interv->idtypeintervention());
        if (typ)
            interventioncombo->setCurrentIndex(interventioncombo->findText(typ->typeintervention()));
        cotecombo->setCurrentIndex(cotecombo->findData(Utils::ConvertCote(interv->cote())));
        anesthcombo->setCurrentIndex(anesthcombo->findData(Intervention::ConvertModeAnesthesie(interv->anesthesie())));
        ObservtextEdit->setText(interv->observation());
        if (interv->idIOL() > 0)
        {
            wdg_IOL->setVisible(true);
            wdg_IOLchk->setChecked(true);
            IOL *iol = Datas::I()->iols->getById(interv->idIOL());
            Manufacturer *man = Q_NULLPTR;
            if (iol != Q_NULLPTR)
            {
                int idiol = iol->id();
                man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                if (man != Q_NULLPTR)
                    wdg_manufacturercombo->setCurrentIndex(wdg_manufacturercombo->findData(man->id()));
                wdg_IOLcombo->setCurrentIndex(wdg_IOLcombo->findData(idiol));
            }
            wdg_pwrIOLspinbox->setValuewithPrefix(interv->puissanceIOL());
            wdg_cylIOLspinbox->setValuewithPrefix(interv->cylindreIOL());
            wdg_choixIOLbutt->setVisible(true);
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
    dlg_intervention->dlglayout()   ->insertLayout(0, titreLay);
    dlg_intervention->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_intervention->dlglayout()   ->setSpacing(5);

    dlg_intervention->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    disconnect(dlg_intervention->CancelButton,   &QPushButton::clicked, dlg_intervention, &UpDialog::reject);
    connect(dlg_intervention->OKButton, &QPushButton::clicked, dlg_intervention, [=, &dlg_intervention]
    {
        qApp->focusWidget()->clearFocus();
        if (!timeedit->time().isValid())
        {
            UpMessageBox::Watch(dlg_intervention, tr("Vous n'avez pas spécifié une heure valide"));
            return;
        }
        if (interventioncombo->currentText() == "")
        {
            UpMessageBox::Watch(dlg_intervention, tr("Vous n'avez pas spécifié le type d'intervention"));
            return;
        }
        if (anesthcombo->currentText() == "" || anesthcombo->currentIndex() == -1)
        {
            UpMessageBox::Watch(dlg_intervention, tr("Vous n'avez pas spécifié le type d'anesthésie"));
            return;
        }
        if (wdg_IOLchk->isChecked())
        {
            if (Datas::I()->manufacturers->getById(wdg_manufacturercombo->currentData().toInt()) == Q_NULLPTR)
            {
                UpMessageBox::Watch(dlg_intervention, tr("Ce fabricant n'est pas retrouvé"));
                return;
            }
            if (Datas::I()->iols->getById(wdg_IOLcombo->currentData().toInt()) == Q_NULLPTR)
            {
                UpMessageBox::Watch(dlg_intervention, tr("Cet implant n'est pas retrouvé"));
                return;
            }
        }
        if (pat != Q_NULLPTR)
            if (pat->telephone() == "" && pat->portable() == "")
            {
                if (!Patients::veriftelephone(pat, dlg_intervention))
                    return;
                else
                    emit updateHtml(pat);
            }
        QTime heure = timeedit->time();
        QStandardItem *itm = m_sessionsmodel->itemFromIndex(sessioncombo->model()->index(sessioncombo->currentIndex(),0));
        UpStandardItem *upitm = dynamic_cast<UpStandardItem*>(itm);
        int idsession = 0;
        if (upitm == Q_NULLPTR)
            return;
        idsession = upitm->item()->id();
        SessionOperatoire * session = qobject_cast<SessionOperatoire*>(upitm->item());
        if (session == Q_NULLPTR)
            return;
        int idpat = 0;
        if (pat)
            idpat = pat->id();
        int idact = DataBase::I()->getidActeCorrespondant(idpat, session->date());
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
        listbinds[CP_IDACTE_LIGNPRGOPERATOIRE]  = (idact > 0? idact : QVariant());
        if (wdg_IOLchk->isChecked())
        {
            listbinds[CP_IDIOL_LIGNPRGOPERATOIRE] = m_currentIOL->id();
            listbinds[CP_PWRIOL_LIGNPRGOPERATOIRE] = wdg_pwrIOLspinbox->value();
            listbinds[CP_CYLIOL_LIGNPRGOPERATOIRE] = (wdg_cylIOLspinbox->isVisible()? wdg_cylIOLspinbox->value() : QVariant());
        }
        else
        {
            listbinds[CP_IDIOL_LIGNPRGOPERATOIRE] = QVariant();
            listbinds[CP_PWRIOL_LIGNPRGOPERATOIRE] = QVariant();
            listbinds[CP_CYLIOL_LIGNPRGOPERATOIRE] = QVariant();
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
                        Intervention *intervention = qobject_cast<Intervention*>(itm->item());
                        if (intervention)
                        {
                            if (intervention->heure() == heure && intervention->idpatient() == idpat)
                            {
                                UpMessageBox::Watch(dlg_intervention, tr("Cette intervention existe déjà!"));
                                return;
                            }
                        }
                    }
                }
            }
            listbinds[CP_IDPATIENT_LIGNPRGOPERATOIRE]  = idpat;
            setcurrentintervention(Datas::I()->interventions->CreationIntervention(listbinds));
            RemplirTreeInterventions(currentintervention());
        }
        else                                                                                                            //! il s'agit de modifier l'intervention passée en paramètre de la fonction
        {
            int oldidsession = interv->idsession();
            DataBase::I()->UpdateTable(TBL_LIGNESPRGOPERATOIRES, listbinds, "where " CP_ID_LIGNPRGOPERATOIRE " = " + QString::number(interv->id()));
            /*
            interv->setheure(heure);
            interv->setidsession(idsession);
            interv->setidtypeintervention(idtype);
            interv->setcote(Utils::ConvertCote(cote));
            interv->setanesthesie(Intervention::ConvertModeAnesthesie(anesth));
            interv->setobservation(ObservtextEdit->toPlainText());
            interv->setidacte(idact);
            if (wdg_IOLchk->isChecked())
            {
                interv->setidIOL(m_currentIOL->id());
                interv->setpuissanceIOL(wdg_pwrIOLspinbox->value());
                interv->setpuissanceIOL(wdg_cylIOLspinbox->value());
            }
            else
            {
                interv->setidIOL(0);
                interv->setpuissanceIOL(0);
                interv->setcylindreIOL(0);
            }*/

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
                RemplirTreeInterventions(interv);
        }
        ModifStatutActeCorrespondant(idact);
        dlg_intervention->accept();
    });
    connect(interventioncombo->lineEdit(),      &QLineEdit::editingFinished,    dlg_intervention,   [&] { VerifExistIntervention(dlg_intervention, verifencours, interventioncombo); });
    connect(dlg_intervention->CancelButton,     &QPushButton::clicked,          dlg_intervention,   [=]
                                                                                                    {
                                                                                                        interventioncombo->lineEdit()->disconnect();
                                                                                                        dlg_intervention->reject();
                                                                                                    });
    timeedit->setFocus();
    dlg_intervention->exec();
    delete dlg_intervention;
}

void dlg_programmationinterventions::FicheImpressions(Patient *pat, Intervention *interv)
{
    if (pat == Q_NULLPTR || interv == Q_NULLPTR)
        return;
    dlg_impressions *Dlg_Imprs   = new dlg_impressions(pat, interv, this);
    m_docimprime = false;
    if (Dlg_Imprs->exec() == QDialog::Accepted)
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
            bool Prescription           = (mapdoc.find(dlg_impressions::d_Prescription).value() == "1");
            bool AvecDupli              = (mapdoc.find(dlg_impressions::d_Dupli).value() == "1");
            bool Administratif          = (mapdoc.find(dlg_impressions::d_Administratif).value() == "1");
            QString Titre               =  mapdoc.find(dlg_impressions::d_Titre).value();
            QString TxtDocument         =  mapdoc.find(dlg_impressions::d_Texte).value();

            QMap<int, QMap<dlg_impressions::DATASAIMPRIMER, QString>> map = Dlg_Imprs->mapdocsaimprimer();
            bool AvecChoixImprimante    = (mapdoc == map.first());            // s'il y a plusieurs documents à imprimer on détermine l'imprimante pour le premier et on garde ce choix pour les autres
            bool AvecPrevisu            = proc->ApercuAvantImpression();
            ALD                         = Dlg_Imprs->ui->ALDcheckBox->checkState() == Qt::Checked && Prescription && db->parametres()->cotationsfrance();
            Entete                      = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
            if (Entete == "") return;
            Entete.replace("{{TITRE1}}"        , "");
            Entete.replace("{{TITRE}}"         , "");
            Entete.replace("{{DDN}}"           , "");
            proc                        ->setNomImprimante(imprimante);
            m_docimprime                = proc->Imprimer_Document(this, pat, userEntete, Titre, Entete, TxtDocument, DateDoc, Prescription, ALD, AvecPrevisu, AvecDupli, AvecChoixImprimante, Administratif);
            if (!m_docimprime)
                break;
            imprimante = proc->nomImprimante();
        }
    }
    delete Dlg_Imprs;
}

void dlg_programmationinterventions::CreerFicheIntervention()
{
    FicheIntervention();
}

void dlg_programmationinterventions::ModifIntervention()
{
    if (currentintervention() != Q_NULLPTR)
        FicheIntervention(currentintervention());
}

void dlg_programmationinterventions::SupprimeIntervention()
{
    if (currentintervention() == Q_NULLPTR)
        return;
    QString nomintervention = "";
    TypeIntervention *typ = Datas::I()->typesinterventions->getById(currentintervention()->idtypeintervention());
    if (typ)
        nomintervention += typ->typeintervention();
    Patient * pat = Datas::I()->patients->getById(currentintervention()->idpatient());
    if (pat)
        nomintervention += " - " + pat->nom() + " " + pat->prenom();
    QString nomsession = currentsession()->date().toString("dd-MMM-yy");
    Site* site = Datas::I()->sites->getById(currentsession()->idlieu());
    if (site != Q_NULLPTR)
        nomsession += " - " + site->nom();
    if (UpMessageBox::Question(this, tr("Voulez-vous supprimer l'intervention"), nomintervention + "\n" + nomsession + " ?") != UpSmallButton::STARTBUTTON)
        return;
    if (currentintervention()->idacte() >0)
    {
        Acte * act = Datas::I()->actes->getById(currentintervention()->idacte(), Item::NoLoadDetails);
        if (act != Q_NULLPTR)
            act->setidintervention(0);
    }
    Intervention* interv = Datas::I()->interventions->getById(currentintervention()->id());
    if (interv)
        Datas::I()->interventions->SupprimeIntervention(interv);
    setcurrentintervention(Q_NULLPTR);
    RemplirTreeInterventions();
}

void dlg_programmationinterventions::VerifExistIntervention(UpDialog * dlg, bool &ok, QComboBox *box)
{
    if (ok) return; // c'est de la bidouille, je sais... mais pas trouvé autre chose sinon, le editingFinished est émis 2 fois en cas d'appui sur les touches Enter ou Return du combobox
    ok = true;
    QString txt = box->lineEdit()->text();
    if (m_typeinterventionsmodel->findItems(txt).size() ==0 && txt !="")
    {
        if (UpMessageBox::Question(dlg, tr("Intervention non référencée!"), tr("Voulez-vous l'enregistrer?")) != UpSmallButton::STARTBUTTON)
            return;
        else
        {
            if (m_currenttypeintervention != Q_NULLPTR)
            {
                delete m_currenttypeintervention;
                m_currenttypeintervention = Q_NULLPTR;
            }
            FicheTypeIntervention(Utils::trimcapitilize(txt));
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
        Intervention *interv = qobject_cast<Intervention*>(upitem->item());
        if (interv == Q_NULLPTR)
        {
            delete m_ctxtmenuinterventions;
            return;
        }
        QAction *pAction_ModifIntervention = m_ctxtmenuinterventions->addAction(tr("Modifier cette intervention"));
        connect (pAction_ModifIntervention,         &QAction::triggered,    this,    &dlg_programmationinterventions::ModifIntervention);
        QAction *pAction_SupprIntervention = m_ctxtmenuinterventions->addAction(tr("Supprimer cette intervention"));
        connect (pAction_SupprIntervention,         &QAction::triggered,    this,    &dlg_programmationinterventions::SupprimeIntervention);
        QAction *pAction_ImprIntervention = m_ctxtmenuinterventions->addAction(tr("Imprimer un document"));
        connect (pAction_ImprIntervention,          &QAction::triggered,    this,    [&] {FicheImpressions(Datas::I()->patients->getById(interv->idpatient()), interv);});
        if (Datas::I()->users->userconnected()->isMedecin())
        {
            QString txt = (interv->incident() != ""? tr("Modifier le rapport d'incident") : tr ("Enregistrer un incident sur cette intervention"));
            QAction *pAction_IncidentIntervention = m_ctxtmenuinterventions->addAction(txt);
            connect (pAction_IncidentIntervention,        &QAction::triggered,    this,    &dlg_programmationinterventions::EnregistreIncidentIntervention);
        }
    }
    // ouvrir le menu
    m_ctxtmenuinterventions->exec(cursor().pos());
    delete m_ctxtmenuinterventions;
}


/*! les types d'intervention ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::FicheTypeIntervention(QString txt)
{
    UpDialog            *dlg_typintervention = new UpDialog(this);

    dlg_typintervention->setWindowTitle(tr("créer un type d'intervention"));
    dlg_typintervention->setWindowModality(Qt::WindowModal);

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
        for (int i = 0; i < m_typeinterventionsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_typeinterventionsmodel->item(i));
            TypeIntervention *typ = qobject_cast<TypeIntervention*>(upitem->item());
            if (typ->typeintervention() == Utils::trimcapitilize(linenom->text()))
            {
                UpMessageBox::Watch(dlg_typintervention, tr("Ce type d'intervention existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_TYPEINTERVENTION_TYPINTERVENTION] = Utils::trimcapitilize(linenom->text());
        listbinds[CP_CODECCAM_TYPINTERVENTION]  = lineccam->text().toUpper();
        if (m_currenttypeintervention != Q_NULLPTR)
            delete m_currenttypeintervention;
        m_currenttypeintervention = Datas::I()->typesinterventions->CreationTypeIntervention(listbinds);
        m_typeinterventionsmodel = Datas::I()->typesinterventions->listetypesinterventionsmodel(true);
        dlg_typintervention->accept();
    });
    dlg_typintervention->exec();
    delete dlg_typintervention;
}


/*! les IOLs ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::CalcRangeBox(IOL *iol)
{
    double actualpwr    = wdg_pwrIOLspinbox->value();
    double actualpwrcyl = wdg_cylIOLspinbox->value();
    bool pwrdefined = false;
    bool cyldefined = false;
    if (iol)
    {
        wdg_pwrIOLspinbox->setMinimum(iol->pwrmin());
        if (iol->pwrmax() > 0.0)
            wdg_pwrIOLspinbox->setMaximum(iol->pwrmax());
        wdg_choixcylwdg->setVisible(iol->istoric());
        if ((actualpwr > iol->pwrmin() && actualpwr < iol->pwrmax())
                || (iol->pwrmin() == 0 && iol->pwrmax() ==  0))
        {
            pwrdefined = true;
            wdg_pwrIOLspinbox->setValue(actualpwr);
        }
        else
        {
            double midvalue = (iol->pwrmin() + iol->pwrmax()) /2;
            midvalue = Utils::roundToNearestPointFifty(midvalue);
            if (midvalue != 0.0)
            {
                pwrdefined = true;
                wdg_pwrIOLspinbox->setValue(midvalue);
            }
        }
        if (iol->istoric())
        {
            wdg_cylIOLspinbox->setMinimum(iol->cylmin());
            if (iol->cylmax() > 0.0)
                wdg_cylIOLspinbox->setMaximum(iol->cylmax());
            if (actualpwrcyl > iol->cylmin() && actualpwr < iol->cylmax())
                wdg_pwrIOLspinbox->setValue(actualpwrcyl);
            else
            {
                double midvalue = Utils::roundToNearestPointFifty((iol->cylmin() + iol->cylmax()) /2);
                if (midvalue != 0.0)
                {
                    cyldefined = true;
                    wdg_cylIOLspinbox->setValue(midvalue);
                }
            }
        }
        else {
            wdg_cylIOLspinbox->setValue(0.00);
        }
    }
    if (!pwrdefined)
    {
        wdg_pwrIOLspinbox->setRange(-10.00, 35.00);
        wdg_cylIOLspinbox->setRange(0.0, 10.0);
        wdg_pwrIOLspinbox->setValue(21.00);
    }
    if (!cyldefined)
        wdg_cylIOLspinbox->setValue(0.00);
}

void dlg_programmationinterventions::FicheListeIOLs()
{
    bool quelesactifs = false;
    dlg_listeiols *Dlg_ListIOLs = new dlg_listeiols(quelesactifs, this);
    Dlg_ListIOLs->exec();
    if (Dlg_ListIOLs->listeIOLsmodifiee())
        RemplirTreeInterventions();
    delete Dlg_ListIOLs;
}

void dlg_programmationinterventions::ImprimeListeIOLsSession()
{
    if (Datas::I()->interventions->interventions()->size() == 0)
        return;
    QList<Manufacturer*> listmanufacturers;
    QList<Manufacturer*> listdistributeurs;
    double c = CORRECTION_td_width;
    bool AvecDupli   = false;
    bool AvecPrevisu = proc->ApercuAvantImpression();
    bool AvecNumPage = true;
    foreach (Intervention *interv, *Datas::I()->interventions->interventions())
    {
        if (interv->idIOL() >0)
        {
            IOL *iol = Datas::I()->iols->getById(interv->idIOL());
            Manufacturer *man = Q_NULLPTR;
            if (iol != Q_NULLPTR)
            {
                man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                if (man != Q_NULLPTR)
                {
                    if (listmanufacturers.indexOf(man) == -1)
                        listmanufacturers << man;
                    Manufacturer *mandistri = Q_NULLPTR;
                    mandistri = Datas::I()->manufacturers->getById(man->iddistributeur());
                    if (mandistri)
                    {
                        iol->setidistributeur(mandistri->id());
                        if (listdistributeurs.indexOf(mandistri) == -1)
                            listdistributeurs << mandistri;
                    }
                    else
                    {
                        iol->setidistributeur(man->id());
                        if (listdistributeurs.indexOf(man) == -1)
                            listdistributeurs << man;
                    }
                }
            }
        }
    }
    foreach (Manufacturer *man, listdistributeurs)
    {
        //--------------------------------------------------------------------
        // Préparation de l'état "liste des implants pour un fabricant" dans un QplainTextEdit
        //--------------------------------------------------------------------

        int iduser = currentsession()->iduser();

        //création de l'entête
        QString EnTete;
        User *userEntete = Datas::I()->users->getById(iduser);
        if(userEntete == Q_NULLPTR)
            return;
        EnTete = proc->CalcEnteteImpression(m_currentdate, userEntete).value("Norm");
        if (EnTete == "") return;

        EnTete.replace("{{TITRE1}}"            , "");
        EnTete.replace("{{PRENOM PATIENT}}"    , "");
        EnTete.replace("{{NOM PATIENT}}"       , "");
        EnTete.replace("{{TITRE}}"             , "");
        EnTete.replace("{{DDN}}"               , "");

        // création du pied
        QString Pied = proc->CalcPiedImpression(userEntete);
        if (Pied == "") return;

        // creation du corps
        QString lign =  "<p align=\"center\"><font color = " COULEUR_TITRES "><span style=\"font-size:10pt;\"><b>" + tr("COMMANDE D'IMPLANTS INTRAOCULAIRES") + "</b></span></font>" ;
        Site *site = Datas::I()->sites->getById(currentsession()->idlieu());
        QString sitadresse = "";
        if (site)
            sitadresse = site->nom() + " - " + site->coordonnees();

        QString date = tr("Programme opératoire du") + " " + currentsession()->date().toString("dddd dd MMMM yyyy");
        QString cor = man->coordonnees().replace("\n", "<br>");
        lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b>" + cor + "</b></span></td>" ;
        lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b></b></span></td>";
        lign += "<p align=\"left\"><font color = " COULEUR_TITRES "><span style=\"font-size:8pt;\"><b>" + sitadresse + "</b></span></td>" ;
        lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b>" + date + "</b></span></td>" ;
        lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\"><b></b></span></td>";
        foreach (Intervention *interv, *Datas::I()->interventions->interventions())
        {
            if (interv->idIOL() >0)
            {
                IOL *iol = Datas::I()->iols->getById(interv->idIOL());
                Manufacturer *maniol = Q_NULLPTR;
                if (iol != Q_NULLPTR)
                {
                    if (iol->iddistributeur() == man->id())
                    {
                        maniol = Datas::I()->manufacturers->getById(iol->idmanufacturer());
                        if (maniol != Q_NULLPTR)
                        {
                            QString ioltxt = maniol->nom().toUpper() + " - " + iol->modele() + " ";
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
                            lign += HTML_RETOURLIGNE "<td width=\"" + QString::number(int(c*30)) + "\"></td><td width=\"" + QString::number(int(c*300)) + "\"><span style=\"font-size:8pt;\">" + ioltxt + "</span></td>" ;
                        }
                    }
                }
            }
        }
        QTextEdit textEdit;
        textEdit.setHtml(lign);
        proc->Imprime_Etat(this, &textEdit, EnTete, Pied,
                           proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                           AvecDupli, AvecPrevisu, AvecNumPage);
    }
}

void dlg_programmationinterventions::ReconstruitListeIOLs(int idmanufacturer, int idiol)
{
    m_currentIOL = Q_NULLPTR;
    m_IOLcompleterlist.clear();
    wdg_IOLcombo->disconnect();
    wdg_IOLcombo->clear();
    if (m_IOLsmodel != Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);
    foreach (IOL* iol, *Datas::I()->iols->iols())
    {
        QList<QStandardItem *> items;
        if (iol->idmanufacturer() == idmanufacturer && iol->isactif())
        {
            UpStandardItem *itemiol = new UpStandardItem(iol->modele(), iol);
            UpStandardItem *itemid  = new UpStandardItem(QString::number(iol->id()), iol);
            items << itemiol << itemid;
            m_IOLsmodel->appendRow(items);
            m_IOLcompleterlist << iol->modele();
        }
    }
    if (m_IOLsmodel->rowCount() > 0)
    {
        if (m_IOLsmodel->rowCount() > 1)
        {
            m_IOLsmodel->sort(0, Qt::AscendingOrder);
            m_IOLcompleterlist.sort();
        }
        if (idiol > 0)
            m_currentIOL = Datas::I()->iols->getById(idiol);
        else
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_IOLsmodel->item(0));
            if (itm)
                m_currentIOL = qobject_cast<IOL*>(itm->item());
        }
        for (int i=0; i< m_IOLsmodel->rowCount(); ++i)
        {
            wdg_IOLcombo->addItem(m_IOLsmodel->item(i,0)->text());              //! le modèle de l'IOL
            wdg_IOLcombo->setItemData(i, m_IOLsmodel->item(i,1)->text());       //! l'id en data
        }
        if(wdg_IOLcombo->completer())
            delete wdg_IOLcombo->completer();
        QCompleter * m_completer = new QCompleter(m_IOLcompleterlist);
        m_completer->setCaseSensitivity(Qt::CaseInsensitive);
        m_completer->setCompletionMode(QCompleter::InlineCompletion);
        wdg_IOLcombo->setCompleter(m_completer);

        if (m_currentIOL)
        {
            int id = m_currentIOL->id();
            int idx = wdg_IOLcombo->findData(id);
            wdg_IOLcombo->setCurrentIndex(idx);
            CalcRangeBox(m_currentIOL);
        }
        connect(wdg_IOLcombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [&] (int idx)   {
                                                                                                                    m_currentIOL = Datas::I()->iols->getById(wdg_IOLcombo->itemData(idx).toInt());
                                                                                                                    CalcRangeBox(m_currentIOL);
                                                                                                                });
        connect(wdg_IOLcombo,   QOverload<int>::of(&QComboBox::highlighted),            this,   [&] (int id)    {
                                                                                                                    IOL * iol = Datas::I()->iols->getById(wdg_IOLcombo->itemData(id).toInt());
                                                                                                                    if (iol)
                                                                                                                        QToolTip::showText(cursor().pos(),iol->tooltip(true));
                                                                                                                });
    }
}

/*! les fabricants ----------------------------------------------------------------------------------------------------------------------------------------------------*/

void dlg_programmationinterventions::ChoixManufacturer(int idx)
{
    int id = wdg_manufacturercombo->itemData(idx).toInt();
    m_currentmanufacturer = Datas::I()->manufacturers->getById(id);
    if (m_currentmanufacturer != Q_NULLPTR)
        ReconstruitListeIOLs(m_currentmanufacturer->id());
}

void dlg_programmationinterventions::FicheListeManufacturers()
{
    if (Datas::I()->manufacturers->manufacturers()->size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de fournisseur enregistré") );
        dlg_identificationmanufacturer *Dlg_IdentManufacturer    = new dlg_identificationmanufacturer(dlg_identificationmanufacturer::Creation, Q_NULLPTR, this);
        Dlg_IdentManufacturer->setWindowModality(Qt::WindowModal);
        Dlg_IdentManufacturer->exec();
        delete Dlg_IdentManufacturer;
        return;
    }
    dlg_listemanufacturers *Dlg_ListManufacturers = new dlg_listemanufacturers(this);
    Dlg_ListManufacturers->setWindowModality(Qt::WindowModal);
    Dlg_ListManufacturers->exec();
    if (Dlg_ListManufacturers->listemanufacturersmodifiee())
        AfficheInterventionsSession(wdg_sessionstreeView->currentIndex());
    delete Dlg_ListManufacturers;
}

void dlg_programmationinterventions::ReconstruitListeManufacturers(int idmanufacturer)
{
    m_currentmanufacturer = Q_NULLPTR;
    m_manufacturercompleterlist.clear();
    wdg_manufacturercombo->disconnect();
    wdg_manufacturercombo->clear();
    if (m_manufacturersmodel == Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);
    QList<int> listidmanufacturer;
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != Q_NULLPTR)
        {
            if (man->isactif())
                if (!listidmanufacturer.contains(man->id()))
                {
                    listidmanufacturer << man->id();
                    QList<QStandardItem *> items;
                    UpStandardItem *itemman = new UpStandardItem(man->nom(), man);
                    UpStandardItem *itemid = new UpStandardItem(QString::number(man->id()), man);
                    items << itemman << itemid;
                    m_manufacturersmodel->appendRow(items);
                    m_manufacturercompleterlist << man->nom();
                }
        }
    }

    if (m_manufacturersmodel->rowCount() > 0)
    {
        if (m_manufacturersmodel->rowCount() > 1)
        {
            m_manufacturersmodel->sort(0, Qt::AscendingOrder);
            m_manufacturercompleterlist.sort();
        }
        if (idmanufacturer > 0)
            m_currentmanufacturer = Datas::I()->manufacturers->getById(idmanufacturer);
        else
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(0));
            if (itm)
                m_currentmanufacturer = qobject_cast<Manufacturer*>(itm->item());
        }
        for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
        {
            wdg_manufacturercombo->addItem(m_manufacturersmodel->item(i,0)->text());            //! le nom du fabricant
            wdg_manufacturercombo->setItemData(i, m_manufacturersmodel->item(i,1)->text());     //! l'id en data
        }
        if(wdg_manufacturercombo->completer())
            delete wdg_IOLcombo->completer();
        QCompleter * m_completer = new QCompleter(m_manufacturercompleterlist);
        m_completer->setCaseSensitivity(Qt::CaseInsensitive);
        m_completer->setCompletionMode(QCompleter::InlineCompletion);
        wdg_IOLcombo->setCompleter(m_completer);
        if (m_currentmanufacturer)
        {
            wdg_manufacturercombo->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
            ReconstruitListeIOLs(m_currentmanufacturer->id());
        }
        connect(wdg_manufacturercombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [&] (int idx) { ChoixManufacturer(idx); });
    }
}

