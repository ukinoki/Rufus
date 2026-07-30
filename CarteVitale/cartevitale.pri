#--------------------
# CARTEVITALE
#--------------------
# Lecture DIRECTE de la carte Vitale, sans logiciel tiers ni CPS, et exploitation
# du résultat dans Rufus. Les 3 classes ne servent qu'à ça et qu'entre elles :
#   * LecteurVitale    : dialogue PC/SC avec le lecteur, rend la liste des porteurs ;
#   * FicheVitale      : présente les porteurs lus et l'action à mener (ouvrir, créer,
#                        salle d'attente) - seule classe appelée depuis Rufus ;
#   * RechercheDossier : recherche du dossier correspondant à un porteur, utilisée
#                        par FicheVitale uniquement.
# Franco-français : le bouton d'appel est masqué hors de France (cotationsfrance()).
INCLUDEPATH += $$PWD
SOURCES += $$PWD/lecteurvitale.cpp \
           $$PWD/fichevitale.cpp \
           $$PWD/recherchedossier.cpp
HEADERS += $$PWD/lecteurvitale.h \
           $$PWD/fichevitale.h \
           $$PWD/recherchedossier.h

# PC/SC : l'API est identique sur les trois plateformes, seule la bibliothèque à lier change.
win32:      LIBS += -lwinscard
macx:       LIBS += -framework PCSC
# Linux (pcsc-lite) : les en-têtes sont dans /usr/include/PCSC, et winscard.h y inclut <pcsclite.h>
# SANS préfixe -> il faut ce dossier dans le chemin d'inclusion (fourni par libpcsclite-dev).
unix:!macx: INCLUDEPATH += /usr/include/PCSC
unix:!macx: LIBS        += -lpcsclite
