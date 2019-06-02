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

#include "cls_docsexternes.h"

DocsExternes::DocsExternes(QObject *parent) : ItemsList(parent)
{
    m_docsexternes = new QMap<int, DocExterne*>();
}

QMap<int, DocExterne *> *DocsExternes::docsexternes()
{
    return m_docsexternes;
}

/*!
 * \brief docsexternes::getById
 * \param id l'idimpr du Document recherché
 * \return Q_NULLPTR si aucun Document trouvée
 * \return DocExterne* le Document correspondant à l'id
 */
DocExterne* DocsExternes::getById(int id, Item::LOADDETAILS loadDetails, ADDTOLIST addToList)
{
    QMap<int, DocExterne*>::const_iterator itdoc = m_docsexternes->find(id);
    DocExterne *result;
    if( itdoc == m_docsexternes->constEnd() )
            result = new DocExterne();
    else
    {
        result = itdoc.value();
        if (loadDetails == Item::NoLoadDetails)
            return result;
        addToList = ItemsList::NoAddToList;
    }

    if( loadDetails == Item::LoadDetails && !result->isAllLoaded() )
    {
        QJsonObject jsonDocExterne = DataBase::I()->loadDocExterneData(id);
        if( jsonDocExterne.isEmpty() )
        {
            delete result;
            return Q_NULLPTR;
        }
        else
            result->setData(jsonDocExterne);
    }
    if( addToList == ItemsList::AddToList)
        add( m_docsexternes, result->id(), result );
    return result;
}

bool DocsExternes::NouveauDocument()
{
    return m_nouveaudocument;
}

void DocsExternes::setNouveauDocumentFalse()
{
    m_nouveaudocument = false;
}

void DocsExternes::setsoustype(DocExterne* docmt, QString soustype)
{
    docmt->setsoustype(soustype);
    if (soustype == "")
        soustype = "null";
    else
        soustype = "'" + Utils::correctquoteSQL(soustype) + "'";
    DataBase::I()->StandardSQL("update " TBL_IMPRESSIONS " set soustypedoc = " + soustype + " where idimpression = " + QString::number(docmt->id()));
}

void DocsExternes::addList(QList<DocExterne*> listdocs)
{
    for(QList<DocExterne*>::const_iterator it = listdocs.constBegin(); it != listdocs.constEnd(); ++it )
    {
        DocExterne *doc = const_cast<DocExterne*>(*it);
        add(m_docsexternes, doc->id(), doc);
    }
}

void DocsExternes::remove(DocExterne *doc)
{
    if (doc == Q_NULLPTR)
        return;
    m_docsexternes->remove(doc->id());
    delete doc;
}

/*!
 * \brief DocsExternes::initListeByPatient
 * Charge l'ensemble des documents externes pour un patient
 * et les ajoute à la classe Patients
 */
void DocsExternes::initListeByPatient(Patient *pat)
{
    clearAll(m_docsexternes);
    addList(DataBase::I()->loadDoscExternesByPatient(pat));
}


bool DocsExternes::SupprimeDocExterne(DocExterne* doc)
{
    DataBase::I()->StandardSQL("delete from " TBL_REFRACTION " where idrefraction = (select idrefraction from " TBL_IMPRESSIONS
                    " where idimpression = " + QString::number(doc->id()) + ")");
    DataBase::I()->StandardSQL("delete from " TBL_IMPRESSIONS " where idimpression = " + QString::number(doc->id()));
    DataBase::I()->StandardSQL("delete from " TBL_ECHANGEIMAGES " where idimpression = " + QString::number(doc->id()));
    remove(doc);
    return true;
}


