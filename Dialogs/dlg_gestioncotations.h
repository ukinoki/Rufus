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

#ifndef DLG_GESTIONCOTATIONS_H
#define DLG_GESTIONCOTATIONS_H

#include "procedures.h"

class dlg_gestioncotations : public UpDialog
{
    Q_OBJECT
public:
    enum Mode       {Creation, Modification};    Q_ENUM(Mode)
    enum TypeActe   {Association, HorsNomenclature};    Q_ENUM(TypeActe)
    explicit        dlg_gestioncotations(enum TypeActe type, enum Mode mode, QString CodeActe = "", QWidget *parent = Q_NULLPTR);
    ~dlg_gestioncotations();

private:
    DataBase    *db = DataBase::I();
    Mode        m_mode;
    bool        m_cotationsfrance = db->parametres()->cotationsfrance();
    TypeActe    m_typeacte;
    QString     m_codeacte;
    QStringList m_listeactes;
    UpLineEdit  *wdg_codeline = Q_NULLPTR;
    UpLineEdit  *wdg_tarifoptamline = Q_NULLPTR;
    UpLineEdit  *wdg_tarifnooptamline = Q_NULLPTR;
    UpLineEdit  *wdg_tarifpratiqueline = Q_NULLPTR;
    UpLineEdit  *wdg_tipline = Q_NULLPTR;
    QWidget     *wdg_codewidg = Q_NULLPTR;
    QWidget     *wdg_tarifoptamwidg = Q_NULLPTR;
    QWidget     *wdg_tarifnooptamwidg = Q_NULLPTR;
    QWidget     *wdg_tarifpratiquewidg = Q_NULLPTR;
    QWidget     *wdg_tipwidg;

    bool        VerifFiche();
};

#endif // DLG_GESTIONCOTATIONS_H
