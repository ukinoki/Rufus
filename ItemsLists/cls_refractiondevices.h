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

#ifndef CLS_REFRACTIONDEVICES_H
#define CLS_REFRACTIONDEVICES_H

#include "cls_refractiondevice.h"
#include "cls_itemslist.h"

class RefractionDevices : public ItemsList
{
    Q_OBJECT
private:
    QMap<int, RefractionDevice*> *map_RefractionDevices = nullptr; //!< la liste des RefractionDevices

public:
    explicit                        RefractionDevices(QObject *parent = nullptr);
    QMap<int, RefractionDevice *>*  refractiondevices() const;

    RefractionDevice*               getById(int id);
    void                            initListe();

    QStringList                     listRefractionDevices(RefractionDevice::Type devtype = RefractionDevice::AppNoneDev);
};

#endif // CLS_REFRACTIONDEVICES_H
