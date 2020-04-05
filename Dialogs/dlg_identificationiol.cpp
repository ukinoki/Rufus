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

dlg_identificationIOL::dlg_identificationIOL(enum Mode mode, IOL *iol, Manufacturer *man, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentIOL", parent)
{
    if (Datas::I()->manufacturers->manufacturers()->size() == 0)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Aucun fabricant enregistré"), tr("Vous devez avoir des fabricants enregistrés dans la base pour pouvoir gérer les implants"));
        m_initok = false;
        return;
    }
    m_mode = mode;
    if (iol != Q_NULLPTR)
        m_currentIOL      = iol;
    m_currentmanufacturer = man;
    if ((m_mode == Creation && man == Q_NULLPTR) || m_mode == Modification)
        if (m_currentIOL)
            m_currentmanufacturer = Datas::I()->manufacturers->getById(m_currentIOL->idmanufacturer());
    setWindowTitle(m_mode == Creation? tr("Enregistrer un IOL") : tr("Modifier un IOL"));

    //! FABRICANT
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
    m_manufacturersmodel->sort(0);

    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    wdg_manufacturercombo = new QComboBox();
    wdg_manufacturercombo       ->setEditable(true);
    wdg_manufacturercombo       ->lineEdit()->setAlignment(Qt::AlignCenter);
    wdg_manufacturercombo       ->lineEdit()->setFocusPolicy(Qt::NoFocus);
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        wdg_manufacturercombo   ->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
        wdg_manufacturercombo   ->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
    }
    if (m_currentmanufacturer)
    {
        wdg_manufacturercombo   ->setEnabled(false);
        wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
    }
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
    lblIOL                      ->setText(tr("Nom du modèle"));
    wdg_nomiolline              = new UpLineEdit();
    wdg_nomiolline              ->setFixedSize(QSize(150,28));
    wdg_nomiolline              ->setValidator(new QRegExpValidator(Utils::rgx_cotation));
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
    wdg_Aoptline                ->setValidator(new QRegExpValidator(rgx_csteA, wdg_Aoptline));
    wdg_Aoptline                ->setFixedSize(QSize(50,28));
    wdg_Aecholine               = new UpLineEdit();
    wdg_Aecholine               ->setValidator(new QRegExpValidator(rgx_csteA, wdg_Aecholine));
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
    QDoubleValidator *ACD_val   = new QDoubleValidator(1,8,2, this);
    wdg_ACDline                 = new UpLineEdit();
    wdg_ACDline                 ->setValidator(ACD_val);
    wdg_ACDline                 ->setFixedSize(QSize(50,28));
    ACDLay                      ->addWidget(lblACDIOL);
    ACDLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    ACDLay                      ->addWidget(wdg_ACDline);
    ACDLay                      ->setSpacing(5);
    ACDLay                      ->setContentsMargins(0,0,0,0);

    //! Diametres optique et ht
    QHBoxLayout *diametresLay   = new QHBoxLayout();
    UpLabel* lbldiametres       = new UpLabel;
    lbldiametres                ->setText(tr("Diamètre"));
    UpLabel* lbldiaht           = new UpLabel;
    lbldiaht                    ->setText(tr("Hors-tout"));
    UpLabel* lbldiaopt          = new UpLabel;
    lbldiaopt                   ->setText(tr("Optique"));
    wdg_diaht                   = new UpLineEdit();
    wdg_diaht                   ->setValidator(new QRegExpValidator(rgx_diaht, wdg_diaht));
    wdg_diaht                   ->setFixedSize(QSize(50,28));
    wdg_diaoptique              = new UpLineEdit();
    wdg_diaoptique              ->setValidator(new QRegExpValidator(rgx_diaoptique, wdg_diaoptique));
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
    lbldiainjecteur             ->setText(tr("Injecteur"));
    wdg_diainjecteur            = new UpLineEdit();
    wdg_diainjecteur            ->setValidator(new QRegExpValidator(rgx_diainjecteur, wdg_diaoptique));
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
    wdg_toricchk                = new QCheckBox(tr("Torique"));
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
    wdg_prechargechk            = new QCheckBox(tr("Prechargé"));
    wdg_jaunechk                = new QCheckBox(tr("Jaune"));
    wdg_edofchk                 = new QCheckBox("EDOF");
    wdg_multifocalchk           = new QCheckBox(tr("Multifocal"));
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
    wdg_haigisaline             ->setValidator(new QRegExpValidator(rgx_haigis, wdg_haigisaline));
    wdg_haigisaline             ->setFixedSize(QSize(60,28));
    wdg_haigisbline             = new UpLineEdit();
    wdg_haigisbline             ->setValidator(new QRegExpValidator(rgx_haigis, wdg_haigisbline));
    wdg_haigisbline             ->setFixedSize(QSize(60,28));
    wdg_haigiscline             = new UpLineEdit();
    wdg_haigiscline             ->setValidator(new QRegExpValidator(rgx_haigis, wdg_haigiscline));
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

    //! Materiau - Image
    QVBoxLayout *MateriauLay    = new QVBoxLayout();
    QHBoxLayout *MateriauImgLay = new QHBoxLayout();
    UpLabel* Materiaulbl        = new UpLabel;
    Materiaulbl                 ->setText(tr("Materiau"));
    wdg_imgIOL                  = new UpLabel;
    setimage(m_nullimage);
    wdg_imgIOL                  ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_materiauline            = new UpComboBox;
    wdg_materiauline            ->setEditable(true);
    wdg_materiauline            ->lineEdit()->setMaxLength(45);
    wdg_materiauline            ->addItems(QStringList() << tr("Acrylique hydrophile") << tr("Acrylique hydrophobe") << tr("PMMA"));
    MateriauLay                 ->insertWidget(0,wdg_materiauline);
    MateriauLay                 ->insertWidget(0,Materiaulbl);
    MateriauLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    MateriauImgLay              ->addLayout(MateriauLay);
    MateriauImgLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    MateriauImgLay              ->addWidget(wdg_imgIOL);
    MateriauImgLay              ->setContentsMargins(0,0,0,0);

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

    wdg_inactifchk              = new QCheckBox(tr("Discontinué"));
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
    ListTab << wdg_manufacturercombo << wdg_nomiolline << wdg_Aoptline << wdg_Aecholine << wdg_ACDline << wdg_diaht << wdg_diaoptique << wdg_diainjecteur
            << wdg_puissanceminspin << wdg_puissancemaxspin
            << wdg_toricchk << wdg_cylindreminspin << wdg_cylindremaxspin
            << wdg_haigisaline << wdg_haigisbline << wdg_haigiscline
            << wdg_prechargechk << wdg_jaunechk << wdg_multifocalchk << wdg_materiauline << wdg_remarquetxt;
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

    AfficheDatasIOL();

    connect(OKButton, &QPushButton::clicked, this, &dlg_identificationIOL::OKpushButtonClicked);
    connect (wdg_manufacturercombo, QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [&](int id) {
                                                                                                                    int idman = wdg_manufacturercombo->itemData(id).toInt();
                                                                                                                    m_currentmanufacturer = Datas::I()->manufacturers->getById(idman);
                                                                                                                    EnableOKpushButton();
                                                                                                                });
    connect (wdg_nomiolline,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Aoptline,          &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Aecholine,         &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_ACDline,           &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigisaline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigisbline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigiscline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_materiauline->lineEdit(),      &QLineEdit::textEdited,                     this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_materiauline,      QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_remarquetxt,       &QTextEdit::textChanged,                                this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_diaoptique,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_diaht,             &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_diainjecteur,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_prechargechk,      &QCheckBox::stateChanged,                               this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_edofchk,           &QCheckBox::stateChanged,                               this,   [&] {
                                                                                                            EnableOKpushButton();
                                                                                                            if (wdg_edofchk->isChecked())
                                                                                                                wdg_multifocalchk->setChecked(false);
                                                                                                        });
    connect (wdg_toricchk,          &QCheckBox::stateChanged,                               this,   [&] {
                                                                                                            EnableOKpushButton();
                                                                                                            wdg_cylindres->setVisible(wdg_toricchk->checkState() == Qt::Checked);
                                                                                                        });
    connect (wdg_jaunechk,          &QCheckBox::stateChanged,                               this,   [&] {
                                                                                                            EnableOKpushButton();
                                                                                                            QString style = (wdg_jaunechk->checkState() == Qt::Checked?
                                                                                                            "background-color: yellow" : "background-color: none" );
                                                                                                            wdg_jaunechk->setStyleSheet(style);
                                                                                                        });
    connect (wdg_inactifchk,        &QCheckBox::stateChanged,                               this,   [&] {
                                                                                                            foreach (QWidget *wdg, findChildren<QWidget*>())
                                                                                                                if (wdg != wdg_manufacturercombo && wdg != widgetbuttons() && !widgetbuttons()->isAncestorOf(wdg))
                                                                                                                    wdg->setEnabled(!wdg_inactifchk->isChecked());
                                                                                                                EnableOKpushButton();
                                                                                                        });
    connect (wdg_multifocalchk,     &QCheckBox::stateChanged,                               this,   [&] {
                                                                                                            EnableOKpushButton();
                                                                                                            if (wdg_multifocalchk->isChecked())
                                                                                                                wdg_edofchk->setChecked(false);
                                                                                                        });
    connect (wdg_puissancemaxspin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_puissanceminspin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_cylindremaxspin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_cylindreminspin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_imgIOL,            &QLabel::customContextMenuRequested,                    this,   &dlg_identificationIOL::menuChangeImage);
    connect (wdg_imgIOL,            &UpLabel::dblclick,                                     this,   &dlg_identificationIOL::changeImage);

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
        UpDoubleSpinBox* objUpdSpin = dynamic_cast<UpDoubleSpinBox*>(obj);
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
    return QWidget::eventFilter(obj, event);
}

