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

#include "dlg_listeiols.h"

dlg_listeiols::dlg_listeiols(bool onlyactifs, QWidget *parent) :
    UpDialog(parent)
{
    m_onlyactifs = onlyactifs;
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Liste des IOLS -  d'après www.iolcon.org"));

    wdg_itemstree = new QTreeView(this);
    wdg_itemstree ->setFixedWidth(320);
    wdg_itemstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_itemstree ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    wdg_itemstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_itemstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_itemstree ->setAnimated(true);
    wdg_itemstree ->setIndentation(10);
    wdg_itemstree ->setMouseTracking(true);
    wdg_itemstree ->header()->setVisible(false);

    wdg_buttonframe         = new WidgetButtonFrame(wdg_itemstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe         ->addSearchLine();

    AjouteLayButtons(UpDialog::ButtonPdf | UpDialog::ButtonCancel | UpDialog::ButtonOK);

    UpLabel* searchlbl          = new UpLabel;
    searchlbl                   ->setText(tr("Recherche d'implant"));
    QHBoxLayout *titrelay       = new QHBoxLayout();
    titrelay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    titrelay                    ->addWidget(searchlbl);
    titrelay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    UpLabel* Manufacturerlbl    = new UpLabel;
    Manufacturerlbl             ->setText(tr("Fabricant"));
    wdg_manufacturerscombo = new UpComboBox();
    wdg_manufacturerscombo->setEditable(false);
    wdg_manufacturerscombo->setFixedSize(250,30);
    QHBoxLayout *manufacturerlay = new QHBoxLayout();
    manufacturerlay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    manufacturerlay     ->addWidget(wdg_manufacturerscombo);
    manufacturerlay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    manufacturerlay     ->setContentsMargins(0,0,0,0);
    UpLabel* Typelbl            = new UpLabel;
    Typelbl                     ->setText(tr("Type"));
    wdg_typebox                 = new UpComboBox;
    wdg_typebox                 ->setEditable(false);
    wdg_typebox                 ->addItems(QStringList() << tr("Tous") << IOL::typeStringtotr(IOL_CP) << IOL::typeStringtotr(IOL_CA) << IOL::typeStringtotr(IOL_ADDON)
                                        << IOL::typeStringtotr(IOL_IRIEN) << IOL::typeStringtotr(IOL_CAREFRACTIF) << IOL::typeStringtotr(IOL_AUTRE));
    wdg_typebox                 ->setFixedWidth(180);
    wdg_prechargechk            = new UpCheckBox(tr("Prechargé"));
    wdg_jaunechk                = new UpCheckBox(tr("Jaune"));
    wdg_clairchk                = new UpCheckBox(tr("Clair"));
    wdg_toricchk                = new UpCheckBox(tr("Torique"));
    QHBoxLayout *colorlay       = new QHBoxLayout();
    colorlay                    ->addWidget(wdg_jaunechk);
    colorlay                    ->addWidget(wdg_clairchk);
    colorlay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    colorlay                    ->setContentsMargins(0,0,0,0);
    colorlay                    ->setSpacing(20);
    wdg_singlepiecechk          = new UpCheckBox(tr("Monobloc"));
    wdg_twopiecechk             = new UpCheckBox(tr("Anses rapportées"));
    QHBoxLayout *singlepiecelay = new QHBoxLayout();
    singlepiecelay              ->addWidget(wdg_singlepiecechk);
    singlepiecelay              ->addWidget(wdg_twopiecechk);
    singlepiecelay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    singlepiecelay              ->setContentsMargins(0,0,0,0);
    singlepiecelay              ->setSpacing(20);
    QHBoxLayout *typelay        = new QHBoxLayout();
    wdg_edofchk                 = new UpCheckBox("EDOF");
    wdg_multifocalchk           = new UpCheckBox(tr("Multifocal"));
    wdg_monofocalchk            = new UpCheckBox(tr("Monofocal"));
    typelay                     ->addWidget(wdg_edofchk);
    typelay                     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    typelay                     ->addWidget(wdg_multifocalchk);
    typelay                     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    typelay                     ->addWidget(wdg_monofocalchk);
    typelay                     ->setContentsMargins(0,0,0,0);
    wdg_pwrchk                  = new UpCheckBox(tr("Filtrer par puissance"));
    QVBoxLayout *pwrlay         = new QVBoxLayout();
    wdg_rangepwrslider          = new RangeSlider(Qt::Horizontal);
    QString maxpwr = IOL_PWRMAX;
    int max = maxpwr.toInt();
    QString minpwr = IOL_PWRMIN;
    int min = minpwr.toInt();
    wdg_rangepwrslider          ->SetRange(min*10,max*10);
    wdg_rangepwrslider          ->SetLowerValue(m_minpwr*10);
    wdg_rangepwrslider          ->SetUpperValue(m_maxpwr*10);
    QHBoxLayout *rangelay       = new QHBoxLayout();
    wdg_minpwrlbl               = new UpLabel;
    wdg_maxpwrlbl               = new UpLabel;
    wdg_minpwrlbl               ->setFixedSize(50,28);
    wdg_maxpwrlbl               ->setFixedSize(50,28);
    wdg_minpwrlbl               ->setText(Utils::PrefixePlus(double(wdg_rangepwrslider->LowerValue())/10));
    wdg_maxpwrlbl               ->setText(Utils::PrefixePlus(double(wdg_rangepwrslider->UpperValue())/10));
    rangelay                    ->addWidget(wdg_minpwrlbl);
    rangelay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    rangelay                    ->addWidget(wdg_maxpwrlbl);
    rangelay                    ->setContentsMargins(0,0,0,0);
    QVBoxLayout *Vrangelay      = new QVBoxLayout();
    Vrangelay                   ->addWidget(wdg_rangepwrslider);
    Vrangelay                   ->addLayout(rangelay);
    Vrangelay                   ->setContentsMargins(0,0,0,0);
    Vrangelay                   ->setSpacing(0);
    wdg_pwrslider               ->setLayout(Vrangelay);
    wdg_pwrslider               ->setEnabled(false);

    pwrlay                      ->addWidget(wdg_pwrchk);
    pwrlay                      ->addWidget(wdg_pwrslider);
    pwrlay                      ->setContentsMargins(0,0,0,0);
    pwrlay                      ->setSpacing(5);
    wdg_annulfiltresbut         = new UpPushButton(tr("Annuler les filtres"));


    QHBoxLayout *globallay  = new QHBoxLayout();
    QVBoxLayout *searchLay  = new QVBoxLayout();
    searchLay               ->addLayout(titrelay);
    searchLay               ->addWidget(Manufacturerlbl);
    searchLay               ->addLayout(manufacturerlay);
    searchLay               ->addWidget(Typelbl);
    searchLay               ->addWidget(wdg_typebox);
    searchLay               ->addWidget(wdg_prechargechk);
    searchLay               ->addLayout(colorlay);
    searchLay               ->addLayout(singlepiecelay);
    searchLay               ->addWidget(wdg_toricchk);
    searchLay               ->addLayout(typelay);
    searchLay               ->addLayout(pwrlay);
    searchLay               ->addWidget(wdg_annulfiltresbut);
    searchLay               ->addWidget(m_IOLtotalcount);
    searchLay               ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding));
    searchLay               ->setSpacing(9);
    globallay               ->addLayout(searchLay);
    globallay               ->addWidget(wdg_buttonframe->widgButtonParent());

    dlglayout()->insertLayout(0,globallay);
    setFixedWidth(wdg_manufacturerscombo->width() + globallay->spacing() + wdg_itemstree->width() + dlglayout()->contentsMargins().right() + dlglayout()->contentsMargins().left());
    TuneSize(false);

    if (!Datas::I()->iols->isfull())
        Datas::I()->iols->initListe();
    ReconstruitTreeViewIOLs();

    wdg_manufacturerscombo->addItem(tr("Tous"), 0);
    for (int i=0; i<m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->rufusitem());
            if (man)
                wdg_manufacturerscombo->addItem(man->nom(), man->id());
        }
    }
    wdg_annulfiltresbut->setEnabled(false);

    wdg_edofchk         ->installEventFilter(this);
    wdg_multifocalchk   ->installEventFilter(this);
    wdg_monofocalchk    ->installEventFilter(this);
    wdg_jaunechk        ->installEventFilter(this);
    wdg_clairchk        ->installEventFilter(this);
    wdg_singlepiecechk  ->installEventFilter(this);
    wdg_twopiecechk     ->installEventFilter(this);
    wdg_pwrchk          ->installEventFilter(this);

    PdfButton->setVisible(false); //! this button is here for testing import Iols from iolcon database. For testin, turn Visible property to true

    connect(OKButton,                       &QPushButton::clicked,      this,   &QDialog::accept);
    connect(PdfButton,                      &QPushButton::clicked,      this,   [=] {
                                                                                        QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
                                                                                        QString path_file_origin = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), desktop,  tr("Fichiers xml (*.xml)"));
                                                                                        if (path_file_origin != "")
                                                                                        {
                                                                                            QFile filexml(path_file_origin);
                                                                                            if (!filexml.open(QIODevice::ReadOnly))
                                                                                            {
                                                                                                UpMessageBox::Watch(this, tr("Impossible d'ouvrir le fichier") + " " + path_file_origin);
                                                                                                return;
                                                                                            }
                                                                                            QByteArray ba;
                                                                                            ba = filexml.readAll();
                                                                                            QDomDocument docxml;
                                                                                            if (!docxml.setContent(ba))   //! fichier XML invalide → on prévient au lieu d'importer un document vide
                                                                                            {
                                                                                                UpMessageBox::Watch(this, tr("Fichier XML invalide"), path_file_origin);
                                                                                                return;
                                                                                            }
                                                                                            ImportListeIOLS(docxml);
                                                                                        }
                                                                                    });
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=] (QString txt) {
                                                                                            wdg_buttonframe->searchline()->setText(txt);
                                                                                            ReconstruitTreeViewIOLs(txt);
                                                                                           });
    connect(wdg_buttonframe,                &WidgetButtonFrame::choix,  this,   &dlg_listeiols::ChoixButtonFrame);
    connect(wdg_annulfiltresbut,            &QPushButton::clicked,      this,   [&] {
                                                                                Annulerlesfiltres();
                                                                                ReconstruitTreeViewIOLs();
                                                                                wdg_annulfiltresbut->setEnabled(false);
                                                                            });
    connectFiltersSignals();

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_buttonframe->searchline()       ->setFocus();
    QString req = "select " CP_ID_IOLS " from " TBL_IOLS " where " CP_ID_IOLS " in (select " CP_IDIOL_LIGNPRGOPERATOIRE " from " TBL_LIGNESPRGOPERATOIRES ")";
    bool ok;
    QList<QVariantList> listidiols = DataBase::I()->StandardSelectSQL(req,ok);
    if (ok)
        for (int i=0; i<listidiols.size(); ++i)
            m_listidiolsutilises << listidiols.at(i).at(0).toInt();
    HasNewVersion();
    setStageCount(1);
    setEnregPosition(true);
    setSaveGeometry(Nom_fiche_ListeIOLs);
}

