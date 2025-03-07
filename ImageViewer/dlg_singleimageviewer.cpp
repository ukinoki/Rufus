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
    buttonslayout() ->insertWidget(buttonslayout()->count()>1?buttonslayout()->count()-3 : 1,m_labinfowdg);
    buttonslayout() ->insertSpacerItem(buttonslayout()->count()>1?buttonslayout()->count()-3 : 1,new QSpacerItem(10,10,QSizePolicy::Expanding));
    installEventFilter(this);
}

void dlg_singleimageviewer::setDocument(DocExterne *docmt)
{
    if (docmt)
    {
        if (docmt->isVideo())
        {
            QString filename = Procedures::I()->settings()->value(Utils::getBaseFromMode(DataBase::I()->ModeAccesDataBase()) + Dossier_Videos).toString() + "/" + docmt->lienversfichier();
            setVideofile(filename);
            DisplayVideo(filename);
        }
        else
        {
            QList<QImage> listimg = docmt->pagelist();
            if (listimg.size() > 0)
                DisplayImage(listimg);
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
            DisplayImage(listimg);
    }
}

void dlg_singleimageviewer::setMapimg(const QMap<QString,QVariant> &newMapimg)
{
    m_mapimg = newMapimg;
    QList<QImage> listimg = QList<QImage>();
    if (m_mapimg.value(M_TYPE).toString() == JPG)
    {
        QImage image;
        image.loadFromData(m_mapimg.value(M_BA).toByteArray());
        listimg << image;
    }
    else if (m_mapimg.value(M_TYPE).toString() == PDF)
        listimg = Utils::calcImagefromPdf(m_mapimg.value(M_BA).toByteArray());

    if (listimg.size() > 0)
        DisplayImage(listimg);
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
    m_imgwdg        ->setListimg(listimage, sizeForMainWidgetDisplay());
    m_wdgratio = widgetRatio(listimage);
    if (m_mode == Zoom)
    {
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
    }
    else m_imgwdg    ->resize(sizeForMainWidgetDisplay());

    m_labinfowdg     ->setText("<font color='magenta'>" + nomdoc + "</font>");

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
    m_imgwdg        ->setVideo(filepath, sizeForMainWidgetDisplay());
    QSize size      = m_imgwdg->mediaPlayer()->videosize();
    m_wdgratio      = size.width() / size.height();
    if (m_mode == Zoom)
    {
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
    }
    else m_imgwdg    ->resize(sizeForMainWidgetDisplay());

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
        if (listscreens.size())
            move((listscreens.first()->geometry().width() - width())/2, 0);
    }
    else
        move(framePosition(originalgeometry()));
}

ListImageWidget *dlg_singleimageviewer::imagewidget() const
{
    return m_imgwdg;
}

void dlg_singleimageviewer::setCorrectionwidget(QWidget *newCorrectionwidget)
{
    m_correctionwidget = newCorrectionwidget;

    if (m_correctionwidget != Q_NULLPTR)
    {
        /*! following make troubles under Windows
        int correctionframewidth = m_correctionwidget->width();
        QFrame *frame = dynamic_cast<QFrame*>(m_correctionwidget);
        if (frame)
            correctionframewidth += frame->lineWidth() *2;
        */
        setCorrectionWidth(m_correctionwidget->width());
    }
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

bool dlg_singleimageviewer::eventFilter(QObject *obj, QEvent *event)
{
    QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
    if (rszevent != Q_NULLPTR && m_mode == Normal)
        setOriginalgeometry(geometry());
    return QWidget::eventFilter(obj, event);
}
