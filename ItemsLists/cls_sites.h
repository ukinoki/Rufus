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

#ifndef CLS_SITES_H
#define CLS_SITES_H

#include "cls_site.h"
#include "cls_itemslist.h"

class Sites : public ItemsList
{
private:
    QMap<int, Site*> *map_all;                        //!<Collection de tous les sites sans exception, généralistes ou pas
    Site*           m_currentsite = Q_NULLPTR;          //!> le site en cours

public:
    //GETTER
    QMap<int, Site *> *sites()     const;

    Sites(QObject *parent = Q_NULLPTR);

    Site*           getById(int id, bool reload = false);
    void            initListe();
    QList<Site*>    initListeByUser(int idusr);
    Site*           currentsite() const             { return m_currentsite; }
    void            setcurrentsite(Site* site)      { m_currentsite = site; }
    int             idcurrentsite() const           { return (m_currentsite != Q_NULLPTR? m_currentsite->id() : -1); }

    //!> actions sur les enregistrements
    void            SupprimeSite(Site *sit);
    Site*           CreationSite(QHash<QString, QVariant> sets);
};

#endif // SITES_H
