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

#include "cls_iol.h"

IOL::IOL(QJsonObject data, QObject *parent) : Item(parent)
{
    resetdatas();
    setData(data);
}

void IOL::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
//    qDebug() << "map[CP_MODELNAME_IOLS]" << map[CP_MODELNAME_IOLS];
//    qDebug() << "map[CP_ACD_IOLS]" << map[CP_ACD_IOLS];
//    qDebug() << "map[CP_IDMANUFACTURER_IOLS]" << map[CP_IDMANUFACTURER_IOLS];
//    qDebug() << "map[CP_INACTIF_IOLS]" << map[CP_INACTIF_IOLS];
    setDataInt(data, CP_ID_IOLS, m_id);
    setDataInt(data, CP_IDMANUFACTURER_IOLS, m_idmanufacturer);
    setDataString(data, CP_MODELNAME_IOLS, m_modele);
    setDataBool(data, CP_INACTIF_IOLS, m_inactif);
    setDataDouble(data, CP_ACD_IOLS, m_acd);
    setDataDouble(data, CP_MINPWR_IOLS, m_pwrmin);
    setDataDouble(data, CP_MAXPWR_IOLS, m_pwrmax);
    setDataDouble(data, CP_PWRSTEP_IOLS, m_pwrstp);
    setDataDouble(data, CP_MINCYL_IOLS, m_cylmin);
    setDataDouble(data, CP_MAXCYL_IOLS, m_cylmax);
    setDataDouble(data, CP_CYLSTEP_IOLS, m_cylstp);
    setDataDouble(data, CP_CSTEAOPT_IOLS, m_csteAopt);
    setDataDouble(data, CP_CSTEAECHO_IOLS, m_csteAEcho);
    setDataDouble(data, CP_HAIGISA0_IOLS, m_haigisa0);
    setDataDouble(data, CP_HAIGISA1_IOLS, m_haigisa1);
    setDataDouble(data, CP_HAIGISA2_IOLS, m_haigisa2);
    setDataDouble(data, CP_HOLL1_IOLS, m_holladay);
    setDataDouble(data, CP_DIAINJECTEUR_IOLS, m_diainjecteur);
    setDataDouble(data, CP_DIAALL_IOLS, m_diaall);
    setDataDouble(data, CP_DIAOPT_IOLS, m_diaoptique);
    setDataByteArray(data, CP_ARRAYIMG_IOLS, m_arrayimgiol);
    setDataString(data, CP_TYPIMG_IOLS, m_imageformat);
    setDataString(data, CP_MATERIAU_IOLS, m_materiau);
    setDataString(data, CP_REMARQUE_IOLS, m_remarque);
    setDataBool(data, CP_PRECHARGE_IOLS, m_precharge);
    setDataBool(data, CP_MULTIFOCAL_IOLS, m_multifocal);
    setDataBool(data, CP_JAUNE_IOLS, m_jaune);
    setDataBool(data, CP_EDOF_IOLS, m_edof);
    setDataBool(data, CP_TORIC_IOLS, m_toric);
    switch (data[CP_TYP_IOLS].toInt()) {
    case 1: m_type = IOL_CP; break;
    case 2: m_type = IOL_CA; break;
    case 3: m_type = IOL_ADDON; break;
    case 4: m_type = IOL_IRIEN; break;
    case 5: m_type = IOL_CAREFRACTIF; break;
    case 6: m_type = IOL_AUTRE; break;
    default: m_type = "";
    }
    QImage img = m_nullimage;
    if (m_arrayimgiol.size())
    {
        if (m_imageformat == PDF)
        {
            QList<QImage> listimg = Utils::calcImagefromPdf(m_arrayimgiol);
            if (listimg.size() > 0)
                if (listimg.at(0) != QImage())
                    img = listimg.at(0);
        }
        else if (!img.loadFromData(m_arrayimgiol))
            img = m_nullimage;
    }
    m_currentimage = img;
    m_data = data;
}

QString IOL::tooltip(bool avecimage) const
{
    QString message = modele();
    if (m_type != "")
        message += "<br>" + m_type;
    if (m_toric)
        message += "<br>" + tr("Torique");
    if (m_edof)
        message += "<br>" + tr("EDOF");
    else if (m_multifocal)
        message += "<br>" + tr("Multifocal");
    else
        message += "<br>" + tr("Monofocal");
    if (m_precharge)
        message += "<br>" + tr("Préchargé");
    if (m_jaune)
        message += "<br>" + tr("Jaune");
    if (m_diaall != 0.0)
        message += "<br>" + tr("diamètre hors tout") + " " + QString::number(m_diaall, 'f', 1) + " mm";
    if (m_diainjecteur != 0.0)
        message += "<br>" + tr("incision") + " " + QString::number(m_diainjecteur, 'f', 1) + " mm";
    if (m_csteAEcho != 0.0)
        message += "<br>" + tr("csteA echo") + " " + QString::number(m_csteAEcho, 'f', 1);
    if (m_materiau != "")
        message += "<br>" + m_materiau;
    if (m_remarque != "")
        message += "<br>" + remarque().replace("\n","<br>");
    if (!avecimage)
        return message;
    /*! si on veut intégrer l'image de l'IOL dans le tooltip */
    if (m_currentimage == m_nullimage)
        return message;
    if(m_arrayimgiol == QByteArray())
        return message;
    int scale = 90;
    QString ttip = "";
    QImage image = m_currentimage.scaled(scale,scale, Qt::KeepAspectRatio);
    QByteArray data;
    QBuffer buffer(&data);
    image.save(&buffer, "PNG", 100);
    ttip = QString("<img src='data:image/png;base64, %0'><br>" + message).arg(QString(data.toBase64()));
    return ttip;
}

void IOL::resetdatas()
{
    QJsonObject data;
    data[CP_ID_IOLS]                 = 0;
    data[CP_IDMANUFACTURER_IOLS]     = 0;
    data[CP_MODELNAME_IOLS]          = "";
    data[CP_INACTIF_IOLS]            = false;
    data[CP_ACD_IOLS]                = 0;
    data[CP_MINPWR_IOLS]             = 0;
    data[CP_MAXPWR_IOLS]             = 0;
    data[CP_PWRSTEP_IOLS]            = 0;
    data[CP_MINCYL_IOLS]             = 0;
    data[CP_MAXCYL_IOLS]             = 0;
    data[CP_CYLSTEP_IOLS]            = 0;
    data[CP_CSTEAOPT_IOLS]           = 0;
    data[CP_CSTEAECHO_IOLS]          = 0;
    data[CP_HAIGISA0_IOLS]           = 0;
    data[CP_HAIGISA1_IOLS]           = 0;
    data[CP_HAIGISA2_IOLS]           = 0;
    data[CP_HOLL1_IOLS]              = 0;
    data[CP_DIAINJECTEUR_IOLS]       = 0;
    data[CP_DIAALL_IOLS]             = 0;
    data[CP_DIAOPT_IOLS]             = 0;
    data[CP_ARRAYIMG_IOLS]           = QLatin1String(QVariant().toByteArray().toBase64());
    data[CP_TYPIMG_IOLS]             = "";
    data[CP_MATERIAU_IOLS]           = "";
    data[CP_REMARQUE_IOLS]           = "";
    data[CP_PRECHARGE_IOLS]          = false;
    data[CP_JAUNE_IOLS]              = false;
    data[CP_MULTIFOCAL_IOLS]         = false;
    data[CP_EDOF_IOLS]               = false;
    data[CP_TORIC_IOLS]              = false;
    data[CP_TYP_IOLS]                = 0;
    setData(data);
}

