QT       += sql core gui network printsupport multimedia xml serialport multimediawidgets

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
    dlg_identificationuser.cpp \
    dlg_actesprecedents.cpp \
    dlg_autresmesures.cpp \
    dlg_refraction.cpp \
    dlg_listemesures.cpp \
    dlg_resumerefraction.cpp \
    dlg_commentaires.cpp \
    dlg_documents.cpp \
    dlg_salledattente.cpp \
    dlg_remisecheques.cpp \
    dlg_paiement.cpp \
    pyxinterf.cpp \
    uptextedit.cpp \
    uplineedit.cpp \
    upcheckbox.cpp \
    uppushbutton.cpp \
    dlg_banque.cpp \
    upcombobox.cpp \
    dlg_depenses.cpp \
    uptablewidget.cpp \
    qlinedelegate.cpp \
    dlg_identificationpatient.cpp \
    textprinter.cpp \
    dlg_recettes.cpp \
    dlg_choixdate.cpp \
    dlg_comptes.cpp \
    dlg_bilanortho.cpp \
    uplabel.cpp \
    dlg_fontdialog.cpp \
    upsmallbutton.cpp \
    dlg_identificationcorresp.cpp \
    updoublespinbox.cpp \
    upspinbox.cpp \
    dlg_param.cpp \
    dlg_docsexternes.cpp \
    dlg_gestioncomptes.cpp \
    dlg_paramconnexion.cpp \
    dlg_gestionusers.cpp \
    dlg_creeracte.cpp \
    updoublevalidator.cpp \
    conversionbase.cpp \
    serialthread.cpp \
    importdocsexternesthread.cpp \
    dlg_motifs.cpp \
    dlg_listemotscles.cpp \
    updialog.cpp \
    upswitch.cpp \
    uptoolbar.cpp \
    dlg_docsscanner.cpp \
    upmessagebox.cpp \
    qlabeldelegate.cpp \
    villecpwidget.cpp \
    widgtono.cpp \
    widgetbuttonframe.cpp \
    dlg_listecorrespondants.cpp \
    dlg_message.cpp \
    functormajpremierelettre.cpp \
    upradiobutton.cpp \
    functormessage.cpp \
    dlg_gestionlieux.cpp \
    dlg_docsvideo.cpp \
    utils.cpp \
    playercontrols.cpp \
    upgroupbox.cpp \
    cls_user.cpp \
    database.cpp \
    cls_acte.cpp \
    cls_item.cpp \
    cls_patient.cpp \
    cls_site.cpp \
    cls_villes.cpp \
    cls_compte.cpp \
    cls_users.cpp \
    gbl_datas.cpp \
    cls_patients.cpp

HEADERS  += rufus.h \
    procedures.h \
    dlg_identificationuser.h \
    dlg_actesprecedents.h \
    dlg_refraction.h \
    dlg_listemesures.h \
    dlg_resumerefraction.h \
    dlg_commentaires.h \
    dlg_documents.h \
    dlg_salledattente.h \
    dlg_remisecheques.h \
    dlg_paiement.h \
    pyxinterf.h \
    uptextedit.h \
    uplineedit.h \
    upcheckbox.h \
    uppushbutton.h \
    dlg_banque.h \
    upcombobox.h \
    dlg_depenses.h \
    uptablewidget.h \
    qlinedelegate.h \
    dlg_identificationpatient.h \
    textprinter.h \
    dlg_recettes.h \
    dlg_choixdate.h \
    dlg_comptes.h \
    dlg_bilanortho.h \
    uplabel.h \
    dlg_fontdialog.h \
    upsmallbutton.h \
    dlg_identificationcorresp.h \
    updoublespinbox.h \
    upspinbox.h \
    dlg_param.h \
    dlg_docsexternes.h \
    dlg_gestioncomptes.h \
    dlg_paramconnexion.h \
    dlg_gestionusers.h \
    dlg_creeracte.h \
    updoublevalidator.h \
    conversionbase.h \
    serialthread.h \
    importdocsexternesthread.h \
    dlg_motifs.h \
    dlg_listemotscles.h \
    updialog.h \
    upswitch.h \
    uptoolbar.h \
    dlg_docsscanner.h \
    upmessagebox.h \
    qlabeldelegate.h \
    villecpwidget.h \
    widgtono.h \
    dlg_autresmesures.h \
    widgetbuttonframe.h \
    dlg_listecorrespondants.h \
    dlg_message.h \
    functordatauser.h \
    functormajpremierelettre.h \
    upradiobutton.h \
    functormessage.h \
    dlg_gestionlieux.h \
    macros.h \
    dlg_docsvideo.h \
    utils.h \
    icons.h \
    playercontrols.h \
    upgroupbox.h \
    cls_user.h \
    database.h \
    log.h \
    cls_acte.h \
    cls_item.h \
    cls_patient.h \
    cls_site.h \
    cls_villes.h \
    cls_compte.h \
    cls_users.h \
    gbl_datas.h \
    cls_patients.h

FORMS    += \
    dlg_identificationuser.ui \
    rufus.ui \
    dlg_actesprecedents.ui \
    dlg_refraction.ui \
    dlg_commentaires.ui \
    dlg_documents.ui \
    dlg_salledattente.ui \
    dlg_remisecheques.ui \
    dlg_paiement.ui \
    dlg_banque.ui \
    dlg_depenses.ui \
    dlg_identificationpatient.ui \
    dlg_choixdate.ui \
    dlg_comptes.ui \
    dlg_bilanortho.ui \
    dlg_identificationcorresp.ui \
    dlg_param.ui \
    dlg_gestioncomptes.ui \
    dlg_paramconnexion.ui \
    dlg_creeracte.ui \
    dlg_motifs.ui \
    villecpwidget.ui \
    dlg_gestionusers.ui \
    widgtono.ui

TRANSLATIONS    = rufus_en.ts

RESOURCES += \
    Images/images.qrc \
    Sons/sons.qrc \
    Fichiers/Fichiers.qrc

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
