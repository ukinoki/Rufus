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

#ifndef INTERVENTIONS_H
#define INTERVENTIONS_H

#include "cls_intervention.h"
#include "cls_itemslist.h"

class Interventions : public ItemsList
{
private:
    QMap<int, Intervention*> *map_all = Q_NULLPTR;              //!< la liste des interventions
    int m_idsession = 0;                                        //!< l'id de la session concernée par la map

public:
    explicit Interventions(QObject *parent = Q_NULLPTR);

    QMap<int, Intervention*> *interventions() const;
    Intervention *m_currentintervention = Q_NULLPTR;            //!> l'intervention en cours
    Intervention* currentintervention()                 { return m_currentintervention; }
    void setcurrentintervention(Intervention *interv);
    int idsession() const { return m_idsession ; }

    Intervention* getById(int id, bool reload = false);
    Intervention* getInterventionByDateIdPatient(QDate date, int idpatient);
    void initListebySessionId(int id);

    //!> actions sur les enregistrements
    void                SupprimeIntervention(Intervention *intervention);
    Intervention*       CreationIntervention(QHash<QString, QVariant> sets);
};

class SessionsOperatoires : public ItemsList
{
private:
    QMap<int, SessionOperatoire*> *map_all = Q_NULLPTR;         //!< la liste des sessions
    int m_iduser = 0;                                           //!< l'id du user concerné par la map

public:
    explicit SessionsOperatoires(QObject *parent = Q_NULLPTR);

    QMap<int, SessionOperatoire*> *sessions() const;
    SessionOperatoire *m_currentsession = Q_NULLPTR;            //!> la session en cours
    SessionOperatoire* currentsession()               { return m_currentsession; }
    void setcurrentsession(SessionOperatoire *sess);
    int iduser() const                      { return m_iduser ; }

    SessionOperatoire* getById(int id, bool reload = false);
    void initListebyUserId(int id);

    //!> actions sur les enregistrements
    void                SupprimeSessionOperatoire(SessionOperatoire *session);
    SessionOperatoire*  CreationSessionOperatoire(QHash<QString, QVariant> sets);
};

class TypesInterventions : public ItemsList
{
private:
    QMap<int, TypeIntervention*> *map_all = Q_NULLPTR;    //!< la liste des TypeInterventions

public:
    explicit TypesInterventions(QObject *parent = Q_NULLPTR);

    QMap<int, TypeIntervention*> *typeinterventions() const;

    TypeIntervention*   getById(int id, bool reload = false);
    void                initListe();

    //!> actions sur les enregistrements
    void                SupprimeTypeIntervention(TypeIntervention *typeintervention);
    TypeIntervention*   CreationTypeIntervention(QHash<QString, QVariant> sets);
};

#endif // INTERVENTIONS_H
