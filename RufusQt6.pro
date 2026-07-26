# (C) 2018 LAINE SERGE
# This file is part of RufusAdmin or Rufus.

# RufusAdmin and Rufus are free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License,
# or any later version.

# RufusAdmin and Rufus are distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.

QT       += sql core gui network printsupport xml serialport multimedia multimediawidgets pdf

# En-têtes précompilés : compile les gros en-têtes Qt UNE fois au lieu de les re-parser dans
# chacun des ~170 fichiers du projet. Gain net à la compilation (surtout MSVC/Windows). Le
# fichier pch.h ne contient QUE des en-têtes Qt stables (cf. son commentaire).
CONFIG += precompile_header
PRECOMPILED_HEADER = pch.h

# C++20 : requis par std::source_location, utilisé comme argument par défaut d'errormsg dans
# database.h (chaque appel aux génériques SQL rapporte automatiquement sa fonction appelante).
CONFIG += c++20

# Nom du produit = « Rufus » sur les 3 plateformes (macOS : Rufus.app ; Windows : Rufus.exe ;
# Linux : binaire « Rufus »). Les anciens scopes « OSX » / « LINUX » n'étaient PAS des scopes
# qmake valides (le scope macOS est « macx », pas « OSX ») : ils ne s'appliquaient jamais et
# laissaient le nom par défaut « RufusQt6 » — d'où l'app macOS installée sous ce nom interne.
TARGET = Rufus
win32 {
RC_ICONS = assets/Images/icon.ico
RC_FILE = assets/Images/icon.rc
}
# macOS : on fournit NOTRE Info.plist (CFBundleName=Rufus, identifiant, icône, Retina…).
# Indispensable car le gabarit « Info.plist.app » manque dans le mkspec macx-clang de
# certaines versions de Qt (présent seulement dans macx-g++) : sans QMAKE_INFO_PLIST, qmake
# échoue à générer le plist d'un bundle NEUF (« Info.plist.app: no such file »).
# Valeurs codées EN DUR dans le plist (CFBundleExecutable=Rufus, CFBundleIconFile=Sunglasses.icns) :
# qmake ne substitue PAS de façon fiable les jetons @…@ dans un plist fourni. L'icône elle-même
# est copiée dans Resources par la variable ICON (Sunglasses.icns) plus bas.
macx {
    # Chemin ABSOLU via $$PWD (dossier du .pro source) : en shadow build (Qt Creator), un chemin
    # RELATIF est résolu depuis le dossier de BUILD où le fichier n'existe pas → Info.plist VIDE.
    QMAKE_INFO_PLIST = $$PWD/build_tools/macOS/Info.plist
    # Quand on fournit son propre Info.plist, qmake NE copie PLUS automatiquement l'icône
    # (la variable ICON ne déploie l'icns que via le plist qu'il génère lui-même). On copie
    # donc Sunglasses.icns à la main dans Contents/Resources, là où CFBundleIconFile la cherche.
    rufus_icon.files = $$PWD/Sunglasses.icns
    rufus_icon.path  = Contents/Resources
    QMAKE_BUNDLE_DATA += rufus_icon

    # Build UNIVERSEL (x86_64 + arm64) + en-tête précompilé : qmake génère un PCH PAR architecture
    # et l'injecte via « -Xarch_arm64 -include Rufus_arm64.pch » (et l'équivalent x86_64). Lors de la
    # passe de compilation d'une architecture, l'argument -Xarch_ destiné à l'AUTRE architecture est
    # vu comme inutilisé → une avalanche de -Wunused-command-line-argument (purement cosmétique, le
    # binaire est correct). On fait taire cette seule catégorie de warning ; le PCH reste actif.
    QMAKE_CFLAGS   += -Wno-unused-command-line-argument
    QMAKE_CXXFLAGS += -Wno-unused-command-line-argument
}

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = $$PREFIX/bin

    shortcutfiles.files = misc/rufus.desktop
    shortcutfiles.path = $$PREFIX/share/applications/
    data.files += misc/rufus.xpm
    data.path = $$PREFIX/share/pixmaps/
    desktop.path = $$PREFIX/share/applications/
    desktop.files += rufus.desktop
    icon512.path = $$PREFIX/share/icons/hicolor/512x512/apps
    icon512.files += Images/Sunglasses.png

    INSTALLS += icon512
    INSTALLS += desktop
    INSTALLS += target
    INSTALLS += shortcutfiles
    INSTALLS += data
}

SOURCES += main.cpp\
    ressources.cpp \
    rufus.cpp \
    procedures.cpp \
    lecteurvitale.cpp \
    fichevitale.cpp \
    recherchedossier.cpp \
    conversionbase.cpp \
    importdocsexternes.cpp \
    utils.cpp \
    gbl_datas.cpp \
    flags.cpp

HEADERS += rufus.h \
    ostask.h \
    procedures.h \
    lecteurvitale.h \
    fichevitale.h \
    recherchedossier.h \
    ressources.h \
    conversionbase.h \
    importdocsexternes.h \
    macros.h \
    timerthread.h \
    utils.h \
    icons.h \
    log.h \
    gbl_datas.h \
    styles.h \
    flags.h

FORMS += \
    rufus.ui

TRANSLATIONS += rufus_fr.ts \
                rufus_en.ts \
                rufus_es.ts \
                rufus_br.ts \
                rufus_pt.ts \
                rufus_it.ts

#--------------------
# INCLUDE
#--------------------
include(Components/components.pri)
include(Database/database.pri)
include(Dialogs/dialogs.pri)
include(Items/items.pri)
include(ItemsLists/itemslists.pri)
include(Mesures/mesures.pri)
include(TcpSocket/tcpsocket.pri)
include(Widgets/widgets.pri)
include(Protocols/protocols.pri)
include(TextPrinter/textprinter.pri)
include(ImageViewer/imageviewer.pri)
include(MySQLInstaller/mysqlinstaller.pri)


unix|win32{
include(SingleApplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication # cette instruction doit être incluse APRES la ligne précédente
}

# SingleApplication calls GetUser
win32 {
LIBS += -lUser32
}

# Lecture directe de la carte Vitale via PC/SC (lecteurvitale.cpp). L'API est identique sur les
# trois plateformes ; seule la bibliothèque à lier change.
win32:      LIBS += -lwinscard
macx:       LIBS += -framework PCSC
# Linux (pcsc-lite) : les en-têtes sont dans /usr/include/PCSC, et winscard.h y inclut <pcsclite.h>
# SANS préfixe -> il faut ce dossier dans le chemin d'inclusion (fourni par libpcsclite-dev).
unix:!macx: INCLUDEPATH += /usr/include/PCSC
unix:!macx: LIBS        += -lpcsclite

RESOURCES += \
    assets/Fichiers/Fichiers.qrc \
    assets/Images/images.qrc \
    assets/Sons/sons.qrc \

ICON += \
    Sunglasses.icns

DISTFILES += \
    assets/Licenses/License.rtf \
    assets/Licenses/LICENSE \
    rufus_en.qm \
    rufus_es.qm \
    rufus_fr.qm \
    rufus_br.qm \
    rufus_pt.qm \
    rufus_it.qm
