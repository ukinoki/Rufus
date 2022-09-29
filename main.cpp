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

#include "rufus.h"
#include <QApplication>
#ifdef Q_OS_LINUX
    #include "singleapplication.h"
#endif

QMap<QString,QPixmap> Icons::m_mapPixmap = QMap<QString,QPixmap>();
QMap<QString,QIcon> Icons::m_mapIcon = QMap<QString,QIcon>();

int main(int argc, char *argv[])
{
    /*! Le lancement de 2 instances du programme sur le même poste provoque un patacaisse dans la gestion des postes connectés et génère des erreurs.
     * la classe SingleApplication permet de bloquer le lancement de plusieurs instances du programme sur le même poste
     * Elle n'est pas utile sous MacOS qui ne le permet pas par défaut.
     * Elle est utile sous Linux.
     */
    #ifdef Q_OS_LINUX
        SingleApplication app(argc, argv);
    #else
        QApplication app(argc, argv);
    #endif

    QString locale = QLocale::system().name().section('_', 0, 0);
    QString dirloc;
    dirloc = QCoreApplication::applicationDirPath();
    dirloc += "/rufus" + locale;
    QTranslator translator;
    translator.load(dirloc);
    app.installTranslator(&translator);

    QSplashScreen *splash = new QSplashScreen(Icons::pxSplash());
    splash->show();
    Utils::Pause(300);
    splash->close();
    delete splash;

    Rufus w;
    w.show();

    return app.exec();
}
