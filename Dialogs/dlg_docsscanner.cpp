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

#include "dlg_docsscanner.h"

dlg_docsscanner::dlg_docsscanner(Item *item, int mode, QString titre, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionDocsScanner", parent)
{
    proc            = Procedures::I();
    gMode           = mode;
    if (gMode == Document)
        iditem = static_cast<Patient*>(item)->id();
    else
        iditem = static_cast<Depense*>(item)->id();
    db              = DataBase::I();
    QString         NomOnglet;
    QString Base;
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    AccesDistant = (db->getMode()==DataBase::Distant);

    NomDirStockageImagerie = proc->DirImagerie();
    switch (db->getMode()) {
    case DataBase::Poste:
    {
        NomOnglet = tr("Monoposte");
        Base = "BDD_POSTE";
        break;
    }
    case DataBase::Distant:
    {
        Base = "BDD_DISTANT";
        NomOnglet = tr("Accès distant");
        break;
    }
    case DataBase::ReseauLocal:
    {
        Base = "BDD_LOCAL";
        NomOnglet = tr("Réseau local");
        break;
    }
    default:
        break;
    }
    /* utilisé pour les tests en simulant un accès distant
    AccesDistant = true;
    Base = "BDD_LOCAL";*/

    if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
        msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Editions/Paramètres/Onglet \"ce poste\" /Onglet \"") + NomOnglet + "</b></font>";
        QStringList listmsg;
        listmsg << msg;
        dlg_message(listmsg, 6000, false);
        initOK = false;
        return;
    }

    docpath = proc->gsettingsIni->value(Base + "/DossiersDocsScannes").toString();
    if (!QDir(docpath).exists())
        docpath = QDir::homePath();
    uptable         = new UpTableWidget(this);
    inflabel        = new QLabel(uptable);
    linetitre       = new UpLineEdit(this);
    editdate        = new QDateEdit(this);
    typeDocCombo    = new UpComboBox(this);
    switch (gMode) {
    case Document:
        ListTypeExams   << COURRIER
                        << tr("CV")
                        << tr("Orthoptie")
                        << tr("ANGIO")
                        << tr("OCT")
                        << tr("Biométrie")
                        << tr("RNM")
                        << tr("Speculaire")
                        << tr("Topographie")
                        << tr("Hess-Weiss")
                        << tr("CRO")
                        << tr("Autre");
        break;
    case Facture:
        ListTypeExams   << FACTURE;
        break;
    case Echeancier:
        ListTypeExams   << ECHEANCIER;
        break;
    }
    toolbar         = new UpToolBar();
    dirsearchbutton = new UpPushButton();

    UpLabel         *lbltitre       = new UpLabel(this);
    UpLabel         *lbldate        = new UpLabel(this);
    UpLabel         *lbltype        = new UpLabel(this);
    QVBoxLayout     *rsgnmtVlay     = new QVBoxLayout();
    QVBoxLayout     *dirVlay        = new QVBoxLayout();
    QHBoxLayout     *dateLay        = new QHBoxLayout();
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    QHBoxLayout     *typeLay        = new QHBoxLayout();

    connect(toolbar,    &UpToolBar::TBSignal, this, [=] {NavigueVers(toolbar->choix());});

    uptable     ->setColumnCount(1);
    uptable     ->horizontalHeader()->setVisible(false);
    uptable     ->verticalHeader()->setVisible(false);
    uptable     ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    uptable     ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    toolbar     ->setMinimumHeight(30);
    dirsearchbutton->setFixedHeight(30);
    dirsearchbutton->setText(tr("Chercher un fichier"));

    typeDocCombo->insertItems(0,ListTypeExams);
    typeDocCombo->setEditable(false);
    linetitre->setValidator(new QRegExpValidator(Utils::rgx_intitulecompta));

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
    linetitre   ->setText(titre);
    editdate    ->setDate(QDate::currentDate());

    typeLay     ->addWidget(lbltype);
    typeLay     ->addWidget(typeDocCombo);
    titreLay    ->addWidget(lbltitre);
    titreLay    ->addWidget(linetitre);
    dateLay     ->addWidget(lbldate);
    dateLay     ->addWidget(editdate);
    rsgnmtVlay  ->addLayout(typeLay);
    rsgnmtVlay  ->addLayout(titreLay);
    rsgnmtVlay  ->addLayout(dateLay);
    rsgnmtVlay  ->setSpacing(2);
    dirVlay     ->addWidget(toolbar);
    dirVlay     ->addSpacerItem(new QSpacerItem(5,5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    dirVlay     ->addWidget(dirsearchbutton);
    dirVlay     ->setSpacing(2);
    titreLay    ->setSpacing(5);
    typeLay     ->setSpacing(5);
    dateLay     ->setSpacing(5);
    rsgnmtVlay  ->setContentsMargins(0,0,0,0);
    typeLay     ->setContentsMargins(0,0,0,0);
    titreLay    ->setContentsMargins(0,0,0,0);
    dateLay     ->setContentsMargins(0,0,0,0);

    dlglayout()   ->insertWidget(0,uptable);

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    connect(OKButton,           &QPushButton::clicked, this,   &dlg_docsscanner::ValideFiche);
    connect(dirsearchbutton,    &QPushButton::clicked, this,   &dlg_docsscanner::ChangeFile);

    buttonslayout()->insertLayout(0,rsgnmtVlay);

    buttonslayout()->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));

    buttonslayout()->insertLayout(0, dirVlay);
    uptable->resize(uptable->sizeHint());
    uptable->installEventFilter(this);
    setModal(true);
    setMinimumWidth(650);
    setStageCount(2);
    int w = width() - dlglayout()->contentsMargins().left() - dlglayout()->contentsMargins().right();
    int y = height() - dlglayout()->contentsMargins().top() - dlglayout()->contentsMargins().bottom() - dlglayout()->spacing()  - widgetbuttons()->height();
    uptable->resize(w, y);
    initOK = true;
    NavigueVers("Fin");
}

