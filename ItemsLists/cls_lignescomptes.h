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

#ifndef LIGNESCOMPTES_H
#define LIGNESCOMPTES_H

#include "cls_lignecompte.h"
#include "cls_itemslist.h"

class LignesComptes : public ItemsList
{
private:
    QMap<int, LigneCompte*> *map_lignes = Q_NULLPTR;  //!< la liste de toutes les lignes

public:
    explicit LignesComptes(QObject *parent = Q_NULLPTR);
    ~LignesComptes();

    QMap<int, LigneCompte*>* lignescomptes() const;

    LigneCompte* getById(int id);
    void reloadLigne(LigneCompte *lign);
    void initListe(int idcompte);
    QMap<int, bool> initListeLignesByIdCompte(int id);    //! reconstruit la liste des lignes pour un compte

    //!> actions sur les enregistrements
    void       SupprimeLigne(LigneCompte *lign);
//    LigneCompte*    CreationLigne(int idCompte,
//                                  int iddepense,
//                                  int idrecette,
//                                  int idrecettespeciale,
//                                  int idremisechq,
//                                  QDate date,
//                                  QString libelle,
//                                  QString typeop,
//                                  double montant,
//                                  bool iscredit,
//                                  bool isconsolide);
};
#endif // LIGNESCOMPTES_H
