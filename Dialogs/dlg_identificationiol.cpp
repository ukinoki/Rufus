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
    m_mode = mode;
    m_currentIOL = iol;
    m_currentmanufacturer = man;
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
    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    QComboBox *manufacturercombo = new QComboBox();
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        manufacturercombo->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
        manufacturercombo->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
    }
    if (m_currentmanufacturer)
    {
        manufacturercombo           ->setCurrentIndex(manufacturercombo->findData(m_currentmanufacturer->id()));
        manufacturercombo           ->setEnabled(false);
    }
    choixManufacturerIOLLay     ->addWidget(lblManufacturerIOL);
    choixManufacturerIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerIOLLay     ->addWidget(manufacturercombo);
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
    upDoubleValidator *csteA_val    = new upDoubleValidator(0, 121, 1, this);
    wdg_Aoptline                    ->setValidator(csteA_val);
    wdg_Aoptline                    ->setFixedSize(QSize(60,28));
    wdg_Aecholine                   = new UpLineEdit();
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

    connect(OKButton, &QPushButton::clicked, this, &dlg_identificationIOL::OKpushButtonClicked);
}

void dlg_identificationIOL::OKpushButtonClicked()
{
    UpLineEdit          *m_nomiolline;
    UpLineEdit          *m_Aoptline;
    UpLineEdit          *m_Aecholine;
    UpLineEdit          *m_ACDline;
    UpLineEdit          *wdg_haigisaline;
    UpLineEdit          *wdg_haigisbline;
    UpLineEdit          *wdg_haigiscline;
    UpLabel             *m_imgIOL;
    UpLineEdit          *m_materiauline;
    UpTextEdit          *m_remarquetxt;
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
        if (ioldata.at(0).toInt() != m_currentIOL->id())
        {
            UpMessageBox::Watch(this,tr("Cet implant existe déjà!"));
            m_currentIOL = Datas::I()->iols->getById(ioldata.at(0).toInt());
            OKButton->setEnabled(false);
            m_mode = Modification;
            disconnect (OKButton,   &QPushButton::clicked,  this,   &dlg_identificationIOL::OKpushButtonClicked);
            connect(OKButton,       &QPushButton::clicked,  this,   &dlg_identificationIOL::accept);
            return;
        }
    }



    switch (m_mode) {
    case Creation:
    {
        QString modele = wdg_nomiolline->text();
        for (int i = 0; i < m_IOLsmodel->rowCount(); ++i)
        {
            UpStandardItem * upitem = dynamic_cast<UpStandardItem*>(m_IOLsmodel->item(i));
            IOL *iol = dynamic_cast<IOL*>(upitem->item());
            if (iol->modele() == modele)
            {
                UpMessageBox::Watch(this, tr("Cet implant existe déjà!"));
                return;
            }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_MODELNAME_IOLS]    = modele;
        listbinds[CP_IDMANUFACTURER_IOLS]  = m_currentmanufacturer->id();
        listbinds[CP_INACTIF_IOLS]  = QVariant();
        m_currentIOL = Datas::I()->iols->CreationIOL(listbinds);
        close();
        break;
    }
    case Modification:
        break;
    }
}
