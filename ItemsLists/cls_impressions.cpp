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

#include "cls_documents.h"


Documents::Documents(QObject *parent) : ItemsList(parent)
{
    m_documents = new QMap<int, Document*>();
}

QMap<int, Document *> *Documents::documents() const
{
    return m_documents;
}

Document* Documents::getById(int id)
{
    QMap<int, Document*>::const_iterator itdoc = m_documents->find(id);
    if( itdoc == m_documents->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
}

/*!
 * \brief Documents::initListe
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe Documents
 */
void Documents::initListe()
{
    clearAll(m_documents);
    addList(m_documents, DataBase::I()->loadDocuments());
}


MetaDocuments::MetaDocuments()
{
    m_metadocuments = new QMap<int, MetaDocument*>();
}

QMap<int, MetaDocument *> *MetaDocuments::metadocuments() const
{
    return m_metadocuments;
}

MetaDocument* MetaDocuments::getById(int id)
{
    QMap<int, MetaDocument*>::const_iterator itdoc = m_metadocuments->find(id);
    if( itdoc == m_metadocuments->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
}

/*!
 * \brief MetaDocumentss::initListeDocument
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe Documents
 */
void MetaDocuments::initListe()
{
    clearAll(m_metadocuments);
    addList(m_metadocuments, DataBase::I()->loadMetaDocuments());
}