dlg_listeiols::~dlg_listeiols()
{
}

int dlg_listeiols::idcurrentIOL() const
{
    int idiol = (m_currentIOL? m_currentIOL->id() : 0);
    return idiol;
}

bool dlg_listeiols::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key()==Qt::Key_Space)
            //! ATTENTION : ici l'event est un QKeyEvent, PAS un QMouseEvent. L'ancien code testait
            //! dynamic_cast<QMouseEvent*>(event)->button() — le cast échouait (renvoyait nullptr) et
            //! ->button() déréférençait nullptr → plantage dès qu'on appuyait sur Espace sur une case.
            //! Espace sur une case cochable = on bascule la case, sans condition de bouton souris.
            {
                UpCheckBox *chk = qobject_cast<UpCheckBox*>(obj);
                if (chk)
                {
                    chk->setChecked(!chk->isChecked());
                    emit chk->uptoggled(chk->isChecked());  //!la propriété checked est affectée après l'event keypress, il faut donc envoyer l'inverse de la propriété pour avoir sa vraie valeur
                    return true;
                }
            }
    }
    if(event->type()==QEvent::MouseButtonPress)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
        {
            UpCheckBox *chk = qobject_cast<UpCheckBox*>(obj);
            if (chk)
            {
                chk->setChecked(!chk->isChecked());
                emit chk->uptoggled(chk->isChecked());  //!la propriété checked est affectée après l'event keypress, il faut donc envoyer l'inverse de la propriété pour avoir sa vraie valeur
                return true;
            }
        }
    return QWidget::eventFilter(obj, event);
}
void dlg_listeiols::Enablebuttons(QModelIndex idx)
{
    IOL *iol = getIOLFromIndex(idx);
    m_currentIOL = iol;
    if (iol != Q_NULLPTR)
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(true);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(m_listidiolsutilises.indexOf(iol->id()) == -1);
        wdg_buttonframe->wdg_moinsBouton->setImmediateToolTip(
            m_listidiolsutilises.indexOf(iol->id()) > -1?
                tr("Vous ne pouvez pas supprimer cet implant") + "\n" + tr("il a déjà été utilisé lors de programmations opératoires précédentes") :
                "");
    }
    else
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
    }
}

void dlg_listeiols::Annulerlesfiltres()
{
    disconnectFiltersSignals();
    wdg_manufacturerscombo  ->setCurrentIndex(0);
    wdg_typebox             ->setCurrentIndex(0);
    wdg_prechargechk        ->setChecked(false);
    wdg_jaunechk            ->setChecked(false);
    wdg_clairchk            ->setChecked(false);
    wdg_singlepiecechk      ->setChecked(false);
    wdg_twopiecechk         ->setChecked(false);
    wdg_toricchk            ->setChecked(false);
    wdg_edofchk             ->setChecked(false);
    wdg_multifocalchk       ->setChecked(false);
    wdg_monofocalchk        ->setChecked(false);   //! était oublié → la case Monofocal restait cochée après « Annuler les filtres »
    wdg_pwrchk              ->setChecked(false);
    wdg_rangepwrslider      ->SetLowerValue(m_minpwr*10);
    wdg_rangepwrslider      ->SetUpperValue(m_maxpwr*10);
    m_filterbypwr           = false;
    wdg_pwrslider           ->setEnabled(false);
    wdg_minpwrlbl           ->setText(Utils::PrefixePlus(double(wdg_rangepwrslider->LowerValue())/10));
    wdg_maxpwrlbl           ->setText(Utils::PrefixePlus(double(wdg_rangepwrslider->UpperValue())/10));
    connectFiltersSignals();
}

