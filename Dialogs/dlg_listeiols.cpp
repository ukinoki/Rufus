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

#include "dlg_listeiols.h"

dlg_listeiols::dlg_listeiols(QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setWindowTitle(tr("Liste des IOLs"));

    wdg_iolstree = new QTreeView(this);
    wdg_iolstree ->setFixedWidth(320);
    wdg_iolstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_iolstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_iolstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_iolstree ->setAnimated(true);
    wdg_iolstree ->setIndentation(10);
    wdg_iolstree ->setMouseTracking(true);
    wdg_iolstree ->header()->setVisible(false);

    ReconstruitTreeViewIOLs(true);

    wdg_buttonframe         = new WidgetButtonFrame(wdg_iolstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    wdg_label               = new UpLabel();
    wdg_label               ->setFixedSize(21,21);
    wdg_label               ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    wdg_chercheuplineedit   = new UpLineEdit();
    wdg_chercheuplineedit   ->setFixedSize(140,25);
    wdg_chercheuplineedit   ->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}");
    wdg_buttonframe->layButtons()->insertWidget(0,wdg_label);
    wdg_buttonframe->layButtons()->insertWidget(0,wdg_chercheuplineedit);
    AjouteLayButtons(UpDialog::ButtonOK);

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());

    connect(OKButton,               &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_chercheuplineedit,  &QLineEdit::textEdited,     this,   [=] (QString txt) { txt = Utils::trimcapitilize(txt, false, true);
                                                                                                    wdg_chercheuplineedit->setText(txt);
                                                                                                    ReconstruitTreeViewIOLs(false, txt);});
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listeiols::ChoixButtonFrame);

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_chercheuplineedit               ->setFocus();
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

void dlg_listeiols::Enablebuttons()
{
    IOL *iol = getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0));
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


