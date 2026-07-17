# FROM https://markuspeitl.github.io/my-linux-scripts/install-appimage.html

START_DIR=$(pwd)
APP_IMG_PATH=~/RufusQt6.AppImage
APP_IMG_NAME=$(basename $APP_IMG_PATH)
NEW_NAME=Rufus

cd ~
echo "Executing: rm -r ~/Deploy"
rm -r ~/Deploy
printf "\n"

#Making the tree
echo "Executing: mkdir -p ~/Deploy/usr/share/icons/hicolor/256x256/apps"
mkdir -p ~/Deploy/usr/share/icons/hicolor/256x256/apps
printf "\n"
echo "Executing: mkdir -p ~/Deploy/usr/share/applications"
mkdir -p ~/Deploy/usr/share/applications 
printf "\n"
echo "Executing: mkdir -p ~/Deploy/usr/bin"
mkdir -p ~/Deploy/usr/bin 
printf "\n"
echo "Executing: cp ~/RufusQt6/build_tools/Linux/rufus.desktop ~/Deploy/usr/share/applications"
cp ~/RufusQt6/build_tools/Linux/rufus.desktop ~/Deploy/usr/share/applications
printf "\n"
echo "Executing: cp ~/RufusQt6/build_tools/Linux/rufus.png ~/Deploy/usr/share/icons/hicolor/256x256/apps"
cp ~/RufusQt6/build_tools/Linux/rufus.png ~/Deploy/usr/share/icons/hicolor/256x256/apps
printf "\n"

echo "Executing: cp ~/RufusQt6App ~/Deploy/usr/bin"
cp ~/RufusQt6App ~/Deploy/usr/bin
printf "\n"

# libxcb-cursor0 : depuis Qt 6.5 le plugin de plateforme xcb (forcé par main.cpp sous Linux,
# cf. QT_QPA_PLATFORM=xcb) EXIGE cette lib pour se charger. linuxdeployqt ne la trace pas
# toujours -> on la pose explicitement dans Deploy/usr/lib pour qu'elle soit embarquée dans
# l'AppImage. Sans elle, Rufus ne démarre pas sur un poste Wayland qui ne l'a pas déjà.
echo "Executing: cp libxcb-cursor.so.0 -> ~/Deploy/usr/lib"
mkdir -p ~/Deploy/usr/lib
cp /usr/lib/x86_64-linux-gnu/libxcb-cursor.so.0 ~/Deploy/usr/lib/ 2>/dev/null \
  || echo "ATTENTION: libxcb-cursor.so.0 introuvable sur la machine de build -> faire 'sudo apt-get install libxcb-cursor0' puis relancer ce script."
printf "\n"

echo "Executing ~/linuxdeployqt ~/Deploy/usr/share/applications/rufus.desktop -appimage"
~/linuxdeployqt ~/Deploy/usr/share/applications/rufus.desktop -appimage
printf "\n"

echo "Executing: mv RufusQt6-x86_64.AppImage Rufus.AppImage"
mv RufusQt6-x86_64.AppImage $APP_IMG_NAME
printf "\n"

#Making the appimage executable
echo "Executing: chmod +x $APP_IMG_PATH"
chmod +x $APP_IMG_PATH
printf "\n"

cd ~
echo "Executing: rm -r ~/Deploy"
rm -r ~/Deploy
