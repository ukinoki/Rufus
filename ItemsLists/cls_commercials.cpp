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

#include "cls_commercials.h"

Commercials::Commercials(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, Commercial*>();
}

QMap<int, Commercial*>* Commercials::commercials() const
{
    return map_all;
}

Commercial* Commercials::getById(int id)
{
    QMap<int, Commercial*>::const_iterator itman = map_all->constFind(id);
    if( itman == map_all->constEnd() )
    {
        Commercial * man = DataBase::I()->loadCommercialById(id);
        if (man)
            add( map_all, man, Item::Update );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<Commercial*>(it.value()) : Q_NULLPTR);
    }
    return itman.value();
}

/*!
 * \brief Commercials::initListe
 * Charge l'ensemble des commerciaux
 * et les ajoute à la classe Commercials
 */
void Commercials::initListe()
{
    QList<Commercial*> listCommercials = DataBase::I()->loadCommercials();
    clearAll(map_all);
    foreach (Commercial *man, listCommercials)
            map_all->insert(man->id(), man);
}

/*!
 * \brief Commercials::initListe
 * Charge l'ensemble des commerciaux
 * et les ajoute à la classe Commercials
 */
void Commercials::initListebyidManufacturer(int idmanufacturer)
{
    QList<Commercial*> listCommercials = DataBase::I()->loadCommercialsByIdManufacturer(idmanufacturer);
    clearAll(map_all);
    foreach (Commercial *man, listCommercials)
            map_all->insert(man->id(), man);
}

void Commercials::SupprimeCommercial(Commercial* man)
{
    Supprime(map_all, man);
}

Commercial* Commercials::CreationCommercial(QHash<QString, QVariant> sets)
{
    Commercial *com = Q_NULLPTR;
    int idcommercial = 0;
    DataBase::I()->locktables(QStringList() << TBL_COMMERCIALS);
    idcommercial = DataBase::I()->selectMaxFromTable(CP_ID_COM, TBL_COMMERCIALS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idcommercial;
        sets[CP_ID_COM] = idcommercial;
        result = DataBase::I()->InsertSQLByBinds(TBL_COMMERCIALS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce fabricant dans la base!"));
        return com;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_COM] = idcommercial;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_NOM_COM)                            data[champ] = itset.value().toString();
        else if (champ == CP_TELEPHONE_COM)                 data[champ] = itset.value().toString();
        else if (champ == CP_MAIL_COM)                      data[champ] = itset.value().toString();
        else if (champ == CP_PRENOM_COM)                    data[champ] = itset.value().toString();
        else if (champ == CP_STATUT_COM)                    data[champ] = itset.value().toString();
        else if (champ == CP_MAIL_COM)                      data[champ] = itset.value().toString();
        else if (champ == CP_IDMANUFACTURER_COM)            data[champ] = itset.value().toInt();
    }
    com = new Commercial(data);
    if (com != Q_NULLPTR)
        map_all->insert(com->id(), com);
    return com;
}

