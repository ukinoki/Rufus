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
            m_manufacturersmodel->sort(0);
        }
    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    wdg_manufacturercombo = new QComboBox();
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        wdg_manufacturercombo   ->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
        wdg_manufacturercombo   ->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
    }
    if (m_currentmanufacturer)
        wdg_manufacturercombo   ->setEnabled(false);
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
    wdg_nomiolline              ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric));
    wdg_nomiolline              ->setMaxLength(40);
    choixIOLLay                 ->addWidget(lblIOL);
    choixIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay                 ->addWidget(wdg_nomiolline);
    choixIOLLay                 ->setSpacing(5);
    choixIOLLay                 ->setContentsMargins(0,0,0,0);

    //! Constante A et ACD
    QHBoxLayout *csteIOLLay         = new QHBoxLayout();
    UpLabel* lblcsteAIOL            = new UpLabel;
    lblcsteAIOL                     ->setText(tr("Constante A"));
    UpLabel* lblAOptIOL             = new UpLabel;
    lblAOptIOL                      ->setText(tr("Optique"));
    UpLabel* lblAEchoIOL            = new UpLabel;
    lblAEchoIOL                     ->setText(tr("Echo"));
    UpLabel* lblACDIOL              = new UpLabel;
    lblACDIOL                       ->setText(tr("ACD"));
    wdg_Aoptline                    = new UpLineEdit();
    wdg_Aoptline                    ->setValidator(new QRegExpValidator(rgx_csteA, wdg_Aoptline));
    wdg_Aoptline                    ->setFixedSize(QSize(60,28));
    wdg_Aecholine                   = new UpLineEdit();
    wdg_Aecholine                   ->setValidator(new QRegExpValidator(rgx_csteA, wdg_Aecholine));
    wdg_Aecholine                   ->setFixedSize(QSize(60,28));
    upDoubleValidator *ACD_val      = new upDoubleValidator(1, 8, 2, this);
    wdg_ACDline                     = new UpLineEdit();
    wdg_ACDline                     ->setValidator(ACD_val);
    wdg_ACDline                     ->setFixedSize(QSize(50,28));
    csteIOLLay                      ->addWidget(lblcsteAIOL);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(lblAOptIOL);
    csteIOLLay                      ->addWidget(wdg_Aoptline);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(lblAEchoIOL);
    csteIOLLay                      ->addWidget(wdg_Aecholine);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(lblACDIOL);
    csteIOLLay                      ->addWidget(wdg_ACDline);
    csteIOLLay                      ->setSpacing(5);
    csteIOLLay                      ->setContentsMargins(0,0,0,0);
    for (int i=0; i < csteIOLLay->count(); ++i)
    {
        if (i==7)
            csteIOLLay->setStretch(i,5);
        else
            csteIOLLay->setStretch(i,1);
    }

    //! Haigis
    QHBoxLayout *HaigisLay          = new QHBoxLayout();
    UpLabel* HaigisIOLlbl           = new UpLabel;
    HaigisIOLlbl                    ->setText("Haigis");
    UpLabel* Haigisalbl             = new UpLabel;
    Haigisalbl                      ->setText("a");
    UpLabel* Haigisblbl             = new UpLabel;
    Haigisblbl                      ->setText("b");
    UpLabel* Haigisclbl             = new UpLabel;
    Haigisclbl                      ->setText(tr("c"));
    upDoubleValidator *haigis_val   = new upDoubleValidator(-1, 1, 4, this);
    wdg_haigisaline                 = new UpLineEdit();
    wdg_haigisaline                 ->setValidator(haigis_val);
    wdg_haigisaline                 ->setFixedSize(QSize(80,28));
    wdg_haigisbline                 = new UpLineEdit();
    wdg_haigisbline                 ->setValidator(haigis_val);
    wdg_haigisbline                 ->setFixedSize(QSize(80,28));
    wdg_haigiscline                 = new UpLineEdit();
    wdg_haigiscline                 ->setValidator(haigis_val);
    wdg_haigiscline                 ->setFixedSize(QSize(80,28));
    HaigisLay                       ->addWidget(HaigisIOLlbl);
    HaigisLay                       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                       ->addWidget(Haigisalbl);
    HaigisLay                       ->addWidget(wdg_haigisaline);
    HaigisLay                       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                       ->addWidget(Haigisblbl);
    HaigisLay                       ->addWidget(wdg_haigisbline);
    HaigisLay                       ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                       ->addWidget(Haigisclbl);
    HaigisLay                       ->addWidget(wdg_haigiscline);
    HaigisLay                       ->setSpacing(5);
    HaigisLay                       ->setContentsMargins(0,0,0,0);

    //! Materiau - Image
    QVBoxLayout *MateriauLay    = new QVBoxLayout();
    QHBoxLayout *MateriauImgLay = new QHBoxLayout();
    UpLabel* Materiaulbl        = new UpLabel;
    Materiaulbl                 ->setText(tr("Materiau"));
    wdg_imgIOL                  = new UpLabel;
    wdg_imgIOL                  ->setPixmap(Icons::pxIOL());
    //ImgIOL                      ->setFixedSize(QSize(50,50));
    wdg_materiauline            = new UpLineEdit();
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

    dlglayout()   ->insertLayout(0, RemarqueHLay);
    dlglayout()   ->insertLayout(0, MateriauImgLay);
    dlglayout()   ->insertLayout(0, HaigisLay);
    dlglayout()   ->insertLayout(0, csteIOLLay);
    dlglayout()   ->insertLayout(0, choixIOLLay);
    dlglayout()   ->insertLayout(0, choixManufacturerIOLLay);
    dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlglayout()   ->setSpacing(5);
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
                                                                                                                    qDebug() << m_currentmanufacturer->nom();
                                                                                                                    EnableOKpushButton();
                                                                                                                });
    connect (wdg_nomiolline,        &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Aoptline,          &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_Aecholine,         &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_ACDline,           &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigisaline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigisbline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_haigiscline,       &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_materiauline,      &QLineEdit::textEdited,                                 this,   &dlg_identificationIOL::EnableOKpushButton);
    connect (wdg_remarquetxt,       &QTextEdit::textChanged,                                this,   &dlg_identificationIOL::EnableOKpushButton);
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));
    setStageCount(1);
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
        wdg_materiauline    ->setText(m_currentIOL->materiau());
        wdg_remarquetxt     ->setPlainText(m_currentIOL->remarque());
    }
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
    m_listbinds[CP_ACD_IOLS]            = (QLocale().toDouble(wdg_ACDline->text()) >0?      QLocale().toDouble(wdg_ACDline->text())     : QVariant());
    m_listbinds[CP_CSTEAOPT_IOLS]       = (QLocale().toDouble(wdg_Aoptline->text()) >0?     QLocale().toDouble(wdg_Aoptline->text())    : QVariant());
    m_listbinds[CP_CSTEAECHO_IOLS]      = (QLocale().toDouble(wdg_Aoptline->text()) >0?     QLocale().toDouble(wdg_Aecholine->text())   : QVariant());
    m_listbinds[CP_HAIGISA0_IOLS]       = (QLocale().toDouble(wdg_haigisaline->text()) >0?  QLocale().toDouble(wdg_haigisaline->text()) : QVariant());
    m_listbinds[CP_HAIGISA1_IOLS]       = (QLocale().toDouble(wdg_haigisbline->text()) >0?  QLocale().toDouble(wdg_haigisbline->text()) : QVariant());
    m_listbinds[CP_HAIGISA2_IOLS]       = (QLocale().toDouble(wdg_haigiscline->text()) >0?  QLocale().toDouble(wdg_haigiscline->text()) : QVariant());
    m_listbinds[CP_MATERIAU_IOLS]       = wdg_materiauline->text();
    m_listbinds[CP_REMARQUE_IOLS]       = wdg_remarquetxt->toPlainText();
    m_listbinds[CP_INACTIF_IOLS]        = QVariant();
    accept();
}