void dlg_listeiols::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauIOL();
        break;
    case WidgetButtonFrame::Modifier:
        if (wdg_iolstree->selectionModel()->selectedIndexes().size()==0)
            return;
        ModifIOL(getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0)));
        break;
    case WidgetButtonFrame::Moins:
        SupprIOL();
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
    FicheIOL();
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::FicheIOL(IOL* nwiol)
{
    UpDialog            *dlg_IOL = new UpDialog(this);
    dlg_IOL->setAttribute(Qt::WA_DeleteOnClose);
    dlg_IOL->setWindowTitle(nwiol == Q_NULLPTR? tr("Créer un IOL") : tr("Modifier un IOL"));

    //! FABRICANT
    QHBoxLayout *choixManufacturerIOLLay    = new QHBoxLayout();
    UpLabel* lblManufacturerIOL = new UpLabel;
    lblManufacturerIOL          ->setText(tr("Fabricant"));
    QComboBox *manufacturercombo = new QComboBox();
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        manufacturercombo->addItem(m_manufacturersmodel->item(i)->text());                  //! le nom du fabricant
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != Q_NULLPTR)
            if (itm->item() != Q_NULLPTR)
                manufacturercombo->setItemData(i, itm->item()->id());                       //! l'id en data
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
    QLineEdit *IOLline          = new QLineEdit();
    IOLline                     ->setFixedSize(QSize(150,28));
    IOLline                     ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric));
    choixIOLLay                 ->addWidget(lblIOL);
    choixIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay                 ->addWidget(IOLline);
    choixIOLLay                 ->setSpacing(5);
    choixIOLLay                 ->setContentsMargins(0,0,0,0);

    QHBoxLayout *checkIOLLay    = new QHBoxLayout();
    QCheckBox *IOLchk           = new QCheckBox(tr("Discontinué"));
    IOLchk                      ->setCheckState(Qt::Unchecked);
    checkIOLLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkIOLLay                 ->addWidget(IOLchk);
    checkIOLLay                 ->setSpacing(5);
    checkIOLLay                 ->setContentsMargins(0,0,0,0);

    if (nwiol != Q_NULLPTR)
    {
        manufacturercombo   ->setCurrentIndex(manufacturercombo->findData(nwiol->idmanufacturer()));
        IOLline             ->setText(nwiol->modele());
        IOLchk              ->setChecked(!nwiol->isactif());
    }

    //! Constante a et ACD
    QHBoxLayout *csteIOLLay         = new QHBoxLayout();
    UpLabel* lblcsteAIOL            = new UpLabel;
    lblcsteAIOL                     ->setText(tr("cst. A"));
    UpLabel* lblAOptIOL             = new UpLabel;
    lblAOptIOL                      ->setText(tr("Optique"));
    UpLabel* lblAEchoIOL            = new UpLabel;
    lblAEchoIOL                     ->setText(tr("Echo"));
    UpLabel* lblACDIOL              = new UpLabel;
    lblACDIOL                       ->setText(tr("ACD"));
    UpDoubleSpinBox *AoptIOLspinbox = new UpDoubleSpinBox();
    AoptIOLspinbox                  ->setRange(116.0, 121.00);
    AoptIOLspinbox                  ->setSingleStep(0.1);
    AoptIOLspinbox                  ->setValue(118.5);
    AoptIOLspinbox                  ->setDecimals(1);
    UpDoubleSpinBox *AechoIOLspinbox= new UpDoubleSpinBox();
    AechoIOLspinbox                 ->setRange(116.0, 121.00);
    AechoIOLspinbox                 ->setSingleStep(0.1);
    AechoIOLspinbox                 ->setValue(118.5);
    AechoIOLspinbox                 ->setDecimals(1);
    UpDoubleSpinBox *ACDIOLspinbox  = new UpDoubleSpinBox();
    ACDIOLspinbox                   ->setRange(0.0, 8.00);
    ACDIOLspinbox                   ->setSingleStep(0.01);
    ACDIOLspinbox                   ->setValue(4.00);
    csteIOLLay                      ->addWidget(lblcsteAIOL);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(lblAOptIOL);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(AoptIOLspinbox);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(lblAEchoIOL);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(AechoIOLspinbox);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(lblACDIOL);
    csteIOLLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    csteIOLLay                      ->addWidget(ACDIOLspinbox);
    csteIOLLay                      ->setSpacing(5);
    csteIOLLay                      ->setContentsMargins(0,0,0,0);

    //! Haigis
    QHBoxLayout *HaigisLay          = new QHBoxLayout();
    UpLabel* HaigisIOL              = new UpLabel;
    HaigisIOL                       ->setText("Haigis");
    UpLabel* HaigisaIOL             = new UpLabel;
    HaigisaIOL                      ->setText("a");
    UpLabel* HaigisbIOL             = new UpLabel;
    HaigisbIOL                      ->setText("b");
    UpLabel* HaigiscIOL             = new UpLabel;
    HaigiscIOL                      ->setText(tr("c"));
    UpDoubleSpinBox *aIOLspinbox    = new UpDoubleSpinBox();
    aIOLspinbox                     ->setRange(-1.0000, 1.0000);
    aIOLspinbox                     ->setValue(0);
    aIOLspinbox                     ->setSingleStep(0.0001);
    aIOLspinbox                     ->setDecimals(4);
    UpDoubleSpinBox *bIOLspinbox    = new UpDoubleSpinBox();
    bIOLspinbox                     ->setRange(-1.0000, 1.0000);
    bIOLspinbox                     ->setValue(0);
    aIOLspinbox                     ->setSingleStep(0.0001);
    bIOLspinbox                     ->setDecimals(4);
    UpDoubleSpinBox *cIOLspinbox    = new UpDoubleSpinBox();
    cIOLspinbox                     ->setRange(-1.0000, 1.0000);
    cIOLspinbox                     ->setValue(0);
    aIOLspinbox                     ->setSingleStep(0.0001);
    cIOLspinbox                     ->setDecimals(4);
    HaigisLay                      ->addWidget(HaigisIOL);
    HaigisLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                      ->addWidget(HaigisaIOL);
    HaigisLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                      ->addWidget(aIOLspinbox);
    HaigisLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                      ->addWidget(HaigisbIOL);
    HaigisLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                      ->addWidget(bIOLspinbox);
    HaigisLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                      ->addWidget(HaigiscIOL);
    HaigisLay                      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    HaigisLay                      ->addWidget(cIOLspinbox);
    HaigisLay                      ->setSpacing(5);
    HaigisLay                      ->setContentsMargins(0,0,0,0);

    //! Materiau - Image
    QVBoxLayout *MateriauLay    = new QVBoxLayout();
    QHBoxLayout *MateriauImgLay = new QHBoxLayout();
    UpLabel* Materiaulbl        = new UpLabel;
    Materiaulbl                 ->setText(tr("Materiau"));
    UpLabel* ImgIOL             = new UpLabel;
    ImgIOL                      ->setPixmap(Icons::pxIOL());
    //ImgIOL                      ->setFixedSize(QSize(50,50));
    UpLineEdit *Materiauline    = new UpLineEdit();
    MateriauLay                 ->insertWidget(0,Materiauline);
    MateriauLay                 ->insertWidget(0,Materiaulbl);
    MateriauLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    MateriauImgLay              ->addLayout(MateriauLay);
    MateriauImgLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    MateriauImgLay              ->addWidget(ImgIOL);
    MateriauImgLay              ->setContentsMargins(0,0,0,0);

    //! Remarque
    QHBoxLayout *remarqueLay    = new QHBoxLayout();
    QVBoxLayout *RemarqueHLay   = new QVBoxLayout();
    UpLabel* Remarquelbl        = new UpLabel;
    Remarquelbl                 ->setText(tr("Remarque"));
    UpTextEdit* Remarquetxt     = new UpTextEdit();
    remarqueLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    remarqueLay                 ->insertWidget(0,Remarquelbl);
    RemarqueHLay                ->addLayout(remarqueLay);
    RemarqueHLay                ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    RemarqueHLay                ->addWidget(Remarquetxt);
    RemarqueHLay                ->setContentsMargins(0,0,0,0);


    dlg_IOL->dlglayout()   ->insertLayout(0, RemarqueHLay);
    dlg_IOL->dlglayout()   ->insertLayout(0, MateriauImgLay);
    dlg_IOL->dlglayout()   ->insertLayout(0, HaigisLay);
    dlg_IOL->dlglayout()   ->insertLayout(0, csteIOLLay);
    dlg_IOL->dlglayout()   ->insertLayout(0, choixIOLLay);
    dlg_IOL->dlglayout()   ->insertLayout(0, choixManufacturerIOLLay);
    dlg_IOL->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_IOL->dlglayout()   ->setSpacing(5);
    dlg_IOL->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_IOL->OKButton, &QPushButton::clicked, dlg_IOL, [&]
    {
        QString modele = IOLline->text();
        int idmanufacturer = manufacturercombo->currentData().toInt();
        foreach(IOL *iol, Datas::I()->iols->iols()->values())
        {
            if (nwiol == Q_NULLPTR)
            {
                if (iol->modele() == modele)
                {
                    UpMessageBox::Watch(this, tr("Cet implant existe déjà!"));
                    return;
                }
            }
            else
                if (iol->modele() == modele && iol->id()!= nwiol->id())
                {
                    UpMessageBox::Watch(this, tr("Cet implant existe déjà!"));
                    return;
                }
        }
        QHash<QString, QVariant> listbinds;
        listbinds[CP_MODELNAME_IOLS]    = modele;
        listbinds[CP_IDMANUFACTURER_IOLS]  = idmanufacturer;
        listbinds[CP_INACTIF_IOLS]  = (IOLchk->isChecked()? "1" : QVariant());
        if (nwiol == Q_NULLPTR)
            nwiol =  Datas::I()->iols->CreationIOL(listbinds);
        else
        {
            DataBase::I()->UpdateTable(TBL_IOLS, listbinds, "where " CP_ID_IOLS " = " + QString::number(nwiol->id()));
            nwiol->setidmanufacturer(idmanufacturer);
            nwiol->setmodele(modele);
            bool a = !IOLchk->isChecked();
            nwiol->setactif(a);
        }
        m_listemodifiee = true;
        ReconstruitTreeViewIOLs();
        for (int i=0; i<m_IOLsmodel->rowCount(); ++i)
        {
            UpStandardItem *manitem = dynamic_cast<UpStandardItem*>(m_IOLsmodel->item(i));
            if (manitem != Q_NULLPTR)
                if (manitem->item() != Q_NULLPTR)
                    if (manitem->item()->id() == nwiol->idmanufacturer() && manitem->hasChildren())
                        for (int k=0; k < manitem->rowCount(); ++k)
                        {
                            UpStandardItem *iolitem = dynamic_cast<UpStandardItem*>(manitem->child(k));
                            if (iolitem != Q_NULLPTR)
                                if (iolitem->item() != Q_NULLPTR)
                                    if (iolitem->item()->id() == nwiol->id())
                                    {
                                        wdg_iolstree->scrollTo(iolitem->index(), QAbstractItemView::PositionAtCenter);
                                        dlg_IOL->close();
                                        return;
                                    }
                        }
        }
        dlg_IOL->close();
    });
    dlg_IOL->exec();
}

