/* (C) 2020 LAINE SERGE
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

#ifndef COMMENTSLUNETS_H
#define COMMENTSLUNETS_H

#include "cls_commentlunet.h"
#include "cls_itemslist.h"

class CommentsLunets : public ItemsList
{
private:
    QMap<int, CommentLunet*> *map_all = Q_NULLPTR;    //!< la liste des comments

public:
    explicit CommentsLunets(QObject *parent = Q_NULLPTR);

    QMap<int, CommentLunet*> *commentaires() const;

    CommentLunet* getById(int id, bool reload = false);
    void initListe();

    //!> actions sur les enregistrements
    void                SupprimeCommentLunet(CommentLunet *comment);
    CommentLunet*       CreationCommentLunet(QHash<QString, QVariant> sets);
};

#endif // COMMENTSLUNETS_H
