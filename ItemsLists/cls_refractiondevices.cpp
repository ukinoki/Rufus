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

#include "cls_refractiondevices.h"

RefractionDevices::RefractionDevices(QObject *parent) : ItemsList(parent)
{
    map_RefractionDevices = new QMap<int, RefractionDevice*>();
}

QMap<int, RefractionDevice *> *RefractionDevices::refractiondevices() const
{
    return map_RefractionDevices;
}

/*!
 * \brief RefractionDevices::initListe
 * Charge l'ensemble des RefractionDevices
 * et les ajoute à la classe RefractionDevices
 */
void RefractionDevices::initListe()
{
    QList<RefractionDevice*> listRefractionDevices = DataBase::I()->loadrefractionDevices();
    epurelist(map_RefractionDevices, &listRefractionDevices);
    addList(map_RefractionDevices, &listRefractionDevices);
}

RefractionDevice* RefractionDevices::getById(int id)
{
    QMap<int, RefractionDevice*>::const_iterator itcpt = map_RefractionDevices->constFind(id);
    if( itcpt == map_RefractionDevices->constEnd() )
        return nullptr;
    return itcpt.value();
}

QStringList RefractionDevices::listRefractionDevices(RefractionDevice::Type devtype)
{
    QStringList listdevices = QStringList();
    for (auto it = map_RefractionDevices->cbegin(); it != map_RefractionDevices->cend(); ++it)
    {
        RefractionDevice *dev = it.value();
        if (devtype == RefractionDevice::AppNoneDev
            || (devtype == RefractionDevice::FrontoDev && dev->isFronto())
            || (devtype == RefractionDevice::AutorefDev && dev->isAutoref())
            || (devtype == RefractionDevice::RefracteurDev && dev->isRefracteur())
            || (devtype == RefractionDevice::TonometreDev && dev->isTono()))
            listdevices << dev->brand() + " " + dev->model();
    }
    return listdevices;
}



