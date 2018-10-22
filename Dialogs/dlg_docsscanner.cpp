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

#include "dlg_docsscanner.h"

dlg_docsscanner::dlg_docsscanner(Procedures *ProcAPasser, int idPat, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionDocsScanner", parent)
{
    proc            = ProcAPasser;
    idpat           = idPat;
    db              = DataBase::getInstance()->getDataBase();
    QString         NomOnglet;
    if (DataBase::getInstance()->getMode() == DataBase::Poste)
    {
        NomOnglet = tr("Monoposte");
        QSqlQuery dirquer("select dirimagerie from " NOM_TABLE_PARAMSYSTEME, db);
        dirquer.first();
        NomDirStockageImagerie = dirquer.value(0).toString();
    }
    if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
    {
        NomOnglet = tr("Réseau local");
        NomDirStockageImagerie  = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
    }
    if (DataBase::getInstance()->getMode() == DataBase::Distant)
    {
        NomOnglet = tr("Accès distant");
        NomDirStockageImagerie  = proc->gsettingsIni->value("BDD_DISTANT/DossierImagerie").toString();
    }

    if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
        msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Editions/Paramètres/Onglet \"ce poste\" /Onglet \"") + NomOnglet + "</b></font>";
        QStringList listmsg;
        listmsg << msg;
        proc->Message(listmsg, 6000, false);
        initOK = false;
        return;
    }
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    QString Base;
    AccesDistant = (DataBase::getInstance()->getMode()==DataBase::Distant);
    switch (DataBase::getInstance()->getMode()) {
    case DataBase::Poste:
        Base = "BDD_POSTE";
        break;
    case DataBase::Distant:
        Base = "BDD_DISTANT";
        break;
    case DataBase::ReseauLocal:
        Base = "BDD_LOCAL";
        break;
    default:
        break;
    }

    docpath = proc->gsettingsIni->value(Base + "/DossiersDocsScannes").toString();
    if (!QDir(docpath).exists())
        docpath = QDir::homePath();
    uptable         = new UpTableWidget(this);
    inflabel        = new UpLabel(uptable);
    linetitre       = new UpLineEdit(this);
    editdate        = new QDateEdit(this);
    typeDocCombo    = new UpComboBox(this);
    ListTypeExams   << tr("Courrier")
                    << tr("CV")
                    << tr("Orthoptie")
                    << tr("ANGIO")
                    << tr("OCT")
                    << tr("Biométrie")
                    << tr("RNM")
                    << tr("Speculaire")
                    << tr("Topographie")
                    << tr("Hess-Weiss")
                    << tr("Autre");
    toolbar         = new UpToolBar();
    dirsearchbutton = new UpPushButton();

    UpLabel         *lbltitre       = new UpLabel(this);
    UpLabel         *lbldate        = new UpLabel(this);
    UpLabel         *lbltype        = new UpLabel(this);
    QVBoxLayout     *globallay      = dynamic_cast<QVBoxLayout*>(layout());
    QVBoxLayout     *rsgnmtVlay     = new QVBoxLayout();
    QVBoxLayout     *dirVlay        = new QVBoxLayout();
    QHBoxLayout     *dateLay        = new QHBoxLayout();
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    QHBoxLayout     *typeLay        = new QHBoxLayout();

    connect(toolbar,    &UpToolBar::TBSignal, [=] {NavigueVers(toolbar->action);});

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

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
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

    globallay   ->insertWidget(0,uptable);

    QFont font = qApp->font();
    font.setPointSize(12);
    inflabel->setFont(font);

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    connect(OKButton,           &QPushButton::clicked, [=] {ValideFiche();});
    connect(dirsearchbutton,    &QPushButton::clicked, [=] {ChangeFile();});

    laybuttons->insertLayout(0,rsgnmtVlay);

    laybuttons->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));

    laybuttons->insertLayout(0, dirVlay);

    uptable->installEventFilter(this);
    setModal(true);
    setMinimumWidth(650);
    globallay->setStretch(0,10);
    globallay->setStretch(1,1);
    initOK = true;
}

