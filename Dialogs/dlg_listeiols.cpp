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
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Liste des IOLS"));
    setWindowModality(Qt::WindowModal);

    wdg_manufacturerscombo = new UpComboBox();

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

    UpPushButton *importbutt =   new UpPushButton(tr("Mettre à jour"));
    importbutt->setIcon(Icons::icImport());
    AjouteWidgetLayButtons(importbutt, false);
    importbutt->setVisible(false);

    UpPushButton *resizebutt =   new UpPushButton(tr("Comprimer"));
    resizebutt->setIcon(Icons::icComputer());
    AjouteWidgetLayButtons(resizebutt, false);
    resizebutt->setVisible(false);

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
    TuneSize();

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

    connect(importbutt,                     &QPushButton::clicked,      this,   &dlg_listeiols::ImportListeIOLS);
    connect(resizebutt,                     &QPushButton::clicked,      this,   [&] {resizeiolimage();});
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
        IOL *iol = Datas::I()->iols->getById(Dlg_IdentIOL->idcurrentIOL());
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
void dlg_listeiols::ImportListeIOLS()
{
    UpMessageBox::Watch(this,tr("Mise à jour de la liste des implants"), tr("Pour mettre à jour la liste des implants") + "\n"
                        + tr("Téléchargez la liste des implants - fichier IOLexport.xml - sur le site https://iolcon.org/") + "\n"
                        + tr("Sélectionnez ce fichier une fois téléchargé dans la boîte de dialogue qui suit") + "\n"
                        + tr("Aucun implant de votre base actuelle ne sera modifié"));

    /*! Choix du fichier xml contenant la liste des implants */
    QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), desktop,  "*.xml");
    if (fileName == "")
        return;
    QFile xmldoc(fileName);
    int nbimplants = 0;
    if (xmldoc.open(QIODevice::ReadOnly))
    {
        QDomDocument *docxml = new QDomDocument();
        docxml->setContent(&xmldoc);
        QDomElement xml = docxml->documentElement();
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
            if (!listmanofficiel.contains(nommanufacturer.toUpper()))
            {
                QHash<QString,QVariant> listbinds;
                listbinds[CP_NOM_MANUFACTURER] = nommanufacturer.toUpper();
                Datas::I()->manufacturers->CreationManufacturer(listbinds);
            }
        /*! fin mise à jour de la liste des fabricants */

        /*! Mise à jour de la liste des IOLs */
        for (int i=0; i<xml.childNodes().size(); i++)     // on reprend chaque IOL un par un
        {
            int idiol;
            QString fabricant  = "";
            QString modele = "";
            QString materiau = "";
            bool precharge = false;
            double incision= 0.0;
            double diaoptique = 0.0;
            double diaall = 0.0;
            bool multifocal = false;
            bool toric = false;
            bool jaune = false;
            bool edof = false;
            double pwrmin = 100.0;
            double pwrmax = -100.0;
            double cylmin = 100.0;
            double cylmax = -100.0;
            double add = 0.0;
            double srkt = 0.0;
            double csteA = 0.0;
            double haigis0 = 0.0;
            double haigis1 = 0.0;
            double haigis2 = 0.0;
            double hofferQ = 0.0;
            double holladay = 0.0;
            double barettdf = 0.0;
            double barettlf = 0.0;
            double olsen = 0.0;
            QDomElement lensnode = xml.childNodes().at(i).toElement();
            idiol = lensnode.attributeNode("id").value().toInt();
            for (int i=0; i<lensnode.childNodes().size(); i++)
            {
                QDomElement node = lensnode.childNodes().at(i).toElement();
                if (node.tagName() == "Manufacturer")
                    fabricant = node.text();
                else if (node.tagName() == "Name")
                    modele = node.text();
                else if (node.tagName() == "Specifications")
                    for (int i=0; i<node.childNodes().size(); i++)
                    {
                        QDomElement nodespec = node.childNodes().at(i).toElement();
                        if (nodespec.tagName() == "OpticMaterial")
                            materiau = nodespec.text();
                        else if (nodespec.tagName() == "Hydro")
                        {
                            if (nodespec.text() != "")
                                materiau += " " + nodespec.text();
                        }
                        else if (nodespec.tagName() == "Preloaded")
                            precharge = (nodespec.text() != "no");
                        else if (nodespec.tagName() == "Filter")
                            jaune = (nodespec.text() == "yellow");
                        else if (nodespec.tagName() == "IncisionWidth")
                            incision = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "OpticDiameter")
                            diaoptique = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "HapticDiameter")
                            diaall = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "OpticConcept")
                        {
                            multifocal = (nodespec.text() == "multifocal");
                            edof = (nodespec.text() == "EDoF");
                        }
                        else if (nodespec.tagName() == "Toric")
                            toric = (nodespec.text() != "no");
                    }
                else if (node.tagName() == "Availability")
                    for (int i=0; i<node.childNodes().size(); i++)
                    {
                        QDomElement nodespec = node.childNodes().at(i).toElement();
                        if (nodespec.tagName() == "Sphere")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodepwr = nodespec.childNodes().at(i).toElement();
                                if (nodepwr.tagName() == "From")
                                    if (pwrmin > 99.0 || pwrmin > nodepwr.text().toDouble())
                                        pwrmin = nodepwr.text().toDouble();
                                if (nodepwr.tagName() == "To")
                                    if (pwrmax < -99.0 || pwrmax < nodepwr.text().toDouble())
                                        pwrmax = nodepwr.text().toDouble();
                            }
                        else if (nodespec.tagName() == "Cylinder")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodecyl = nodespec.childNodes().at(i).toElement();
                                if (nodecyl.tagName() == "From")
                                    if (cylmin > 99.0 || cylmin > nodecyl.text().toDouble())
                                    cylmin = nodecyl.text().toDouble();
                                if (nodecyl.tagName() == "To")
                                    if (cylmax < -99.0 || cylmax < nodecyl.text().toDouble())
                                        cylmax = nodecyl.text().toDouble();
                            }
                        else if (nodespec.tagName() == "Addition")
                            add = nodespec.text().toDouble();
                    }
                else if (node.tagName() == "Constants" && node.attributeNode("type").value() == "nominal")
                    for (int i=0; i<node.childNodes().size(); i++)
                    {
                        QDomElement nodespec = node.childNodes().at(i).toElement();
                        if (nodespec.tagName() == "Ultrasound")
                            csteA = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "SRKt")
                            srkt = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "Haigis")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodehaigis = nodespec.childNodes().at(i).toElement();
                                if (nodehaigis.tagName() == "a0")
                                    haigis0 = nodehaigis.text().toDouble();
                                else if (nodehaigis.tagName() == "a1")
                                    haigis1 = nodehaigis.text().toDouble();
                                else if (nodehaigis.tagName() == "a2")
                                    haigis2 = nodehaigis.text().toDouble();
                            }
                        else if (nodespec.tagName() == "HofferQ")
                            hofferQ = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "Holladay1")
                            holladay = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "Barrett")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodebarrett = nodespec.childNodes().at(i).toElement();
                                if (nodebarrett.tagName() == "DF")
                                    barettdf = nodebarrett.text().toDouble();
                                if (nodebarrett.tagName() == "LF")
                                    barettlf = nodebarrett.text().toDouble();
                            }
                        else if (nodespec.tagName() == "Olsen")
                            olsen = nodespec.text().toDouble();
                    }
            }
            bool foundiol = false;
            if (pwrmin > 99.0) pwrmin= 0.0;
            if (pwrmax < -99.0) pwrmax= 0.0;
            if (cylmin > 99.0) cylmin= 0.0;
            if (cylmax < -99.0) cylmax= 0.0;
            Manufacturer *man = Q_NULLPTR;
            for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
            {
                Manufacturer *manf = const_cast<Manufacturer*>(it.value());
                 if (manf)
                    if (manf->nom().toUpper() == fabricant.toUpper())
                    {
                        man = manf;
                        break;
                    }
            }
            if (!man)
                continue;
            foreach (IOL *iol, *Datas::I()->iols->iols())
                if (iol->modele().toUpper() == modele.toUpper() && man->nom().toUpper() == fabricant.toUpper())
                {
                    foundiol = true;
                    break;
                }
            if (!foundiol && man != Q_NULLPTR)
            {
                QHash<QString, QVariant> m_listbinds;
                m_listbinds[CP_MODELNAME_IOLS]      = modele;
                m_listbinds[CP_IDMANUFACTURER_IOLS] = man->id();
                m_listbinds[CP_CSTEAOPT_IOLS]       = (csteA >0.0?    csteA    : QVariant());
                m_listbinds[CP_CSTEAECHO_IOLS]      = (srkt >0.0?     srkt     : QVariant());
                m_listbinds[CP_HAIGISA0_IOLS]       = (haigis0 >0.0?  haigis0  : QVariant());
                m_listbinds[CP_HAIGISA1_IOLS]       = (haigis1 >0.0?  haigis1  : QVariant());
                m_listbinds[CP_HAIGISA2_IOLS]       = (haigis2 >0.0?  haigis2  : QVariant());
                m_listbinds[CP_ACD_IOLS]            = (hofferQ>0.0?   hofferQ  : QVariant());
                m_listbinds[CP_HOLL1_IOLS]          = (holladay>0.0?  holladay : QVariant());
                m_listbinds[CP_MATERIAU_IOLS]       = materiau;
                m_listbinds[CP_DIAALL_IOLS]         = (diaall >0.0?  diaall : QVariant());
                m_listbinds[CP_DIAOPT_IOLS]         = (diaoptique >0.0? diaoptique  : QVariant());
                m_listbinds[CP_DIAINJECTEUR_IOLS]   = (incision >0.0? incision : QVariant());
                m_listbinds[CP_PRECHARGE_IOLS]      = (precharge?   "1" : QVariant());
                m_listbinds[CP_MAXPWR_IOLS]         = pwrmax;
                m_listbinds[CP_MINPWR_IOLS]         = pwrmin;
                m_listbinds[CP_MAXCYL_IOLS]         = (toric? cylmax : QVariant());
                m_listbinds[CP_MINCYL_IOLS]         = (toric? cylmin : QVariant());
                m_listbinds[CP_JAUNE_IOLS]          = (jaune?       "1" : QVariant());
                m_listbinds[CP_MULTIFOCAL_IOLS]     = (multifocal?  "1" : QVariant());
                m_listbinds[CP_EDOF_IOLS]           = (edof?        "1" : QVariant());
                m_listbinds[CP_TORIC_IOLS]          = (toric?       "1" : QVariant());
                Datas::I()->iols->CreationIOL(m_listbinds);
                ++nbimplants;
            }
        }
    }
    /*! fin mise à jour de la liste des IOLs */
    QString msg = "Aucun implant n'a été rajouté à la base";
    switch (nbimplants) {
    case 0:
        break;
    case 1:
        msg = "Un implant a été rajouté à la base";
        break;
    default:
        msg = QString::number(nbimplants) + " implants ont été rajoutés à la base";
    }
    UpMessageBox::Watch(this, msg);
    Datas::I()->iols->initListe();
    ReconstruitTreeViewIOLs();
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

