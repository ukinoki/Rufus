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
DocExterne* DocsExternes::getById(int id, bool loadDetails, bool addToList)
{
    QMap<int, DocExterne*>::const_iterator itdoc = m_docsexternes->find(id);
    DocExterne *result;
    if( itdoc == m_docsexternes->constEnd() )
            result = new DocExterne();
    else
    {
        result = itdoc.value();
        if (!loadDetails)
            return result;
        addToList = false;
    }

    if( loadDetails && !result->isAllLoaded() )
    {
        QJsonObject jsonDocExterne = DataBase::I()->loadDocExterneData(id);
        if( jsonDocExterne.isEmpty() )
            return Q_NULLPTR;
        else
            result->setData(jsonDocExterne);
    }
    if( addToList )
        add( result );
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

DocExterne* DocsExternes::reload(DocExterne* docmt)
{
    docmt->setAllLoaded(false);
    return getById(docmt->id());
}

bool DocsExternes::add(DocExterne *doc)
{
    if( doc == Q_NULLPTR)
        return false;
    if( m_docsexternes->contains(doc->id()) )
        return false;
    m_docsexternes->insert(doc->id(), doc);
    m_nouveaudocument = true;
    return true;
}

void DocsExternes::addList(QList<DocExterne*> listdocs)
{
    for(QList<DocExterne*>::const_iterator it = listdocs.constBegin(); it != listdocs.constEnd(); ++it )
    {
        DocExterne *doc = const_cast<DocExterne*>(*it);
        add(doc);
    }
}

void DocsExternes::clearAll()
{
    for( QMap<int, DocExterne*>::const_iterator itdoc = m_docsexternes->constBegin(); itdoc != m_docsexternes->constEnd(); ++itdoc)
        delete itdoc.value();
    m_docsexternes->clear();
}

void DocsExternes::remove(DocExterne *doc)
{
    if (doc == Q_NULLPTR)
        return;
    m_docsexternes->remove(doc->id());
    delete doc;
}
