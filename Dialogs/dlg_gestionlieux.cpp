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

#include "dlg_gestionlieux.h"
#include "utils.h"

dlg_GestionLieux::dlg_GestionLieux(QWidget *parent)  : UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionLieux", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    db              = DataBase::getInstance();
    AjouteLayButtons(UpDialog::ButtonClose);
    connect(CloseButton, SIGNAL(clicked(bool)), this, SLOT(Slot_EnregLieux()));

    QVBoxLayout *lay = dynamic_cast<QVBoxLayout*>(layout());
    tabLM = new QTableView(this);
    Adressuplbl = new UpLabel();
    Adressuplbl->setFixedWidth(240);

    ReconstruitModel();
    widg = new WidgetButtonFrame(tabLM);
    widg->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect(tabLM->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
    tabLM->selectRow(0);

    QVBoxLayout *vlay   = new QVBoxLayout();
    QHBoxLayout *hlay   = new QHBoxLayout();
    vlay    ->addWidget(Adressuplbl);
    vlay    ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));
    hlay    ->addWidget(widg->widgButtonParent());
    hlay    ->addLayout(vlay);
    lay     ->insertLayout(0,hlay);
    lay     ->setSizeConstraint(QLayout::SetFixedSize);

    connect(widg,                       SIGNAL(choix(int)),                                 this,   SLOT(Slot_ChoixButtonFrame(int)));
}

dlg_GestionLieux::~dlg_GestionLieux()
{
}

void dlg_GestionLieux::Slot_AfficheDetails(QModelIndex idx, QModelIndex)
{
    int row = idx.row();
    QString data ("");
    for (int i=2; i<9; i++)
    {
        QString datasuiv = tabModel->itemData(tabModel->index(row,i)).value(0).toString();
        switch (i) {
        case 2:
        case 3:
        case 4:
        case 5:
            if (data != "")
                if (datasuiv != "")
                    data += "\n";
            data += datasuiv;
            break;
        case 6:
            if (data != "")
                if (datasuiv != "")
                {
                    if (tabModel->itemData(tabModel->index(row,5)).value(0).toString() != "")
                        data += " ";
                    else
                        data += "\n";
                }
            data += datasuiv;
            break;
        case 7:
            if (data != "")
                if (datasuiv != "")
                    data += "\n";
            if (datasuiv != "")
                data += "Tel: " + datasuiv;
            break;
        case 8:
            if (data != "")
                if (datasuiv != "")
                    data += "\n";
            if (datasuiv != "")
                data += "Fax: " + datasuiv;
            break;
        default:
            break;
        }
    }
    Adressuplbl->setText(data);
    widg->moinsBouton->setEnabled(db->StandardSelectSQL("select iduser from " NOM_TABLE_JOINTURESLIEUX " where idlieu = " + tabModel->itemData(tabModel->index(row,0)).value(0).toString(), ok).size() == 0
                                  && tabModel->itemData(tabModel->index(row,0)).value(0).toInt() != idlieuserveur);
}

void dlg_GestionLieux::Slot_ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        CreerLieu();
        break;
    case 0:
        ModifLieu();
        break;
    case -1:
        SupprLieu();
        break;
    default:
        break;
    }
}

void dlg_GestionLieux::Slot_EnregLieux()
{

    reject();
}

void dlg_GestionLieux::CreerLieu()
{
    ModifLieuxDialog();
    connect(gLieuDialog->OKButton, SIGNAL(clicked(bool)), this, SLOT(Slot_EnregNouvLieu()));
    gLieuDialog->exec();
    delete  gLieuDialog;
}

void dlg_GestionLieux::Slot_EnableOKButton()
{
    gLieuDialog->OKButton->setEnabled(true);
}

