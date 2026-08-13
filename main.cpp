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
#include "dlg_initbase.h"
#include <QApplication>
#include <QTranslator>
#include <QKeyEvent>
#include <QLineEdit>
#include <QAbstractSpinBox>

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    #include "singleapplication.h"
#endif

QMap<QString,QPixmap> Icons::m_mapPixmap = QMap<QString,QPixmap>();
QMap<QString,QIcon> Icons::m_mapIcon = QMap<QString,QIcon>();

/*! Réécrit la touche décimale du PAVÉ NUMÉRIQUE vers le séparateur décimal local.
 *  Sous Windows et Linux, cette touche est câblée en dur par l'OS sur VK_DECIMAL et envoie
 *  TOUJOURS "." quel que soit le séparateur décimal local : aucun réglage ne le change, il faut
 *  intercepter la frappe et la réémettre. macOS, lui, envoie déjà la virgule en France — on
 *  reproduit donc ici, pour les 3 OS, le comportement attendu (QLocale().decimalPoint(), soit la
 *  locale que le parsing QLocale().toDouble() attend déjà partout dans Rufus).
 *  Pas de macro Q_OBJECT : une sous-classe de QObject qui ne fait qu'override eventFilter n'en a
 *  pas besoin (donc pas de passage par moc). */
class FiltreSeparateurDecimal : public QObject
{
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            //! uniquement la touche "." / "," du pavé numérique, sans Ctrl/Alt/Maj (on ne
            //! détourne donc jamais un raccourci du type Ctrl + pavé numérique)
            if (ke->modifiers() == Qt::KeypadModifier
                && (ke->key() == Qt::Key_Period || ke->key() == Qt::Key_Comma))
            {
                const QString sep = QLocale().decimalPoint();   //! "," en France
                //! on n'agit que dans une zone de saisie, et seulement si la frappe diffère déjà
                //! du séparateur local (si la locale utilise ".", rien à faire : locale-aware)
                if (!sep.isEmpty() && ke->text() != sep
                    && (qobject_cast<QLineEdit*>(obj) || qobject_cast<QAbstractSpinBox*>(obj)))
                {
                    //! on réémet la frappe avec le bon caractère ; QLineEdit gère l'insertion et
                    //! le remplacement d'une éventuelle sélection. L'événement réémis porte
                    //! text() == sep, donc ce filtre le laisse passer → aucune récursion.
                    QKeyEvent corrige(QEvent::KeyPress, ke->key(), ke->modifiers(), sep);
                    QApplication::sendEvent(obj, &corrige);
                    return true;                                //! on consomme l'événement d'origine
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    //! Sous Wayland, un client n'a PAS le droit de positionner lui-même ses fenêtres de premier
    //! plan : restoreGeometry() y restaure la taille mais JAMAIS la position (restriction du
    //! protocole). D'où le comportement "aléatoire" selon la session (X11 vs Wayland) et l'arrêt
    //! total depuis qu'Ubuntu démarre en Wayland. On force la plateforme xcb (XWayland), où le
    //! positionnement fonctionne comme sous X11/macOS/Windows — sans aucune perte de résolution
    //! ni de détail d'imagerie (XWayland rend au pixel près ; seul le scaling FRACTIONNAIRE de
    //! l'UI est un peu moins net). Bénéfice annexe confirmé : le redimensionnement de fenêtre à la
    //! souris, saccadé sous Wayland (limite connue de Qt), redevient fluide sous xcb. Surchargeable :
    //! si l'utilisateur a déjà fixé QT_QPA_PLATFORM, on respecte son choix (Wayland natif possible).
    //! NB déploiement : depuis Qt 6.5 le plugin xcb exige libxcb-cursor0 ; elle doit être embarquée
    //! dans l'AppImage (cf. build_tools/Linux/create-rufus-appimage.sh), sinon Rufus ne démarre pas.
    //! À poser AVANT la construction de QApplication (lue à l'initialisation de la plateforme).
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "xcb");
#endif

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

    //! Pavé numérique : la touche décimale envoie le séparateur décimal local (cf. classe ci-dessus).
    app.installEventFilter(new FiltreSeparateurDecimal(&app));

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

    //! Mise à jour DIFFÉRÉE du socle MySQL : si la connexion a détecté un MySQL trop ancien, on ne
    //! le signale (et on ne migre) qu'ICI, APRÈS w.show(), pour que Rufus soit déjà visible — il
    //! fonctionne en mode dégradé sur l'ancien socle en attendant. Le dialogue est modal (il bloque)
    //! mais la fenêtre principale, peinte juste avant, reste visible derrière (cf. la méthode).
    Procedures::I()->ControleSocleMySQLApresAffichage();

    const int ret = app.exec();
    //! NB : la sauvegarde de Rufus.ini ne se fait PLUS ici (à la fermeture), mais à l'OUVERTURE
    //! réussie (cf. constructeur Rufus, après Connexion_A_La_Base) : la fermeture ne garantit pas
    //! la cohérence du fichier, l'ouverture réussie si. On évite ainsi d'écraser une sauvegarde
    //! saine par un Rufus.ini éventuellement corrompu en cours de session.
    return ret;
}