/*--------------------------------------------------------------------------------------------
-- Afficher la fiche de l'implant
--------------------------------------------------------------------------------------------*/
void dlg_identificationIOL::AfficheDatasIOL()
{
    if (!m_currentIOL)
        return;
    if (m_currentmanufacturer)
        wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));

    if (m_mode == Modification)
    {
        wdg_nomiolline      ->setText(m_currentIOL->modele());
        if (m_currentIOL->csteAopt() > 0.0)
            wdg_Aoptline        ->setText(QLocale().toString(m_currentIOL->csteAopt(), 'f', 1));
        if (m_currentIOL->csteAEcho() > 0.0)
            wdg_Aecholine       ->setText(QLocale().toString(m_currentIOL->csteAEcho(), 'f', 1));
        if (m_currentIOL->acd() > 0.0)
            wdg_ACDline         ->setText(QLocale().toString(m_currentIOL->acd(), 'f', 2));
        if (m_currentIOL->haigisa0() > 0.0)
            wdg_haigisaline     ->setText(QLocale().toString(m_currentIOL->haigisa0(), 'f', 4));
        if (m_currentIOL->haigisa1() > 0.0)
            wdg_haigisbline     ->setText(QLocale().toString(m_currentIOL->haigisa1(), 'f', 4));
        if (m_currentIOL->haigisa2() > 0.0)
            wdg_haigiscline     ->setText(QLocale().toString(m_currentIOL->haigisa2(), 'f', 4));
        if (m_currentIOL->pwrmax() > 0.0)
        {
            wdg_puissancemaxspin->setValue(m_currentIOL->pwrmax());
            PrefixePlus(wdg_puissancemaxspin);
        }
        if (m_currentIOL->pwrmin() > 0.0)
        {
            wdg_puissanceminspin->setValue(m_currentIOL->pwrmin());
            PrefixePlus(wdg_puissanceminspin);
        }
        wdg_materiauline    ->setCurrentText(m_currentIOL->materiau());
        wdg_remarquetxt     ->setPlainText(m_currentIOL->remarque());
        wdg_inactifchk      ->setChecked(!m_currentIOL->isactif());
        wdg_jaunechk        ->setChecked(m_currentIOL->isjaune());
        wdg_multifocalchk   ->setChecked(m_currentIOL->ismultifocal());
        wdg_prechargechk    ->setChecked(m_currentIOL->isprecharge());
        wdg_edofchk         ->setChecked(m_currentIOL->isedof());
        wdg_toricchk        ->setChecked(m_currentIOL->istoric());
        wdg_cylindres       ->setVisible(m_currentIOL->istoric());
        if (m_currentIOL->istoric())
        {
            if (m_currentIOL->cylmax() > 0.0)
            {
                wdg_cylindremaxspin->setValue(m_currentIOL->cylmax());
                PrefixePlus(wdg_cylindremaxspin);
            }
            if (m_currentIOL->cylmin() > 0.0)
            {
                wdg_cylindreminspin->setValue(m_currentIOL->cylmin());
                PrefixePlus(wdg_cylindreminspin);
            }
        }
        if (m_currentIOL->diaall() > 0.0)
            wdg_diaht       ->setText(QLocale().toString(m_currentIOL->diaall(), 'f', 1));
        if (m_currentIOL->diaoptique() > 0.0)
            wdg_diaoptique   ->setText(QLocale().toString(m_currentIOL->diaoptique(), 'f', 1));
        if (m_currentIOL->diainjecteur() > 0.0)
            wdg_diainjecteur ->setText(QLocale().toString(m_currentIOL->diainjecteur(), 'f', 1));
        if (m_currentIOL->imgiol() != QByteArray())
        {
            QImage image;
            if (m_currentIOL->imageformat() == PDF)
                setpdf(m_currentIOL->imgiol());
            else if (image.loadFromData(m_currentIOL->imgiol()))
                setimage(image);
        }
    }
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
    QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), desktop,  tr("Images (*.pdf *.png *.jpg)"));
    if (fileName != "")
    {
        QFile fileimage(fileName);
        QString formatdoc = QFileInfo(fileimage).suffix().toLower();
        QString m_pathdirstockageprovisoire = Procedures::I()->DefinitDossierImagerie() + NOM_DIR_PROV;
        // Contenu du document------------------------------------------------------------------------------------------------------------------------------------------------
        QByteArray ba;
        QString nomfichresize = m_pathdirstockageprovisoire + "/resize" + QFileInfo(fileimage).fileName();
        QString szorigin, szfinal;
        // on vide le dossier provisoire
        QStringList listfichresize = QDir(m_pathdirstockageprovisoire).entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (int t=0; t<listfichresize.size(); t++)
        {
            QString nomdocrz  = listfichresize.at(t);
            QString CheminFichierResize = m_pathdirstockageprovisoire + "/" + nomdocrz;
            QFile(CheminFichierResize).remove();
        }
        if (fileimage.open(QIODevice::ReadOnly))
        {
            double sz = fileimage.size();
            if (sz/(1024*1024) > 1)
                szorigin = QString::number(sz/(1024*1024),'f',1) + "Mo";
            else
                szorigin = QString::number(sz/1024,'f',1) + "Ko";
            szfinal = szorigin;
            fileimage.copy(nomfichresize);
            fileimage.setFileName(nomfichresize);
            if (formatdoc == "jpg" && sz > TAILLEMAXIIMAGES)
            {
                QImage  img(nomfichresize);
                fileimage.remove();
                QPixmap pixmap;
                pixmap = pixmap.fromImage(img.scaledToWidth(2560,Qt::SmoothTransformation));
                int     tauxcompress = 90;
                while (sz > TAILLEMAXIIMAGES && tauxcompress > 1)
                {
                    pixmap.save(nomfichresize, "jpeg",tauxcompress);
                    sz = fileimage.size();
                    tauxcompress -= 10;
                }
                if (sz/(1024*1024) > 1)
                    szfinal = QString::number(sz/(1024*1024),'f',0) + "Mo";
                else
                    szfinal = QString::number(sz/1024,'f',0) + "Ko";
            }
            fileimage.open(QIODevice::ReadOnly);
            ba = fileimage.readAll();
        }
        m_listbinds[CP_IMG_IOLS] = ba;
        QString suffix = QFileInfo(fileimage).suffix().toLower();
        m_listbinds[CP_TYPIMG_IOLS] = suffix;
        EnableOKpushButton();
        if (suffix == PDF)
            setpdf(m_listbinds[CP_IMG_IOLS].toByteArray());
        else
            setimage(QImage(fileName));
    }
}

