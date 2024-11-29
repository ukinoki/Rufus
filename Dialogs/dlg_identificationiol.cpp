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
    int spacing = 5;

    //! FABRICANT - MODELE
    QHBoxLayout *choixIOLLay    = new QHBoxLayout();

    if (m_manufacturersmodel != Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);
    foreach (Manufacturer *man, *Datas::I()->manufacturers->manufacturers())
        if (man->isactif()) {
            QList<QStandardItem *> items;
            UpStandardItem *itemman = new UpStandardItem(man->nom(), man);
            UpStandardItem *itemid = new UpStandardItem(QString::number(man->id()), man);
            items << itemman << itemid;
            m_manufacturersmodel->appendRow(items);
        }
    m_manufacturersmodel->sort(0);

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

    UpLabel* lblIOL             = new UpLabel;
    lblIOL                      ->setText(tr("Modèle"));
    wdg_nomiolline              = new UpLineEdit();
    wdg_nomiolline              ->setFixedSize(QSize(250,28));
    wdg_nomiolline              ->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9.-/ ]*")));
    wdg_nomiolline              ->setMaxLength(40);
    choixIOLLay                 ->addWidget(lblManufacturerIOL);
    choixIOLLay                 ->addWidget(wdg_manufacturercombo);
    choixIOLLay                 ->addWidget(lblIOL);
    choixIOLLay                 ->addWidget(wdg_nomiolline);
    choixIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay                 ->setSpacing(spacing);
    choixIOLLay                 ->setContentsMargins(0,0,0,0);

    //! edof multifocal precharge jaune
    QHBoxLayout *edofmfocLay    = new QHBoxLayout();
    wdg_addinterlbl             ->setText("       Add.  inter");
    wdg_addnearlbl              ->setText(tr("près"));
    wdg_addinterspin            = new UpDoubleSpinBox();
    wdg_addnearspin             = new UpDoubleSpinBox();
    wdg_addinterspin            ->setFixedSize(QSize(55,28));
    wdg_addnearspin             ->setFixedSize(QSize(55,28));
    wdg_addinterspin            ->setRange(0.0, 2.5);
    wdg_addinterspin            ->setSingleStep(0.25);
    wdg_addnearspin             ->setRange(0.0, 4.5);
    wdg_addnearspin             ->setSingleStep(0.25);
    wdg_prechargechk            ->setFixedHeight(35);
    wdg_jaunechk                ->setFixedHeight(35);
    wdg_toricchk                ->setFixedHeight(35);

    edofmfocLay                 ->addWidget(wdg_prechargechk);
    edofmfocLay                 ->addWidget(wdg_toricchk);
    edofmfocLay                 ->addWidget(wdg_jaunechk);
    edofmfocLay                 ->addWidget(wdg_edofchk);
    edofmfocLay                 ->addWidget(wdg_multifocalchk);
    edofmfocLay                 ->addWidget(wdg_addinterlbl);
    edofmfocLay                 ->addWidget(wdg_addinterspin);
    edofmfocLay                 ->addWidget(wdg_addnearlbl);
    edofmfocLay                 ->addWidget(wdg_addnearspin);
    edofmfocLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    edofmfocLay                 ->setSpacing(spacing*2);
    edofmfocLay                 ->setContentsMargins(0,0,0,0);

    //! Materiau - Image recopie
    QStringList listopticmaterials, listhapticmaterials, listhydrofily, listtypes;
    for (auto it = Datas::I()->iols->iols()->begin(); it != Datas::I()->iols->iols()->end(); ++it)
    {
        IOL* iol = Datas::I()->iols->getById(it.key());
        if (iol)
        {
            if (!listopticmaterials.contains(iol->opticalmaterial()) && iol->opticalmaterial() != "")
                listopticmaterials << iol->opticalmaterial();
            if (!listhapticmaterials.contains(iol->hapticalmaterial()) && iol->hapticalmaterial() != "")
                listhapticmaterials << iol->hapticalmaterial();
            if (!listhydrofily.contains(iol->hydrofily()) && iol->hydrofily() != "")
                listhydrofily << iol->hydrofily();
        }

    }
    listtypes << IOL_CP << IOL_CA << IOL_ADDON << IOL_IRIEN << IOL_CAREFRACTIF << IOL_AUTRE;
    listhydrofily << "";

    QHBoxLayout *TypeLay        = new QHBoxLayout();
    UpLabel* Typelbl            = new UpLabel;
    Typelbl                     ->setText(tr("Type"));
    TypeLay                     ->insertWidget(0,wdg_typebox);
    TypeLay                     ->insertWidget(0,Typelbl);
    wdg_typebox                 ->setEditable(false);
    wdg_typebox                 ->setFocusPolicy(Qt::StrongFocus);
    wdg_typebox                 ->addItems(listtypes);
    wdg_typebox                 ->setFixedWidth(180);
    wdg_typebox                 ->setCurrentIndex(-1);
    wdg_imgIOL                  ->setFixedSize(160,180);
    wdg_imgIOL                  ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_imgIOL                  ->setImmediateToolTip(tr("Clic droit pour modifier l'image de l'implant"));
    wdg_imgIOL                  ->setAlignment(Qt::AlignCenter);

    QHBoxLayout *MateriaulblLay = new QHBoxLayout();
    UpLabel* Materiaulbl        = new UpLabel;
    Materiaulbl                 ->setText(tr("Materiau"));
    Materiaulbl                 ->setFixedWidth(115);
    UpLabel* Hydrofilylbl       = new UpLabel;
    Hydrofilylbl                ->setText(tr("Hydrophilie"));
    Hydrofilylbl                ->setFixedWidth(115);
    MateriaulblLay              ->insertWidget(0,Hydrofilylbl);
    MateriaulblLay              ->insertWidget(0,Materiaulbl);
    MateriaulblLay              ->setContentsMargins(0,0,0,0);
    MateriaulblLay              ->setSpacing(spacing);

    QHBoxLayout *MateriauboxLay = new QHBoxLayout();
    wdg_materiaubox             ->setEditable(false);
    wdg_materiaubox             ->setFixedWidth(115);
    wdg_materiaubox             ->addItems(listopticmaterials);
    wdg_materiaubox             ->setCurrentIndex(-1);
    wdg_hydrofilybox            ->setEditable(false);
    wdg_hydrofilybox            ->setFixedWidth(115);
    wdg_hydrofilybox            ->addItems(listhydrofily);
    wdg_hydrofilybox            ->setCurrentIndex(-1);
    MateriauboxLay              ->insertWidget(0,wdg_hydrofilybox);
    MateriauboxLay              ->insertWidget(0,wdg_materiaubox);
    MateriauboxLay              ->setContentsMargins(0,0,0,0);
    MateriauboxLay              ->setSpacing(spacing);

    wdg_HapticMateriaulbl       ->setText(tr("Haptique"));
    wdg_hapticmateriaubox       ->setEditable(true);

    wdg_hapticmateriaubox       ->lineEdit()->setMaxLength(45);
    wdg_hapticmateriaubox       ->setFixedWidth(180);
    wdg_hapticmateriaubox       ->addItems(listhapticmaterials);
    wdg_hapticmateriaubox       ->setCurrentIndex(-1);

    lay_materiau                ->insertWidget(0,wdg_hapticmateriaubox);
    lay_materiau                ->insertWidget(0,wdg_HapticMateriaulbl);
    lay_materiau                ->insertLayout(0,MateriauboxLay);
    lay_materiau                ->insertLayout(0,MateriaulblLay);
    lay_materiau                ->insertWidget(0,wdg_singlepiecechk);
    lay_materiau                ->insertLayout(0,TypeLay);
    lay_materiau                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    lay_materiau                ->setContentsMargins(0,0,0,0);

    QHBoxLayout *MateriauImgLay = new QHBoxLayout();
    MateriauImgLay              ->addLayout(lay_materiau);
    MateriauImgLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    MateriauImgLay              ->addWidget(wdg_imgIOL);
    MateriauImgLay              ->setContentsMargins(0,0,0,0);
    if (m_mode == Modification)
    {
        wdg_recopiebutton           = new UpPushButton ("Recopier l'IOL");
        wdg_recopiebutton           ->setVisible(false);
        lay_materiau                ->addWidget(wdg_recopiebutton);
    }

    /*! vertical2 widget */
    QWidget * wdg_csteremarque  = new QWidget;
    QVBoxLayout *VglobalLay2    = new QVBoxLayout();

    /*! début constantes */
    QWidget *wdg_cstes          = new QWidget;
    QVBoxLayout *cstesLay       = new QVBoxLayout;

    //! Groupbox nominal, Ulib & optimized
    QHBoxLayout *Typecst_Lay    = new QHBoxLayout();
    QGroupBox *typcstbx         = new QGroupBox("");
    QHBoxLayout *cst_Lay        = new QHBoxLayout();
    cst_Lay                     ->addWidget(wdg_nominalrb);
    cst_Lay                     ->addWidget(wdg_ulibrb);
    cst_Lay                     ->addWidget(wdg_optimizedrb);
    typcstbx                    ->setLayout(cst_Lay);
    wdg_modifButton             ->setIcon(Icons::icEditer());
    wdg_modifButton             ->setIconSize(QSize(24,24));
    int szicon  = 20;
    int geo     = szicon + 4;
    wdg_modifButton             ->setFlat(true);
    wdg_modifButton             ->setIconSize(QSize(szicon, szicon));
    wdg_modifButton             ->setFixedSize(geo,geo);
    wdg_modifButton             ->setStyleSheet(Utils::nullmargingstyle());
    Typecst_Lay                 ->addWidget(typcstbx);
    Typecst_Lay                 ->addWidget(wdg_modifButton);

    //! Constante A Echo
    QHBoxLayout *csteAEcho_Lay  = new QHBoxLayout();

        //! nominal
    QHBoxLayout *NcsteAEcho_Lay = new QHBoxLayout();
    UpLabel* NlblcsteAIOL       = new UpLabel;
    NlblcsteAIOL                ->setText(tr("Cste A Echo"));
    NlblcsteAIOL                ->setFixedSize(QSize(100,28));
    wdg_Aecholine               ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Aecholine));
    wdg_Aecholine               ->setFixedSize(QSize(50,28));
    NcsteAEcho_Lay              ->addWidget(NlblcsteAIOL);
    NcsteAEcho_Lay              ->addWidget(wdg_Aecholine);
    NcsteAEcho_Lay              ->setContentsMargins(0,0,0,0);
    wdg_nominalCsteAEcho        ->setLayout(NcsteAEcho_Lay);

        //! ulib
    QHBoxLayout *UcsteAEcho_Lay = new QHBoxLayout();
    UpLabel* UlblcsteAIOL       = new UpLabel;
    UlblcsteAIOL                ->setText("<font color=\"red\"><b>" + tr("Cste A Echo") + "</b></font>");
    UlblcsteAIOL                ->setFixedSize(QSize(100,28));
    wdg_UAecholine              ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_UAecholine));
    wdg_UAecholine              ->setFixedSize(QSize(50,28));
    UcsteAEcho_Lay              ->addWidget(UlblcsteAIOL);
    UcsteAEcho_Lay              ->addWidget(wdg_UAecholine);
    UcsteAEcho_Lay              ->setContentsMargins(0,0,0,0);
    wdg_ulibCsteAEcho           ->setLayout(UcsteAEcho_Lay);

    csteAEcho_Lay               ->addWidget(wdg_ulibCsteAEcho);
    csteAEcho_Lay               ->addWidget(wdg_nominalCsteAEcho);
    csteAEcho_Lay               ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteAEcho_Lay               ->setContentsMargins(0,0,0,0);
    csteAEcho_Lay               ->setStretch(0,0);
    csteAEcho_Lay               ->setStretch(1,0);
    csteAEcho_Lay               ->setStretch(2,100);
    wdg_CsteAEcho               ->setLayout(csteAEcho_Lay);


    //! Cste A Optique + Holladay1SF
    QHBoxLayout *CsteAOPT_Holladay_Lay  = new QHBoxLayout();

        //! nominal
    QHBoxLayout *NCsteAOPT_Holladay_Lay = new QHBoxLayout();
    UpLabel* NlblAOptIOL        = new UpLabel;
    NlblAOptIOL                 ->setText(tr("Cste A Optique"));
    NlblAOptIOL                 ->setFixedSize(QSize(100,28));
    wdg_Aoptline                ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Aoptline));
    wdg_Aoptline                ->setFixedSize(QSize(50,28));
    UpLabel* lblHolIOL          = new UpLabel;
    lblHolIOL                   ->setText("Holladay1sf");
    QDoubleValidator *HOL_val   = new QDoubleValidator(0,8,2, this);
    wdg_holladayline            ->setValidator(HOL_val);
    wdg_holladayline            ->setFixedSize(QSize(50,28));
    NCsteAOPT_Holladay_Lay      ->addWidget(NlblAOptIOL);
    NCsteAOPT_Holladay_Lay      ->addWidget(wdg_Aoptline);
    NCsteAOPT_Holladay_Lay      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    NCsteAOPT_Holladay_Lay      ->addWidget(lblHolIOL);
    NCsteAOPT_Holladay_Lay      ->addWidget(wdg_holladayline);
    NCsteAOPT_Holladay_Lay      ->setSpacing(spacing);
    NCsteAOPT_Holladay_Lay      ->setContentsMargins(0,0,0,0);
    NCsteAOPT_Holladay_Lay      ->setStretch(2,100);
    wdg_nominalCsteA            ->setLayout(NCsteAOPT_Holladay_Lay);


        //! ULIB
    QHBoxLayout *UCsteAOPT_Holladay_Lay = new QHBoxLayout();
    UpLabel* UlblAOptIOL        = new UpLabel;
    UlblAOptIOL                 ->setText("<font color=\"red\"><b>" + tr("Cste A Optique") + "</b></font>");
    UlblAOptIOL                 ->setFixedSize(QSize(100,28));
    wdg_UAoptline               ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Aoptline));
    wdg_UAoptline               ->setFixedSize(QSize(50,28));
    UpLabel* UlblHolIOL         = new UpLabel;
    UlblHolIOL                  ->setText("<font color=\"red\"><b>Holladay1sf</b></font>");
    wdg_Uholladayline           ->setValidator(HOL_val);
    wdg_Uholladayline           ->setFixedSize(QSize(50,28));
    UCsteAOPT_Holladay_Lay      ->addWidget(UlblAOptIOL);
    UCsteAOPT_Holladay_Lay      ->addWidget(wdg_UAoptline);
    UCsteAOPT_Holladay_Lay      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    UCsteAOPT_Holladay_Lay      ->addWidget(UlblHolIOL);
    UCsteAOPT_Holladay_Lay      ->addWidget(wdg_Uholladayline);
    UCsteAOPT_Holladay_Lay      ->setSpacing(spacing);
    UCsteAOPT_Holladay_Lay      ->setContentsMargins(0,0,0,0);
    UCsteAOPT_Holladay_Lay      ->setStretch(2,100);
    wdg_ulibCsteA               ->setLayout(UCsteAOPT_Holladay_Lay);

        //! optimized
    QHBoxLayout *OCsteAOPT_Holladay_Lay = new QHBoxLayout();
    UpLabel* OlblAOptIOL        = new UpLabel;
    OlblAOptIOL                 ->setText("<font color=\"blue\"><b>" + tr("Cste A Optique") + "</b></font>");
    OlblAOptIOL                 ->setFixedSize(QSize(110,28));
    wdg_OAoptline               ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Aoptline));
    wdg_OAoptline               ->setFixedSize(QSize(50,28));
    UpLabel* OlblHolIOL         = new UpLabel;
    OlblHolIOL                  ->setText("<font color=\"blue\"><b>Holladay1sf</b></font>");
    wdg_Oholladayline           ->setValidator(HOL_val);
    wdg_Oholladayline           ->setFixedSize(QSize(50,28));
    OCsteAOPT_Holladay_Lay      ->addWidget(OlblAOptIOL);
    OCsteAOPT_Holladay_Lay      ->addWidget(wdg_OAoptline);
    OCsteAOPT_Holladay_Lay      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    OCsteAOPT_Holladay_Lay      ->addWidget(OlblHolIOL);
    OCsteAOPT_Holladay_Lay      ->addWidget(wdg_Oholladayline);
    OCsteAOPT_Holladay_Lay      ->setSpacing(spacing);
    OCsteAOPT_Holladay_Lay      ->setContentsMargins(0,0,0,0);
    OCsteAOPT_Holladay_Lay      ->setStretch(2,100);
    wdg_optimizedCsteA          ->setLayout(OCsteAOPT_Holladay_Lay);

    CsteAOPT_Holladay_Lay       ->addWidget(wdg_nominalCsteA);
    CsteAOPT_Holladay_Lay       ->addWidget(wdg_ulibCsteA);
    CsteAOPT_Holladay_Lay       ->addWidget(wdg_optimizedCsteA);

    //! Haigis
    QHBoxLayout *Haigis_Lay  = new QHBoxLayout();

        //! nominal
    QHBoxLayout *NHaigis_Lay    = new QHBoxLayout();
    UpLabel* NHaigisIOLlbl      = new UpLabel;
    NHaigisIOLlbl               ->setText("Haigis");
    UpLabel* NHaigisalbl        = new UpLabel;
    NHaigisalbl                 ->setText("a0");
    NHaigisalbl                 ->setFixedSize(QSize(18,28));
    UpLabel* NHaigisblbl        = new UpLabel;
    NHaigisblbl                 ->setText("a1");
    NHaigisblbl                 ->setFixedSize(QSize(18,28));
    UpLabel* NHaigisclbl        = new UpLabel;
    NHaigisclbl                 ->setText(tr("a2"));
    NHaigisclbl                 ->setFixedSize(QSize(18,28));
    wdg_haigisaline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_haigisaline));
    wdg_haigisaline             ->setFixedSize(QSize(60,28));
    wdg_haigisbline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_haigisbline));
    wdg_haigisbline             ->setFixedSize(QSize(60,28));
    wdg_haigiscline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_haigiscline));
    wdg_haigiscline             ->setFixedSize(QSize(60,28));
    NHaigis_Lay                 ->addWidget(NHaigisIOLlbl);
    NHaigis_Lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    NHaigis_Lay                 ->addWidget(NHaigisalbl);
    NHaigis_Lay                 ->addWidget(wdg_haigisaline);
    NHaigis_Lay                 ->addWidget(NHaigisblbl);
    NHaigis_Lay                 ->addWidget(wdg_haigisbline);
    NHaigis_Lay                 ->addWidget(NHaigisclbl);
    NHaigis_Lay                 ->addWidget(wdg_haigiscline);
    NHaigis_Lay                 ->setSpacing(spacing);
    NHaigis_Lay                 ->setContentsMargins(0,0,0,0);
    wdg_nominalhaigis           ->setLayout(NHaigis_Lay);

        //! ulib
    QHBoxLayout *UHaigis_Lay     = new QHBoxLayout();
    UpLabel* UHaigisIOLlbl       = new UpLabel;
    UHaigisIOLlbl                ->setText("<font color=\"red\"><b>Haigis</b></font>");
    UpLabel* UHaigisalbl         = new UpLabel;
    UHaigisalbl                  ->setText("<font color=\"red\"><b>a0</b></font>");
    UHaigisalbl                  ->setFixedSize(QSize(18,28));
    UpLabel* UHaigisblbl         = new UpLabel;
    UHaigisblbl                  ->setText("<font color=\"red\"><b>a1</b></font>");
    UHaigisblbl                  ->setFixedSize(QSize(18,28));
    UpLabel* UHaigisclbl         = new UpLabel;
    UHaigisclbl                  ->setText(tr("<font color=\"red\"><b>a2</b></font>"));
    UHaigisclbl                  ->setFixedSize(QSize(18,28));
    wdg_Uhaigisaline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_Uhaigisaline));
    wdg_Uhaigisaline             ->setFixedSize(QSize(60,28));
    wdg_Uhaigisbline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_Uhaigisbline));
    wdg_Uhaigisbline             ->setFixedSize(QSize(60,28));
    wdg_Uhaigiscline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_Uhaigiscline));
    wdg_Uhaigiscline             ->setFixedSize(QSize(60,28));
    UHaigis_Lay                  ->addWidget(UHaigisIOLlbl);
    UHaigis_Lay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    UHaigis_Lay                  ->addWidget(UHaigisalbl);
    UHaigis_Lay                  ->addWidget(wdg_Uhaigisaline);
    UHaigis_Lay                  ->addWidget(UHaigisblbl);
    UHaigis_Lay                  ->addWidget(wdg_Uhaigisbline);
    UHaigis_Lay                  ->addWidget(UHaigisclbl);
    UHaigis_Lay                  ->addWidget(wdg_Uhaigiscline);
    UHaigis_Lay                  ->setSpacing(spacing);
    UHaigis_Lay                  ->setContentsMargins(0,0,0,0);
    wdg_ulibhaigis               ->setLayout(UHaigis_Lay);

        //! optimized
    QHBoxLayout *OHaigis_Lay     = new QHBoxLayout();
    UpLabel* OHaigisIOLlbl       = new UpLabel;
    OHaigisIOLlbl                ->setText("<font color=\"blue\"><b>Haigis</b></font>");
    UpLabel* OHaigisalbl         = new UpLabel;
    OHaigisalbl                  ->setText("<font color=\"blue\"><b>aO</b></font>");
    OHaigisalbl                  ->setFixedSize(QSize(18,28));
    UpLabel* OHaigisblbl         = new UpLabel;
    OHaigisblbl                  ->setText("<font color=\"blue\"><b>a1</b></font>");
    OHaigisblbl                  ->setFixedSize(QSize(18,28));
    UpLabel* OHaigisclbl         = new UpLabel;
    OHaigisclbl                  ->setText("<font color=\"blue\"><b>a2</b></font>");
    OHaigisclbl                  ->setFixedSize(QSize(18,28));
    wdg_Ohaigisaline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_Ohaigisaline));
    wdg_Ohaigisaline             ->setFixedSize(QSize(60,28));
    wdg_Ohaigisbline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_Ohaigisbline));
    wdg_Ohaigisbline             ->setFixedSize(QSize(60,28));
    wdg_Ohaigiscline             ->setValidator(new QRegularExpressionValidator(rgx_haigis, wdg_Ohaigiscline));
    wdg_Ohaigiscline             ->setFixedSize(QSize(60,28));
    OHaigis_Lay                  ->addWidget(OHaigisIOLlbl);
    OHaigis_Lay                  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    OHaigis_Lay                  ->addWidget(OHaigisalbl);
    OHaigis_Lay                  ->addWidget(wdg_Ohaigisaline);
    OHaigis_Lay                  ->addWidget(OHaigisblbl);
    OHaigis_Lay                  ->addWidget(wdg_Ohaigisbline);
    OHaigis_Lay                  ->addWidget(OHaigisclbl);
    OHaigis_Lay                  ->addWidget(wdg_Ohaigiscline);
    OHaigis_Lay                  ->setSpacing(spacing);
    OHaigis_Lay                  ->setContentsMargins(0,0,0,0);
    wdg_optimizedhaigis          ->setLayout(OHaigis_Lay);

    Haigis_Lay                  ->addWidget(wdg_nominalhaigis);
    Haigis_Lay                  ->addWidget(wdg_ulibhaigis);
    Haigis_Lay                  ->addWidget(wdg_optimizedhaigis);

    //! Barrett
    QHBoxLayout *barrett_Lay  = new QHBoxLayout();

        //! nominal
    QHBoxLayout *Nbarrett_Lay   = new QHBoxLayout();
    UpLabel* Nlblbarrett_IOL    = new UpLabel;
    Nlblbarrett_IOL             ->setText("Barrett");
    Nlblbarrett_IOL             ->setFixedSize(QSize(110,28));
    UpLabel* NlbDF_IOL          = new UpLabel;
    NlbDF_IOL                   ->setText("DF");
    NlbDF_IOL                   ->setFixedSize(QSize(18,28));
    UpLabel* NlbLF_IOL          = new UpLabel;
    NlbLF_IOL                   ->setText("LF");
    NlbLF_IOL                   ->setFixedSize(QSize(18,28));
    wdg_barrettDFline           ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_barrettDFline));
    wdg_barrettDFline           ->setFixedSize(QSize(60,28));
    wdg_barrettLFline           ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_barrettLFline));
    wdg_barrettLFline           ->setFixedSize(QSize(60,28));
    Nbarrett_Lay                ->addWidget(Nlblbarrett_IOL);
    Nbarrett_Lay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    Nbarrett_Lay                ->addWidget(NlbDF_IOL);
    Nbarrett_Lay                ->addWidget(wdg_barrettDFline);
    Nbarrett_Lay                ->addWidget(NlbLF_IOL);
    Nbarrett_Lay                ->addWidget(wdg_barrettLFline);
    Nbarrett_Lay                ->setSpacing(spacing);
    Nbarrett_Lay                ->setContentsMargins(0,0,0,0);
    wdg_nominalbarrett          ->setLayout(Nbarrett_Lay);

        //! Ulib
    QHBoxLayout *Ubarrett_Lay   = new QHBoxLayout();
    UpLabel* Ulblbarrett_IOL    = new UpLabel;
    Ulblbarrett_IOL             ->setText(tr("<font color=\"red\"><b>Barrett</b></font>"));
    Ulblbarrett_IOL             ->setFixedSize(QSize(110,28));
    UpLabel* UlbDF_IOL          = new UpLabel;
    UlbDF_IOL                   ->setText("<font color=\"red\"><b>DF</b></font>");
    UlbDF_IOL                   ->setFixedSize(QSize(18,28));
    UpLabel* UlbLF_IOL          = new UpLabel;
    UlbLF_IOL                   ->setText("<font color=\"red\"><b>LF</b></font>");
    UlbLF_IOL                   ->setFixedSize(QSize(18,28));
    wdg_UbarrettDFline          ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_UbarrettDFline));
    wdg_UbarrettDFline          ->setFixedSize(QSize(60,28));
    wdg_UbarrettLFline          ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_UbarrettLFline));
    wdg_UbarrettLFline          ->setFixedSize(QSize(60,28));
    Ubarrett_Lay                ->addWidget(Ulblbarrett_IOL);
    Ubarrett_Lay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    Ubarrett_Lay                ->addWidget(UlbDF_IOL);
    Ubarrett_Lay                ->addWidget(wdg_UbarrettDFline);
    Ubarrett_Lay                ->addWidget(UlbLF_IOL);
    Ubarrett_Lay                ->addWidget(wdg_UbarrettLFline);
    Ubarrett_Lay                ->setSpacing(spacing);
    Ubarrett_Lay                ->setContentsMargins(0,0,0,0);
    wdg_ulibbarrett             ->setLayout(Ubarrett_Lay);

    barrett_Lay                 ->addWidget(wdg_nominalbarrett);
    barrett_Lay                 ->addWidget(wdg_ulibbarrett);

    //! HofferQ Olsen
    QHBoxLayout *HofOls_Lay  = new QHBoxLayout();

        //! nominal
    QHBoxLayout *NHofOls_Lay    = new QHBoxLayout();
    UpLabel* NlbHofferQ_IOL     = new UpLabel;
    NlbHofferQ_IOL              ->setText("HofferQ");
    NlbHofferQ_IOL              ->setFixedSize(QSize(60,28));
    wdg_hofferQline             ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_hofferQline));
    wdg_hofferQline             ->setFixedSize(QSize(50,28));
    UpLabel* NlbOlsen_IOL       = new UpLabel;
    NlbOlsen_IOL                ->setText("Olsen");
    NlbOlsen_IOL                ->setFixedSize(QSize(50,28));
    wdg_olsenline               ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_olsenline));
    wdg_olsenline               ->setFixedSize(QSize(50,28));
    NHofOls_Lay                 ->addWidget(NlbHofferQ_IOL);
    NHofOls_Lay                 ->addWidget(wdg_hofferQline);
    NHofOls_Lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    NHofOls_Lay                 ->addWidget(NlbOlsen_IOL);
    NHofOls_Lay                 ->addWidget(wdg_olsenline);
    NHofOls_Lay                 ->setSpacing(spacing);
    NHofOls_Lay                 ->setContentsMargins(0,0,0,0);
    wdg_nominalhofferq          ->setLayout(NHofOls_Lay);

        //! ulib
    QHBoxLayout *UHofOls_Lay    = new QHBoxLayout();
    UpLabel* UlbHofferQ_IOL     = new UpLabel;
    UlbHofferQ_IOL              ->setText("<font color=\"red\"><b>HofferQ</b></font>");
    UlbHofferQ_IOL              ->setFixedSize(QSize(60,28));
    wdg_UhofferQline            ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_UhofferQline));
    wdg_UhofferQline            ->setFixedSize(QSize(50,28));
    UpLabel* UlbOlsen_IOL       = new UpLabel;
    UlbOlsen_IOL                ->setText("<font color=\"red\"><b>Olsen</b></font>");
    UlbOlsen_IOL                ->setFixedSize(QSize(50,28));
    wdg_Uolsenline              ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_Uolsenline));
    wdg_Uolsenline              ->setFixedSize(QSize(50,28));
    UHofOls_Lay                 ->addWidget(UlbHofferQ_IOL);
    UHofOls_Lay                 ->addWidget(wdg_UhofferQline);
    UHofOls_Lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    UHofOls_Lay                 ->addWidget(UlbOlsen_IOL);
    UHofOls_Lay                 ->addWidget(wdg_Uolsenline);
    UHofOls_Lay                 ->setSpacing(spacing);
    UHofOls_Lay                 ->setContentsMargins(0,0,0,0);
    wdg_ulibhofferq             ->setLayout(UHofOls_Lay);

        //! optimized
    QHBoxLayout *OHofOls_Lay    = new QHBoxLayout();
    UpLabel* OlbHofferQ_IOL     = new UpLabel;
    OlbHofferQ_IOL              ->setText("<font color=\"blue\"><b>HofferQ</b></font>");
    OlbHofferQ_IOL              ->setFixedSize(QSize(60,28));
    wdg_OhofferQline            ->setValidator(new QRegularExpressionValidator(rgx_csteA, wdg_OhofferQline));
    wdg_OhofferQline            ->setFixedSize(QSize(50,28));
    UpLabel* OlbOlsen_IOL       = new UpLabel;
    OlbOlsen_IOL                ->setText("<font color=\"blue\"><b>Olsen</b></font>");
    OlbOlsen_IOL                ->setFixedSize(QSize(60,28));
    OHofOls_Lay                 ->addWidget(OlbHofferQ_IOL);
    OHofOls_Lay                 ->addWidget(wdg_OhofferQline);
    OHofOls_Lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    OHofOls_Lay                 ->setSpacing(spacing);
    OHofOls_Lay                 ->setContentsMargins(0,0,0,0);
    wdg_optimizedhofferq        ->setLayout(OHofOls_Lay);

    HofOls_Lay                  ->addWidget(wdg_nominalhofferq);
    HofOls_Lay                  ->addWidget(wdg_ulibhofferq);
    HofOls_Lay                  ->addWidget(wdg_optimizedhofferq);

    //! Nb cases
    QHBoxLayout *NbCases_Lay  = new QHBoxLayout();

        //! ulib
    QHBoxLayout *UNbCases_Lay   = new QHBoxLayout();
    UpLabel* UNbCasesQ_IOL      = new UpLabel;
    UNbCasesQ_IOL               ->setText("<font color=\"red\"><b>nb cas</b></font>");
    UNbCasesQ_IOL               ->setFixedSize(QSize(60,28));
    wdg_UNbCasesline            ->setValidator(new QRegularExpressionValidator(rgx_int, wdg_UhofferQline));
    wdg_UNbCasesline            ->setFixedSize(QSize(50,28));
    UNbCases_Lay                ->addWidget(UNbCasesQ_IOL);
    UNbCases_Lay                ->addWidget(wdg_UNbCasesline);
    UNbCases_Lay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    UNbCases_Lay                ->setSpacing(spacing);
    UNbCases_Lay                ->setContentsMargins(0,0,0,0);
    wdg_ulibnbcases             ->setLayout(UNbCases_Lay);

       //! optimized
    QHBoxLayout *ONbCases_Lay   = new QHBoxLayout();
    UpLabel* ONbCases_IOL       = new UpLabel;
    ONbCases_IOL                ->setText("<font color=\"blue\"><b>nb cas</b></font>");
    ONbCases_IOL                ->setFixedSize(QSize(60,28));
    wdg_ONbCasesline            ->setValidator(new QRegularExpressionValidator(rgx_int, wdg_OhofferQline));
    wdg_ONbCasesline            ->setFixedSize(QSize(50,28));
    ONbCases_Lay                ->addWidget(ONbCases_IOL);
    ONbCases_Lay                ->addWidget(wdg_ONbCasesline);
    ONbCases_Lay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    ONbCases_Lay                ->setSpacing(spacing);
    ONbCases_Lay                ->setContentsMargins(0,0,0,0);
    wdg_optimizednbcases        ->setLayout(ONbCases_Lay);

    NbCases_Lay                 ->addWidget(wdg_ulibnbcases);
    NbCases_Lay                 ->addWidget(wdg_optimizednbcases);

    cstesLay   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    cstesLay   ->insertLayout(0, HofOls_Lay);
    cstesLay   ->insertLayout(0, barrett_Lay);
    cstesLay   ->insertLayout(0, Haigis_Lay);
    cstesLay   ->insertLayout(0, CsteAOPT_Holladay_Lay);
    cstesLay   ->insertWidget(0, wdg_CsteAEcho);
    cstesLay   ->insertLayout(0, Typecst_Lay);
    cstesLay   ->setContentsMargins(0,0,0,0);
    cstesLay   ->setSpacing(spacing);
    cstesLay   ->setStretch(cstesLay->count()-1,1000);
    wdg_cstes  ->setLayout(cstesLay);
    wdg_cstes  ->setFixedSize(350,210);

    /*! fin constantes */




    //! Remarque
    QWidget *wdg_remarque       = new QWidget;
    QVBoxLayout *remarqueLay    = new QVBoxLayout;
    QHBoxLayout *remarqueHLay   = new QHBoxLayout();
    UpLabel* Remarquelbl        = new UpLabel;
    Remarquelbl                 ->setFixedSize(70,28);
    Remarquelbl                 ->setText(tr("Remarque"));
    remarqueHLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    remarqueHLay                ->insertWidget(0,Remarquelbl);
    remarqueHLay                ->setContentsMargins(0,0,0,0);
    wdg_remarquetxt             ->setFixedHeight(120);
    remarqueLay                 ->insertWidget(0,wdg_remarquetxt);
    remarqueLay                 ->insertLayout(0,remarqueHLay);
    remarqueLay                 ->setContentsMargins(0,0,0,0);
    wdg_remarque                ->setLayout(remarqueLay);
    wdg_remarque                ->setFixedHeight(160);

    VglobalLay2                 ->insertWidget(0,wdg_remarque);
    VglobalLay2                 ->insertLayout(0, NbCases_Lay);
    VglobalLay2                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    VglobalLay2                 ->insertWidget(0, wdg_cstes);
    VglobalLay2                 ->setContentsMargins(0,0,0,0);
    VglobalLay2                 ->setStretch(0,1);
    VglobalLay2                 ->setStretch(1,2000);

    wdg_csteremarque            ->setLayout(VglobalLay2);
    wdg_csteremarque            ->setFixedHeight(420);

    //! Diametres optique et ht
    QHBoxLayout *diametresLay   = new QHBoxLayout();
    UpLabel* lbldiametres       = new UpLabel;
    lbldiametres                ->setText(tr("Diamètre (mm)"));
    UpLabel* lbldiaht           = new UpLabel;
    lbldiaht                    ->setText(tr("Hors-tout"));
    UpLabel* lbldiaopt          = new UpLabel;
    lbldiaopt                   ->setText(tr("Optique"));
    wdg_diaht                   ->setValidator(new QRegularExpressionValidator(rgx_diaht, wdg_diaht));
    wdg_diaht                   ->setFixedSize(QSize(50,28));
    wdg_diaoptique              ->setValidator(new QRegularExpressionValidator(rgx_diaoptique, wdg_diaoptique));
    wdg_diaoptique              ->setFixedSize(QSize(50,28));
    diametresLay                ->addWidget(lbldiametres);
    diametresLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    diametresLay                ->addWidget(lbldiaht);
    diametresLay                ->addWidget(wdg_diaht);
    diametresLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    diametresLay                ->addWidget(lbldiaopt);
    diametresLay                ->addWidget(wdg_diaoptique);
    diametresLay                ->setSpacing(spacing);
    diametresLay                ->setContentsMargins(0,0,0,0);

    //! Diametres injecteur
    QHBoxLayout *injecteurLay   = new QHBoxLayout();
    UpLabel* lbldiainjecteur    = new UpLabel;
    lbldiainjecteur             ->setText(tr("Injecteur (mm)"));
    wdg_diainjecteur            ->setValidator(new QRegularExpressionValidator(rgx_diainjecteur, wdg_diaoptique));
    wdg_diainjecteur            ->setFixedSize(QSize(50,28));
    injecteurLay                ->addWidget(lbldiainjecteur);
    injecteurLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    injecteurLay                ->addWidget(wdg_diainjecteur);
    injecteurLay                ->setSpacing(spacing);
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
    QString maxpwr = IOL_PWRMAX;
    double max = maxpwr.toDouble();
    QString minpwr = IOL_PWRMIN;
    double min = minpwr.toDouble();
    wdg_puissancemaxspin        ->setRange(0.0, max);
    wdg_puissancemaxspin        ->setSingleStep(0.5);
    wdg_puissanceminspin        ->setRange(min, 15.0);
    wdg_puissanceminspin        ->setSingleStep(0.5);
    puissancesLay               ->addWidget(lplpuissances);
    puissancesLay               ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    puissancesLay               ->addWidget(lblpuissancemin);
    puissancesLay               ->addWidget(wdg_puissanceminspin);
    puissancesLay               ->addWidget(lblpuissancemax);
    puissancesLay               ->addWidget(wdg_puissancemaxspin);
    puissancesLay               ->setSpacing(spacing);
    puissancesLay               ->setContentsMargins(0,0,0,0);

    //! Plage cylindres
    QHBoxLayout *wdgcylindresLay  = new QHBoxLayout();
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
    wdgcylindresLay             ->addWidget(lblcylindremin);
    wdgcylindresLay             ->addWidget(wdg_cylindreminspin);
    wdgcylindresLay             ->addWidget(lblcylindremax);
    wdgcylindresLay             ->addWidget(wdg_cylindremaxspin);
    wdgcylindresLay             ->setContentsMargins(0,0,0,0);
    wdg_cylindres               ->setLayout(wdgcylindresLay);

    QHBoxLayout *cylindresLay   = new QHBoxLayout();
    wdg_toricchk2               ->setFixedHeight(35);
    cylindresLay                ->addWidget(wdg_toricchk2);
    cylindresLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    cylindresLay                ->addWidget(wdg_cylindres);
    cylindresLay                ->setSpacing(spacing);
    cylindresLay                ->setContentsMargins(0,0,0,0);



    if (m_mode == Modification)
    {
        AjouteWidgetLayButtons(wdg_toolbar, false);
        wdg_toolbar     ->setEnabled(m_listeidIOLs.size()>1);
    }
    wdg_inactifchk              = new UpCheckBox(tr("Discontinué"));
    AjouteWidgetLayButtons(wdg_inactifchk, false);

    QWidget *wdg_global         = new QWidget;
    QHBoxLayout *globalLay      = new QHBoxLayout();
    QVBoxLayout *VglobalLay1    = new QVBoxLayout();

    VglobalLay1   ->insertLayout(0, cylindresLay);
    VglobalLay1   ->insertLayout(0, puissancesLay);
    QLabel *line1 = new QLabel();
    line1         ->setFrameShape(QFrame::HLine);
    line1         ->setLineWidth(3);
    line1         ->setStyleSheet(m_stylesheet);
    VglobalLay1   ->insertWidget(0, line1);
    VglobalLay1   ->insertLayout(0, diametresLay);
    VglobalLay1   ->insertLayout(0, injecteurLay);
    QLabel *line2 = new QLabel();
    line2         ->setFrameShape(QFrame::HLine);
    line2         ->setLineWidth(3);
    line2         ->setStyleSheet(m_stylesheet);
    VglobalLay1   ->insertWidget(0, line2);
    VglobalLay1   ->insertLayout(0, MateriauImgLay);
    VglobalLay1   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    VglobalLay1   ->setContentsMargins(0,0,0,0);

    QLabel *line3 = new QLabel();
    line3         ->setFrameShape(QFrame::VLine);
    line3         ->setLineWidth(3);
    line3         ->setStyleSheet(m_stylesheet);

    globalLay      ->insertWidget(0, wdg_csteremarque);
    globalLay      ->insertWidget(0, line3);
    globalLay      ->insertLayout(0, VglobalLay1);
    globalLay      ->setSpacing(spacing);
    globalLay      ->setContentsMargins(0,0,0,0);

    wdg_global      ->setLayout(globalLay);
    wdg_global      ->setFixedSize(QSize(770,430));

    dlglayout()    ->insertWidget(0, wdg_global);

    QLabel *lineh = new QLabel();
    lineh         ->setFrameShape(QFrame::HLine);
    lineh         ->setLineWidth(3);
    lineh         ->setStyleSheet(m_stylesheet);
    dlglayout()   ->insertWidget(0, lineh);
    dlglayout()   ->insertLayout(0, edofmfocLay);

    QLabel *lineh1 = new QLabel();
    lineh1         ->setFrameShape(QFrame::HLine);
    lineh1         ->setLineWidth(3);
    lineh1         ->setStyleSheet(m_stylesheet);
    dlglayout()   ->insertWidget(0, lineh1);

    dlglayout()   ->insertLayout(0, choixIOLLay);
    dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);

    QList <QWidget*> ListTab;
    ListTab << wdg_manufacturercombo << wdg_nomiolline
            << wdg_prechargechk << wdg_toricchk << wdg_jaunechk << wdg_edofchk << wdg_multifocalchk << wdg_addinterspin << wdg_addnearspin
            << wdg_typebox << wdg_singlepiecechk << wdg_materiaubox << wdg_hydrofilybox << wdg_hapticmateriaubox
            << wdg_diainjecteur << wdg_diaht << wdg_diaoptique
            << wdg_puissanceminspin << wdg_puissancemaxspin
            << wdg_toricchk2 << wdg_cylindreminspin << wdg_cylindremaxspin
            << wdg_Aecholine << wdg_UAecholine << wdg_Aoptline << wdg_UAoptline << wdg_OAoptline << wdg_holladayline << wdg_Uholladayline << wdg_Oholladayline
            << wdg_haigisaline << wdg_haigisbline << wdg_haigiscline << wdg_Uhaigisaline << wdg_Uhaigisbline << wdg_Uhaigiscline << wdg_Ohaigisaline << wdg_Ohaigisbline << wdg_Ohaigiscline
            << wdg_barrettDFline << wdg_barrettLFline << wdg_UbarrettDFline << wdg_UbarrettLFline
            << wdg_hofferQline << wdg_olsenline << wdg_UhofferQline << wdg_Uolsenline << wdg_OhofferQline
            << wdg_UNbCasesline << wdg_ONbCasesline << wdg_inactifchk;

    for (int i = 0; i<ListTab.size()-1 ; i++ )
    {
        setTabOrder(ListTab.at(i), ListTab.at(i+1));
        ListTab.at(i)->installEventFilter(this);
    }

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonRecord | UpDialog::ButtonOK);

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

    foreach (QWidget *wdg, findChildren<QWidget*>())        //! ce micmac sert a créé une émission du signal uptoggled seulement si le checkbox est coché/décoché par l'utilisateur pas s'il est coché/décoché par le programme
    {
        UpCheckBox *chk = qobject_cast<UpCheckBox*>(wdg);
        if (chk)
        {
            chk->installEventFilter(this);
            chk->setFocusPolicy(Qt::StrongFocus);
        }
    }

    wdg_puissancemaxspin->installEventFilter(this);
    wdg_cylindreminspin ->installEventFilter(this);
    wdg_cylindremaxspin ->installEventFilter(this);
    wdg_puissanceminspin->installEventFilter(this);
    wdg_addinterspin    ->installEventFilter(this);
    wdg_addnearspin     ->installEventFilter(this);
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer et fermer"));
    CancelButton->setText(tr("Annuler"));
    RecordButton->setEnabled(false);
    RecordButton->setText(tr("Enregistrer"));
    RecordButton->setVisible(m_mode == Modification);
    setStageCount(1);
    wdg_prechargechk->setFocus();
    AfficheDatasIOL(m_currentIOL);
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
        else if (keyEvent->key()==Qt::Key_Space)
        {
            UpCheckBox *chk = qobject_cast<UpCheckBox*>(obj);
            if (chk)
                emit chk->uptoggled(!chk->isChecked());
        }
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
    connect (OKButton,             &QPushButton::clicked,                                   this,   [&] {
                                                                                                            if (EnregistreIOL())
                                                                                                                accept();
                                                                                                        });
    connect (RecordButton,         &QPushButton::clicked,                                   this,   &dlg_identificationIOL::EnregistreIOL);
    connect (wdg_manufacturercombo,QOverload<int>::of(&QComboBox::currentIndexChanged),     this,   [&](int id) {
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
    connect (wdg_holladayline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigisaline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigisbline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigiscline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_hofferQline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_barrettDFline,     &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_barrettLFline,     &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_olsenline,         &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);

    connect (wdg_UAoptline,         &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_UAecholine,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Uholladayline,     &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Uhaigisaline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Uhaigisbline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Uhaigiscline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_UhofferQline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_UbarrettDFline,    &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_UbarrettLFline,    &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Uolsenline,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_UNbCasesline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);

    connect (wdg_OAoptline,         &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Oholladayline,     &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Ohaigisaline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Ohaigisbline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Ohaigiscline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_OhofferQline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Uolsenline,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_ONbCasesline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);

    connect (wdg_materiaubox,       QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_hydrofilybox,      QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_hapticmateriaubox->lineEdit(),    &QLineEdit::textEdited,                  this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_hapticmateriaubox, QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_typebox,           QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_remarquetxt,       &UpTextEdit::textEdited,                                this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_diaoptique,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_diaht,             &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_diainjecteur,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_prechargechk,      &UpCheckBox::uptoggled,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_edofchk,           &UpCheckBox::uptoggled,                                 this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    wdg_addinterlbl         ->setVisible(a);
                                                                                                                    wdg_addnearlbl          ->setVisible(false);
                                                                                                                    wdg_addnearspin         ->setVisible(false);
                                                                                                                    wdg_addinterspin        ->setVisible(a);
                                                                                                                    wdg_multifocalchk       ->setChecked(false);
                                                                                                                });
    connect (wdg_toricchk,         &UpCheckBox::uptoggled,                                  this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    wdg_cylindres->setVisible(a);
                                                                                                                    wdg_toricchk2->setChecked(a);
                                                                                                                });
    connect (wdg_toricchk2,        &UpCheckBox::uptoggled,                                  this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    wdg_cylindres->setVisible(a);
                                                                                                                    wdg_toricchk->setChecked(a);
                                                                                                                });
    connect (wdg_singlepiecechk,   &UpCheckBox::uptoggled,                                  this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    wdg_hapticmateriaubox   ->setVisible(!a);
                                                                                                                    wdg_HapticMateriaulbl   ->setVisible(!a);
                                                                                                                });
    connect (wdg_jaunechk,         &UpCheckBox::uptoggled,                                  this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    QString style = (a? "background-color: yellow" : "background-color: none" );
                                                                                                                    wdg_jaunechk->setStyleSheet(style);
                                                                                                                });
    connect (wdg_inactifchk,       &UpCheckBox::uptoggled,                                  this,   [&](bool a) {
                                                                                                                    EnableWidget(!a);
                                                                                                                    EnableOKpushButton();
                                                                                                                });
    connect (wdg_multifocalchk,    &UpCheckBox::uptoggled,                                  this,   [&](bool a) {
                                                                                                                    EnableOKpushButton();
                                                                                                                    wdg_addinterlbl         ->setVisible(a);
                                                                                                                    wdg_addnearlbl          ->setVisible(a);
                                                                                                                    wdg_addnearspin         ->setVisible(a);
                                                                                                                    wdg_addinterspin        ->setVisible(a);
                                                                                                                    wdg_edofchk             ->setChecked(false);
                                                                                                                });
    connect (wdg_modifButton,      &QPushButton::released,                                  this,   [&] {
                                                                                                            EnableOKpushButton();
                                                                                                            wdg_nominalrb  ->setEnabled(true);
                                                                                                            wdg_ulibrb  ->setEnabled(true);
                                                                                                            wdg_optimizedrb  ->setEnabled(true);
                                                                                                        });
    connect (wdg_puissancemaxspin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_puissanceminspin,  QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_cylindremaxspin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_cylindreminspin,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_addinterspin,      QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_addnearspin,       QOverload<double>::of(&QDoubleSpinBox::valueChanged),   this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_imgIOL,            &QLabel::customContextMenuRequested,                    this,   &dlg_identificationIOL::menuChangeImage);
    connect (wdg_imgIOL,            &UpLabel::dblclick,                                     this,   [&] {if (wdg_imgIOL->isEnabled()) changeImage();});
    connect (wdg_nominalrb,         &QRadioButton::clicked,                                 this,   [&](bool a) { if (a) switchDisplayConstant(Nominal);});
    connect (wdg_ulibrb,            &QRadioButton::clicked,                                 this,   [&](bool a) { if (a) switchDisplayConstant(Ulib);});
    connect (wdg_optimizedrb,       &QRadioButton::clicked,                                 this,   [&](bool a) { if (a) switchDisplayConstant(Optimized);});

    if (wdg_recopiebutton)
        connect (wdg_recopiebutton,  &UpPushButton::clicked,                                this,   &dlg_identificationIOL::creeCopieIOL);
    if (wdg_toolbar)
        connect (wdg_toolbar,        &UpToolBar::TBSignal,                                  this,   [=] {NavigueVers(wdg_toolbar->choix());});
}