// ------------------------------------------------------------------------------------------
// renvoie l'IOL correspondant à l'index
// ------------------------------------------------------------------------------------------
IOL* dlg_listeiols::getIOLFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_IOLsmodel->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return dynamic_cast<IOL *>(it->item());
    else
        return Q_NULLPTR;
}


// ------------------------------------------------------------------------------------------
// Modifie un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::ModifIOL(IOL *iol)
{
    if (iol == Q_NULLPTR)
        return;
    FicheIOL(iol);
}

// ------------------------------------------------------------------------------------------
// Supprime un correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::SupprIOL()
{
    if (wdg_iolstree->selectionModel()->selectedIndexes().size() == 0) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer la fiche") + "\n " +
            m_IOLsmodel->itemFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0))->text() + "?" +
            "\n" + tr("La suppression de cette fiche est IRRÉVERSIBLE.");
    UpMessageBox msgbox;
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
        Datas::I()->iols->SupprimeIOL(getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0)));
        m_listemodifiee = true;
        ReconstruitTreeViewIOLs(true);
    }
}

void dlg_listeiols::ReconstruitListeManufacturers()
{
    if (m_manufacturersmodel == Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);

    QStringList list;
    UpStandardItem *manufactureritem;
    foreach(IOL *iol, Datas::I()->iols->iols()->values())
    {
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != Q_NULLPTR)
        {
            QString fabricant  = man->nom();
            if (!list.contains(fabricant))
            {
                list << fabricant;
                manufactureritem  = new UpStandardItem(fabricant, man);
                manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                manufactureritem  ->setEditable(false);
                manufactureritem  ->setEnabled(false);
                m_manufacturersmodel->appendRow(manufactureritem);
            }
        }
    }
    m_manufacturersmodel->sort(0);
}

