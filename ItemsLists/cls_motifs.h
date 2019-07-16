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

#ifndef CLS_MOTIFS_H
#define CLS_MOTIFS_H

#include "cls_motif.h"
#include "database.h"
#include "cls_itemslist.h"

class Motifs : public ItemsList
{
private:
    QMap<int, Motif*> *m_motifs; //!< la liste des Motifs

public:
    explicit Motifs(QObject *parent = Q_NULLPTR);

    QMap<int, Motif *> *motifs() const;

    Motif* getById(int id);
    Motif* getMotifFromRaccourci(QString txt);
    void initListe();

    //!> actions sur les enregistrements
    void        SupprimeMotif(Motif *motif);
    Motif*      CreationMotif(QString Motif, QString Raccourci, QString Couleur, int Duree, bool ParDefaut, bool Utiliser, int NoOrdre);

};

#endif // CLS_MOTIFS_H
