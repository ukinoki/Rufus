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

#ifndef CLS_DOCSEXTERNES_H
#define CLS_DOCSEXTERNES_H

#include <QObject>
#include "cls_docexterne.h"
#include "database.h"

class DocsExternes
{
private:
    QMap<int, DocExterne*> m_docsexternes; //!< la liste des DocExternes pour un patient donné
    bool m_nouveaudocument;

public:
    explicit DocsExternes();
    QMap<int, DocExterne *> docsexternes();
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