void dlg_listeiols::connectFiltersSignals()
{
    connect(wdg_manufacturerscombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_typebox,            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_prechargechk,       &UpCheckBox::checkStateChanged,  this,   [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_toricchk,           &UpCheckBox::checkStateChanged,  this,   [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_edofchk,            &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                      {
                                                                                        wdg_multifocalchk->setChecked(false);
                                                                                        wdg_monofocalchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_clairchk,           &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                        wdg_jaunechk->setChecked(false);
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_jaunechk,           &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                        wdg_clairchk->setChecked(false);
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_singlepiecechk,     &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                        wdg_twopiecechk->setChecked(false);
                                                                                       ReconstruitTreeViewIOLs();
                                                                                       wdg_annulfiltresbut->setEnabled(true);
                                                                                     } );
    connect(wdg_twopiecechk,        &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                        wdg_singlepiecechk->setChecked(false);
                                                                                       ReconstruitTreeViewIOLs();
                                                                                       wdg_annulfiltresbut->setEnabled(true);
                                                                                      } );
    connect(wdg_multifocalchk,      &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                      {
                                                                                        wdg_edofchk->setChecked(false);
                                                                                        wdg_monofocalchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_monofocalchk,       &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                      {
                                                                                        wdg_multifocalchk->setChecked(false);
                                                                                        wdg_edofchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_pwrchk,           &UpCheckBox::uptoggled,  this,      [=](bool a) {   wdg_pwrslider->setEnabled(a);
                                                                                      m_filterbypwr = a;
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      if (a)
                                                                                          wdg_annulfiltresbut->setEnabled(true);
                                                                                  } );
    connect(wdg_rangepwrslider,          &RangeSlider::lowerValueChanged,    this, [&](int a) {
                                                                                            m_minpwr = double(Utils::roundToNearestFive(a))/10;
                                                                                            wdg_minpwrlbl->setText(Utils::PrefixePlus(m_minpwr));
                                                                                            ReconstruitTreeViewIOLs();
                                                                                            wdg_annulfiltresbut->setEnabled(true);
                                                                                         } );
    connect(wdg_rangepwrslider,          &RangeSlider::upperValueChanged,    this, [&](int a) {
                                                                                            m_maxpwr = double(Utils::roundToNearestFive(a))/10;
                                                                                            wdg_maxpwrlbl->setText(Utils::PrefixePlus(m_maxpwr));
                                                                                            ReconstruitTreeViewIOLs();
                                                                                            wdg_annulfiltresbut->setEnabled(true);
                                                                                         } );
}
void dlg_listeiols::disconnectFiltersSignals()
{
    wdg_manufacturerscombo  ->disconnect();
    wdg_typebox             ->disconnect();
    wdg_prechargechk        ->disconnect();
    wdg_jaunechk            ->disconnect();
    wdg_clairchk            ->disconnect();
    wdg_singlepiecechk      ->disconnect();
    wdg_twopiecechk         ->disconnect();
    wdg_toricchk            ->disconnect();
    wdg_edofchk             ->disconnect();
    wdg_monofocalchk        ->disconnect();
    wdg_multifocalchk       ->disconnect();
    wdg_pwrchk              ->disconnect();   //! était oublié → ses connexions s'accumulaient à chaque remise à zéro des filtres
    wdg_rangepwrslider           ->disconnect();
}

void dlg_listeiols::ChoixButtonFrame()
{
    IOL *iol = Q_NULLPTR;
    if (wdg_itemstree->selectionModel()->hasSelection())
        iol = getIOLFromIndex(wdg_itemstree->currentIndex());
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauIOL();
        break;
    case WidgetButtonFrame::Modifier:
        if (iol)
            ModifIOL(iol);
        break;
    case WidgetButtonFrame::Moins:
        if (iol)
            SupprIOL(iol);
        break;
    }
}

bool dlg_listeiols::listeIOLsmodifiee() const
{
    return m_listemodifiee;
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::EnregistreNouveauIOL()
{
    dlg_identificationIOL *Dlg_IdentIOL    = new dlg_identificationIOL(Q_NULLPTR, this);
    if (!Dlg_IdentIOL->initok())
    {
        delete Dlg_IdentIOL;
        return;
    }
    if (Dlg_IdentIOL->exec() == QDialog::Accepted)
    {
        IOL *iol = Datas::I()->iols->getById(Dlg_IdentIOL->idcurrentIOL(), true);
        m_listemodifiee = true;
        disconnectFiltersSignals();
        wdg_manufacturerscombo->setCurrentIndex(0);
        Annulerlesfiltres();
        ReconstruitTreeViewIOLs();
        if (iol)
            scrollToIOL(iol);
    }
    delete Dlg_IdentIOL;
}
// ------------------------------------------------------------------------------------------
// renvoie l'IOL correspondant à l'index
// ------------------------------------------------------------------------------------------
IOL* dlg_listeiols::getIOLFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_IOLsmodel->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return qobject_cast<IOL *>(it->rufusitem());
    else
        return Q_NULLPTR;
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mise à jour de la liste des implants  --------------------------
-----------------------------------------------------------------------------------------------------------------*/
void dlg_listeiols::ImportListeIOLS(QDomDocument docxml)
{
    QDomElement xml = docxml.documentElement();
    /*! Mise à jour de la liste des fabricants */
    QStringList listmanufacturers;
    for (int i=0; i<xml.childNodes().size(); i++)
    {
        QDomElement lensnode = xml.childNodes().at(i).toElement();
        for (int i=0; i<lensnode.childNodes().size(); i++)
        {
            QDomElement node = lensnode.childNodes().at(i).toElement();
            if (node.tagName() == "Manufacturer")
                if (!listmanufacturers.contains(node.text()))
                    listmanufacturers << node.text();
        }
    }
    listmanufacturers.sort();
    QStringList listmanofficiel;
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *manf = const_cast<Manufacturer*>(it.value());
        if (manf->nom().toUpper() == "JOHNSON & JOHNSON VISION")
            ItemsList::update(manf, CP_NOM_MANUFACTURER, "JOHNSON AND JOHNSON VISION");
        listmanofficiel << manf->nom().toUpper();
    }
    foreach (QString nommanufacturer, listmanufacturers)
    {
        if (!listmanofficiel.contains(nommanufacturer.toUpper()))
        {
            QHash<QString,QVariant> listbinds;
            listbinds[CP_NOM_MANUFACTURER] = nommanufacturer.toUpper();
            Datas::I()->manufacturers->CreationManufacturer(listbinds);
        }
    }
    /*! fin mise à jour de la liste des fabricants */

    /*! Mise à jour des IOLS - correction des modifs de la base 1 iolcon des IOLS */
    int id1stq = 0;
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *manf = const_cast<Manufacturer*>(it.value());
        if (manf->nom().toUpper() == "1STQ")
        {
            id1stq = manf->id();
            break;
        }
    }
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        QString optmaterial = Utils::trim(iol->opticalmaterial()).toLower();
        QString Hydrof = Utils::trim(iol->hydrofily()).toLower();
        if (optmaterial.split(" ").size()>1)
        {
            QString split = optmaterial;
            Hydrof = split.split(" ").at(1).toLower();
            optmaterial = split.split(" ").at(0).toLower();
        }
        if (optmaterial == "acrylic" || optmaterial == "hydrophobic")
            optmaterial ="acrylique";
        else if (optmaterial == "copolymer")
            optmaterial = "copolymère";
        ItemsList::update(iol, CP_OPTICMATERIAU_IOLS,optmaterial);
        ItemsList::update(iol, CP_HYDROFILY_IOLS, Hydrof);
        if (!iol->isedof() && !iol->ismultifocal())
        {
            if(iol->addnear() > 0)
                ItemsList::update(iol, CP_ADDNEAR_IOLS, 0.0);
            if(iol->addintermediate() > 0)
                ItemsList::update(iol, CP_ADDINTERMEDIATE_IOLS, 0.0);
        }
        if (iol->idmanufacturer() == id1stq)
            ItemsList::update(iol, CP_SINGLEPIECE_IOLS,true);
    }
    //! copy data Physiol IOL to BVI IOL
    QMap<int, QString> physioliolmap, BVIiolmap;
    int idbvi = -1;
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *manf = const_cast<Manufacturer*>(it.value());
        if (manf->nom().toUpper() == "BVI")
            idbvi = manf->id();
    }
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        if( iol)
        {
            Manufacturer* manf = Datas::I()->manufacturers->getById(iol->idmanufacturer());
            if (manf)
            {
                if (manf->nom().toUpper() == "PHYSIOL")
                    physioliolmap.insert(iol->id(), iol->modele());
                else if (manf->nom().toUpper() == "BVI")
                    BVIiolmap.insert(iol->id(), iol->modele());
            }
        }
    }
    for (auto it = physioliolmap.cbegin(); it!= physioliolmap.cend(); ++it)
    {
        IOL *physioliol = Datas::I()->iols->getById(it.key());
        QImage img = physioliol->image();
        QString physioliolname = physioliol->modele();
        QStringList bvinameslist = BVIiolmap.values();
        if (bvinameslist.contains(physioliolname))
        {
            for (auto itbv = BVIiolmap.cbegin(); itbv!= BVIiolmap.cend(); ++itbv)
            {
                IOL *bviol = Datas::I()->iols->getById(itbv.key());
                if (bviol)
                {
                    if (bviol->modele() == Datas::I()->iols->getById(it.key())->modele())
                    {
                        bviol->setData(Datas::I()->iols->getById(itbv.key())->datas());
                        bviol->setidmanufacturer(idbvi);
                        bviol->setimage(img);
                        QHash <QString, QVariant> sets = bviol->datas().toVariantHash();
                        sets[CP_ARRAYIMG_IOLS] = physioliol->arrayimgiol();
                        DataBase::I()->UpDateImgIOL(bviol->id(), sets);
                        break;   //! le BVI de même modèle est trouvé et mis à jour : inutile de scruter les autres (était « itbv->cend() », sans effet)
                    }
                }
            }
        }
        else
        {
            QHash <QString, QVariant> sets = physioliol->datas().toVariantHash();
            sets[CP_IDMANUFACTURER_IOLS] = idbvi;
            sets[CP_ARRAYIMG_IOLS] = physioliol->arrayimgiol();
            Datas::I()->iols->CreationIOL(sets);
        }
        if (!m_listidiolsutilises.contains(physioliol->id()))
            Datas::I()->iols->SupprimeIOL(physioliol);
    }
    /*! fin mise à jour de la liste des IOLs */

    /*! Mise à jour de la liste des IOLs */
    int newiols = 0;
    int updateiols = 0;
    QStringList iolbrandmodellist= QStringList();
    QList<int> iollistid = QList<int>();
    foreach (IOL *iolfromlist, *Datas::I()->iols->iols())
    {
        if (iolfromlist != Q_NULLPTR)
        {
            Manufacturer* man = Datas::I()->manufacturers->getById(iolfromlist->idmanufacturer());
            if (man != Q_NULLPTR)
                if (!iolbrandmodellist.contains(man->nom()) && !iolbrandmodellist.contains(man->nom().toUpper()) && !iolbrandmodellist.contains(man->nom().toLower()))
                {
                    iolbrandmodellist << man->nom().toUpper() + " " + iolfromlist->modele().toUpper();
                    iollistid << iolfromlist->id();
                }
        }
    }
    for (int i=0; i<xml.childNodes().size(); i++)
    {
        QDomElement childnode = xml.childNodes().at(i).toElement();
        if (childnode.tagName() == "Lens")
        {
            IOL iol;
            iol.setidiolcon(childnode.attribute("id").toInt());
            for (int j=0; j<childnode.childNodes().size(); j++)
            {
                QDomElement Lensnode = childnode.childNodes().at(j).toElement();
                if (Lensnode.tagName() == "Manufacturer")
                    iol.setstringid(Lensnode.text());
                else if (Lensnode.tagName() == "Name")
                {
                    iol.setmodele(Lensnode.text());
                }
                else if (Lensnode.tagName() == "Specifications")
                {
                    for (int k=0; k<Lensnode.childNodes().size(); k++)
                    {
                        QDomElement Specsnode = Lensnode.childNodes().at(k).toElement();
                        if (Specsnode.tagName() == "SinglePiece")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yes")
                                b = true;
                            iol.setsinglepiece(b);
                       }
                        else if (Specsnode.tagName() == "OpticMaterial")
                        {
                            QString optmaterial = Specsnode.text();
                            if (optmaterial == "acrylic")
                                iol.setOpticalMaterial("acrylique");
                            else if (optmaterial == "copolymer")
                                iol.setOpticalMaterial("copolymère");
                            else if (optmaterial == "silicone")
                                iol.setOpticalMaterial("silicone");
                            else
                                iol.setOpticalMaterial(optmaterial);
                            //if (!listitem.contains(Specsnode.text()))
                            //    listitem << Specsnode.text();
                        }
                        else if (Specsnode.tagName() == "HapticMaterial" && !iol.issinglepiece())
                        {
                            iol.setHapticalMaterial(Specsnode.text());
                        }
                        else if (Specsnode.tagName() == "Hydro")
                        {
                            QString Hydrofily = Specsnode.text();
                            if (Hydrofily == "hydrophilic")
                                iol.setHydrofily("hydrophile");
                            else if (Hydrofily == "hydrophobic")
                                iol.setHydrofily("hydrophobe");
                            else
                                iol.setHydrofily(Hydrofily);
                        }
                        else if (Specsnode.tagName() == "Preloaded")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yes")
                                b = true;
                            iol.setpreloaded(b);
                        }
                        else if (Specsnode.tagName() == "Toric")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yes")
                                b = true;
                            iol.setToric(b);
                        }
                        else if (Specsnode.tagName() == "OpticConcept")
                        {
                            bool b = false;
                            if (Specsnode.text()=="EDoF")
                            {
                                b = true;
                                iol.setEdof(b);
                            }
                            else if (Specsnode.text()=="multifocal" || Specsnode.text()=="bifocal")
                            {
                                b = true;
                                iol.setMultifocal(b);
                            }
                        }
                        else if (Specsnode.tagName() == "Filter")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yellow")
                                b = true;
                            iol.setyellow(b);
                        }
                        else if (Specsnode.tagName() == "IncisionWidth")
                        {
                            if (Specsnode.text().toDouble() >0)
                                iol.setDiainjecteur(Specsnode.text().toDouble());
                        }
                        else if (Specsnode.tagName() == "InjectorSize")
                        {
                            if (Specsnode.text().toDouble() >0)
                                iol.setDiainjecteur(Specsnode.text().toDouble());
                        }
                        else if (Specsnode.tagName() == "OpticDiameter")
                            iol.setOpticalDiameter(Specsnode.text().toDouble());
                        else if (Specsnode.tagName() == "HapticDiameter")
                            iol.setDiaall(Specsnode.text().toDouble());
                        else if (Specsnode.tagName() == "HapticDesign")
                        {
                            if (Specsnode.text() == "Iris claw")
                                iol.setType(IOL_IRIEN);
                        }
                        else if (Specsnode.tagName() == "IntendedLocation" && iol.typeString() == "")
                        {
                            if (Specsnode.text()=="anterior chamber" || Specsnode.text()=="pre iridal")
                                iol.setType(IOL_CA);
                            else if (Specsnode.text()=="capsular bag" || Specsnode.text()=="retro iridal" || Specsnode.text()=="")
                                iol.setType(IOL_CP);
                            else if (Specsnode.text()=="sulcus ciliaris")
                                iol.setType(IOL_ADDON);
                        }
                    }
                }
                else if (Lensnode.tagName() == "Availability")
                {
                    QList<double> listpwr;
                    for (int k=0; k<Lensnode.childNodes().size(); k++)
                    {
                        QDomElement Availabilitynode = Lensnode.childNodes().at(k).toElement();
                        if (Availabilitynode.tagName() == "Sphere")
                        {
                            for (int l=0; l<Availabilitynode.childNodes().size(); l++)
                            {
                                QDomElement Spherenode = Availabilitynode.childNodes().at(l).toElement();
                                if (Spherenode.tagName() == "From")
                                    listpwr << Spherenode.text().toDouble();
                                else if (Spherenode.tagName() == "To")
                                    listpwr << Spherenode.text().toDouble();
                            }
                        }
                        if (Availabilitynode.tagName() == "Addition" && Availabilitynode.attribute("distance") == "intermediate")
                        {
                            if (Availabilitynode.text().toDouble() > 0)
                                iol.setAddintermediate(Availabilitynode.text().toDouble());
                        }
                        if (Availabilitynode.tagName() == "Addition" && Availabilitynode.attribute("distance") == "near")
                        {
                            if (Availabilitynode.text().toDouble() > 0)
                                iol.setAddnear(Availabilitynode.text().toDouble());
                        }
                    }
                    if (listpwr.size() >0)
                    {
                        auto mm = std::minmax_element(listpwr.begin(), listpwr.end());
                        iol.setPwrmin(*mm.first);
                        iol.setPwrmax(*mm.second);
                    }
                    if (iol.istoric())
                    {
                        listpwr.clear();
                        for (int k=0; k<Lensnode.childNodes().size(); k++)
                        {
                            QDomElement Availabilitynode = Lensnode.childNodes().at(k).toElement();
                            for (int l=0; l<Availabilitynode.childNodes().size(); l++)
                            {
                                if (Availabilitynode.tagName() == "Cylinder")
                                {
                                    QDomElement Cylindrenode = Availabilitynode.childNodes().at(l).toElement();
                                    if (Cylindrenode.tagName() == "From")
                                        listpwr << Cylindrenode.text().toDouble();
                                    else if (Cylindrenode.tagName() == "To")
                                        listpwr << Cylindrenode.text().toDouble();
                                    else if (Cylindrenode.tagName() == "Power")
                                        listpwr << Cylindrenode.text().toDouble();
                                }
                            }
                        }
                        if (listpwr.size() >0)
                        {
                            auto mm = std::minmax_element(listpwr.begin(), listpwr.end());
                            iol.setCylmin(*mm.first);
                            iol.setCylmax(*mm.second);
                        }
                    }
                }
                else if (Lensnode.tagName() == "Constants" && Lensnode.attribute("type")=="nominal")
                {
                    for (int i=0; i<Lensnode.childNodes().size(); i++)
                    {
                        QDomElement Constantnode = Lensnode.childNodes().at(i).toElement();
                        if (Constantnode.tagName() == "Ultrasound")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAEcho_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "SRKt")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAopt_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Haigis")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Haigisnode = Constantnode.childNodes().at(i).toElement();
                                if (Haigisnode.tagName() == "a0")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa0_nominal(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a1")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa1_nominal(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a2")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa2_nominal(Haigisnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "HofferQ")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHofferQ_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Holladay1")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHolladay1_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Barrett")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Barrettnode = Constantnode.childNodes().at(i).toElement();
                                if (Barrettnode.tagName() == "DF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettDF_nominal(Barrettnode.text().toDouble());
                                }
                                else if (Barrettnode.tagName() == "LF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettLF_nominal(Barrettnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "Olsen")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setOlsen_nominal(Constantnode.text().toDouble());
                        }
                    }
                }
                else if (Lensnode.tagName() == "Constants" && Lensnode.attribute("type")=="ULIB")
                {
                    iol.setresults_ulib(Lensnode.attribute("results").toInt());
                    for (int i=0; i<Lensnode.childNodes().size(); i++)
                    {
                        QDomElement Constantnode = Lensnode.childNodes().at(i).toElement();
                        if (Constantnode.tagName() == "Ultrasound")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAEcho_ulib(Constantnode.text().toDouble());
                        }
                        if (Constantnode.tagName() == "SRKt")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAopt_ulib(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Haigis")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Haigisnode = Constantnode.childNodes().at(i).toElement();
                                if (Haigisnode.tagName() == "a0")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa0_ulib(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a1")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa1_ulib(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a2")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa2_ulib(Haigisnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "HofferQ")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHofferQ_ulib(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Holladay1")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHolladay1_ulib(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Barrett")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Barrettnode = Constantnode.childNodes().at(i).toElement();
                                if (Barrettnode.tagName() == "DF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettDF_ulib(Barrettnode.text().toDouble());
                                }
                                else if (Barrettnode.tagName() == "LF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettLF_ulib(Barrettnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "Olsen")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setOlsen_ulib(Constantnode.text().toDouble());
                        }
                    }
                }
                else if (Lensnode.tagName() == "Constants" && Lensnode.attribute("type")=="optimized" && Lensnode.attribute("results").toInt() >0)
                {
                    iol.setresults_optimized(Lensnode.attribute("results").toInt());
                    for (int i=0; i<Lensnode.childNodes().size(); i++)
                    {
                        QDomElement Constantnode = Lensnode.childNodes().at(i).toElement();
                        if (Constantnode.tagName() == "SRKt")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAopt_optimized(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Haigis")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Haigisnode = Constantnode.childNodes().at(i).toElement();
                                if (Haigisnode.tagName() == "a0")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa0_optimized(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a1")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa1_optimized(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a2")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa2_optimized(Haigisnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "HofferQ")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHofferQ_optimized(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Holladay1")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHolladay1_optimized(Constantnode.text().toDouble());
                        }
                    }
                }
            }
            Manufacturer *man = Q_NULLPTR;
            for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
            {
                Manufacturer *manf = const_cast<Manufacturer*>(it.value());
                if (manf)
                    if (manf->nom().toUpper() == iol.stringid().toUpper())
                    {
                        man = manf;
                        break;
                    }
            }
            if (!man)
                continue;
            int idx = iolbrandmodellist.indexOf(iol.stringid().toUpper() + " " + iol.modele().toUpper());
            if (idx > -1)
            {
                //! update existant iol
                IOL* ioloflist = Datas::I()->iols->getById(iollistid.at(idx));
                if (ioloflist != Q_NULLPTR)
                {
                    //! update existant iol
                    ItemsList::update(ioloflist, CP_MODELNAME_IOLS,       iol.modele());
                    ItemsList::update(ioloflist, CP_IDMANUFACTURER_IOLS,  man->id());
                    ItemsList::update(ioloflist, CP_CSTEAECHO_IOLS,       iol.csteAEcho_nominal());

                    ItemsList::update(ioloflist, CP_CSTEAOPT_IOLS,        iol.csteAopt_nominal());
                    ItemsList::update(ioloflist, CP_HAIGISA0_IOLS,        iol.haigisa0_nominal());
                    ItemsList::update(ioloflist, CP_HAIGISA1_IOLS,        iol.haigisa1_nominal());
                    ItemsList::update(ioloflist, CP_HAIGISA2_IOLS,        iol.haigisa2_nominal());
                    ItemsList::update(ioloflist, CP_HOLL1_IOLS,           iol.holladay1_nominal());
                    ItemsList::update(ioloflist, CP_HOFFERQ_IOLS,         iol.hofferQ_nominal());
                    ItemsList::update(ioloflist, CP_BARRETTDF_IOLS,       iol.barrettDF_nominal());
                    ItemsList::update(ioloflist, CP_BARRETTLF_IOLS,       iol.barrettLF_nominal());
                    ItemsList::update(ioloflist, CP_OLSEN_IOLS,           iol.olsen_nominal());

                    ItemsList::update(ioloflist, CP_ACD_IOLS,             iol.acd());
                    ItemsList::update(ioloflist, CP_OPTICMATERIAU_IOLS,   iol.opticalmaterial());
                    ItemsList::update(ioloflist, CP_HYDROFILY_IOLS,       iol.hydrofily());
                    ItemsList::update(ioloflist, CP_HAPTICMATERIAU_IOLS,  iol.hapticalmaterial());
                    ItemsList::update(ioloflist, CP_DIAALL_IOLS,          iol.diaall());
                    ItemsList::update(ioloflist, CP_DIAOPT_IOLS,          iol.opticdiameter());
                    ItemsList::update(ioloflist, CP_DIAINJECTEUR_IOLS,    iol.diainjecteur());
                    ItemsList::update(ioloflist, CP_PRECHARGE_IOLS,       iol.ispreloaded());
                    ItemsList::update(ioloflist, CP_MAXPWR_IOLS,          iol.pwrmax());
                    ItemsList::update(ioloflist, CP_MINPWR_IOLS,          iol.pwrmin());
                    ItemsList::update(ioloflist, CP_MAXCYL_IOLS,          iol.cylmax());
                    ItemsList::update(ioloflist, CP_MINCYL_IOLS,          iol.cylmin());
                    ItemsList::update(ioloflist, CP_ADDINTERMEDIATE_IOLS, iol.addintermediate());
                    ItemsList::update(ioloflist, CP_ADDNEAR_IOLS,         iol.addnear());
                    ItemsList::update(ioloflist, CP_JAUNE_IOLS,           iol.isyellow());
                    ItemsList::update(ioloflist, CP_MULTIFOCAL_IOLS,      iol.ismultifocal());
                    ItemsList::update(ioloflist, CP_EDOF_IOLS,            iol.isedof());
                    ItemsList::update(ioloflist, CP_TORIC_IOLS,           iol.istoric());
                    ItemsList::update(ioloflist, CP_SINGLEPIECE_IOLS,     iol.issinglepiece());

                    ItemsList::update(ioloflist, CP_RESULTSU_IOLS,        iol.results_ulib());
                    ItemsList::update(ioloflist, CP_CSTEAECHOU_IOLS,      iol.csteAEcho_ulib());
                    ItemsList::update(ioloflist, CP_CSTEAOPTU_IOLS,       iol.csteAopt_ulib());
                    ItemsList::update(ioloflist, CP_HAIGISA0U_IOLS,       iol.haigisa0_ulib());
                    ItemsList::update(ioloflist, CP_HAIGISA1U_IOLS,       iol.haigisa1_ulib());
                    ItemsList::update(ioloflist, CP_HAIGISA2U_IOLS,       iol.haigisa2_ulib());
                    ItemsList::update(ioloflist, CP_HOLL1U_IOLS,          iol.holladay1_ulib());
                    ItemsList::update(ioloflist, CP_HOFFERQU_IOLS,        iol.hofferQ_ulib());
                    ItemsList::update(ioloflist, CP_BARRETTDFU_IOLS,      iol.barrettDF_ulib());
                    ItemsList::update(ioloflist, CP_BARRETTLFU_IOLS,      iol.barrettLF_ulib());
                    ItemsList::update(ioloflist, CP_OLSENU_IOLS,          iol.olsen_ulib());

                    ItemsList::update(ioloflist, CP_RESULTSO_IOLS,        iol.results_optimized());
                    ItemsList::update(ioloflist, CP_CSTEAOPTO_IOLS,       iol.csteAopt_optimized());
                    ItemsList::update(ioloflist, CP_HAIGISA0O_IOLS,       iol.haigisa0_optimized());
                    ItemsList::update(ioloflist, CP_HAIGISA1O_IOLS,       iol.haigisa1_optimized());
                    ItemsList::update(ioloflist, CP_HAIGISA2O_IOLS,       iol.haigisa2_optimized());
                    ItemsList::update(ioloflist, CP_HOLL1O_IOLS,          iol.holladay1_optimized());
                    ItemsList::update(ioloflist, CP_HOFFERQO_IOLS,        iol.hofferQ_optimized());

                    ItemsList::update(ioloflist, CP_TYP_IOLS,             iol.type());
                    ++ updateiols;
                }
            }
            else //! newiol
            {
                QHash<QString, QVariant> m_listbinds;
                m_listbinds[CP_MODELNAME_IOLS]      = iol.modele();
                m_listbinds[CP_IDMANUFACTURER_IOLS] = man->id();
                m_listbinds[CP_CSTEAECHO_IOLS]      = (iol.csteAEcho_nominal() >0.0?    iol.csteAEcho_nominal()     : QVariant());

                m_listbinds[CP_CSTEAOPT_IOLS]       = (iol.csteAopt_nominal() >0.0?     iol.csteAopt_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA0_IOLS]       = (iol.haigisa0_nominal() >0.0?     iol.haigisa0_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA1_IOLS]       = (iol.haigisa1_nominal() >0.0?     iol.haigisa1_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA2_IOLS]       = (iol.haigisa2_nominal() >0.0?     iol.haigisa2_nominal()      : QVariant());
                m_listbinds[CP_HOLL1_IOLS]          = (iol.holladay1_nominal()>0.0?     iol.holladay1_nominal()     : QVariant());
                m_listbinds[CP_HOFFERQ_IOLS]        = (iol.hofferQ_nominal()>0.0?       iol.hofferQ_nominal()       : QVariant());
                m_listbinds[CP_BARRETTDF_IOLS]      = (iol.barrettDF_nominal()>0.0?     iol.barrettDF_nominal()     : QVariant());
                m_listbinds[CP_BARRETTLF_IOLS]      = (iol.barrettLF_nominal()>0.0?     iol.barrettLF_nominal()     : QVariant());
                m_listbinds[CP_OLSEN_IOLS]          = (iol.olsen_nominal()>0.0?         iol.olsen_nominal()         : QVariant());

                m_listbinds[CP_ACD_IOLS]            = (iol.acd()>0.0?                   iol.acd()                   : QVariant());
                m_listbinds[CP_OPTICMATERIAU_IOLS]  = iol.opticalmaterial();
                m_listbinds[CP_HYDROFILY_IOLS]      = iol.hydrofily();
                m_listbinds[CP_HAPTICMATERIAU_IOLS] = iol.hapticalmaterial();
                m_listbinds[CP_DIAALL_IOLS]         = (iol.diaall() >0.0?               iol.diaall()                : QVariant());
                m_listbinds[CP_DIAOPT_IOLS]         = (iol.opticdiameter() >0.0?        iol.opticdiameter()         : QVariant());
                m_listbinds[CP_DIAINJECTEUR_IOLS]   = (iol.diainjecteur() >0.0?         iol.diainjecteur()          : QVariant());
                m_listbinds[CP_PRECHARGE_IOLS]      = (iol.ispreloaded()?               "1"                         : QVariant());
                m_listbinds[CP_MAXPWR_IOLS]         = iol.pwrmax();
                m_listbinds[CP_MINPWR_IOLS]         = iol.pwrmin();
                m_listbinds[CP_MAXCYL_IOLS]         = (iol.istoric()?                   iol.cylmax()                : QVariant());
                m_listbinds[CP_MINCYL_IOLS]         = (iol.istoric()?                   iol.cylmin()                : QVariant());
                m_listbinds[CP_ADDINTERMEDIATE_IOLS]= (iol.addintermediate() >0.0?      iol.addintermediate()       : QVariant());
                m_listbinds[CP_ADDNEAR_IOLS]        = (iol.addnear() >0.0?              iol.addnear()               : QVariant());
                m_listbinds[CP_JAUNE_IOLS]          = (iol.isyellow()?                  "1"                         : QVariant());
                m_listbinds[CP_MULTIFOCAL_IOLS]     = (iol.ismultifocal()?              "1"                         : QVariant());
                m_listbinds[CP_EDOF_IOLS]           = (iol.isedof()?                    "1"                         : QVariant());
                m_listbinds[CP_TORIC_IOLS]          = (iol.istoric()?                   "1"                         : QVariant());
                m_listbinds[CP_SINGLEPIECE_IOLS]    = (iol.issinglepiece()?             "1"                         : QVariant());

                m_listbinds[CP_RESULTSU_IOLS]       = (iol.results_ulib() >0?           iol.results_ulib()          : QVariant());
                m_listbinds[CP_CSTEAECHOU_IOLS]     = (iol.csteAEcho_ulib() >0.0?       iol.csteAEcho_ulib()        : QVariant());
                m_listbinds[CP_CSTEAOPTU_IOLS]      = (iol.csteAopt_ulib() >0.0?        iol.csteAopt_ulib()         : QVariant());
                m_listbinds[CP_HAIGISA0U_IOLS]      = (iol.haigisa0_ulib() >0.0?        iol.haigisa0_ulib()         : QVariant());
                m_listbinds[CP_HAIGISA1U_IOLS]      = (iol.haigisa1_ulib() >0.0?        iol.haigisa1_ulib()         : QVariant());
                m_listbinds[CP_HAIGISA2U_IOLS]      = (iol.haigisa2_ulib() >0.0?        iol.haigisa2_ulib()         : QVariant());
                m_listbinds[CP_HOLL1U_IOLS]         = (iol.holladay1_ulib()>0.0?        iol.holladay1_ulib()        : QVariant());
                m_listbinds[CP_HOFFERQU_IOLS]       = (iol.hofferQ_ulib()>0.0?          iol.hofferQ_ulib()          : QVariant());
                m_listbinds[CP_BARRETTDFU_IOLS]     = (iol.barrettDF_ulib()>0.0?        iol.barrettDF_ulib()        : QVariant());
                m_listbinds[CP_BARRETTLFU_IOLS]     = (iol.barrettLF_ulib()>0.0?        iol.barrettLF_ulib()        : QVariant());
                m_listbinds[CP_OLSENU_IOLS]         = (iol.olsen_ulib()>0.0?            iol.olsen_ulib()            : QVariant());

                m_listbinds[CP_RESULTSO_IOLS]       = (iol.results_optimized() >0?      iol.results_optimized()     : QVariant());
                m_listbinds[CP_CSTEAOPTO_IOLS]      = (iol.csteAopt_optimized() >0.0?   iol.csteAopt_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA0O_IOLS]      = (iol.haigisa0_optimized() >0.0?   iol.haigisa0_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA1O_IOLS]      = (iol.haigisa1_optimized() >0.0?   iol.haigisa1_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA2O_IOLS]      = (iol.haigisa2_optimized() >0.0?   iol.haigisa2_optimized()    : QVariant());
                m_listbinds[CP_HOLL1O_IOLS]         = (iol.holladay1_optimized()>0.0?   iol.holladay1_optimized()   : QVariant());
                m_listbinds[CP_HOFFERQO_IOLS]       = (iol.hofferQ_optimized()>0.0?     iol.hofferQ_optimized()     : QVariant());
                m_listbinds[CP_TYP_IOLS]            = (iol.type()>0?                    iol.type()                  : QVariant());
                Datas::I()->iols->CreationIOL(m_listbinds);
                ++newiols;
            }
        }
    }
    //for (int i = 0; i<listitem.size();++i)
    //    qDebug()<<listitem.at(i);
    /*! fin mise à jour de la liste des IOLs */

    //! Redimensionnement des images surdimensionnées : UNE SEULE FOIS, ici après l'import, et non au
    //! survol de la souris comme auparavant (ce qui écrivait en base à chaque passage de souris sur une
    //! ligne). resizeiolimage ne touche que les images bitmap dépassant le seuil (SIZEMAXIMGIOL).
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
        resizeiolimage(const_cast<IOL*>(it.value()));

    QString totaliol = QString::number(Datas::I()->iols->iols()->size());
    QString msg = "Aucun implant n'a été rajouté à la base";
    switch (newiols) {
    case 0:
        break;
    case 1:
        msg = tr("Un implant a été rajouté à la base");
        break;
    default:
        msg = QString::number(newiols) + " " + tr("implants ont été rajoutés à la base");
    }
    QString msgupdate = tr("Aucun implant n'a été mis à jour");
    switch (updateiols) {
    case 0:
        break;
    case 1:
        msgupdate += tr("Un implant a été mis à jour");
        break;
    default:
        msgupdate = QString::number(updateiols) + " " + tr("implants ont été mis à jour");
    }
    msg += "\n" + msgupdate;
    msg += "\n" + tr("Il y a") + " " + totaliol + " " + tr("implants dans la base");
    UpMessageBox::Watch(this,tr("Mise à jour de la liste des implants"),  msg);

    wdg_manufacturerscombo->clear();
    wdg_manufacturerscombo->addItem(tr("Tous"), 0);
    for (int i=0; i<m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->rufusitem());
            if (man)
                wdg_manufacturerscombo->addItem(man->nom(), man->id());
        }
    }
}

