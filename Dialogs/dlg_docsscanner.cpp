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

#include "dlg_docsscanner.h"

dlg_docsscanner::dlg_docsscanner(Item *item, Mode mode, QString titre, QWidget *parent) :
    UpDialog(PATH_FILE_INI, Position_Fiche Nom_fiche_DocsScanner, parent)
{
    m_mode           = mode;
    if ( m_mode == Document)
        m_iditem = static_cast<Patient*>(item)->id();
    else
        m_iditem = static_cast<Depense*>(item)->id();
    QString         NomOnglet;
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    Utils::ModeAcces mod = db->ModeAccesDataBase();
    if (mod == Utils::Poste)
        NomOnglet = tr("Monoposte");
    else if (mod == Utils::Distant)
        NomOnglet = tr("Accès distant");
    else if (mod == Utils::ReseauLocal)
        NomOnglet = tr("Réseau local");

    /* utilisé pour les tests en simulant un accès distant
    AccesDistant = true;
    Base = Utils::getBaseFromMode(Utils::ReseauLocal);*/
    m_accesdistant = (db->ModeAccesDataBase()==Utils::Distant);
    m_pathdirstockageimagerie = proc->AbsolutePathDirImagerie();

    if (!QDir(m_pathdirstockageimagerie).exists() || m_pathdirstockageimagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + m_pathdirstockageimagerie + "</b></font>" + tr(" n'existe pas");
        msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Editions/Paramètres/Onglet \"ce poste\" /Onglet \"") + NomOnglet + "</b></font>";
        ShowMessage::I()->SplashMessage(msg, 6000);
        m_initok = false;
        return;
    }

    m_docpath = proc->settings()->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_DocsScannes).toString();
    if (!QDir(m_docpath).exists())
        m_docpath = QDir::homePath();
    wdg_uptable         = new UpTableWidget(this);
    wdg_inflabel        = new QLabel(wdg_uptable);
    wdg_linetitre       = new UpLineEdit(this);
    wdg_editdate        = new QDateEdit(this);
    wdg_typedoccombobx  = new UpComboBox(this);
    switch ( m_mode) {
    case Document:
        m_listtypesexamen   << COURRIER
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
        m_listtypesexamen   << FACTURE;
        break;
    case Echeancier:
        m_listtypesexamen   << ECHEANCIER;
        break;
    }
    wdg_toolbar         = new UpToolBar();
    wdg_dirsearchbutton = new UpPushButton();

    UpLabel         *lbltitre       = new UpLabel(this);
    UpLabel         *lbldate        = new UpLabel(this);
    UpLabel         *lbltype        = new UpLabel(this);
    QVBoxLayout     *rsgnmtVlay     = new QVBoxLayout();
    QVBoxLayout     *dirVlay        = new QVBoxLayout();
    QHBoxLayout     *dateLay        = new QHBoxLayout();
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    QHBoxLayout     *typeLay        = new QHBoxLayout();

    connect(wdg_toolbar,    &UpToolBar::TBSignal, this, [=] {NavigueVers(wdg_toolbar->choix());});

    wdg_uptable     ->setColumnCount(1);
    wdg_uptable     ->horizontalHeader()->setVisible(false);
    wdg_uptable     ->verticalHeader()->setVisible(false);
    wdg_uptable     ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_uptable     ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_toolbar     ->setMinimumHeight(30);
    wdg_dirsearchbutton->setFixedHeight(30);
    wdg_dirsearchbutton->setText(tr("Chercher un fichier"));

    wdg_typedoccombobx->insertItems(0,m_listtypesexamen);
    wdg_typedoccombobx->setEditable(false);
    wdg_linetitre->setValidator(new QRegularExpressionValidator(Utils::rgx_intitulecompta));

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
    wdg_linetitre   ->setText(titre);
    wdg_editdate    ->setDate(m_currentdate);

    typeLay     ->addWidget(lbltype);
    typeLay     ->addWidget(wdg_typedoccombobx);
    titreLay    ->addWidget(lbltitre);
    titreLay    ->addWidget(wdg_linetitre);
    dateLay     ->addWidget(lbldate);
    dateLay     ->addWidget(wdg_editdate);
    rsgnmtVlay  ->addLayout(typeLay);
    rsgnmtVlay  ->addLayout(titreLay);
    rsgnmtVlay  ->addLayout(dateLay);
    rsgnmtVlay  ->setSpacing(2);
    dirVlay     ->addWidget(wdg_toolbar);
    dirVlay     ->addSpacerItem(new QSpacerItem(5,5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    dirVlay     ->addWidget(wdg_dirsearchbutton);
    dirVlay     ->setSpacing(2);
    titreLay    ->setSpacing(5);
    typeLay     ->setSpacing(5);
    dateLay     ->setSpacing(5);
    rsgnmtVlay  ->setContentsMargins(0,0,0,0);
    typeLay     ->setContentsMargins(0,0,0,0);
    titreLay    ->setContentsMargins(0,0,0,0);
    dateLay     ->setContentsMargins(0,0,0,0);

    dlglayout()   ->insertWidget(0,wdg_uptable);

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    connect(OKButton,           &QPushButton::clicked, this,   &dlg_docsscanner::ValideFiche);
    connect(wdg_dirsearchbutton,    &QPushButton::clicked, this,   &dlg_docsscanner::ChangeFile);

    buttonslayout()->insertLayout(0,rsgnmtVlay);

    buttonslayout()->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));

    buttonslayout()->insertLayout(0, dirVlay);
    wdg_uptable->resize(wdg_uptable->sizeHint());
    wdg_uptable->installEventFilter(this);
    setMinimumWidth(650);
    setStageCount(2);
    int w = width() - dlglayout()->contentsMargins().left() - dlglayout()->contentsMargins().right();
    int y = height() - dlglayout()->contentsMargins().top() - dlglayout()->contentsMargins().bottom() - dlglayout()->spacing()  - widgetbuttons()->height();
    wdg_uptable->resize(w, y);
    m_initok = true;
    NavigueVers(UpToolBar::_last);
}

