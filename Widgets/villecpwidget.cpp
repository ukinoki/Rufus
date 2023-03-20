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

#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "database.h"
#include "cls_villes.h"
#include "utils.h"

/*!
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
Même chose mais en plus simple : pas de recherche à partir du QCompleter, pas de suggestion sur des orthographes avoisinantes (par la force des choses)

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

VilleCPWidget::VilleCPWidget(Villes *villes, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VilleCPWidget)
{
    ui->setupUi(this);
    ui->CPlineEdit              ->setValidator(new QRegExpValidator(Utils::rgx_CP,this));
    ui->VillelineEdit           ->setValidator(new QRegExpValidator(Utils::rgx_ville,this));
    m_villes                    = villes;
    wdg_parent                  = parent;
    QSettings m_settings(PATH_FILE_INI, QSettings::IniFormat);
    if (m_settings.value(Utilise_BDD_Villes).toBool() != false || m_settings.value(Utilise_BDD_Villes) == QVariant())
        m_settings.setValue(Utilise_BDD_Villes, m_rechercheCP);
    m_rechercheCP = m_settings.value(Utilise_BDD_Villes).toBool();

    setFocusProxy(ui->CPlineEdit);

    complListVilles             ->setModel(new QStringListModel(m_villes->ListeNomsVilles()));
    complListVilles             ->setCaseSensitivity(Qt::CaseInsensitive);
    complListVilles             ->setCompletionMode(QCompleter::PopupCompletion);
    complListVilles             ->setFilterMode(Qt::MatchStartsWith);
    ui->VillelineEdit           ->setCompleter(complListVilles);

    complListCP                 ->setModel(new QStringListModel(m_villes->ListeCodesPostaux()));
    complListCP                 ->setCaseSensitivity(Qt::CaseInsensitive);
    complListCP                 ->setCompletionMode(QCompleter::InlineCompletion);
    ui->CPlineEdit              ->setCompleter(complListCP);

    if (m_rechercheCP)
        connectrecherche();
    else
    {
        connect(ui->VillelineEdit, &QLineEdit::editingFinished, this,[=] {
                                                                            ui->VillelineEdit->setText(Utils::trimcapitilize(ui->VillelineEdit->text()));
                                                                            if  (ui->VillelineEdit->text() != "")
                                                                                ChercheVilleBaseIndividual(ui->VillelineEdit->text());
                                                                            emit villecpmodified();
                                                                         });
        connect(complListVilles,    QOverload<const QString &>::of(&QCompleter::activated), this, [=] {
                                                                                                        ChercheCPBaseIndividual(ui->VillelineEdit->text());
                                                                                                        emit villecpmodified();
                                                                                                      });
    }
}

VilleCPWidget::~VilleCPWidget()
{
    delete ui;
}

bool   VilleCPWidget::isValid(bool avecmsgbox)
{
    if (m_rechercheCP)
    {
        if (ui->CPlineEdit->text() == "" && ui->VillelineEdit->text() == "")
        {
            if (avecmsgbox)
            {
                UpMessageBox::Watch(this,tr("Vous n'avez indiqué ni la ville ni le code postal!"));
                ui->CPlineEdit->setFocus();
            }
            return false;
        }
        if (ui->CPlineEdit->text() == "")
        {
            if (avecmsgbox)
            {
                UpMessageBox::Watch(this,tr("Il manque le code postal"));
                ui->CPlineEdit->setFocus();
            }
            return false;
        }
    }
    if (ui->VillelineEdit->text() == "")
    {
        if (avecmsgbox)
        {
            UpMessageBox::Watch(this,tr("Il manque le nom de la ville"));
            ui->VillelineEdit->setFocus();
        }
        return false;
    }
    return true;
}

Villes *VilleCPWidget::villes() const
{
    return m_villes;
}

void VilleCPWidget::StartChercheVille()
{
    disconnectrecherche();
    ChercheVille();
    emit villecpmodified();
    connectrecherche();
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
            QSound::play(NOM_ALARME);
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
    QString newValue = dialogList(villes, VilleListModel::NOM, tr("Nom de la ville"));
    if (newValue != Q_NULLPTR)
        ui->VillelineEdit->setText(newValue);
}

void VilleCPWidget::connectrecherche()
{
    m_villeobject = new QObject(this);
    m_CPobject = new QObject(this);
    m_completerobject = new QObject(this);
    connect(complListVilles,    QOverload<const QString &>::of(&QCompleter::activated), m_completerobject, [=] { ChercheCodePostal(false);
                                                                                                    emit villecpmodified(); });
    connect(ui->CPlineEdit, &QLineEdit::textEdited, m_CPobject, [=]{
            connect(ui->CPlineEdit, &QLineEdit::editingFinished, this, &VilleCPWidget::StartChercheVille);
            });
    connect(ui->VillelineEdit, &QLineEdit::textEdited, m_villeobject, [&]{
            connect(ui->VillelineEdit, &QLineEdit::editingFinished, this, &VilleCPWidget::StartChercheCodePostal);
            });
}

void VilleCPWidget::disconnectrecherche()
{
    delete m_villeobject;
    delete m_CPobject;
    delete m_completerobject;
    complListVilles->disconnect();
    ui->CPlineEdit->disconnect();
    ui->VillelineEdit->disconnect();
}


void VilleCPWidget::StartChercheCodePostal()
{
    disconnectrecherche();
    ChercheCodePostal();
    emit villecpmodified();
    connectrecherche();
}


/*!
 * \brief VilleCPWidget::ChercheCodePostal
 * \param confirmerlaville -> true = la fonction a été appelée par la ligne et pass par le QCompleter
*/
void VilleCPWidget::ChercheCodePostal(bool confirmerlaville)
{
    if (ui->VillelineEdit->text() == "")
        return;

    if( confirmerlaville )
        ui->VillelineEdit->setText(Utils::trimcapitilize(ui->VillelineEdit->text()));
    else
    {
        disconnectrecherche();
    }

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
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(this,tr("Ville inconnue"));
        ui->CPlineEdit->clear();
        ui->CPlineEdit->setFocus();
        return;
    }
    if( villes.size() == 1 )
    {
        // on a trouvé la ville - on affiche sa valeur dans le champ codePostal
        ui->CPlineEdit->setText( villes.at(0)->codepostal() );
        return;
    }

    // on a trouvé plusieurs villes
    QString newValue = "";
    while(newValue.isEmpty() ) //pour etre sur que l'utilisateur choississe bien un code postal
    {
        newValue = dialogList(villes, VilleListModel::CODEPOSTAL, tr("Code postal"));
        if( newValue.isEmpty() )
        {
            //TODO : METTRE POPUP : tr(veillez choisir un Code Postal)
        }
    }
    ui->CPlineEdit->setText(newValue);
    if (!confirmerlaville && m_rechercheCP)
        connectrecherche();
}

