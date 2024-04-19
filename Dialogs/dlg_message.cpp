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

#include "dlg_message.h"

ShowMessage* ShowMessage::instance = Q_NULLPTR;
ShowMessage* ShowMessage::I()
{
    if( !instance )
        instance = new ShowMessage();
    return instance;
}
ShowMessage::ShowMessage()
{
    idprioritymessage = 0;
}

void ShowMessage::SplashMessage(QString msg, int duree)
{
    QDialog *dlg = new QDialog();
    dlg                 ->setAttribute(Qt::WA_DeleteOnClose);
    dlg                 ->setSizeGripEnabled(false);

    QLabel *imglbl     = new QLabel(dlg);
    imglbl              ->setPixmap(Icons::pxDetente().scaled(45,45)); //WARNING : icon scaled : pxDetente 45,45
    imglbl              ->setFixedWidth(45);

    QTextEdit *Msgtxt  = new QTextEdit(dlg);
    Msgtxt              ->setAttribute( Qt::WA_NoSystemBackground, true );
    QString txt = msg;
    if (msg.contains("<!DOCTYPE HTML PUBLIC"))
    {
        //! parce que de vielles versions de QT enregistraient la police avec tout un lot d'attributs et Qt6 ne comprend pas
        epureFontFamily(txt);
        if (!msg.contains(HTMLCOMMENT))
        {
            QString newsize = "font-size:" + QString::number(qApp->font().pointSize()) + "pt";
            QRegularExpression rs;
            rs.setPattern("font-size( *: *[\\d]{1,2} *)pt");
            QRegularExpressionMatch const match = rs.match(msg);
            if (match.hasMatch()) {
                QString matcheds = match.captured(0);
                txt.replace(matcheds, newsize);
            }
        }
        Msgtxt->setText(txt);
    }
    else
        Msgtxt          ->setText(msg);
    Msgtxt              ->setReadOnly(true);
    Msgtxt              ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Msgtxt              ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList lmsg            = Msgtxt->toPlainText().split("\n");
    int         w               = 0;
    double      hauteurligne    = QFontMetrics(qApp->font()).height()*1.2;
    int         nlignes         = lmsg.size();
    for (int k=0; k<nlignes; k++)
    {
        int x   = int(QFontMetrics(qApp->font()).horizontalAdvance(lmsg.at(k))*1.1); //le 1.1 est là pour tenir compte des éventuels caractères gras
        w       = (x>w? x : w);
    }
    Msgtxt              ->setFixedSize(w,int(hauteurligne)*nlignes);
    QHBoxLayout *lay    = new QHBoxLayout(dlg);
    lay                 ->addWidget(imglbl);
    lay                 ->addWidget(Msgtxt);
    int marge           = 16;
    lay                 ->setContentsMargins( marge, marge*2, marge, marge*2);
    dlg                 ->setLayout(lay);
    lay                 ->addSpacerItem(new QSpacerItem(marge,0,QSizePolicy::Fixed, QSizePolicy::Fixed));
    dlg                 ->setFixedHeight((hauteurligne)*nlignes + marge*4);
    dlg                 ->setFixedWidth(w + imglbl->width() + marge*4);
    dlg                 ->setWindowFlags(Qt::SplashScreen);

    int yy              = QGuiApplication::primaryScreen()->availableGeometry().height();
    int xx              = QGuiApplication::primaryScreen()->availableGeometry().width();
//    dlg                 ->move(xx - w - 45 - (marge*2) - lay->spacing()-15, yy - (int(hauteurligne)*nlignes) - marge*2);

    // Calculate the size of dlg
    int tx = w - 45 - (marge*2) - lay->spacing()-15;
    int ty =(int(hauteurligne)*nlignes) - marge*2;

    // Calculate the size of dlg (from lay) IF isValid()
    QSize sz =dlg->sizeHint();
    if( sz.isValid() )
    {
        tx= sz.width();
        ty= sz.height();
    }

    dlg                 ->move(xx - tx, yy - ty);
    dlg                 ->show();
    QTimer::singleShot(duree, dlg, &QDialog::close);
}

