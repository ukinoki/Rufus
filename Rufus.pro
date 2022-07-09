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
TARGET = /home/serge/RufusApp
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

TRANSLATIONS    = rufus_en.ts

RESOURCES += \
    assets/Fichiers/Fichiers.qrc \
    assets/Images/images.qrc \
    assets/Sons/sons.qrc \

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
    _Diagrams/Gestion des documents sous Rufus.pdf \
    _Diagrams/Recuperation_CCAM-howto.txt \
    _Diagrams/Creer_des_procedures_mysql_avec_Qt.rtf \
    assets/A_faire.odt \
    assets/Fichiers/ComptaMedicale.sql \
    assets/Fichiers/Corps_OrdoALD.txt \
    assets/Fichiers/Corps_Ordonnance.txt \
    assets/Fichiers/Entete_OrdoALD.txt \
    assets/Fichiers/Entete_Ordonnance.txt \
    assets/Fichiers/Images.sql \
    assets/Fichiers/Ophtalmologie.sql \
    assets/Fichiers/Pied_Ordonnance.txt \
    assets/Fichiers/Pied_Ordonnance_Lunettes.txt \
    assets/Fichiers/majbase54.sql \
    assets/Fichiers/majbase55.sql \
    assets/Fichiers/majbase56.sql \
    assets/Fichiers/majbase57.sql \
    assets/Fichiers/majbase58.sql \
    assets/Fichiers/majbase59.sql \
    assets/Fichiers/majbase60.sql \
    assets/Fichiers/majbase61.sql \
    assets/Fichiers/majbase62.sql \
    assets/Fichiers/majbase63.sql \
    assets/Fichiers/majbase64.sql \
    assets/Fichiers/majbase65.sql \
    assets/Fichiers/majbase66.sql \
    assets/Fichiers/majbase67.sql \
    assets/Fichiers/majbase68.sql \
    assets/Fichiers/majbase69.sql \
    assets/Fichiers/majbase70.sql \
    assets/Fichiers/majbase71.sql \
    assets/Fichiers/oplus.sql \
    assets/Fichiers/pdf.pdf \
    assets/Fichiers/recupccam.sql \
    assets/Fichiers/rufus.sql