// ------------------------------------------------------------------------------------------
// Modifie un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::ModifIOL(IOL *iol)
{
    if (iol == Q_NULLPTR)
        return;
    dlg_identificationIOL *Dlg_IdentIOL = new dlg_identificationIOL(iol, this);
    if (!Dlg_IdentIOL->initok())
    {
        delete Dlg_IdentIOL;
        return;
    }
    if (Dlg_IdentIOL->exec() == QDialog::Accepted)
    {
            m_listemodifiee = true;
            wdg_manufacturerscombo->disconnect();
            wdg_manufacturerscombo->setCurrentIndex(0);
            connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); } );
    }
    iol = Datas::I()->iols->getById(Dlg_IdentIOL->idcurrentIOL(), true);
    if (iol)
    {
        ReconstruitTreeViewIOLs();
        scrollToIOL(iol);
    }
    delete Dlg_IdentIOL;
}

void dlg_listeiols::resizeiolimage(IOL *iol)
{
    int maxsizeimg = SIZEMAXIMGIOL;
    Utils::RemoveProvDir();
    //! On ne traite que les images bitmap. Il faut des && : « si le format n'est AUCUN des trois, on
    //! sort ». L'ancien code utilisait ||, condition toujours vraie (un format ne peut différer à la
    //! fois de JPG, PNG et JPEG) → la fonction sortait systématiquement et ne redimensionnait jamais rien.
    if (iol->imageformat() != JPG && iol->imageformat() != PNG && iol->imageformat() != JPEG)
        return;
    if (iol->arrayimgiol().size() < maxsizeimg)
        return;
    QString nomfichresize = Utils::ProvDir() + "/resize" + iol->modele() + "." JPG;
    QPixmap pix;
    pix.loadFromData(iol->arrayimgiol());
    /*!
    * On utilise le passage par les QPixmap parce que le mèthode suivante consistant
    * à réintégrer le QByteArray directement dans le fichier aboutit à un fichier corrompu...
    QFile file_image(nomfichresize);
    if (file_image.open(QIODevice::ReadWrite))
    {
        QByteArray ba(iol->arrayimgiol());
        QTextStream out(&file_image);
        out << ba;
    }
    */
    if (!pix.save(nomfichresize, "jpeg"))
        return;

    QString msg = "";
    if (Utils::CompressFileToJPG(nomfichresize, msg, false, maxsizeimg))
    {
        QFile file_image(nomfichresize);
        file_image.setFileName(nomfichresize);
        if (!file_image.open(QIODevice::ReadOnly))
             {
                 UpMessageBox::Watch(this, tr("Impossible d'ouvrir le fichier") + " " + nomfichresize);
                 return;
             }
        QByteArray ba = file_image.readAll();
        QHash<QString, QVariant> m_listbinds;
        m_listbinds[CP_ARRAYIMG_IOLS] = ba;
        m_listbinds[CP_TYPIMG_IOLS] = JPG;
        DataBase::I()->UpDateImgIOL(iol->id(), m_listbinds);
        Utils::removeWithoutPermissions(file_image);
    }
    Utils::RemoveProvDir();
}

