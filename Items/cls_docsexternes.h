#ifndef CLS_DOCSEXTERNES_H
#define CLS_DOCSEXTERNES_H

#include <QObject>
#include "cls_docexterne.h"
#include "database.h"

class DocsExternes
{
private:
    QHash<int, DocExterne*> m_docsexternes; //!< la liste des DocExternes pour un patient donné
    bool m_nouveaudocument;

public:
    explicit DocsExternes();
    QHash<int, DocExterne *> docsexternes();
    DocExterne* getDocumentById(int id, bool loadDetails=true, bool addToList=true);
    void addDocExterne(DocExterne *DocExterne);
    void addListDocsExternes(QList<DocExterne*> listdocs);
    bool NouveauDocument();
    void setNouveauDocumentFalse();
    DocExterne* reloadDocument(DocExterne* docmt);
    void RemoveKey(int key);
    void VideLaListe();
};
#endif // CLS_DOCSEXTERNES_H