void dlg_identificationIOL::supprimeImage()
{
    setimage(m_nullimage);
    m_listbinds[CP_IMG_IOLS] = QByteArray();
    m_listbinds[CP_TYPIMG_IOLS] = "";
    EnableOKpushButton();
}

void    dlg_identificationIOL:: EnableOKpushButton()
{
    bool a  = wdg_nomiolline->text() != ""
           && wdg_manufacturercombo->currentData().toInt()>0;
    OKButton->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationIOL::OKpushButtonClicked()
{
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
                AfficheDatasIOL();
                disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationIOL::OKpushButtonClicked);
                connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationIOL::accept);
                return;
            }
            break;
        case Creation:
            UpMessageBox::Watch(this,tr("Cet implant existe déjà!"));
            m_currentIOL = Datas::I()->iols->getById(ioldata.at(0).toInt());
            OKButton->setEnabled(false);
            m_mode = Modification;
            AfficheDatasIOL();
            disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationIOL::OKpushButtonClicked);
            connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationIOL::accept);
            return;
        }
     }

    m_listbinds[CP_MODELNAME_IOLS]      = wdg_nomiolline->text();
    m_listbinds[CP_IDMANUFACTURER_IOLS] = m_currentmanufacturer->id();
    m_listbinds[CP_ACD_IOLS]            = (QLocale().toDouble(wdg_ACDline->text()) >0.0?      QLocale().toDouble(wdg_ACDline->text())     : QVariant());
    m_listbinds[CP_CSTEAOPT_IOLS]       = (QLocale().toDouble(wdg_Aoptline->text()) >0.0?     QLocale().toDouble(wdg_Aoptline->text())    : QVariant());
    m_listbinds[CP_CSTEAECHO_IOLS]      = (QLocale().toDouble(wdg_Aecholine->text()) >0.0?    QLocale().toDouble(wdg_Aecholine->text())   : QVariant());
    m_listbinds[CP_HAIGISA0_IOLS]       = (QLocale().toDouble(wdg_haigisaline->text()) >0.0?  QLocale().toDouble(wdg_haigisaline->text()) : QVariant());
    m_listbinds[CP_HAIGISA1_IOLS]       = (QLocale().toDouble(wdg_haigisbline->text()) >0.0?  QLocale().toDouble(wdg_haigisbline->text()) : QVariant());
    m_listbinds[CP_HAIGISA2_IOLS]       = (QLocale().toDouble(wdg_haigiscline->text()) >0.0?  QLocale().toDouble(wdg_haigiscline->text()) : QVariant());
    m_listbinds[CP_MATERIAU_IOLS]       = wdg_materiauline->currentText();
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
    if (m_mode == Creation)
        m_currentIOL = Datas::I()->iols->CreationIOL(m_listbinds);
    else if (m_mode == Modification)
        DataBase::I()->UpDateIOL(m_currentIOL->id(), m_listbinds);
    accept();
}

