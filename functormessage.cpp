/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "functormessage.h"

void FunctorMessage::operator()(QStringList listmes, int pause, bool bottom)
{
    int decalage=0;
    for (int i=0; i<listmes.size(); i++)
    {
        QDialog *dlg = new QDialog();
        dlg                 ->setAttribute(Qt::WA_DeleteOnClose);
        dlg                 ->setSizeGripEnabled(false);

        UpLabel *imglbl     = new UpLabel(dlg);
        imglbl              ->setPixmap(QPixmap("://beach.png").scaled(45,45));

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
            int x   = QFontMetrics(qApp->font()).width(lmsg.at(k))*1.1; //le 1.1 est là pour tenir compte des éventuels caractères gras
            w       = (x>w? x : w);
            //qDebug() << lmsg.at(k) + " - ligne = " + QString::number(k+1);
        }
        Msgtxt              ->setFixedSize(w,hauteurligne*nlignes);
        QHBoxLayout *lay    = new QHBoxLayout(dlg);
        lay                 ->addWidget(imglbl);
        lay                 ->addWidget(Msgtxt);
        int marge           = 8;
        lay                 ->setContentsMargins( marge, marge*2, marge, marge*2);
        lay                 ->setSizeConstraint(QLayout::SetFixedSize);
        dlg                 ->setLayout(lay);
        dlg                 ->setWindowFlags(Qt::SplashScreen);

        int yy          = qApp->desktop()->availableGeometry().height();
        int xx          = qApp->desktop()->availableGeometry().width();
        if (bottom)
            dlg             ->move(xx - w - 45 - (marge*2) - lay->spacing()-15, yy - (hauteurligne*nlignes) - marge*2 - decalage);
        dlg                 ->show();
//        qDebug() << " desktop xx = " + QString::number(xx) << "widh() = " +
//                    QString::number(w - 45 - (marge*2) - 10) << "desktop yy = " +
//                    QString::number(yy)  << "heigth() = " +
//                    QString::number((hauteurligne*nlignes) - marge*2 - decalage);
        QTimer *timer   = new QTimer();
        timer           ->setSingleShot(true);
        QObject::connect(timer, SIGNAL(timeout()), dlg, SLOT(reject()));
        timer           ->start(pause);
        decalage        += dlg->height();
    }
}
