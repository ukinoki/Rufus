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

#include "cls_refractions.h"

Refractions::Refractions(QObject *parent) : ItemsList(parent)
{
    map_refractions = new QMap<int, Refraction*>();
}

QMap<int, Refraction *>* Refractions::refractions() const
{
    return map_refractions;
}

Refraction* Refractions::getById(int id)
{
    QMap<int, Refraction*>::const_iterator itref = map_refractions->find(id);
    if( itref == map_refractions->constEnd() )
    {
        Refraction * ref = DataBase::I()->loadRefractionById(id);
        if (ref != Q_NULLPTR)
            add( map_refractions, ref );
        return ref;
    }
    return itref.value();
}

/*!
 * \brief Refractions::initListeByPatId
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe Refractions
 */
void Refractions::initListebyPatId(int id)
{
    m_idpat = id;
    QList<Refraction*> listrefractions = DataBase::I()->loadRefractionsByPatId(m_idpat);
    epurelist(map_refractions, &listrefractions);
    addList(map_refractions, &listrefractions, Item::Update);
}


void Refractions::SupprimeRefraction(Refraction* ref)
{
    Supprime(map_refractions, ref);
}

Refraction* Refractions::CreationRefraction(QHash<QString, QVariant> sets)
{
    Refraction *ref = Q_NULLPTR;
    int idrefraction = 0;
    DataBase::I()->locktables(QStringList() << TBL_REFRACTIONS);
    idrefraction = DataBase::I()->selectMaxFromTable(CP_ID_REFRACTIONS, TBL_REFRACTIONS, m_ok);
    bool result = (m_ok && idrefraction  > 0);
    if (result)
    {
        ++ idrefraction;
        sets[CP_ID_REFRACTIONS] = idrefraction;
        result = DataBase::I()->InsertSQLByBinds(TBL_REFRACTIONS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cette refraction dans la base!"));
        return ref;
    }

    QJsonObject  data = QJsonObject{};
    data[CP_ID_REFRACTIONS] = idrefraction;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {

        champ  = itset.key();
        if (champ == CP_IDPAT_REFRACTIONS)                  data[champ] = itset.value().toInt();
        else if (champ == CP_IDACTE_REFRACTIONS)            data[champ] = itset.value().toInt();
        else if (champ == CP_DATE_REFRACTIONS)              data[champ] = itset.value().toString();
        else if (champ == CP_TYPEMESURE_REFRACTIONS)        data[champ] = itset.value().toString();
        else if (champ == CP_DISTANCEMESURE_REFRACTIONS)    data[champ] = itset.value().toString();
        else if (champ == CP_CYCLOPLEGIE_REFRACTIONS)       data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_ODMESURE_REFRACTIONS)          data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_SPHEREOD_REFRACTIONS)          data[champ] = itset.value().toDouble();
        else if (champ == CP_CYLINDREOD_REFRACTIONS)        data[champ] = itset.value().toDouble();
        else if (champ == CP_AXECYLOD_REFRACTIONS)          data[champ] = itset.value().toInt();
        else if (champ == CP_AVLOD_REFRACTIONS)             data[champ] = itset.value().toString();
        else if (champ == CP_ADDVPOD_REFRACTIONS)           data[champ] = itset.value().toDouble();
        else if (champ == CP_AVPOD_REFRACTIONS)             data[champ] = itset.value().toString();
        else if (champ == CP_PRISMEOD_REFRACTIONS)          data[champ] = itset.value().toDouble();
        else if (champ == CP_BASEPRISMEOD_REFRACTIONS)      data[champ] = itset.value().toInt();
        else if (champ == CP_BASEPRISMETEXTOD_REFRACTIONS)  data[champ] = itset.value().toString();
        else if (champ == CP_PRESSONOD_REFRACTIONS)         data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_DEPOLIOD_REFRACTIONS)          data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_PLANOD_REFRACTIONS)            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_RYSEROD_REFRACTIONS)           data[champ] = itset.value().toInt();
        else if (champ == CP_FORMULEOD_REFRACTIONS)         data[champ] = itset.value().toString();
        else if (champ == CP_OGMESURE_REFRACTIONS)          data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_SPHEREOG_REFRACTIONS)          data[champ] = itset.value().toDouble();
        else if (champ == CP_CYLINDREOG_REFRACTIONS)        data[champ] = itset.value().toDouble();
        else if (champ == CP_AXECYLOG_REFRACTIONS)          data[champ] = itset.value().toInt();
        else if (champ == CP_AVLOG_REFRACTIONS)             data[champ] = itset.value().toString();
        else if (champ == CP_ADDVPOG_REFRACTIONS)           data[champ] = itset.value().toDouble();
        else if (champ == CP_AVPOG_REFRACTIONS)             data[champ] = itset.value().toString();
        else if (champ == CP_PRISMEOG_REFRACTIONS)          data[champ] = itset.value().toDouble();
        else if (champ == CP_BASEPRISMEOG_REFRACTIONS)      data[champ] = itset.value().toInt();
        else if (champ == CP_BASEPRISMETEXTOG_REFRACTIONS)  data[champ] = itset.value().toString();
        else if (champ == CP_PRESSONOG_REFRACTIONS)         data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_DEPOLIOG_REFRACTIONS)          data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_PLANOG_REFRACTIONS)            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_RYSEROG_REFRACTIONS)           data[champ] = itset.value().toInt();
        else if (champ == CP_FORMULEOG_REFRACTIONS)         data[champ] = itset.value().toString();
        else if (champ == CP_COMMENTAIREORDO_REFRACTIONS)   data[champ] = itset.value().toString();
        else if (champ == CP_TYPEVERRES_REFRACTIONS)        data[champ] = itset.value().toString();
        else if (champ == CP_OEIL_REFRACTIONS)              data[champ] = itset.value().toString();
        else if (champ == CP_MONTURE_REFRACTIONS)           data[champ] = itset.value().toString();
        else if (champ == CP_VERRETEINTE_REFRACTIONS)       data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_PD_REFRACTIONS)                data[champ] = itset.value().toInt();
    }
    ref = new Refraction(data);
    if (ref != Q_NULLPTR)
    map_refractions->insert(ref->id(), ref);
    return ref;
}
