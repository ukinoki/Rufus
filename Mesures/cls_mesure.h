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

#ifndef MESURE_H
#define MESURE_H

#include <QObject>

class Mesure : public QObject
{
    Q_OBJECT
public:
    Mesure() {}

protected:
    int m_idmesure = 0;
    int m_idpatient = 0;
    bool m_cleandatas   = true;         //!> les données sont vierges
    bool m_isnullOD     = true;         //!> aucune mesure n'a été effectuée à droite
    bool m_isnullOG     = true;         //!> aucune mesure n'a été effectuée à gauche

public:
    bool isdataclean() const        { return m_cleandatas; }
    bool isnullLOD() const          { return m_isnullOD; }
    bool isnullLOG() const          { return m_isnullOG; }

    int  id() const                 { return m_idmesure; }
    void setid(int id)              { m_idmesure = id; }

    int idpatient() const           { return m_idpatient; }
    void setidpatient(int id)       { m_idpatient = id;}
 };

#endif // MESURE_H
