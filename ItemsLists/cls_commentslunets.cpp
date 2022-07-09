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

#include "cls_commentslunets.h"

CommentsLunets::CommentsLunets(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, CommentLunet*>();
}

QMap<int, CommentLunet*>* CommentsLunets::commentaires() const
{
    return map_all;
}

CommentLunet* CommentsLunets::getById(int id, bool reload)
{
    QMap<int, CommentLunet*>::const_iterator itref = map_all->constFind(id);
    if( itref == map_all->constEnd() )
    {
        CommentLunet * itm = DataBase::I()->loadCommentLunetById(id);
        if (itm != Q_NULLPTR)
            add( map_all, itm );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<CommentLunet*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        CommentLunet* itm = DataBase::I()->loadCommentLunetById(id);
        if (itm)
        {
            itref.value()->setData(itm->datas());
            delete itm;
        }
    }
    return itref.value();
}

/*!
 * \brief CommentsLunets::initListe
 * Charge l'ensemble des commentaires
 * et les ajoute à la classe CommentsLunets
 */
void CommentsLunets::initListe()
{
    QList<CommentLunet*> listCommentsLunets = DataBase::I()->loadCommentsLunets();
    epurelist(map_all, &listCommentsLunets);
    addList(map_all, &listCommentsLunets, Item::Update);
}

void CommentsLunets::SupprimeCommentLunet(CommentLunet* comment)
{
    Supprime(map_all, comment);
}

CommentLunet* CommentsLunets::CreationCommentLunet(QHash<QString, QVariant> sets)
{
    CommentLunet *comment = Q_NULLPTR;
    int idcomment = 0;
    DataBase::I()->locktables(QStringList() << TBL_COMMENTAIRESLUNETTES);
    idcomment = DataBase::I()->selectMaxFromTable(CP_ID_COMLUN, TBL_COMMENTAIRESLUNETTES, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idcomment;
        sets[CP_ID_COMLUN] = idcomment;
        result = DataBase::I()->InsertSQLByBinds(TBL_COMMENTAIRESLUNETTES, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce commentaire dans la base!"));
        return comment;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_COMLUN] = idcomment;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDUSER_COMLUN)              data[champ] = itset.value().toInt();
        else if (champ == CP_RESUME_COMLUN)         data[champ] = itset.value().toString();
        else if (champ == CP_TEXT_COMLUN)           data[champ] = itset.value().toString();
        else if (champ == CP_PUBLIC_COMLUN)         data[champ] = itset.value().toBool();
        else if (champ == CP_PARDEFAUT_COMLUN)      data[champ] = itset.value().toBool();
    }
    comment = new CommentLunet(data);
    if (comment != Q_NULLPTR)
        map_all->insert(comment->id(), comment);
    return comment;
}

