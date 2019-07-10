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


#include "cls_docexterne.h"
#include <QDebug>

DocExterne::DocExterne(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
    m_data = data;
}

bool DocExterne::isAllLoaded() const                { return m_isAllLoaded;}

int DocExterne::iduser() const                      { return m_iduser;}
int DocExterne::idpatient() const                   { return m_idpatient;}
QString DocExterne::typedoc() const                 { return m_typedoc;}
QString DocExterne::soustypedoc() const             { return m_soustypedoc;}
QString DocExterne::titre() const                   { return m_titre;}

QString DocExterne::textentete() const              { return m_textentete;}
QString DocExterne::textcorps() const               { return m_textcorps;}
QString DocExterne::textorigine() const             { return m_textorigine;}
QString DocExterne::textpied() const                { return m_textpied;}
QDateTime DocExterne::date() const                  { return m_dateimpression;}

int DocExterne::compression() const                 { return m_compression;}
QString DocExterne::lienversfichier() const         { return m_lienversfichier;}
bool DocExterne::isALD() const                      { return m_ald==1;}
int DocExterne::useremetteur() const                { return m_useremetteur;}
QString DocExterne::format() const                  { return m_formatdoc;}
QByteArray DocExterne::imageblob() const            { return m_blob;}
QString DocExterne::imageformat() const             { return m_formatimage;}

int DocExterne::importance() const                  { return m_importance;}

int DocExterne::idrefraction() const                { return m_idrefraction; }

void DocExterne::setDate(QDateTime date)            { m_dateimpression = date;}
void DocExterne::setimportance(int imptce)          { m_importance = imptce;}
void DocExterne::setAllLoaded(bool AllLoaded)       { m_isAllLoaded = AllLoaded;}
void DocExterne::setimageblob(QByteArray blob)      { m_blob = blob; }
void DocExterne::setimageformat(QString format)     { m_formatimage = format; }

void DocExterne::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataBool(data, CP_ISALLLOADED, m_isAllLoaded);

    setDataInt(data, CP_IDIMPRESSION_IMPRESSIONS, m_id);
    setDataInt(data, CP_IDUSER_IMPRESSIONS, m_iduser);
    setDataInt(data, CP_IDPAT_IMPRESSIONS, m_idpatient);
    setDataString(data, CP_TYPEDOC_IMPRESSIONS, m_typedoc);
    setDataString(data, CP_SOUSTYPEDOC_IMPRESSIONS, m_soustypedoc);

    setDataString(data, CP_TITRE_IMPRESSIONS, m_titre);
    setDataString(data, CP_TEXTENTETE_IMPRESSIONS, m_textentete);
    setDataString(data, CP_TEXTCORPS_IMPRESSIONS, m_textcorps);
    setDataString(data, CP_TEXTORIGINE_IMPRESSIONS, m_textorigine);
    setDataString(data, CP_TEXTPIED_IMPRESSIONS, m_textpied);

    setDataDateTime(data, CP_DATE_IMPRESSIONS, m_dateimpression);
    setDataInt(data, CP_COMPRESSION_IMPRESSIONS, m_compression);
    setDataString(data, CP_LIENFICHIER_IMPRESSIONS, m_lienversfichier);
    setDataInt(data, CP_ALD_IMPRESSIONS, m_ald);
    setDataInt(data, CP_IDEMETTEUR_IMPRESSIONS, m_useremetteur);

    setDataString(data, CP_FORMATDOC_IMPRESSIONS, m_formatdoc);
    setDataInt(data, CP_IMPORTANCE_IMPRESSIONS, m_importance);
    setDataInt(data, CP_EMISORRECU_IMPRESSIONS, m_emisrecu);
    setDataInt(data, CP_IDLIEU_IMPRESSIONS, m_idlieu);
    setDataInt(data, CP_IDREFRACTION_IMPRESSIONS, m_idrefraction);
}

