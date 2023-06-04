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

#include "dlg_identificationiol.h"

/*!
 * \brief dlg_identificationIOL::dlg_identificationIOL
 * la fiche peut-être appelée de 2 façons
        * Mode Creation totale depuis dlg_listeiols
            * la fiche est lancée en mode Creation, tous les champs sont vierges et les paramètres IOL et Manufacturer sont nullptr
        * Mode Modification
            * la fiche est lancée depuis dlg_listeiols pour modifier les paramètres d'un IOL existant
            * le paramètre IOL est donc celui de l'implant à modifier
            * la paramètre Manufacturer peut-être laissé à Q_NULLPTR, il sera automatiquement rempli
            * toutes les zones de saisie sont renseignées avec les paramètres déjà connus de l'implant
 * \param mode
 * \param iol
 * \param man
 * \param parent
 */
dlg_identificationIOL::dlg_identificationIOL(IOL *iol, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentIOL", parent)
{
    if (Datas::I()->manufacturers->manufacturers()->size() == 0)
    {
        UpMessageBox::Watch(parent,tr("Aucun fabricant enregistré"), tr("Vous devez avoir des fabricants enregistrés dans la base pour pouvoir gérer les implants"));
        m_initok = false;
        return;
    }
    setWindowModality(Qt::WindowModal);
    m_currentIOL = iol;

    m_mode = (m_currentIOL != Q_NULLPTR? Modification : Creation);
    //qDebug() << Utils::EnumDescription(QMetaEnum::fromType<dlg_identificationIOL::Mode>(), m_mode);
    if (m_mode == Modification)
        m_currentmanufacturer = Datas::I()->manufacturers->getById(m_currentIOL->idmanufacturer());
    if (m_currentmanufacturer)
        reconstruitListeIOLs(m_currentmanufacturer);

    setWindowTitle(m_mode == Creation? tr("Enregistrer un IOL") : tr("Modifier un IOL"));

    //! FABRICANT
    if (m_manufacturersmodel == Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);
    foreach (Manufacturer *man, *Datas::I()->manufacturers->manufacturers())
        if (man->isactif()  && man->buildIOLs()) {
            QList<QStandardItem *> items;
            UpStandardItem *itemman = new UpStandardItem(man->nom(), man);
            UpStandardItem *itemid = new UpStandardItem(QString::number(man->id()), man);
            items << itemman << itemid;
            m_manufacturersmodel->appendRow(items);
        }
    m_manufacturersmodel->sort(0);

    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    wdg_manufacturercombo       = new QComboBox();
    wdg_manufacturercombo       ->setEditable(true); /*! on met cette propriété à true sinon on ne peut pas agir sur la lineEdit du combo, ça plante...*/
    wdg_manufacturercombo       ->lineEdit()->setAlignment(Qt::AlignCenter);
    wdg_manufacturercombo       ->lineEdit()->setFocusPolicy(Qt::NoFocus);
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        wdg_manufacturercombo   ->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
        wdg_manufacturercombo   ->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
    }
    if (m_currentmanufacturer)
        wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
    else
        m_currentmanufacturer = Datas::I()->manufacturers->getById(wdg_manufacturercombo->itemData(0).toInt());
    choixManufacturerIOLLay     ->addWidget(lblManufacturerIOL);
    choixManufacturerIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerIOLLay     ->addWidget(wdg_manufacturercombo);
    choixManufacturerIOLLay     ->setSpacing(5);
    choixManufacturerIOLLay     ->setContentsMargins(0,0,0,0);

    //! MODELE
    QHBoxLayout *choixIOLLay    = new QHBoxLayout();
    UpLabel* lblIOL             = new UpLabel;
    lblIOL                      ->setText(tr("Modèle"));
    wdg_nomiolline              = new UpLineEdit();
    wdg_nomiolline              ->setFixedSize(QSize(250,28));
    wdg_nomiolline              ->setValidator(new QRegularExpressionValidator(Utils::rgx_cotation));
    wdg_nomiolline              ->setMaxLength(40);
    choixIOLLay                 ->addWidget(lblIOL);
    choixIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay                 ->addWidget(wdg_nomiolline);
    choixIOLLay                 ->setSpacing(5);
    choixIOLLay                 ->setContentsMargins(0,0,0,0);

    //! Constante A
    QHBoxLayout *csteIOLLay     = new QHBoxLayout();
    UpLabel* lblcsteAIOL        = new UpLabel;
    lblcsteAIOL                 ->setText(tr("Constante A"));
    UpLabel* lblAOptIOL         = new UpLabel;
    lblAOptIOL                  ->setText(tr("Optique"));
    lblAOptIOL                  ->setFixedSize(QSize(50,28));
    UpLabel* lblAEchoIOL        = new UpLabel;
    lblAEchoIOL                 ->setText(tr("Echo"));
    lblAEchoIOL                 ->setFixedSize(QSize(50,28));
    wdg_Aoptline                = new UpLineEdit();
    wdg_Aoptline                ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Aoptline));
    wdg_Aoptline                ->setFixedSize(QSize(50,28));
    wdg_Aecholine               = new UpLineEdit();
    wdg_Aecholine               ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Aecholine));
    wdg_Aecholine               ->setFixedSize(QSize(50,28));
    csteIOLLay                  ->addWidget(lblcsteAIOL);
    csteIOLLay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                  ->addWidget(lblAOptIOL);
    csteIOLLay                  ->addWidget(wdg_Aoptline);
    csteIOLLay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                  ->addWidget(lblAEchoIOL);
    csteIOLLay                  ->addWidget(wdg_Aecholine);
    csteIOLLay                  ->setSpacing(5);
    csteIOLLay                  ->setContentsMargins(0,0,0,0);
    for (int i=0; i < csteIOLLay->count(); ++i)
    {
        if (i==1)
            csteIOLLay->setStretch(i,5);
        else
            csteIOLLay->setStretch(i,1);
    }
    //! ACD
    QHBoxLayout *ACDLay         = new QHBoxLayout();
    UpLabel* lblACDIOL          = new UpLabel;
    lblACDIOL                   ->setText(tr("ACD"));
    lblACDIOL                   ->setFixedSize(QSize(50,28));
    QDoubleValidator *ACD_val   = new QDoubleValidator(1,8,2, this);
    wdg_ACDline                 = new UpLineEdit();
    wdg_ACDline                 ->setValidator(ACD_val);
    wdg_ACDline                 ->setFixedSize(QSize(50,28));
    UpLabel* lblHolIOL          = new UpLabel;
    lblHolIOL                   ->setText("Holladay1sf");
    QDoubleValidator *HOL_val   = new QDoubleValidator(0,8,2, this);
    wdg_holladayline            = new UpLineEdit();
    wdg_holladayline            ->setValidator(HOL_val);
    wdg_holladayline            ->setFixedSize(QSize(50,28));
    ACDLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    ACDLay                      ->addWidget(lblHolIOL);
    ACDLay                      ->addWidget(wdg_holladayline);
    ACDLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    ACDLay                      ->addWidget(lblACDIOL);
    ACDLay                      ->addWidget(wdg_ACDline);
    ACDLay                      ->setSpacing(5);
    ACDLay                      ->setContentsMargins(0,0,0,0);
    for (int i=0; i < ACDLay->count(); ++i)
    {
        if (i==0)
            ACDLay->setStretch(i,8);
        else
            ACDLay->setStretch(i,1);
    }

    //! Diametres optique et ht
    QHBoxLayout *diametresLay   = new QHBoxLayout();
    UpLabel* lbldiametres       = new UpLabel;
    lbldiametres                ->setText(tr("Diamètre (mm)"));
    UpLabel* lbldiaht           = new UpLabel;
    lbldiaht                    ->setText(tr("Hors-tout"));
    UpLabel* lbldiaopt          = new UpLabel;
    lbldiaopt                   ->setText(tr("Optique"));
    wdg_diaht                   = new UpLineEdit();
    wdg_diaht                   ->setValidator(new QRegularExpressionValidator(rgx_diaht, wdg_diaht));
    wdg_diaht                   ->setFixedSize(QSize(50,28));
    wdg_diaoptique              = new UpLineEdit();
    wdg_diaoptique              ->setValidator(new QRegularExpressionValidator(rgx_diaoptique, wdg_diaoptique));
    wdg_diaoptique              ->setFixedSize(QSize(50,28));
    diametresLay                ->addWidget(lbldiametres);
    diametresLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    diametresLay                ->addWidget(lbldiaht);
    diametresLay                ->addWidget(wdg_diaht);
    diametresLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    diametresLay                ->addWidget(lbldiaopt);
    diametresLay                ->addWidget(wdg_diaoptique);
    diametresLay                ->setSpacing(5);
    diametresLay                ->setContentsMargins(0,0,0,0);
    for (int i=0; i < diametresLay->count(); ++i)
    {
        if (i==1)
            diametresLay->setStretch(i,5);
        else
            diametresLay->setStretch(i,1);
    }

    //! Diametres injecteur
    QHBoxLayout *injecteurLay   = new QHBoxLayout();
    UpLabel* lbldiainjecteur    = new UpLabel;
    lbldiainjecteur             ->setText(tr("Injecteur (mm)"));
    wdg_diainjecteur            = new UpLineEdit();
    wdg_diainjecteur            ->setValidator(new QRegularExpressionValidator(rgx_diainjecteur, wdg_diaoptique));
    wdg_diainjecteur            ->setFixedSize(QSize(50,28));
    injecteurLay                ->addWidget(lbldiainjecteur);
    injecteurLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    injecteurLay                ->addWidget(wdg_diainjecteur);
    injecteurLay                ->setSpacing(5);
    injecteurLay                ->setContentsMargins(0,0,0,0);

    //! Plage puissances
    QHBoxLayout *puissancesLay  = new QHBoxLayout();
    UpLabel* lplpuissances      = new UpLabel;
    lplpuissances               ->setText(tr("Puissances"));
    UpLabel* lblpuissancemax    = new UpLabel;
    lblpuissancemax             ->setText(tr("Max."));
    UpLabel* lblpuissancemin    = new UpLabel;
    lblpuissancemin             ->setText(tr("Min."));
    wdg_puissancemaxspin        = new UpDoubleSpinBox();
    wdg_puissanceminspin        = new UpDoubleSpinBox();
    wdg_puissancemaxspin        ->setFixedSize(QSize(70,28));
    wdg_puissanceminspin        ->setFixedSize(QSize(70,28));
    wdg_puissancemaxspin        ->setRange(0.0, 35.0);
    wdg_puissancemaxspin        ->setSingleStep(0.5);
    wdg_puissanceminspin        ->setRange(-10.0, 15.0);
    wdg_puissanceminspin        ->setSingleStep(0.5);
    puissancesLay               ->addWidget(lplpuissances);
    puissancesLay               ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    puissancesLay               ->addWidget(lblpuissancemin);
    puissancesLay               ->addWidget(wdg_puissanceminspin);
    puissancesLay               ->addWidget(lblpuissancemax);
    puissancesLay               ->addWidget(wdg_puissancemaxspin);
    puissancesLay               ->setSpacing(5);
    puissancesLay               ->setContentsMargins(0,0,0,0);
    for (int i=0; i < puissancesLay->count(); ++i)
    {
        if (i==1)
            puissancesLay->setStretch(i,5);
        else
            puissancesLay->setStretch(i,1);
    }

    //! Plage cylindres
    QHBoxLayout *cylindresLay   = new QHBoxLayout();
    QHBoxLayout *wdgcylindresLay  = new QHBoxLayout();
    wdg_cylindres               = new QWidget();
    UpLabel* lblcylindremax     = new UpLabel;
    lblcylindremax              ->setText(tr("Max."));
    UpLabel* lblcylindremin     = new UpLabel;
    lblcylindremin              ->setText(tr("Cylindre Min."));
    wdg_cylindremaxspin         = new UpDoubleSpinBox();
    wdg_cylindreminspin         = new UpDoubleSpinBox();
    wdg_cylindremaxspin         ->setFixedSize(QSize(70,28));
    wdg_cylindreminspin         ->setFixedSize(QSize(70,28));
    wdg_cylindremaxspin         ->setRange(-8.0, 8.0);
    wdg_cylindremaxspin         ->setSingleStep(0.25);
    wdg_cylindreminspin         ->setRange(-8.0, 8.0);
    wdg_cylindreminspin         ->setSingleStep(0.25);
    wdg_toricchk                = new UpCheckBox(tr("Torique"));
    wdg_toricchk                ->setFixedHeight(35);
    cylindresLay                ->insertWidget(0,wdg_toricchk);
    cylindresLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    wdgcylindresLay             ->addWidget(lblcylindremin);
    wdgcylindresLay             ->addWidget(wdg_cylindreminspin);
    wdgcylindresLay             ->addWidget(lblcylindremax);
    wdgcylindresLay             ->addWidget(wdg_cylindremaxspin);
    wdgcylindresLay             ->setContentsMargins(0,0,0,0);
    wdg_cylindres               ->setLayout(wdgcylindresLay);
    cylindresLay                ->addWidget(wdg_cylindres);

    cylindresLay                ->setSpacing(5);
    cylindresLay                ->setContentsMargins(0,0,0,0);
    for (int i=0; i < cylindresLay->count(); ++i)
    {
        if (i==1)
            cylindresLay->setStretch(i,5);
        else
            cylindresLay->setStretch(i,1);
    }


    //! Constante precharge jaune multifocal
    QHBoxLayout *checkboxLay    = new QHBoxLayout();
    wdg_prechargechk            = new UpCheckBox(tr("Prechargé"));
    wdg_jaunechk                = new UpCheckBox(tr("Jaune"));
    wdg_edofchk                 = new UpCheckBox("EDOF");
    wdg_multifocalchk           = new UpCheckBox(tr("Multifocal"));
    checkboxLay                 ->addWidget(wdg_prechargechk);
    checkboxLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkboxLay                 ->addWidget(wdg_jaunechk);
    checkboxLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkboxLay                 ->addWidget(wdg_edofchk);
    checkboxLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkboxLay                 ->addWidget(wdg_multifocalchk);
    checkboxLay                 ->setSpacing(5);
    checkboxLay                 ->setContentsMargins(0,0,0,0);

    //! Haigis
    QHBoxLayout *HaigisLay      = new QHBoxLayout();
    UpLabel* HaigisIOLlbl       = new UpLabel;
    HaigisIOLlbl                ->setText("Haigis");
    UpLabel* Haigisalbl         = new UpLabel;
    Haigisalbl                  ->setText("a0");
    UpLabel* Haigisblbl         = new UpLabel;
    Haigisblbl                  ->setText("a1");
    UpLabel* Haigisclbl         = new UpLabel;
    Haigisclbl                  ->setText(tr("a2"));
    wdg_haigisaline             = new UpLineEdit();
    wdg_haigisaline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_haigisaline));
    wdg_haigisaline             ->setFixedSize(QSize(60,28));
    wdg_haigisbline             = new UpLineEdit();
    wdg_haigisbline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_haigisbline));
    wdg_haigisbline             ->setFixedSize(QSize(60,28));
    wdg_haigiscline             = new UpLineEdit();
    wdg_haigiscline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_haigiscline));
    wdg_haigiscline             ->setFixedSize(QSize(60,28));
    HaigisLay                   ->addWidget(HaigisIOLlbl);
    HaigisLay                   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                   ->addWidget(Haigisalbl);
    HaigisLay                   ->addWidget(wdg_haigisaline);
    HaigisLay                   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                   ->addWidget(Haigisblbl);
    HaigisLay                   ->addWidget(wdg_haigisbline);
    HaigisLay                   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                   ->addWidget(Haigisclbl);
    HaigisLay                   ->addWidget(wdg_haigiscline);
    HaigisLay                   ->setSpacing(5);
    HaigisLay                   ->setContentsMargins(0,0,0,0);
    for (int i=0; i < HaigisLay->count(); ++i)
    {
        if (i==1)
            HaigisLay->setStretch(i,5);
        else
            HaigisLay->setStretch(i,1);
    }

    //! Materiau - Image - Navigation - recopie
    QHBoxLayout *MateriauImgLay = new QHBoxLayout();
    UpLabel* Materiaulbl        = new UpLabel;
    UpLabel* Typelbl            = new UpLabel;
    Materiaulbl                 ->setText(tr("Materiau"));
    Typelbl                     ->setText(tr("Type"));
    wdg_imgIOL                  = new UpLabel;
    wdg_imgIOL                  ->setFixedSize(180,180);
    wdg_imgIOL                  ->setContextMenuPolicy(Qt::CustomContextMenu);
    setimage(m_nullimage);
    wdg_materiaubox             = new UpComboBox;
    wdg_typebox                 = new UpComboBox;
    wdg_materiaubox             ->setEditable(true);
    wdg_materiaubox             ->lineEdit()->setMaxLength(45);
    wdg_materiaubox             ->setFixedWidth(180);
    wdg_materiaubox             ->addItems(QStringList() << tr("Acrylique hydrophile") << tr("Acrylique hydrophobe") << tr("PMMA") << tr("copolymère"));
    wdg_typebox                 ->setEditable(false);
    wdg_typebox                 ->addItems(QStringList() << IOL_CP << IOL_CA << IOL_ADDON << IOL_IRIEN << IOL_CAREFRACTIF << IOL_AUTRE);
    wdg_typebox                 ->setFixedWidth(180);
    wdg_typebox                 ->setCurrentIndex(-1);
    wdg_materiaubox             ->setCurrentIndex(-1);
    lay_materiau                ->insertWidget(0,wdg_materiaubox);
    lay_materiau                ->insertWidget(0,Materiaulbl);
    lay_materiau                ->insertWidget(0,wdg_typebox);
    lay_materiau                ->insertWidget(0,Typelbl);
    lay_materiau                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    lay_materiau                ->setContentsMargins(0,0,0,0);
    MateriauImgLay              ->addLayout(lay_materiau);
    MateriauImgLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    MateriauImgLay              ->addWidget(wdg_imgIOL);
    MateriauImgLay              ->setContentsMargins(0,0,0,0);
    if (m_mode == Modification)
    {
        wdg_recopiebutton           = new UpPushButton ("Recopier l'IOL");
        wdg_recopiebutton           ->setVisible(false);
        lay_materiau                ->addWidget(wdg_recopiebutton);
        wdg_toolbar                 = new UpToolBar();
        lay_materiau                ->addWidget(wdg_toolbar);
        wdg_toolbar                 ->setEnabled(m_listeidIOLs.size()>1);
   }

    //! Remarque
    QHBoxLayout *remarqueLay    = new QHBoxLayout();
    QVBoxLayout *RemarqueHLay   = new QVBoxLayout();
    UpLabel* Remarquelbl        = new UpLabel;
    Remarquelbl                 ->setText(tr("Remarque"));
    wdg_remarquetxt             = new UpTextEdit();
    remarqueLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    remarqueLay                 ->insertWidget(0,Remarquelbl);
    RemarqueHLay                ->addLayout(remarqueLay);
    RemarqueHLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    RemarqueHLay                ->addWidget(wdg_remarquetxt);
    RemarqueHLay                ->setContentsMargins(0,0,0,0);

    wdg_inactifchk              = new UpCheckBox(tr("Discontinué"));
    wdg_inactifchk              ->setFocusPolicy(Qt::NoFocus);
    AjouteWidgetLayButtons(wdg_inactifchk, false);

    dlglayout()   ->insertLayout(0, RemarqueHLay);
    dlglayout()   ->insertLayout(0, MateriauImgLay);
    dlglayout()   ->insertLayout(0, checkboxLay);
    dlglayout()   ->insertLayout(0, HaigisLay);
    dlglayout()   ->insertLayout(0, cylindresLay);
    dlglayout()   ->insertLayout(0, puissancesLay);
    dlglayout()   ->insertLayout(0, injecteurLay);
    dlglayout()   ->insertLayout(0, diametresLay);
    dlglayout()   ->insertLayout(0, ACDLay);
    dlglayout()   ->insertLayout(0, csteIOLLay);
    dlglayout()   ->insertLayout(0, choixIOLLay);
    dlglayout()   ->insertLayout(0, choixManufacturerIOLLay);
    dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlglayout()   ->setSpacing(5);

    QList <QWidget*> ListTab;
    ListTab << wdg_manufacturercombo << wdg_nomiolline << wdg_Aoptline << wdg_Aecholine << wdg_holladayline << wdg_ACDline << wdg_diaht << wdg_diaoptique << wdg_diainjecteur
            << wdg_puissanceminspin << wdg_puissancemaxspin
            << wdg_toricchk << wdg_cylindreminspin << wdg_cylindremaxspin
            << wdg_haigisaline << wdg_haigisbline << wdg_haigiscline
            << wdg_prechargechk << wdg_jaunechk << wdg_multifocalchk << wdg_typebox << wdg_materiaubox << wdg_remarquetxt;
    for (int i = 0; i<ListTab.size()-1 ; i++ )
    {
        setTabOrder(ListTab.at(i), ListTab.at(i+1));
        ListTab.at(i)->installEventFilter(this);
    }

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    QFont font = qApp->font();
    font.setBold(true);
    //font.setItalic(true);
    //font.setPointSize(font.pointSize()+2);
    foreach (UpLabel *lbl, findChildren<UpLabel*>())
        lbl->setFont(font);
    foreach (UpLineEdit *line, findChildren<UpLineEdit*>())
        line->setAlignment(Qt::AlignCenter);
    TuneSize();

    connectSignals();
    if (m_mode == Modification)
        AfficheDatasIOL(m_currentIOL);
    foreach (QWidget *wdg, findChildren<QWidget*>())        //! ce micmac sert a créé une émission du signal uptoggled seulement si le checkbox est coché/décoché par l'utilisateur pas s'il est coché/décoché par le programme
    {
        UpCheckBox *chk = qobject_cast<UpCheckBox*>(wdg);
        if (chk)
            chk->installEventFilter(this);
    }

    wdg_puissancemaxspin->installEventFilter(this);
    wdg_cylindreminspin->installEventFilter(this);
    wdg_cylindremaxspin->installEventFilter(this);
    wdg_puissanceminspin->installEventFilter(this);
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));
    setStageCount(1);
}

