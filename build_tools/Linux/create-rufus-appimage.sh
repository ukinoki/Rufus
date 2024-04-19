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
