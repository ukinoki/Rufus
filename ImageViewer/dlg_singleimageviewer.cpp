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
    dlglayout()->insertLayout(0,m_mainlayout);
    m_mainlayout->setContentsMargins(m_marges);
    m_mainlayout->setSpacing(m_spacing);
    setStageCount(1);
    QFont font          = qApp->font();
    font                .setPointSize(14);
    m_labinfowdg        ->setFont(font);
    buttonslayout()     ->insertWidget(buttonslayout()->count()>1?buttonslayout()->count()-3 : 1,m_labinfowdg);
    buttonslayout()     ->insertSpacerItem(buttonslayout()->count()>1?buttonslayout()->count()-3 : 1,new QSpacerItem(10,10,QSizePolicy::Expanding));
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
            QList<QImage> listimg = QList<QImage>();
            if (docmt->isJPG())
            {
                QImage image;
                image.loadFromData(docmt->imageblob());
                listimg << image;
            }
            else if (docmt->isPDF())
                listimg = docmt->pagelist();
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

void dlg_singleimageviewer::setVideofile(QString filename)
{
    InitDisplay(Video);
    m_vdwdg         ->setFilename(filename);
    DisplayVideo(filename);
}

void dlg_singleimageviewer::calcWidgetRatio(QList<QImage> listimg)
{
    double maxw(0), maxh(0);
    for (int i=0; i < listimg.size(); i++)
    {
        QPixmap pix = QPixmap::fromImage(listimg.at(i));
        if (pix.width() > maxw)
            maxw = pix.width();
        if (pix.height() > maxh)
            maxh = pix.height();
    }
    m_wdgratio  = maxw / maxh;
}

void dlg_singleimageviewer::DisplayImage(QList<QImage> listimage, QString nomdoc)
{
    InitDisplay(Image);
    calcWidgetRatio(listimage);
    if (m_mode == Zoom)
    {
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
    }

    m_imgwdg            ->setListimg(listimage, sizeForMainWidgetDisplay());
    m_currentwidget     = m_imgwdg;
    m_imgwdg            ->resize(sizeForMainWidgetDisplay());

    m_labinfowdg        ->setText("<font color='magenta'>" + nomdoc + "</font>");

    QList<QScreen*> listscreens = QGuiApplication::screens();
    double hscroll  = 0;
    if (listscreens.size())
        hscroll  = listscreens.first()->availableGeometry().height();
    double finalh = hscroll * 0.98;
    setMaximumHeight(int(finalh));

    m_currentwidget     = m_imgwdg;

    if (m_mode == Zoom)
        move(0, 0);
    else
        move(framePosition(originalgeometry()));
}

void dlg_singleimageviewer::DisplayVideo(QString filepath)
{
    QSize size      = m_vdwdg->player()->videosize();
    m_wdgratio      = size.width() / size.height();
    if (m_mode == Zoom)
    {
        setOptimalSizesForZoom(m_wdgratio);
        resize(optimalgeometryforzoom().width(), optimalgeometryforzoom().height());
    }

    QList<QScreen*> listscreens = QGuiApplication::screens();
    double hscroll  = 0;
    if (listscreens.size())
        hscroll  = listscreens.first()->availableGeometry().height();
    double finalh = hscroll * 0.98;
    setMaximumHeight(int(finalh));

    m_currentwidget         = m_vdwdg;
    m_vdwdg                 ->resize(sizeForMainWidgetDisplay());

    m_labinfowdg            ->setText("<font color='magenta'>" + filepath + "</font>");

    m_controlplayer         ->setMinimumWidth(250);
    m_controlplayer         ->setPlayer(m_vdwdg->player());
    m_controlplayer         ->startplay();

    if (m_mode == Zoom)
    {
        if (listscreens.size())
            move((listscreens.first()->geometry().width() - width())/2, 0);
    }
    else
        move(framePosition(originalgeometry()));
}

void dlg_singleimageviewer::InitDisplay(TypeDoc typ)
{

    switch (typ)
    {
    case  Video:
    {
        if (m_imgwdg != Q_NULLPTR)
        {
            delete m_imgwdg;
            m_imgwdg    = Q_NULLPTR;
        }
        if (m_vdwdg == Q_NULLPTR)
        {
            m_vdwdg     = new UpVideoWidget();
            m_mainlayout->insertWidget(m_mainlayout->count(),m_vdwdg);
        }
        if (m_controlplayer == Q_NULLPTR)
        {
            m_controlplayer     = new PlayerControls(this);
            buttonslayout()     ->insertWidget(buttonslayout()->count()>1?buttonslayout()->count()-2 : 0,m_controlplayer);
        }
        break;
    }
    case (Image):
    {
        if (m_vdwdg != Q_NULLPTR)
        {
            delete m_vdwdg;
            m_vdwdg    = Q_NULLPTR;
        }
        if (m_controlplayer != Q_NULLPTR)
        {
            delete m_controlplayer;
            m_controlplayer = Q_NULLPTR;
        }
        if (m_imgwdg == Q_NULLPTR)
        {
            m_imgwdg     = new ListImageWidget();
            m_mainlayout->insertWidget(m_mainlayout->count(),m_imgwdg);
        }
    }
    }
}

ListImageWidget *dlg_singleimageviewer::imagewidget() const
{
    return m_imgwdg;
}

UpVideoWidget *dlg_singleimageviewer::videoWidget() const
{
    return m_vdwdg;
}

void dlg_singleimageviewer::setCorrectionwidget(QWidget *newCorrectionwidget)
{
    m_correctionwidget = newCorrectionwidget;
    int correctionframewidth = 0;
    if (m_correctionwidget != Q_NULLPTR)
    {
        QFrame *frame = dynamic_cast<QFrame*>(m_correctionwidget);
        if (frame)
            correctionframewidth += frame->lineWidth() *2;
        correctionframewidth += m_correctionwidget->width();
        setCorrectionWidth(m_correctionwidget->width() + correctionframewidth);
    }}

QHBoxLayout *dlg_singleimageviewer::mainlayout() const
{
    return m_mainlayout;
}

UpLabel *dlg_singleimageviewer::labinfowidget() const
{
    return m_labinfowdg;
}

QWidget *dlg_singleimageviewer::currentwidget() const
{
    return m_currentwidget;
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
