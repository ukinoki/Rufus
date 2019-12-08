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

#include "cls_impression.h"

Impression::Impression(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Impression::iduser() const                        { return m_iduser; }
QString Impression::texte() const                     { return m_texte; }
QString Impression::resume() const                    { return m_resume; }
QString Impression::conclusion() const                { return m_conclusion; }
bool Impression::ispublic() const                     { return m_public; }
bool Impression::isprescription() const               { return m_prescription; }
bool Impression::iseditable() const                   { return m_editable; }
bool Impression::ismedical() const                    { return m_medical; }

void Impression::settext(QString txt)                 { m_texte = txt; }
void Impression::setresume(QString resume)            { m_resume = resume; }
void Impression::setconclusion(QString conclusion)    { m_conclusion = conclusion; }
void Impression::setpublic(bool pblic)                { m_public = pblic; }
void Impression::setprescription(bool prescription)   { m_prescription = prescription; }
void Impression::seteditable(bool editable)           { m_editable = editable; }
void Impression::setmedical(bool medical)             { m_medical = medical; }

void Impression::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_ID_IMPRESSIONS, m_id);
    Utils::setDataInt(data, CP_IDUSER_IMPRESSIONS, m_iduser);

    Utils::setDataString(data, CP_TEXTE_IMPRESSIONS, m_texte);
    Utils::setDataString(data, CP_RESUME_IMPRESSIONS, m_resume);
    Utils::setDataString(data, CP_CONCLUSION_IMPRESSIONS, m_conclusion);

    Utils::setDataBool(data, CP_DOCPUBLIC_IMPRESSIONS, m_public);
    Utils::setDataBool(data, CP_PRESCRIPTION_IMPRESSIONS, m_prescription);
    Utils::setDataBool(data, CP_EDITABLE_IMPRESSIONS, m_editable);
    Utils::setDataBool(data, CP_MEDICAL_IMPRESSIONS, m_medical);
    m_data = data;
}


DossierImpression::DossierImpression(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int DossierImpression::id() const                { return m_dossierimpression; }
int DossierImpression::iduser() const            { return m_iduser; }
QString DossierImpression::texte() const         { return m_textedossier; }
QString DossierImpression::resume() const        { return m_resumedossier; }

bool DossierImpression::ispublic() const         { return m_public; }

void DossierImpression::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_ID_DOSSIERIMPRESSIONS, m_dossierimpression);
    Utils::setDataInt(data, CP_IDUSER_DOSSIERIMPRESSIONS, m_iduser);
    Utils::setDataString(data, CP_TEXTE_DOSSIERIMPRESSIONS, m_textedossier);
    Utils::setDataString(data, CP_RESUME_DOSSIERIMPRESSIONS, m_resumedossier);
    Utils::setDataBool(data, CP_PUBLIC_DOSSIERIMPRESSIONS, m_public);
}

