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

#include "cls_interventions.h"

Interventions::Interventions(QObject *parent) : ItemsList(parent)
{
    map_interventions = new QMap<int, Intervention*>();
}

QMap<int, Intervention*>* Interventions::interventions() const
{
    return map_interventions;
}

Intervention* Interventions::getById(int id)
{
    QMap<int, Intervention*>::const_iterator itref = map_interventions->find(id);
    if( itref == map_interventions->constEnd() )
    {
        Intervention * ref = DataBase::I()->loadInterventionById(id);
        if (ref != Q_NULLPTR)
            add( map_interventions, ref );
        return ref;
    }
    return itref.value();
}

/*!
 * \brief Interventions::initListeByPatId
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe Interventions
 */
void Interventions::initListebyUserId(int id)
{
    m_idpat = id;
    QList<Intervention*> listInterventions = DataBase::I()->loadInterventionsByUserId(m_idpat);
    epurelist(map_interventions, &listInterventions);
    addList(map_interventions, &listInterventions, Item::Update);
}


void Interventions::SupprimeIntervention(Intervention* intervention)
{
    Supprime(map_interventions, intervention);
}

Intervention* Interventions::CreationIntervention(QHash<QString, QVariant> sets)
{
    Intervention *intervention = Q_NULLPTR;
    int idintervention = 0;
    DataBase::I()->locktables(QStringList() << TBL_LIGNESPRGOPERATOIRES);
    idintervention = DataBase::I()->selectMaxFromTable(CP_ID_LIGNPRGOPERATOIRE, TBL_LIGNESPRGOPERATOIRES, m_ok);
    bool result = (m_ok && idintervention  > 0);
    if (result)
    {
        ++ idintervention;
        sets[CP_ID_LIGNPRGOPERATOIRE] = idintervention;
        result = DataBase::I()->InsertSQLByBinds(TBL_LIGNESPRGOPERATOIRES, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cette intervention dans la base!"));
        return intervention;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_LIGNPRGOPERATOIRE] = idintervention;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDPATIENT_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toInt();
        else if (champ == CP_DATE_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toString();
        else if (champ == CP_IDUSER_LIGNPRGOPERATOIRE)            data[champ] = itset.value().toInt();
        else if (champ == CP_IDLIEU_LIGNPRGOPERATOIRE)            data[champ] = itset.value().toInt();
        else if (champ == CP_TYPEANESTH_LIGNPRGOPERATOIRE)        data[champ] = itset.value().toString();
        else if (champ == CP_TYPEINTERVENTION_LIGNPRGOPERATOIRE)  data[champ] = itset.value().toString();
        else if (champ == CP_COTE_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toString();
        else if (champ == CP_IDIOL_LIGNPRGOPERATOIRE)             data[champ] = itset.value().toInt();
        else if (champ == CP_PWRIOL_LIGNPRGOPERATOIRE)            data[champ] = itset.value().toDouble();
        else if (champ == CP_CYLIOL_LIGNPRGOPERATOIRE)            data[champ] = itset.value().toDouble();
        else if (champ == CP_OBSERV_LIGNPRGOPERATOIRE)            data[champ] = itset.value().toString();
    }
    intervention = new Intervention(data);
    if (intervention != Q_NULLPTR)
        map_interventions->insert(intervention->id(), intervention);
    return intervention;
}
