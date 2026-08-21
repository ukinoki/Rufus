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
    connect(PdfButton,                      &QPushButton::clicked,      this,   [=, this] {
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
                                                                                            Datas::I()->iols->ImportListeIOLS(docxml, this);
                                                                                        }
                                                                                    });
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=, this] (QString txt) {
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
    //! Rafraîchit le combo fabricants quand la liste des implants est modifiée (import/màj),
    //! quel qu'en soit le déclencheur (this dialogue ou un autre appelant de IOLs).
    connect(Datas::I()->iols, &IOLs::listeModifiee, this, [this]{
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
    });
    Datas::I()->iols->HasNewVersion(this);
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
    if (iol != nullptr)
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
    connect(wdg_manufacturerscombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=, this] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_typebox,            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=, this] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_prechargechk,       &UpCheckBox::checkStateChanged,  this,   [=, this] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_toricchk,           &UpCheckBox::checkStateChanged,  this,   [=, this] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_edofchk,            &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                      {
                                                                                        wdg_multifocalchk->setChecked(false);
                                                                                        wdg_monofocalchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_clairchk,           &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                        wdg_jaunechk->setChecked(false);
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_jaunechk,           &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                        wdg_clairchk->setChecked(false);
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_singlepiecechk,     &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                        wdg_twopiecechk->setChecked(false);
                                                                                       ReconstruitTreeViewIOLs();
                                                                                       wdg_annulfiltresbut->setEnabled(true);
                                                                                     } );
    connect(wdg_twopiecechk,        &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                        wdg_singlepiecechk->setChecked(false);
                                                                                       ReconstruitTreeViewIOLs();
                                                                                       wdg_annulfiltresbut->setEnabled(true);
                                                                                      } );
    connect(wdg_multifocalchk,      &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                      {
                                                                                        wdg_edofchk->setChecked(false);
                                                                                        wdg_monofocalchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_monofocalchk,       &UpCheckBox::uptoggled,  this,      [=, this](bool a) { if(a)
                                                                                      {
                                                                                        wdg_multifocalchk->setChecked(false);
                                                                                        wdg_edofchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_pwrchk,           &UpCheckBox::uptoggled,  this,      [=, this](bool a) {   wdg_pwrslider->setEnabled(a);
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
    //! disconnect(this) et non disconnect() tout court : on ne coupe QUE les connexions
    //! que ce dialogue a posées (toutes ont « this » pour destinataire). Un disconnect()
    //! nu couperait aussi les liaisons internes que Qt établit sur ces widgets.
    wdg_manufacturerscombo  ->disconnect(this);
    wdg_typebox             ->disconnect(this);
    wdg_prechargechk        ->disconnect(this);
    wdg_jaunechk            ->disconnect(this);
    wdg_clairchk            ->disconnect(this);
    wdg_singlepiecechk      ->disconnect(this);
    wdg_twopiecechk         ->disconnect(this);
    wdg_toricchk            ->disconnect(this);
    wdg_edofchk             ->disconnect(this);
    wdg_monofocalchk        ->disconnect(this);
    wdg_multifocalchk       ->disconnect(this);
    wdg_pwrchk              ->disconnect(this);   //! était oublié → ses connexions s'accumulaient à chaque remise à zéro des filtres
    wdg_rangepwrslider      ->disconnect(this);
}

void dlg_listeiols::ChoixButtonFrame()
{
    IOL *iol = nullptr;
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
    dlg_identificationIOL *Dlg_IdentIOL    = new dlg_identificationIOL(nullptr, this);
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
    if (it != nullptr)
        return qobject_cast<IOL *>(it->rufusitem());
    else
        return nullptr;
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mise à jour de la liste des implants  --------------------------
-----------------------------------------------------------------------------------------------------------------*/

// ------------------------------------------------------------------------------------------
// Modifie un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::ModifIOL(IOL *iol)
{
    if (iol == nullptr)
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
            wdg_manufacturerscombo->disconnect(this);
            wdg_manufacturerscombo->setCurrentIndex(0);
            connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=, this] { ReconstruitTreeViewIOLs(); } );
    }
    iol = Datas::I()->iols->getById(Dlg_IdentIOL->idcurrentIOL(), true);
    if (iol)
    {
        ReconstruitTreeViewIOLs();
        scrollToIOL(iol);
    }
    delete Dlg_IdentIOL;
}


void dlg_listeiols::scrollToIOL(IOL *iol)
{
    if (iol != nullptr)
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
        wdg_manufacturerscombo->disconnect(this);
        wdg_manufacturerscombo->setCurrentIndex(0);
        connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=, this] { ReconstruitTreeViewIOLs(); } );
        ReconstruitTreeViewIOLs();
    }
}

void dlg_listeiols::ReconstruitListeManufacturers()
{
    if (m_manufacturersmodel != nullptr)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);

    QStringList list;
    UpStandardItem *manufactureritem;
    QMap<int, Manufacturer*> mapman = QMap<int, Manufacturer*>();
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != nullptr)
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
    //! Cibler « this » : un disconnect() nu sur la vue et surtout sur son selectionModel
    //! couperait les liaisons internes de Qt (vue ↔ modèle de sélection) et casserait le
    //! surlignage / la navigation clavier.
    wdg_itemstree->disconnect(this);
    wdg_itemstree->selectionModel()->disconnect(this);
    if (m_IOLsmodel != nullptr)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);

    /*! filtrage du fabricant */
    int idman   = wdg_manufacturerscombo->currentData().toInt();
    UpStandardItem *pitem;
    QHash<QString, QStandardItem*> manufacturerNodes;   //! nom de fabricant → nœud parent, pour rattacher les implants en O(1)
    for (int i=0; i<  m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != nullptr)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->rufusitem());
            if (man != nullptr)
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
            if (man != nullptr)
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
        if (m_IOLsmodel->item(i) != nullptr)
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
        connect(wdg_itemstree,    &QAbstractItemView::entered,       this,   [=, this] (QModelIndex idx) { QStandardItem *item = m_IOLsmodel->itemFromIndex(idx);   //! peut être nul (index invalide) → on garde avant d'appeler hasChildren()
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
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked, this,   [=, this] (QModelIndex idx) { QStandardItem *item = m_IOLsmodel->itemFromIndex(idx);   //! peut être nul → on garde
                                                                                                     if (item && !item->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}
