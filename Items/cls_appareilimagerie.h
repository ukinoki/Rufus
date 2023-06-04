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

#ifndef CLS_APPAREILIMAGERIE_H
#define CLS_APPAREILIMAGERIE_H

#include "cls_item.h"

class AppareilImagerie : public Item
{
    Q_OBJECT
private:
    QString m_titreexamen;
    QString m_nomappareil;
    QString m_nomdossierechange;

public:
    explicit AppareilImagerie(QString titreexamen = "", QString nomappareil = "", QString nomdossierechange ="")
    {
        m_titreexamen = titreexamen;
        m_nomappareil = nomappareil;
        m_nomdossierechange = nomdossierechange;
    }

    QString titreexamen() const             { return m_titreexamen; }
    QString nomappareil() const             { return m_nomappareil; }
    QString nomdossierechange() const       { return m_nomdossierechange; }
    void settitreexamen(QString txt)        { m_titreexamen = txt; }
    void setnomappareil(QString txt)        { m_nomappareil = txt; }
    void setnomdossierechange(QString txt)  { m_nomdossierechange = txt; }
};

#endif // CLS_APPAREILIMAGERIE_H
