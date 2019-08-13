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

#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "database.h"
#include "cls_villes.h"
#include "utils.h"

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

VilleCPWidget::VilleCPWidget(Villes *villes, QWidget *parent, QString Son) :
    QWidget(parent),
    ui(new Ui::VilleCPWidget)
{
    ui->setupUi(this);
    ui->CPlineEdit              ->setValidator(new QRegExpValidator(Utils::rgx_CP,this));
    ui->VillelineEdit           ->setValidator(new QRegExpValidator(Utils::rgx_ville,this));
    m_alarme                      = Son;
    m_villes                    = villes;

    setFocusProxy(ui->CPlineEdit);

    QCompleter *complListVilles = new QCompleter(m_villes->getListVilles(),this);
    complListVilles             ->setCaseSensitivity(Qt::CaseInsensitive);
    complListVilles             ->setCompletionMode(QCompleter::PopupCompletion);
    complListVilles             ->setFilterMode(Qt::MatchStartsWith);
    ui->VillelineEdit           ->setCompleter(complListVilles);

    QCompleter *complListCP     = new QCompleter(m_villes->getListCodePostal(),this);
    complListCP                 ->setCaseSensitivity(Qt::CaseInsensitive);
    complListCP                 ->setCompletionMode(QCompleter::InlineCompletion);
    ui->CPlineEdit              ->setCompleter(complListCP);


    connect(complListVilles,    QOverload<const QString &>::of(&QCompleter::activated), [=](const QString &) { ChercheCodePostal(false); emit villecpmodified(); });
    //connect(complListCP,        QOverload<const QString &>::of(&QCompleter::activated), [=](const QString &) { qDebug()<<"CP Completer"; ChercheVille(false); emit villecpmodified(); });
    connect(ui->CPlineEdit, &QLineEdit::textEdited, this, [=]{
        connect(ui->CPlineEdit, &QLineEdit::editingFinished, this, &VilleCPWidget::Slot_ChercheVille);
    });
    connect(ui->VillelineEdit, &QLineEdit::textEdited, [=]{
        connect(ui->VillelineEdit, &QLineEdit::editingFinished, this, &VilleCPWidget::Slot_ChercheCodePostal);
    });

}

VilleCPWidget::~VilleCPWidget()
{
    delete ui;
}

Villes *VilleCPWidget::villes() const
{
    return m_villes;
}

void VilleCPWidget::Slot_ChercheVille()
{
    disconnect(ui->CPlineEdit, &QLineEdit::editingFinished, this, &VilleCPWidget::Slot_ChercheVille);
    ChercheVille();
    emit villecpmodified();
}
void VilleCPWidget::ChercheVille(bool confirmerleCP)  // Recherche la ville une fois qu'on a entré un code postal
{
    if (ui->CPlineEdit->text().isEmpty())
        return;

    QString CP = ui->CPlineEdit->text();
    QList<Ville*> villes;
    try
    {
        villes = m_villes->getVilleByCodePostal(CP, confirmerleCP);
    }
    catch( QJsonObject const& err )
    {
        if( err.value("errorCode").toInt() == 1 )
            UpMessageBox::Watch(this, err.value("errorMessage").toString());
        else if( err.value("errorCode").toInt() == 2 )
        {
            QSound::play(m_alarme);
            UpMessageBox::Watch(this, err.value("errorMessage").toString());
            ui->VillelineEdit->clear();
            ui->VillelineEdit->setFocus();
        }
        return;
    }

    if( villes.size() == 1 )
    {
        // on a trouvé la ville - on affiche sa valeur dans le champ ville
        ui->VillelineEdit->setText( villes.at(0)->nom() );
        return;
    }
    // on a trouvé plusieurs villes
    QString newValue = dialogList(villes, "nom", tr("Nom de la ville"));
    if (newValue != Q_NULLPTR)
        ui->VillelineEdit->setText(newValue);
}

