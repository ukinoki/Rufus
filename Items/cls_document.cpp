#include "cls_document.h"
#include <QDebug>

Document::Document(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Document::id() const                { return m_iddocument; }
int Document::iduser() const            { return m_iduser; }
QString Document::texte() const         { return m_texte; }
QString Document::resume() const        { return m_resume; }
QString Document::conclusion() const    { return m_conclusion; }

bool Document::ispublic() const         { return m_public; }
bool Document::isprescription() const   { return m_prescription; }
bool Document::iseditable() const       { return m_editable; }
bool Document::ismedical() const        { return m_medical;}

void Document::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "iddocument", m_iddocument);
    setDataInt(data, "iduser", m_iduser);

    setDataString(data, "texte", m_texte);
    setDataString(data, "resume", m_resume);
    setDataString(data, "conclusion", m_conclusion);

    setDataBool(data, "public", m_public);
    setDataBool(data, "prescription", m_prescription);
    setDataBool(data, "editable", m_editable);
    setDataBool(data, "medical", m_medical);
}


Documents::Documents()
{
    m_documents = new QMap<int, Document*>();
}

QMap<int, Document *> *Documents::documents() const
{
    return m_documents;
}

void Documents::addDocument(Document *Document)
{
    if( m_documents->contains(Document->id()) )
        return;
    m_documents->insert(Document->id(), Document);
}

void Documents::addDocument(QList<Document*> listDocuments)
{
    QList<Document*>::const_iterator it;
    for( it = listDocuments.constBegin(); it != listDocuments.constEnd(); ++it )
        addDocument( *it );
}

void Documents::clearAll()
{
    QList<Document*> listdocs;
    for( QMap<int, Document*>::const_iterator itdoc = m_documents->constBegin(); itdoc != m_documents->constEnd(); ++itdoc)
        delete itdoc.value();
    m_documents->clear();
}

void Documents::removeDocument(Document *doc)
{
    QMap<int, Document*>::const_iterator itdoc = m_documents->find(doc->id());
    if( itdoc == m_documents->constEnd() )
        return;
    m_documents->remove(doc->id());
    delete doc;
}

Document* Documents::getDocumentById(int id)
{
    QMap<int, Document*>::const_iterator itdoc = m_documents->find(id);
    if( itdoc == m_documents->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
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


MetaDocuments::MetaDocuments()
{
    m_metadocuments = new QMap<int, MetaDocument*>();
}

QMap<int, MetaDocument *> *MetaDocuments::metadocuments() const
{
    return m_metadocuments;
}

void MetaDocuments::addmetaDocument(MetaDocument *MetaDoc)
{
    if( m_metadocuments->contains(MetaDoc->id()) )
        return;
    m_metadocuments->insert(MetaDoc->id(), MetaDoc);
}

void MetaDocuments::addmetaDocument(QList<MetaDocument*> listMetaDocs)
{
    QList<MetaDocument*>::const_iterator it;
    for( it = listMetaDocs.constBegin(); it != listMetaDocs.constEnd(); ++it )
        addmetaDocument( *it );
}

void MetaDocuments::clearAll()
{
    QList<MetaDocument*> listdocs;
    for( QMap<int, MetaDocument*>::const_iterator itdoc = m_metadocuments->constBegin(); itdoc != m_metadocuments->constEnd(); ++itdoc)
        delete itdoc.value();
    m_metadocuments->clear();
}

void MetaDocuments::removemetaDocument(MetaDocument *metadoc)
{
    QMap<int, MetaDocument*>::const_iterator itdoc = m_metadocuments->find(metadoc->id());
    if( itdoc == m_metadocuments->constEnd() )
        return;
    m_metadocuments->remove(metadoc->id());
    delete metadoc;
}

MetaDocument* MetaDocuments::getmetaDocumentById(int id)
{
    QMap<int, MetaDocument*>::const_iterator itdoc = m_metadocuments->find(id);
    if( itdoc == m_metadocuments->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
}



