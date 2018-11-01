#ifndef CLS_DOCSEXTERNES_H
#define CLS_DOCSEXTERNES_H

#include <QObject>
#include "cls_docexterne.h"
#include "database.h"

class DocsExternes
{
private:
    QHash<int, DocExterne*> m_docsexternespardate; //!< la liste des DocExternes pour un patient donné triée par date
    QHash<int, DocExterne*> m_docsexternespartype; //!< la liste des DocExternes pour un patient donné triée par type de document
    void TrieDocsExternes();
public:
    explicit DocsExternes();
    QHash<int, DocExterne *> docsexternespardate();
    QHash<int, DocExterne *> docsexternespartype();
    DocExterne* getDocumentById(int id, bool loadDetails=true, bool addToList=true);
    void addDocExterneByDat(DocExterne *DocExterne);
    void addDocExterneByTyp(DocExterne *DocExterne);
    void addListDocsExternesByDat(QList<DocExterne*> listdocs);
    void addListDocsExternesByTyp(QList<DocExterne*> listdocs);
    DocExterne* reloadDocument(DocExterne* docmt);
    void VideLesListes();
};
#endif // CLS_DOCSEXTERNES_H
