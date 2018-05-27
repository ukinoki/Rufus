/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "villecpwidget.h"
#include "ui_villecpwidget.h"

/*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------- GESTION DES VILLES ET DES CODES POSTAUX
CE WIDGET SERT À GÉRER LES VILLES ET LES CODES POSTAUX
Il intègre 2 QLabel et 2 QLineEdit pour le code postal et la ville
Le QlineEdit de la ville a un QCompleter en mode poppup pour suggérer des villes correspondant à la saisie.

A - LA RECHERCHE DU CODE POSTAL A PARTIR D'UN NOM DE VILLE
Le recherche du code postal est déclenchée par:
    . soit par un choix dans le QCompleter,
    . soit par le focusout() du QLineEdit de la ville (le signal editingFinished() est compliqué à gérer dans ce cas).
Le choix dans le QCompleter appelle aussi le focusout() du QLineEdit de la ville. On désactive donc la recherche du code postal par le focusout() avec le bool VilleAConfirmer
quand le choix de la ville a été fait par le completer, sinon, la recherche du CP est appelée 2 fois.
Ce bool sert aussi à désactiver la recherche de cohérence du nom de la ville par la fonction ChoixCodePostal de VilleCPWidget
puisque c'est inutile quand le nom de la ville est choisi à partirt du QCOmpleter.
Enfin, une fonction VerifCoherenceCP() sert à vérifier la cohérence du couple Codepostal-Ville
Donc, dans l'ordre qui doit gérer les codes postaux:
    . définition du QCompleter du QlineEdit de la ville
    . installeventfilter sur le QlineEdit de la ville
    . VilleAConfirmer initialisé à false
    . le signal textEdited du QLineEdit de la ville remet VilleAcofirmer à true:
    . le focusout du QlineEdit de la ville démarre la fonction ChercheCP(): si VilleAConfirmer est true, la fonction verifie la cohérence du code postal et le cherche en cas d'incohérence.
    . le signal activated() du QCompleter déclenche le slot Slot_ChercheCPdepuisCompleter()). Le slot remet villeAConfirmer à false,
      puis verifie la cohérence du code postal et le cherche en cas d'incohérence
Le signal villecpmodified() est émis quand le CP ou la ville ont été modifiés

B - LA RECHERCHE DE LA VILLE A PARTIR D'UN CODE POSTAL
Même chose mais en plus simple : pas de recherche à partir du QCompleter, pas de suggestion sur des orthographes avoisinantes (par la force desc choses)

Pour intéggrer le widget, il suffit de faire
    VilleCPWidget *VilleCPwidg  = new VilleCPWidget(NomDeLaBDD, NomDeLaTableDesVillesEtDesCodesPostaux, leParent, ListedesVilles, ListeDesCodesPostaux, NomDeLAlarme);
Les paramètres sont
    QSqlDatabase gdb:       la base de données
    QString NomtableVilles: le nom de la table
    QStringList listVilles: la liste des villes
    QStringList listCP:     la liste des codes postaux
    QString Son:            le son d'alarme
    QWidget *parent:        le parent

puis, on l'utilise avec, par exemple:
    QLineEdit                   *CPlineEdit, *VillelineEdit;
    CPlineEdit                  = VilleCPwidg->ui->CPlineEdit;
    VillelineEdit               = VilleCPwidg->ui->VillelineEdit;
    VilleCPwidg                 ->move(x,y);
    connect(VilleCPwidg,    SIGNAL(villecpmodified()),  this,   SLOT(Faitcequejeveuxaveclesignal()));

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

VilleCPWidget::VilleCPWidget(QSqlDatabase gdb, QString NomtableVilles, QWidget *parent, QStringList listVilles, QStringList listCP, QString Son) :
    QWidget(parent),
    ui(new Ui::VilleCPWidget)
{
    ui->setupUi(this);
    ui->CPlineEdit              ->setValidator(new QRegExpValidator(QRegExp("[0-9]{5}"),this));
    ui->VillelineEdit           ->setValidator(new QRegExpValidator(QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*"),this));
    VilleAConfirmer             = false;
    CPAConfirmer                = false;
    TableVilles                 = NomtableVilles;
    db                          = gdb;
    Alarme                      = Son;
    setFocusProxy(ui->CPlineEdit);

    if (listVilles.isEmpty())
    {
        QString req = "SELECT distinct Ville FROM " + NomtableVilles +  " ORDER BY Ville";
        QSqlQuery ChercheVillesQuery (req,db);
        if (!TraiteErreurRequete(ChercheVillesQuery,req,tr("Impossible de retrouver les villes!")))
        {
            for (int i = 0; i < ChercheVillesQuery.size(); i++)
            {
                ChercheVillesQuery.seek(i);
                listVilles << ChercheVillesQuery.value(0).toString();
            }
        }
    }
    if (listCP.isEmpty())
    {
        QString req = "SELECT distinct codePostal FROM " + NomtableVilles +  " ORDER BY codePOstal";
        QSqlQuery ChercheCPQuery (req,db);
        if (!TraiteErreurRequete(ChercheCPQuery,req,tr("Impossible de retrouver les codes postaux!")))
        {
            for (int i = 0; i < ChercheCPQuery.size(); i++)
            {
                ChercheCPQuery.seek(i);
                listeCP << ChercheCPQuery.value(0).toString();
            }
        }
    }
    else
        listeCP = listCP;

    QCompleter *complListVilles = new QCompleter(listVilles,this);
    complListVilles             ->setCaseSensitivity(Qt::CaseInsensitive);
    complListVilles             ->setCompletionMode(QCompleter::PopupCompletion);
    complListVilles             ->setFilterMode(Qt::MatchStartsWith);
    ui->VillelineEdit           ->setCompleter(complListVilles);
    QCompleter *complListCP     = new QCompleter(listeCP,this);
    complListCP                 ->setCaseSensitivity(Qt::CaseInsensitive);
    complListCP                 ->setCompletionMode(QCompleter::InlineCompletion);
    ui->CPlineEdit              ->setCompleter(complListCP);

    ui->CPlineEdit              ->installEventFilter(this);
    ui->VillelineEdit           ->installEventFilter(this);

    connect(complListVilles,    SIGNAL(activated(QString)),         this,       SLOT(Slot_ChercheCPdepuisCompleter()));
    connect(complListCP,        SIGNAL(activated(QString)),         this,       SLOT(Slot_ChercheVilledepuisCompleter()));
    connect (ui->CPlineEdit,    SIGNAL(textEdited(QString)),        this,       SLOT(Slot_EnableOKpushButton()));
    connect (ui->VillelineEdit, SIGNAL(textEdited(QString)),        this,       SLOT(Slot_EnableOKpushButton()));
}

VilleCPWidget::~VilleCPWidget()
{
    delete ui;
}

void VilleCPWidget::Slot_ConfirmerLaVille()
{
    VilleAConfirmer = true;
}

void VilleCPWidget::Slot_ConfirmerLeCP()
{
    CPAConfirmer = true;
}

bool VilleCPWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (obj == ui->CPlineEdit)
            ChercheVilledepuisQLine();
        if (obj == ui->VillelineEdit)
            ChercheCPdepuisQLine();
    }
    return QWidget::eventFilter(obj, event);
}


void VilleCPWidget::Slot_EnableOKpushButton()
{
    if (sender()==ui->CPlineEdit)       CPAConfirmer = true;
    if (sender()==ui->VillelineEdit)    VilleAConfirmer = true;
    emit villecpmodified();
}

void VilleCPWidget::ChercheCPdepuisQLine()
{
    ui->VillelineEdit->setText(fMAJPremiereLettre(ui->VillelineEdit->text()));
    if (VilleAConfirmer)
        if (!VerifCoherence())
            ChercheCodePostal();
}

void VilleCPWidget::Slot_ChercheCPdepuisCompleter()
{
    VilleAConfirmer = false;
    if (!VerifCoherence())
    {
        ChercheCodePostal(VilleAConfirmer);
        emit villecpmodified();
    }
}

void VilleCPWidget::ChercheVilledepuisQLine()
{
    if (CPAConfirmer)
        if (!VerifCoherence())
            ChercheVille();
}

void VilleCPWidget::Slot_ChercheVilledepuisCompleter()
{
    CPAConfirmer = false;
    if (!VerifCoherence())
    {
        ChercheVille(CPAConfirmer);
        emit villecpmodified();
    }
}

void VilleCPWidget::ChercheVille(bool confirmerleCP)  // Recherche la ville une fois qu'on a entré un code postal
{
    NouvVille   = QString();
    if (ui->CPlineEdit->text() == "")
        return;
    QString CP = ui->CPlineEdit->text();
    QString requete;
    if (confirmerleCP)
    {
        requete = "select CodePostal from " + TableVilles + " WHERE CodePostal = '" + CP + "' ORDER BY CodePostal";
        QSqlQuery chercheCPquery (requete, db);
        if (chercheCPquery.size()==0)
        {
            UpMessageBox::Watch(this,tr("Code postal inconnu"));
            CP = "";
        }
    }
    if (CP == QString() || CP == "")
        return;
    requete = "SELECT Ville FROM " + TableVilles + " WHERE CodePostal = '" + CP + "' ORDER BY CodePostal";
    QSqlQuery ChercheVilleQuery (requete, db);
    if (!TraiteErreurRequete(ChercheVilleQuery,requete, tr("Impossible de trouver la ville correspondant au code postal ") + ui->CPlineEdit->text()))
    {
        if (ChercheVilleQuery.size() == 0)
        {
            QSound::play(Alarme);
            UpMessageBox::Watch(this, tr("Impossible de trouver la ville correspondant au code postal ") + ui->CPlineEdit->text());
            ui->VillelineEdit->clear();
            ui->VillelineEdit->setFocus();
            return;
        }
        ChercheVilleQuery.first();
        if (ChercheVilleQuery.size() == 1)  // on a trouvé la ville - on affiche sa valeur dans le champ ville
        {
            NouvVille = ChercheVilleQuery.value(0).toString();
            ui->VillelineEdit->setText(NouvVille);
        }

        else // if (ChercheVilleQuery.size() > 1)  // on a trouvé plusieurs villes
        {
            NouvVille                           = QString();
            gAskVille                           = new UpDialog(this);
            gAskVille                           ->setModal(true);
            QVBoxLayout         *globallay      = dynamic_cast<QVBoxLayout*>(gAskVille->layout());
            QListView           *listVille      = new QListView(gAskVille);
            QStandardItemModel  *listVillemodel = new QStandardItemModel;

            QLabelDelegate      *deleglabl      = new QLabelDelegate;
            int larg = 300;

            gAskVille->AjouteLayButtons(UpDialog::ButtonOK);
            connect(gAskVille->OKButton,       SIGNAL(clicked(bool)),      this,   SLOT(Slot_ReponsVille()));

            listVille->setFixedWidth(larg);
            listVille->setPalette(QPalette(Qt::white));
            listVille->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            listVille->setEditTriggers(QAbstractItemView::NoEditTriggers);
            listVille->setSelectionMode(QAbstractItemView::SingleSelection);
            connect(listVille,      SIGNAL(doubleClicked(QModelIndex)),    this,   SLOT(Slot_ReponsVille()));

            listVillemodel->setHorizontalHeaderItem(0, new QStandardItem(tr("Code postal")));
            ChercheVilleQuery.first();
            listVille->setModel(listVillemodel);
            QStandardItem *pitem;
            for (int i=0; i<ChercheVilleQuery.size(); i++)
            {
                pitem = new QStandardItem(ChercheVilleQuery.value(0).toString());
                listVillemodel->appendRow(pitem);
                ChercheVilleQuery.next();
            }
            listVille->setItemDelegate(deleglabl);
            connect(deleglabl, SIGNAL(focusitem(int)),   this,   SLOT(Slot_VilleEnableOKbutton()));
            globallay->insertWidget(0,listVille);
            globallay->setSizeConstraint(QLayout::SetFixedSize);
            gAskVille->OKButton->setEnabled(false);
            gAskVille->exec();
            if (NouvVille != NULL)
                ui->VillelineEdit->setText(NouvVille);
            delete gAskVille;
        }
    }
}

QString VilleCPWidget::ConfirmeVille(QString Ville)
{
    QString repons = QString();
    if (fMAJPremiereLettre(Ville).length() == 0) return repons;
    QString requete = "SELECT distinct Ville FROM " + TableVilles + " WHERE Ville = '" + CorrigeApostrophe(Ville) + "' ORDER BY Ville";
    QSqlQuery   ChercheVilleQuery (db);
    ChercheVilleQuery.exec(requete);
    if (ChercheVilleQuery.size()==0)
    {
        QString VilleRegExp = Ville;
        VilleRegExp.replace(QRegExp("[ -]"),"[ -]");
        VilleRegExp = CorrigeApostrophe(VilleRegExp);
        requete = "SELECT distinct Ville FROM " + TableVilles + " WHERE Ville like '" + VilleRegExp + "%'"
                  " or Ville regexp '" + VilleRegExp + "' ORDER BY Ville";
        ChercheVilleQuery.exec(requete);
    }
    if (ChercheVilleQuery.lastError().type() != QSqlError::NoError)
        return repons;

    ChercheVilleQuery.first();
    if (ChercheVilleQuery.size() == 1)
        repons = ChercheVilleQuery.value(0).toString();
    else
    {
        if (gAskVille != Q_NULLPTR)
            return QString();
        NouvVille                           = QString();
        gAskVille                           = new UpDialog(this);
        gAskVille                           ->setModal(true);
        QVBoxLayout         *globallay      = dynamic_cast<QVBoxLayout*>(gAskVille->layout());
        QListView           *listVille      = new QListView(gAskVille);
        QItemSelectionModel *villeselect    = new QItemSelectionModel;
        villeselect = listVille->selectionModel();
        QStandardItemModel  *listVillemodel = new QStandardItemModel;

        QLabelDelegate      *deleglabl      = new QLabelDelegate;
        int larg = 250;

        gAskVille->AjouteLayButtons(UpDialog::ButtonOK);
        connect(gAskVille->OKButton,       SIGNAL(clicked(bool)),      this,   SLOT(Slot_ReponsVille()));

        listVille->setFixedWidth(larg);
        listVille->setPalette(QPalette(Qt::white));
        listVille->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listVille->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listVille->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(listVille,      SIGNAL(doubleClicked(QModelIndex)),    this,   SLOT(Slot_ReponsVille()));

        listVillemodel->setHorizontalHeaderItem(0, new QStandardItem(tr("Code postal")));
        ChercheVilleQuery.first();
        listVille->setModel(listVillemodel);
        QStandardItem *pitem;
        for (int i=0; i<ChercheVilleQuery.size(); i++)
        {
            pitem = new QStandardItem(ChercheVilleQuery.value(0).toString());
            listVillemodel->appendRow(pitem);
            ChercheVilleQuery.next();
        }
        listVille->setItemDelegate(deleglabl);
        connect(deleglabl, SIGNAL(focusitem(int)),   this,   SLOT(Slot_VilleEnableOKbutton()));
        globallay->insertWidget(0,listVille);
        globallay->setSizeConstraint(QLayout::SetFixedSize);
        gAskVille->OKButton->setEnabled(false);
        gAskVille->exec();
        repons = NouvVille;
        delete gAskVille;
    }
    return repons;
}

void VilleCPWidget::ChercheCodePostal(bool confirmerlaville)
{
    NouveauCP = QString();
    if (ui->VillelineEdit->text() == "")
        return;
    QString Ville;
    if (confirmerlaville)
        Ville = ConfirmeVille(ui->VillelineEdit->text());
    else
        Ville = ui->VillelineEdit->text();
    if (Ville == NULL || Ville == "")
        return;
    if (Ville!=ui->VillelineEdit->text())
        ui->VillelineEdit->setText(Ville);
    QString requete = "SELECT distinct CodePostal FROM " + TableVilles + " WHERE Ville = '" + CorrigeApostrophe(Ville) + "' ORDER BY Codepostal";
    QSqlQuery   ChercheCodePostalQuery (db);
    ChercheCodePostalQuery.exec(requete);
    if (ChercheCodePostalQuery.size()==0)
    {
        QString VilleRegExp = Ville;
        VilleRegExp.replace(QRegExp("[ -]"),"[ -]");
        VilleRegExp = CorrigeApostrophe(VilleRegExp);
        requete = "SELECT distinct CodePostal FROM " + TableVilles + " WHERE Ville regexp '" + VilleRegExp + "%'"
                  " or Ville regexp '" + VilleRegExp + "' ORDER BY Codepostal";
        ChercheCodePostalQuery.exec(requete);
    }
    if (ChercheCodePostalQuery.lastError().type() != QSqlError::NoError)
        return;
    if (ChercheCodePostalQuery.size() == 0)
    {
        QSound::play(Alarme);
        UpMessageBox::Watch(0,tr("Ville inconnue"));
        ui->CPlineEdit->clear();
        ui->CPlineEdit->setFocus();
        return;
    }
    ChercheCodePostalQuery.first();

    if (ChercheCodePostalQuery.size() == 1)
    {
        NouveauCP = ChercheCodePostalQuery.value(0).toString();
        ui->CPlineEdit->setText(NouveauCP);
    }
    else
    {
        if (gAskCP != Q_NULLPTR)
            return;
        gAskCP                          = new UpDialog(this);
        gAskCP                          ->setModal(true);
        QVBoxLayout *globallay          = dynamic_cast<QVBoxLayout*>(gAskCP->layout());
        QListView *listCP               = new QListView(gAskCP);
        QStandardItemModel *listCPmodel = new QStandardItemModel;

        QLabelDelegate  *deleglabl      = new QLabelDelegate;
        int larg = 100;

        gAskCP->AjouteLayButtons(UpDialog::ButtonOK);
        connect(gAskCP->OKButton,       SIGNAL(clicked(bool)),      this,   SLOT(Slot_ReponsCodePostal()));

        listCP->setFixedWidth(larg);
        listCP->setPalette(QPalette(Qt::white));
        listCP->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listCP->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listCP->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(listCP,      SIGNAL(doubleClicked(QModelIndex)),    this,   SLOT(Slot_ReponsCodePostal()));

        listCPmodel->setHorizontalHeaderItem(0, new QStandardItem(tr("Code postal")));
        ChercheCodePostalQuery.first();
        QStandardItem *pitem;
        for (int i=0; i<ChercheCodePostalQuery.size(); i++)
        {
            pitem = new QStandardItem(ChercheCodePostalQuery.value(0).toString());
            listCPmodel->appendRow(pitem);
            ChercheCodePostalQuery.next();
        }
        listCP->setModel(listCPmodel);
        listCP->setItemDelegate(deleglabl);
        connect(deleglabl, SIGNAL(focusitem(int)),   this,   SLOT(Slot_CPEnableOKbutton()));
        globallay->insertWidget(0,listCP);
        globallay->setSizeConstraint(QLayout::SetFixedSize);
        gAskCP->OKButton->setEnabled(false);

        gAskCP->exec();
        if (NouveauCP != NULL)
            ui->CPlineEdit->setText(NouveauCP);
        delete gAskCP;
    }
}

void VilleCPWidget::Slot_ReponsCodePostal()
{
    if (gAskCP == Q_NULLPTR)
        return;
    if (gAskCP->findChild<QListView*>()->selectionModel()->selectedIndexes().size()>0)
    {
        NouveauCP = gAskCP->findChild<QListView*>()->selectionModel()->selectedIndexes().at(0).data().toString();
        gAskCP->close();
    }
}

void VilleCPWidget::Slot_ReponsVille()
{
    if (gAskVille == Q_NULLPTR)
        return;
    if (gAskVille->findChild<QListView*>()->selectionModel()->selectedIndexes().size()>0)
    {
        NouvVille = gAskVille->findChild<QListView*>()->selectionModel()->selectedIndexes().at(0).data().toString();
        gAskVille->close();
    }
}

void VilleCPWidget::Slot_CPEnableOKbutton()
{
    if (gAskCP != Q_NULLPTR)
        gAskCP->OKButton->setEnabled(true);
}

void VilleCPWidget::Slot_VilleEnableOKbutton()
{
    if (gAskVille != Q_NULLPTR)
        gAskVille->OKButton->setEnabled(true);
}

bool VilleCPWidget::VerifCoherence()
{
    QString Ville = ui->VillelineEdit->text();
    QString VilleRegExp = Ville;
    //    VilleRegExp.replace(QRegExp("[ -]"),"[ -]");
    VilleRegExp = CorrigeApostrophe(VilleRegExp);
    bool b = false;
    QString req = " select CodePostal FROM " + TableVilles + " WHERE Ville = '" + VilleRegExp + "'";
    QSqlQuery qer(req, db);
    if (qer.size() > 0)
    {
        qer.first();
        for (int i=0; i<qer.size(); i++)
        {
            if (qer.value(0).toString() == ui->CPlineEdit->text())
            {
                b = true;
                i = qer.size();
            }
            qer.next();
        }
    }
    return b;
}

QString VilleCPWidget::CorrigeApostrophe(QString RechAp)    //++++ voir commentaire dans rufus.h
{
    RechAp.replace("\\","\\\\");
    return RechAp.replace("'","\\'");
}

bool VilleCPWidget::TraiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage)
{
    if (query.lastError().type() != QSqlError::NoError)
    {
        UpMessageBox::Watch(0, ErrorMessage, tr("\nErreur\n") + query.lastError().text() +  tr("\nrequete = ") + requete);
        return true;
    }
    else return false;
}



