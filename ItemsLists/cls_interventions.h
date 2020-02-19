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

#ifndef INTERVENTIONS_H
#define INTERVENTIONS_H

#include "cls_intervention.h"
#include "cls_itemslist.h"

class Interventions : public ItemsList
{
private:
    QMap<int, Intervention*> *map_interventions = Q_NULLPTR;    //!< la liste des interventions
    int m_iduser = 0;                                            //!< l'id du patient concerné par la map

public:
    explicit Interventions(QObject *parent = Q_NULLPTR);

    QMap<int, Intervention*> *interventions() const;
    int iduser() const { return m_iduser ; }

    Intervention* getById(int id);
    void initListebyUserId(int id);

    //!> actions sur les enregistrements
    void                SupprimeIntervention(Intervention *intervention);
    Intervention*       CreationIntervention(QHash<QString, QVariant> sets);
};

class IOLs : public ItemsList
{
private:
    QMap<int, IOL*> *map_IOLs = Q_NULLPTR;    //!< la liste des IOLS

public:
    explicit IOLs(QObject *parent = Q_NULLPTR);

    QMap<int, IOL*> *iols() const;

    IOL*    getById(int id);
    void    initListe();

    //!> actions sur les enregistrements
    void    SupprimeIOL(IOL *iol);
    IOL*    CreationIOL(QHash<QString, QVariant> sets);
};

class TypesInterventions : public ItemsList
{
private:
    QMap<int, TypeIntervention*> *map_typeinterventions = Q_NULLPTR;    //!< la liste des TypeInterventions

public:
    explicit TypesInterventions(QObject *parent = Q_NULLPTR);

    QMap<int, TypeIntervention*> *typeinterventions() const;

    TypeIntervention*   getById(int id);
    void                initListe();

    //!> actions sur les enregistrements
    void                SupprimeTypeIntervention(TypeIntervention *typeintervention);
    TypeIntervention*   CreationTypeIntervention(QHash<QString, QVariant> sets);
};

#endif // INTERVENTIONS_H