bool dlg_identificationIOL::eventFilter(QObject *obj, QEvent *event)
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
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key()==Qt::Key_Return  && keyEvent->modifiers() == Qt::NoModifier) || keyEvent->key() == Qt::Key_Enter)
            return QWidget::focusNextChild();
    }
    if(event->type()==QEvent::MouseButtonPress)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
        {
            UpCheckBox *chk = qobject_cast<UpCheckBox*>(obj);
            if (chk)
                emit chk->uptoggled(!chk->isChecked()); //!la propriété checkecd est affectée après l'event keypress, il faut donc envoyer l'inverse de la propriété pour avoir sa vraie valeur
        }
    return QWidget::eventFilter(obj, event);
}

void dlg_identificationIOL::connectSignals()
{
     connect(OKButton, &QPushButton::clicked, this, &dlg_identificationIOL::OKpushButtonClicked);
     connect (wdg_manufacturercombo, QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [&](int id) {
                                                                                                                     int idman = wdg_manufacturercombo->itemData(id).toInt();
                                                                                                                     m_currentmanufacturer = Datas::I()->manufacturers->getById(idman);
                                                                                                                     if (m_currentmanufacturer && m_mode == Modification)
                                                                                                                     {
                                                                                                                         reconstruitListeIOLs(m_currentmanufacturer);
                                                                                                                         if (m_listeidIOLs.size() >0)
                                                                                                                         {
                                                                                                                             m_currentIOL = Datas::I()->iols->getById(m_listeidIOLs.first());
                                                                                                                             wdg_toolbar->setEnabled(m_listeidIOLs.size()>1);
                                                                                                                             NavigueVers(UpToolBar::_first);
                                                                                                                             OKButton->setEnabled(false);
                                                                                                                         }
                                                                                                                     }
                                                                                                                 });
     connect (wdg_nomiolline,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_Aoptline,          &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_Aecholine,         &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_ACDline,           &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_holladayline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_haigisaline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_haigisbline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_haigiscline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_materiaubox->lineEdit(),      &QLineEdit::textEdited,                      this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_materiaubox,       QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_typebox,           QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_remarquetxt,       &UpTextEdit::textEdited,                                this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_diaoptique,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_diaht,             &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_diainjecteur,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_prechargechk,      &UpCheckBox::uptoggled,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_edofchk,           &UpCheckBox::uptoggled,                                 this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    if (a) wdg_multifocalchk->setChecked(!a);
                                                                                                                 });
     connect (wdg_toricchk,          &UpCheckBox::uptoggled,                                 this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    wdg_cylindres->setVisible(a);
                                                                                                                 });
     connect (wdg_jaunechk,          &UpCheckBox::uptoggled,                                 this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    QString style = (a? "background-color: yellow" : "background-color: none" );
                                                                                                                    wdg_jaunechk->setStyleSheet(style);
                                                                                                                 });
     connect (wdg_inactifchk,        &UpCheckBox::uptoggled,                                 this,   [&](bool a) {
                                                                                                                    EnableWidget(!a);
                                                                                                                    EnableOKpushButton();
                                                                                                                  });
     connect (wdg_multifocalchk,     &UpCheckBox::uptoggled,                                 this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    if (a) wdg_edofchk->setChecked(!a);
                                                                                                                 });
     connect (wdg_puissancemaxspin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_puissanceminspin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_cylindremaxspin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_cylindreminspin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
     connect (wdg_imgIOL,            &QLabel::customContextMenuRequested,                    this,   &dlg_identificationIOL::menuChangeImage);
     connect (wdg_imgIOL,            &UpLabel::dblclick,                                     this,   [&] {if (wdg_imgIOL->isEnabled()) changeImage();});
     if (wdg_recopiebutton)
        connect (wdg_recopiebutton,     &UpPushButton::clicked,                              this,   &dlg_identificationIOL::creeCopieIOL);
     if (wdg_toolbar)
        connect (wdg_toolbar,           &UpToolBar::TBSignal,                                this,  [=] {NavigueVers(wdg_toolbar->choix());});
}