void dlg_GestionLieux::Slot_EnregNouvLieu()
{
    if (ValidationFiche())
    {
        QString req = "insert into " NOM_TABLE_LIEUXEXERCICE "(NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax)  values("
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(leditnom->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(leditadr1->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(leditadr2->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(leditadr3->text())) + "', "
                        ""  + Utils::correctquoteSQL(Utils::trimcapitilize(leditcp->text())) + ", "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(leditville->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(ledittel->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(leditfax->text())) + "')";
        //qDebug() << req;
        db->StandardSQL(req);
        delete tabModel;
        ReconstruitModel();
        gLieuDialog->accept();
        connect(tabLM->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        tabLM->selectRow(0);
    }
}

void dlg_GestionLieux::ModifLieuxDialog()
{
    gLieuDialog = new UpDialog(this);
    gLieuDialog->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    gLieuDialog->setWindowTitle(tr("Enregistrer un nouveau lieu"));
    QVBoxLayout *laylbl = new QVBoxLayout();
    QVBoxLayout *layledit = new QVBoxLayout();
    QHBoxLayout *laycom = new QHBoxLayout();
    QVBoxLayout *lay = dynamic_cast<QVBoxLayout*>(gLieuDialog->layout());

    UpLabel *lblnom = new UpLabel(gLieuDialog, tr("Nom de la structure"));
    UpLabel *lbladr1 = new UpLabel(gLieuDialog, tr("Adresse1"));
    UpLabel *lbladr2 = new UpLabel(gLieuDialog, tr("Adresse2"));
    UpLabel *lbladr3 = new UpLabel(gLieuDialog, tr("Adresse3"));
    UpLabel *lblcp = new UpLabel(gLieuDialog, tr("Code postal"));
    UpLabel *lblville = new UpLabel(gLieuDialog, tr("Ville"));
    UpLabel *lbltel = new UpLabel(gLieuDialog, tr("Telephone"));
    UpLabel *lblfax = new UpLabel(gLieuDialog, tr("Fax"));

    int h = 22;
    lblnom  ->setFixedHeight(h);
    lbladr1 ->setFixedHeight(h);
    lbladr2 ->setFixedHeight(h);
    lbladr3 ->setFixedHeight(h);
    lblcp   ->setFixedHeight(h);
    lblville->setFixedHeight(h);
    lbltel  ->setFixedHeight(h);
    lblfax  ->setFixedHeight(h);

    laylbl->addWidget(lblnom);
    laylbl->addWidget(lbladr1);
    laylbl->addWidget(lbladr2);
    laylbl->addWidget(lbladr3);
    laylbl->addWidget(lblcp);
    laylbl->addWidget(lblville);
    laylbl->addWidget(lbltel);
    laylbl->addWidget(lblfax);
    laylbl->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    leditnom   = new UpLineEdit(gLieuDialog);
    leditadr1  = new UpLineEdit(gLieuDialog);
    leditadr2  = new UpLineEdit(gLieuDialog);
    leditadr3  = new UpLineEdit(gLieuDialog);
    leditcp    = new UpLineEdit(gLieuDialog);
    leditville = new UpLineEdit(gLieuDialog);
    ledittel   = new UpLineEdit(gLieuDialog);
    leditfax   = new UpLineEdit(gLieuDialog);

    leditnom    ->setFixedWidth(240);       // NomLieu
    leditadr1   ->setFixedWidth(240);       // Adresse1
    leditadr2   ->setFixedWidth(240);       // Adresse2
    leditadr3   ->setFixedWidth(240);       // Adresse3
    leditcp     ->setFixedWidth(90);        // CP
    leditville  ->setFixedWidth(240);       // Ville
    ledittel    ->setFixedWidth(120);       // Telephone
    leditfax    ->setFixedWidth(120);       // Fax

    leditnom    ->setMaxLength(80);
    leditadr1   ->setMaxLength(45);
    leditadr2   ->setMaxLength(45);
    leditadr3   ->setMaxLength(45);
    leditcp     ->setMaxLength(9);
    leditville  ->setMaxLength(45);
    ledittel    ->setMaxLength(17);
    leditfax    ->setMaxLength(17);

    leditnom    ->setValidator(new QRegExpValidator(Utils::rgx_ville));
    leditadr1   ->setValidator(new QRegExpValidator(Utils::rgx_adresse));
    leditadr2   ->setValidator(new QRegExpValidator(Utils::rgx_adresse));
    leditadr3   ->setValidator(new QRegExpValidator(Utils::rgx_adresse));
    leditcp     ->setValidator(new QRegExpValidator(Utils::rgx_CP));
    leditville  ->setValidator(new QRegExpValidator(Utils::rgx_ville));
    ledittel    ->setValidator(new QRegExpValidator(Utils::rgx_telephone));
    leditfax    ->setValidator(new QRegExpValidator(Utils::rgx_telephone));

    layledit->addWidget(leditnom);
    layledit->addWidget(leditadr1);
    layledit->addWidget(leditadr2);
    layledit->addWidget(leditadr3);
    layledit->addWidget(leditcp);
    layledit->addWidget(leditville);
    layledit->addWidget(ledittel);
    layledit->addWidget(leditfax);
    layledit->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    for (int i=0; i< gLieuDialog->findChildren<UpLineEdit*>().size(); i++)
        connect(gLieuDialog->findChildren<UpLineEdit*>().at(i), SIGNAL(textEdited(QString)), this, SLOT(Slot_EnableOKButton()));
    gLieuDialog->OKButton->setEnabled(false);

    laycom->addLayout(laylbl);
    laylbl->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed));
    laycom->addLayout(layledit);

    lay     ->insertLayout(0,laycom);
    lay     ->setSizeConstraint(QLayout::SetFixedSize);
}

void dlg_GestionLieux::ModifLieu()
{
    ModifLieuxDialog();
    idLieuAModifier = tabModel->itemData(tabModel->index(tabLM->currentIndex().row(),0)).value(0).toInt();
    leditnom    ->setText(tr("non défini"));
    QString req = "select idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax from " NOM_TABLE_LIEUXEXERCICE
                  " where idLieu = " + QString::number(idLieuAModifier);
    //qDebug() << req;
    QVariantList lieudata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && lieudata.size()>0)
        leditnom    ->setText(lieudata.at(1).toString());
    leditadr1   ->setText(lieudata.at(2).toString());
    leditadr2   ->setText(lieudata.at(3).toString());
    leditadr3   ->setText(lieudata.at(4).toString());
    leditcp     ->setText(lieudata.at(5).toString());
    leditville  ->setText(lieudata.at(6).toString());
    ledittel    ->setText(lieudata.at(7).toString());
    leditfax    ->setText(lieudata.at(8).toString());
    connect(gLieuDialog->OKButton, SIGNAL(clicked(bool)), this, SLOT(Slot_ModifLieu()));
    gLieuDialog->exec();
    delete  gLieuDialog;
}

void dlg_GestionLieux::Slot_ModifLieu()
{

    if (ValidationFiche())
    {
        QString req = "update " NOM_TABLE_LIEUXEXERCICE " set "
                        "NomLieu = '"       + Utils::correctquoteSQL(Utils::trimcapitilize(leditnom->text())) + "', "
                        "LieuAdresse1 = '"  + Utils::correctquoteSQL(Utils::trimcapitilize(leditadr1->text())) + "', "
                        "LieuAdresse2 = '"  + Utils::correctquoteSQL(Utils::trimcapitilize(leditadr2->text())) + "', "
                        "LieuAdresse3 = '"  + Utils::correctquoteSQL(Utils::trimcapitilize(leditadr3->text())) + "', "
                        "LieuCodePostal = " + Utils::correctquoteSQL(Utils::trimcapitilize(leditcp->text())) + ", "
                        "LieuVille = '"     + Utils::correctquoteSQL(Utils::trimcapitilize(leditville->text())) + "', "
                        "LieuTelephone = '" + Utils::correctquoteSQL(Utils::trimcapitilize(ledittel->text())) + "', "
                        "LieuFax = '"       + Utils::correctquoteSQL(Utils::trimcapitilize(leditfax->text())) + "' " +
                        "where idLieu = "   + QString::number(idLieuAModifier);
        //qDebug() << req;
        db->StandardSQL(req);
        delete tabModel;
        ReconstruitModel();
        gLieuDialog->accept();
        connect(tabLM->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        tabLM->selectRow(0);
    }
}

void dlg_GestionLieux::SupprLieu()
{
    int idLieuASupprimer = tabModel->itemData(tabModel->index(tabLM->currentIndex().row(),0)).value(0).toInt();
    QString lieu = tabModel->itemData(tabModel->index(tabLM->currentIndex().row(),1)).value(0).toString();
    if (UpMessageBox::Question(this,tr("Suppression d'un lieu de soins"),tr("voulez vous vraiment supprimer") + "\n" + lieu + " ?") == UpSmallButton::STARTBUTTON)
    {
        db->SupprRecordFromTable(idLieuASupprimer, "idLieu", NOM_TABLE_LIEUXEXERCICE);
        ReconstruitModel();
        dlg_message(QStringList() << lieu + " supprimé", 3000);
        ReconstruitModel();
        connect(tabLM->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        tabLM->selectRow(0);
    }
}

bool dlg_GestionLieux::ValidationFiche()
{
    QString Msg = tr("Vous n'avez pas spécifié ");
    if (leditnom->text() == QString() || leditnom->text() == tr("non défini"))
    {
        Msg += tr("le nom de la structure de soins");
        leditnom->setFocus();
        UpMessageBox::Watch(this,Msg);
        return false;
    }
    if (leditcp->text().toInt() ==  0)
    {
        Msg += tr("le code postal");
        leditcp->setFocus();
        UpMessageBox::Watch(this,Msg);
        return false;
    }
    if (leditville->text() == QString())
    {
        Msg += tr("la ville");
        leditville->setFocus();
        UpMessageBox::Watch(this,Msg);
        return false;
    }
    return true;
}

void dlg_GestionLieux::ReconstruitModel()
{
    tabModel = new QStandardItemModel();
    tabModel->deleteLater();

    QStandardItem *pitem0;
    QStandardItem *pitem1;
    QStandardItem *pitem2;
    QStandardItem *pitem3;
    QStandardItem *pitem4;
    QStandardItem *pitem5;
    QStandardItem *pitem6;
    QStandardItem *pitem7;
    QStandardItem *pitem8;

    QList<QVariantList> listlieux = db->StandardSelectSQL("select idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax from " NOM_TABLE_LIEUXEXERCICE,ok);
    if(!ok)
        return;
    for (int i=0; i<listlieux.size(); i++)
    {
        pitem0 = new QStandardItem(listlieux.at(i).at(0).toString());
        pitem1 = new QStandardItem(listlieux.at(i).at(1).toString()==""? tr("non défini") : listlieux.at(i).at(1).toString());
        pitem2 = new QStandardItem(listlieux.at(i).at(2).toString());
        pitem3 = new QStandardItem(listlieux.at(i).at(3).toString());
        pitem4 = new QStandardItem(listlieux.at(i).at(4).toString());
        pitem5 = new QStandardItem(listlieux.at(i).at(5).toString());
        pitem6 = new QStandardItem(listlieux.at(i).at(6).toString());
        pitem7 = new QStandardItem(listlieux.at(i).at(7).toString());
        pitem8 = new QStandardItem(listlieux.at(i).at(8).toString());
        tabModel->appendRow(QList<QStandardItem*>() << pitem0 << pitem1 << pitem2 << pitem3 << pitem4 << pitem5 << pitem6 << pitem7 << pitem8);
    }

    tabLM->setModel(tabModel);

    tabModel->setHeaderData(0, Qt::Horizontal, tr("idLieu"));
    tabModel->setHeaderData(1, Qt::Horizontal, tr("Structure de soins"));
    tabModel->setHeaderData(2, Qt::Horizontal, tr("Adresse1"));
    tabModel->setHeaderData(3, Qt::Horizontal, tr("Adresse2"));
    tabModel->setHeaderData(4, Qt::Horizontal, tr("Adresse3"));
    tabModel->setHeaderData(5, Qt::Horizontal, tr("Code postal"));
    tabModel->setHeaderData(6, Qt::Horizontal, tr("Ville"));
    tabModel->setHeaderData(7, Qt::Horizontal, tr("Telephone"));
    tabModel->setHeaderData(8, Qt::Horizontal, tr("Fax"));

    tabLM->setColumnWidth(0,1);         // idLieu
    tabLM->setColumnWidth(1,240);       // NomLieu
    tabLM->setColumnWidth(2,240);       // Adresse1
    tabLM->setColumnWidth(3,240);       // Adresse2
    tabLM->setColumnWidth(4,240);       // Adresse3
    tabLM->setColumnWidth(5,90);        // CP
    tabLM->setColumnWidth(6,240);       // Ville
    tabLM->setColumnWidth(7,120);       // Telephone
    tabLM->setColumnWidth(8,120);       // Fax
    tabLM->hideColumn(0);
    for (int i=2; i<9; i++)
        tabLM->setColumnHidden(i, true);
    tabLM->verticalHeader()->setVisible(false);
    tabLM->setFocusPolicy(Qt::StrongFocus);
    tabLM->setGridStyle(Qt::NoPen);
    tabLM->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabLM->setSelectionMode(QAbstractItemView::SingleSelection);
    tabLM->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabLM->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    tabLM->setStyleSheet("QTableView {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");

    int larg = 0;
    for (int i=0; i < tabModel->columnCount(); i++)
        if (!tabLM->isColumnHidden(i))
            larg += tabLM->columnWidth(i);
    tabLM   ->setFixedWidth(larg+2);
    int h = int(QFontMetrics(qApp->font()).height()*1.1);
    for (int i=0; i < tabModel->rowCount(); i++)
        tabLM->setRowHeight(i, h);
    bool ok;
    idlieuserveur = -1;
    QVariantList serveurLieudata = db->getFirstRecordFromStandardSelectSQL("select idlieupardefaut from " NOM_TABLE_PARAMSYSTEME, ok);
    if (ok && serveurLieudata.size()>0)
        idlieuserveur = serveurLieudata.at(0).toInt();
}

