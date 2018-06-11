#ifndef STYLES_H
#define STYLES_H
// définit les styles de l'application

#define STYLE_UPGROUBOXINACTIVE                     "UpGroupBox {font: bold; border: 1px solid rgb(164,164,164); border-radius: 10px;}"
#define STYLE_UPGROUBOXACTIVE                       "UpGroupBox {border: 2px solid rgb(164, 205, 255); border-radius: 10px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);}"
#define STYLE_UPSMALLBUTTON                         "UpSmallButton {border: 1px none black; padding-left: 8px; padding-top: 3px; padding-right: 8px; padding-bottom: 3px;}UpSmallButton:pressed {background-color: rgb(205, 205, 205);}UpSmallButton:focus {color : #000000; border: 1px solid rgb(164, 205, 255); border-radius: 5px;}"
#define STYLE_UPPUSHBUTTON                          "UpPushButton {border: 1px solid gray; border-radius: 5px; margin-left: 5px; margin-right: 5px;  margin-top: 3px; margin-bottom: 3px; padding-left: 8px; padding-right: 8px; padding-top: 3px; padding-bottom: 3px; qproperty-iconSize: 30px 30px; qproperty-flat: false; color : #000000; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);} UpPushButton:focus {color : #000000; border: 2px solid rgb(164, 205, 255); border-radius: 5px;} UpPushButton:pressed {color : gray; background-color: rgb(175, 175, 175);} UpPushButton:!enabled {color : gray;}"
#define STYLE_UPSWITCH                              "UpSwitch {border-radius: 10px; background-color:rgb(50,200,105,145);}"

#endif // STYLES_H
