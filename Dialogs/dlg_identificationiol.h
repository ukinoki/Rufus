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

#include <procedures.h>
#include <updialog.h>
#include <cls_iols.h>
#include <updoublevalidator.h>
#include <updoublespinbox.h>
#include <uptoolbar.h>
#include <uppushbutton.h>

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
    QRegularExpression  rgx_csteA               = QRegularExpression("1[1-2][0-9]" + QString(QLocale().decimalPoint()) + "[0-9]{0,2}");
    QRegularExpression  rgx_diainjecteur        = QRegularExpression("[1-3]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegularExpression  rgx_diaoptique          = QRegularExpression("[4-9]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegularExpression  rgx_diaht               = QRegularExpression("1[1-3]" + QString(QLocale().decimalPoint()) + "[0-9]");
    QRegularExpression  rgx_haigis              = QRegularExpression("^-?[0-1]" + QString(QLocale().decimalPoint()) + "[0-9]*4");
    QRegularExpression  rgx_int                 = QRegularExpression("[0-9]{6}");
    QStandardItemModel  *m_manufacturersmodel   = Q_NULLPTR;
    QStandardItemModel  *m_IOLsmodel            = Q_NULLPTR;
    QList<int>          m_listeidIOLs           = QList<int>();
    Mode                m_mode = Creation;
    Manufacturer        *m_currentmanufacturer  = Q_NULLPTR;
    IOL                 *m_currentIOL           = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    bool                m_initok                = true;
    QImage              m_nullimage             = QImage("://IOL.png");
    QImage              m_currentIOLimage;
    QComboBox           *wdg_manufacturercombo ;
    UpLineEdit          *wdg_nomiolline;
    UpDoubleSpinBox     *wdg_puissancemaxspin;
    UpDoubleSpinBox     *wdg_puissanceminspin;
    UpDoubleSpinBox     *wdg_cylindremaxspin;
    UpDoubleSpinBox     *wdg_cylindreminspin;
    UpDoubleSpinBox     *wdg_addinterspin;
    UpDoubleSpinBox     *wdg_addnearspin;
    UpLabel             *wdg_addinterlbl        = new UpLabel;
    UpLabel             *wdg_addnearlbl         = new UpLabel;


    /*! constants */
    UpCheckBox          *wdg_nominalrb          = new UpCheckBox("nominal");
    UpCheckBox          *wdg_ulibrb             = new UpCheckBox("ulib");
    UpCheckBox          *wdg_optimizedrb        = new UpCheckBox("optimized");
    enum                TypeConstant {Nominal, Ulib, Optimized, None};    Q_ENUM(TypeConstant)
    TypeConstant        m_typcste               = Nominal;
    UpSmallButton       *wdg_modifButton        = new UpSmallButton;
    void                switchDisplayConstant(TypeConstant typcst);

    QWidget             *wdg_CsteAEcho          = new QWidget;
        /*! nominal */
    QWidget             *wdg_nominalCsteAEcho   = new QWidget;
    QWidget             *wdg_nominalCsteA       = new QWidget;
    QWidget             *wdg_nominalhaigis      = new QWidget;
    QWidget             *wdg_nominalbarrett     = new QWidget;
    QWidget             *wdg_nominalhofferq     = new QWidget;
    UpLineEdit          *wdg_Aecholine          = new UpLineEdit;
    UpLineEdit          *wdg_Aoptline           = new UpLineEdit;
    UpLineEdit          *wdg_holladayline       = new UpLineEdit;
    UpLineEdit          *wdg_haigisaline        = new UpLineEdit;
    UpLineEdit          *wdg_haigisbline        = new UpLineEdit;
    UpLineEdit          *wdg_haigiscline        = new UpLineEdit;
    UpLineEdit          *wdg_hofferQline        = new UpLineEdit;
    UpLineEdit          *wdg_barrettDFline      = new UpLineEdit;
    UpLineEdit          *wdg_barrettLFline      = new UpLineEdit;
    UpLineEdit          *wdg_olsenline          = new UpLineEdit;

        /*! ulib */
    QWidget             *wdg_ulibCsteAEcho      = new QWidget;
    QWidget             *wdg_ulibCsteA          = new QWidget;
    QWidget             *wdg_ulibhaigis         = new QWidget;
    QWidget             *wdg_ulibbarrett        = new QWidget;
    QWidget             *wdg_ulibhofferq        = new QWidget;
    QWidget             *wdg_ulibnbcases        = new QWidget;
    UpLineEdit          *wdg_UAecholine         = new UpLineEdit;
    UpLineEdit          *wdg_UAoptline          = new UpLineEdit;
    UpLineEdit          *wdg_UNbCasesline       = new UpLineEdit;
    UpLineEdit          *wdg_Uholladayline      = new UpLineEdit;
    UpLineEdit          *wdg_Uhaigisaline       = new UpLineEdit;
    UpLineEdit          *wdg_Uhaigisbline       = new UpLineEdit;
    UpLineEdit          *wdg_Uhaigiscline       = new UpLineEdit;
    UpLineEdit          *wdg_UhofferQline       = new UpLineEdit;
    UpLineEdit          *wdg_UbarrettDFline     = new UpLineEdit;
    UpLineEdit          *wdg_UbarrettLFline     = new UpLineEdit;
    UpLineEdit          *wdg_Uolsenline         = new UpLineEdit;

        /*! optimized */
    QWidget             *wdg_optimizedCsteA     = new QWidget;
    QWidget             *wdg_optimizedhaigis    = new QWidget;
    QWidget             *wdg_optimizedhofferq   = new QWidget;
    QWidget             *wdg_optimizednbcases   = new QWidget;
    UpLineEdit          *wdg_OAoptline          = new UpLineEdit;
    UpLineEdit          *wdg_ONbCasesline       = new UpLineEdit;
    UpLineEdit          *wdg_Oholladayline      = new UpLineEdit;
    UpLineEdit          *wdg_Ohaigisaline       = new UpLineEdit;
    UpLineEdit          *wdg_Ohaigisbline       = new UpLineEdit;
    UpLineEdit          *wdg_Ohaigiscline       = new UpLineEdit;
    UpLineEdit          *wdg_OhofferQline       = new UpLineEdit;

    UpLineEdit          *wdg_diaoptique;
    UpLineEdit          *wdg_diaht;
    UpLineEdit          *wdg_diainjecteur;
    UpLabel             *wdg_imgIOL;
    UpLabel             *wdg_HapticMateriaulbl;
    UpComboBox          *wdg_materiaubox        = new UpComboBox;
    UpComboBox          *wdg_hapticmateriaubox  = new UpComboBox;
    UpComboBox          *wdg_typebox            = new UpComboBox;
    UpTextEdit          *wdg_remarquetxt        = new UpTextEdit;
    UpToolBar           *wdg_toolbar            = new UpToolBar();
    UpPushButton        *wdg_recopiebutton      = Q_NULLPTR;
    UpCheckBox          *wdg_prechargechk       = new UpCheckBox(tr("Prechargé"));
    UpCheckBox          *wdg_jaunechk           = new UpCheckBox(tr("Jaune"));
    UpCheckBox          *wdg_inactifchk;
    UpCheckBox          *wdg_multifocalchk      = new UpCheckBox(tr("Multifocal"));
    UpCheckBox          *wdg_edofchk            = new UpCheckBox("EDOF");
    UpCheckBox          *wdg_toricchk           = new UpCheckBox(tr("Torique"));
    UpCheckBox          *wdg_singlepiecechk     = new UpCheckBox(tr("Monobloc"));
    QWidget             *wdg_cylindres;
    QVBoxLayout         *lay_materiau           = new QVBoxLayout();

    bool                eventFilter(QObject *obj, QEvent *event);
    void                AfficheDatasIOL(IOL *iol);
    void                changeImage();
    void                connectSignals();
    void                disconnectSignals();
    void                creeCopieIOL();
    void                menuChangeImage();
    void                EnableOKpushButton();
    void                EnableWidget(bool a);
    void                NavigueVers(UpToolBar::Choix choix);
    void                OKpushButtonClicked();
    void                reconstruitListeIOLs(Manufacturer *man);
    void                setimage(QImage img);
    void                supprimeImage();

    QString             m_stylesheet = "color:rgb(164, 205, 255); margin:5px";
};

#endif // DLG_IDENTIFICATIONIOL_H