void ShowMessage::PriorityMessage(QString msg, qintptr &idmessage, int duree, QWidget *parent)
{
    idprioritymessage ++;
    idmessage           = idprioritymessage;
    QDialog             *prioritydlg = new QDialog(parent);
    prioritydlg         ->setAttribute(Qt::WA_DeleteOnClose);
    prioritydlg         ->setSizeGripEnabled(false);

    QLabel *imglbl     = new QLabel(prioritydlg);
    imglbl              ->setPixmap(Icons::pxDetente().scaled(45,45)); //WARNING : icon scaled : pxDetente 45,45
    imglbl              ->setFixedWidth(45);

    QTextEdit *Msgtxt  = new QTextEdit(prioritydlg);
    Msgtxt              ->setAttribute( Qt::WA_NoSystemBackground, true );
    QString txt = msg;
    if (msg.contains("<!DOCTYPE HTML PUBLIC"))
    {
        //! parce que de vielles versions de QT enregistraient la police avec tout un lot d'attributs et Qt6 ne comprend pas
        epureFontFamily(txt);
        if (!msg.contains(HTMLCOMMENT))
        {
            QString newsize = "font-size:" + QString::number(qApp->font().pointSize()) + "pt";
            QRegularExpression rs;
            rs.setPattern("font-size( *: *[\\d]{1,2} *)pt");
            QRegularExpressionMatch const match = rs.match(msg);
            if (match.hasMatch()) {
                QString matcheds = match.captured(0);
                txt.replace(matcheds, newsize);
            }
        }
        Msgtxt->setText(txt);
    }
    else
        Msgtxt              ->setText(msg);
    Msgtxt              ->setReadOnly(true);
    Msgtxt              ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Msgtxt              ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList lmsg            = Msgtxt->toPlainText().split("\n");
    int         w               = 0;
    double      hauteurligne    = QFontMetrics(qApp->font()).height()*1.2;
    int         nlignes         = lmsg.size();
    for (int k=0; k<nlignes; k++)
    {
        int x   = int(QFontMetrics(qApp->font()).horizontalAdvance(lmsg.at(k))*1.1); //le 1.1 est là pour tenir compte des éventuels caractères gras
        w       = (x>w? x : w);
    }
    Msgtxt              ->setFixedSize(w,int(hauteurligne)*nlignes);
    QHBoxLayout *lay    = new QHBoxLayout(prioritydlg);
    lay                 ->addWidget(imglbl);
    lay                 ->addWidget(Msgtxt);
    int marge           = 16;
    lay                 ->setContentsMargins( marge, marge*2, marge, marge*2);
    lay                 ->addSpacerItem(new QSpacerItem(marge,0,QSizePolicy::Fixed, QSizePolicy::Fixed));
    prioritydlg         ->setLayout(lay);
    prioritydlg         ->setFixedHeight((hauteurligne)*nlignes + marge*4);
    prioritydlg         ->setFixedWidth(w + imglbl->width() + marge*4);
    prioritydlg         ->setWindowFlags(Qt::SplashScreen);


    int yy              = QGuiApplication::primaryScreen()->availableGeometry().height();
    int xx              = QGuiApplication::primaryScreen()->availableGeometry().width();
    prioritydlg         ->move(xx/2 - w/2 - marge - lay->spacing()-15, yy/2 - (int(hauteurligne)*nlignes)/2 - marge);
    prioritydlg         ->show();
    if (parent != Q_NULLPTR)
        parent->setEnabled(false);
    int msec = 500;
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    connect(this,   &ShowMessage::closeprioiritydlg, prioritydlg, [=](qintptr a) { if (idmessage == a) {
            if (prioritydlg->parent() != Q_NULLPTR)
                qobject_cast<QWidget*>(prioritydlg->parent())->setEnabled(true);
            prioritydlg->reject();
        }
        });
    if (duree > 0)
        QTimer::singleShot(duree, prioritydlg, &QDialog::close);

}

bool ShowMessage::epureFontFamily(QString &text)
{
    QString txt= text;
    QRegularExpression rx;
    rx.setPattern("font-family:'([a-zA-Z0-9 ,-]+)");
    auto it = rx.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString txtaremplacer = match.captured(0);
        if (txtaremplacer != "")
        {
            QString replacmt = txtaremplacer.split(",").at(0);
            text.replace(txtaremplacer, replacmt);
        }
    }
    return (txt != text);
}

