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
    int             m_stageheight       = 40;
    double          m_nstages           = 1;
    int             m_spacing           = 5;
    int             m_marge             = 10;
    int             m_correctionwith    = 0;
    QMargins        m_marges            = QMargins(m_marge,m_marge,m_marge,m_marge);
    QObject*        obj_data;
    QVBoxLayout     *m_globallay        = new QVBoxLayout(this);
    UpLabel         *wdg_label;
    UpLineEdit      *wdg_chercheuplineedit;
    QRect           m_originalgeometry  = QRect();

public:
    UpSmallButton   *OKButton, *CancelButton, *PrintButton, *SupprButton, *CloseButton, *EditButton, *RecordButton, *PdfButton, *OupsButton;
    void            setEnregPosition(bool);
    void            addSearchLine();
    void            AjouteLayButtons(Buttons Button=ButtonOK);
    void            AjouteWidgetLayButtons(QWidget *widg, bool ALaFin = true);
    void            setMode(Mode mode);
    void            TuneSize(bool fixh = true, bool fixw = true);
    Mode            mode() const;
    QVBoxLayout*    dlglayout() const;
    QHBoxLayout*    buttonslayout() const;
    QWidget*        widgetbuttons() const;
    UpLineEdit*     searchline() const                                  { return wdg_chercheuplineedit; }
    void            setStageCount(double stage =  1);
    QObject*        data() const                                        { return obj_data; }
    void            setdata(QObject* data) { obj_data = data; }
    int             stageheight() const;


    //! resize() & geometry() talks about geometry(), excluding frameGeometry()
    //! move() talks about widget framePosition
    QRect           originalgeometry() const                            { return m_originalgeometry; }
    void            setOriginalgeometry(const QRect &geometry)          { m_originalgeometry = geometry; }
    QPoint          framePosition(QRect rectgeometry) const             { return QPoint(rectgeometry.left() + frameGeometry().left() - geometry().left(),
                                                                                        rectgeometry.top() + frameGeometry().top() - geometry().top()); }

    void            setOptimalSizesForZoom(double ratioimgorigine, QSize &sizeform, QSize &sizewidget, int correctionwidth);
    void            setOriginalSizes(QSize size, QSize &sizeform, QSize &sizewidget, int correctionwidth);
    QSize           sizeForMainWidgetDisplay(int correctionwidth);

    void            setSaveGeometry(QString geometryname);

    //! difference between geometry size and all widget's size   --> contentsmargins and spacings + correctionwidth (= width of allwidgets excluding this of MainWidget)
    QSize           deltas(int correctionwidth);
    void            setCorrectionWidth(const int &correctionwidth)      { m_correctionwith = correctionwidth; }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UpDialog::Buttons)

#endif // UPDIALOG_H


