#include "dlg_imagezoom.h"

dlg_imagezoom::dlg_imagezoom(QWidget * wdg, QWidget *parent) : UpDialog(parent)
{
    double h = 1.0;
    double w = 1.0;
    m_vdwdg = dynamic_cast<UpVideoWidget*>(wdg);
    if (m_vdwdg)
    {
        h = m_vdwdg->videoSink()->videoSize().height();
        w = m_vdwdg->videoSink()->videoSize().width();
    }
    else
    {
        m_labwdg = dynamic_cast<UpLabel*>(wdg);
        if (m_labwdg){
            if (m_labwdg)
            {
                h = m_labwdg->height();
                w = m_labwdg->width();
            }
        }
    }
    if (m_labwdg == Q_NULLPTR && m_vdwdg == Q_NULLPTR)
        return;
    setSizeAvailable(QSize(w,h), this, m_sizeform, m_sizewidget);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    AjouteLayButtons(UpDialog::ButtonOK | UpDialog::ButtonPrint | UpDialog::ButtonRecord);

    if (m_labwdg)
    {
        RecordButton            ->setVisible(false);
        UpLabel *lab            = new UpLabel;
        lab                     ->setImage(m_labwdg->image());
        QPixmap pix             = QPixmap::fromImage(m_labwdg->image()).scaled(m_sizewidget,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        lab                     ->setPixmap(pix);
        lab                     ->setAutoFillBackground(false);
        dlglayout()             ->insertWidget(0,lab);
    }
    else if (m_vdwdg)
    {
        PrintButton                     ->setVisible(false);
        m_controlplayer                 = new PlayerControls(this);
        AjouteWidgetLayButtons(m_controlplayer, false);
        UpVideoWidget* vid              = new UpVideoWidget(m_vdwdg->filename());
        m_player                        = new UpMediaPlayer(m_vdwdg->filename());
        m_player                        ->setSource( QUrl::fromLocalFile(vid->filename()));
        m_player                        ->setVideoOutput(m_vdwdg);
        m_player                        ->play();
        m_player                        ->setPosition(m_vdwdg->player()->position());
        vid                             ->setFixedSize(m_sizewidget);
        dlglayout()                     ->insertWidget(0,vid);
        m_controlplayer                 ->setPlayer(m_player);
    }
    connect(OKButton, &QPushButton::clicked, this, &QDialog::close);
    move(0,0);
}

dlg_imagezoom::dlg_imagezoom(DocExterne *doc, int pagepdf, QWidget *parent) : UpDialog(parent)
{
    QSize size  = QSize();
    QList<QImage> listimg = QList<QImage>();
    if (doc)
    {
        if (doc->isVideo())
        {
            QString filename    = DataBase::I()->dirimagerie() + NOM_DIR_VIDEOS "/" + doc->lienversfichier();
            UpVideoWidget* vid  = new UpVideoWidget(filename);
            m_player            = new UpMediaPlayer(filename, this);
            vid                 ->setPlayer(m_player);
            m_player            ->setVideoOutput(vid);
            size                = m_player->videoresolution();
            delete vid;
            delete m_player;
            setSizeAvailable(size, this, m_sizeform, m_sizewidget);
            setAttribute(Qt::WA_ShowWithoutActivating, true);
            PlayVideo(filename);
        }
        else if (doc->isJPG())     // le document est un JPG
        {
            QImage image;
            image.loadFromData(doc->imageblob());
            listimg << image;
        }
        else if (doc->isPDF())     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage)
            listimg = doc->pagelist();
        if (listimg.size() > 0)
        {
            int maxw(0), maxh(0);
            for (int i=0; i < listimg.size(); i++)
            {
                QPixmap pix = QPixmap::fromImage(listimg.at(i));
                if (pix.width() > maxw)
                    maxw = pix.width();
                if (pix.height() > maxh)
                    maxh = pix.height();
            }
            setSizeAvailable(QSize(maxw, maxh), this, m_sizeform, m_sizewidget);
            setAttribute(Qt::WA_ShowWithoutActivating, true);
            Display(listimg, doc->titre(), pagepdf);
        }
    }
}


dlg_imagezoom::dlg_imagezoom(QMap<QString,QVariant> doc, int pagepdf, QWidget *parent) : UpDialog(parent)
{
    QList<QImage> listimg = QList<QImage>();

    if (doc.value(M_TYPE).toString() == PDF)
        listimg = Utils::calcImagefromPdf(doc.value(M_BA).toByteArray());
    else
    {
        QImage image;
        image.loadFromData(doc.value(M_BA).toByteArray());
        listimg << image;
    }
    if (listimg.size() > 0)
    {
        int maxw(0), maxh(0);
        for (int i=0; i < listimg.size(); i++)
        {
            QPixmap pix = QPixmap::fromImage(listimg.at(i));
            if (pix.width() > maxw)
                maxw = pix.width();
            if (pix.height() > maxh)
                maxh = pix.height();
        }
        setSizeAvailable(QSize(maxw, maxh), this, m_sizeform, m_sizewidget);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        Display(listimg, "", pagepdf);
    }
}