void VilleCPWidget::Slot_ChercheCodePostal()
{
    disconnect(ui->VillelineEdit, &QLineEdit::editingFinished, this, &VilleCPWidget::Slot_ChercheCodePostal);
    ChercheCodePostal();
    emit villecpmodified();
}
void VilleCPWidget::ChercheCodePostal(bool confirmerlaville)
{
    if (ui->VillelineEdit->text() == "")
        return;

    if( confirmerlaville )
        ui->VillelineEdit->setText(Utils::trimcapitilize(ui->VillelineEdit->text()));

    QString ville = ui->VillelineEdit->text();

    if (confirmerlaville)
        ville = ConfirmeVille(ville);

    if( ville.isEmpty() )
        return;

    if(ville != ui->VillelineEdit->text())
        ui->VillelineEdit->setText(ville);

    QList<Ville*> villes = m_villes->getVilleByName(ville);
    if( villes.isEmpty() )
    {
        QSound::play(m_alarme);
        UpMessageBox::Watch(Q_NULLPTR,tr("Ville inconnue"));
        ui->CPlineEdit->clear();
        ui->CPlineEdit->setFocus();
        return;
    }
    if( villes.size() == 1 )
    {
        // on a trouvé la ville - on affiche sa valeur dans le champ codePostal
        ui->CPlineEdit->setText( villes.at(0)->codePostal() );
        return;
    }

    // on a trouvé plusieurs villes
    QString newValue = "";
    while(newValue.isEmpty() ) //pour etre sur que l'utilisateur choississe bien un code postal
    {
        newValue = dialogList(villes, "codepostal", tr("Code postal"));
        if( newValue.isEmpty() )
        {
            //TODO : METTRE POPUP : tr(veillez choisir un Code Postal)
        }
    }
    ui->CPlineEdit->setText(newValue);
}

QString VilleCPWidget::ConfirmeVille(QString ville)
{
    QList<Ville*> villes = m_villes->getVilleByName(ville, true);
    if( villes.isEmpty() )
    {
        QSound::play(m_alarme);
        UpMessageBox::Watch(Q_NULLPTR,tr("Ville inconnue"));
        ui->CPlineEdit->clear();
        ui->CPlineEdit->setFocus();
        return QString();
    }
    if( villes.size() == 1 )
    {
        // on a trouvé la ville - on retourne le nom de la ville
        return villes.at(0)->nom();
    }

    // on a trouvé plusieurs villes
    return dialogList(villes, "nom", tr("Nom de la ville"));
}

QString VilleCPWidget::dialogList(QList<Ville*> &listData, QString fieldName, QString headerName)
{
    UpDialog *gAsk                 = new UpDialog();
    gAsk                           ->setModal(true);
    QListView *list                = new QListView(gAsk);
    VilleListModel *listModel      = new VilleListModel(listData,fieldName);

    QLabelDelegate  *deleglabl      = new QLabelDelegate;
    gAsk->AjouteLayButtons(UpDialog::ButtonOK);

    //list->setFixedWidth(100);
    list->setPalette(QPalette(Qt::white));
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->setSelectionMode(QAbstractItemView::SingleSelection);

    listModel->setHeaderData(0, Qt::Orientation::Horizontal, headerName);
    list->setModel(listModel);
    list->setItemDelegate(deleglabl);

    gAsk->dlglayout()->insertWidget(0,list);
    gAsk->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    gAsk->OKButton->setEnabled(false);

    QString newValue;
    connect(gAsk->OKButton, &QPushButton::clicked,      gAsk, [=, &newValue] { Repons(list, gAsk, newValue); });
    connect(list,           &QListView::doubleClicked,  gAsk, [=, &newValue] { Repons(list, gAsk, newValue); });
    connect(deleglabl,      &QLabelDelegate::focusitem, gAsk, [gAsk] { gAsk->OKButton->setEnabled(true); });
    connect(list->selectionModel(), &QItemSelectionModel::currentChanged, gAsk,  [=] { gAsk->OKButton->setEnabled(list->selectionModel()->selectedIndexes().size()>0); });

    gAsk->exec();
    delete gAsk;
    return newValue;
}
void VilleCPWidget::Repons(QListView *lv, UpDialog *ud, QString &newValue )
{
    QModelIndexList mi = lv->selectionModel()->selectedIndexes();
    if( mi.size() )
    {
        newValue = mi.at(0).data().toString();
        ud->close();
    }
}


