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

#include "cls_tierspayants.h"

TiersPayants::TiersPayants(QObject *parent) : ItemsList(parent)
{
    map_tierspayants = new QMap<int, Tiers *>();
}

QMap<int, Tiers *> *TiersPayants::tierspayants() const
{
    return map_tierspayants;
}

Tiers* TiersPayants::getById(int id)
{
    QMap<int, Tiers*>::const_iterator itcpt = map_tierspayants->find(id);
    if( itcpt == map_tierspayants->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

/*!
 * \brief TiersPayants::initListe
 * Charge l'ensemble des tiers payants
 * et les ajoute à la classe Tiers
 */
void TiersPayants::initListe()
{
    QList<Tiers*> listtiers = DataBase::I()->loadTiersPayants();
    epurelist(map_tierspayants, &listtiers);
    addList(map_tierspayants, &listtiers);
}

bool TiersPayants::isUtilise(QString nom) // si un tiers n'est plus utilisé depuis plus d'un an, on peut le supprimer
{
    bool ok;

    QString req = "select " CP_ID_LIGNRECETTES " from " TBL_RECETTES " where " CP_NOMPAYEUR_LIGNRECETTES " = '" + Utils::correctquoteSQL(nom) + "' AND " CP_DATE_LIGNRECETTES " > AddDate(NOW(),-365)";
    //qDebug() << req;
    return DataBase::I()->StandardSelectSQL(req, ok).size() >0;
}

void TiersPayants::SupprimeTiers(Tiers *tiers)
{
    Supprime(map_tierspayants, tiers);
}

Tiers* TiersPayants::CreationTiers(QHash<QString, QVariant> sets)
{
    Tiers *tiers = Q_NULLPTR;
    int idTiers = 0;
    DataBase::I()->locktables(QStringList() << TBL_TIERS);
    idTiers = DataBase::I()->selectMaxFromTable(CP_ID_TIERS, TBL_TIERS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idTiers;
        sets[CP_ID_TIERS] = idTiers;
        result = DataBase::I()->InsertSQLByBinds(TBL_TIERS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce tiers payant dans la base!"));
        return tiers;
    }
    QJsonObject  data = QJsonObject{};
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        data[champ] = itset.value().toString();
    }
    data[CP_ID_TIERS] = idTiers;
    tiers = new Tiers(data);
    if (tiers != Q_NULLPTR)
        initListe();
    return tiers;
}