dlg_docsscanner::~dlg_docsscanner()
{
}

void dlg_docsscanner::NavigueVers(QString but)
{
    QStringList filters;
    filters << "*.pdf" << "*.jpg" << "*.jpeg";
    QStringList listfich = QDir(docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(this,tr("Il n'y a aucun document dans le dossier ") + docpath,
                             tr("Vous devez scanner les documents au format pdf ou jpg."));
        return;
    }
    int idx = listfich.indexOf(fichierimageencours);
    if (but == "Fin")
        idx = listfich.size()-1;
    else if (but == "Début")
        idx = 0;
    else if (but == "Suivant")
        idx += 1;
    else if (but == "Précédent")
        idx -= 1;
    toolbar->First()    ->setEnabled(idx>0);
    toolbar->Prec()     ->setEnabled(idx>0);
    toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
    toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
    if (idx>-1)
    {
        fichierimageencours = listfich.at(idx);
        QMap<QString,QVariant> doc;
        QFile       qFile(docpath + "/" + fichierimageencours);
        if (!qFile.open( QIODevice::ReadOnly ))
        {
            UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier"), qFile.fileName());
            return;
        }
        QByteArray bapdf = qFile.readAll();
        QString suffixe = QFileInfo(qFile).suffix().toLower();
        qFile.close ();
        doc["ba"]   = bapdf   ;
        doc["type"] = suffixe;
        glistImg = uptable->AfficheDoc(doc);
        inflabel    ->setText("<font color='magenta'>" + fichierimageencours + "</font>");
        QFont font = qApp->font();
        font.setPointSize(12);
        inflabel->setFont(font);
        Utils::Pause(10);  //force la maj de l'affichage sinon le calcul de la hauteur pour le placement du qlabel est faux
        inflabel    ->setGeometry(10,uptable->height()-40,350,25);
    }
}

void dlg_docsscanner::ChangeFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), docpath,  tr("Images (*.pdf *.jpg)"));
    if (fileName != "")
    {
        docpath = QFileInfo(fileName).dir().absolutePath();
        fichierimageencours = QFileInfo(fileName).fileName();
        QStringList filters;
        filters << "*.pdf" << "*.jpg";
        QStringList listfich = QDir(docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
        int idx = listfich.indexOf(fichierimageencours);
        toolbar->First()    ->setEnabled(idx>0);
        toolbar->Prec()     ->setEnabled(idx>0);
        toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
        toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
        QMap<QString,QVariant> doc;
        QFile       qFile(docpath + "/" + fichierimageencours);
        if (!qFile.open( QIODevice::ReadOnly ))
        {
            UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier"), qFile.fileName());
            return;
        }
        QByteArray bapdf = qFile.readAll();
        QString suffixe = QFileInfo(qFile).suffix().toLower();
        qFile.close ();
        doc["ba"]   = bapdf   ;
        doc["type"] = suffixe;
        glistImg = uptable->AfficheDoc(doc);
        inflabel    ->setText("<font color='magenta'>" + fichierimageencours + "</font>");
        QFont font = qApp->font();
        font.setPointSize(12);
        inflabel->setFont(font);
        inflabel    ->setGeometry(10,uptable->viewport()->height()-40,350,25);
        QString Base;
        if (db->getMode() == DataBase::Poste)
            Base = "BDD_POSTE";
        else if (db->getMode() == DataBase::ReseauLocal)
            Base = "BDD_LOCAL";
        else if (db->getMode() == DataBase::Distant)
            Base = "BDD_DISTANT";
        proc->gsettingsIni->setValue(Base + "/DossiersDocsScannes", docpath);
    }
}