void dlg_identificationIOL::disconnectSignals()
{
     OKButton->disconnect();
     wdg_manufacturercombo->disconnect();
     wdg_nomiolline->disconnect();
     wdg_Aoptline->disconnect();
     wdg_Aecholine->disconnect();
     wdg_ACDline->disconnect();
     wdg_holladayline->disconnect();
     wdg_haigisaline->disconnect();
     wdg_haigisbline->disconnect();
     wdg_haigiscline->disconnect();
     wdg_materiaubox->lineEdit()->disconnect();
     wdg_materiaubox->disconnect();
     wdg_typebox->disconnect();
     wdg_remarquetxt->disconnect();
     wdg_diaoptique->disconnect();
     wdg_diaht->disconnect();
     wdg_diainjecteur->disconnect();
     wdg_prechargechk->disconnect();
     wdg_edofchk->disconnect();
     wdg_toricchk->disconnect();
     wdg_jaunechk->disconnect();
     wdg_inactifchk->disconnect();
     wdg_multifocalchk->disconnect();
     wdg_puissancemaxspin->disconnect();
     wdg_puissanceminspin->disconnect();
     wdg_cylindremaxspin->disconnect();
     wdg_cylindreminspin->disconnect();
     wdg_imgIOL->disconnect();
     if (wdg_recopiebutton)
         wdg_recopiebutton->disconnect();
     if (wdg_toolbar)
         wdg_toolbar->disconnect();
}


