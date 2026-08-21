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
    explicit UpDialog(QWidget *parent = nullptr);
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
    void            mouseMoveEvent(QMouseEvent* event) override;
    bool            m_enregistreposition    = false;
    Mode            m_mode;
    QString         m_nomfichierini;
    QHBoxLayout     *wdg_buttonslayout;
    QWidget         *wdg_buttonswidget;
    void            UpdateTabOrder();
    int             m_nbbuttons             = 0;
    int             m_stageheight           = 40;
    double          m_nstages               = 1;
    int             m_spacing               = 5;
    int             m_marge                 = 10;
    int             m_correctionwidth       = 0;
    QMargins        m_marges                = QMargins(m_marge,m_marge,m_marge,m_marge);
    QObject*        obj_data;
    QVBoxLayout     *m_globallay            = new QVBoxLayout(this);
    UpLabel         *wdg_label;
    UpLineEdit      *wdg_chercheuplineedit;
    QRect           m_originalgeometry      = QRect();
    Buttons         m_Buttons               = NoButton;
    QWidget         *m_voile                = nullptr;    /*!< voile sombre, créé au premier blocage */

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
    UpLineEdit*     searchline() const                                  { return wdg_chercheuplineedit; }
    void            setStageCount(double stage =  1);
    QObject*        data() const                                        { return obj_data; }
    void            setdata(QObject* data) { obj_data = data; }
    void            setSaveGeometry(QString geometryname);
    Buttons         buttons() const;

protected:
    //! Helpers réservés aux sous-classes (fiches) : tout ce qui ne sert qu'à une fiche dérivée — et
    //! jamais appelé de l'extérieur sur une autre instance — vit ici, pas dans l'API publique.

    QWidget*        widgetbuttons() const;
    int             nbbuttons() const;                                  //! amount of buttons in buttons layout

    //! resize(), size(), width() & geometry() makes reference to geometry() excluding frameGeometry()
    //! move(), pos() makes reference to geometry() including framePosition
    QRect           originalgeometry() const                            { return m_originalgeometry; }
    void            setOriginalgeometry(const QRect &geometry)          { m_originalgeometry = geometry; }

    //!position of framegeometry for a Dialog whose geometry() would be rectgeometry
    QPoint          framePosition(QRect rectgeometry) const
    {
        QPoint p(rectgeometry.left() + frameGeometry().left() - geometry().left(),
                rectgeometry.top() + frameGeometry().top() - geometry().top());
        //qDebug() << p;
        return p;
    }

    QSize           sizeForMainWidgetDisplay() const;
    void            setCorrectionWidth(const int &correctionwidth)      { m_correctionwidth = correctionwidth; }

    //! difference between geometry size and all widget's size --> contentsmargins and spacings +
    //! correctionwidth (= width of allwidgets excluding this of MainWidget).
    //! protected (et non public) : seul un viewer (sous-classe) en a besoin. Aucun appelant externe.
    QSize           deltas() const;

    //! À l'affichage, on aligne la hauteur des boutons de la rangée (cf. uniformiserHauteurBoutons).
    void            showEvent(QShowEvent *event) override;

    //! WindowBlocked/WindowUnblocked pilotent le voile (cf. montreVoile).
    bool            event(QEvent *event) override;

private:
    int             stageheight() const;
    void            montreVoile(bool actif);                            //! assombrit la fiche tant qu'une fille modale la bloque
    void            addnbbuttons(int add = 1) { m_nbbuttons += add;}
    //! Aligne tous les UpSmallButton de la rangée sur le PLUS HAUT. Corrige le défaut historique où un
    //! bouton avec texte (hauteur 45) et un bouton à icône seule (hauteur 35) n'étaient pas à la même
    //! hauteur. On n'aligne QUE vers le haut (jamais au-delà du plus grand) : la rangée ne grandit pas.
    void            uniformiserHauteurBoutons();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UpDialog::Buttons)

#endif // UPDIALOG_H