void dlg_listeiols::scrollToIOL(IOL *iol)
{
    if (iol != Q_NULLPTR)
    {
        for (int i=0; i < m_IOLsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_IOLsmodel->item(i));
            if (itm)
            {
                if (itm->hasChildren())
                {
                    for (int j=0; j < itm->rowCount(); ++j)
                    {
                        UpStandardItem *childitm = dynamic_cast<UpStandardItem *>(itm->child(j));
                        if (childitm)
                            if (childitm->rufusitem())
                            {
                                IOL *siol = qobject_cast<IOL*>(childitm->rufusitem());
                                if (siol)
                                {
                                    if (siol->id() == iol->id())
                                    {
                                        wdg_itemstree->scrollTo(childitm->index(), QAbstractItemView::PositionAtCenter);
                                        wdg_itemstree->selectionModel()->select(childitm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                                        Enablebuttons(childitm->index());
                                        j = itm->rowCount();
                                        i = m_IOLsmodel->rowCount();
                                    }
                                }
                            }
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------
// Supprime un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::SupprIOL(IOL *iol)
{
    if (!iol) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer l'implant") + "\n " +
            iol->modele() + "?" +
            "\n" + tr("La suppression est IRRÉVERSIBLE.");
    UpMessageBox msgbox(this);
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton NoBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer la fiche"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        Datas::I()->iols->SupprimeIOL(iol);
        m_listemodifiee = true;
        wdg_manufacturerscombo->disconnect();
        wdg_manufacturerscombo->setCurrentIndex(0);
        connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); } );
        ReconstruitTreeViewIOLs();
    }
}

void dlg_listeiols::ReconstruitListeManufacturers()
{
    if (m_manufacturersmodel != Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);

    QStringList list;
    UpStandardItem *manufactureritem;
    QMap<int, Manufacturer*> mapman = QMap<int, Manufacturer*>();
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != Q_NULLPTR)
        {
            if (mapman.constFind(iol->idmanufacturer()) == mapman.constEnd())
            {
                if (m_onlyactifs && !iol->isactif())
                    continue;
                mapman.insert(iol->idmanufacturer(),man);
                man->setbuildIOLS(true);
            }
        }
    }
    for (auto it = mapman.constBegin(); it != mapman.constEnd(); ++it)
    {
        Manufacturer *man = const_cast<Manufacturer*>(it.value());
        list << man->nom();
        manufactureritem  = new UpStandardItem(man->nom(), man);
        manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
        manufactureritem  ->setEditable(false);
        manufactureritem  ->setEnabled(false);
        m_manufacturersmodel->appendRow(manufactureritem);
    }
    m_manufacturersmodel->sort(0);
}