dlg_docsscanner::~dlg_docsscanner()
{
}

void dlg_docsscanner::NavigueVers(UpToolBar::Choix choix)
{
    QStringList filters;
    filters << "*.pdf" << "*.jpg" << "*.jpeg";
    QStringList listfich = QDir(m_docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(this,tr("Il n'y a aucun document dans le dossier ") + m_docpath,
                             tr("Vous devez scanner les documents au format pdf ou jpg."));
        return;
    }
    int idx = listfich.indexOf(m_nomfichierimageencours);
    if (choix == UpToolBar::_last)
        idx = listfich.size()-1;
    else if (choix == UpToolBar::_first)
        idx = 0;
    else if (choix == UpToolBar::_next)
        idx += 1;
    else if (choix == UpToolBar::_prec)
        idx -= 1;
    wdg_toolbar->First()    ->setEnabled(idx>0);
    wdg_toolbar->Prec()     ->setEnabled(idx>0);
    wdg_toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
    wdg_toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
    if (idx>-1)
    {
        m_nomfichierimageencours = listfich.at(idx);
        QMap<QString,QVariant> doc;
        QFile       qFile(m_docpath + "/" + m_nomfichierimageencours);
        if (!qFile.open( QIODevice::ReadOnly ))
        {
            UpMessageBox::Watch(this,  tr("Erreur d'accès au fichier"), qFile.fileName());
            return;
        }
        QByteArray bapdf = qFile.readAll();
        QString suffixe = QFileInfo(qFile).suffix().toLower();
        qFile.close ();
        doc["ba"]   = bapdf   ;
        doc["type"] = suffixe;
        m_listimages = wdg_uptable->AfficheDoc(doc);
        wdg_inflabel    ->setText("<font color='magenta'>" + m_nomfichierimageencours + "</font>");
        QFont font = qApp->font();
        font.setPointSize(12);
        wdg_inflabel->setFont(font);
        Utils::Pause(10);  //force la maj de l'affichage sinon le calcul de la hauteur pour le placement du qlabel est faux
        wdg_inflabel    ->setGeometry(10,wdg_uptable->height()-40,350,25);
    }
}