QString VilleCPWidget::ConfirmeVille(QString ville)
{
    QList<Ville*> villes = m_villes->getVilleByName(ville, true);
    if( villes.isEmpty() )
    {
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(this,tr("Ville inconnue"));
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
    return dialogList(villes, VilleListModel::NOM, tr("Nom de la ville"));
}

QString VilleCPWidget::dialogList(QList<Ville*> &listvilles, VilleListModel::FieldName fieldName, QString headerName)
{
    UpDialog *gAsk                 = new UpDialog(wdg_parent);
    gAsk                           ->setWindowModality(Qt::WindowModal);
    QListView *listvw              = new QListView(gAsk);
    VilleListModel *listModel      = new VilleListModel(listvilles,fieldName);

    UpLabelDelegate  *deleglabl    = new UpLabelDelegate;
    gAsk                           ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    //listw->setFixedWidth(100);
    listvw      ->setPalette(QPalette(Qt::white));
    listvw      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listvw      ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listvw      ->setSelectionMode(QAbstractItemView::SingleSelection);

    listModel   ->setHeaderData(0, Qt::Orientation::Horizontal, headerName);
    listvw      ->setModel(listModel);
    listvw      ->setItemDelegate(deleglabl);

    gAsk->dlglayout()   ->insertWidget(0,listvw);
    gAsk->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    gAsk->OKButton      ->setEnabled(false);

    QString newValue;
    connect(gAsk->OKButton, &QPushButton::clicked,                          gAsk,   [=, &newValue] { Repons(listvw, gAsk, newValue); });
    connect(listvw,           &QListView::doubleClicked,                    gAsk,   [=, &newValue] { Repons(listvw, gAsk, newValue); });
    connect(deleglabl,      &UpLabelDelegate::focusitem,                    gAsk,   [gAsk] { gAsk->OKButton->setEnabled(true); });
    connect(listvw->selectionModel(), &QItemSelectionModel::currentChanged, gAsk,   [=] { gAsk->OKButton->setEnabled(listvw->selectionModel()->selectedIndexes().size()>0); });

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

void VilleCPWidget::ChercheVilleBaseIndividual(QString nomville)
{
    if (!m_villes->ListeNomsVilles().contains(nomville))
    {
        if  (UpMessageBox::Question(this,
                                    tr("Ville  inconnue!"),
                                    tr("Voulez-vous enregistrer ") + nomville + tr(" dans la base de données?"))
             == UpSmallButton::STARTBUTTON)
        {
            UpDialog *dlg_ask       = new UpDialog(this);
            dlg_ask                 ->setWindowModality(Qt::WindowModal);

            UpLineEdit *CP          = new UpLineEdit(dlg_ask);
            CP                      ->setValidator(new QRegExpValidator(Utils::rgx_CP,this));
            CP                      ->setAlignment(Qt::AlignCenter);
            CP                      ->setMaxLength(12);
            dlg_ask->dlglayout()    ->insertWidget(0,CP);

            dlg_ask->dlglayout()    ->insertSpacerItem(0,new QSpacerItem(0,10,QSizePolicy::Expanding, QSizePolicy::Expanding));

            UpLabel *labelCP        = new UpLabel();
            labelCP                 ->setText(tr("Code postal (facultatif)"));
            dlg_ask->dlglayout()    ->insertWidget(0,labelCP);

            dlg_ask->dlglayout()    ->insertSpacerItem(0,new QSpacerItem(0,10,QSizePolicy::Expanding, QSizePolicy::Expanding));

            UpLabel *labelVille     = new UpLabel();
            labelVille              ->setText(nomville.toUpper());
            labelVille              ->setAlignment(Qt::AlignCenter);
            QFont font = qApp->font();
            font.setBold(true);
            font.setPointSize(font.pointSize()+4);
            labelVille              ->setFont(font);
            dlg_ask->dlglayout()    ->insertWidget(0,labelVille);

            dlg_ask->dlglayout()    ->insertSpacerItem(0,new QSpacerItem(0,10,QSizePolicy::Expanding, QSizePolicy::Expanding));

            dlg_ask                 ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
            dlg_ask                 ->setWindowTitle(tr("Enregistrement d'une localité"));
            connect(dlg_ask->OKButton,    &QPushButton::clicked, this, [=]  {
                                                                                if (Datas::I()->villes->enregistreNouvelleVille(Utils::trim(CP->text()), nomville) != Q_NULLPTR)
                                                                                {
                                                                                    delete complListVilles->model();
                                                                                    complListVilles         ->setModel(new QStringListModel(m_villes->ListeNomsVilles()));
                                                                                    delete complListCP->model();
                                                                                    complListCP             ->setModel(new QStringListModel(m_villes->ListeCodesPostaux()));
                                                                                    ChercheCPBaseIndividual(nomville);
                                                                                }
                                                                                dlg_ask                 ->close();
                                                                            });
            dlg_ask->dlglayout()    ->setSizeConstraint(QLayout::SetFixedSize);
            dlg_ask                 ->exec();
        }
        else
            ui->CPlineEdit->clear();
    }
    else
         ChercheCPBaseIndividual(nomville);
}

void VilleCPWidget::ChercheCPBaseIndividual(QString nomville)
{
    ui->CPlineEdit->clear();
    int idx = m_villes->ListeNomsVilles().indexOf(nomville);
    if (idx != -1)
    {
        QList<Ville *> listvilles = m_villes->getVilleByName(nomville);
        if (listvilles.size() > 0)
        {
            Ville * ville = listvilles.at(0);
            if (ville != Q_NULLPTR)
                ui->CPlineEdit->setText(ville->codepostal());
        }
    }
}
