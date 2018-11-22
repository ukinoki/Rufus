#* (C) 2016 LAINE SERGE

#    This file is part of Rufus.

#    Rufus is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    Rufus is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with Rufus.  If not, see <http://www.gnu.org/licenses/>.

QT       += sql core gui network printsupport multimedia xml serialport multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = /home/travis/build/Ukinoki/Rufus

TEMPLATE = app

linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = /home/travis/build/Ukinoki/Rufus.AppDir
    desktop.path = /home/travis/build/Ukinoki/Rufus.AppDir
    desktop.files += ./Rufus.AppDir/Rufus.desktop
    shortcutfiles.path = $$PREFIX/share/applications
    icon.path = /home/travis/build/Ukinoki/Rufus.AppDir
    icon.files += ./Rufus.AppDir/rufus.png

    INSTALLS += target
    INSTALLS += desktop
    INSTALLS += shortcutfiles
    INSTALLS += icon
}

SOURCES += main.cpp\
        rufus.cpp \
    procedures.cpp \
    pyxinterf.cpp \
    qlinedelegate.cpp \
    textprinter.cpp \
    conversionbase.cpp \
    serialthread.cpp \
    importdocsexternesthread.cpp \
    qlabeldelegate.cpp \
    villecpwidget.cpp \
    widgtono.cpp \
    widgetbuttonframe.cpp \
    functormajpremierelettre.cpp \
    utils.cpp \
    playercontrols.cpp \
    gbl_datas.cpp

HEADERS += rufus.h \
    procedures.h \
    pyxinterf.h \
    qlinedelegate.h \
    textprinter.h \
    conversionbase.h \
    serialthread.h \
    importdocsexternesthread.h \
    qlabeldelegate.h \
    villecpwidget.h \
    widgtono.h \
    widgetbuttonframe.h \
    functordatauser.h \
    functormajpremierelettre.h \
    macros.h \
    utils.h \
    icons.h \
    playercontrols.h \
    log.h \
    gbl_datas.h \
    styles.h

FORMS += \
    rufus.ui \
    villecpwidget.ui \
    widgtono.ui


#--------------------
# INCLUDE
#--------------------
include(Components/components.pri)
include(Database/database.pri)
include(Dialogs/dialogs.pri)
include(Items/items.pri)
include(TcpSocket/tcpsocket.pri)

TRANSLATIONS    = rufus_en.ts

RESOURCES += \
    assets/Images/images.qrc \
    assets/Sons/sons.qrc \
    assets/Fichiers/Fichiers.qrc

ICON += \
    Sunglasses.icns

INCLUDEPATH += /usr/include/poppler/qt5

LIBS += -Llibs/poppler -lpoppler-qt5

QTPLUGIN += QSQLMYSQL

DISTFILES += \
    _Diagrams/ImpressionsRufus.vpp \
    _Diagrams/readme.txt \
    Rufus.AppDir/Rufus.desktop \
    Rufus.AppDir/rufus.png