void dlg_docsscanner::ChangeFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), m_docpath,  tr("Images (*.pdf *.jpg)"));
    if (fileName != "")
    {
        m_docpath = QFileInfo(fileName).dir().absolutePath();
        m_nomfichierimageencours = QFileInfo(fileName).fileName();
        QStringList filters;
        filters << "*.pdf" << "*.jpg";
        QStringList listfich = QDir(m_docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
        int idx = listfich.indexOf(m_nomfichierimageencours);
        wdg_toolbar->First()    ->setEnabled(idx>0);
        wdg_toolbar->Prec()     ->setEnabled(idx>0);
        wdg_toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
        wdg_toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
        QMap<QString,QVariant> doc;
        QFile       qFile(m_docpath + "/" + m_nomfichierimageencours);
        if (!qFile.open( QIODevice::ReadOnly ))
        {
            UpMessageBox::Watch(this,  tr("Erreur d'accès au fichier"), qFile.fileName());
            return;
        }
        QByteArray bapdf = qFile.readAll();
        QString suffixe = QFileInfo(qFile).suffix().toLower();
        qFile.close ();
        doc["ba"]   = bapdf   ;
        doc["type"] = suffixe;
        m_listimages = wdg_uptable->AfficheDoc(doc);
        wdg_inflabel    ->setText("<font color='magenta'>" + m_nomfichierimageencours + "</font>");
        QFont font = qApp->font();
        font.setPointSize(12);
        wdg_inflabel->setFont(font);
        wdg_inflabel    ->setGeometry(10,wdg_uptable->viewport()->height()-40,350,25);
        proc->settings()->setValue(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_DocsScannes, m_docpath);
    }
}

QMap<QString, QVariant> dlg_docsscanner::getdataFacture()
{
    return map_datafacture;
}

bool dlg_docsscanner::initOK() const
{
    return m_initok;
}

