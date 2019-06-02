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

#include "cls_document.h"
#include <QDebug>

Document::Document(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Document::iduser() const                        { return m_iduser; }
QString Document::texte() const                     { return m_texte; }
QString Document::resume() const                    { return m_resume; }
QString Document::conclusion() const                { return m_conclusion; }
bool Document::ispublic() const                     { return m_public; }
bool Document::isprescription() const               { return m_prescription; }
bool Document::iseditable() const                   { return m_editable; }
bool Document::ismedical() const                    { return m_medical; }

void Document::settext(QString txt)                 { m_texte = txt; }
void Document::setresume(QString resume)            { m_resume = resume; }
void Document::setconclusion(QString conclusion)    { m_conclusion = conclusion; }
void Document::setpublic(bool pblic)                { m_public = pblic; }
void Document::setprescription(bool prescription)   { m_prescription = prescription; }
void Document::seteditable(bool editable)           { m_editable = editable; }
void Document::setmedical(bool medical)             { m_medical = medical; }

void Document::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "iddocument", m_id);
    setDataInt(data, "iduser", m_iduser);

    setDataString(data, "texte", m_texte);
    setDataString(data, "resume", m_resume);
    setDataString(data, "conclusion", m_conclusion);

    setDataBool(data, "public", m_public);
    setDataBool(data, "prescription", m_prescription);
    setDataBool(data, "editable", m_editable);
    setDataBool(data, "medical", m_medical);
}


MetaDocument::MetaDocument(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int MetaDocument::id() const                { return m_idmetadocument; }
int MetaDocument::iduser() const            { return m_iduser; }
QString MetaDocument::texte() const         { return m_textemeta; }
QString MetaDocument::resume() const        { return m_resumemeta; }

bool MetaDocument::ispublic() const         { return m_public; }

void MetaDocument::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "idmetadocument", m_idmetadocument);
    setDataInt(data, "iduser", m_iduser);
    setDataString(data, "texte", m_textemeta);
    setDataString(data, "resume", m_resumemeta);
    setDataBool(data, "public", m_public);
}

