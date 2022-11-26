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

#ifndef DLG_IDENTIFICATIONIOL_H
#define DLG_IDENTIFICATIONIOL_H

#include "procedures.h"
#include "updialog.h"
#include "cls_iols.h"
#include "updoublevalidator.h"
#include "updoublespinbox.h"
#include "uptoolbar.h"
#include "uppushbutton.h"

class dlg_identificationIOL : public UpDialog
{
public:
    enum                        Mode {Creation, Modification};    Q_ENUM(Mode)
    explicit                    dlg_identificationIOL(IOL *iol = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    dlg_identificationIOL();
    void                        setNomIOL(QString nom)  {wdg_nomiolline->setText(nom);}
    int                         idcurrentIOL() const    { return (m_currentIOL? m_currentIOL->id() : 0); }
    bool                        initok() const          { return m_initok; }

private:
    QRegExp             rgx_csteA           = QRegExp("1[1-2][0-9]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_diainjecteur    = QRegExp("[1-3]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_diaoptique      = QRegExp("[4-9]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_diaht           = QRegExp("1[1-3]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_haigis          = QRegExp("-?[0-1]" + QString(QLocale().decimalPoint()) + "[0-9]*4");
    QStandardItemModel  *m_manufacturersmodel   = Q_NULLPTR;
    QStandardItemModel  *m_IOLsmodel            = Q_NULLPTR;
    QList<int>          m_listeidIOLs           = QList<int>();
    Mode                m_mode = Creation;
    Manufacturer        *m_currentmanufacturer  = Q_NULLPTR;
    IOL                 *m_currentIOL           = Q_NULLPTR;
    QHash<QString, QVariant>
                        m_listbinds;
    bool                m_initok = true;
    QImage              m_nullimage = QImage("://IOL.png");
    QImage              m_currentimage;
    QComboBox           *wdg_manufacturercombo ;
    UpLineEdit          *wdg_nomiolline;
    UpLineEdit          *wdg_Aoptline;
    UpLineEdit          *wdg_Aecholine;
    UpDoubleSpinBox     *wdg_puissancemaxspin;
    UpDoubleSpinBox     *wdg_puissanceminspin;
    UpDoubleSpinBox     *wdg_cylindremaxspin;
    UpDoubleSpinBox     *wdg_cylindreminspin;
    UpLineEdit          *wdg_ACDline;
    UpLineEdit          *wdg_holladayline;
    UpLineEdit          *wdg_haigisaline;
    UpLineEdit          *wdg_haigisbline;
    UpLineEdit          *wdg_haigiscline;
    UpLineEdit          *wdg_diaoptique;
    UpLineEdit          *wdg_diaht;
    UpLineEdit          *wdg_diainjecteur;
    UpLabel             *wdg_imgIOL;
    UpComboBox          *wdg_materiaubox;
    UpComboBox          *wdg_typebox;
    UpTextEdit          *wdg_remarquetxt;
    UpToolBar           *wdg_toolbar = Q_NULLPTR;
    UpPushButton        *wdg_recopiebutton = Q_NULLPTR;
    UpCheckBox          *wdg_prechargechk;
    UpCheckBox          *wdg_jaunechk;
    UpCheckBox          *wdg_inactifchk;
    UpCheckBox          *wdg_multifocalchk;
    UpCheckBox          *wdg_edofchk;
    UpCheckBox          *wdg_toricchk;
    QWidget             *wdg_cylindres;
    QVBoxLayout         *lay_materiau    = new QVBoxLayout();

    bool                eventFilter(QObject *obj, QEvent *event);
    void                AfficheDatasIOL(IOL *iol);
    void                changeImage();
    void                connectSignals();
    void                disconnectSignals();
    void                creeCopieIOL();
    void                menuChangeImage();
    void                EnableOKpushButton();
    void                EnableWidget(bool a);
    void                NavigueVers(QString but);
    void                OKpushButtonClicked();
    void                reconstruitListeIOLs(Manufacturer *man);
    void                setimage(QImage img);
    void                supprimeImage();
};

#endif // DLG_IDENTIFICATIONIOL_H
