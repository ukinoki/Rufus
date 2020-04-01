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

#ifndef DLG_IDENTIFICATIONIOL_H
#define DLG_IDENTIFICATIONIOL_H

#include "procedures.h"
#include "updialog.h"
#include "cls_interventions.h"
#include "updoublevalidator.h"

class dlg_identificationIOL : public UpDialog
{
public:
    enum                        Mode {Creation, Modification};    Q_ENUM(Mode)
    explicit                    dlg_identificationIOL(enum Mode mode, IOL *iol = Q_NULLPTR, Manufacturer *man = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    dlg_identificationIOL();
    void                        setNomIOL(QString nom)  {wdg_nomiolline->setText(nom);}
    IOL*                        currentIOL() const      { return m_currentIOL; }
    QHash<QString, QVariant>    Listbinds() const       { return m_listbinds;  }
    bool                        initok() const          { return m_initok; }

private:
    QRegExp             rgx_csteA           = QRegExp("1[1-2][0-9]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_diainjecteur    = QRegExp("[1-3]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_diaoptique      = QRegExp("[4-9]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_diaht           = QRegExp("1[1-3]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegExp             rgx_haigis          = QRegExp("[0]" + QString(QLocale().decimalPoint()) + "[0-9]*4");
    QStandardItemModel  *m_manufacturersmodel   = Q_NULLPTR;
    QStandardItemModel  *m_IOLsmodel            = Q_NULLPTR;
    Mode                m_mode;
    Manufacturer        *m_currentmanufacturer  = Q_NULLPTR;
    IOL                 *m_currentIOL           = Q_NULLPTR;
    QHash<QString, QVariant>
                        m_listbinds;
    bool                m_initok = true;
    QComboBox           *wdg_manufacturercombo ;
    UpLineEdit          *wdg_nomiolline;
    UpLineEdit          *wdg_Aoptline;
    UpLineEdit          *wdg_Aecholine;
    UpLineEdit          *wdg_ACDline;
    UpLineEdit          *wdg_haigisaline;
    UpLineEdit          *wdg_haigisbline;
    UpLineEdit          *wdg_haigiscline;
    UpLineEdit          *wdg_diaoptique;
    UpLineEdit          *wdg_diaht;
    UpLineEdit          *wdg_diainjecteur;
    UpLabel             *wdg_imgIOL;
    UpLineEdit          *wdg_materiauline;
    UpTextEdit          *wdg_remarquetxt;
    QCheckBox           *wdg_prechargechk;
    QCheckBox           *wdg_jaunechk;
    QCheckBox           *wdg_inactifchk;
    QCheckBox           *wdg_multifocalchk;

    void                AfficheDatasIOL();
    void                changeImage();
    void                menuChangeImage();
    void                EnableOKpushButton();
    void                OKpushButtonClicked();
    void                setimage(QImage img);
    void                setpdf(QByteArray ba);
    void                supprimeImage();
};

#endif // DLG_IDENTIFICATIONIOL_H
