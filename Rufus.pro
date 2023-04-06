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

QT       += sql core gui network printsupport xml serialport multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OSX {
TARGET = /Applications/Rufus
}
LINUX {
TARGET = $(HOME)/RufusApp
}

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

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
    rufus.cpp \
    procedures.cpp \
    pyxinterf.cpp \
    textprinter.cpp \
    conversionbase.cpp \
    serialthread.cpp \
    importdocsexternesthread.cpp \
    utils.cpp \
    gbl_datas.cpp \
    flags.cpp

HEADERS += rufus.h \
    ostask.h \
    procedures.h \
    pyxinterf.h \
    textprinter.h \
    conversionbase.h \
    serialthread.h \
    importdocsexternesthread.h \
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

LINUX {
include(SingleApplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication # cette instruction doit être incluse APRES la ligne précédente
}

TRANSLATIONS    = rufus_en.ts

RESOURCES += \
    assets/Fichiers/Fichiers.qrc \
    assets/Images/images.qrc \
    assets/Sons/sons.qrc

DISTFILES += \
    assets/Licenses/License.rtf \
    assets/Licenses/LICENSE

ICON += \
    Sunglasses.icns

OSX {
INCLUDEPATH += /usr/local/opt/poppler-qt5/include/poppler/qt5
LIBS += -L/usr/local/opt/poppler-qt5/lib/ -lpoppler-qt5
}
LINUX {
INCLUDEPATH += /usr/include/poppler/qt5
LIBS += -L/usr/local/lib -lpoppler-qt5
}

