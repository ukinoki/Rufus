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

QT       += sql core gui network printsupport xml serialport multimedia multimediawidgets webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OSX {
TARGET = /Applications/Rufus
}
LINUX {
TARGET = /home/alexandre/RufusApp
}

TEMPLATE = app

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
        rufus.cpp \
    procedures.cpp \
    pyxinterf.cpp \
    textprinter.cpp \
    conversionbase.cpp \
    serialthread.cpp \
    importdocsexternesthread.cpp \
    qlabeldelegate.cpp \
    utils.cpp \
    gbl_datas.cpp

HEADERS += rufus.h \
    procedures.h \
    pyxinterf.h \
    textprinter.h \
    conversionbase.h \
    serialthread.h \
    importdocsexternesthread.h \
    qlabeldelegate.h \
    macros.h \
    utils.h \
    icons.h \
    log.h \
    gbl_datas.h \
    styles.h

FORMS += \
    rufus.ui


#--------------------
# INCLUDE
#--------------------
include(Components/components.pri)
include(Database/database.pri)
include(Dialogs/dialogs.pri)
include(Items/items.pri)
include(TcpSocket/tcpsocket.pri)
include(Widgets/widgets.pri)

TRANSLATIONS    = rufus_en.ts

RESOURCES += \
    assets/Images/images.qrc \
    assets/Sons/sons.qrc \
    assets/Fichiers/Fichiers.qrc \

ICON += \
    Sunglasses.icns

OSX {
INCLUDEPATH += /usr/local/include/poppler/qt5
LIBS += -L/usr/local/lib/ -lpoppler-qt5
}
LINUX {
INCLUDEPATH += /usr/include/poppler/qt5
LIBS += -L/usr/local/lib -lpoppler-qt5
}

DISTFILES += \
    _Diagrams/ImpressionsRufus.vpp \
    _Diagrams/readme.txt \
    _Diagrams/Gestion des documents sous Rufus.pdf

ALEX {
    DEFINES += ALEX
}

