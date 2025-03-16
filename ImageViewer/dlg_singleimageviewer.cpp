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
    m_imgwdg        = new ListImageWidget();
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
            setVideofile(filename);
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

void dlg_singleimageviewer::setVideofile(QString filepath)
{
    DisplayVideo(filepath);
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
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
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
        move(0, 0);
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
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
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
    {
        move(0,0);
        //if (listscreens.size())  move((listscreens.first()->geometry().width() - width())/2, 0);
    }
    else
        move(framePosition(originalgeometry()));
}

QList<DocExterne *> dlg_singleimageviewer::ListDocs() const
{
    return m_ListDocs;
}

ListImageWidget *dlg_singleimageviewer::imagewidget() const
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
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
        move(0,0);
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

