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

#include "dlg_docsexternes.h"
#include "icons.h"

static inline double mmToInches(double mm) { return mm * 0.039370147; }

dlg_docsexternes::dlg_docsexternes(Procedures *ProcAPasser, int idpat, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionDocsExternes", parent)
{
    proc                = ProcAPasser;
    gidPatient          = idpat;

    ScrollTable         = new UpTableWidget(this);
    Scene               = new QGraphicsScene(this);
    GraphicView         = new QGraphicsView(Scene, this);
    ListDocsTreeView    = new QTreeView(this);
    inflabel            = new QLabel();
    QFont font          = qApp->font();
    font                .setPointSize(font.pointSize()+2);
    inflabel            ->setFont(font);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    QHBoxLayout *lay   = new QHBoxLayout();
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
    ScrollTable->horizontalHeader() ->setVisible(false);
    ScrollTable->verticalHeader()   ->setVisible(false);

    gFont = QApplication::font();
    int d=0;
#ifdef QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE
    d=2;
#endif
    gFont.setPointSize(gFont.pointSize()-d);
    ScrollTable         ->installEventFilter(this);
    GraphicView         ->installEventFilter(this);
    GraphicView         ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GraphicView         ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ScrollTable         ->setFocusPolicy(Qt::NoFocus);
    ScrollTable         ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    ScrollTable         ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMaximumHeight(qApp->desktop()->availableGeometry().height());
    ListDocsTreeView    ->setFixedWidth(185);
    ListDocsTreeView    ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    player              = new QMediaPlayer;

    lay                 ->addWidget(ListDocsTreeView,2);
    lay                 ->addWidget(ScrollTable,8);
    lay                 ->addWidget(GraphicView,8);
    lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay                 ->setSpacing(10);
    globallay           ->insertLayout(0,lay);

    playctrl            = new PlayerControls(player, this);
    connect (playctrl,  &PlayerControls::ctrl,      [=] {PlayerCtrl(playctrl->State());});
    connect (playctrl,  &PlayerControls::recfile,   [=] {EnregistreVideo();});
    AjouteWidgetLayButtons(playctrl, false);
    sw                  = new UpSwitch(this);
    AjouteWidgetLayButtons(sw, false);
    connect(sw, &UpSwitch::Bascule, [=] {BasculeTriListe(sw->PosSwitch());});
    AjouteLayButtons(UpDialog::ButtonSuppr | UpDialog::ButtonPrint);

    installEventFilter(this);

    gModeTri            = parDate;
    TreeQuery           = QSqlQuery(DataBase::getInstance()->getDataBase());
    initOK = true;
    RemplirTreeView();
    if(!initOK)
        return;
    ListDocsTreeView    ->setFont(gFont);
    ListDocsTreeView    ->setEditTriggers(QAbstractItemView::DoubleClicked);
    ListDocsTreeView    ->setSelectionMode(QAbstractItemView::SingleSelection);
    AvecPrevisu = proc  ->ApercuAvantImpression();
    /*Gestion des XML - exemple
    QString adressexml = QDir::homePath() + NOMDIR_RUFUS + "/XML/" + QString::number(idpat) + "/Exam_Data.xml";
    QFile xmldoc(adressexml);
    if (xmldoc.open(QIODevice::ReadOnly))
    {
        QDomDocument *docxml = new QDomDocument();
        docxml->setContent(&xmldoc);
        QDomElement xml = docxml->documentElement();
        QString info;
        for (int i=0; i<xml.childNodes().size(); i++)
            info += "Nom de la balise\t" + xml.childNodes().at(i).toElement().tagName() + "\n";
        proc->Edit(info);
    }*/

    MAJTreeViewTimer    = new QTimer;
    MAJTreeViewTimer    ->start(10000);
    connect(MAJTreeViewTimer,   SIGNAL(timeout()),   this,   SLOT(Slot_CompteNbreDocs()));
    // connect(MAJTreeViewTimer,   &QTimer::timeout, [=] {Slot_CompteNbreDocs();}); provoque un plantage aléatoire...
    int margemm         = proc->TailleTopMarge(); // exprimé en mm
    printer             = new QPrinter(QPrinter::HighResolution);
    printer             ->setFullPage(true);
    rect                = printer->paperRect();

    rect.adjust(mmToInches(margemm) * printer->logicalDpiX(),
                mmToInches(margemm) * printer->logicalDpiY(),
                -mmToInches(margemm) * printer->logicalDpiX(),
                -mmToInches(margemm) * printer->logicalDpiY());
    gMode               = Normal;
    connect(PrintButton,        &QPushButton::clicked, [=] {ImprimeDoc();});
    connect(SupprButton,        &QPushButton::clicked, [=] {SupprimeDoc();});

    ScrollTable     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //TODO : icon scaled : pxZoomIn 30,30
    GraphicView     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //TODO : icon scaled : pxZoomIn 30,30
    hdelta          = 0;
    wdelta          = 0;
    hdeltaframe     = 0;
    wdeltaframe     = 0;
}

dlg_docsexternes::~dlg_docsexternes()
{
    proc = 0;
    delete proc;
}

bool dlg_docsexternes::InitOK()
{
    return initOK;
}

void dlg_docsexternes::AfficheDoc(QModelIndex idx)
{
    if (gmodele->itemFromIndex(idx)->hasChildren())
    {
        PrintButton->setEnabled(false);
        SupprButton->setEnabled(false);
        return;
    }
    playctrl                    ->setVisible(false);
    PrintButton                 ->setVisible(true);
    PrintButton                 ->setEnabled(true);
    SupprButton                 ->setEnabled(true);
    QStandardItem *it           = gmodele->itemFromIndex(idx);
    QPixmap pix;
    glistPix    .clear();
    ScrollTable ->clear();
    ScrollTable ->setVisible(false);
    GraphicView ->setVisible(false);
    Scene       ->clear();
    double x;
    double y;

    QSqlQuery quer("select formatdoc, lienversfichier from " NOM_TABLE_IMPRESSIONS " where idimpression = " + it->accessibleDescription(), DataBase::getInstance()->getDataBase());
    quer.first();

    if (quer.value(0).toString() == VIDEO)  // le document est une video -> n'est pas stocké dans la base mais est un fichier sur le disque
    {
        if (DataBase::getInstance()->getMode() == DataBase::Distant)
        {
            UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
            return;
        }
        QString NomOnglet, NomDirStockageImagerie;
        if (DataBase::getInstance()->getMode() == DataBase::Poste)
        {
            NomOnglet = tr("Monoposte");
            NomDirStockageImagerie = proc->DirImagerie();
        }
        if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
        {
            NomOnglet = tr("Réseau local");
            NomDirStockageImagerie  = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
        }
        if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
        {
            QString msg = tr("Le dossier de sauvegarde d'imagerie ") + "<font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
            QString msg2 = tr("Renseignez un dossier valide dans Editions/Paramètres/Onglet \"ce poste\"/Onglet \"") + NomOnglet + "\"";
            UpMessageBox::Watch(this,msg, msg2);
            return;
        }
        QString filename = NomDirStockageImagerie + NOMDIR_VIDEOS "/" + quer.value(1).toString();
        QFile   qFile(filename);
        if (!qFile.open(QIODevice::ReadOnly))
        {
            UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
            return;
        }
        inflabel->setParent(GraphicView);

        gTypeDoc                = VIDEO;
        GraphicView             ->setVisible(true);
        videoItem               = new QGraphicsVideoItem;
        Scene                   ->addItem(videoItem);
        player                  ->setMedia(QUrl::fromLocalFile(filename));
        videoItem               ->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        player                  ->setVideoOutput(videoItem);
        playctrl                ->setPlayer(player);
        playctrl                ->setVisible(true);
        PrintButton             ->setVisible(false);
        x = videoItem->size().width();
        y = videoItem->size().height();
        idealproportion = x/y;
        videoItem               ->setSize(QSize(GraphicView->width(),GraphicView->height()));
        x = videoItem->size().width();
        y = videoItem->size().height();
        Scene->setSceneRect(1,1,x-1,y-1);
        player                  ->play();
    }
    else                                    // le document est une image ou un document écrit (ordonnance, certificat...)
    {
        bool pict = (quer.value(0).toString() == IMAGERIE || quer.value(0).toString() == DOCUMENTRECU);
        QMap<QString,QVariant> doc = CalcImage(it->accessibleDescription().toInt(), pict, true);
        QByteArray bapdf = doc.value("ba").toByteArray();
        if (doc.value("Type").toString() == JPG)     // le document est un JPG
        {
            inflabel->setParent(GraphicView);
            gTypeDoc                = JPG;
            GraphicView             ->setVisible(true);
            QImage image;
            if (!image.loadFromData(bapdf))
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            pix = QPixmap::fromImage(image).scaled(QSize(qApp->desktop()->availableGeometry().width(),
                                                         qApp->desktop()->availableGeometry().height()),
                                                   Qt::KeepAspectRatioByExpanding,
                                                   Qt::SmoothTransformation);
            x = pix.size().width();
            y = pix.size().height();
            idealproportion = x/y;
            glistPix << pix;
            pix = QPixmap::fromImage(image).scaled(QSize(GraphicView->width()-2,GraphicView->height()-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            Scene   ->addPixmap(pix);
            x = pix.size().width();
            y = pix.size().height();
            Scene   ->setSceneRect(1,1,x-1,y-1);
        }
        else if (doc.value("Type").toString() == PDF)     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage
        {
            Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
            if (!document || document->isLocked()) {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                delete document;
                return;
            }
            if (document == 0) {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                delete document;
                return;
            }
            inflabel->setParent(ScrollTable);
            gTypeDoc    = PDF;
            ScrollTable ->setVisible(true);
            ScrollTable ->setColumnCount(1);
            ScrollTable ->setColumnWidth(0,ScrollTable->width()-2);
            document->setRenderHint(Poppler::Document::TextAntialiasing);
            int numpages = document->numPages();
            for (int i=0; i<numpages ;i++)
            {
                ScrollTable->setRowCount(numpages);
                Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
                if (pdfPage == 0) {
                    UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                    delete document;
                    return;
                }
                QImage image = pdfPage->renderToImage(300,300);
                if (image.isNull()) {
                    UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                    delete document;
                    return;
                }
                // ... use image ...
                pix = QPixmap::fromImage(image).scaled(QSize(qApp->desktop()->availableGeometry().width(),
                                                             qApp->desktop()->availableGeometry().height()),
                                                       Qt::KeepAspectRatioByExpanding,
                                                       Qt::SmoothTransformation);
                if (i==0)
                {
                    x = pix.size().width();
                    y = pix.size().height();
                    idealproportion = x/y;
                }
                glistPix << pix;
                pix = QPixmap::fromImage(image).scaled(QSize(ScrollTable->width()-2,ScrollTable->height()-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
                ScrollTable->setRowHeight(i,pix.height());
                UpLabel *lab = new UpLabel(ScrollTable);
                lab->resize(pix.width(),pix.height());
                lab->setPixmap(pix);
                connect(lab,    &UpLabel::clicked, [=] {ZoomDoc();});
                delete pdfPage;
                ScrollTable->setCellWidget(i,0,lab);
            }
            delete document;
        }
    }
    if (gMode == Zoom)
    {
        // les dimensions maxi de la zone de visu
        const double maxwscroll  = qApp->desktop()->availableGeometry().width()*2/3    - wdelta - wdeltaframe;
        const double maxhscroll  = qApp->desktop()->availableGeometry().height()       - hdelta - hdeltaframe;
        // les dimensions calculées de la zone de visu
        int wfinal(0), hfinal(0);

        double proportion = maxwscroll/maxhscroll;
        if (idealproportion > proportion)
        {   wfinal  = maxwscroll;   hfinal  = wfinal / idealproportion; }
        else
        {   hfinal  = maxhscroll;   wfinal  = hfinal * idealproportion; }

        int w = wfinal + wdelta;
        int h = hfinal + hdelta;
        resize(w, h);
        if (gTypeDoc == PDF)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(glistPix.at(i).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    ScrollTable->setRowHeight(i,hfinal-2);
                    ScrollTable->setColumnWidth(0,wfinal-2);
                    if (i==0) ScrollTable->resize(wfinal, hfinal);
                }
            }
        }
        else if (gTypeDoc == VIDEO)
        {
            GraphicView ->resize(wfinal, hfinal);
            videoItem   ->setSize(QSize(wfinal-2, hfinal-2));
            Scene->setSceneRect(1,1,wfinal-1,hfinal-1);
        }
        else if (gTypeDoc == JPG)
        {
            GraphicView ->resize(wfinal, hfinal);
            Scene->clear();
            QPixmap pix = glistPix.at(0).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            Scene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            Scene->setSceneRect(1,1,x-1,y-1);
        }

        if ((w + wdeltaframe) > (qApp->desktop()->availableGeometry().width() - this->x()))
            move(qApp->desktop()->availableGeometry().width() - (w + wdeltaframe), 0);
    }
    if (gTypeDoc == PDF)
        inflabel    ->setGeometry(10,ScrollTable->viewport()->height()-40, 500, 25);
    else
        inflabel    ->setGeometry(10,GraphicView->height() -40, 500, 25);
}

void dlg_docsexternes::BasculeTriListe(int a)
{
    QString             idimpraretrouver = "";
    gmodele = dynamic_cast<QStandardItemModel*>(ListDocsTreeView->model());
    if (gmodele)
    {
        if (ListDocsTreeView->selectionModel()->selectedIndexes().size()>0)
        {
            QModelIndex         actifidx;
            actifidx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
            if (!gmodele->itemFromIndex(actifidx)->hasChildren())
                idimpraretrouver = gmodele->itemFromIndex(actifidx)->accessibleDescription();
        }
    }
    if (a == 0)
    {
        gModeTri = parDate;
        gmodele  = gmodeleTriParDate;
    }
    else if (a == 1)
    {
        gModeTri = parType;
        gmodele = gmodeleTriParType;
    }
    ListDocsTreeView->setModel(gmodele);

    int nrows = gmodele->item(gmodele->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item date
    QStandardItem *item =  gmodele->item(gmodele->rowCount()-1)->child(nrows,0);    // le tout dernier item
    QModelIndex idx = item->index();                                                // l'index de ce dernier item
    if (idimpraretrouver != "")
    {
        bool nouvelidx = false;
        for (int m = 0; m<gmodele->rowCount(); m++)
        {
            for (int n=0; n<gmodele->item(m)->rowCount(); n++)
                if (gmodele->item(m)->child(n)->accessibleDescription() == idimpraretrouver)
                {
                    idx = gmodele->item(m)->child(n)->index();
                    nouvelidx = true;
                    break;
                }
            if (nouvelidx) break;
        }
    }
    ListDocsTreeView->scrollTo(idx, QAbstractItemView::EnsureVisible);
    ListDocsTreeView->setCurrentIndex(idx);
    ListDocsTreeView->expandAll();
    connect(gmodele,    &QStandardItemModel::itemChanged, [=] {EditSousTitre(gmodele->itemFromIndex(ListDocsTreeView->selectionModel()->currentIndex()));});
    connect(ListDocsTreeView->selectionModel(), &QItemSelectionModel::currentChanged, [=] {AfficheDoc(ListDocsTreeView->selectionModel()->currentIndex());});
}

void dlg_docsexternes::Slot_CompteNbreDocs()
{
    int ndocs = CompteNbreDocs();
    if (ndocs == 0)
        reject();
    if (nbredocs != ndocs)
    {
        nbredocs = ndocs;
        RemplirTreeView();
    }
}

void dlg_docsexternes::EditSousTitre(QStandardItem *item)
{
    QString txt = (item->text().contains(" - ")? item->text().split(" - ").at(1) : item->text());
    QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set soustypedoc = '" + txt + "' where idimpression = " + item->accessibleDescription(), DataBase::getInstance()->getDataBase());
    QSqlQuery quer("select typedoc, dateimpression from " NOM_TABLE_IMPRESSIONS " where idimpression = " + item->accessibleDescription(), DataBase::getInstance()->getDataBase());
    quer.first();
    if (gModeTri == parDate)
        item->setText(quer.value(0).toString() + " - " + txt);
    else
        item->setText(quer.value(1).toDate().toString(tr("d-M-yyyy")) + " - " + txt);
    QString sstitre = "<font color='magenta'>" + quer.value(1).toDate().toString(tr("d-M-yyyy")) + " - " + quer.value(0).toString() + " - " + txt + "</font>";
    inflabel    ->setText(sstitre);
    qDebug() << "Sous-titre = " + sstitre;
}

void dlg_docsexternes::EnregistreVideo()
{
    QString filename = player->media().canonicalUrl().path();
    QFileDialog dialog(this, tr("Enregistrer un fichier"), QDir::homePath());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        QFile(filename).copy(dockdir.path() + "/" + player->media().canonicalUrl().fileName());
    }
}

bool dlg_docsexternes::EcritDansUnFichier(QString NomFichier, QByteArray TexteFichier)
{
    //syntaxe = EcritDansUnFichier(QDir::homePath()+ "/Documents/test.txt", texte);
    QDir DirRssces;
    if (!DirRssces.exists(QDir::homePath() + "/Documents/Rufus/Ressources"))
        DirRssces.mkdir(QDir::homePath() + "/Documents/Rufus/Ressources");
    QFile testfile(NomFichier);
    if (!testfile.open(QIODevice::ReadWrite))
    {
        UpMessageBox::Watch(0, tr("Impossible d'ouvrir le fichier\n") + NomFichier);
        return false;
    }
    QTextStream out(&testfile);
    out << TexteFichier;
    return true;
}

QMap<QString,QVariant> dlg_docsexternes::CalcImage(int idimpression, bool imagerie, bool afficher)
{
    /* la fonction est appelée par Slot_AfficheDoc(), on utilise la table impressions
     *      pour afficher un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour afficher un document d'imagerie stocké directement dans la base, dans la table impressions - on va extraire le ByteArray directement de la base, de la table impressions
     * la fonction est applée par ImprimeDoc() - on utilise la table echangeimages
     *      pour imprimer un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour imprimer un document d'imagerie stocké dans la table echangeimages - on va extraire le ByteArray directement de la base, de la table echangeimages
    */

    QMap<QString,QVariant> result;
    QString idimpr = QString::number(idimpression);
    QString filename = "";
    QByteArray bapdf;
    result["Type"]    = "";
    result["ba"]      = QByteArray("");
    if (imagerie)
    {
        if (afficher)
        {
            QSqlQuery quer("select typedoc, dateimpression, soustypedoc, formatdoc, lienversfichier, compression from " NOM_TABLE_IMPRESSIONS
                           " where idimpression = " + idimpr,
                           DataBase::getInstance()->getDataBase());
            quer.first();
            QString sstitre = "<font color='magenta'>" + quer.value(1).toDate().toString(tr("d-M-yyyy")) + " - " + quer.value(0).toString() + " - " + quer.value(2).toString() + "</font>";
            inflabel    ->setText(sstitre);
            filename = quer.value(4).toString();
            if (filename != "")
            {
                QString filesufx;
                if (filename.contains("."))
                {
                    QStringList lst = filename.split(".");
                    filesufx        = lst.at(lst.size()-1);
                }
                QString sfx = (filesufx == PDF? PDF : JPG);
                QString imgs = "select idimpression from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr + " and (pdf is not null or jpg is not null)";
                //qDebug() << imgs;
                if (QSqlQuery (imgs, DataBase::getInstance()->getDataBase()).size()==0)
                {
                    QSqlQuery ("delete from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr, DataBase::getInstance()->getDataBase());
                    QString req = "INSERT INTO " NOM_TABLE_ECHANGEIMAGES " (idimpression, " + sfx + ", compression) "
                                "VALUES (" +
                                idimpr + ", " +
                                " LOAD_FILE('" + proc->DirImagerie() + NOMDIR_IMAGES + filename + "'), " +
                                quer.value(5).toString() + ")";
                    //qDebug() << req;
                    QSqlQuery (req, DataBase::getInstance()->getDataBase());
                }
            }
        }
        QSqlQuery ChercheImgquer( DataBase::getInstance()->getDataBase());
        QString req2 = "select pdf, jpg, compression  from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr;
        ChercheImgquer.exec(req2);
        if (ChercheImgquer.size()==0)
        {
            req2 = "select pdf, jpg, compression  from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr;
            ChercheImgquer.exec(req2);
        }
        if (ChercheImgquer.size()==0)
            return result;
        ChercheImgquer.first();
        if (ChercheImgquer.value(0).toByteArray().size()>0)                                                 // c'est un pdf
        {
            if (ChercheImgquer.value(2).toString()=="1")
                bapdf.append(qUncompress(ChercheImgquer.value(0).toByteArray()));
            else
                bapdf.append(ChercheImgquer.value(0).toByteArray());
            result["Type"]    = PDF;
            result["ba"]      = bapdf;
        }
        else if (ChercheImgquer.value(1).toByteArray().size()>0)                                            // c'est un jpg
        {
            bapdf.append(ChercheImgquer.value(1).toByteArray());
            result["Type"]    = JPG;
            result["ba"]      = bapdf;
        }
    }
    else                                                                                                    // il s'agit d'un document écrit, on le traduit en pdf et on l'affiche
    {
        QString req = "select TextEntete, TextCorps, TextPied, ALD, formatdoc from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr;
        QSqlQuery quer0(req,DataBase::getInstance()->getDataBase());
        if (quer0.size() == 0) {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            return result;
        }
        inflabel    ->setText("");
        quer0.first();
        QByteArray bapdf;
        QString Entete  = quer0.value(0).toString();
        QString Corps   = quer0.value(1).toString();
        QString Pied    = quer0.value(2).toString();
        QTextEdit   *Etat_textEdit = new QTextEdit;
        Etat_textEdit->setHtml(Corps);
        TextPrinter *TexteAImprimer = new TextPrinter();
        if (quer0.value(4).toString() == PRESCRIPTIONLUNETTES)
            TexteAImprimer->setFooterSize(proc->TaillePieddePageOrdoLunettes());
        else
            TexteAImprimer->setFooterSize(proc->TaillePieddePage());
        TexteAImprimer->setHeaderText(Entete);
        int TailleEnTete = proc->TailleEnTete();
        if (quer0.value(3).toString()=="1") TailleEnTete = proc->TailleEnTeteALD();
        TexteAImprimer->setHeaderSize(TailleEnTete);
        TexteAImprimer->setFooterText(Pied);
        TexteAImprimer->setTopMargin(proc->TailleTopMarge());
        TexteAImprimer->print(Etat_textEdit->document(), NOMFIC_PDF, "", false, true);
        // le paramètre true de la fonction print() génère la création du fichier pdf NOMFIC_PDF et pas son impression
        QString ficpdf = QDir::homePath() + NOMFIC_PDF;
        QFile filepdf(ficpdf);
        if (!filepdf.open( QIODevice::ReadOnly ))
            UpMessageBox::Watch(0,  tr("Erreur d'accès au fichier:\n") + ficpdf, tr("Impossible d'enregistrer l'impression dans la base"));
        bapdf = filepdf.readAll();
        filepdf.close ();
        result["Type"]    = PDF;
        result["ba"]      = bapdf;
    }
    return result;
}

int dlg_docsexternes::CompteNbreDocs()
{
    return QSqlQuery("Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient),DataBase::getInstance()->getDataBase()).size();
}

void dlg_docsexternes::ImprimeDoc()
{
#ifndef QT_NO_PRINTER
    disconnect(PrintButton,  0,0,0);
    QModelIndex idx             = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
    QStandardItemModel *modele  = static_cast<QStandardItemModel*>(ListDocsTreeView->model());
    QString idimpr              = modele->itemFromIndex(idx)->accessibleDescription();
    AvecPrevisu                 = proc->ApercuAvantImpression();
    if (idimpr != "")
    {
        bool detruirealafin = false;
        QSqlQuery quer1("select Typedoc, DateImpression, TextOrigine, formatdoc from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr, DataBase::getInstance()->getDataBase());
        quer1.first();

        UpMessageBox msgbox;
        UpSmallButton *OKBouton = new UpSmallButton();
        UpSmallButton *NoBouton = new UpSmallButton();
        UpSmallButton *ImpBouton = new UpSmallButton();
        UpSmallButton *AnnulBouton = new UpSmallButton();
        msgbox.setText(tr("Réimprimer un document"));
        msgbox.setIcon(UpMessageBox::Print);

        AnnulBouton->setText(tr("Annuler"));
        msgbox.addButton(AnnulBouton,UpSmallButton::CANCELBUTTON);
        if (QDate::currentDate() > quer1.value(1).toDate())
        {
            if (proc->getDataUser()->isMedecin())
            {
                if (quer1.value(3).toString() != IMAGERIE && quer1.value(3).toString() != DOCUMENTRECU)
                {
                    if (quer1.value(2).toString() != "")    // si on a un texte d'origine, on peut modifier le document
                    {
                        NoBouton->setText(tr("Modifier et imprimer\nà la date d'aujourd'hui"));
                        msgbox.addButton(NoBouton,UpSmallButton::EDITBUTTON);
                    }
                    ImpBouton->setText(tr("Réimprimer à\nla date d'aujourd'hui"));
                    msgbox.addButton(ImpBouton,UpSmallButton::COPYBUTTON);
                }
            }
        }
        else if (QDate::currentDate() == quer1.value(1).toDate())
        {
            if (proc->getDataUser()->isMedecin())
                if (quer1.value(3).toString() != IMAGERIE && quer1.value(3).toString() != DOCUMENTRECU)
                {
                    {
                        NoBouton->setText(tr("Modifier\net imprimer"));
                        msgbox.addButton(NoBouton,UpSmallButton::EDITBUTTON);
                        detruirealafin = true;
                    }
                }
        }
        OKBouton->setText(tr("Réimprimer\nle document"));
        msgbox.addButton(OKBouton, UpSmallButton::PRINTBUTTON);
        msgbox.exec();

        //Reimpression simple du document, sans réédition => pas d'action sur la BDD
        if (msgbox.clickedButton() == OKBouton)
        {
            bool pict = (quer1.value(3).toString() == IMAGERIE || quer1.value(3).toString() == DOCUMENTRECU);
            QMap<QString,QVariant> doc = CalcImage(idimpr.toInt(), pict, false);
            QByteArray bapdf = doc.value("ba").toByteArray();
            if (doc.value("Type").toString() == PDF)     // le document est un pdf ou un document texte
            {
                Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
                if (!document || document->isLocked()) {
                    UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                    delete document;
                    return;
                }
                if (document == 0) {
                    UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                    delete document;
                    return;
                }

                document->setRenderHint(Poppler::Document::TextAntialiasing);
                int numpages = document->numPages();
                for (int i=0; i<numpages ;i++)
                {
                    Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
                    if (pdfPage == 0) {
                        UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                        delete document;
                        return;
                    }
                    image = pdfPage->renderToImage(600,600);
                    if (image.isNull()) {
                        UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                        delete document;
                        return;
                    }
                    // ... use image ...
                    if (i == 0)
                    {
                        if (AvecPrevisu)
                        {
                            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
                            connect(dialog, &QPrintPreviewDialog::paintRequested, [=] {Print(printer);});
                            dialog->exec();
                            delete dialog;
                        }
                        else
                        {
                            QPrintDialog *dialog = new QPrintDialog(printer, this);
                            if (dialog->exec() != QDialog::Rejected)
                                Print(printer);
                            delete dialog;
                            AvecPrevisu = proc->ApercuAvantImpression();
                        }
                    }
                    else
                        Print(printer);
                    delete pdfPage;
                }
                delete document;
            }
            if (doc.value("Type").toString() == JPG)     // le document est un jpg
            {
                QPixmap pix;
                pix.loadFromData(bapdf);
                image= pix.toImage();
                if (AvecPrevisu)
                {
                    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
                    connect(dialog, &QPrintPreviewDialog::paintRequested, [=] {Print(printer);});
                    dialog->exec();
                    delete dialog;
                }
                else
                {
                    QPrintDialog *dialog = new QPrintDialog(printer, this);
                    if (dialog->exec() != QDialog::Rejected)
                        Print(printer);
                    delete dialog;
                    AvecPrevisu = proc->ApercuAvantImpression();
                }
            }
        }

        //Réédition d'un document - ne concerne que les courriers et ordonnances émis => on enregistre le nouveau document dans la BDD
        else if (msgbox.clickedButton() == NoBouton
                 || msgbox.clickedButton() == ImpBouton)
        {
            // reconstruire le document en refaisant l'entête et en récupérant le corps et le pied enregistrés dans la base
            QString req = "select idUser, Titre, TypeDoc, TextEntete, TextCorps, TextOrigine, TextPied, ALD, FormatDoc, SousTypeDoc from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr;
            QSqlQuery quer(req,DataBase::getInstance()->getDataBase());
            if (quer.size() == 0) {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }
            quer.first();
            QString     Corps, Entete, Pied, txt;
            QTextEdit   *Etat_textEdit = new QTextEdit;
            bool        AvecNumPage = false;
            bool        aa;

            gDataUser = proc->setDataOtherUser(quer.value(0).toInt());
            if (gDataUser == nullptr)
            {
                UpMessageBox::Watch(this,tr("Impossible de retrouver l'utilisateur"));
                return;
            }

            //création de l'entête
            Entete = quer.value(3).toString();
            if (Entete == "")
            {
                UpMessageBox::Watch(this,tr("Impossible de retrouver l'entête"));
                return;
            }
            Entete.replace(QRegExp("<!--date-->[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùüûÙçÇ'a-zA-ZŒœ0-9°, -]*<!--date-->"), "<!--date-->" + gDataUser->getVille() + tr(" le, ") + QDate::currentDate().toString(tr("d MMMM yyyy")) + "<!--date-->");
            //création du pied
            Pied = quer.value(6).toString();
            if (Pied == "")
            {
                UpMessageBox::Watch(this,tr("Impossible de retrouver le pied"));
                return;
            }

            // creation du corps de l'ordonnance
            QString txtautiliser    = (quer.value(5).toString() == QString()?           quer.value(4).toString()        : quer.value(5).toString());
            bool ALD                = (quer.value(7).toInt()==1);
            txt                     = (msgbox.clickedButton() == NoBouton?              proc->Edit(txtautiliser)        : txtautiliser);
            Corps                   = (quer.value(2).toString()==tr("Prescription")?    proc->ImpressionCorps(txt,ALD)  : proc->ImpressionCorps(txt));
            Etat_textEdit           ->setHtml(Corps);
            if (Etat_textEdit->toPlainText() == "")
            {   UpMessageBox::Watch(this,tr("Rien à imprimer"));    return; }
            int TailleEnTete        = (ALD?                                             proc->TailleEnTeteALD()      : proc->TailleEnTete());

            bool AvecDupli          = (proc->gsettingsIni->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES" && quer.value(2).toString() == tr("Prescription"));
            bool AvecChoixImprimante = true;
            aa = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                                    proc->TaillePieddePage(), TailleEnTete, proc->TailleTopMarge(),
                                    AvecDupli, AvecPrevisu, AvecNumPage, AvecChoixImprimante);

            // stockage du document dans la base de donnees - table impressions
            if (aa)
            {
                bool a = true;
                while (a) {
                    // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                    int debut = Corps.lastIndexOf("<p");
                    int fin   = Corps.lastIndexOf("</p>");
                    int longARetirer = fin - debut + 4;
                    if (Corps.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                        Corps.remove(debut,longARetirer);
                    else a = false;
                }
                Corps.replace("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
                Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");

                QSqlQuery query = QSqlQuery(DataBase::getInstance()->getDataBase());
                // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
                query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete, TextCorps, TextOrigine, TextPied, Dateimpression, FormatDoc, idLieu)"
                                                                   " values(:iduser, :idpat, :typeDoc, :soustypedoc, :titre, :textEntete, :textCorps, :textOrigine, :textPied, :dateimpression, :formatdoc, :idlieu)");
                query.bindValue(":iduser", quer.value(0).toString());
                query.bindValue(":idpat", QString::number(gidPatient));
                query.bindValue(":typeDoc", quer.value(2).toString());
                query.bindValue(":soustypedoc", quer.value(9).toString());
                query.bindValue(":titre", quer.value(1).toString());
                query.bindValue(":textEntete", Entete);
                query.bindValue(":textCorps", Corps);
                query.bindValue(":textOrigine", txt);
                query.bindValue(":textPied", Pied);
                query.bindValue(":dateimpression", QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
                query.bindValue(":formatdoc", quer.value(8).toString());
                query.bindValue(":idlieu", QString::number(proc->getDataUser()->getIdLieu()));
                if(!query.exec())
                    UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
                RemplirTreeView();
                if (detruirealafin)
                    QSqlQuery("delete from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr, DataBase::getInstance()->getDataBase());
            }
            delete Etat_textEdit;
        }
        delete OKBouton;
        delete NoBouton;
        delete AnnulBouton;
        msgbox.close();
    }
    connect(PrintButton,        &QPushButton::clicked, [=] {ImprimeDoc();});
#endif
}

void dlg_docsexternes::PlayerCtrl(int ctrl)
{
    switch (ctrl){
    case 0: player->stop();     break;
    case 1: player->pause();    break;
    case 2:
        player     ->setVideoOutput(videoItem);
        player->play();
        break;
    case 3:
        player  ->stop();
        player  ->setVideoOutput(videoItem);
        player  ->play();
        break;
    }
}

void dlg_docsexternes::Print(QPrinter *Imprimante)
{
    QPainter PrintingPreView(Imprimante);
    QPixmap pix         = QPixmap::fromImage(image).scaledToWidth(rect.width(),Qt::SmoothTransformation);
    PrintingPreView.drawImage(QPoint(0,0),pix.toImage());
}

void dlg_docsexternes::SupprimeDoc()
{
    int ndocs = CompteNbreDocs();
    QModelIndex idx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
    QString idimpr = gmodele->itemFromIndex(idx)->accessibleDescription();
    if (!proc->getDataUser()->isSoignant())         //le user n'est pas un soignant
    {
        QString     req = "Select Useremetteur, idrefraction from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr;
        QSqlQuery quer(req,DataBase::getInstance()->getDataBase());
        if (quer.size()== 0)
        {
            UpMessageBox::Watch(this,tr("Suppression refusée"), tr("Vous ne pouvez pas supprimer un document dont vous n'êtes pas l'auteur"));
            return;
        }
        quer.first();
        if (quer.value(0).toInt() != DataBase::getInstance()->getUserConnected()->id())
        {
            UpMessageBox::Watch(this,tr("Suppression refusée"), tr("Vous ne pouvez pas supprimer un document dont vous n'êtes pas l'auteur"));
            return;
        }
    }

    if (idimpr != "")
    {
        UpMessageBox msgbox;
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("Supprimer"));
        UpSmallButton *NoBouton = new UpSmallButton();
        NoBouton->setText(tr("Annuler"));
        msgbox.setText("Euuhh... " + proc->getDataUser()->getLogin());
        msgbox.setInformativeText(tr("Etes vous certain de vouloir supprimer ce document?"));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(NoBouton,UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
        msgbox.exec();
        bool a = (msgbox.clickedButton() == OKBouton);
        delete OKBouton;
        delete NoBouton;
        msgbox.close();
        if (!a) return;
        QString req = "select lienversfichier, formatdoc from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr;
        //qDebug() << req;
        QSqlQuery quer(req, DataBase::getInstance()->getDataBase());
        if (quer.size()>0)
        {
            quer.first();
            if (quer.value(0).toString() != "")
            {
                QString filename = (quer.value(1).toString() == VIDEO? "/" : "") + quer.value(0).toString();
                QString cheminFichier = (quer.value(1).toString() == VIDEO? NOMDIR_VIDEOS : NOMDIR_IMAGES);
                filename = cheminFichier + filename;
                QSqlQuery ("insert into " NOM_TABLE_DOCSASUPPRIMER " (FilePath) VALUES ('" + filename + "')", DataBase::getInstance()->getDataBase());
            }
        }
        QString idaafficher = "";
        if (ndocs>1)
        {
            QSqlQuery idquer("Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient) + " order by dateimpression", DataBase::getInstance()->getDataBase());
            for (int i=0; i<idquer.size(); i++)
            {
                idquer.seek(i);
                if (idquer.value(0).toInt() == idimpr.toInt())
                {
                    if (i==0)
                        idquer.next();
                    else
                        idquer.previous();
                    idaafficher = idquer.value(0).toString();
                    break;
                }
            }
        }
        QSqlQuery("delete from " NOM_TABLE_REFRACTION " where idrefraction = (select idrefraction from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr + ")", DataBase::getInstance()->getDataBase());
        QSqlQuery("delete from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr, DataBase::getInstance()->getDataBase());
        gmodele->removeRow(idx.row(),idx);
        QModelIndex idx2;
        bool OK = false;
        if (idaafficher != "")
            for (int m = 0; m<gmodele->rowCount(); m++)
            {
                for (int n=0; n<gmodele->item(m)->rowCount(); n++)
                    if (gmodele->item(m)->child(n)->accessibleDescription() == idaafficher)
                    {
                        idx2 = gmodele->item(m)->child(n)->index();
                        OK = true;
                        break;
                    }
                if (OK) break;
            }
        if (ndocs==1)
            reject();
        ListDocsTreeView->reset();
        ListDocsTreeView->expandAll();
        ListDocsTreeView->scrollTo(idx2, QAbstractItemView::EnsureVisible);
        ListDocsTreeView->setCurrentIndex(idx2);
        RemplirTreeView();
     }
}

void dlg_docsexternes::ZoomDoc()
{
    if (gMode == Normal)
    {
        PosOrigine      = pos();
        SizeOrigine     = size();
        gMode           = Zoom;
        ScrollTable     ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //TODO : icon scaled : pxZoomOut 30,30
        GraphicView     ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //TODO : icon scaled : pxZoomOut 30,30

        if (hdeltaframe == 0)   hdeltaframe = frameGeometry().height() - height();
        if (wdeltaframe == 0)   wdeltaframe = frameGeometry().width()  - width();
        if (hdelta == 0)
        {
            if (gTypeDoc == PDF)    hdelta  = height() - ScrollTable->height();
            else                    hdelta  = height() - GraphicView->height();
        }
        if (wdelta == 0)
        {
            if (gTypeDoc == PDF)    wdelta  = width() - ScrollTable->width();
            else                    wdelta  = width() - GraphicView->width();
        }

        // les dimensions maxi de la zone de visu
        const double maxwscroll  = qApp->desktop()->availableGeometry().width()*2/3    - wdelta - wdeltaframe;
        const double maxhscroll  = qApp->desktop()->availableGeometry().height()       - hdelta - hdeltaframe;
        // les dimensions calculées de la zone de visu
        int wfinal(0), hfinal(0);

        double proportion = maxwscroll/maxhscroll;
        if (idealproportion > proportion)
        {   wfinal  = maxwscroll;   hfinal  = wfinal / idealproportion; }
        else
        {   hfinal  = maxhscroll;   wfinal  = hfinal * idealproportion; }
        int w = wfinal + wdelta;
        int h = hfinal + hdelta;
        resize(w, h);

        if (gTypeDoc == PDF)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(glistPix.at(i).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    ScrollTable->setRowHeight(i,hfinal-2);
                    ScrollTable->setColumnWidth(0,wfinal-2);
                    if (i==0) ScrollTable->resize(wfinal, hfinal);
                }
            }
        }
        else if (gTypeDoc == JPG)
        {
            Scene->clear();
            QPixmap pix = glistPix.at(0).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            Scene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            Scene->setSceneRect(1,1,x-1,y-1);
        }
        else if (gTypeDoc == VIDEO)
        {
            if (Scene->items().size()>0)
            {
                if (dynamic_cast<QGraphicsVideoItem*>(Scene->items().at(0)) != Q_NULLPTR)
                    videoItem   ->setSize(QSize(wfinal-2,hfinal-2));
                int x = videoItem->size().width();
                int y = videoItem->size().height();
                Scene->setSceneRect(1,1,x-1,y-1);
            }
        }
        move (qApp->desktop()->availableGeometry().width() - w, 0);
    }
    else if (gMode == Zoom)
    {
        ScrollTable     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //TODO : icon scaled : pxZoomIn 30,30
        GraphicView     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //TODO : icon scaled : pxZoomIn 30,30
        move(PosOrigine);
        resize(SizeOrigine);
        if (gTypeDoc == PDF)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = glistPix.at(i).scaled(SizeOrigine.width() - wdelta - 2, SizeOrigine.height() - hdelta - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    lbl->setPixmap(pix);
                    ScrollTable->setRowHeight(i,pix.height());
                    ScrollTable->setColumnWidth(0,pix.width());
                    ScrollTable->resize(SizeOrigine.width()-wdelta, SizeOrigine.height()-hdelta);
                }
            }
        }
        else if (gTypeDoc == JPG)
        {
            Scene->clear();
            QPixmap pix = glistPix.at(0).scaled(SizeOrigine.width() - wdelta - 2, SizeOrigine.height() - hdelta - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            Scene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            Scene->setSceneRect(1,1,x-1,y-1);
        }
        else if (gTypeDoc == VIDEO)
        {
            if (Scene->items().size()>0)
            {
                if (dynamic_cast<QGraphicsVideoItem*>(Scene->items().at(0)) != Q_NULLPTR)
                    videoItem   ->setSize(QSize(SizeOrigine.width() - wdelta - 2, SizeOrigine.height() - hdelta - 2));
                int x = videoItem->size().width();
                int y = videoItem->size().height();
                Scene->setSceneRect(1,1,x-1,y-1);
            }
        }
        gMode           = Normal;
    }
    setEnregPosition(gMode == Normal);
    inflabel    ->move(10, (gTypeDoc == PDF? ScrollTable->height() : GraphicView->height())-40);
}

bool dlg_docsexternes::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==ScrollTable)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(glistPix.at(i).scaled(ScrollTable->width(), ScrollTable->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    ScrollTable->setRowHeight(i,lbl->pixmap()->height());
                    ScrollTable->setColumnWidth(i,lbl->pixmap()->width());
                }
            }
            inflabel    ->move(10,ScrollTable->viewport()->height()-40);
        }
    }
    if (obj == GraphicView)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            if (gTypeDoc == JPG)
            {
                if (Scene->items().size()>0)
                {
                    Scene->clear();
                    QPixmap pix = glistPix.at(0).scaled(GraphicView->width(), GraphicView->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    Scene->addPixmap(pix);
                    int x = pix.size().width();
                    int y = pix.size().height();
                    Scene->setSceneRect(1,1,x-1,y-1);
                 }
            }
            else if (gTypeDoc == VIDEO)
            {
                if (Scene->items().size()>0)
                {
                    if (dynamic_cast<QGraphicsVideoItem*>(Scene->items().at(0)) != Q_NULLPTR)
                        videoItem   ->setSize(QSize(GraphicView->width(),GraphicView->height()));
                    int x = videoItem->size().width();
                    int y = videoItem->size().height();
                    Scene->setSceneRect(1,1,x-1,y-1);
                }
            }
            inflabel    ->move(10,GraphicView->height()-40);
        }
        if (event->type() == QEvent::MouseButtonPress)
        {
            //static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
            //qDebug() << QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(event->type());
            ZoomDoc();;
        }

     }
    return QWidget::eventFilter(obj, event);
}

void dlg_docsexternes::RemplirTreeView(bool recalcul)
{
    /*
     * 0-idImpression
     * 1-idUser
     * 2-idpat
     * 3-TypeDoc
     * 4-Titre
     * 5-TextEntete
     * 6-TextCorps
     * 7-TextPied
     * 8-Dateimpression
     * 9-pdf
     * 10-formatautre
     * 11-lienversfichier
    */
    int                 ndocs = 0;
    QStandardItem       *dateitem, *typitem, *pitem, *pitem1;
    QString             idimpraretrouver = "";
    gmodele = dynamic_cast<QStandardItemModel*>(ListDocsTreeView->model());
    if (gmodele)
    {
        if (ListDocsTreeView->selectionModel()->selectedIndexes().size()>0)
        {
            QModelIndex         actifidx;
            actifidx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
            if (!gmodele->itemFromIndex(actifidx)->hasChildren())
                idimpraretrouver = gmodele->itemFromIndex(actifidx)->accessibleDescription();
        }
        gmodele->clear();
    }
    else
    {
        gmodele             = new QStandardItemModel(this);
        gmodeleTriParDate   = new QStandardItemModel(this);
        gmodeleTriParType   = new QStandardItemModel(this);
    }
    if (recalcul)
        TreeQuery.exec("Select idImpression, TypeDoc, Titre, Dateimpression, SousTypeDoc from " NOM_TABLE_IMPRESSIONS
                       " where idpat = " + QString::number(gidPatient));
    QStandardItemModel treemodel;
    QStandardItem *tit0, *tit1, *tit2, *tit3, *tit4;
    for (int i=0; i<TreeQuery.size(); i++)
    {
        tit0 = new QStandardItem();
        tit1 = new QStandardItem();
        tit2 = new QStandardItem();
        tit3 = new QStandardItem();
        tit4 = new QStandardItem();
        TreeQuery.seek(i);
        tit0->setText(TreeQuery.value(0).toString());
        tit1->setText(TreeQuery.value(1).toString());
        tit2->setText(TreeQuery.value(2).toString());
        tit3->setText(TreeQuery.value(3).toString());
        tit4->setText(TreeQuery.value(4).toString());
        QList<QStandardItem *> pitemlist;
        pitemlist << tit0 << tit1 << tit2 << tit3 << tit4;
        treemodel.appendRow(pitemlist);
    }

    //1 Liste des documents (ordonnances, certificats, courriers, imagerie...etc...) imprimés par le poste ou reçus
    if (treemodel.rowCount()>0)
    {
        ndocs = treemodel.rowCount();
        // Tri par date
        treemodel.sort(3);
        QString datestring ="";
        for (int i=0;i<treemodel.rowCount();i++)
        {
            QString textdate = treemodel.item(i,3)->text();
            QString date = QDateTime::fromString(textdate, "yyyy-MM-ddThh:mm:ss").toString(tr("dd-MM-yyyy"));
            if (date != datestring)
            {
                dateitem   = new QStandardItem();
                datestring = date;
                dateitem->setText(datestring);
                dateitem->setForeground(QBrush(QColor(Qt::red)));
                dateitem->setEditable(false);
                dateitem->setEnabled(false);
                gmodeleTriParDate->appendRow(dateitem);
            }
        }
        for (int i=0;i<treemodel.rowCount();i++)
        {
            QString date = QDateTime::fromString(treemodel.item(i,3)->text(), "yyyy-MM-ddThh:mm:ss").toString(tr("dd-MM-yyyy"));
            QString a = treemodel.item(i,1)->text();                       //Type de document
            if (treemodel.item(i,4)->text() != "")
            {
                if (a!="") a += " - ";
                a += treemodel.item(i,4)->text();                       //Sous-type du document
            }
            else if (treemodel.item(i,2)->text() != "")                // Titre du document
            {
                if (a!="") a += " - ";
                QTextEdit text;
                text.setHtml(treemodel.item(i,2)->text());
                a += text.toPlainText();
            }
            pitem   = new QStandardItem(a);
            pitem1  = new QStandardItem(QDateTime::fromString(treemodel.item(i,3)->text(), "yyyy-MM-ddThh:mm:ss").toString("yyyyMMddHHmmss") +"000");
            pitem->setAccessibleDescription(treemodel.item(i,0)->text());
            pitem->setEditable(true);
            QList<QStandardItem *> pitemlist;
            pitemlist << pitem << pitem1;
            QList<QStandardItem *> listitems = gmodeleTriParDate->findItems(date);
            if (listitems.size()>0)
            {
                listitems.at(0)->appendRow(pitemlist);
                listitems.at(0)->sortChildren(1);
            }
        }
        // Tri par type
        treemodel.sort(2);
        QString typedoc ="";
        for (int i=0;i<treemodel.rowCount();i++)
        {
            if (treemodel.item(i,1)->text() != typedoc)
            {
                typitem     = new QStandardItem();
                typedoc     = treemodel.item(i,1)->text();
                typitem     ->setText(typedoc);
                typitem     ->setForeground(QBrush(QColor(Qt::red)));
                typitem     ->setEditable(false);
                typitem     ->setEnabled(false);
                gmodeleTriParType     ->appendRow(typitem);
            }
        }
        for (int i=0;i<treemodel.rowCount();i++)
        {
            QString textdate = treemodel.item(i,3)->text();
            QString date = QDateTime::fromString(textdate, "yyyy-MM-ddThh:mm:ss").toString(tr("dd-MM-yyyy"));
            QString a = date + " - ";
            if (treemodel.item(i,4)->text() != "")
                a += treemodel.item(i,4)->text();                       //Sous-type du document
            else if (treemodel.item(i,2)->text() != "")                // Titre du document
            {
                QTextEdit text;
                text.setHtml(treemodel.item(i,2)->text());
                a += text.toPlainText();
            }
            else
                a += treemodel.item(i,1)->text();                       //Type de document

            pitem   = new QStandardItem(a);
            pitem1  = new QStandardItem(QDateTime::fromString(treemodel.item(i,3)->text(), "yyyy-MM-ddThh:mm:ss").toString("yyyyMMddHHmmss") +"000");
            pitem   ->setAccessibleDescription(treemodel.item(i,0)->text());
            pitem   ->setEditable(true);
            QList<QStandardItem *> pitemlist;
            pitemlist << pitem << pitem1;
            QList<QStandardItem *> listitems = gmodeleTriParType->findItems(treemodel.item(i,1)->text());
            if (listitems.size()>0)
            {
                listitems.at(0)->appendRow(pitemlist);
                listitems.at(0)->sortChildren(1);
            }
        }
    }
    nbredocs = ndocs;
    if (gModeTri == parDate)
        gmodele  = gmodeleTriParDate;
    else gmodele = gmodeleTriParType;
    if (gmodele->rowCount()>0)
    {
        int nrows = gmodele->item(gmodele->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item date
        QStandardItem *item =  gmodele->item(gmodele->rowCount()-1)->child(nrows,0);    // le tout dernier item
        QModelIndex idx = item->index();                                                // l'index de ce dernier item
        if (idimpraretrouver != "")
        {
            bool nouvelidx = false;
            for (int m = 0; m<gmodele->rowCount(); m++)
            {
                for (int n=0; n<gmodele->item(m)->rowCount(); n++)
                    if (gmodele->item(m)->child(n)->accessibleDescription() == idimpraretrouver)
                    {
                        idx = gmodele->item(m)->child(n)->index();
                        nouvelidx = true;
                        break;
                    }
                if (nouvelidx) break;
            }
        }
        else
        {
            ListDocsTreeView->setModel(gmodele);
            connect(gmodele,    &QStandardItemModel::itemChanged, [=] {EditSousTitre(gmodele->itemFromIndex(ListDocsTreeView->selectionModel()->currentIndex()));});
            ListDocsTreeView->setAnimated(true);
            ListDocsTreeView->setIndentation(3);
            ListDocsTreeView->header()->setVisible(false);
        }
        ListDocsTreeView->expandAll();
        ListDocsTreeView->scrollTo(idx, QAbstractItemView::EnsureVisible);
        ListDocsTreeView->setCurrentIndex(idx);
        connect(ListDocsTreeView->selectionModel(), &QItemSelectionModel::currentChanged, [=] {AfficheDoc(ListDocsTreeView->selectionModel()->currentIndex());});
        //if (recalcul)
        //Slot_AfficheDoc(idx);
    }
    else
    {
        initOK =false;
        close();
    }
}