void dlg_identificationIOL::disconnectSignals()
{
    OKButton->disconnect();
    RecordButton->disconnect();
    wdg_manufacturercombo->disconnect();
    wdg_nomiolline->disconnect();
    wdg_Aoptline->disconnect();
    wdg_Aecholine->disconnect();
    wdg_holladayline->disconnect();
    wdg_haigisaline->disconnect();
    wdg_haigisbline->disconnect();
    wdg_haigiscline->disconnect();
    wdg_materiaubox->lineEdit()->disconnect();
    wdg_materiaubox->disconnect();
    wdg_hydrofilybox->disconnect();
    wdg_typebox->disconnect();
    wdg_remarquetxt->disconnect();
    wdg_diaoptique->disconnect();
    wdg_diaht->disconnect();
    wdg_diainjecteur->disconnect();
    wdg_prechargechk->disconnect();
    wdg_edofchk->disconnect();
    wdg_toricchk->disconnect();
    wdg_toricchk2->disconnect();
    wdg_jaunechk->disconnect();
    wdg_inactifchk->disconnect();
    wdg_multifocalchk->disconnect();
    wdg_modifButton->disconnect();
    wdg_singlepiecechk->disconnect();
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
    disconnectSignals();
    bool nominalvalid = true;
    bool ulibvalid = true;
    bool optimizedvalid = true;
    if (wdg_toolbar)
        wdg_toolbar         ->setEnabled(true);
    if (wdg_recopiebutton)
        wdg_recopiebutton   ->setEnabled(false);

    if (m_mode == Modification)        
    {
        m_currentIOL = iol;
        nominalvalid = (m_currentIOL->csteAEcho_nominal() > 0 || m_currentIOL->csteAopt_nominal() > 0);
        ulibvalid = (m_currentIOL->results_ulib() > 0);
        optimizedvalid = (m_currentIOL->results_optimized() > 0);
        if (m_currentmanufacturer)
            wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
        wdg_ulibrb      ->setText("ulib (" + QString::number(m_currentIOL->results_ulib()) + ")");
        wdg_optimizedrb ->setText("iolcon optim. (" + QString::number(m_currentIOL->results_optimized()) + ")");
        if (nominalvalid || iol->hasnoregisteredcsteA())
            switchDisplayConstant(Nominal);
        else if (ulibvalid)
            switchDisplayConstant(Ulib);
        else if (optimizedvalid)
            switchDisplayConstant(Optimized);
        else
            switchDisplayConstant(None);

        wdg_nomiolline      ->setText(m_currentIOL->modele());
        wdg_modifButton     ->setVisible(true);

        double n = m_currentIOL->csteAopt_nominal();
            wdg_Aoptline        ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->csteAEcho_nominal();
            wdg_Aecholine       ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->holladay1_nominal();
            wdg_holladayline    ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->haigisa0_nominal();
            wdg_haigisaline     ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->haigisa1_nominal();
            wdg_haigisbline     ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->haigisa2_nominal();
            wdg_haigiscline     ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->hofferQ_nominal();
            wdg_hofferQline     ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->barrettDF_nominal();
            wdg_barrettDFline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->barrettLF_nominal();
            wdg_barrettLFline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->olsen_nominal();
            wdg_olsenline       ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");

        n = m_currentIOL->csteAopt_ulib();
            wdg_UAoptline       ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->csteAEcho_ulib();
            wdg_UAecholine      ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->holladay1_ulib();
            wdg_Uholladayline   ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->haigisa0_ulib();
            wdg_Uhaigisaline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->haigisa1_ulib();
            wdg_Uhaigisbline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->haigisa2_ulib();
            wdg_Uhaigiscline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->hofferQ_ulib();
            wdg_UhofferQline    ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->barrettDF_ulib();
            wdg_UbarrettDFline   ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->barrettLF_ulib();
            wdg_UbarrettLFline   ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->olsen_ulib();
            wdg_Uolsenline      ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        int nb = m_currentIOL->results_ulib();
            wdg_UNbCasesline    ->setText(nb != 0? QString::number(nb) : "");

        n = m_currentIOL->csteAopt_optimized();
            wdg_OAoptline       ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->holladay1_optimized();
            wdg_Oholladayline   ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        n = m_currentIOL->haigisa0_optimized();
            wdg_Ohaigisaline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->haigisa1_optimized();
            wdg_Ohaigisbline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->haigisa2_optimized();
            wdg_Ohaigiscline    ->setText(n != 0.0? QLocale().toString(n, 'f', 4) : "");
        n = m_currentIOL->hofferQ_optimized();
            wdg_OhofferQline    ->setText(n != 0.0? QLocale().toString(n, 'f', 2) : "");
        nb = m_currentIOL->results_optimized();
            wdg_ONbCasesline    ->setText(nb != 0? QString::number(nb) : "");

        wdg_puissancemaxspin->setValuewithPrefix(m_currentIOL->pwrmax());
        wdg_puissanceminspin->setValuewithPrefix(m_currentIOL->pwrmin());
        if (m_currentIOL->opticalmaterial() != "")
            wdg_materiaubox ->setCurrentText(m_currentIOL->opticalmaterial());
        else
            wdg_materiaubox ->setCurrentIndex(-1);
        wdg_hydrofilybox ->setCurrentIndex(wdg_hydrofilybox->findText(m_currentIOL->hydrofily()));
        if (m_currentIOL->typeString() != "")
            wdg_typebox     ->setCurrentText(m_currentIOL->typeString());
        else
            wdg_typebox     ->setCurrentIndex(-1);
        wdg_remarquetxt     ->setPlainText(m_currentIOL->remarque());
        wdg_inactifchk      ->setChecked(!m_currentIOL->isactif());
        wdg_jaunechk        ->setChecked(m_currentIOL->isyellow());
        if (m_currentIOL->isyellow())
            wdg_jaunechk->setStyleSheet("background-color: yellow");
        else
            wdg_jaunechk->setStyleSheet("");
        wdg_multifocalchk   ->setChecked(m_currentIOL->ismultifocal());
        wdg_prechargechk    ->setChecked(m_currentIOL->ispreloaded());
        wdg_edofchk         ->setChecked(m_currentIOL->isedof());
        wdg_addnearspin     ->setVisible(m_currentIOL->addnear() > 0 || m_currentIOL->ismultifocal());
        wdg_addinterspin    ->setVisible(m_currentIOL->addintermediate() > 0 || m_currentIOL->isedof() || m_currentIOL->ismultifocal());
        wdg_addnearlbl      ->setVisible(m_currentIOL->addnear() > 0 || m_currentIOL->ismultifocal());
        wdg_addinterlbl     ->setVisible(m_currentIOL->addintermediate() > 0 || m_currentIOL->isedof() || m_currentIOL->ismultifocal());
        if (m_currentIOL->addnear() >0)
            wdg_addnearspin     ->setValue(m_currentIOL->addnear());
        if (m_currentIOL->addintermediate() >0)
            wdg_addinterspin    ->setValue(m_currentIOL->addintermediate());
        wdg_toricchk            ->setChecked(m_currentIOL->istoric());
        wdg_toricchk2           ->setChecked(m_currentIOL->istoric());
        wdg_singlepiecechk      ->setChecked(m_currentIOL->issinglepiece());
        wdg_hapticmateriaubox   ->setVisible(!m_currentIOL->issinglepiece());
        wdg_HapticMateriaulbl   ->setVisible(!m_currentIOL->issinglepiece());
        wdg_hapticmateriaubox   ->setCurrentText(m_currentIOL->hapticalmaterial());
        wdg_cylindres           ->setVisible(m_currentIOL->istoric());
        if (m_currentIOL->istoric())
        {
            if (m_currentIOL->cylmax() > 0.0)
                wdg_cylindremaxspin->setValuewithPrefix(m_currentIOL->cylmax());
            if (m_currentIOL->cylmin() > 0.0)
                wdg_cylindreminspin->setValuewithPrefix(m_currentIOL->cylmin());
        }
        n = m_currentIOL->diaall();
        wdg_diaht               ->setText(n > 0.0? QLocale().toString(n, 'f', 1) : "");
        n = m_currentIOL->opticdiameter();
        wdg_diaoptique          ->setText(n > 0.0? QLocale().toString(n, 'f', 1) : "");
        n = m_currentIOL->diainjecteur();
        wdg_diainjecteur        ->setText(n > 0.0? QLocale().toString(n, 'f', 1) : "");
        if (m_currentIOL->arrayimgiol() != QByteArray())
        {
            setimage(m_currentIOL->image());
            m_listbinds[CP_ARRAYIMG_IOLS]   = m_currentIOL->arrayimgiol();
            m_listbinds[CP_TYPIMG_IOLS]     = m_currentIOL->imageformat();
        }
        else setimage(m_nullimage);
        EnableWidget(m_currentIOL->isactif());
        int idx = m_listeidIOLs.indexOf(m_currentIOL->id());
        wdg_toolbar->First()    ->setEnabled(idx>0);
        wdg_toolbar->Prec()     ->setEnabled(idx>0);
        wdg_toolbar->Next()     ->setEnabled(idx < m_listeidIOLs.size()-1);
        wdg_toolbar->Last()     ->setEnabled(idx < m_listeidIOLs.size()-1);
    }
    else if (m_mode == Creation)
    {
        wdg_manufacturercombo   ->setCurrentIndex(-1);
        wdg_ulibrb              ->setText("ulib (0)");
        wdg_optimizedrb         ->setText("iolcon optim. (0)");
        wdg_modifButton         ->setVisible(false);

        int n = 0;
        wdg_UNbCasesline        ->setText(QString::number(n));
        wdg_ONbCasesline        ->setText(QString::number(n));

        wdg_addnearspin         ->setVisible(false);
        wdg_addinterspin        ->setVisible(false);
        wdg_addnearlbl          ->setVisible(false);
        wdg_addinterlbl         ->setVisible(false);
        wdg_cylindres           ->setVisible(false);
        setimage(m_nullimage);
        switchDisplayConstant(Nominal);
        EnableWidget(true);
        wdg_toolbar             ->setVisible(false);
    }

    wdg_nominalrb   ->setEnabled(nominalvalid);
    wdg_ulibrb      ->setEnabled(ulibvalid);
    wdg_optimizedrb ->setEnabled(optimizedvalid);
    RecordButton->setEnabled(false);
    connectSignals();
}

