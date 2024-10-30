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
    UpDialog(PATH_FILE_INI, "PositionsFiches/ListeIOLs",parent)
{
    m_onlyactifs = onlyactifs;
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Liste des IOLS"));

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

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    UpLabel* searchlbl          = new UpLabel;
    searchlbl                   ->setText(tr("Recherche d'implant"));
    QHBoxLayout *titrelay       = new QHBoxLayout();
    titrelay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    titrelay                    ->addWidget(searchlbl);
    titrelay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    wdg_manufacturerscombo = new UpComboBox();
    wdg_manufacturerscombo->setEditable(false);
    wdg_manufacturerscombo->addItem(tr("Tous"), 0);
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
    wdg_typebox                 ->addItems(QStringList() << tr("Tous") << IOL_CP << IOL_CA << IOL_ADDON << IOL_IRIEN << IOL_CAREFRACTIF << IOL_AUTRE);
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
    wdg_twopiecechk             = new UpCheckBox(tr("Anses rapportée"));
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
    UpLabel* pwrlbl             = new UpLabel;
    typelay                     ->addWidget(wdg_edofchk);
    typelay                     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    typelay                     ->addWidget(wdg_multifocalchk);
    typelay                     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    typelay                     ->addWidget(wdg_monofocalchk);
    typelay                     ->setContentsMargins(0,0,0,0);
    pwrlbl                      ->setText(tr("Puissance"));
    QVBoxLayout *pwrlay         = new QVBoxLayout();
    wdg_pwrslider               = new RangeSlider(Qt::Horizontal);
    wdg_pwrslider               ->SetRange(-150,400);
    QHBoxLayout *rangelay       = new QHBoxLayout();
    wdg_minpwrlbl               = new UpLabel;
    wdg_maxpwrlbl               = new UpLabel;
    wdg_minpwrlbl               ->setFixedSize(50,28);
    wdg_maxpwrlbl               ->setFixedSize(50,28);
    wdg_minpwrlbl               ->setText("-10.00");
    wdg_maxpwrlbl               ->setText("+40.00");
    rangelay                    ->addWidget(wdg_minpwrlbl);
    rangelay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    rangelay                    ->addWidget(wdg_maxpwrlbl);
    rangelay                    ->setContentsMargins(0,0,0,0);
    pwrlay                      ->addWidget(pwrlbl);
    pwrlay                      ->addWidget(wdg_pwrslider);
    pwrlay                      ->addLayout(rangelay);
    pwrlay                      ->setContentsMargins(0,0,0,0);
    pwrlay                      ->setSpacing(0);
    wdg_annulfiltresbut         = new UpPushButton(tr("Annuler les filtres"));


    QHBoxLayout *globallay  = new QHBoxLayout();
    QVBoxLayout *searchLay  = new QVBoxLayout();
    searchLay               ->addLayout(titrelay);
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
    searchLay               ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding));
    searchLay               ->setSpacing(10);
    globallay               ->addLayout(searchLay);
    globallay               ->addWidget(wdg_buttonframe->widgButtonParent());

    dlglayout()->insertLayout(0,globallay);
    setFixedWidth(wdg_manufacturerscombo->width() + globallay->spacing() + wdg_itemstree->width() + dlglayout()->contentsMargins().right() + dlglayout()->contentsMargins().left());
    TuneSize(false);

    if (!Datas::I()->iols->isfull())
        Datas::I()->iols->initListe();
    ReconstruitTreeViewIOLs();

    for (int i=0; i<m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->item());
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

    connect(OKButton,                       &QPushButton::clicked,      this,   &QDialog::accept);
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
            if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            {
                UpCheckBox *chk = qobject_cast<UpCheckBox*>(obj);
                if (chk)
                {
                    chk->setChecked(!chk->isChecked());
                    emit chk->uptoggled(chk->isChecked());  //!la propriété checkecd est affectée après l'event keypress, il faut donc envoyer l'inverse de la propriété pour avoir sa vraie valeur
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
                emit chk->uptoggled(chk->isChecked());  //!la propriété checkecd est affectée après l'event keypress, il faut donc envoyer l'inverse de la propriété pour avoir sa vraie valeur
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
    wdg_pwrslider           ->SetLowerValue(wdg_pwrslider->Minimun());
    wdg_pwrslider           ->SetUpperValue(wdg_pwrslider->Maximun());
    wdg_minpwrlbl           ->setText(Utils::PrefixePlus(double(wdg_pwrslider->Minimun())/10));
    wdg_maxpwrlbl           ->setText(Utils::PrefixePlus(double(wdg_pwrslider->Maximun())/10));
    connectFiltersSignals();
}

void dlg_listeiols::connectFiltersSignals()
{
    connect(wdg_manufacturerscombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_typebox,            QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_prechargechk,       &UpCheckBox::stateChanged,  this,   [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_toricchk,           &UpCheckBox::stateChanged,  this,   [=] { ReconstruitTreeViewIOLs(); wdg_annulfiltresbut->setEnabled(true);} );
    connect(wdg_edofchk,            &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                      {
                                                                                        wdg_multifocalchk->setChecked(false);
                                                                                        wdg_monofocalchk->setChecked(false);
                                                                                      }
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_clairchk,            &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                        wdg_jaunechk->setChecked(false);
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_jaunechk,            &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                        wdg_clairchk->setChecked(false);
                                                                                      ReconstruitTreeViewIOLs();
                                                                                      wdg_annulfiltresbut->setEnabled(true);
                                                                                    } );
    connect(wdg_singlepiecechk,      &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
                                                                                         wdg_twopiecechk->setChecked(false);
                                                                                       ReconstruitTreeViewIOLs();
                                                                                       wdg_annulfiltresbut->setEnabled(true);
                                                                                     } );
    connect(wdg_twopiecechk,         &UpCheckBox::uptoggled,  this,      [=](bool a) { if(a)
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
    connect(wdg_pwrslider,          &RangeSlider::lowerValueChanged,    this, [&](int a) {
                                                                                            m_minpwr = double(Utils::roundToNearestFive(a))/10;
                                                                                            wdg_minpwrlbl->setText(Utils::PrefixePlus(m_minpwr));
                                                                                            ReconstruitTreeViewIOLs();
                                                                                            wdg_annulfiltresbut->setEnabled(true);
                                                                                         } );
    connect(wdg_pwrslider,          &RangeSlider::upperValueChanged,    this, [&](int a) {
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
    wdg_pwrslider           ->disconnect();
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
        return qobject_cast<IOL *>(it->item());
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

    /*! Mise à jour de la liste des IOLs */
    int newiols = 0;
    int updateiols = 0;
    QStringList iolbrandmodel= QStringList();
    QList<int> iollistid = QList<int>();
    foreach (IOL *iolfromlist, *Datas::I()->iols->iols())
    {
        if (iolfromlist != Q_NULLPTR)
        {
            Manufacturer* man = Datas::I()->manufacturers->getById(iolfromlist->idmanufacturer());
            if (man != Q_NULLPTR)
                if (!iolbrandmodel.contains(man->nom()))
                {
                    iolbrandmodel << man->nom().toUpper() + " " + iolfromlist->modele().toUpper();
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
                    iol.setmodele(Lensnode.text());
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
                            iol.setOpticalMaterial(Specsnode.text());
                        else if (Specsnode.tagName() == "HapticMaterial" && !iol.issinglepiece())
                        {
                            iol.setHapticalMaterial(Specsnode.text());
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
                            iol.setpreloaded(b);
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
                        else if (Specsnode.tagName() == "IntendedLocation" && iol.type() == "")
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
                        else if (Constantnode.tagName() == "Barrett")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Barrettnode = Constantnode.childNodes().at(i).toElement();
                                if (Barrettnode.tagName() == "DF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettDF_optimized(Barrettnode.text().toDouble());
                                }
                                else if (Barrettnode.tagName() == "LF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettLF_optimized(Barrettnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "Olsen")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setOlsen_optimized(Constantnode.text().toDouble());
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
            int idx = iolbrandmodel.indexOf(iol.stringid().toUpper() + " " + iol.modele().toUpper());
            if (idx > -1)
            {
                /*! update existant iol */
                IOL* iolfromlist = Datas::I()->iols->getById(iollistid.at(idx));
                if (iolfromlist != Q_NULLPTR)
                {
                    /*! update existant iol */
                    ItemsList::update(iolfromlist, CP_MODELNAME_IOLS,       iol.modele());
                    ItemsList::update(iolfromlist, CP_IDMANUFACTURER_IOLS,  man->id());
                    ItemsList::update(iolfromlist, CP_CSTEAECHO_IOLS,       iol.csteAEcho_nominal());

                    ItemsList::update(iolfromlist, CP_CSTEAOPT_IOLS,        iol.csteAopt_nominal());
                    ItemsList::update(iolfromlist, CP_HAIGISA0_IOLS,        iol.haigisa0_nominal());
                    ItemsList::update(iolfromlist, CP_HAIGISA1_IOLS,        iol.haigisa1_nominal());
                    ItemsList::update(iolfromlist, CP_HAIGISA2_IOLS,        iol.haigisa2_nominal());
                    ItemsList::update(iolfromlist, CP_HOLL1_IOLS,           iol.holladay1_nominal());
                    ItemsList::update(iolfromlist, CP_HOFFERQ_IOLS,         iol.hofferQ_nominal());
                    ItemsList::update(iolfromlist, CP_BARRETTDF_IOLS,        iol.barrettDF_nominal());
                    ItemsList::update(iolfromlist, CP_BARRETTLF_IOLS,        iol.barrettLF_nominal());
                    ItemsList::update(iolfromlist, CP_OLSEN_IOLS,           iol.olsen_nominal());

                    ItemsList::update(iolfromlist, CP_ACD_IOLS,             iol.acd());
                    ItemsList::update(iolfromlist, CP_OPTICMATERIAU_IOLS,   iol.opticalmaterial());
                    ItemsList::update(iolfromlist, CP_HAPTICMATERIAU_IOLS,  iol.hapticalmaterial());
                    ItemsList::update(iolfromlist, CP_DIAALL_IOLS,          iol.diaall());
                    ItemsList::update(iolfromlist, CP_DIAOPT_IOLS,          iol.opticdiameter());
                    ItemsList::update(iolfromlist, CP_DIAINJECTEUR_IOLS,    iol.diainjecteur());
                    ItemsList::update(iolfromlist, CP_PRECHARGE_IOLS,       iol.ispreloaded());
                    ItemsList::update(iolfromlist, CP_MAXPWR_IOLS,          iol.pwrmax());
                    ItemsList::update(iolfromlist, CP_MINPWR_IOLS,          iol.pwrmin());
                    ItemsList::update(iolfromlist, CP_MAXCYL_IOLS,          iol.cylmax());
                    ItemsList::update(iolfromlist, CP_MINCYL_IOLS,          iol.cylmin());
                    ItemsList::update(iolfromlist, CP_ADDINTERMEDIATE_IOLS, iol.addintermediate());
                    ItemsList::update(iolfromlist, CP_ADDNEAR_IOLS,         iol.addnear());
                    ItemsList::update(iolfromlist, CP_JAUNE_IOLS,           iol.isyellow());
                    ItemsList::update(iolfromlist, CP_MULTIFOCAL_IOLS,      iol.ismultifocal());
                    ItemsList::update(iolfromlist, CP_EDOF_IOLS,            iol.isedof());
                    ItemsList::update(iolfromlist, CP_TORIC_IOLS,           iol.istoric());
                    ItemsList::update(iolfromlist, CP_SINGLEPIECE_IOLS,     iol.issinglepiece());

                    ItemsList::update(iolfromlist, CP_RESULTSU_IOLS,        iol.results_ulib());
                    ItemsList::update(iolfromlist, CP_CSTEAECHOU_IOLS,      iol.csteAEcho_ulib());
                    ItemsList::update(iolfromlist, CP_CSTEAOPTU_IOLS,       iol.csteAopt_ulib());
                    ItemsList::update(iolfromlist, CP_HAIGISA0U_IOLS,       iol.haigisa0_ulib());
                    ItemsList::update(iolfromlist, CP_HAIGISA1U_IOLS,       iol.haigisa1_ulib());
                    ItemsList::update(iolfromlist, CP_HAIGISA2U_IOLS,       iol.haigisa2_ulib());
                    ItemsList::update(iolfromlist, CP_HOLL1U_IOLS,          iol.holladay1_ulib());
                    ItemsList::update(iolfromlist, CP_HOFFERQU_IOLS,        iol.hofferQ_ulib());
                    ItemsList::update(iolfromlist, CP_BARRETTDFU_IOLS,      iol.barrettDF_ulib());
                    ItemsList::update(iolfromlist, CP_BARRETTLFU_IOLS,      iol.barrettLF_ulib());
                    ItemsList::update(iolfromlist, CP_OLSENU_IOLS,          iol.olsen_ulib());

                    ItemsList::update(iolfromlist, CP_RESULTSO_IOLS,        iol.results_optimized());
                    ItemsList::update(iolfromlist, CP_CSTEAOPTO_IOLS,       iol.csteAopt_optimized());
                    ItemsList::update(iolfromlist, CP_HAIGISA0O_IOLS,       iol.haigisa0_optimized());
                    ItemsList::update(iolfromlist, CP_HAIGISA1O_IOLS,       iol.haigisa1_optimized());
                    ItemsList::update(iolfromlist, CP_HAIGISA2O_IOLS,       iol.haigisa2_optimized());
                    ItemsList::update(iolfromlist, CP_HOLL1O_IOLS,          iol.holladay1_optimized());
                    ItemsList::update(iolfromlist, CP_HOFFERQO_IOLS,        iol.hofferQ_optimized());
                    ItemsList::update(iolfromlist, CP_BARRETTDFO_IOLS,      iol.barrettDF_optimized());
                    ItemsList::update(iolfromlist, CP_BARRETTLFO_IOLS,      iol.barrettLF_optimized());
                    ItemsList::update(iolfromlist, CP_OLSENO_IOLS,          iol.olsen_optimized());

                    ItemsList::update(iolfromlist, CP_TYP_IOLS,             iol.typetoint());
                    ++ updateiols;
                }
            }
            else /*! newiol */
            {
                QHash<QString, QVariant> m_listbinds;
                m_listbinds[CP_MODELNAME_IOLS]      = iol.modele();
                m_listbinds[CP_IDMANUFACTURER_IOLS] = man->id();
                m_listbinds[CP_CSTEAECHO_IOLS]      = (iol.csteAEcho_nominal() >0.0?            iol.csteAEcho_nominal()             : QVariant());

                m_listbinds[CP_CSTEAOPT_IOLS]       = (iol.csteAopt_nominal() >0.0?     iol.csteAopt_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA0_IOLS]       = (iol.haigisa0_nominal() >0.0?     iol.haigisa0_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA1_IOLS]       = (iol.haigisa1_nominal() >0.0?     iol.haigisa1_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA2_IOLS]       = (iol.haigisa2_nominal() >0.0?     iol.haigisa2_nominal()      : QVariant());
                m_listbinds[CP_HOLL1_IOLS]          = (iol.holladay1_nominal()>0.0?     iol.holladay1_nominal()     : QVariant());
                m_listbinds[CP_HOFFERQ_IOLS]        = (iol.hofferQ_nominal()>0.0?       iol.hofferQ_nominal()       : QVariant());
                m_listbinds[CP_BARRETTDF_IOLS]       = (iol.barrettDF_nominal()>0.0?      iol.barrettDF_nominal()      : QVariant());
                m_listbinds[CP_BARRETTLF_IOLS]       = (iol.barrettLF_nominal()>0.0?      iol.barrettLF_nominal()      : QVariant());
                m_listbinds[CP_OLSEN_IOLS]          = (iol.olsen_nominal()>0.0?         iol.olsen_nominal()         : QVariant());

                m_listbinds[CP_ACD_IOLS]            = (iol.acd()>0.0?                   iol.acd()                   : QVariant());
                m_listbinds[CP_OPTICMATERIAU_IOLS]  = iol.opticalmaterial();
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
                m_listbinds[CP_BARRETTDFU_IOLS]      = (iol.barrettDF_ulib()>0.0?         iol.barrettDF_ulib()         : QVariant());
                m_listbinds[CP_BARRETTLFU_IOLS]      = (iol.barrettLF_ulib()>0.0?         iol.barrettLF_ulib()         : QVariant());
                m_listbinds[CP_OLSENU_IOLS]         = (iol.olsen_ulib()>0.0?            iol.olsen_ulib()            : QVariant());

                m_listbinds[CP_RESULTSO_IOLS]       = (iol.results_optimized() >0?      iol.results_optimized()     : QVariant());
                m_listbinds[CP_CSTEAOPTO_IOLS]      = (iol.csteAopt_optimized() >0.0?   iol.csteAopt_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA0O_IOLS]      = (iol.haigisa0_optimized() >0.0?   iol.haigisa0_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA1O_IOLS]      = (iol.haigisa1_optimized() >0.0?   iol.haigisa1_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA2O_IOLS]      = (iol.haigisa2_optimized() >0.0?   iol.haigisa2_optimized()    : QVariant());
                m_listbinds[CP_HOLL1O_IOLS]         = (iol.holladay1_optimized()>0.0?   iol.holladay1_optimized()   : QVariant());
                m_listbinds[CP_HOFFERQO_IOLS]       = (iol.hofferQ_optimized()>0.0?     iol.hofferQ_optimized()     : QVariant());
                m_listbinds[CP_BARRETTDFO_IOLS]      = (iol.barrettDF_optimized()>0.0?    iol.barrettDF_optimized()    : QVariant());
                m_listbinds[CP_BARRETTLFO_IOLS]      = (iol.barrettLF_optimized()>0.0?    iol.barrettLF_optimized()    : QVariant());
                m_listbinds[CP_OLSENO_IOLS]         = (iol.olsen_optimized()>0.0?       iol.olsen_optimized()       : QVariant());
                m_listbinds[CP_TYP_IOLS]            = (iol.typetoint()>0?               iol.typetoint()             : QVariant());
                Datas::I()->iols->CreationIOL(m_listbinds);
                ++newiols;
            }
        }
    }
    /*! fin mise à jour de la liste des IOLs */

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
    for (int i=0; i<m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->item());
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
        if (iol)
        {
            Datas::I()->iols->getById(Dlg_IdentIOL->idcurrentIOL(), true);
            m_listemodifiee = true;
            wdg_manufacturerscombo->disconnect();
            wdg_manufacturerscombo->setCurrentIndex(0);
            connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(); } );
            ReconstruitTreeViewIOLs();
            scrollToIOL(iol);
        }
    }
    delete Dlg_IdentIOL;
}

void dlg_listeiols::resizeiolimage(IOL *iol)
{
    int maxsizeimg = 16384;
    Utils::RemoveProvDir();
    if (iol->imageformat() != JPG || iol->imageformat() != PNG || iol->imageformat() != JPEG)
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

    if (Utils::CompressFileToJPG(nomfichresize, false, maxsizeimg))
    {
        QFile file_image(nomfichresize);
        file_image.setFileName(nomfichresize);
        file_image.open(QIODevice::ReadOnly);
        QByteArray ba = file_image.readAll();
        QHash<QString, QVariant> m_listbinds;
        m_listbinds[CP_ARRAYIMG_IOLS] = ba;
        m_listbinds[CP_TYPIMG_IOLS] = JPG;
        DataBase::I()->UpDateIOL(iol->id(), m_listbinds);
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
                            if (childitm->item())
                            {
                                IOL *siol = qobject_cast<IOL*>(childitm->item());
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
    ReconstruitListeManufacturers();
    wdg_itemstree->disconnect();
    wdg_itemstree->selectionModel()->disconnect();
    if (m_IOLsmodel != Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);

    /*! filtrage du fabricant */
    int idman   = wdg_manufacturerscombo->currentData().toInt();
    UpStandardItem *pitem;
    for (int i=0; i<  m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != Q_NULLPTR)
        {
            Manufacturer *man = qobject_cast<Manufacturer*>(itm->item());
            if (man != Q_NULLPTR)
            {
                if (idman == 0)                                                                 //! tous les fabricants
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
                }
                else if (man->id() == idman)                                                    //! un seul fabricant
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
                }
            }
        }
        m_IOLsmodel->sort(0);
    }
    int dim = 45;
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        if (m_onlyactifs && !iol->isactif())
            continue;
        if (iol->modele().startsWith(filtre, Qt::CaseInsensitive))
        {
            /*! filtrage des types */
            if (wdg_typebox->currentIndex() != 0)
            {
                if (iol->type() != wdg_typebox->currentText())
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
            /*! filtrage des jaunes */
            if (wdg_singlepiecechk->isChecked())
                if (!iol->issinglepiece())
                    continue;
            /*! filtrage des clairs */
            if (wdg_twopiecechk->isChecked())
                if (!iol->issinglepiece())
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
            /*! filtrage des monfoocaux */
            if (wdg_monofocalchk->isChecked())
                if (iol->ismultifocal() || iol->isedof())
                    continue;
            /*! filtrage des puissances */
            if (iol->pwrmin() > m_maxpwr || iol->pwrmax() < m_minpwr)
                continue;
            pitem   = new UpStandardItem(iol->modele(), iol);
            if (!iol->isactif())
                pitem ->setForeground(QBrush(QColor(Qt::darkGray)));
            pitem   ->setEditable(false);
            QImage image= iol->image();
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
                QString fabricant  = man->nom();
                QList<QStandardItem *> listitems = m_IOLsmodel->findItems(fabricant);
                if (listitems.size()>0)
                    listitems.at(0)->appendRow(pitem);
            }
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
    if (m_IOLsmodel->rowCount()>0)
    {
        m_IOLsmodel->sort(0);
        m_IOLsmodel->sort(1);
        connect(wdg_itemstree,    &QAbstractItemView::entered,       this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                        {
                                                                                                            IOL*iol = getIOLFromIndex(idx);
                                                                                                            if (iol)
                                                                                                            {
                                                                                                                resizeiolimage(iol);
                                                                                                                QToolTip::showText(cursor().pos(), iol->tooltip());
                                                                                                            }
                                                                                                        }
                                                                                                    } );
        connect(wdg_itemstree->selectionModel(),    &QItemSelectionModel::currentChanged,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}void dlg_listeiols::HasNewVersion()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(LIEN_XML_IOLCONLASTVERSION));
    QNetworkReply *reply = manager->get(request);

    connect(manager,
            &QNetworkAccessManager::finished,
            this, [=]
            {
                QByteArray data;
                if(reply->error() == QNetworkReply::NoError)
                {
                    data = reply->readAll();
                    QDomDocument docxml;
                    docxml.setContent(data);
                    QDomElement xml = docxml.documentElement();
                    double lastversion = xml.attribute("fileVersion").toDouble();
                    double actualversion = DataBase::I()->versionbaseiol();
                    if(actualversion < lastversion)
                    {
                        if (UpMessageBox::Question
                            (this,tr("Mise à jour de la liste des implants"),
                             tr("Vous utilisez la version") + " " + QString::number(actualversion) + "\n"
                                 + tr("La version") + " " + QString::number(lastversion) + " " + tr("de la liste des implants est disponible sur le site https://iolcon.org/") + "\n"
                                 + tr("Voulez vous l'incorporer dans Rufus?") + "\n"
                                 + tr("Aucun implant de votre base actuelle ne sera modifié"))
                            == UpSmallButton::STARTBUTTON)
                        {
                            ImportListeIOLS(docxml);
                            DataBase::I()->setversionbaseiol(lastversion);
                        }
                    }
                }
            });
}

