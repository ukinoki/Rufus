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
    AjouteLayButtons(UpDialog::ButtonClose);
    connect(CloseButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    QVBoxLayout *lay = dynamic_cast<QVBoxLayout*>(layout());
    wdg_bigtable = new QTableView(this);
    wdg_adressuplbl = new UpLabel();
    wdg_adressuplbl->setFixedWidth(240);
    if (Datas::I()->sites->sites()->size() == 0)
        Datas::I()->sites->initListe();
    ReconstruitModel();
    wdg_buttonframe = new WidgetButtonFrame(wdg_bigtable);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect(wdg_bigtable->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
    wdg_bigtable->selectRow(0);

    QVBoxLayout *vlay   = new QVBoxLayout();
    QHBoxLayout *hlay   = new QHBoxLayout();
    vlay    ->addWidget(wdg_adressuplbl);
    vlay    ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));
    hlay    ->addWidget(wdg_buttonframe->widgButtonParent());
    hlay    ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed));
    hlay    ->addLayout(vlay);
    lay     ->insertLayout(0,hlay);
    lay     ->setSizeConstraint(QLayout::SetFixedSize);

    connect(wdg_buttonframe,   SIGNAL(choix(int)), this,   SLOT(Slot_ChoixButtonFrame(int)));
}

dlg_GestionLieux::~dlg_GestionLieux()
{
}

void dlg_GestionLieux::Slot_AfficheDetails(QModelIndex idx, QModelIndex)
{
    Site * sit = getSiteFromIndex(idx);
    if (sit == Q_NULLPTR)
    {
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
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
    wdg_adressuplbl->setText(data);
    QString ttip = "";
    int nlieux = db->StandardSelectSQL("select iduser from " TBL_JOINTURESLIEUX " where idlieu = " + QString::number(sit->id()), m_ok).size();
    if (nlieux == 0)
        nlieux = db->StandardSelectSQL("select idlieu from " TBL_ACTES " where idlieu = " + QString::number(sit->id()), m_ok).size();
    if (nlieux == 0)
    {
        if (sit->id() == m_idlieuserveur)
            ttip = tr("Vous ne pouvez pas supprimer ce site car il héberge le serveur") + "\n"
                    + tr ("Pour modifier ce paramètre, modifier le choix dans la box \"Emplacement du serveur\"") + "\n"
                    + tr ("Menu Edition/Paramètres - Onglet Général");
    }
    if (nlieux > 0)
        ttip = tr("Vous ne pouvez pas supprimer ce site car il est utilisé");
    wdg_buttonframe->wdg_moinsBouton->setEnabled(ttip == "");
    wdg_buttonframe->wdg_moinsBouton->setImmediateToolTip(ttip, true);
}

Site* dlg_GestionLieux::getSiteFromIndex(QModelIndex idx)
{
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_tabmodel->itemFromIndex(idx));
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
    connect(dlg_lieu->OKButton, SIGNAL(clicked(bool)), this, SLOT(Slot_EnregNouvLieu()));
    dlg_lieu->exec();
    delete  dlg_lieu;
}

void dlg_GestionLieux::Slot_EnableOKButton()
{
    dlg_lieu->OKButton->setEnabled(true);
}

void dlg_GestionLieux::Slot_EnregNouvLieu()
{
    if (ValidationFiche())
    {
        QString req = "insert into " TBL_LIEUXEXERCICE "(NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax)  values("
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_nomlineedit->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_adress1lineedit->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_adress2lineedit->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_adress3lineedit->text())) + "', "
                        ""  + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_CPlineedit->text())) + ", "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_villelineedit->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_tellineedit->text())) + "', "
                        "'" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_faxlineedit->text())) + "')";
        //qDebug() << req;
        db->StandardSQL(req);
        Datas::I()->sites->initListe();
        ReconstruitModel();
        dlg_lieu->accept();
        connect(wdg_bigtable->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        wdg_bigtable->selectRow(0);
    }
}