QMap<QString, QVariant> dlg_docsscanner::getdataFacture()
{
    return datafacture;
}

bool dlg_docsscanner::getinitOK()
{
    return initOK;
}

void dlg_docsscanner::ValideFiche()
{
    if (typeDocCombo->currentText() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier le type de document"));
        typeDocCombo->setFocus();
        return;
    }
    if (linetitre->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un nom pour le document"));
        linetitre->setFocus();
        return;
    }
    if (editdate->date() == QDate::currentDate())
    {
        editdate->setFocus();
        UpMessageBox msgbox;
        msgbox.setText(tr("Confirmez la date d'aujourd'hui pour ce document"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKDateBouton;
        OKDateBouton.setText("Je confirme");
        UpSmallButton AnnulBouton;
        AnnulBouton.setText(tr("Annuler"));
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKDateBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKDateBouton)
            return;
    }

    // enregistrement du document ----------------------------------------------------------------------------------------------------------------------------------------------
    QString filename = docpath + "/" + fichierimageencours;
    QFile   qFileOrigin(filename);
    if (!qFileOrigin.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    QByteArray ba = qFileOrigin.readAll();
    QString suffixe = QFileInfo(qFileOrigin).suffix().toLower();
    if (suffixe == "jpeg")
        suffixe= "jpg";

    QString datetransfer = QDate::currentDate().toString("yyyy-MM-dd");
    QString user("");
    if (gMode != Document)
        user = Datas::I()->users->getLoginById(Datas::I()->depenses->getById(iditem)->iduser());
    QString CheminBackup = NomDirStockageImagerie + NOMDIR_ORIGINAUX + (gMode==Document? NOMDIR_IMAGES : NOMDIR_FACTURES) + "/" + (gMode==Document? datetransfer : user);
    Utils::mkpath(CheminBackup);
    qFileOrigin.copy(CheminBackup + "/" + fichierimageencours);

    QString CheminOKTransfrDir  = NomDirStockageImagerie + (gMode == Document? NOMDIR_IMAGES "/" + datetransfer : NOMDIR_FACTURES "/" + user) ;
    if (!Utils::mkpath(CheminOKTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        dlg_message(listmsg, 3000, false);
        return;
    }

    if (suffixe == "jpg" && qFileOrigin.size() > TAILLEMAXIIMAGES)
    {
        qFileOrigin.close();
        if (!Utils::CompressFileJPG(filename, proc->DirImagerie()))
            return;
        if (!qFileOrigin.open( QIODevice::ReadOnly ))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur d'accès au fichier:"), filename);
            return;
        }
        ba = qFileOrigin.readAll();
    }

    QString sstypedoc = linetitre->text();
    int idimpr (0);
    QHash<QString,QVariant> listbinds;
    bool b = false;
    bool ok;
    QString lien;

    if (gMode == Document)      // c'est un document scanné
    {
        idimpr =  db->selectMaxFromTable("idimpression", NOM_TABLE_IMPRESSIONS, ok) + 1;
        QString NomFileDoc = QString::number(iditem) + "_"
                + typeDocCombo->currentText() + "_"
                + sstypedoc.replace("/",".") + "_"                  // on fait ça pour que le / ne soit pas interprété comme un / de séparation de dossier dans le nom du fichier, ce qui planterait l'enregistrement
                + editdate->dateTime().toString("yyyy-MM-dd");
        lien = "/" + datetransfer + "/" + NomFileDoc + "-" + QString::number(idimpr) + "." + suffixe;
        if (!AccesDistant)
        {
            listbinds["idImpression"] =     idimpr;
            listbinds["idPat"] =            iditem;
            listbinds["TypeDoc"] =          typeDocCombo->currentText();
            listbinds["SousTypeDoc"] =      sstypedoc;
            listbinds["Titre"] =            typeDocCombo->currentText();
            listbinds["DateImpression"] =   editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
            listbinds["UserEmetteur"] =     db->getUserConnected()->id();
            listbinds["lienversfichier"] =  lien;
            listbinds["EmisRecu"] =         "1";
            listbinds["FormatDoc"] =        DOCUMENTRECU;
            listbinds["idLieu"] =           db->getUserConnected()->getSite()->id();
        }
        else
        {
            listbinds["idImpression"] =     idimpr;
            listbinds["idPat"] =            iditem;
            listbinds["TypeDoc"] =          typeDocCombo->currentText();
            listbinds["SousTypeDoc"] =      sstypedoc;
            listbinds["Titre"] =            typeDocCombo->currentText();
            listbinds["DateImpression"] =   editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
            listbinds["UserEmetteur"] =     db->getUserConnected()->id();
            listbinds[suffixe] =            ba;
            listbinds["EmisRecu"] =         "1";
            listbinds["FormatDoc"] =        DOCUMENTRECU;
            listbinds["idLieu"] =           db->getUserConnected()->getSite()->id();
        }
        b = db->InsertSQLByBinds(NOM_TABLE_IMPRESSIONS, listbinds);
    }
    else                        // c'est une facture ou un échéancier
    {
        idimpr =  db->selectMaxFromTable("idFacture", NOM_TABLE_FACTURES, ok) + 1;
        QString NomFileDoc = QString::number(idimpr) + "_"
                + typeDocCombo->currentText() + "_"
                + sstypedoc.replace("/",".") + "_"                  // on fait ça pour que le / ne soit pas interprété comme un / de séparation de dossier dans le nom du fichier, ce qui planterait l'enregistrement
                + editdate->dateTime().toString("yyyy-MM-dd");
        lien = "/" + user + "/" + NomFileDoc  + (gMode== Echeancier? "" : "-" + QString::number(idimpr)) +"." + suffixe;
        if (!AccesDistant)
        {
            listbinds["idFacture"] =        idimpr;
            listbinds["DateFacture"] =      editdate->date().toString("yyyy-MM-dd");
            listbinds["Intitule"] =         sstypedoc;
            listbinds["LienFichier"] =      lien;
            listbinds["Echeancier"] =       (gMode== Echeancier? "1" : QVariant(QVariant::String));
            listbinds["idDepense"] =        (gMode== Echeancier? QVariant(QVariant::String) : QString::number(iditem));
            datafacture["lien"] =           lien;
        }
        else
        {
            listbinds["idFacture"] =        idimpr;
            listbinds["DateFacture"] =      editdate->date().toString("yyyy-MM-dd");
            listbinds["Intitule"] =         sstypedoc;
            listbinds["Echeancier"] =       (gMode== Echeancier? "1" : QVariant(QVariant::String));
            listbinds["idDepense"] =        (gMode== Echeancier? QVariant(QVariant::String) : QString::number(iditem));
            listbinds[suffixe] =            ba;
            datafacture["lien"] =           "";
        }
        b = db->InsertSQLByBinds(NOM_TABLE_FACTURES, listbinds);
        datafacture["idfacture"] = idimpr;
        datafacture["echeancier"] = (gMode == Echeancier);
        datafacture["objetecheancier"] = (gMode == Echeancier? sstypedoc : "");
    }
    if(!b)
    {
        UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
        qFileOrigin.close ();
        reject();
        return;
    }
    else if (!AccesDistant)
    {
        QString CheminOKTransfrDoc = NomDirStockageImagerie + (gMode == Document? NOMDIR_IMAGES : NOMDIR_FACTURES) + lien;
        if (suffixe == JPG)
        {
            QFile CF(filename);
            CF.copy(CheminOKTransfrDoc);
        }
        else if (suffixe == PDF)
            qFileOrigin.copy(CheminOKTransfrDoc);
        QFile CC(CheminOKTransfrDoc);
        if (CC.open(QIODevice::ReadWrite))
            CC.setPermissions(QFileDevice::ReadOther
                              | QFileDevice::ReadGroup
                              | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                              | QFileDevice::ReadUser   | QFileDevice::WriteUser);
    }
    qFileOrigin.remove();
    QString msg;
    switch (gMode) {
    case Document:      msg = tr("Document ") + sstypedoc +  tr(" enregistré");     break;
    case Facture:       msg = tr("Facture ") + sstypedoc +  tr(" enregistrée");     break;
    case Echeancier:    msg = tr("Echeancier ") + sstypedoc +  tr(" enregistré");   break;
    }
    dlg_message(QStringList() << msg, 1000, false);
    accept();
}

bool dlg_docsscanner::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==uptable)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            uptable->setColumnWidth(0,uptable->width()-2);
            for (int i=0; i < uptable->rowCount(); i++)
            {
                UpLabel *lbl = static_cast<UpLabel*>(uptable->cellWidget(i,0));
                QPixmap  pix = QPixmap::fromImage(glistImg.at(i).scaled(uptable->width()-2,uptable->height()-2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
                lbl->setPixmap(pix);
                uptable->setRowHeight(i,pix.height());
            }
            inflabel    ->move(10,uptable->viewport()->height()-40);
        }
    }
    return QWidget::eventFilter(obj, event);
}

