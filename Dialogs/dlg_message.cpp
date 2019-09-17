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

dlg_message::dlg_message(QStringList listmsg, int pause, bool bottom)
{
    QThread thread;
    moveToThread(&thread);
    thread.start();
    AfficheMsg(listmsg, pause, bottom);
    thread.exit();
    thread.wait();
}

dlg_message::dlg_message(QString msg, int pause, bool bottom)
{
    QThread thread;
    moveToThread(&thread);
    thread.start();
    QStringList listmsg = QStringList() << msg;
    AfficheMsg(listmsg, pause, bottom);
    thread.exit();
    thread.wait();
}

dlg_message::~dlg_message()
{
}

void dlg_message::delay(int msec)
{
    QEventLoop loop;
    QTimer t;
    t.setSingleShot(true);
    connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(msec);
    loop.exec();
}

void dlg_message::AfficheMsg(QStringList listmes, int pause, bool bottom)
{
    int decalage=0;
    for (int i=0; i<listmes.size(); i++)
    {
        QDialog *dlg = new QDialog();
        dlg                 ->setAttribute(Qt::WA_DeleteOnClose);
        dlg                 ->setSizeGripEnabled(false);

        UpLabel *imglbl     = new UpLabel(dlg);
        imglbl              ->setPixmap(Icons::pxDetente().scaled(45,45)); //WARNING : icon scaled : pxDetente 45,45

        UpTextEdit *Msgtxt  = new UpTextEdit(dlg);
        Msgtxt              ->setAttribute( Qt::WA_NoSystemBackground, true );
        Msgtxt              ->setText(listmes.at(i));
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
            //qDebug() << lmsg.at(k) + " - ligne = " + QString::number(k+1);
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

        int yy          = QGuiApplication::screens().first()->geometry().height();
        int xx          = QGuiApplication::screens().first()->geometry().width();
        if (bottom)
            dlg             ->move(xx - w - 45 - (marge*2) - lay->spacing()-15, yy - (int(hauteurligne)*nlignes) - marge*2 - decalage);
        dlg                 ->show();
//        qDebug() << " desktop xx = " + QString::number(xx) << "widh() = " +
//                    QString::number(w - 45 - (marge*2) - 10) << "desktop yy = " +
//                    QString::number(yy)  << "heigth() = " +
//                    QString::number((hauteurligne*nlignes) - marge*2 - decalage);
        QTimer *timer = new QTimer(dlg);
        timer           ->setSingleShot(true);
        connect(timer, &QTimer::timeout, dlg, &QDialog::reject);
        timer           ->start(pause);
        decalage        += dlg->height();
        LogMessage(listmes.at(i));
    }
}

void dlg_message::LogMessage(QString msg)
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