void dlg_GestionLieux::ModifLieuxDialog()
{
    dlg_lieu = new UpDialog(this);
    dlg_lieu->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_lieu->setWindowTitle(tr("Enregistrer un nouveau lieu"));
    QVBoxLayout *laylbl = new QVBoxLayout();
    QVBoxLayout *layledit = new QVBoxLayout();
    QHBoxLayout *laycom = new QHBoxLayout();
    QVBoxLayout *lay = dynamic_cast<QVBoxLayout*>(dlg_lieu->layout());

    UpLabel *lblnom = new UpLabel(dlg_lieu, tr("Nom de la structure"));
    UpLabel *lbladr1 = new UpLabel(dlg_lieu, tr("Adresse1"));
    UpLabel *lbladr2 = new UpLabel(dlg_lieu, tr("Adresse2"));
    UpLabel *lbladr3 = new UpLabel(dlg_lieu, tr("Adresse3"));
    UpLabel *lblcp = new UpLabel(dlg_lieu, tr("Code postal"));
    UpLabel *lblville = new UpLabel(dlg_lieu, tr("Ville"));
    UpLabel *lbltel = new UpLabel(dlg_lieu, tr("Telephone"));
    UpLabel *lblfax = new UpLabel(dlg_lieu, tr("Fax"));

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

    wdg_nomlineedit   = new UpLineEdit(dlg_lieu);
    wdg_adress1lineedit  = new UpLineEdit(dlg_lieu);
    wdg_adress2lineedit  = new UpLineEdit(dlg_lieu);
    wdg_adress3lineedit  = new UpLineEdit(dlg_lieu);
    wdg_CPlineedit    = new UpLineEdit(dlg_lieu);
    wdg_villelineedit = new UpLineEdit(dlg_lieu);
    wdg_tellineedit   = new UpLineEdit(dlg_lieu);
    wdg_faxlineedit   = new UpLineEdit(dlg_lieu);

    wdg_nomlineedit    ->setFixedWidth(240);       // NomLieu
    wdg_adress1lineedit   ->setFixedWidth(240);       // Adresse1
    wdg_adress2lineedit   ->setFixedWidth(240);       // Adresse2
    wdg_adress3lineedit   ->setFixedWidth(240);       // Adresse3
    wdg_CPlineedit     ->setFixedWidth(90);        // CP
    wdg_villelineedit  ->setFixedWidth(240);       // Ville
    wdg_tellineedit    ->setFixedWidth(120);       // Telephone
    wdg_faxlineedit    ->setFixedWidth(120);       // Fax

    wdg_nomlineedit    ->setMaxLength(80);
    wdg_adress1lineedit   ->setMaxLength(45);
    wdg_adress2lineedit   ->setMaxLength(45);
    wdg_adress3lineedit   ->setMaxLength(45);
    wdg_CPlineedit     ->setMaxLength(9);
    wdg_villelineedit  ->setMaxLength(45);
    wdg_tellineedit    ->setMaxLength(17);
    wdg_faxlineedit    ->setMaxLength(17);

    wdg_nomlineedit    ->setValidator(new QRegExpValidator(Utils::rgx_ville));
    wdg_adress1lineedit   ->setValidator(new QRegExpValidator(Utils::rgx_adresse));
    wdg_adress2lineedit   ->setValidator(new QRegExpValidator(Utils::rgx_adresse));
    wdg_adress3lineedit   ->setValidator(new QRegExpValidator(Utils::rgx_adresse));
    wdg_CPlineedit     ->setValidator(new QRegExpValidator(Utils::rgx_CP));
    wdg_villelineedit  ->setValidator(new QRegExpValidator(Utils::rgx_ville));
    wdg_tellineedit    ->setValidator(new QRegExpValidator(Utils::rgx_telephone));
    wdg_faxlineedit    ->setValidator(new QRegExpValidator(Utils::rgx_telephone));

    layledit->addWidget(wdg_nomlineedit);
    layledit->addWidget(wdg_adress1lineedit);
    layledit->addWidget(wdg_adress2lineedit);
    layledit->addWidget(wdg_adress3lineedit);
    layledit->addWidget(wdg_CPlineedit);
    layledit->addWidget(wdg_villelineedit);
    layledit->addWidget(wdg_tellineedit);
    layledit->addWidget(wdg_faxlineedit);
    layledit->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    for (int i=0; i< dlg_lieu->findChildren<UpLineEdit*>().size(); i++)
        connect(dlg_lieu->findChildren<UpLineEdit*>().at(i), SIGNAL(textEdited(QString)), this, SLOT(Slot_EnableOKButton()));
    dlg_lieu->OKButton->setEnabled(false);

    laycom->addLayout(laylbl);
    laylbl->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed));
    laycom->addLayout(layledit);

    lay     ->insertLayout(0,laycom);
    lay     ->setSizeConstraint(QLayout::SetFixedSize);
}

void dlg_GestionLieux::ModifLieu()
{
    ModifLieuxDialog();
    Site * sit = getSiteFromIndex(m_tabmodel->index(wdg_bigtable->currentIndex().row(),0));
    if (sit == Q_NULLPTR)
        return;
    wdg_nomlineedit    ->setText(sit->nom());
    wdg_adress1lineedit   ->setText(sit->adresse1());
    wdg_adress2lineedit   ->setText(sit->adresse2());
    wdg_adress3lineedit   ->setText(sit->adresse3());
    wdg_CPlineedit     ->setText(QString::number(sit->codePostal()));
    wdg_villelineedit  ->setText(sit->ville());
    wdg_tellineedit    ->setText(sit->telephone());
    wdg_faxlineedit    ->setText(sit->fax());
    connect(dlg_lieu->OKButton, SIGNAL(clicked(bool)), this, SLOT(Slot_ModifLieu()));
    dlg_lieu->exec();
    delete  dlg_lieu;
}

