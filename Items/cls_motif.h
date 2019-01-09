/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CLS_MOTIF_H
#define CLS_MOTIF_H

#include <QObject>
#include "cls_item.h"

/*!
 * \brief Motif class
 * l'ensemble des informations concernant un motif d'acte
 */


class Motif : public Item
{
private:
    int m_id, m_duree, m_noordre;
    QString m_motif, m_raccourci, m_couleur;
    bool m_pardefaut, m_utiliser;

public:
    explicit Motif(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);

    int id() const;
    QString motif() const;
    QString raccourci() const;
    QString couleur() const;
    int duree() const;
    bool pardefaut() const;
    bool utiliser() const;
    int noordre() const;
};


class Motifs
{
private:
    QMap<int, Motif*> *m_motifs; //!< la liste des Motifs

public:
    explicit Motifs();

    QMap<int, Motif *> *motifs() const;

    void addMotif(Motif *Motif);
    void addMotif(QList<Motif*> listMotifs);
    void removeMotif(Motif* Motif);
    Motif* getMotifById(int id);
    void clearAll();
};

#endif // CLS_MOTIF_H
