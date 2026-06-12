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
#include "procedures.h"
#include <QApplication>
#include <QTranslator>

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
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
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    SingleApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

    /*QString locale = QLocale::system().name().section('_', 0, 0);
    QDir dirloc = QDir(QCoreApplication::applicationDirPath());
    dirloc.cdUp();
    locale = dirloc.absolutePath() + "/Locale/rufus_" + locale + ".qm";*/

    QSettings settings(PATH_FILE_INI, QSettings::IniFormat);
    QDir dirloc = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MACOS)
    dirloc.cdUp();
#endif
    QString locale = QDir::toNativeSeparators(dirloc.absolutePath() + "/Locale/rufus_" + settings.value(Param_Poste_Version).toString().toLower() + ".qm");
    QTranslator translator;
    if( translator.load(locale) )
        app.installTranslator(&translator);

    QSplashScreen *splash;
    if (QDate::currentDate().month() != 12)
        splash = new QSplashScreen(Icons::pxSplash());
    else
        splash = new QSplashScreen(Icons::pxSplashNoel());
    splash->show();
    Utils::Pause(3000);
    splash->close();
    delete splash;

    Rufus w;
    w.setApp(&app, &translator); // For dynamic translations
    w.show();

    const int ret = app.exec();
    Procedures::SauvegardeIni();   //! sauvegarde silencieuse de Rufus.ini à la fermeture (restaurable dans VerifIni)
    return ret;
}