void dlg_GestionLieux::Slot_ModifLieu()
{

    if (ValidationFiche())
    {
        QString req = "update " TBL_LIEUXEXERCICE " set "
                        "NomLieu = '"       + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_nomlineedit->text())) + "', "
                        "LieuAdresse1 = '"  + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_adress1lineedit->text())) + "', "
                        "LieuAdresse2 = '"  + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_adress2lineedit->text())) + "', "
                        "LieuAdresse3 = '"  + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_adress3lineedit->text())) + "', "
                        "LieuCodePostal = " + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_CPlineedit->text())) + ", "
                        "LieuVille = '"     + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_villelineedit->text())) + "', "
                        "LieuTelephone = '" + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_tellineedit->text())) + "', "
                        "LieuFax = '"       + Utils::correctquoteSQL(Utils::trimcapitilize(wdg_faxlineedit->text())) + "' " +
                        "where idLieu = "   + QString::number(m_idlieuamodifier);
        //qDebug() << req;
        db->StandardSQL(req);
        Datas::I()->sites->initListe();
        ReconstruitModel();
        dlg_lieu->accept();
        connect(wdg_bigtable->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        wdg_bigtable->selectRow(0);
    }
}

void dlg_GestionLieux::SupprLieu()
{
    Site * sit = getSiteFromIndex(m_tabmodel->index(wdg_bigtable->currentIndex().row(),0));
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
        connect(wdg_bigtable->selectionModel(),    SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this,   SLOT(Slot_AfficheDetails(QModelIndex,QModelIndex)));
        wdg_bigtable->selectRow(0);
    }
}

bool dlg_GestionLieux::ValidationFiche()
{
    QString Msg = tr("Vous n'avez pas spécifié ");
    if (wdg_nomlineedit->text() == QString() || wdg_nomlineedit->text() == tr("non défini"))
    {
        Msg += tr("le nom de la structure de soins");
        wdg_nomlineedit->setFocus();
        UpMessageBox::Watch(this,Msg);
        return false;
    }
    if (wdg_CPlineedit->text().toInt() ==  0)
    {
        Msg += tr("le code postal");
        wdg_CPlineedit->setFocus();
        UpMessageBox::Watch(this,Msg);
        return false;
    }
    if (wdg_villelineedit->text() == QString())
    {
        Msg += tr("la ville");
        wdg_villelineedit->setFocus();
        UpMessageBox::Watch(this,Msg);
        return false;
    }
    return true;
}

void dlg_GestionLieux::ReconstruitModel()
{
    QMap<int, Site*> *listsites = Datas::I()->sites->sites();
    m_tabmodel = dynamic_cast<QStandardItemModel*>(wdg_bigtable->model());
    if (m_tabmodel != Q_NULLPTR)
        m_tabmodel->clear();
    else
        m_tabmodel = new QStandardItemModel;

    foreach (Site* sit, listsites->values() )
    {
        UpStandardItem *pitem0 = new UpStandardItem(sit->nom()==""? tr("non défini") : sit->nom());
        pitem0->setitem(sit);
        m_tabmodel->appendRow(QList<QStandardItem*>() << pitem0);
    }
    wdg_bigtable->setModel(m_tabmodel);

    m_tabmodel->setHeaderData(0, Qt::Horizontal, tr("Structure de soins"));
    wdg_bigtable->setColumnWidth(0,240);       // NomLieu
    wdg_bigtable->verticalHeader()->setVisible(false);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setGridStyle(Qt::NoPen);
    wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_bigtable->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_bigtable->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    wdg_bigtable->setStyleSheet("QTableView {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");

    int larg = 0;
    for (int i=0; i < m_tabmodel->columnCount(); i++)
        if (!wdg_bigtable->isColumnHidden(i))
            larg += wdg_bigtable->columnWidth(i);
    wdg_bigtable   ->setFixedWidth(larg+2);
    int h = int(QFontMetrics(qApp->font()).height()*1.1);
    for (int i=0; i < m_tabmodel->rowCount(); i++)
        wdg_bigtable->setRowHeight(i, h);
    m_idlieuserveur = -1;
    m_idlieuserveur = db->parametres()->idlieupardefaut();
}

