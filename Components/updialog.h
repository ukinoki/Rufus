/* (C) 2020 LAINE SERGE
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
#include "uppushbutton.h"
#include "macros.h"

class UpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpDialog(QString NomSettings, QString NomPosition, QWidget *parent = Q_NULLPTR);
    explicit UpDialog(QWidget *parent = Q_NULLPTR);
    enum Button {
                NoButton                = 0x0,
                ButtonPrint             = 0x1,
                ButtonOK                = 0x2,
                ButtonSuppr             = 0x4,
                ButtonCancel            = 0x8,
                ButtonClose             = 0x10,
                ButtonEdit              = 0x20,
                ButtonRecord            = 0x40,
                ButtonOups              = 0x80
                };
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_ENUM(Button)
    enum Mode {NullMode, Modification,  Creation}; Q_ENUM(Mode)

private:
    bool            m_enregistreposition;
    QString         m_position;
    Mode            m_mode;
    QString         m_nomfichierini;
    QSettings       *m_settings;
    QHBoxLayout     *wdg_buttonslayout;
    QWidget         *wdg_buttonswidget;
    void            AjouteLay();
    void            closeEvent(QCloseEvent *);
    void            UpdateTabOrder();
    double          m_stageheight = 35;
    QObject*        obj_data;

public:
    UpSmallButton   *OKButton, *CancelButton, *PrintButton, *SupprButton, *CloseButton, *EditButton, *RecordButton;
    void            setEnregPosition(bool);
    void            AjouteLayButtons(Buttons Button=ButtonOK);
    void            AjouteWidgetLayButtons(QWidget *widg, bool ALaFin = true);
    void            EnregistrePosition();
    void            setMode(Mode mode);
    void            TuneSize(bool fix = true);
    Mode            mode() const;
    QVBoxLayout*    dlglayout();
    QHBoxLayout*    buttonslayout() const;
    QWidget*        widgetbuttons() const;
    void            setStageCount(double stage =  0);
    QObject*        data() const { return obj_data; }
    void            setdata(QObject* data) { obj_data = data; }

};

Q_DECLARE_OPERATORS_FOR_FLAGS(UpDialog::Buttons)

#endif // UPDIALOG_H


