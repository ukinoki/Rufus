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

#ifndef MOTSCLES_H
#define MOTSCLES_H

#include "cls_motcle.h"
#include "cls_itemslist.h"
#include <QCompleter>

class MotsCles : ItemsList
{
private:
    QMap<int, MotCle*> *map_motscles = Q_NULLPTR;    //!< la liste des motcles
    QCompleter *m_completer= new QCompleter();

public:
    explicit MotsCles(QObject *parent = Q_NULLPTR);

    QMap<int, MotCle*> *motscles() const;

    MotCle* getById(int id, bool reload = false);
    void initListe();
    QCompleter* completer();

    //!> actions sur les enregistrements
    void          SupprimeMotCle(MotCle *motcle);
    MotCle*       CreationMotCle(QHash<QString, QVariant> sets);
    bool          isThisMCusedForOtherPatients(MotCle* mc, int idpat);

};

#endif // MOTSCLES_H
