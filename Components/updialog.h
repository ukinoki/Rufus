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
#include <QtCore/qdir.h>
#include "macros.h"
#include "icons.h"

#include "upsmallbutton.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "uppushbutton.h"

class UpLabel;
class UpSmallButton;
class UpLineEdit;
class UpPushButton;

class UpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpDialog(QString NomFiche, QWidget *parent = Q_NULLPTR);
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
                ButtonOups              = 0x80,
                ButtonPdf               = 0x100
                };
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_ENUM(Button)
    enum Mode {NullMode, Modification,  Creation}; Q_ENUM(Mode)

private:
    bool            m_enregistreposition;
    Mode            m_mode;
    QString         m_nomfichierini;
    QHBoxLayout     *wdg_buttonslayout;
    QWidget         *wdg_buttonswidget;
    void            AjouteLay();
    void            UpdateTabOrder();
    int             m_stageheight = 40;
    int             m_nstages   = 1;
    int             m_spacing = 5;
    int             m_marge = 10;
    QMargins        m_marges = QMargins(m_marge,m_marge,m_marge,m_marge);
    QObject*        obj_data;
    UpLabel         *wdg_label;
    UpLineEdit      *wdg_chercheuplineedit;

public:
    UpSmallButton   *OKButton, *CancelButton, *PrintButton, *SupprButton, *CloseButton, *EditButton, *RecordButton, *PdfButton, *OupsButton;
    void            setEnregPosition(bool);
    void            addSearchLine();
    void            AjouteLayButtons(Buttons Button=ButtonOK);
    void            AjouteWidgetLayButtons(QWidget *widg, bool ALaFin = true);
    void            setMode(Mode mode);
    void            TuneSize(bool fixh = true, bool fixw = true);
    Mode            mode() const;
    QVBoxLayout*    dlglayout();
    QHBoxLayout*    buttonslayout() const;
    QWidget*        widgetbuttons() const;
    UpLineEdit*     searchline() const { return wdg_chercheuplineedit; }
    void            setStageCount(int stage =  1);
    QObject*        data() const { return obj_data; }
    void            setdata(QObject* data) { obj_data = data; }
    QSize                   sizefordisplay() {
        int width = frameGeometry().width()
                    - dlglayout()->contentsMargins().left()
                    - dlglayout()->contentsMargins().right();
        int height = frameGeometry().height()
                     - dlglayout()->contentsMargins().top()
                     - dlglayout()->contentsMargins().bottom()
                     - dlglayout()->spacing()
                     - (m_stageheight*m_nstages);
        return QSize(width,height);
    }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UpDialog::Buttons)

#endif // UPDIALOG_H


