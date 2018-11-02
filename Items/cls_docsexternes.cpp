#include "cls_docsexternes.h"

DocsExternes::DocsExternes()
{
}

QHash<int, DocExterne *> DocsExternes::docsexternespardate()
{
    return m_docsexternespardate;
}
QHash<int, DocExterne *> DocsExternes::docsexternespartype()
{
    return m_docsexternespartype;
}

/*!
 * \brief docsexternes::getDocExterneById
 * \param id l'idimpr du Document recherché
 * \return nullptr si aucun Document trouvée
 * \return DocExterne* le Document correspondant à l'id
 */
DocExterne* DocsExternes::getDocumentById(int id, bool loadDetails, bool addToList)
{
    QHash<int, DocExterne*>::const_iterator itdoc = m_docsexternespardate.find(id);
    DocExterne *result;
    if( itdoc == m_docsexternespardate.constEnd() )
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
    {
        addDocExterneByDat( result );
        addDocExterneByTyp( result );
    }
    return result;
}

DocExterne* DocsExternes::reloadDocument(DocExterne* docmt)
{
    docmt->setAllLoaded(false);
    return getDocumentById(docmt->id());
}

void DocsExternes::addDocExterneByDat(DocExterne *docext)
{
    if( m_docsexternespardate.contains(docext->id()) )
        return;
    else
        m_docsexternespardate.insert(docext->id(), docext);
}

void DocsExternes::addDocExterneByTyp(DocExterne *docext)
{
    if( m_docsexternespartype.contains(docext->id()) )
        return;
    else
        m_docsexternespartype.insert(docext->id(), docext);
}

void DocsExternes::addListDocsExternesByTyp(QList<DocExterne*> listdocs)
{
    for(QList<DocExterne*>::const_iterator it = listdocs.constBegin(); it != listdocs.constEnd(); ++it )
    {
        DocExterne *doc = const_cast<DocExterne*>(*it);
        addDocExterneByTyp(doc);
    }
}

void DocsExternes::addListDocsExternesByDat(QList<DocExterne*> listdocs)
{
    for(QList<DocExterne*>::const_iterator it = listdocs.constBegin(); it != listdocs.constEnd(); ++it )
    {
        DocExterne *doc = const_cast<DocExterne*>(*it);
        addDocExterneByDat(doc);
    }
}

void DocsExternes::VideLesListes()
{
    m_docsexternespardate.clear();
    m_docsexternespartype.clear();
}

void DocsExternes::RemoveKey(int key)
{
    m_docsexternespardate.remove(key);
    m_docsexternespartype.remove(key);
}

void DocsExternes::TrieDocsExternes()
{
    QList<QDateTime>listdates;
    QStringList listtypes;
    QHash<int, DocExterne*>::const_iterator itdocdate;
    for( itdocdate = m_docsexternespartype.constBegin(); itdocdate != m_docsexternespartype.constEnd(); ++itdocdate )
    {
        DocExterne *doc= const_cast<DocExterne*>(itdocdate.value());
        listdates << doc->date();
        listtypes << doc->typedoc();
    }
    std::sort(listdates.begin(), listdates.end());
    listtypes.sort();
}

