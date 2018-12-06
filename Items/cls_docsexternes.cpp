/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cls_docsexternes.h"

DocsExternes::DocsExternes()
{
}

QMap<int, DocExterne *> DocsExternes::docsexternes()
{
    return m_docsexternes;
}

/*!
 * \brief docsexternes::getDocExterneById
 * \param id l'idimpr du Document recherché
 * \return nullptr si aucun Document trouvée
 * \return DocExterne* le Document correspondant à l'id
 */
DocExterne* DocsExternes::getDocumentById(int id, bool loadDetails, bool addToList)
{
    QMap<int, DocExterne*>::const_iterator itdoc = m_docsexternes.find(id);
    DocExterne *result;
    if( itdoc == m_docsexternes.constEnd() )
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
        QJsonObject jsonDocExterne = DataBase::getInstance()->loadDocExterneData(id);
        if( jsonDocExterne.isEmpty() )
            return Q_NULLPTR;
        else
            result->setData(jsonDocExterne);
    }
    if( addToList )
        addDocExterne( result );
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

DocExterne* DocsExternes::reloadDocument(DocExterne* docmt)
{
    docmt->setAllLoaded(false);
    return getDocumentById(docmt->id());
}

void DocsExternes::addDocExterne(DocExterne *docext)
{
    if( m_docsexternes.contains(docext->id()) )
        return;
    m_docsexternes.insert(docext->id(), docext);
    m_nouveaudocument = true;
}

void DocsExternes::addListDocsExternes(QList<DocExterne*> listdocs)
{
    for(QList<DocExterne*>::const_iterator it = listdocs.constBegin(); it != listdocs.constEnd(); ++it )
    {
        DocExterne *doc = const_cast<DocExterne*>(*it);
        addDocExterne(doc);
    }
}

void DocsExternes::VideLaListe()
{
    m_docsexternes.clear();
}

void DocsExternes::RemoveKey(int key)
{
    m_docsexternes.remove(key);
}