void dlg_listeiols::ReconstruitTreeViewIOLs(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
    {
        Datas::I()->iols->initListe();
        ReconstruitListeManufacturers();
    }
    wdg_iolstree->disconnect();
    if (m_IOLsmodel == Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);

    UpStandardItem *pitem;
    for (int i=0; i<  m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != Q_NULLPTR)
        {
            Manufacturer *man = dynamic_cast<Manufacturer*>(itm->item());
            if (man != Q_NULLPTR)
            {
                UpStandardItem *manufactureritem = new UpStandardItem(man->nom(), man);
                manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                manufactureritem  ->setEditable(false);
                manufactureritem  ->setEnabled(false);
                m_IOLsmodel->appendRow(manufactureritem);
            }
        }
        m_IOLsmodel->sort(0);
    }
    foreach(IOL *iol, Datas::I()->iols->iols()->values())
    {
        if (iol->modele().startsWith(filtre))
        {
            pitem   = new UpStandardItem(iol->modele(), iol);
            if (!iol->isactif())
                pitem ->setForeground(QBrush(QColor(Qt::darkGray)));
            pitem   ->setEditable(false);
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
    wdg_iolstree     ->setModel(m_IOLsmodel);
    wdg_iolstree     ->expandAll();
    if (m_IOLsmodel->rowCount()>0)
    {
        m_IOLsmodel->sort(0);
        m_IOLsmodel->sort(1);
        connect(wdg_iolstree,    &QAbstractItemView::pressed,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_iolstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}