void dlg_listeiols::ReconstruitTreeViewIOLs(QString filtre)
{
    int IOLtotal = 0;
    ReconstruitListeManufacturers();
    wdg_itemstree->disconnect();
    wdg_itemstree->selectionModel()->disconnect();
    if (m_IOLsmodel != Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);

    /*! filtrage du fabricant */
    int idman   = wdg_manufacturerscombo->currentData().toInt();
    UpStandardItem *pitem;
    QHash<QString, QStandardItem*> manufacturerNodes;   //! nom de fabricant → nœud parent, pour rattacher les implants en O(1)
    for (int i=0; i<  m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != Q_NULLPTR)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->rufusitem());
            if (man != Q_NULLPTR)
            {
                //! « Tous les fabricants » (idman==0) ou le seul fabricant sélectionné.
                if (idman == 0 || man->id() == idman)
                {
                    UpStandardItem *manufactureritem = new UpStandardItem(man->nom(), man);
                    manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                    manufactureritem  ->setEditable(false);
                    manufactureritem  ->setEnabled(false);
                    UpStandardItem *manufacturer2item = new UpStandardItem();
                    manufacturer2item  ->setForeground(QBrush(QColor(Qt::red)));
                    manufacturer2item  ->setEditable(false);
                    manufacturer2item  ->setEnabled(false);
                    m_IOLsmodel->appendRow(QList<QStandardItem*>() << manufactureritem << manufacturer2item);
                    manufacturerNodes.insert(man->nom(), manufactureritem);   //! index nom→nœud : évite un findItems par implant
                }
            }
        }
    }
    m_IOLsmodel->sort(0);   //! une seule fois, APRÈS la boucle (était appelé à chaque tour de boucle)
    int dim = 45;
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        if (m_onlyactifs && !iol->isactif())
            continue;
        if (iol->modele().contains(filtre, Qt::CaseInsensitive))
        {
            /*! filtrage des types */
            if (wdg_typebox->currentIndex() != 0)
            {
                if (iol->typeString() != wdg_typebox->currentText())
                    continue;
            }
            /*! filtrage des precharge */
            if (wdg_prechargechk->isChecked())
                if (!iol->ispreloaded())
                    continue;
            /*! filtrage des jaunes */
            if (wdg_jaunechk->isChecked())
                if (!iol->isyellow())
                    continue;
            /*! filtrage des clairs */
            if (wdg_clairchk->isChecked())
                if (iol->isyellow())
                    continue;
            /*! filtrage des monoblocs */
            if (wdg_singlepiecechk->isChecked())
                if (!iol->issinglepiece())
                    continue;
            /*! filtrage des anses rapportées */
            if (wdg_twopiecechk->isChecked())
                if (iol->issinglepiece())
                    continue;
            /*! filtrage des toric */
            if (wdg_toricchk->isChecked())
                if (!iol->istoric())
                    continue;
            /*! filtrage des edof */
            if (wdg_edofchk->isChecked())
                if (!iol->isedof())
                    continue;
            /*! filtrage des multifocaux */
            if (wdg_multifocalchk->isChecked())
                if (!iol->ismultifocal())
                    continue;
            /*! filtrage des monofocaux */
            if (wdg_monofocalchk->isChecked())
                if (iol->ismultifocal() || iol->isedof())
                    continue;
            /*! filtrage des puissances */
            if (m_filterbypwr)
            {
                if (iol->pwrmin() > m_minpwr || iol->pwrmax() < m_maxpwr)
                    continue;
            }
            pitem   = new UpStandardItem(iol->modele(), iol);
            if (!iol->isactif())
                pitem ->setForeground(QBrush(QColor(Qt::darkGray)));
            pitem   ->setEditable(false);
            QImage image = iol->image();
            QPixmap pix;
            QImage img2 = image.scaledToWidth(dim);
            if (img2.height()>dim)
                pix = QPixmap::fromImage(image.scaledToHeight(50));
            else
                pix = QPixmap::fromImage(img2);
            pitem->setData(pix,Qt::DecorationRole);
            Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
            if (man != Q_NULLPTR)
            {
                QStandardItem *node = manufacturerNodes.value(man->nom(), nullptr);   //! accès direct (remplace findItems, recherche linéaire)
                if (node)
                    node->appendRow(pitem);
            }
            IOLtotal ++;
        }
    }
    for (int i=0; i<m_IOLsmodel->rowCount();i++)
    {
        if (m_IOLsmodel->item(i) != Q_NULLPTR)
            if (!m_IOLsmodel->item(i)->hasChildren())
            {
                m_IOLsmodel->removeRow(i);
                i--;
            }
    }
    wdg_itemstree   ->setModel(m_IOLsmodel);
    wdg_itemstree   ->setColumnWidth(0,280);
    wdg_itemstree   ->setColumnWidth(1,30);
    wdg_itemstree   ->expandAll();
    m_treedelegate  .setHeight(dim + 5);
    wdg_itemstree   ->setItemDelegate(&m_treedelegate);
    m_IOLtotalcount ->setText(QString::number(IOLtotal) + " " + tr("implants"));
    if (m_IOLsmodel->rowCount()>0)
    {
        m_IOLsmodel->sort(0);
        m_IOLsmodel->sort(1);
        connect(wdg_itemstree,    &QAbstractItemView::entered,       this,   [=] (QModelIndex idx) { QStandardItem *item = m_IOLsmodel->itemFromIndex(idx);   //! peut être nul (index invalide) → on garde avant d'appeler hasChildren()
                                                                                                     if (item && !item->hasChildren())
                                                                                                        {
                                                                                                            IOL*iol = getIOLFromIndex(idx);
                                                                                                            if (iol)
                                                                                                            {
                                                                                                                //! Pas de redimensionnement ici : ce serait une écriture en base à CHAQUE survol
                                                                                                                //! de souris. Le redimensionnement des images surdimensionnées se fait une seule
                                                                                                                //! fois, après un import (boucle en fin de ImportListeIOLS).
                                                                                                                int i = 100;
                                                                                                                QToolTip::showText(cursor().pos(), iol->tooltip(), this, QRect(cursor().pos(),QSize(i,i)), 5000);
                                                                                                            }
                                                                                                        }
                                                                                                    } );
        connect(wdg_itemstree->selectionModel(),    &QItemSelectionModel::currentChanged,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { QStandardItem *item = m_IOLsmodel->itemFromIndex(idx);   //! peut être nul → on garde
                                                                                                     if (item && !item->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}void dlg_listeiols::HasNewVersion()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(LIEN_XML_IOLCONLASTVERSION));
    request.setUrl(QUrl(LIEN_XML_IOLCONLASTVERSION_TEST));
    QNetworkReply *reply = manager->get(request);

    connect(manager,
            &QNetworkAccessManager::finished,
            this, [=]
            {
                if(reply->error() == QNetworkReply::NoError)
                {
                    QByteArray data = reply->readAll();
                    QDomDocument docxml;
                    if (docxml.setContent(data))   //! on ne traite que si le XML est valide (sinon on ignore, au lieu de traiter un document vide en silence)
                    {
                        QDomElement xml = docxml.documentElement();
                        double lastversion = xml.attribute("fileVersion").toDouble();
                        double actualversion = DataBase::I()->parametres()->versionbaseiol();
                        if(actualversion < lastversion)
                        {
                            if (UpMessageBox::Question
                                (this,tr("Mise à jour de la liste des implants"),
                                 tr("Vous utilisez la version") + " " + QLocale(QLocale::English).toString(actualversion, 'f',1) + "\n"
                                     + tr("La version") + " " + QLocale(QLocale::English).toString(lastversion, 'f',1) + " " + tr("de la liste des implants est disponible sur le site https://iolcon.org/") + "\n"
                                     + tr("Voulez vous l'incorporer dans Rufus?") + "\n"
                                     + tr("Aucun implant de votre base actuelle ne sera modifié"))
                                == UpSmallButton::STARTBUTTON)
                            {
                                ImportListeIOLS(docxml);
                                DataBase::I()->setversionbaseiol(lastversion);
                            }
                        }
                    }
                }
                reply->deleteLater();       //! évite la fuite du reply et du manager à chaque ouverture de la boîte
                manager->deleteLater();
            });
}

