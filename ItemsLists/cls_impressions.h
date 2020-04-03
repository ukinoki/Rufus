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

#ifndef CLS_IMPRESSIONS_H
#define CLS_IMPRESSIONS_H

#include "cls_impression.h"
#include "cls_itemslist.h"

class Impressions : public ItemsList
{
private:
    QMap<int, Impression*> *map_impressions = Q_NULLPTR;      //!< la liste des impressions

public:
    explicit Impressions(QObject *parent = Q_NULLPTR);

    QMap<int, Impression *> *impressions() const;

    Impression* getById(int id);
    void initListe();
};

class DossiersImpressions : public ItemsList
{
private:
    QMap<int, DossierImpression*> *map_dossiersimpressions = Q_NULLPTR;      //!< la liste des dossiers d'impressions

public:
    explicit DossiersImpressions();

    QMap<int, DossierImpression *> *dossiersimpressions() const;

    DossierImpression* getById(int id);
    void initListe();
};


#endif // CLS_IMPRESSIONS_H
