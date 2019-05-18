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

#ifndef CLS_COMPTES_H
#define CLS_COMPTES_H

#include "cls_compte.h"
#include "database.h"

class Comptes : public QObject
{
private:
    QMap<int, Compte*> *m_comptes = Q_NULLPTR;  //!< la liste de tous les comptes

public:
    explicit Comptes(QObject *parent = Q_NULLPTR);
    ~Comptes();

    QMap<int, Compte*>* comptes() const;

    void add(Compte *compte);
    void addList(QList<Compte*> listCompte);
    void removeCompte(Compte* cpt);
    void clearAll();
    Compte* getCompteById(int id);
    void reloadCompte(Compte*compte);

    void initListe();
};

#endif // CLS_COMPTES_H
