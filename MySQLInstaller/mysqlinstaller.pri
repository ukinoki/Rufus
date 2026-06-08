#--------------------
# MYSQLINSTALLER
#--------------------
# Module natif d'installation / vérification de MySQL pour Rufus.
# Porté depuis l'utilitaire autonome ukinoki/mysqlinstaller-for-rufus :
# détecte (et installe/configure au besoin) un serveur MySQL conforme aux
# exigences de Rufus, et crée les comptes adminrufus / adminrufusSSL avec un
# mot de passe aléatoire propre à l'installation.
INCLUDEPATH += $$PWD
SOURCES += $$PWD/mysqlinstaller.cpp
HEADERS += $$PWD/mysqlinstaller.h
