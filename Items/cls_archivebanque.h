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

#ifndef CLS_ARCHIVE_H
#define CLS_ARCHIVE_H

#include <QObject>
#include "cls_item.h"

class Archive : public Item
{
private:
    int m_idligne, m_idcompte;
    int m_iddepense, m_idrecette, m_idrecettespeciale, m_idremisecheque;
    QDate m_lignedate, m_lignedateconsolidation;
    QString m_lignelibelle, m_lignetypeoperation;
    int m_idarchive;
    double m_montant;

public:
    explicit Archive(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int id() const;
    int idcompte() const;
    int iddepense() const;
    int idrecette() const;
    int idrecettespeciale() const;
    int idremisecheque() const;
    QDate lignedate() const;
    QString lignelibelle() const;
    double montant() const;
    QString lignetypeoperation() const;
    QDate lignedateconsolidation() const;
    int idarchive() const;
};


class Archives
{
private:
    QMap<int, Archive*>* m_archives = Q_NULLPTR; //!< la liste des archives
public:
    explicit Archives();
    ~Archives();
    QMap<int, Archive *>* archives() const;
    void addArchive(Archive *archive);
    void addArchive(QList<Archive*> listarchives);
    void removeArchive(Archive* arch);
    void clearAll();
};

#endif // CLS_ARCHIVE_H
