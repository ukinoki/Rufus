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

/*
 * Première initisialization de la classe Icons
*/
#include "icons.h"
QMap<QString,QPixmap> Icons::m_mapPixmap = QMap<QString,QPixmap>();
QMap<QString,QIcon> Icons::m_mapIcon = QMap<QString,QIcon>();

#include "rufus.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name().section('_', 0, 0);
    QString dirloc;
    dirloc = QCoreApplication::applicationDirPath();
    dirloc += "/rufus" + locale;
    QTranslator translator;
    translator.load(dirloc);
    a.installTranslator(&translator);

    QSplashScreen *splash = new QSplashScreen(Icons::pxSplash());
    splash->show();
    Utils::Pause(1500);
    splash->close();
    delete splash;

    Rufus w;
    w.show();

    return a.exec();
}
