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

#ifndef CLS_ACTES_H
#define CLS_ACTES_H

#include "cls_acte.h"
#include "cls_itemslist.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include "upstandarditem.h"

class Actes: public ItemsList

{
private:
    QMap<int, Acte*>        *map_actes = Q_NULLPTR;                                         //!< une liste d'actes
    Acte                    *m_currentacte = Q_NULLPTR;                                     //! l'acte en cours
    QSortFilterProxyModel   *m_actesortmodel;
    QSortFilterProxyModel   *m_heuresortmodel;
    QStandardItemModel      *m_actesmodel;

public:
    explicit Actes(QObject *parent = Q_NULLPTR);

    QMap<int, Acte *> *actes() const;

    void setcurrentacte(Acte *act);
    Acte* currentacte() const       { return m_currentacte; }

    Acte*   getById(int id, Item::LOADDETAILS details = Item::LoadDetails);                 //!> charge un acte à partir de son id
    QMap<int, Acte*>::const_iterator   getLast();                                           //!> renvoie le dernier acte de la liste
    QMap<int, Acte*>::const_iterator   getAt(int idx);                                      //!> renvoie l'acte de la liste à l'index idx
    QMap<int, Acte*> *listCourriersByUser(int iduser);                                     //!> renvoie laliste des actes pour lesqules un courreir est à faire pour un user donné
    void    initListeByPatient(Patient *pat, Item::UPDATE upd = Item::NoUpdate, bool quelesid = false);
                                                                                            //!> charge tous les actes d'un patient

    void    sortActesByDate();                                                              /*! > trie la liste des actes par date, heure et met le résultat dans un QSortFilterProxyModel
                                                                                             * il arrive que la liste d'actes ne soit pas triée dans le bon ordre
                                                                                             * (acte créé a posteriori ou erreur sur la date) */
    Acte* getActeFromRow(int row);
    Acte* getActeFromIndex(QModelIndex idx);


    //!> actions combinées sur l'item et l'enregistrement correspondant en base de données

    //!> actions sur les champs

    //!> actions sur les enregistrements
    void    SupprimeActe(Acte *act);
    Acte*   CreationActe(Patient *pat, User *usr, int idcentre, int idlieu);

    //!< action sur toutes les données
    void    updateActe(Acte* acte);                                                         //!> met à jour les datas d'un acte à partir des données enregistrées dans la base

};


#endif // CLS_ACTES_H
