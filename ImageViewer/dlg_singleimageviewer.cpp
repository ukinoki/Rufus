/* (C) 2025 LAINE SERGE
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

#include "dlg_singleimageviewer.h"

dlg_singleimageviewer::dlg_singleimageviewer(QWidget *parent) : UpDialog(parent)
{
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    dlglayout()     ->insertLayout(0,m_mainlayout);
    m_mainlayout    ->setContentsMargins(m_marges);
    m_mainlayout    ->setSpacing(m_spacing);
    m_imgwdg        = new DisplayWidget();
    m_mainlayout    ->addWidget(m_imgwdg);
    setStageCount(1);
    QFont font      = qApp->font();
    font            .setPointSize(14);
    m_labinfowdg    ->setFont(font);
    m_labinfowdg    ->setParent(m_imgwdg);
    installEventFilter(this);
    //AjouteLayButtons(UpDialog::ButtonOK | UpDialog::ButtonRecord | UpDialog::ButtonPrint);
}

void dlg_singleimageviewer::setListDocuments(QList<DocExterne *> listdocs, DocExterne *doc)
{
    m_ListDocs      = listdocs;
    m_currentDoc   = doc;
    if (m_ListDocs.size() > 1)
    {
        wdg_toolbar = new UpToolBar;
        AjouteWidgetLayButtons(wdg_toolbar, false);
        connect(wdg_toolbar, &UpToolBar::TBSignal, this, [=] {goTo(wdg_toolbar->choice());});
        int idx = m_ListDocs.indexOf(m_currentDoc);
        wdg_toolbar->First()    ->setEnabled(idx>0);
        wdg_toolbar->Prec()     ->setEnabled(idx>0);
        wdg_toolbar->Next()     ->setEnabled(idx < m_ListDocs.size()-1);
        wdg_toolbar->Last()     ->setEnabled(idx < m_ListDocs.size()-1);
    }
    else
        m_currentDoc = listdocs.at(0);
    displaycurrentDocument();
}

void dlg_singleimageviewer::displaycurrentDocument()
{
    if (m_currentDoc)
    {
        if (m_currentDoc->isVideo())
        {
            QString filename = Procedures::I()->settings()->value(Utils::getBaseFromMode(DataBase::I()->ModeAccesDataBase()) + Dossier_Videos).toString() + "/" + m_currentDoc->lienversfichier();
            DisplayVideo(filename);
        }
        else
        {
            QList<QImage> listimg = m_currentDoc->pagelist();
            if (listimg.size() > 0)
                DisplayImage(listimg, (m_ListDocs.size()>1? QLocale::system().toString(m_currentDoc->date(), tr("dd-MMM-yyyy")) + "- " : "") + m_currentDoc->soustypedoc());
        }
    }
}

void dlg_singleimageviewer::setDepense(Depense *dep)
{
    if (dep)
    {
        QList<QImage> listimg = QList<QImage>();
        if (dep->factureformat() == JPG)
        {
            QImage image;
            image.loadFromData(dep->factureblob());
            listimg << image;
        }
        else if (dep->factureformat() == PDF)
            listimg = Utils::calcImagefromPdf(dep->factureblob());

        if (listimg.size() > 0)
            DisplayImage(listimg, dep->objet());
    }
}

QRect dlg_singleimageviewer::optimalGeometryForZoom(double ratioimgorigine) const
{
    //! Géométrie du dialogue en mode Zoom : la fiche doit occuper toute la place possible et
    //! afficher l'image à sa taille MAXIMALE sans la tronquer. Qt::KeepAspectRatio loge un
    //! rectangle de ratio image dans la place disponible et choisit TOUT SEUL si c'est la hauteur
    //! ou la largeur qui plafonne (selon l'orientation) — plus de calcul d'orientation à la main.
    //!
    //! Le 0.98 (marge de 2 %) est une réserve de sécurité ASSUMÉE, pas un détail cosmétique :
    //!  - Qt ne connaît la taille du CADRE de la fenêtre (barre de titre + bordures, soit
    //!    frameGeometry() - geometry()) qu'APRÈS le premier affichage : impossible de la déduire
    //!    ici pour la soustraire proprement avant d'afficher la fiche.
    //!  - availableGeometry() n'excluait pas toujours le dock macOS (selon les versions de Qt).
    //! Ces 2 % couvrent forfaitairement cadre + dock pour éviter que la fiche déborde de l'écran.
    //! Solution propre possible (2 passes : afficher puis réajuster sur frameGeometry connu), à ne
    //! tenter que si le besoin réel se présente — gain ~2 %, et code de géométrie cross-plateforme.
    if (QGuiApplication::screens().isEmpty())
        return QRect();
    QSize  ecran  = QGuiApplication::screens().first()->availableGeometry().size() * 0.98;
    QSize  chrome = deltas();                            //! place prise par boutons/marges autour de l'image
    QSizeF placeImage(ecran.width() - chrome.width(), ecran.height() - chrome.height());
    QSize  image  = QSizeF(ratioimgorigine, 1.0).scaled(placeImage, Qt::KeepAspectRatio).toSize();
    return QRect(QPoint(0, 0), image + chrome);
}

void dlg_singleimageviewer::centerForZoom()
{
    //! En mode Zoom, la fiche remplit l'écran dans UNE dimension et laisse de la place dans l'autre
    //! (selon l'orientation de l'image). On la centre dans les DEUX dimensions : l'axe "plein" se
    //! recentre sur sa petite marge (le 0.98), l'axe "libre" se centre vraiment. Résultat : image
    //! portrait -> fiche centrée horizontalement ; image paysage -> centrée verticalement.
    //! framePosition() corrige le décalage du cadre (barre de titre) quand la fiche est déjà visible.
    if (QGuiApplication::screens().isEmpty())
        return;
    //! On centre d'après la taille CIBLE (recalculée), et NON width()/height() : juste après resize(),
    //! sous xcb/X11 la nouvelle taille n'est pas encore appliquée (le gestionnaire de fenêtres la
    //! traite de façon asynchrone) -> width() est périmé -> la fiche se retrouve déportée à droite.
    QSize sz    = optimalGeometryForZoom(m_wdgratio).size();
    QRect avail = QGuiApplication::screens().first()->availableGeometry();
    int x = avail.left() + (avail.width()  - sz.width())  / 2;
    int y = avail.top()  + (avail.height() - sz.height()) / 2;
    move(framePosition(QRect(QPoint(x, y), sz)));
}

qreal dlg_singleimageviewer::widgetRatio(QList<QImage> listimg)
{
    qreal maxw(0), maxh(0);
    for (int i=0; i < listimg.size(); i++)
    {
        maxw = std::max(maxw,qreal(listimg.at(i).width()));
        maxh = std::max(maxh,qreal(listimg.at(i).height()));
    }
    return maxw / maxh;
}

void dlg_singleimageviewer::DisplayImage(QList<QImage> listimage, QString nomdoc)
{
    if (m_controlplayer != Q_NULLPTR)
    {
        delete m_controlplayer;
        m_controlplayer = Q_NULLPTR;
    }
    if (buttons().testFlag(ButtonPrint))
        PrintButton     ->setVisible(true);
    m_imgwdg        ->setListimg(listimage, sizeForMainWidgetDisplay());
    m_wdgratio = widgetRatio(listimage);
    if (m_mode == Zoom)
    {
        QRect geozoom = optimalGeometryForZoom(m_wdgratio);
        resize(geozoom.width(), geozoom.height());
    }
    else m_imgwdg   ->resize(sizeForMainWidgetDisplay());
    m_labinfowdg    ->setText("<font color='magenta'>" + nomdoc + "</font>");
    m_labinfowdg    ->setFixedWidth(m_imgwdg->width() -10);
    m_labinfowdg    ->move(10, m_imgwdg->viewport()->height() -30);

    QList<QScreen*> listscreens = QGuiApplication::screens();
    double hscroll  = 0;
    if (listscreens.size())
        hscroll  = listscreens.first()->availableGeometry().height();
    double finalh = hscroll * 0.98;
    setMaximumHeight(int(finalh));

    if (m_mode == Zoom)
        centerForZoom();
    else
        move(framePosition(originalgeometry()));
}

void dlg_singleimageviewer::DisplayVideo(QString filepath)
{
    if (m_controlplayer == Q_NULLPTR)
    {
        m_controlplayer     = new PlayerControls(this);
        buttonslayout()     ->insertWidget(buttonslayout()->count()>1?buttonslayout()->count()-2 : 0,m_controlplayer);
    }
    if (buttons().testFlag(ButtonPrint))
        PrintButton     ->setVisible(false);
    m_imgwdg        ->setVideo(filepath, sizeForMainWidgetDisplay());
    QSize size      = m_imgwdg->mediaPlayer()->videosize();
    m_wdgratio      = size.width() / size.height();
    if (m_mode == Zoom)
    {
        QRect geozoom = optimalGeometryForZoom(m_wdgratio);
        resize(geozoom.width(), geozoom.height());
    }
    else m_imgwdg   ->resize(sizeForMainWidgetDisplay());
    m_labinfowdg    ->setText(QFileInfo(filepath).fileName());

    QList<QScreen*> listscreens = QGuiApplication::screens();
    double hscroll  = 0;
    if (listscreens.size())
        hscroll  = listscreens.first()->availableGeometry().height();
    double finalh = hscroll * 0.98;
    setMaximumHeight(int(finalh));

    m_labinfowdg        ->setText("<font color='magenta'>" + filepath + "</font>");

    m_controlplayer     ->setMinimumWidth(250);
    m_controlplayer     ->setPlayer(m_imgwdg->mediaPlayer());
    m_controlplayer     ->startplay();

    if (m_mode == Zoom)
        centerForZoom();
    else
        move(framePosition(originalgeometry()));
}

QList<DocExterne *> dlg_singleimageviewer::ListDocs() const
{
    return m_ListDocs;
}

DisplayWidget *dlg_singleimageviewer::imagewidget() const
{
    return m_imgwdg;
}

void dlg_singleimageviewer::setCorrectionwidget(QWidget *newCorrectionwidget)
{
    if (newCorrectionwidget != Q_NULLPTR)
        setCorrectionWidth(newCorrectionwidget->width());
}

QHBoxLayout *dlg_singleimageviewer::mainlayout() const
{
    return m_mainlayout;
}

UpLabel *dlg_singleimageviewer::labinfowidget() const
{
    return m_labinfowdg;
}

dlg_singleimageviewer::Mode dlg_singleimageviewer::mode() const
{
    return m_mode;
}

void dlg_singleimageviewer::setMode(Mode newMode)
{
    m_mode = newMode;
    setEnregPosition(m_mode == Normal);
}

void dlg_singleimageviewer::changeMode(Mode newMode)
{
    m_mode = newMode;
    if (m_mode == Zoom)
    {
        QRect geozoom = optimalGeometryForZoom(m_wdgratio);
        resize(geozoom.width(), geozoom.height());
        centerForZoom();
        setEnregPosition(false);
    }
    else
    {
        resize(originalgeometry().size());
        move(framePosition(originalgeometry()));
        setEnregPosition(true);
    }
}

QMap<QString,QVariant> dlg_singleimageviewer::mapimg() const
{
    return m_mapimg;
}

void dlg_singleimageviewer::goTo(UpToolBar::Choice choice)
{
    int idx = m_ListDocs.indexOf(m_currentDoc);
    if (choice == UpToolBar::_last)
        idx     = m_ListDocs.size()-1;
    else if (choice == UpToolBar::_first)
        idx     = 0;
    else if (choice == UpToolBar::_next)
        idx     += 1;
    else if (choice == UpToolBar::_prec)
        idx     -= 1;
    wdg_toolbar->First()    ->setEnabled(idx>0);
    wdg_toolbar->Prec()     ->setEnabled(idx>0);
    wdg_toolbar->Next()     ->setEnabled(idx < m_ListDocs.size()-1);
    wdg_toolbar->Last()     ->setEnabled(idx < m_ListDocs.size()-1);
    if (idx>-1)
    {
        m_currentDoc = m_ListDocs.at(idx);
        displaycurrentDocument();
    }
}

bool dlg_singleimageviewer::eventFilter(QObject *obj, QEvent *event)
{
    QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
    if (rszevent != Q_NULLPTR)
    {
        if (m_mode == Normal)
            setOriginalgeometry(geometry());
        m_labinfowdg    ->setFixedWidth(m_imgwdg->width() -10);
        m_labinfowdg    ->move(10, m_imgwdg->viewport()->height() -30);
    }
    return QWidget::eventFilter(obj, event);
}

