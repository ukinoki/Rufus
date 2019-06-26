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

#ifndef CLS_LIGNESPAIEMENTS_H
#define CLS_LIGNESPAIEMENTS_H


#include "cls_lignepaiement.h"
#include "database.h"
#include "cls_itemslist.h"

class LignesPaiements : public ItemsList
{
private:
    QMap<QString, LignePaiement*> *m_lignespaiements; /*! la liste des lignes de paiements
                                                        * on utilise un QString pour la key du QMap parce qu'il n'y a pas de champ id
                                                        * dans la table ComptaMedicale.lignesaiements correspondant à cette classe (erreur du début de la rédaction du programme)
                                                        * par contre, le couple idActe/idRecette est unique pour chaque enregistrement
                                                        * la key est donc la concaténation de ces 2 champs séparés par un séparateur */
    void                            addList(QList<LignePaiement*> listlignes);

public:
    explicit                        LignesPaiements(QObject *parent = Q_NULLPTR);

    QMap<QString, LignePaiement *>* lignespaiements() const;
    LignePaiement*                  getById(QString stringid);
    void                            initListeByPatient(Patient *pat);

    //!> actions sur les enregistrements
    void                            SupprimeActeLignesPaiements(Acte *act);         //!> Supprime toutes les lignes de paiement correspondant à un acte
};

#endif // CLS_LIGNESPAIEMENTS_H
