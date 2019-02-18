/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STYLES_H
#define STYLES_H
#include <QString>
// définit les styles de l'application

#define STYLE_UPGROUBOXINACTIVE     "UpGroupBox {font: bold; border: 1px solid rgb(164,164,164); border-radius: 10px;}"
#define STYLE_UPGROUBOXACTIVE       "UpGroupBox {border: 2px solid rgb(164, 205, 255); border-radius: 10px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);}"
#define STYLE_UPSMALLBUTTON         "UpSmallButton {border: 1px none black; padding-left: 8px; padding-top: 3px; padding-right: 8px; padding-bottom: 3px;}UpSmallButton:pressed {background-color: rgb(205, 205, 205);}UpSmallButton:focus {color : #000000; border: 1px solid rgb(164, 205, 255); border-radius: 5px;}"
#define STYLE_UPPUSHBUTTON          "UpPushButton {border: 1px solid gray; border-radius: 5px; margin-left: 5px; margin-right: 5px;  margin-top: 3px; margin-bottom: 3px; padding-left: 8px; padding-right: 8px; padding-top: 3px; padding-bottom: 3px; qproperty-iconSize: 30px 30px; qproperty-flat: false; color : #000000; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);} UpPushButton:focus {color : #000000; border: 2px solid rgb(164, 205, 255); border-radius: 5px;} UpPushButton:pressed {color : gray; background-color: rgb(175, 175, 175);} UpPushButton:!enabled {color : gray;}"
#define STYLE_UPSWITCH              "UpSwitch {border-radius: 10px; background-color:rgb(50,200,105,145);}"

class Styles
{
public:
    static QString StyleAppli() {
        QString border = "border-image: url(://wallpaper.jpg)";
        QString style =
                "QDialog{" + border + "}"
                "QGroupBox{font:bold;}"
                "QLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 5px;}"
                "QLineEdit:focus {border: 1px solid rgb(164, 205, 255);border-radius: 5px;}"
                "QRadioButton::indicator {width: 18px; height: 18px;}"
                "QRadioButton::indicator::checked {image: url(://blueball.png);}"
                "QScrollArea {background-color:rgb(237, 237, 237); border: 1px solid rgb(150,150,150);}"
                "QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #fafafa, stop: 1.0 rgb(164, 205, 255));}"
                "QTabBar::tab:hover {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #fafafa, stop: 0.4 #f4f4f4,stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);}"
                "QTabBar::tab:selected {border-color: #9B9B9B; border-bottom-color: #C2C7CB;}"
                "QTabBar::tab:!selected {margin-top: 4px;}"
                "QTabBar::tab {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #E0E0E0, stop: 1 #FFFFFF);"
                    "border: 1px solid #C4C4C3;"
                    "border-bottom-color: #C2C7CB;"
                    "border-top-left-radius: 4px; border-top-right-radius: 4px;"
                    "min-width: 8ex;"
                    "padding: 2px;}"
                "QTabWidget::pane {border-top: 1px solid #C2C7CB;}"
                "QTabWidget::tab-bar {left: 20px;}"
                "QAbstractItemView {selection-color: rgb(255, 255, 255); selection-background-color: rgb(164, 205, 255); }"
                "QTextEdit {background-color:white; border: 1px solid rgb(150,150,150); border-radius: 10px;}"
                "QTextEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}"
                "QTreeWidget{background-color: white;}"
                "QToolBar {border-radius: 5px; margin-left: 5px; margin-right: 5px;  margin-top: 3px; margin-bottom: 3px;}"
                "QWidget#centralWidget{" + border + "}"
                "QSlider::groove:horizontal {"
                    "border: 1px solid #999999;"
                    "height: 8px;" /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */
                    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
                    "margin: 2px 0;}"
                "QSlider::handle:horizontal {"
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
                    "border: 1px solid #5c5c5c;"
                    "width: 18px;"
                    "margin: -2px 0;" /* handle is placed by default on the contents rect of the groove. Expand outside the groove */
                    "border-radius: 3px;}";
        return style;
    }
};
#endif // STYLES_H