/*--------------------------------------------------------------------------------------------
-- Afficher la fiche de l'implant
--------------------------------------------------------------------------------------------*/
void dlg_identificationIOL::AfficheDatasIOL(IOL *iol)
{
    m_listbinds[CP_ARRAYIMG_IOLS] = QVariant();
    m_listbinds[CP_TYPIMG_IOLS] = "";
    if (iol == Q_NULLPTR)
        return;
    disconnectSignals();
    m_currentIOL = iol;
    if (m_currentmanufacturer)
        wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
    if (wdg_toolbar)
        wdg_toolbar         ->setEnabled(true);
    if (wdg_recopiebutton)
        wdg_recopiebutton   ->setEnabled(false);

    if (m_mode == Modification)
    {
        wdg_nomiolline      ->setText(m_currentIOL->modele());
        if (m_currentIOL->csteAopt() > 0.0)
            wdg_Aoptline        ->setText(QLocale().toString(m_currentIOL->csteAopt(), 'f', 1));
        if (m_currentIOL->csteAEcho() > 0.0)
            wdg_Aecholine       ->setText(QLocale().toString(m_currentIOL->csteAEcho(), 'f', 1));
        if (m_currentIOL->acd() > 0.0)
            wdg_ACDline         ->setText(QLocale().toString(m_currentIOL->acd(), 'f', 2));
        if (m_currentIOL->holladay() > 0.0)
            wdg_holladayline    ->setText(QLocale().toString(m_currentIOL->holladay(), 'f', 2));
        if (m_currentIOL->haigisa0() > 0.0)
            wdg_haigisaline     ->setText(QLocale().toString(m_currentIOL->haigisa0(), 'f', 4));
        if (m_currentIOL->haigisa1() > 0.0)
            wdg_haigisbline     ->setText(QLocale().toString(m_currentIOL->haigisa1(), 'f', 4));
        if (m_currentIOL->haigisa2() > 0.0)
            wdg_haigiscline     ->setText(QLocale().toString(m_currentIOL->haigisa2(), 'f', 4));
        wdg_puissancemaxspin->setValuewithPrefix(m_currentIOL->pwrmax());
        wdg_puissanceminspin->setValuewithPrefix(m_currentIOL->pwrmin());
        if (m_currentIOL->materiau() != "")
            wdg_materiaubox ->setCurrentText(m_currentIOL->materiau());
        else
            wdg_materiaubox ->setCurrentIndex(-1);
        if (m_currentIOL->type() != "")
            wdg_typebox     ->setCurrentText(m_currentIOL->type());
        else
            wdg_typebox     ->setCurrentIndex(-1);
        wdg_remarquetxt     ->setPlainText(m_currentIOL->remarque());
        wdg_inactifchk      ->setChecked(!m_currentIOL->isactif());
        wdg_jaunechk        ->setChecked(m_currentIOL->isjaune());
        if (m_currentIOL->isjaune())
            wdg_jaunechk->setStyleSheet("background-color: yellow");
        else
            wdg_jaunechk->setStyleSheet("");
        wdg_multifocalchk   ->setChecked(m_currentIOL->ismultifocal());
        wdg_prechargechk    ->setChecked(m_currentIOL->isprecharge());
        wdg_edofchk         ->setChecked(m_currentIOL->isedof());
        wdg_toricchk        ->setChecked(m_currentIOL->istoric());
        wdg_cylindres       ->setVisible(m_currentIOL->istoric());
        if (m_currentIOL->istoric())
        {
            if (m_currentIOL->cylmax() > 0.0)
                wdg_cylindremaxspin->setValuewithPrefix(m_currentIOL->cylmax());
            if (m_currentIOL->cylmin() > 0.0)
                wdg_cylindreminspin->setValuewithPrefix(m_currentIOL->cylmin());
        }
        if (m_currentIOL->diaall() > 0.0)
            wdg_diaht       ->setText(QLocale().toString(m_currentIOL->diaall(), 'f', 1));
        if (m_currentIOL->diaoptique() > 0.0)
            wdg_diaoptique   ->setText(QLocale().toString(m_currentIOL->diaoptique(), 'f', 1));
        if (m_currentIOL->diainjecteur() > 0.0)
            wdg_diainjecteur ->setText(QLocale().toString(m_currentIOL->diainjecteur(), 'f', 1));
        if (m_currentIOL->arrayimgiol() != QByteArray())
        {
            setimage(m_currentIOL->image());
            m_listbinds[CP_ARRAYIMG_IOLS] = m_currentIOL->arrayimgiol();
            m_listbinds[CP_TYPIMG_IOLS] = m_currentIOL->imageformat();
        }
        else setimage(m_nullimage);
        EnableWidget(m_currentIOL->isactif());
        int idx = m_listeidIOLs.indexOf(m_currentIOL->id());
        wdg_toolbar->First()    ->setEnabled(idx>0);
        wdg_toolbar->Prec()     ->setEnabled(idx>0);
        wdg_toolbar->Next()     ->setEnabled(idx < m_listeidIOLs.size()-1);
        wdg_toolbar->Last()     ->setEnabled(idx < m_listeidIOLs.size()-1);
    }
    connectSignals();
}

