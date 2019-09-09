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

#ifndef WIDGETBUTTONFRAME_H
#define WIDGETBUTTONFRAME_H

#include <QFrame>
#include "upsmallbutton.h"
#include <QLayout>

class WidgetButtonFrame : public QFrame
{
    Q_OBJECT
public:
    WidgetButtonFrame(QWidget *proprio=Q_NULLPTR);
    enum Button {
                NoButton                = 0x0,
                PlusButton              = 0x1,
                MoinsButton             = 0x2,
                ModifButton             = 0x4
                };
    Q_ENUM(Button)
    Q_DECLARE_FLAGS(Buttons, Button)
    enum Bouton {Plus, Modifier, Moins}; Q_ENUM(Bouton)
    void            AddButtons(Buttons);
    void            replace();
    UpSmallButton   *wdg_plusBouton;
    UpSmallButton   *wdg_moinsBouton;
    UpSmallButton   *wdg_modifBouton;
    QWidget*        widgButtonParent() const;
    QHBoxLayout*    layButtons() const;
    Bouton           Choix() const;

private:
    Bouton           m_reponse;
    QWidget         *wdg_proprio, *widg_parent;
    QHBoxLayout     *wdg_buttonwidglayout;
    void            Choix(int id);

signals:
    void            choix();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WidgetButtonFrame::Buttons)

#endif // WIDGETBUTTONFRAME_H