void dlg_listeiols::resizeiolimage(int size)
{
    QString szorigin, szfinal;
    QString m_pathdirstockageprovisoire = Procedures::I()->DefinitDossierImagerie() + NOM_DIR_PROV ;
    QStringList listfichresize = QDir(m_pathdirstockageprovisoire).entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (int t=0; t<listfichresize.size(); t++)
    {
        QString nomdocrz  = listfichresize.at(t);
        QString CheminFichierResize = m_pathdirstockageprovisoire + "/" + nomdocrz;
        QFile file(CheminFichierResize);
        Utils::removeWithoutPermissions(file);
    }
    bool ok;
    QString req = "select " CP_ID_IOLS ", " CP_ARRAYIMG_IOLS " from " TBL_IOLS
                " where " CP_ARRAYIMG_IOLS " is not null and " CP_TYPIMG_IOLS " <> '" PDF "'";
    QList<QVariantList> listimg  = DataBase::I()->StandardSelectSQL(req, ok);
    for (int i=0; i<listimg.size(); ++i)
    {
        QString nomfichresize = m_pathdirstockageprovisoire + "/resize" + listimg.at(i).at(0).toString() + "." JPG;
        QByteArray ba = QByteArray();
        int id = listimg.at(i).at(0).toInt();
        ba = listimg.at(i).at(1).toByteArray();
        QFile file_image;
        QImage img;
        img.loadFromData(ba);
        double sz = ba.size();
        if (sz/(1024*1024) > 1)
            szorigin = QString::number(sz/(1024*1024),'f',1) + "Mo";
        else
            szorigin = QString::number(sz/1024,'f',1) + "Ko";
        szfinal = szorigin;
        file_image.setFileName(nomfichresize);
        int tauxcompress = 90;
        bool resized = false;
        if (sz > size)
        {
            resized = true;
            Utils::removeWithoutPermissions(file_image);
            QPixmap pixmap;
            pixmap = pixmap.fromImage(img.scaledToWidth(256,Qt::SmoothTransformation));
            tauxcompress = 90;
            while (sz > size && tauxcompress > 1)
            {
                pixmap.save(nomfichresize, "jpeg",tauxcompress);
                ba = file_image.readAll();
                sz = ba.size();
                if (tauxcompress > 19)
                    tauxcompress -= 10;
                else tauxcompress -= 1;
            }
            if (sz/(1024*1024) > 1)
                szfinal = QString::number(sz/(1024*1024),'f',0) + "Mo";
            else
                szfinal = QString::number(sz/1024,'f',0) + "Ko";
        }
        if (resized)
        {
            file_image.open(QIODevice::ReadOnly);
            ba = file_image.readAll();
            QHash<QString, QVariant> m_listbinds;
            m_listbinds[CP_ARRAYIMG_IOLS] = ba;
            m_listbinds[CP_TYPIMG_IOLS] = JPG;
            DataBase::I()->UpDateIOL(id, m_listbinds);
            Utils::removeWithoutPermissions(file_image);
        }
    }
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
    if (m_manufacturersmodel == Q_NULLPTR)
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
    if (m_IOLsmodel == Q_NULLPTR)
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
    //qDebug() << Datas::I()->iols->iols()->size();
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
                if (!iol->isprecharge())
                    continue;
            /*! filtrage des jaunes */
            if (wdg_jaunechk->isChecked())
                if (!iol->isjaune())
                    continue;
            /*! filtrage des clairs */
            if (wdg_clairchk->isChecked())
                if (iol->isjaune())
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
    //qDebug() << m_IOLsmodel->rowCount();
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
                                                                                                                QToolTip::showText(cursor().pos(), iol->tooltip());
                                                                                                        }
                                                                                                    } );
        connect(wdg_itemstree->selectionModel(),    &QItemSelectionModel::currentChanged,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_itemstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}