void dlg_identificationIOL::menuChangeImage()
{
    QMenu m_menuContextuel;
    QAction *pAction_ChangeImage = m_menuContextuel.addAction(tr("Modifier l'image"));
    connect (pAction_ChangeImage,  &QAction::triggered,    this, &dlg_identificationIOL::changeImage);
    if (m_currentimage != m_nullimage)
    {
        QAction *pAction_ChangeImage = m_menuContextuel.addAction(tr("Supprimer l'image"));
        connect (pAction_ChangeImage,  &QAction::triggered,    this, &dlg_identificationIOL::supprimeImage);
    }
    m_menuContextuel.exec(cursor().pos());
}

void dlg_identificationIOL::changeImage()
{
    int sizemaxi = 8192;
    QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), desktop + "/ImagesIOL" ,  tr("Images (*.pdf *.png *.jpg)"));
    if (fileName != "")
    {
        QFile file_origine;
        file_origine.setFileName(fileName);
        QFile file_image;
        QString formatdoc = QFileInfo(file_origine).suffix().toLower();
        QString m_pathdirstockageprovisoire = Procedures::I()->DefinitDossierImagerie();
        if (!QDir(m_pathdirstockageprovisoire).exists())
        {
            UpMessageBox::Watch(this, tr("Impossible d'enregistrer cette image"), tr("Le dossier d'imagerie") + "\n" + m_pathdirstockageprovisoire +"\n" + tr("n'existe pas") + "\n"
                                + tr("Revoyez le réglage de l'emplacement de ce dossier dans la fiche Paramètres"));
            return;
        }
        m_pathdirstockageprovisoire += NOM_DIR_PROV;
        if (!QDir(m_pathdirstockageprovisoire).exists())
            Utils::mkpath(m_pathdirstockageprovisoire);

        // Contenu du document------------------------------------------------------------------------------------------------------------------------------------------------
        QByteArray ba;
        QString nomfichresize = m_pathdirstockageprovisoire + "/resize" + QFileInfo(file_origine).fileName();
        QString szorigin, szfinal;
        // on vide le dossier provisoire
        QStringList listfichresize = QDir(m_pathdirstockageprovisoire).entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (int t=0; t<listfichresize.size(); t++)
        {
            QString nomdocrz  = listfichresize.at(t);
            QString CheminFichierResize = m_pathdirstockageprovisoire + "/" + nomdocrz;
            QFile file(CheminFichierResize);
            Utils::removeWithoutPermissions(file);
        }
        if (file_origine.open(QIODevice::ReadOnly))
        {
            double sz = file_origine.size();
            if (sz/(1024*1024) > 1)
                szorigin = QString::number(sz/(1024*1024),'f',1) + "Mo";
            else
                szorigin = QString::number(sz/1024,'f',1) + "Ko";
            szfinal = szorigin;
            Utils::copyWithPermissions(file_origine,nomfichresize);
            file_image.setFileName(nomfichresize);
            if ((formatdoc == JPG ||formatdoc == PNG) && sz > sizemaxi)
            {
                QImage  img(nomfichresize);
                Utils::removeWithoutPermissions(file_image);
                QPixmap pixmap;
                pixmap = pixmap.fromImage(img);
                pixmap.save(nomfichresize, "jpeg");
                pixmap = pixmap.fromImage(img.scaledToWidth(256));
                int     tauxcompress = 90;
                while (sz > sizemaxi && tauxcompress > 1)
                {
                    pixmap.save(nomfichresize, "jpeg",tauxcompress);
                    sz = file_image.size();
                    if (tauxcompress > 19)
                        tauxcompress -= 10;
                    else
                        tauxcompress -= 1;
                }
                if (sz/(1024*1024) > 1)
                    szfinal = QString::number(sz/(1024*1024),'f',0) + "Mo";
                else
                    szfinal = QString::number(sz/1024,'f',0) + "Ko";
            }
            file_image.open(QIODevice::ReadOnly);
            ba = file_image.readAll();
            Utils::removeWithoutPermissions(file_image);
        }
        m_listbinds[CP_ARRAYIMG_IOLS] = ba;
        QString suffix = QFileInfo(file_origine).suffix().toLower();
        suffix = (suffix == PDF? PDF : JPG);
        m_listbinds[CP_TYPIMG_IOLS] = suffix;
        EnableOKpushButton();
        QImage img;
        if (suffix == PDF)
            img = Utils::calcImagefromPdf(ba).at(0);
        else
            img = QImage(fileName);
        setimage(img);
        Utils::removeWithoutPermissions(file_origine);
    }
}