dlg_docsscanner::~dlg_docsscanner()
{
}

void dlg_docsscanner::NavigueVers(QString but)
{
    QString fichencours = uptable->Attribut();
    QStringList filters;
    filters << "*.pdf" << "*.jpg";
    QStringList listfich = QDir(docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(this,tr("Il n'y a aucun document dans le dossier ") + docpath,
                             tr("Vous devez scanner les documents au format pdf."));
        return;
    }
    int idx = listfich.indexOf(fichencours);
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
        QString filebut = listfich.at(idx);
        //for (int i = 0; i<listfich.size();i++)
        //    qDebug() << listfich.at(i) + " - index = " + QString::number(i) + "/" + QString::number(listfich.size());
        AfficheDoc(filebut);
    }
}

void dlg_docsscanner::ChangeFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), docpath,  tr("Images (*.pdf *.jpg)"));
    if (fileName != "")
    {
        docpath = QFileInfo(fileName).dir().absolutePath();
        QString fichierencours = QFileInfo(fileName).fileName();
        QStringList filters;
        filters << "*.pdf" << "*.jpg";
        QStringList listfich = QDir(docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
        int idx = listfich.indexOf(fichierencours);
        toolbar->First()    ->setEnabled(idx>0);
        toolbar->Prec()     ->setEnabled(idx>0);
        toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
        toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
        AfficheDoc(fichierencours);
        QString Base;
        if (DataBase::getInstance()->getMode() == DataBase::Poste)
            Base = "BDD_POSTE";
        else if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
            Base = "BDD_LOCAL";
        else if (DataBase::getInstance()->getMode() == DataBase::Distant)
            Base = "BDD_DISTANT";
        proc->gsettingsIni->setValue(Base + "/DossiersDocsScannes", docpath);
    }
}

