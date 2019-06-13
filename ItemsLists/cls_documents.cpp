#include "cls_documents.h"


Documents::Documents()
{
    m_documents = new QMap<int, Document*>();
}

QMap<int, Document *> *Documents::documents() const
{
    return m_documents;
}

void Documents::add(Document *Document)
{
    if( m_documents->contains(Document->id()) )
        return;
    m_documents->insert(Document->id(), Document);
}

void Documents::addList(QList<Document*> listDocuments)
{
    QList<Document*>::const_iterator it;
    for( it = listDocuments.constBegin(); it != listDocuments.constEnd(); ++it )
        add( *it );
}

void Documents::clearAll()
{
    for( QMap<int, Document*>::const_iterator itdoc = m_documents->constBegin(); itdoc != m_documents->constEnd(); ++itdoc)
        delete itdoc.value();
    m_documents->clear();
}

void Documents::remove(Document *doc)
{
    if (doc == Q_NULLPTR)
        return;
    m_documents->remove(doc->id());
    delete doc;
}

Document* Documents::getById(int id)
{
    QMap<int, Document*>::const_iterator itdoc = m_documents->find(id);
    if( itdoc == m_documents->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
}

/*!
 * \brief Documentss::initListeDocument
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe Documents
 */
void Documents::initListe()
{
    clearAll();
    QList<Document*> listdocs = DataBase::I()->loadDocuments();
    QList<Document*>::const_iterator itdoc;
    for( itdoc = listdocs.constBegin(); itdoc != listdocs.constEnd(); ++itdoc )
    {
        Document *doc = const_cast<Document*>(*itdoc);
        add(doc);
    }
}


MetaDocuments::MetaDocuments()
{
    m_metadocuments = new QMap<int, MetaDocument*>();
}

QMap<int, MetaDocument *> *MetaDocuments::metadocuments() const
{
    return m_metadocuments;
}

void MetaDocuments::add(MetaDocument *MetaDoc)
{
    if( m_metadocuments->contains(MetaDoc->id()) )
        return;
    m_metadocuments->insert(MetaDoc->id(), MetaDoc);
}

void MetaDocuments::addList(QList<MetaDocument*> listMetaDocs)
{
    QList<MetaDocument*>::const_iterator it;
    for( it = listMetaDocs.constBegin(); it != listMetaDocs.constEnd(); ++it )
        add( *it );
}

void MetaDocuments::clearAll()
{
    QList<MetaDocument*> listdocs;
    for( QMap<int, MetaDocument*>::const_iterator itdoc = m_metadocuments->constBegin(); itdoc != m_metadocuments->constEnd(); ++itdoc)
        delete itdoc.value();
    m_metadocuments->clear();
}

void MetaDocuments::remove(MetaDocument *metadoc)
{
    QMap<int, MetaDocument*>::const_iterator itdoc = m_metadocuments->find(metadoc->id());
    if( itdoc == m_metadocuments->constEnd() )
        return;
    m_metadocuments->remove(metadoc->id());
    delete metadoc;
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
    clearAll();
    QList<MetaDocument*> listmetadocs = DataBase::I()->loadMetaDocuments();
    QList<MetaDocument*>::const_iterator itmetadoc;
    for( itmetadoc = listmetadocs.constBegin(); itmetadoc != listmetadocs.constEnd(); ++itmetadoc )
    {
        MetaDocument *metadoc = const_cast<MetaDocument*>(*itmetadoc);
        add(metadoc);
    }
}