void dlg_docsscanner::ValideFiche()
{
    if (wdg_typedoccombobx->currentText() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier le type de document"));
        wdg_typedoccombobx->setFocus();
        return;
    }
    if (wdg_linetitre->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un nom pour le document"));
        wdg_linetitre->setFocus();
        return;
    }
    if (wdg_editdate->date() == m_currentdate)
    {
        wdg_editdate->setFocus();
        UpMessageBox msgbox(this);
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
    QString filename = m_docpath + "/" + m_nomfichierimageencours;
    QFile   qFileOrigin(filename);
    if (!qFileOrigin.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    QByteArray ba = qFileOrigin.readAll();
    QString suffixe = QFileInfo(qFileOrigin).suffix().toLower();
    if (suffixe == "jpeg")
        suffixe= "jpg";

    QString datetransfer = m_currentdate.toString("yyyy-MM-dd");
    QString user("");
    if ( m_mode != Document )                   //! il s'agit d'une facture ou d'un échéancier
    {
        int iduserdep = 0;
        if (Datas::I()->depenses->getById(m_iditem) != Q_NULLPTR)
            iduserdep = Datas::I()->depenses->getById(m_iditem)->iduser();
        if (Datas::I()->users->getById(iduserdep) != Q_NULLPTR)
            user = Datas::I()->users->getById(iduserdep)->login();
    }
    QString CheminBackup = m_pathdirstockageimagerie + NOM_DIR_ORIGINAUX + ( m_mode==Document? NOM_DIR_IMAGES : NOM_DIR_FACTURES) + "/" + ( m_mode==Document? datetransfer : user);
    Utils::mkpath(CheminBackup);
    qFileOrigin.copy(CheminBackup + "/" + m_nomfichierimageencours);

    QString CheminOKTransfrDir  = m_pathdirstockageimagerie + ( m_mode == Document? NOM_DIR_IMAGES "/" + datetransfer : NOM_DIR_FACTURES "/" + user) ;
    if (!Utils::mkpath(CheminOKTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
        ShowMessage::I()->SplashMessage(msg, 3000);
        return;
    }

    if (suffixe == "jpg" && qFileOrigin.size() > TAILLEMAXIIMAGES)
    {
        qFileOrigin.close();
        if (!Utils::CompressFileJPG(filename, proc->AbsolutePathDirImagerie()))
            return;
        if (!qFileOrigin.open( QIODevice::ReadOnly ))
        {
            UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
            return;
        }
        ba = qFileOrigin.readAll();
    }

    QString sstypedoc = wdg_linetitre->text();
    int idimpr (0);
    QHash<QString,QVariant> listbinds;
    bool b = false;
    bool ok;
    QString lien;

    if ( m_mode == Document)      // c'est un document scanné
    {
        DataBase::I()->locktables(QStringList() << TBL_DOCSEXTERNES);
        idimpr =  db->selectMaxFromTable(CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES, ok) + 1;
        QString NomFileDoc = QString::number(m_iditem) + "_"
                + wdg_typedoccombobx->currentText() + "_"
                + sstypedoc.replace("/",".") + "_"                  // on fait ça pour que le / ne soit pas interprété comme un / de séparation de dossier dans le nom du fichier, ce qui planterait l'enregistrement
                + wdg_editdate->dateTime().toString("yyyy-MM-dd");
        lien = "/" + datetransfer + "/" + NomFileDoc + "-" + QString::number(idimpr) + "." + suffixe;
//        QJsonValue val = QJsonValue::fromVariant(ba);
//        qDebug() << "val" << val;
        if (!m_accesdistant)
        {
            listbinds[CP_ID_DOCSEXTERNES] =               idimpr;
            listbinds[CP_IDPAT_DOCSEXTERNES] =            m_iditem;
            listbinds[CP_TYPEDOC_DOCSEXTERNES] =          wdg_typedoccombobx->currentText();
            listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES] =      sstypedoc;
            listbinds[CP_TITRE_DOCSEXTERNES] =            wdg_typedoccombobx->currentText();
            listbinds[CP_DATE_DOCSEXTERNES] =             wdg_editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
            listbinds[CP_IDEMETTEUR_DOCSEXTERNES] =       Datas::I()->users->userconnected()->id();
            listbinds[CP_LIENFICHIER_DOCSEXTERNES] =      lien;
            listbinds[CP_EMISORRECU_DOCSEXTERNES] =       "1";
            listbinds[CP_FORMATDOC_DOCSEXTERNES] =        DOCUMENTRECU;
            listbinds[CP_IDLIEU_DOCSEXTERNES] =           Datas::I()->sites->idcurrentsite();
        }
        else
        {
            if (suffixe == "pdf")
                suffixe = CP_PDF_DOCSEXTERNES;
            else if (suffixe== "jpg" || suffixe == "jpeg")
                suffixe = CP_JPG_DOCSEXTERNES;
            listbinds[CP_ID_DOCSEXTERNES] =               idimpr;
            listbinds[CP_IDPAT_DOCSEXTERNES] =            m_iditem;
            listbinds[CP_TYPEDOC_DOCSEXTERNES] =          wdg_typedoccombobx->currentText();
            listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES] =      sstypedoc;
            listbinds[CP_TITRE_DOCSEXTERNES] =            wdg_typedoccombobx->currentText();
            listbinds[CP_DATE_DOCSEXTERNES] =             wdg_editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
            listbinds[CP_IDEMETTEUR_DOCSEXTERNES] =       Datas::I()->users->userconnected()->id();
            //QJsonValue val = QJsonDocument::fromBinaryData(ba)[suffixe];
            //            if (val.isObject())
            //                QJsonDocument doc(val.toObject());
            //listbinds[suffixe] =                         val;
            listbinds[suffixe] =                          ba;
            listbinds[CP_EMISORRECU_DOCSEXTERNES] =       "1";
            listbinds[CP_FORMATDOC_DOCSEXTERNES] =        DOCUMENTRECU;
            listbinds[CP_IDLIEU_DOCSEXTERNES] =           Datas::I()->sites->idcurrentsite();
        }
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        b = (doc != Q_NULLPTR);
        delete doc;
        doc = Q_NULLPTR;
    }
    else                        // c'est une facture ou un échéancier
    {
        idimpr =  db->selectMaxFromTable(CP_ID_FACTURES, TBL_FACTURES, ok) + 1;
        QString NomFileDoc = QString::number(idimpr) + "_"
                + wdg_typedoccombobx->currentText() + "_"
                + sstypedoc.replace("/",".") + "_"                  // on fait ça pour que le / ne soit pas interprété comme un / de séparation de dossier dans le nom du fichier, ce qui planterait l'enregistrement
                + wdg_editdate->dateTime().toString("yyyy-MM-dd");
        lien = "/" + user + "/" + NomFileDoc  + ( m_mode== Echeancier? "" : "-" + QString::number(idimpr)) +"." + suffixe;
        if (!m_accesdistant)
        {
            listbinds[CP_ID_FACTURES] =               idimpr;
            listbinds[CP_DATEFACTURE_FACTURES] =      wdg_editdate->date().toString("yyyy-MM-dd");
            listbinds[CP_INTITULE_FACTURES] =         sstypedoc;
            listbinds[CP_LIENFICHIER_FACTURES] =      lien;
            listbinds[CP_ECHEANCIER_FACTURES] =       ( m_mode== Echeancier? "1" : QVariant(QMetaType::fromType<QString>()));
            listbinds[CP_IDDEPENSE_FACTURES] =        ( m_mode== Echeancier? QVariant(QMetaType::fromType<QString>()) : QString::number(m_iditem));
            map_datafacture["lien"] =                 lien;
        }
        else
        {
            listbinds[CP_ID_FACTURES] =               idimpr;
            listbinds[CP_DATEFACTURE_FACTURES] =      wdg_editdate->date().toString("yyyy-MM-dd");
            listbinds[CP_INTITULE_FACTURES] =         sstypedoc;
            listbinds[CP_ECHEANCIER_FACTURES] =       ( m_mode== Echeancier? "1" : QVariant(QMetaType::fromType<QString>()));
            listbinds[CP_IDDEPENSE_FACTURES] =        ( m_mode== Echeancier? QVariant(QMetaType::fromType<QString>()) : QString::number(m_iditem));
            listbinds[suffixe] =                      ba;
            map_datafacture["lien"] =                 "";
        }
        b = db->InsertSQLByBinds(TBL_FACTURES, listbinds);
        if(!b)
            UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
        map_datafacture["idfacture"] = idimpr;
        map_datafacture["echeancier"] = ( m_mode == Echeancier);
        map_datafacture["objetecheancier"] = ( m_mode == Echeancier? sstypedoc : "");
    }
    if(!b)
    {
        qFileOrigin.close ();
        reject();
        return;
    }
    else if (!m_accesdistant)
    {
        QString CheminOKTransfrDoc = m_pathdirstockageimagerie + ( m_mode == Document? NOM_DIR_IMAGES : NOM_DIR_FACTURES) + lien;
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
    switch ( m_mode) {
    case Document:      msg = tr("Document ") + sstypedoc +  tr(" enregistré");     break;
    case Facture:       msg = tr("Facture ") + sstypedoc +  tr(" enregistrée");     break;
    case Echeancier:    msg = tr("Echeancier ") + sstypedoc +  tr(" enregistré");   break;
    }
    UpSystemTrayIcon::I()->showMessage(tr("Messages"), msg, Icons::icSunglasses(), 1000);
    accept();
}

bool dlg_docsscanner::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==wdg_uptable)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            wdg_uptable->setColumnWidth(0,wdg_uptable->width()-2);
            for (int i=0; i < wdg_uptable->rowCount(); i++)
            {
                UpLabel *lbl = static_cast<UpLabel*>(wdg_uptable->cellWidget(i,0));
                QPixmap  pix = QPixmap::fromImage(m_listimages.at(i).scaled(wdg_uptable->width()-2,wdg_uptable->height()-2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
                lbl->setPixmap(pix);
                wdg_uptable->setRowHeight(i,pix.height());
            }
            wdg_inflabel    ->move(10,wdg_uptable->viewport()->height()-40);
        }
    }
    return QWidget::eventFilter(obj, event);
}