void dlg_identificationIOL::creeCopieIOL()
{

}

void dlg_identificationIOL::supprimeImage()
{
    setimage(m_nullimage);
    m_listbinds[CP_ARRAYIMG_IOLS] = QByteArray();
    m_listbinds[CP_TYPIMG_IOLS] = "";
    EnableOKpushButton();
}

void dlg_identificationIOL:: EnableOKpushButton()
{
    bool a  = wdg_nomiolline->text() != ""
           && wdg_manufacturercombo->currentData().toInt()>0;
    OKButton            ->setEnabled(a);
    if (wdg_toolbar)
        wdg_toolbar         ->setEnabled(!a);
    if(wdg_recopiebutton)
        wdg_recopiebutton   ->setEnabled(false);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationIOL::EnableWidget(bool a)
{
    foreach (QWidget *wdg, findChildren<QWidget*>())
        if (wdg != wdg_manufacturercombo && wdg != widgetbuttons()
            && wdg != wdg_toolbar  && !wdg_toolbar->isAncestorOf(wdg) && !widgetbuttons()->isAncestorOf(wdg))
                wdg->setEnabled(a);
}

void dlg_identificationIOL::NavigueVers(UpToolBar::Choix choix)
{
    int idx = m_listeidIOLs.indexOf(m_currentIOL->id());
    if (choix == UpToolBar::_last)
        idx = m_listeidIOLs.size()-1;
    else if (choix == UpToolBar::_first)
        idx = 0;
    else if (choix == UpToolBar::_next)
        idx += 1;
    else if (choix == UpToolBar::_prec)
        idx -= 1;
    wdg_toolbar->First()    ->setEnabled(idx>0);
    wdg_toolbar->Prec()     ->setEnabled(idx>0);
    wdg_toolbar->Next()     ->setEnabled(idx < m_listeidIOLs.size()-1);
    wdg_toolbar->Last()     ->setEnabled(idx < m_listeidIOLs.size()-1);
    if (idx>-1)
    {
        m_currentIOL = Datas::I()->iols->getById(m_listeidIOLs.at(idx));
        AfficheDatasIOL(m_currentIOL);
    }
}

void dlg_identificationIOL::OKpushButtonClicked()
{
    qApp->focusWidget()->clearFocus();
    if (wdg_nomiolline->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas indiqué le modèle d'implant!"));
        wdg_nomiolline->setFocus();
        return;
    }
    // D - On vérifie ensuite si cet implant existe déjà
    bool ok;
    QString requete = "select " CP_ID_IOLS " from " TBL_IOLS
            " where " CP_MODELNAME_IOLS " = '" + wdg_nomiolline->text() + "'";
    QVariantList ioldata = DataBase::I()->getFirstRecordFromStandardSelectSQL(requete,ok, tr("Impossible d'interroger la table des correspondants!"));
    if (!ok)
    {
        reject();
        return;
    }
    if (ioldata.size() > 0)
    {
        switch (m_mode) {
        case Modification:
            if (ioldata.at(0).toInt() != m_currentIOL->id())
            {
                UpMessageBox::Watch(this,tr("Cet implant existe déjà!"));
                delete m_currentIOL;
                m_currentIOL = Datas::I()->iols->getById(ioldata.at(0).toInt());
                OKButton->setEnabled(false);
                wdg_toolbar         ->setEnabled(true);
                wdg_recopiebutton   ->setEnabled(true);
                AfficheDatasIOL(m_currentIOL);
                disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationIOL::OKpushButtonClicked);
                connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationIOL::accept);
                return;
            }
            break;
        case Creation:
            UpMessageBox::Watch(this,tr("Cet implant existe déjà!"));
            m_currentIOL = Datas::I()->iols->getById(ioldata.at(0).toInt());
            m_currentmanufacturer = Datas::I()->manufacturers->getById(m_currentIOL->idmanufacturer());
            reconstruitListeIOLs(m_currentmanufacturer);
            OKButton->setEnabled(false);
            wdg_toolbar         ->setEnabled(true);
            wdg_recopiebutton   ->setEnabled(true);
            m_mode = Modification;           
            wdg_recopiebutton           = new UpPushButton ("Recopier l'IOL");
            wdg_recopiebutton           ->setVisible(false);
            lay_materiau                ->addWidget(wdg_recopiebutton);
            wdg_toolbar                 = new UpToolBar();
            lay_materiau                ->addWidget(wdg_toolbar);
            wdg_toolbar                 ->setEnabled(m_listeidIOLs.size()>1);
            AfficheDatasIOL(m_currentIOL);
            disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationIOL::OKpushButtonClicked);
            connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationIOL::accept);
            return;
        }
     }

    m_listbinds[CP_MODELNAME_IOLS]      = wdg_nomiolline->text();
    m_listbinds[CP_IDMANUFACTURER_IOLS] = m_currentmanufacturer->id();
    m_listbinds[CP_ACD_IOLS]            = (QLocale().toDouble(wdg_ACDline->text()) >0.0?      QLocale().toDouble(wdg_ACDline->text())     : QVariant());
    m_listbinds[CP_HOLL1_IOLS]          = (QLocale().toDouble(wdg_holladayline->text()) >0.0? QLocale().toDouble(wdg_holladayline->text()): QVariant());
    m_listbinds[CP_CSTEAOPT_IOLS]       = (QLocale().toDouble(wdg_Aoptline->text()) >0.0?     QLocale().toDouble(wdg_Aoptline->text())    : QVariant());
    m_listbinds[CP_CSTEAECHO_IOLS]      = (QLocale().toDouble(wdg_Aecholine->text()) >0.0?    QLocale().toDouble(wdg_Aecholine->text())   : QVariant());
    m_listbinds[CP_HAIGISA0_IOLS]       = (QLocale().toDouble(wdg_haigisaline->text()) >0.0?  QLocale().toDouble(wdg_haigisaline->text()) : QVariant());
    m_listbinds[CP_HAIGISA1_IOLS]       = (QLocale().toDouble(wdg_haigisbline->text()) >0.0?  QLocale().toDouble(wdg_haigisbline->text()) : QVariant());
    m_listbinds[CP_HAIGISA2_IOLS]       = (QLocale().toDouble(wdg_haigiscline->text()) >0.0?  QLocale().toDouble(wdg_haigiscline->text()) : QVariant());
    m_listbinds[CP_MATERIAU_IOLS]       = wdg_materiaubox->currentText();
    m_listbinds[CP_REMARQUE_IOLS]       = wdg_remarquetxt->toPlainText();
    m_listbinds[CP_DIAALL_IOLS]         = (QLocale().toDouble(wdg_diaht->text()) >0.0?        QLocale().toDouble(wdg_diaht->text())       : QVariant());
    m_listbinds[CP_DIAOPT_IOLS]         = (QLocale().toDouble(wdg_diaoptique->text()) >0.0?   QLocale().toDouble(wdg_diaoptique->text())  : QVariant());
    m_listbinds[CP_DIAINJECTEUR_IOLS]   = (QLocale().toDouble(wdg_diainjecteur->text()) >0.0? QLocale().toDouble(wdg_diainjecteur->text()): QVariant());
    m_listbinds[CP_PRECHARGE_IOLS]      = (wdg_prechargechk->isChecked()?   "1" : QVariant());
    m_listbinds[CP_MAXPWR_IOLS]         = wdg_puissancemaxspin->value();
    m_listbinds[CP_MINPWR_IOLS]         = wdg_puissanceminspin->value();
    m_listbinds[CP_MAXCYL_IOLS]         = (wdg_toricchk->checkState() == Qt::Checked? wdg_cylindremaxspin->value() : QVariant());
    m_listbinds[CP_MINCYL_IOLS]         = (wdg_toricchk->checkState() == Qt::Checked? wdg_cylindreminspin->value() : QVariant());
    m_listbinds[CP_JAUNE_IOLS]          = (wdg_jaunechk->isChecked()?       "1" : QVariant());
    m_listbinds[CP_MULTIFOCAL_IOLS]     = (wdg_multifocalchk->isChecked()?  "1" : QVariant());
    m_listbinds[CP_INACTIF_IOLS]        = (wdg_inactifchk->isChecked()?     "1" : QVariant());
    m_listbinds[CP_EDOF_IOLS]           = (wdg_edofchk->isChecked()?        "1" : QVariant());
    m_listbinds[CP_TORIC_IOLS]          = (wdg_toricchk->isChecked()?       "1" : QVariant());
    m_listbinds[CP_TYP_IOLS]            = (wdg_typebox->currentIndex()>-1?   QString::number(wdg_typebox->currentIndex()+1) : QVariant());
    if (m_mode == Creation)
        m_currentIOL = Datas::I()->iols->CreationIOL(m_listbinds);
    else if (m_mode == Modification)
        DataBase::I()->UpDateIOL(m_currentIOL->id(), m_listbinds);
    accept();
}

void dlg_identificationIOL::reconstruitListeIOLs(Manufacturer *man)
{
    m_currentmanufacturer = man;
    m_listeidIOLs.clear();
    if (!m_IOLsmodel)
        m_IOLsmodel = new QStandardItemModel(this);
    else
        m_IOLsmodel->clear();
    if (!m_currentmanufacturer)
        return;
    UpStandardItem *pitem;
    foreach (IOL* iol, *Datas::I()->iols->iols())
    {
        if (iol->idmanufacturer() == man->id())
        {
            pitem = new UpStandardItem(iol->modele(), iol);
            m_IOLsmodel->appendRow(pitem);
        }
    }
    m_IOLsmodel->sort(0);
    for (int i=0; i<m_IOLsmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_IOLsmodel->item(i));
        if (itm)
        {
            IOL* iol = qobject_cast<IOL*>(itm->item());
            if (iol)
                m_listeidIOLs << iol->id();
        }
    }
}


void dlg_identificationIOL::setimage(QImage img)
{
    wdg_imgIOL   ->setPixmap(QPixmap::fromImage(img.scaled(wdg_imgIOL->width(),wdg_imgIOL->height(), Qt::KeepAspectRatio)));
    m_currentimage = img;
}
