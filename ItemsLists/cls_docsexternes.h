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


#ifndef CLS_DOCSEXTERNES_H
#define CLS_DOCSEXTERNES_H

#include "cls_docexterne.h"
#include "database.h"

class DocsExternes
{
private:
    QMap<int, DocExterne*> *m_docsexternes = Q_NULLPTR; //!< la liste des DocExternes pour un patient donné
    bool m_nouveaudocument;

public:
    explicit DocsExternes();
    QMap<int, DocExterne *>* docsexternes();
    DocExterne* getById(int id, bool loadDetails=true, bool addToList=true);
    bool add(DocExterne *doc);
    void addList(QList<DocExterne*> listdocs);
    bool NouveauDocument();
    void setNouveauDocumentFalse();
    DocExterne* reload(DocExterne* docmt);
    void remove(DocExterne *doc);
    void clearAll();
};
#endif // CLS_DOCSEXTERNES_H
