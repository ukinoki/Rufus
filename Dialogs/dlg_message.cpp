/* (C) 2018 LAINE SERGE
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

Message* Message::instance = Q_NULLPTR;

void Message::LogMessage(QString msg)
{
    QDir DirRssces;
    QString dirlog = QDir::homePath() + DIR_RUFUS DIR_LOGS;
    if (!DirRssces.exists(dirlog))
        DirRssces.mkdir(dirlog);
    QString datelog = QDate::currentDate().toString("yyyy-MM-dd");
    QString fileName(dirlog + "/" + datelog + "_errorlog.txt");
    QFile testfile(fileName);
    if( testfile.open(QIODevice::Append) )
    {
        QTextStream out(&testfile);
        QString timelog = QTime::currentTime().toString();
        out << timelog << " - " << "MESSAGE" << " : " << msg << "\n";
        testfile.close();
    }
}

void Message::SplashMessage(QString msg, int duree)
{
    QDialog *dlg = new QDialog();
    dlg                 ->setAttribute(Qt::WA_DeleteOnClose);
    dlg                 ->setSizeGripEnabled(false);

    UpLabel *imglbl     = new UpLabel(dlg);
    imglbl              ->setPixmap(Icons::pxDetente().scaled(45,45)); //WARNING : icon scaled : pxDetente 45,45

    UpTextEdit *Msgtxt  = new UpTextEdit(dlg);
    Msgtxt              ->setAttribute( Qt::WA_NoSystemBackground, true );
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
        int x   = int(QFontMetrics(qApp->font()).width(lmsg.at(k))*1.1); //le 1.1 est là pour tenir compte des éventuels caractères gras
        w       = (x>w? x : w);
    }
    Msgtxt              ->setFixedSize(w,int(hauteurligne)*nlignes);
    QHBoxLayout *lay    = new QHBoxLayout(dlg);
    lay                 ->addWidget(imglbl);
    lay                 ->addWidget(Msgtxt);
    int marge           = 8;
    lay                 ->setContentsMargins( marge, marge*2, marge, marge*2);
    lay                 ->setSizeConstraint(QLayout::SetFixedSize);
    dlg                 ->setLayout(lay);
    dlg                 ->setWindowFlags(Qt::SplashScreen);

    int yy              = qApp->desktop()->availableGeometry().height();
    int xx              = qApp->desktop()->availableGeometry().width();
    dlg                 ->move(xx - w - 45 - (marge*2) - lay->spacing()-15, yy - (int(hauteurligne)*nlignes) - marge*2);
    dlg                 ->show();
    //        qDebug() << " desktop xx = " + QString::number(xx) << "widh() = " +
    //                    QString::number(w - 45 - (marge*2) - 10) << "desktop yy = " +
    //                    QString::number(yy)  << "heigth() = " +
    //                    QString::number((hauteurligne*nlignes) - marge*2);
    QTimer::singleShot(duree, dlg, &QDialog::reject);
}

void Message::PriorityMessage(QString msg, qintptr &idmessage)
{
    idprioritymessage ++;
    idmessage           = idprioritymessage;
    QDialog             *prioritydlg = new QDialog();
    prioritydlg         ->setAttribute(Qt::WA_DeleteOnClose);
    prioritydlg         ->setSizeGripEnabled(false);

    UpLabel *imglbl     = new UpLabel(prioritydlg);
    imglbl              ->setPixmap(Icons::pxDetente().scaled(45,45)); //WARNING : icon scaled : pxDetente 45,45

    UpTextEdit *Msgtxt  = new UpTextEdit(prioritydlg);
    Msgtxt              ->setAttribute( Qt::WA_NoSystemBackground, true );
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
        int x   = int(QFontMetrics(qApp->font()).width(lmsg.at(k))*1.1); //le 1.1 est là pour tenir compte des éventuels caractères gras
        w       = (x>w? x : w);
    }
    Msgtxt              ->setFixedSize(w,int(hauteurligne)*nlignes);
    QHBoxLayout *lay    = new QHBoxLayout(prioritydlg);
    lay                 ->addWidget(imglbl);
    lay                 ->addWidget(Msgtxt);
    int marge           = 8;
    lay                 ->setContentsMargins( marge, marge*2, marge, marge*2);
    lay                 ->setSizeConstraint(QLayout::SetFixedSize);
    prioritydlg                 ->setLayout(lay);
    prioritydlg                 ->setWindowFlags(Qt::SplashScreen);

    int yy              = qApp->desktop()->availableGeometry().height();
    int xx              = qApp->desktop()->availableGeometry().width();
    prioritydlg         ->move(xx/2 - w/2 - marge - lay->spacing()-15, yy/2 - (int(hauteurligne)*nlignes)/2 - marge);
    prioritydlg         ->show();
    Utils::Pause(500);
    connect(this,   &Message::closeprioiritydlg, prioritydlg, [=](qintptr a) { if (idmessage == a) prioritydlg->reject(); });
}
