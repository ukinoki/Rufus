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

#include "cls_commentlunet.h"

CommentLunet::CommentLunet(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void CommentLunet::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_ID_COMLUN, m_id);
    Utils::setDataInt(data, CP_IDUSER_COMLUN, m_iduser);
    Utils::setDataString(data, CP_TEXT_COMLUN, m_text);
    Utils::setDataString(data, CP_RESUME_COMLUN, m_resume);
    Utils::setDataBool(data, CP_PARDEFAUT_COMLUN, m_defaut);
    Utils::setDataBool(data, CP_PUBLIC_COMLUN, m_public);
    m_data = data;
}

/*! comme son nom l'indique */
void CommentLunet::resetdatas()
{
    QJsonObject data;
    data[CP_ID_COMLUN]          = 0;
    data[CP_TEXT_COMLUN]        = "";
    data[CP_IDUSER_COMLUN]      = 0;
    data[CP_RESUME_COMLUN]      = "";
    data[CP_PUBLIC_COMLUN]      = false;
    data[CP_PARDEFAUT_COMLUN]   = false;
    setData(data);
}

QString CommentLunet::tooltip() const
{
    QString ttip = m_text;
    ttip.replace(QRegularExpression("\n\n[\n]*"),"\n");
    if (ttip.size()>300)
    {
        ttip = ttip.left(300);
        if (ttip.endsWith("\n"))
            ttip = ttip.left(ttip.size()-1);
        else if (!ttip.endsWith("."))
        {
            if (ttip.endsWith(" "))
                ttip = ttip.left(ttip.size()-1) + "...";
            else
            {
                int a = ttip.lastIndexOf(" ");
                int b = ttip.lastIndexOf(".");
                int c = b;
                if (a>b) c = a;
                ttip = ttip.left(c) + "...";
            }
        }
    }
    int cassure = 50;
    QStringList listhash1;
    listhash1 = ttip.split("\n");
    QString ResumeItem = "";
    for (int i=0; i<listhash1.size(); i++)
    {
        QString ttipc = listhash1.at(i);
        if (listhash1.at(i).size() > cassure)
        {
            QStringList listhash;
            listhash = listhash1.at(i).split(" ");
            ttipc = listhash.at(0);
            int count = ttipc.size() + 1;
            for (int j=1; j<listhash.size(); j++)
            {
                count += listhash.at(j).size();
                if (count>cassure)
                {
                    ttipc += "\n";
                    count = listhash.at(j).size() + 1;
                }
                else
                {
                    ttipc += " ";
                    count += 1;
                }
                ttipc += listhash.at(j);
            }
        }
        ResumeItem += ttipc;
        if (i <(listhash1.size()-1)) ResumeItem += "\n";
    }
    return ResumeItem;
}

