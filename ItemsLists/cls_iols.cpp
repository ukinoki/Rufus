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

#include "cls_iols.h"

IOLs::IOLs(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, IOL*>();
}

QMap<int, IOL*>* IOLs::iols() const
{
    return map_all;
}

IOL* IOLs::getById(int id, bool reload)
{
    QMap<int, IOL*>::const_iterator itref = map_all->find(id);
    if( itref == map_all->constEnd() )
    {
        IOL* iol = DataBase::I()->loadIOLById(id);
        if (iol != Q_NULLPTR)
            add( map_all, iol, Item::Update );
        return iol;
    }
    else if (reload)
    {
        IOL* iol = DataBase::I()->loadIOLById(id);
        if (iol)
        {
            itref.value()->setData(iol->datas());
            delete iol;
        }
    }
    return itref.value();
}

/*!
 * \brief IOLs::initListe
 * Charge l'ensemble des IOLs
 * et les ajoute à la classe IOLS
 */
void IOLs::initListe()
{
    QList<IOL*> listIOLs = DataBase::I()->loadIOLs();
    clearAll(map_all);
    addList(map_all, &listIOLs);
}

void IOLs::SupprimeIOL(IOL *iol)
{
    Supprime(map_all, iol);
}

IOL* IOLs::CreationIOL(QHash<QString, QVariant> sets)
{
    IOL *iol = Q_NULLPTR;
    int idiol = 0;
    DataBase::I()->locktables(QStringList() << TBL_IOLS);
    idiol = DataBase::I()->selectMaxFromTable(CP_ID_IOLS, TBL_IOLS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idiol;
        sets[CP_ID_IOLS] = idiol;
        result = DataBase::I()->InsertSQLByBinds(TBL_IOLS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cet implant dans la base!"));
        return iol;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_IOLS] = idiol;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDMANUFACTURER_IOLS)                        data[champ] = itset.value().toInt();
        else if (champ == CP_MODELNAME_IOLS)                        data[champ] = itset.value().toString();
        else if (champ == CP_INACTIF_IOLS)                          data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_ACD_IOLS)                              data[champ] = itset.value().toDouble();
        else if (champ == CP_MINPWR_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_MAXPWR_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_PWRSTEP_IOLS)                          data[champ] = itset.value().toDouble();
        else if (champ == CP_MINCYL_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_MAXCYL_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_CYLSTEP_IOLS)                          data[champ] = itset.value().toDouble();
        else if (champ == CP_CSTEAOPT_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_CSTEAECHO_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA0_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA1_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA2_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_HOLL1_IOLS)                            data[champ] = itset.value().toDouble();
        else if (champ == CP_DIAOPT_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_DIAALL_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_DIAINJECTEUR_IOLS)                     data[champ] = itset.value().toDouble();
        else if (champ == CP_TYPIMG_IOLS)                           data[champ] = itset.value().toString();
        else if (champ == CP_REMARQUE_IOLS)                         data[champ] = itset.value().toString();
        else if (champ == CP_MATERIAU_IOLS)                         data[champ] = itset.value().toString();
        else if (champ == CP_TYP_IOLS)                              data[champ] = itset.value().toString();
        else if (champ == CP_PRECHARGE_IOLS)                        data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_JAUNE_IOLS)                            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_MULTIFOCAL_IOLS)                       data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_EDOF_IOLS)                             data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_JAUNE_IOLS)                            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_ARRAYIMG_IOLS)                         data[champ] = QLatin1String(itset.value().toByteArray().toBase64());
     }
    iol = new IOL(data);
    if (iol != Q_NULLPTR)
        map_all->insert(iol->id(), iol);
    return iol;
}