void dlg_identificationIOL::menuChangeImage()
{
    QMenu m_menuContextuel;
    QAction *pAction_ChangeImage = m_menuContextuel.addAction(tr("Modifier l'image"));
    connect (pAction_ChangeImage,  &QAction::triggered,    this, &dlg_identificationIOL::changeImage);
    if (m_currentIOLimage != m_nullimage)
    {
        QAction *pAction_ChangeImage = m_menuContextuel.addAction(tr("Supprimer l'image"));
        connect (pAction_ChangeImage,  &QAction::triggered,    this, &dlg_identificationIOL::supprimeImage);
    }
    m_menuContextuel.exec(cursor().pos());
}

void dlg_identificationIOL::changeImage()
{
    QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    QString path_file_origin = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), desktop + "/ImagesIOL" ,  tr("Images (*.pdf *.png *.jpg *.jpeg)"));
    if (path_file_origin != "")
    {
        QString formatdoc = QFileInfo(path_file_origin).suffix().toLower();
        // Contenu du document------------------------------------------------------------------------------------------------------------------------------------------------
        if ((formatdoc == JPG || formatdoc == JPEG || formatdoc == PNG))
            if (!Utils::CompressFileToJPG(path_file_origin, false, SIZEMAXIMGIOL))
                return;
        QFile       file_origin(path_file_origin);
        file_origin                         .open(QIODevice::ReadOnly);
        QByteArray  ba                      = file_origin.readAll();
        QString     suffix                  = QFileInfo(file_origin).suffix().toLower();
        suffix                              = (suffix == PDF? PDF : JPG);
        QImage      img;
        img                                 = (suffix == PDF? Utils::calcImagefromPdf(ba).at(0) : QImage(path_file_origin));
        file_origin                         .close();
        setimage(img);
        m_listbinds[CP_ARRAYIMG_IOLS]       = ba;
        m_listbinds[CP_TYPIMG_IOLS]         = suffix;
        RecordButton->setEnabled(true);
        EnableOKpushButton();
        /*! le code qui suit provoque une erreur.
         *  Quand la UpmessageBox est fermée, la fonction ModifIol() de dlg_listeiols s'interrompt et ne reprend qu'à la fermeture de la fiche dlg_listeiols appelante...
         *  if (UpMessageBox::Question(this, tr("Image enregistrée"), tr("Voulez-vous suprimer le fichier image origine?")) == UpSmallButton::STARTBUTTON)
         *      Utils::removeWithoutPermissions(file_origin);
         */
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

void dlg_identificationIOL::switchDisplayConstant(TypeConstant typcst)
{
        wdg_nominalCsteAEcho    ->setVisible(typcst == Nominal);
        wdg_ulibCsteAEcho       ->setVisible(typcst == Ulib);

        wdg_nominalCsteA        ->setVisible(typcst == Nominal);
        wdg_ulibCsteA           ->setVisible(typcst == Ulib);
        wdg_optimizedCsteA      ->setVisible(typcst == Optimized);

        wdg_nominalhaigis       ->setVisible(typcst == Nominal);
        wdg_ulibhaigis          ->setVisible(typcst == Ulib);
        wdg_optimizedhaigis     ->setVisible(typcst == Optimized);

        wdg_nominalbarrett      ->setVisible(typcst == Nominal);
        wdg_ulibbarrett         ->setVisible(typcst == Ulib);
        wdg_nominalhofferq      ->setVisible(typcst == Nominal);
        wdg_ulibhofferq         ->setVisible(typcst == Ulib);
        wdg_optimizedhofferq    ->setVisible(typcst == Optimized);
        wdg_ulibnbcases         ->setVisible(typcst == Ulib);
        wdg_optimizednbcases    ->setVisible(typcst == Optimized);
        wdg_ulibrb              ->setChecked(typcst == Ulib);
        wdg_nominalrb           ->setChecked(typcst == Nominal);
        wdg_optimizedrb         ->setChecked(typcst == Optimized);

    bool nominalvalid = true;
    bool ulibvalid = true;
    bool optimizedvalid = true;
    if (m_currentIOL)
    {
        nominalvalid   = (m_currentIOL->csteAEcho_nominal() > 0 || m_currentIOL->csteAopt_nominal() > 0);
        ulibvalid      = (m_currentIOL->results_ulib() > 0);
        optimizedvalid = (m_currentIOL->results_optimized() > 0);
    }
    wdg_nominalrb       ->setEnabled(nominalvalid);
    wdg_ulibrb          ->setEnabled(ulibvalid);
    wdg_optimizedrb     ->setEnabled(optimizedvalid);
}

void dlg_identificationIOL:: EnableOKpushButton()
{
    bool a  = wdg_nomiolline->text() != ""
           && wdg_manufacturercombo->currentData().toInt()>0;
    OKButton                ->setEnabled(a);
    RecordButton            ->setEnabled(a);
    if (wdg_toolbar)
        wdg_toolbar         ->setEnabled(a);
    if(wdg_recopiebutton)
        wdg_recopiebutton   ->setEnabled(false);
    wdg_modifButton->disconnect();
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

bool dlg_identificationIOL::EnregistreIOL()
{
    qApp->focusWidget()->clearFocus();

    // - Verifying if all importants fields are completed
    if (wdg_nomiolline->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas indiqué le modèle d'implant!"));
        wdg_nomiolline->setFocus();
        return false;
    }
    if (wdg_typebox->currentText() == "")
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas saisi le type de l'implant"));
        wdg_typebox->setFocus();
        return false;
    }
    QWidget* widg = VerifCstesANominal();
    if (widg != Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Vous n'avez saisi aucune constante A!"));
        switchDisplayConstant(Nominal);
        widg->setFocus();
        return false;
    }

    widg = VerifCstesAUlib();
    if (widg != Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Vous avez saisi des paramètres Ulib incomplets!"), tr("Vous devez renseigner au moins une constante A et le nombre de cas"));
        switchDisplayConstant(Ulib);
        widg->setFocus();
        return false;
    }

    widg = VerifCstesAOptimized();
    if (widg != Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Vous avez saisi des paramètres iolcon optimized incomplets!"), tr("Vous devez renseigner au moins la constante A et le nombre de cas"));
        switchDisplayConstant(Optimized);
        widg->setFocus();
        return false;
    }

    // - Verifying if this IOL already existst in db
    bool ok;
    QString requete = "select " CP_ID_IOLS " from " TBL_IOLS
                      " where " CP_MODELNAME_IOLS " = '" + wdg_nomiolline->text() + "'";
    QVariantList ioldata = DataBase::I()->getFirstRecordFromStandardSelectSQL(requete,ok, tr("Impossible d'interroger la table des implants!"));
    if (!ok)
    {
        reject();
        return false;
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
                wdg_toolbar         ->setEnabled(true);
                wdg_recopiebutton   ->setEnabled(true);
                AfficheDatasIOL(m_currentIOL);
                OKButton            ->setEnabled(false);
                RecordButton        ->setEnabled(false);
                connect (wdg_modifButton,   &QPushButton::released, this,   [&] {
                    EnableOKpushButton();
                    wdg_nominalrb   ->setEnabled(true);
                    wdg_ulibrb      ->setEnabled(true);
                    wdg_optimizedrb ->setEnabled(true);
                });
                return false;
            }
            break;
        case Creation:
            UpMessageBox::Watch(this,tr("Cet implant existe déjà!"));
            m_currentIOL = Datas::I()->iols->getById(ioldata.at(0).toInt());
            m_currentmanufacturer = Datas::I()->manufacturers->getById(m_currentIOL->idmanufacturer());
            reconstruitListeIOLs(m_currentmanufacturer);
            OKButton                ->setEnabled(false);
            RecordButton            ->setEnabled(false);
            wdg_toolbar             ->setEnabled(true);
            wdg_recopiebutton       ->setEnabled(true);
            m_mode = Modification;
            wdg_recopiebutton       = new UpPushButton ("Recopier l'IOL");
            wdg_recopiebutton       ->setVisible(false);
            lay_materiau            ->addWidget(wdg_recopiebutton);
            wdg_toolbar             = new UpToolBar();
            lay_materiau            ->addWidget(wdg_toolbar);
            wdg_toolbar             ->setEnabled(m_listeidIOLs.size()>1);
            AfficheDatasIOL(m_currentIOL);
            connect (wdg_modifButton,       &QPushButton::released, this,   [&] {
                EnableOKpushButton();
                wdg_nominalrb       ->setEnabled(true);
                wdg_ulibrb          ->setEnabled(true);
                wdg_optimizedrb     ->setEnabled(true);
            });
            return false;
        }
    }

    m_listbinds[CP_MODELNAME_IOLS]      = wdg_nomiolline->text();
    m_listbinds[CP_IDMANUFACTURER_IOLS] = m_currentmanufacturer->id();
    m_listbinds[CP_HOLL1_IOLS]          = (QLocale().toDouble(wdg_holladayline->text()) != 0.0?     QLocale().toDouble(wdg_holladayline->text())    : QVariant());
    m_listbinds[CP_CSTEAECHO_IOLS]      = (QLocale().toDouble(wdg_Aecholine->text()) != 0.0?        QLocale().toDouble(wdg_Aecholine->text())       : QVariant());
    m_listbinds[CP_CSTEAOPT_IOLS]       = (QLocale().toDouble(wdg_Aoptline->text()) != 0.0?         QLocale().toDouble(wdg_Aoptline->text())        : QVariant());
    m_listbinds[CP_HAIGISA0_IOLS]       = (QLocale().toDouble(wdg_haigisaline->text()) != 0.0?      QLocale().toDouble(wdg_haigisaline->text())     : QVariant());
    m_listbinds[CP_HAIGISA1_IOLS]       = (QLocale().toDouble(wdg_haigisbline->text()) != 0.0?      QLocale().toDouble(wdg_haigisbline->text())     : QVariant());
    m_listbinds[CP_HAIGISA2_IOLS]       = (QLocale().toDouble(wdg_haigiscline->text()) != 0.0?      QLocale().toDouble(wdg_haigiscline->text())     : QVariant());
    m_listbinds[CP_HOFFERQ_IOLS]        = (QLocale().toDouble(wdg_hofferQline->text()) != 0.0?      QLocale().toDouble(wdg_hofferQline->text())     : QVariant());
    m_listbinds[CP_BARRETTDF_IOLS]      = (QLocale().toDouble(wdg_barrettDFline->text()) != 0.0?    QLocale().toDouble(wdg_barrettDFline->text())   : QVariant());
    m_listbinds[CP_BARRETTLF_IOLS]      = (QLocale().toDouble(wdg_barrettLFline->text()) != 0.0?    QLocale().toDouble(wdg_barrettLFline->text())   : QVariant());
    m_listbinds[CP_OLSEN_IOLS]          = (QLocale().toDouble(wdg_olsenline->text()) != 0.0?        QLocale().toDouble(wdg_olsenline->text())       : QVariant());
    m_listbinds[CP_OPTICMATERIAU_IOLS]  = wdg_materiaubox->currentText();
    m_listbinds[CP_HYDROFILY_IOLS]      = wdg_hydrofilybox->currentText();
    m_listbinds[CP_HAPTICMATERIAU_IOLS] = wdg_hapticmateriaubox->currentText();
    m_listbinds[CP_REMARQUE_IOLS]       = wdg_remarquetxt->toPlainText();
    m_listbinds[CP_DIAALL_IOLS]         = (QLocale().toDouble(wdg_diaht->text()) >0.0?              QLocale().toDouble(wdg_diaht->text())           : QVariant());
    m_listbinds[CP_DIAOPT_IOLS]         = (QLocale().toDouble(wdg_diaoptique->text()) >0.0?         QLocale().toDouble(wdg_diaoptique->text())      : QVariant());
    m_listbinds[CP_DIAINJECTEUR_IOLS]   = (QLocale().toDouble(wdg_diainjecteur->text()) >0.0?       QLocale().toDouble(wdg_diainjecteur->text())    : QVariant());
    m_listbinds[CP_PRECHARGE_IOLS]      = (wdg_prechargechk->isChecked()?                           "1"                                             : QVariant());
    m_listbinds[CP_SINGLEPIECE_IOLS]    = (wdg_singlepiecechk->isChecked()?                         "1"                                             : QVariant());
    m_listbinds[CP_MAXPWR_IOLS]         = wdg_puissancemaxspin->value();
    m_listbinds[CP_MINPWR_IOLS]         = wdg_puissanceminspin->value();
    m_listbinds[CP_ADDINTERMEDIATE_IOLS]= wdg_addinterspin->value();
    m_listbinds[CP_ADDNEAR_IOLS]        = wdg_addnearspin->value();
    m_listbinds[CP_MAXCYL_IOLS]         = (wdg_toricchk->checkState() == Qt::Checked?               wdg_cylindremaxspin->value()                    : QVariant());
    m_listbinds[CP_MINCYL_IOLS]         = (wdg_toricchk->checkState() == Qt::Checked?               wdg_cylindreminspin->value()                    : QVariant());
    m_listbinds[CP_JAUNE_IOLS]          = (wdg_jaunechk->isChecked()?                               "1"                                             : QVariant());
    m_listbinds[CP_MULTIFOCAL_IOLS]     = (wdg_multifocalchk->isChecked()?                          "1"                                             : QVariant());
    m_listbinds[CP_INACTIF_IOLS]        = (wdg_inactifchk->isChecked()?                             "1"                                             : QVariant());
    m_listbinds[CP_EDOF_IOLS]           = (wdg_edofchk->isChecked()?                                "1"                                             : QVariant());
    m_listbinds[CP_TORIC_IOLS]          = (wdg_toricchk->isChecked()?                               "1"                                             : QVariant());
    m_listbinds[CP_TYP_IOLS]            = (wdg_typebox->currentIndex()>-1?                          QString::number(wdg_typebox->currentIndex()+1)  : QVariant());

    m_listbinds[CP_CSTEAECHOU_IOLS]     = (QLocale().toDouble(wdg_UAecholine->text()) != 0.0?       QLocale().toDouble(wdg_UAecholine->text())      : QVariant());
    m_listbinds[CP_CSTEAOPTU_IOLS]      = (QLocale().toDouble(wdg_UAoptline->text()) != 0.0?        QLocale().toDouble(wdg_UAoptline->text())       : QVariant());
    m_listbinds[CP_HAIGISA0U_IOLS]      = (QLocale().toDouble(wdg_Uhaigisaline->text()) != 0.0?     QLocale().toDouble(wdg_Uhaigisaline->text())    : QVariant());
    m_listbinds[CP_HAIGISA1U_IOLS]      = (QLocale().toDouble(wdg_Uhaigisbline->text()) != 0.0?     QLocale().toDouble(wdg_Uhaigisbline->text())    : QVariant());
    m_listbinds[CP_HAIGISA2U_IOLS]      = (QLocale().toDouble(wdg_Uhaigiscline->text()) != 0.0?     QLocale().toDouble(wdg_Uhaigiscline->text())    : QVariant());
    m_listbinds[CP_HOFFERQU_IOLS]       = (QLocale().toDouble(wdg_UhofferQline->text()) != 0.0?     QLocale().toDouble(wdg_UhofferQline->text())    : QVariant());
    m_listbinds[CP_BARRETTDFU_IOLS]     = (QLocale().toDouble(wdg_UbarrettDFline->text()) != 0.0?   QLocale().toDouble(wdg_UbarrettDFline->text())  : QVariant());
    m_listbinds[CP_BARRETTLFU_IOLS]     = (QLocale().toDouble(wdg_UbarrettLFline->text()) != 0.0?   QLocale().toDouble(wdg_UbarrettLFline->text())  : QVariant());
    m_listbinds[CP_OLSENU_IOLS]         = (QLocale().toDouble(wdg_Uolsenline->text()) != 0.0?       QLocale().toDouble(wdg_Uolsenline->text())      : QVariant());
    m_listbinds[CP_RESULTSU_IOLS]       = wdg_UNbCasesline->text().toInt();

    m_listbinds[CP_CSTEAOPTO_IOLS]      = (QLocale().toDouble(wdg_OAoptline->text()) != 0.0?        QLocale().toDouble(wdg_OAoptline->text())       : QVariant());
    m_listbinds[CP_HAIGISA0O_IOLS]      = (QLocale().toDouble(wdg_Ohaigisaline->text()) != 0.0?     QLocale().toDouble(wdg_Ohaigisaline->text())    : QVariant());
    m_listbinds[CP_HAIGISA1O_IOLS]      = (QLocale().toDouble(wdg_Ohaigisbline->text()) != 0.0?     QLocale().toDouble(wdg_Ohaigisbline->text())    : QVariant());
    m_listbinds[CP_HAIGISA2O_IOLS]      = (QLocale().toDouble(wdg_Ohaigiscline->text()) != 0.0?     QLocale().toDouble(wdg_Ohaigiscline->text())    : QVariant());
    m_listbinds[CP_HOFFERQO_IOLS]       = (QLocale().toDouble(wdg_OhofferQline->text()) != 0.0?     QLocale().toDouble(wdg_OhofferQline->text())    : QVariant());
    m_listbinds[CP_RESULTSO_IOLS]       = wdg_ONbCasesline->text().toInt();
    if (m_mode == Creation)
        m_currentIOL = Datas::I()->iols->CreationIOL(m_listbinds);
    else if (m_mode == Modification)
    {
        DataBase::I()->UpDateIOL(m_currentIOL->id(), m_listbinds);
        Datas::I()->iols->getById(m_currentIOL->id(), true);
    }
    RecordButton->setEnabled(false);
    return true;
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
    m_currentIOLimage = img;
    if (m_currentIOL)
        m_currentIOL->setimage(img);
}