/*!
 *  \brief      dlg_imagezoom::setSizeAvailable
 *  \abstract   calc the right size for maximum size view respect with screen resolution depending on item resolution to display and screen resolution
 *  \param      size =  actual size of item in order toi cal frameration
 *  \return
    *  m_sizeform = final size for QDialog
    *  m_sizewidget = size available inside form for widget to display
*/
void dlg_imagezoom::setSizeAvailable(QSize size, UpDialog *dlg, QSize &sizeform, QSize &sizewidget)
{
    /*! screen resolution */
    double          wscroll  = 0;
    double          hscroll  = 0;
    double          screenratio = 1;
    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
    {
        wscroll     = listscreens.first()->availableGeometry().width();
        hscroll     = listscreens.first()->availableGeometry().height();
        screenratio = wscroll/hscroll;
    }

    double          wdgratio = 1;
    double          w = size.width();
    double          h = size.height();
    wdgratio        = w/h;

    dlg->resize(size.width(),size.height()); //! uses only for "fix" the form in order to calculate hdelta & wdelta

    int wdelta    = dlg->geometry().width() -dlg->sizefordisplay().width();
    int hdelta    = dlg->geometry().height() - dlg->sizefordisplay().height();

    /*! if screenration >= videoratio  => screenheight is used for max height else screenwidthfor max width */
    int finalh (0), finalw(0);
    if (screenratio >= wdgratio)
    {
        finalh = int(hscroll * 0.98);
        double hd = finalh - hdelta;    //! height available for scrollarea - double is used to cast (hd * m_vidorimgratio) to double
        finalw = int(hd * wdgratio) + wdelta;
    }
    else
    {
        finalw = int(wscroll);
        double wd = finalw - wdelta;     //! width available for scrollarea - double is used to cast (wd / m_vidorimgratio) to double
        finalh = int(wd / wdgratio) + wdelta;
    }
    sizeform      = QSize(finalw, finalh);
    sizewidget    = QSize(finalw - wdelta, finalh - hdelta);
    dlg->resize(sizeform);
}


void dlg_imagezoom::Display(QList<QImage> listimage, QString nomdoc, int pagepdf)
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_tblwdg            = new UpTableWidget();
    m_labinfowdg        = new UpLabel(m_tblwdg);
    m_tblwdg             ->installEventFilter(this);

    dlglayout()          ->insertWidget(0,m_tblwdg);
    QFont font          = qApp->font();
    font                .setPointSize(12);
    m_labinfowdg        ->setFont(font);
    m_labinfowdg        ->setText("<font color='magenta'>" + nomdoc + "</font>");

    AjouteLayButtons(UpDialog::ButtonOK);
    connect(OKButton,       &QPushButton::clicked,  this,   &dlg_imagezoom::close);
    //setMaximumWidth(m_sizeform.width());
    setMaximumHeight(m_sizeform.height());
    resize(m_sizeform.width(), m_sizeform.height());

    m_tblwdg            ->setListimages(listimage);
    m_tblwdg            ->calcSizeForDisplay(m_sizewidget);

    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
        move(0, 0);
    //move((listscreens.first()->geometry().width() - width())/2, 0);
}

void dlg_imagezoom::PlayVideo(QString filepath)
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_labinfowdg        = new UpLabel(this);
    m_vdwdg             = new UpVideoWidget();
    m_vdwdg             ->installEventFilter(this);

    dlglayout()         ->insertWidget(0,m_vdwdg);

    QFont font          = qApp->font();
    font                .setPointSize(12);
    m_labinfowdg        ->setFont(font);

    m_controlplayer     = new PlayerControls;
    AjouteWidgetLayButtons(m_controlplayer);
    AjouteLayButtons(UpDialog::ButtonOK);
    setStageCount(2);
    connect(OKButton,       &QPushButton::clicked,  this,   &dlg_imagezoom::close);

    setMinimumWidth(650);
    //setMaximumWidth(m_sizeform.width());
    setMaximumHeight(m_sizeform.height());
    m_vdwdg                 ->resize(m_sizeform);

    m_labinfowdg            ->setText("<font color='magenta'>" + filepath + "</font>");

    QMediaPlayer *player    = new QMediaPlayer;
    player                  ->setSource(QUrl::fromLocalFile(filepath));
    player                  ->setVideoOutput(m_vdwdg);

    m_controlplayer         ->setFixedWidth(500);
    m_controlplayer         ->setPlayer(player);
    m_controlplayer         ->startplay();

    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
        move((listscreens.first()->geometry().width() - width())/2, 0);//, m_size.width(), m_size.height());
}

bool dlg_imagezoom::eventFilter(QObject *obj, QEvent *event)
{
    QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
    if (rszevent != Q_NULLPTR)
    {
        if (obj==m_tblwdg)
        {
            for (int i=0; i < m_tblwdg->rowCount(); i++)
            {
                UpLabel *lbl = qobject_cast<UpLabel*>(m_tblwdg->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = QPixmap::fromImage(m_tblwdg->listimg().at(i).scaled(m_tblwdg->width(), m_tblwdg->height(),
                                                                                             Qt::KeepAspectRatioByExpanding,
                                                                                             Qt::SmoothTransformation));
                    lbl->setPixmap(pix);
                    m_tblwdg->setRowHeight(i,lbl->pixmap().height());
                    m_tblwdg->setColumnWidth(i,lbl->pixmap().width());
                }
            }
        }
        if (obj==m_tblwdg || obj == m_vdwdg)
            m_labinfowdg    ->setGeometry(20, height() - widgetbuttons()->height(), 500, 25);
    }
    return QWidget::eventFilter(obj, event);
}

