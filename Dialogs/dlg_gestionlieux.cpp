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

dlg_GestionLieux::dlg_GestionLieux(QWidget *parent)  : UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionLieux", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    db              = DataBase::I();
    AjouteLayButtons(UpDialog::ButtonClose);
    connect(CloseButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    QVBoxLayout *lay = dynamic_cast<QVBoxLayout*>(layout());
    tabLM = new QTableView(this);
    Adressuplbl = new UpLabel();
    Adressuplbl->setFixedWidth(240);
    if (Datas::I()->sites->sites()->size() == 0)
        Datas::I()->sites->initListe();
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
    hlay    ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed));
    hlay    ->addLayout(vlay);
    lay     ->insertLayout(0,hlay);
    lay     ->setSizeConstraint(QLayout::SetFixedSize);

    connect(widg,   SIGNAL(choix(int)), this,   SLOT(Slot_ChoixButtonFrame(int)));
}

dlg_GestionLieux::~dlg_GestionLieux()
{
}

void dlg_GestionLieux::Slot_AfficheDetails(QModelIndex idx, QModelIndex)
{
    Site * sit = getSiteFromIndex(idx);
    if (sit == Q_NULLPTR)
    {
        widg->moinsBouton->setEnabled(false);
        return;
    }
    QString data;
    if (sit->adresse1() != "")
        data = sit->adresse1();
    if (sit->adresse2() != "")
    {
        if (data !="")
            data += "\n";
        data += sit->adresse2();
    }
    if (sit->adresse3() != "")
    {
        if (data !="")
            data += "\n";
        data += sit->adresse3();
    }
    if( sit->codePostal()>0 || sit->ville() != "")
    {
        if (data !="")
            data += "\n";
        if( sit->codePostal()>0)
            data += QString::number(sit->codePostal());
        if (sit->ville() != "" && sit->codePostal()>0)
            data += " ";
        data += sit->ville();
    }
    if (sit->telephone() != "")
    {
        if (data !="")
            data += "\n";
        data += "Tel: " + sit->telephone();
    }
    if (sit->fax() != "")
    {
        if (data !="")
            data += "\n";
        data += "Tel: " + sit->fax();
    }
    Adressuplbl->setText(data);
    QString ttip = "";
    int nlieux = db->StandardSelectSQL("select iduser from " TBL_JOINTURESLIEUX " where idlieu = " + QString::number(sit->id()), ok).size();
    if (nlieux == 0)
        nlieux = db->StandardSelectSQL("select idlieu from " TBL_ACTES " where idlieu = " + QString::number(sit->id()), ok).size();
    if (nlieux == 0)
    {
        if (sit->id() == idlieuserveur)
            ttip = tr("Vous ne pouvez pas supprimer ce site car il héberge le serveur") + "\n"
                    + tr ("Pour modifier ce paramètre, modifier le choix dans la box \"Emplacement du serveur\"") + "\n"
                    + tr ("Menu Edition/Paramètres - Onglet Général");
    }
    if (nlieux > 0)
        ttip = tr("Vous ne pouvez pas supprimer ce site car il est utilisé");
    widg->moinsBouton->setEnabled(ttip == "");
    widg->moinsBouton->setImmediateToolTip(ttip, true);
}

Site* dlg_GestionLieux::getSiteFromIndex(QModelIndex idx)
{
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(tabModel->itemFromIndex(idx));
    if (upitem == Q_NULLPTR)
        return Q_NULLPTR;
    Site *sit = dynamic_cast<Site *>(upitem->item());
    return sit;
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
        QString req = "insert into " TBL_LIEUXEXERCICE "(NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax)  values("
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
        Datas::I()->sites->initListe();
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
    Site * sit = getSiteFromIndex(tabModel->index(tabLM->currentIndex().row(),0));
    if (sit == Q_NULLPTR)
        return;
    leditnom    ->setText(sit->nom());
    leditadr1   ->setText(sit->adresse1());
    leditadr2   ->setText(sit->adresse2());
    leditadr3   ->setText(sit->adresse3());
    leditcp     ->setText(QString::number(sit->codePostal()));
    leditville  ->setText(sit->ville());
    ledittel    ->setText(sit->telephone());
    leditfax    ->setText(sit->fax());
    connect(gLieuDialog->OKButton, SIGNAL(clicked(bool)), this, SLOT(Slot_ModifLieu()));
    gLieuDialog->exec();
    delete  gLieuDialog;
}

void dlg_GestionLieux::Slot_ModifLieu()
{

    if (ValidationFiche())
    {
        QString req = "update " TBL_LIEUXEXERCICE " set "
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
        Datas::I()->sites->initListe();
        ReconstruitModel();
        gLieuDialog->accept();
        connect(tabLM->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        tabLM->selectRow(0);
    }
}

void dlg_GestionLieux::SupprLieu()
{
    Site * sit = getSiteFromIndex(tabModel->index(tabLM->currentIndex().row(),0));
    if (sit == Q_NULLPTR)
        return;
    int idLieuASupprimer = sit->id();
    QString lieu = sit->nom();
    if (UpMessageBox::Question(this,tr("Suppression d'un lieu de soins"),tr("voulez vous vraiment supprimer") + "\n" + lieu + " ?") == UpSmallButton::STARTBUTTON)
    {
        db->SupprRecordFromTable(idLieuASupprimer, "idLieu", TBL_LIEUXEXERCICE);
        dlg_message(QStringList() << lieu + " supprimé", 3000);
        Datas::I()->sites->initListe();
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
    QMap<int, Site*> *listsites = Datas::I()->sites->sites();
    tabModel = dynamic_cast<QStandardItemModel*>(tabLM->model());
    if (tabModel != Q_NULLPTR)
        tabModel->clear();
    else
        tabModel = new QStandardItemModel;

    for( auto itsite = listsites->cbegin(); itsite != listsites->cend(); ++itsite )
    {
        Site *sit = const_cast<Site*>(itsite.value());
        UpStandardItem *pitem0 = new UpStandardItem(sit->nom()==""? tr("non défini") : sit->nom());
        pitem0->setItem(sit);
        tabModel->appendRow(QList<QStandardItem*>() << pitem0);
    }
    tabLM->setModel(tabModel);

    tabModel->setHeaderData(0, Qt::Horizontal, tr("Structure de soins"));
    tabLM->setColumnWidth(0,240);       // NomLieu
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
    idlieuserveur = -1;
    idlieuserveur = db->parametres()->idlieupardefaut();
}

