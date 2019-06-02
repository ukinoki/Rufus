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

#ifndef CLS_DOCUMENTS_H
#define CLS_DOCUMENTS_H

#include "database.h"
#include "cls_document.h"
#include "cls_itemslist.h"

class Documents : public ItemsList
{
private:
    QMap<int, Document*> *m_documents = Q_NULLPTR;      //!< la liste des Documents
    void addList(QList<Document*> listDocuments);

public:
    explicit Documents(QObject *parent = Q_NULLPTR);

    QMap<int, Document *> *documents() const;

    Document* getById(int id);
    void initListe();
};

class MetaDocuments : public ItemsList
{
private:
    QMap<int, MetaDocument*> *m_metadocuments = Q_NULLPTR;      //!< la liste des metadocuments
    void addList(QList<MetaDocument*> listmetaDocs);

public:
    explicit MetaDocuments();

    QMap<int, MetaDocument *> *metadocuments() const;

    void clearAll();
    MetaDocument* getById(int id);
    void initListe();
};


#endif // CLS_DOCUMENTS_H
