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

#ifndef CLS_VILLES_H
#define CLS_VILLES_H

#include "cls_ville.h"
#include "cls_itemslist.h"

#include <QAbstractListModel>
#include <QJsonArray>

class VilleListModel : public QAbstractListModel
{
    Q_OBJECT
public: //STATIC
    static bool sortByName(const Ville *v1, const Ville *v2) { return v1->nom() < v2->nom(); }
    static bool sortByCodePostal(const Ville *v1, const Ville *v2) { return v1->codePostal() < v2->codePostal(); }

public:
    enum FieldName {NOM, CODEPOSTAL}; Q_ENUM(FieldName)
    explicit VilleListModel(const QList<Ville*> &villes, FieldName fieldName, QObject *parent = Q_NULLPTR);

    void setFieldName(FieldName fieldName);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    QList<Ville *>  m_villes;
    FieldName       m_fieldName;

};

class Villes : public ItemsList
{
    Q_OBJECT
private:
    QMultiMap<QString, Ville*> map_villes;          //!< map des villes par nom
    QMultiMap<QString, Ville*> map_codespostaux;    //!< map des villes par codePostal
    QStringList m_listeNomVilles;                   //!< la liste de nom de ville
    QStringList m_listeCodePostal;                  //!< la liste des codes postaux
    QMap<int, Ville*> *m_mapvilles = new QMap<int, Ville*>();   //!< la liste des villes
    bool add(Ville *ville);
    void addList(QList<Ville*> listvilles);

public:
    Villes(QObject *parent = Q_NULLPTR);
    enum TownsFrom{DATABASE, CUSTOM}; Q_ENUM(TownsFrom)
    void initListe(TownsFrom from = DATABASE);
    void enregistreNouvelleVille(QString CP, QString nomville);

    QStringList ListeNomsVilles();
    QStringList ListeCodesPostaux();
    QMap<int, Ville*> *villes() { return m_mapvilles; };

    QList<Ville *> getVilleByCodePostal(QString codePostal, bool testIntegrite = true);
    QList<Ville *> getVilleByName(QString name, bool distinct=false);
    QList<Ville *> getVilleByCodePostalEtNom(QString codePostal, QString name);
};

#endif // CLS_VILLES_H
