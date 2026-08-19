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

#ifndef IOLS_H
#define IOLS_H

#include "cls_iol.h"
#include "cls_itemslist.h"

class QWidget;
class QDomDocument;

class IOLs : public ItemsList
{
    Q_OBJECT
private:
    QMap<int, IOL*> *map_all = nullptr;       //!< la liste de tous les IOLs, y compris ceux qui nes ont plus fabriqués
    bool m_isfull = false;                      //! la liste des IOL a été complètement initialisée
public:
    //! Récapitulatif d'un import/mise à jour de la liste des implants (pour l'affichage par l'appelant).
    struct ImportResult { int nouveaux = 0; int misAJour = 0; int total = 0; };

    explicit IOLs(QObject *parent = nullptr);

    QMap<int, IOL*> *iols() const;

    IOL*    getById(int id, bool reload = false);
    void    initListe();                        //! la liste de tous les IOLs
    bool    isfull() const  { return m_isfull; }

    //!> actions sur les enregistrements
    void    SupprimeIOL(IOL *iol, QWidget *parent = nullptr);
    IOL*    CreationIOL(QHash<QString, QVariant> sets, QWidget *parent = nullptr);

    //! Met à jour la liste des implants à partir d'un export IOLCon (docxml). Toute la logique base
    //! (fabricants, création/mise à jour des IOLs, images) — appelable depuis n'importe quelle classe.
    //! parent ne sert que de parent aux éventuelles boîtes d'erreur. Renvoie un récapitulatif.
    ImportResult ImportListeIOLS(QDomDocument docxml, QWidget *parent = nullptr);

    //! Vérifie en ligne (IOLCon) qu'une version plus récente de la liste des implants existe et,
    //! si l'utilisateur l'accepte, l'incorpore. Appelable depuis n'importe quelle classe ; parent
    //! sert de parent aux boîtes (question puis récapitulatif).
    void HasNewVersion(QWidget *parent = nullptr);

signals:
    //! Émis après une mise à jour de la liste des implants, pour que l'IHM se rafraîchisse.
    void listeModifiee();
};


#endif // IOLS_H