void dlg_docsscanner::AfficheDoc(QString filebut)
{
    glistPix    .clear();
    uptable     ->clear();
    uptable     ->setColumnCount(1);
    uptable     ->setColumnWidth(0,uptable->width()-2);
    uptable     ->horizontalHeader()->setVisible(false);
    uptable     ->verticalHeader()->setVisible(false);
    uptable     ->setAttribut(filebut);

    QPixmap     pix;
    QDir        dirpict = QDir(docpath);
    inflabel    ->setText("<font color='magenta'>" + filebut + "</font>");
    inflabel    ->setGeometry(10,uptable->height()-30,350,25);

    QFile       qFile(dirpict.filePath(filebut));
    if (!qFile.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier"), qFile.fileName());
        return;
    }
    QByteArray bapdf = qFile.readAll();
    QString suffixe = QFileInfo(qFile).suffix().toLower();
    qFile.close ();
    if (suffixe == "pdf")
    {
        Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
        if (!document || document->isLocked()) {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            delete document;
            return;
        }
        if (document == Q_NULLPTR) {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            delete document;
            return;
        }

        document->setRenderHint(Poppler::Document::TextAntialiasing);
        int numpages = document->numPages();
        uptable->setRowCount(numpages);
        for (int i=0; i<numpages ;i++)
        {
            Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
            if (pdfPage == Q_NULLPTR) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return;
            }
            QImage image = pdfPage->renderToImage(150,150);
            if (image.isNull()) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return;
            }
            // ... use image ...
            pix = QPixmap::fromImage(image).scaled(uptable->width()-2,uptable->height()-2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            glistPix << pix;
            uptable->setRowHeight(i,pix.height());
            UpLabel *lab = new UpLabel(uptable);
            lab->resize(pix.width(),pix.height());
            lab->setPixmap(pix);
            delete pdfPage;
            uptable->setCellWidget(i,0,lab);
        }
        delete document;
    }
    else if (suffixe=="jpg" || suffixe == "jpeg")
    {
        QImage image;
        if (!image.loadFromData(bapdf))
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
        pix = QPixmap::fromImage(image).scaled(uptable->width()-2,uptable->height()-2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
        glistPix << pix;
        UpLabel* labpix     = new UpLabel(uptable);
        labpix->setPixmap(pix);
        uptable->setRowCount(1);
        uptable->setRowHeight(0,pix.height());
        uptable->setCellWidget(0,0,labpix);
    }
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
        UpMessageBox *msgbox = new UpMessageBox(this);
        msgbox->setText(tr("Confirmez la date d'aujourd'hui pour ce document."));
        msgbox->setIcon(UpMessageBox::Warning);
        UpSmallButton *OKDateBouton = new UpSmallButton(tr("Je confirme"),msgbox);
        UpSmallButton *AnnulBouton = new UpSmallButton(tr("Annuler"), msgbox);
        msgbox->addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox->addButton(OKDateBouton, UpSmallButton::STARTBUTTON);
        msgbox->exec();
        if (msgbox->clickedButton() != OKDateBouton)
            return;
    }

    // enregistrement du document ----------------------------------------------------------------------------------------------------------------------------------------------
    QTextEdit txtedit;
    txtedit.setHtml(inflabel->text());
    QString filename = docpath + "/" + txtedit.toPlainText();
    QFile   qFile(filename);
    if (!qFile.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    QString suffixe = QFileInfo(qFile).suffix().toLower();
    if (suffixe == "jpeg")
        suffixe= "jpg";
    QByteArray bapdf = qFile.readAll();
    QString bbb = QDir::homePath() + NOMDIR_RUFUS;
    QString nomfichresize = bbb + "/resize" + "." + suffixe;
    qFile.copy(nomfichresize);
    QFile fich(nomfichresize);
    qint64 sz = fich.size();
    if (suffixe == "jpg" && sz > TAILLEMAXIIMAGES)
    {
        QImage img(filename);
        QPixmap pixmap;
        QFile fich(docpath + "/resize.jpg");
        int tauxcompress = 100;
        while (sz > TAILLEMAXIIMAGES && tauxcompress > 10)
        {
            pixmap = pixmap.fromImage(img.scaledToWidth(2560,Qt::SmoothTransformation));
            fich.remove();
            pixmap.save(docpath + "/resize.jpg", "jpeg",tauxcompress);
            fich.open(QIODevice::ReadWrite);
            sz = fich.size();
            if (sz > TAILLEMAXIIMAGES)
            {
                fich.close();
                tauxcompress -= 10;
            }
        }
        bapdf = fich.readAll();
    }
    QString datetransfer            = QDate::currentDate().toString("yyyy-MM-dd");
    QString CheminOKTransfrDir      = NomDirStockageImagerie + NOMDIR_IMAGES;
    QDir DirTrsferOK;
    if (!QDir(CheminOKTransfrDir).exists())
        if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            QStringList listmsg;
            listmsg << msg;
            proc->Message(listmsg, 3000, false);  // on shunte le thread de dlg_message
            return;
        }
    CheminOKTransfrDir      = CheminOKTransfrDir + "/" + datetransfer;
    if (!QDir(CheminOKTransfrDir).exists())
        if (!DirTrsferOK.mkdir(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            QStringList listmsg;
            listmsg << msg;
            proc->Message(listmsg, 3000, false);  // on shunte le thread de dlg_message
            return;
        }
    QString sstypedoc = linetitre->text();
    QString NomFileDoc = QString::number(idpat) + "_"
            + typeDocCombo->currentText() + "_"
            + sstypedoc + "_"
            + editdate->dateTime().toString("yyyy-MM-dd-HHmm");

    QSqlQuery ("LOCK TABLES '" NOM_TABLE_IMPRESSIONS "' WRITE", db);
    int idimpr(0);
    QSqlQuery maxquer("select max(idimpression) from " NOM_TABLE_IMPRESSIONS, db);
    if (maxquer.size()>0)
    {
        maxquer.first();
        idimpr = maxquer.value(0).toInt() + 1;
    }

    QSqlQuery query = QSqlQuery(db);
    if (!AccesDistant)
    {
        query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idimpression, idpat, TypeDoc, SousTypeDoc, Titre, Dateimpression, UserEmetteur, lienversfichier, EmisRecu, FormatDoc, idLieu)"
                                                           " values(:idimpr, :idpat, :typeDoc, :soustypedoc, :titre, :dateimpression, :useremetteur, :lien, :emisrecu, :formatdoc, :lieu)");
        query.bindValue(":idimpr",          QString::number(idimpr));
        query.bindValue(":idpat",           QString::number(idpat));
        query.bindValue(":typeDoc",         typeDocCombo->currentText());
        query.bindValue(":soustypedoc",     sstypedoc);
        query.bindValue(":titre",           typeDocCombo->currentText());
        query.bindValue(":dateimpression",  editdate->date().toString("yyyy-MM-dd") + " 00:00:00");
        query.bindValue(":useremetteur",    QString::number(DataBase::getInstance()->getUserConnected()->id()));
        query.bindValue(":lien",            "/" + datetransfer + "/" + NomFileDoc + "-" + QString::number(idimpr) + "." + suffixe);
        query.bindValue(":emisrecu",        "1");
        query.bindValue(":formatdoc",       DOCUMENTRECU);
        query.bindValue(":lieu",            QString::number(proc->getUserConnected()->getSite()->id()) );
    }
    else
    {
        query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idimpression, idpat,  TypeDoc,  SousTypeDoc,  Titre,  Dateimpression,  UserEmetteur," + suffixe + ", EmisRecu,  FormatDoc,  idLieu)"
                                                     " values(:idimpr,      :idpat, :typeDoc, :soustypedoc, :titre, :dateimpression, :useremetteur,   :doc,        :emisrecu, :formatdoc, :lieu)");
        query.bindValue(":idimpr",          QString::number(idimpr));
        query.bindValue(":idpat",           QString::number(idpat));
        query.bindValue(":typeDoc",         typeDocCombo->currentText());
        query.bindValue(":soustypedoc",     sstypedoc);
        query.bindValue(":titre",           typeDocCombo->currentText());
        query.bindValue(":dateimpression",  editdate->date().toString("yyyy-MM-dd") + " 00:00:00");
        query.bindValue(":useremetteur",    QString::number(DataBase::getInstance()->getUserConnected()->id()));
        query.bindValue(":doc",             bapdf);
        query.bindValue(":emisrecu",        "1");
        query.bindValue(":formatdoc",       DOCUMENTRECU);
        query.bindValue(":lieu",            QString::number(proc->getUserConnected()->getSite()->id()) );
    }

    bool b = query.exec();
    QSqlQuery("UNLOCK TABLES", db);
    if(!b)
    {
        UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
        qFile.close ();
    }
    else
    {
        QString CheminOKTransfrDoc      = CheminOKTransfrDir + "/" + NomFileDoc + "-" + QString::number(idimpr) + "." + suffixe;
        fich.copy(CheminOKTransfrDoc);
        fich.remove();
        QFile CC(CheminOKTransfrDoc);
        CC.open(QIODevice::ReadWrite);
        CC.setPermissions(QFileDevice::ReadOther
                          | QFileDevice::ReadGroup
                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        qFile.remove();
        proc->Message(tr("Document ") + sstypedoc +  tr(" enregistré"), 1000, false);
        close();
    }
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
                lbl->setPixmap(glistPix.at(i).scaled(uptable->width()-2, uptable->height()-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                uptable->setRowHeight(i,lbl->pixmap()->height());
            }
            inflabel->move(10,uptable->viewport()->height()-30);
        }
    }
    return QWidget::eventFilter(obj, event);
}

