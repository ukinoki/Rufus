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

#ifndef UPDIALOG_H
#define UPDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QSettings>
#include "upsmallbutton.h"
#include "macros.h"

class UpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpDialog(QString NomSettings, QString NomPosition, QWidget *parent = Q_NULLPTR);
    explicit UpDialog(QWidget *parent=Q_NULLPTR);
    enum Button {
                NoButton                = 0x0,
                ButtonPrint             = 0x1,
                ButtonOK                = 0x2,
                ButtonSuppr             = 0x4,
                ButtonCancel            = 0x8,
                ButtonClose             = 0x10,
                ButtonEdit              = 0x20
                };
    Q_DECLARE_FLAGS(Buttons, Button)

private:
    bool            EnregPosition;
    QString         Position;
    QString         gMode;
    QString         Boutons;
    QString         NomFichIni;
    QSettings       *SettingsIni;
    QHBoxLayout     *laybuttons;
    QWidget         *widgbuttons;
    void            AjouteLay();
    void            closeEvent(QCloseEvent *);
    void            UpdateTabOrder();
    double          stageheight = 35;

public:
    UpSmallButton   *OKButton, *CancelButton, *PrintButton, *SupprButton, *CloseButton, *EditButton;
    void            setEnregPosition(bool);
    void            AjouteLayButtons(Buttons Button=ButtonOK);
    void            AjouteWidgetLayButtons(QWidget *widg, bool ALaFin = true);
    void            setMode(QString);
    void            TuneSize();
    QString         mode();
    QVBoxLayout*    dlglayout();
    QHBoxLayout*    buttonslayout();
    QWidget*        widgetbuttons();
    void            setStageCount(double stage =  0);

    QObject     *mData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UpDialog::Buttons)

#endif // UPDIALOG_H


