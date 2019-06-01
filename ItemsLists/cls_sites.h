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

#ifndef CLS_SITES_H
#define CLS_SITES_H

#include "cls_site.h"
#include "database.h"
#include "cls_itemslist.h"

class Sites : public ItemsList
{
private:
    QMap<int, Site*> *m_sites;    //!<Collection de tous les sites sans exception, généralistes ou pas

public:
    //GETTER
    QMap<int, Site *> *sites()     const;

    Sites(QObject *parent = Q_NULLPTR);

    bool add(Site *sit);
    void addList(QList<Site*> listSites);
    Site* getById(int id);
    void remove(Site* sit);
    void clearAll();
    void initListe();
};

#endif // SITES_H
