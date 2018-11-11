QT       += sql core gui network printsupport multimedia xml serialport multimediawidgets

QMAKE_CXX = g++-7

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

isEmpty(PREFIX) {
 PREFIX = /usr/local
}
TARGET = Rufus
TARGET.path = $$PREFIX/

TEMPLATE = app

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
    functormessage.cpp \
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
    functormessage.h \
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

INCLUDEPATH += /build/rufus/stage/include/poppler/qt5

LIBS += -Llibs/poppler -lpoppler-qt5

QTPLUGIN += QSQLMYSQL

DISTFILES += \
    _Diagrams/ImpressionsRufus.vpp \
    _Diagrams/readme.txt \
    Rufus.AppDir/Rufus.desktop \
    Rufus.AppDir/rufus.png