QWidget* dlg_identificationIOL::VerifCstesANominal()
{
    if (wdg_UNbCasesline->text().toInt() == 0 && wdg_ONbCasesline->text().toInt() == 0
        && QLocale().toDouble(wdg_Aecholine->text()) == 0 && QLocale().toDouble(wdg_Aoptline->text())==0
        && QLocale().toDouble(wdg_UAecholine->text()) == 0 && QLocale().toDouble(wdg_UAoptline->text()) == 0
        && QLocale().toDouble(wdg_OAoptline->text()) == 0)
    {
        if (QLocale().toDouble(wdg_Aecholine->text()) == 0)
            return wdg_Aecholine;
        if (QLocale().toDouble(wdg_Aoptline->text()) == 0)
            return wdg_Aoptline;
    }
    return Q_NULLPTR;
}

QWidget *dlg_identificationIOL::VerifCstesAUlib()
{
    if ((QLocale().toDouble(wdg_UAecholine->text()) > 0 && wdg_UNbCasesline->text().toInt() == 0)
        || (QLocale().toDouble(wdg_UAoptline->text()) > 0 && wdg_UNbCasesline->text().toInt() == 0)
        || (QLocale().toDouble(wdg_UAoptline->text()) == 0 && QLocale().toDouble(wdg_UAecholine->text()) == 0 && wdg_UNbCasesline->text().toInt() > 0))
    {
        if (QLocale().toDouble(wdg_UAecholine->text()) == 0)
            return wdg_UAecholine;
        if (QLocale().toDouble(wdg_UAoptline->text()) == 0)
            return wdg_UAoptline;
        if (wdg_UNbCasesline->text().toInt() == 0)
            return wdg_UNbCasesline;
    }
    return Q_NULLPTR;
}

QWidget* dlg_identificationIOL::VerifCstesAOptimized()
{
    if ((QLocale().toDouble(wdg_OAoptline->text()) > 0 && wdg_ONbCasesline->text().toInt() == 0)
        || (QLocale().toDouble(wdg_OAoptline->text()) == 0 && wdg_ONbCasesline->text().toInt() > 0))
    {
        if (QLocale().toDouble(wdg_OAoptline->text()) == 0)
            return wdg_OAoptline;
        if (wdg_ONbCasesline->text().toInt() == 0)
            return wdg_ONbCasesline;
    }
    return Q_NULLPTR;
}
