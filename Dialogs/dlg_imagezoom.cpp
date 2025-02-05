#include "dlg_imagezoom.h"

ImageZoom::ImageZoom(QWidget * wdg, QWidget *parent) : UpDialog(parent)
{
    /*! screen resolution */
    double wscroll      = 0;
    double hscroll      = 0;
    double screenratio  = 1;
    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
    {
        wscroll     = listscreens.first()->availableGeometry().width();
        hscroll     = listscreens.first()->availableGeometry().height();
        screenratio = wscroll/hscroll;
    }


    UpVideoWidget* vdwdg = Q_NULLPTR;
    UpLabel* labwdg     = Q_NULLPTR;
    double wdgratio = 1;
    double h = 1;
    double w = 1;
    vdwdg = dynamic_cast<UpVideoWidget*>(wdg);
    if (vdwdg)
    {
        h = vdwdg->videoSink()->videoSize().height();
        w = vdwdg->videoSink()->videoSize().width();
    }
    else
    {
        labwdg = dynamic_cast<UpLabel*>(wdg);
        if (labwdg)
        {
            h = labwdg->height();
            w = labwdg->width();
        }
        else
            return;
    }
    wdgratio = w/h;

    setAttribute(Qt::WA_ShowWithoutActivating, true);
    AjouteLayButtons(UpDialog::ButtonOK | UpDialog::ButtonPrint | UpDialog::ButtonRecord);
    resize(w,h);

    m_wdelta    = geometry().width()
                - frameGeometry().width()
                + dlglayout()->contentsMargins().right()
                + dlglayout()->contentsMargins().left();
    m_hdelta    = geometry().height()
                - frameGeometry().height()
                + dlglayout()->contentsMargins().top()
                + dlglayout()->contentsMargins().bottom()
                + dlglayout()->spacing()
                + widgetbuttons()->height();

    /*! if screenration >= videoratio  => screenheight is used for max height else screenwidthfor max width */
    int finalh (0), finalw(0);
    if (screenratio >= wdgratio)
    {
        finalh = int(hscroll);
        double hd = finalh - m_hdelta;    //! height available for scrollarea - double is used to cast (hd * m_vidorimgratio) to double
        finalw = int(hd * wdgratio) + m_wdelta;
    }
    else
    {
        finalw = int(wscroll);
        double wd = finalw - m_wdelta;     //! width available for scrollarea - double is used to cast (wd / m_vidorimgratio) to double
        finalh = int(wd / wdgratio) + m_wdelta;
    }
    resize(finalw, finalh);
    QSize szavailable = QSize(finalw - m_wdelta, finalh - m_hdelta);

    if (labwdg)
    {
        RecordButton            ->setVisible(false);
        UpLabel *lab            = new UpLabel;
        lab                     ->setImage(labwdg->image());
        QPixmap pix             = QPixmap::fromImage(labwdg->image()).scaled(szavailable,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        lab                     ->setPixmap(pix);
        lab                     ->setAutoFillBackground(false);
        dlglayout()             ->insertWidget(0,lab);
    }
    else if (vdwdg)
    {
        PrintButton                     ->setVisible(false);
        wdg_playctrl                    = new PlayerControls(this);
        AjouteWidgetLayButtons(wdg_playctrl, false);
        UpVideoWidget* vid              = new UpVideoWidget(vdwdg->filename());
        m_player                        = new QMediaPlayer;
        m_player                        ->setSource( QUrl::fromLocalFile(vid->filename()));
        m_player                        ->setVideoOutput(vid);
        m_player                        ->play();
        m_player                        ->setPosition(vdwdg->player()->position());
        vid                             ->setFixedSize(szavailable);
        dlglayout()                     ->insertWidget(0,vid);
        wdg_playctrl                    ->setPlayer(m_player);
    }
    connect(OKButton, &QPushButton::clicked, this, &QDialog::close);
    move(wscroll/2-width()/2,0);
}