void dlg_identificationIOL::setimage(QImage img)
{
    int l = img.width();
    int h = img.height();
    int max = (l>h? l : h);
    int min = (max==l? h : l);
    double prop = max/170;
    double width  = (max == l? max : min) /prop;
    double height = (max == h? max : min) /prop;
    wdg_imgIOL   ->setFixedSize(QSize(int(width), int(height)));
    wdg_imgIOL   ->setPixmap(QPixmap::fromImage(img.scaled(wdg_imgIOL->width(),wdg_imgIOL->height())));
    m_currentimage = img;
}

void dlg_identificationIOL::setpdf(QByteArray ba)
{
    Poppler::Document* document = Poppler::Document::loadFromData(ba);
    if (!document || document->isLocked()) {
        UpMessageBox::Watch(this,tr("Impossible de charger le document"));
        delete document;
        return;
    }
    if (document == Q_NULLPTR) {
        UpMessageBox::Watch(this,tr("Impossible de charger le document"));
        delete document;
        return;
    }
    document->setRenderHint(Poppler::Document::TextAntialiasing);
    Poppler::Page* pdfPage = document->page(0);  // Document starts at page 0
    if (pdfPage == Q_NULLPTR) {
        UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
        delete document;
        return;
    }
    QImage image = pdfPage->renderToImage(300,300);
    if (image.isNull()) {
        UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
        delete document;
        return;
    }
    setimage(image);
    delete document;
}

//---------------------------------------------------------------------------------------------------------
// Traitement du prefixe + ou - devant les doubles.
//---------------------------------------------------------------------------------------------------------
void dlg_identificationIOL::PrefixePlus(QDoubleSpinBox *spinbox)
{
    spinbox->setPrefix("");
    if (spinbox->value() >= 0)    spinbox->setPrefix("+");
}

