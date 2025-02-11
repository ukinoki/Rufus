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

#include "dlg_imagezoom.h"

dlg_imagezoom::dlg_imagezoom(QWidget *parent) : UpDialog(parent)
{
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
}
dlg_imagezoom::dlg_imagezoom(DocExterne *doc, int pagepdf, QWidget *parent) : UpDialog(parent)
{
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    QList<QImage> listimg = QList<QImage>();
    m_pagepdf   = pagepdf;
    if (doc)
    {
        if (doc->isVideo())
        {
            QString filename    = DataBase::I()->dirimagerie() + NOM_DIR_VIDEOS "/" + doc->lienversfichier();
            setVideofile(filename);
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
            double maxw(0), maxh(0);
            for (int i=0; i < listimg.size(); i++)
            {
                QPixmap pix = QPixmap::fromImage(listimg.at(i));
                if (pix.width() > maxw)
                    maxw = pix.width();
                if (pix.height() > maxh)
                    maxh = pix.height();
            }
            setSizes(maxw / maxh, this, m_sizeform, m_sizewidget);
            Display(listimg, doc->titre());
        }
    }
}

dlg_imagezoom::dlg_imagezoom(QMap<QString,QVariant> doc, int pagepdf, QWidget *parent) : UpDialog(parent)
{
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_pagepdf = pagepdf;
    setMapimg(doc);
}

void dlg_imagezoom::setVideofile(QString filename)
{
    if (m_vdwdg != Q_NULLPTR)
        delete m_vdwdg;
    m_vdwdg             = new UpVideoWidget(filename);

    QSize size          = m_vdwdg->player()->videosize();
    setStageCount(2);
    setSizes(Utils::sizeratio(size), this, m_sizeform, m_sizewidget);
    PlayVideo(filename);
}

void dlg_imagezoom::Display(QList<QImage> listimage, QString nomdoc)
{
    if (m_tblwdg == Q_NULLPTR)
    {
        m_tblwdg            = new UpTableWidget();
        dlglayout()         ->insertWidget(0,m_tblwdg);
        m_tblwdg            ->installEventFilter(this);
    }
    if (m_labinfowdg ==Q_NULLPTR)
    {
        m_labinfowdg        = new UpLabel(m_tblwdg);
        QFont font          = qApp->font();
        font                .setPointSize(12);
        m_labinfowdg        ->setFont(font);
    }

    m_tblwdg            ->clearContents();
    m_labinfowdg        ->setText("<font color='magenta'>" + nomdoc + "</font>");

    setMaximumHeight(m_sizeform.height());
    resize(m_sizeform.width(), m_sizeform.height());

    m_tblwdg            ->setListimages(listimage);
    m_tblwdg            ->calcSizeForDisplay(m_sizewidget);

    move(0, 0);
}

void dlg_imagezoom::PlayVideo(QString filepath)
{
    if (m_controlplayer == Q_NULLPTR)
    {
        m_controlplayer     = new PlayerControls(this);
        buttonslayout()     ->insertWidget(buttonslayout()->count()>1?buttonslayout()->count()-2 : 0,m_controlplayer);
    }
    if (m_labinfowdg == Q_NULLPTR)
    {
        m_labinfowdg        = new UpLabel();
        QFont font          = qApp->font();
        font                .setPointSize(12);
        m_labinfowdg        ->setFont(font);
        buttonslayout()     ->insertWidget(buttonslayout()->count()>1?buttonslayout()->count()-3 : 1,m_labinfowdg);
        buttonslayout()     ->insertSpacerItem(buttonslayout()->count()>1?buttonslayout()->count()-3 : 1,new QSpacerItem(10,10,QSizePolicy::Expanding));
    }
    dlglayout()             ->insertWidget(0,m_vdwdg);

    setMaximumHeight(m_sizeform.height());
    m_vdwdg                 ->resize(m_sizeform);

    m_labinfowdg            ->setText("<font color='magenta'>" + filepath + "</font>");

    m_controlplayer         ->setFixedWidth(500);
    m_controlplayer         ->setPlayer(m_vdwdg->player());
    m_controlplayer         ->startplay();

    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size())
        move((listscreens.first()->geometry().width() - width())/2, 0);
}

void dlg_imagezoom::setMapimg(const QMap<QString,QVariant> &newMapimg)
{
    m_mapimg = newMapimg;
    QList<QImage> listimg = QList<QImage>();
    if (m_mapimg.value(M_TYPE).toString() == PDF)
        listimg = Utils::calcImagefromPdf(m_mapimg.value(M_BA).toByteArray());
    else
    {
        QImage image;
        image.loadFromData(m_mapimg.value(M_BA).toByteArray());
        listimg << image;
    }
    if (listimg.size() > 0)
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
        setStageCount(2);
        setSizes(maxw / maxh, this, m_sizeform, m_sizewidget);
        Display(listimg);
    }
}

QMap<QString,QVariant> dlg_imagezoom::mapimg() const
{
    return m_mapimg;
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
            m_labinfowdg    ->setGeometry(20, height() - widgetbuttons()->height(), 500, 25);
        }
    }
    return QWidget::eventFilter(obj, event);
}
