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

#include "cls_refractiondevice.h"

RefractionDevice::RefractionDevice(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void RefractionDevice::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, CP_ID_REF, m_id);
    setDataString(data, CP_BRAND_REF, m_brand);
    setDataString(data, CP_MODEL_REF, m_model);
    QString s = data[CP_TYPE_REF].toString();
    if (s == FRONTO)
        m_type = FrontoDev;
    else if (s == AUTOREF)
        m_type = AutorefDev;
    else if (s == REFRACTEUR)
        m_type = RefracteurDev;
    else if (s == TONOMETRE)
        m_type = TonometreDev;
    setDataInt(data, CP_LIKEDEVICE_REF, m_likedevice);
    setDataBool(data, CP_SERIAL_REF, m_hasCOM);
    setDataBool(data, CP_LAN_REF, m_hasLAN);
    setDataBool(data, CP_IMPLEMENTE_REF, m_isrecognized);
    setDataString(data, CP_COMMENT_REF, m_comment);
    m_data = data;
}



QString RefractionDevice::model() const
{
    return m_model;
}

void RefractionDevice::setModel(const QString &newModel)
{
    m_model = newModel;
}

QString RefractionDevice::comment() const
{
    return m_comment;
}

void RefractionDevice::setComment(const QString &newComment)
{
    m_comment = newComment;
}

bool RefractionDevice::hasCOM() const
{
    return m_hasCOM;
}

void RefractionDevice::setHasCOM(bool newHasCOM)
{
    m_hasCOM = newHasCOM;
}

bool RefractionDevice::hasLAN() const
{
    return m_hasLAN;
}

void RefractionDevice::setHasLAN(bool newHasLAN)
{
    m_hasLAN = newHasLAN;
}

bool RefractionDevice::isrecognized() const
{
    return m_isrecognized;
}

void RefractionDevice::setIsrecognized(bool newIsrecognized)
{
    m_isrecognized = newIsrecognized;
}

int RefractionDevice::likedevice() const
{
    return m_likedevice;
}

void RefractionDevice::setLikedevice(int newLikedevice)
{
    m_likedevice = newLikedevice;
}

RefractionDevice::Type RefractionDevice::type() const
{
    return m_type;
}

void RefractionDevice::setType(Type newType)
{
    m_type = newType;
}

QString RefractionDevice::brand() const
{
    return m_brand;
}

void RefractionDevice::setBrand(const QString &newBrand)
{
    m_brand = newBrand;
}
