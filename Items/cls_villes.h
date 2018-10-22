#ifndef CLS_SITES_H
#define CLS_SITES_H

#include "cls_item.h"

#include <QAbstractListModel>
#include <QJsonArray>
#include <QObject>

class Ville : public Item
{
private:
    int m_id;           //!< l'id de la ville en base
    QString m_codePostal;   //!< le code postal de la ville
    QString m_nom;      //!< le nom de la ville

public:
    explicit Ville(QJsonObject data = {}, QObject *parent = nullptr);

    int id() const;
    QString codePostal() const;
    QString nom() const;


    void setData(QJsonObject data);

};

class VilleListModel : public QAbstractListModel
{
    Q_OBJECT
public: //STATIC
    static bool sortByName(const Ville *v1, const Ville *v2) { return v1->nom() < v2->nom(); }
    static bool sortByCodePostal(const Ville *v1, const Ville *v2) { return v1->codePostal() < v2->codePostal(); }

private:
    QList<Ville *> m_villes;
    QString m_fieldName;

public:
    explicit VilleListModel(const QList<Ville*> &villes, QString fieldName, QObject *parent=0);

    void setFieldName(QString fieldName);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

private:

};

class Villes
{
private:
    QMultiMap<QString, Ville*> m_villes;        //!< la liste des villes par nom
    QMultiMap<QString, Ville*> m_codePostal;    //!< la liste des villes par codePostal
    QStringList m_listeNomVilles;               //!< la liste de nom de ville
    QStringList m_listeCodePostal;              //!< la liste des codes postaux

public:
    explicit Villes();

    void addVille(Ville *ville);

    QStringList getListVilles();
    QStringList getListCodePostal();

    QList<Ville *> getVilleByCodePostal(QString codePostal, bool testIntegrite = true);
    QList<Ville *> getVilleByName(QString name, bool distinct=false);
    QList<Ville *> getVilleByCodePostalEtNom(QString codePostal, QString name);
};




#endif // CLS_SITES_H
