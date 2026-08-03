# Créer l'AppImage d'installation de Rufus

Ce script **fabrique juste le fichier `.AppImage`**. Il n'installe rien

## Les libs nécessaires
sudo apt-get install -y build-essential libmysqlclient-dev libmysqlclient21 libgl1-mesa-dev libxkbcommon-dev libpcsclite-dev libxcb-cursor0 libxcb-cursor-dev

Pas de `libmariadb3` : il tire `mariadb-common`, qui détourne `/etc/mysql/my.cnf` vers
`mariadb.cnf` et met hors chaîne la configuration écrite par l'installeur de Rufus.

## Pour fabriquer l'AppImage
cd ~/Documents/GitHub/Rufus/build_tools/Linux
export PATH="$HOME/Qt/6.111.1/gcc_64/bin:$PATH"
./creer-appimage-installeur.sh


## Pour fabriquer l'AppImage directement depuis le programme compilé
cd ~/Documents/GitHub/Rufus/build_tools/Linux
export PATH="$HOME/Qt/6.111.1/gcc_64/bin:$PATH"
./creer-appimage-installeur.sh ~/Documents/GitHub/Rufus/build/Desktop_Qt_6_11_1_Release/Rufus
