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

#include "cls_manufacturers.h"

Manufacturers::Manufacturers(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, Manufacturer*>();
}

QMap<int, Manufacturer*>* Manufacturers::manufacturers() const
{
    return map_all;
}

Manufacturer* Manufacturers::getById(int id, bool reload)
{
    QMap<int, Manufacturer*>::const_iterator itman = map_all->constFind(id);
    if( itman == map_all->constEnd() )
    {
        Manufacturer * man = DataBase::I()->loadManufacturerById(id);
        if (man != Q_NULLPTR)
            add( map_all, man, Item::Update );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<Manufacturer*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        Manufacturer* man = DataBase::I()->loadManufacturerById(id);
        if (man)
        {
            itman.value()->setData(man->datas());
            delete man;
        }
    }
    return itman.value();
}

/*!
 * \brief Manufacturers::initListe
 * Charge l'ensemble des fabricants
 * et les ajoute à la classe Manufacturers
 */
void Manufacturers::initListe()
{
    QList<Manufacturer*> listManufacturers = DataBase::I()->loadManufacturers();
    clearAll(map_all);
    foreach (Manufacturer *man, listManufacturers)
        if( man )
            map_all->insert(man->id(), man);
}

void Manufacturers::SupprimeManufacturer(Manufacturer* man)
{
    Supprime(map_all, man);
}

Manufacturer* Manufacturers::CreationManufacturer(QHash<QString, QVariant> sets)
{
    Manufacturer *man = Q_NULLPTR;
    int idManufacturer = 0;
    DataBase::I()->locktables(QStringList() << TBL_MANUFACTURERS);
    idManufacturer = DataBase::I()->selectMaxFromTable(CP_ID_MANUFACTURER, TBL_MANUFACTURERS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idManufacturer;
        sets[CP_ID_MANUFACTURER] = idManufacturer;
        result = DataBase::I()->InsertSQLByBinds(TBL_MANUFACTURERS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce fabricant dans la base!"));
        return man;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_MANUFACTURER] = idManufacturer;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_NOM_MANUFACTURER)                           data[champ] = itset.value().toString();
        else if (champ == CP_ADRESSE1_MANUFACTURER)                 data[champ] = itset.value().toString();
        else if (champ == CP_ADRESSE2_MANUFACTURER)                 data[champ] = itset.value().toString();
        else if (champ == CP_ADRESSE3_MANUFACTURER)                 data[champ] = itset.value().toString();
        else if (champ == CP_CODEPOSTAL_MANUFACTURER)               data[champ] = itset.value().toString();
        else if (champ == CP_VILLE_MANUFACTURER)                    data[champ] = itset.value().toString();
        else if (champ == CP_TELEPHONE_MANUFACTURER)                data[champ] = itset.value().toString();
        else if (champ == CP_FAX_MANUFACTURER)                      data[champ] = itset.value().toString();
        else if (champ == CP_PORTABLE_MANUFACTURER)                 data[champ] = itset.value().toString();
        else if (champ == CP_WEBSITE_MANUFACTURER)                  data[champ] = itset.value().toString();
        else if (champ == CP_MAIL_MANUFACTURER)                     data[champ] = itset.value().toString();
        else if (champ == CP_INACTIF_MANUFACTURER)                  data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_DISTRIBUEPAR_MANUFACTURER)             data[champ] = itset.value().toInt();
        else if (champ == CP_IDRUFUS_MANUFACTURER)                  data[champ] = itset.value().toInt();
    }
    man = new Manufacturer(data);
    if (man != Q_NULLPTR)
        map_all->insert(man->id(), man);
    return man;
}

