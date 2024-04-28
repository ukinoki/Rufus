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

#ifndef WIDGETBUTTONFRAME_H
#define WIDGETBUTTONFRAME_H

#include <QFrame>
#include <QAbstractItemView>
#include <QLayout>

#include "uplabel.h"
#include "uplineedit.h"
#include "upsmallbutton.h"


/*!
 * \brief The WidgetButtonFrame class
 * Cette classe gère un QFrame qui va être ajouté au bas d'un QabstractItemView
 * Le QFrame incorpore un layout avec groupe de 3 boutons positionnés à droite : ajouter, mofidifer ou supprimer un composant de la liste d'item
 * on peut choisr le nombre de boutons avec le flag Buttons
 * on peut rajouter des QWidget dans ce layout qui est accessible par la fonction laybuttons()
 * Elle s'utilise comme ça
        wdg_buttonframe     = new WidgetButtonFrame(ui->ComupTableWidget);
        wdg_buttonframe     ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
        wdg_buttonframe     ->layButtons()->insertWidget(0, ui->ChercheupLineEdit);
        wdg_buttonframe     ->layButtons()->insertWidget(0, ui->label);
        this->dlglayout()   ->insertWidget(0,wdg_buttonframe->widgButtonParent());
        connect (wdg_buttonframe,   &WidgetButtonFrame::choix,  this,   &dlg_listecommentaires::ChoixButtonFrame);
        void dlg_listecommentaires::ChoixButtonFrame()
        {
            switch (wdg_buttonframe->Choix()) {
                case WidgetButtonFrame::Plus:
                    ...;
                    break;
                case WidgetButtonFrame::Modifier:
                    ...;
                    break;
                case WidgetButtonFrame::Moins:
                    ...;
                    break;
            }
        }
    }
 */
class UpSmallButton;
class UpLabel;
class UpLineEdit;

class WidgetButtonFrame : public QFrame
{
    Q_OBJECT
public:
    WidgetButtonFrame(QAbstractItemView *proprio=Q_NULLPTR);
    enum Bouton {
                Plus             = 0x1,
                Moins            = 0x2,
                Modifier         = 0x4
                };
    Q_ENUM(Bouton)
    Q_DECLARE_FLAGS(Buttons, Bouton)
    void            AddButtons(Buttons butt);
    void            enableButtons(Buttons Butt);
    void            replace();
    UpSmallButton   *wdg_plusBouton;
    UpSmallButton   *wdg_moinsBouton;
    UpSmallButton   *wdg_modifBouton;
    QWidget*        widgButtonParent() const;
    QHBoxLayout*    layButtons() const;
    Bouton          Choix() const;
    void            addSearchLine();
    UpLineEdit*     searchline() const { return wdg_chercheuplineedit; }

private:
    Bouton           m_reponse;
    QWidget         *wdg_proprio, *widg_parent;
    QHBoxLayout     *wdg_buttonwidglayout;
    void            Choix(int id);
    UpLabel         *wdg_label;
    UpLineEdit      *wdg_chercheuplineedit;

signals:
    void            choix();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WidgetButtonFrame::Buttons)

#endif // WIDGETBUTTONFRAME_H
