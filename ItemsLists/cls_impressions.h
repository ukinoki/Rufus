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
    QMap<int, Impression*> *map_all = Q_NULLPTR;      //!< la liste des impressions
    bool m_isfull = false;

public:
    explicit Impressions(QObject *parent = Q_NULLPTR);

    QMap<int, Impression *> *impressions() const;

    Impression* getById(int id, bool reload = false);
    void initListe();
    bool isfull() const { return  m_isfull; }

    //!> actions sur les enregistrements
    void          SupprimeImpression(Impression *impr);
    Impression*   CreationImpression(QHash<QString, QVariant> sets);
};

class DossiersImpressions : public ItemsList
{
private:
    QMap<int, DossierImpression*> *map_all = Q_NULLPTR;      //!< la liste des dossiers d'impressions
    bool m_isfull = false;

public:
    explicit DossiersImpressions();

    QMap<int, DossierImpression *> *dossiersimpressions() const;
    bool isfull() const { return  m_isfull; }

    DossierImpression* getById(int id, bool reload = false);
    void initListe();
    void loadlistedocs(DossierImpression *dossier);
    void setlistedocs(DossierImpression *dossier, QList<int> listiddocs);
    //!> actions sur les enregistrements
    void                    SupprimeDossierImpression(DossierImpression *impr);
    DossierImpression*      CreationDossierImpression(QHash<QString, QVariant> sets);
};


#endif // CLS_IMPRESSIONS_H
