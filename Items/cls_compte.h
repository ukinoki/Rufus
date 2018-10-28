#ifndef CLS_COMPTE_H
#define CLS_COMPTE_H

#include <QObject>
#include "cls_item.h"

class Compte : public Item
{
private:
    int m_id, m_idbanque, m_iduser;
    QString m_nom, m_iban, m_intitulecompte, m_nombanque;
    bool m_desactive, m_partage, m_prefere;
    double m_solde;

public:
    explicit Compte(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);
    void setSolde(double solde);

    int id() const;
    int idBanque() const;
    int idUser() const;
    QString iban() const;
    QString intitulecompte() const;
    QString nom() const;
    double solde() const;
    bool isPartage() const;
    bool isDesactive() const;
    QString nombanque() const;
    bool isPrefere() const;
};


class Comptes
{
private:
    QMultiMap<int, Compte*> m_comptes; //!< la liste des comptes actifs d'un user
    QMultiMap<int, Compte*> m_comptesAll; //!< la liste de tous les comptes même inactifs d'un user
    QMultiMap<int, Compte*> m_comptesAllusers; //!< la liste de tous les comptes actifs de tous les utsers

public:
    explicit Comptes();

    QMultiMap<int, Compte *> comptes() const;
    QMultiMap<int, Compte *> comptesAll() const;
    QMultiMap<int, Compte *> comptesAllUsers() const;

    void addCompte(Compte *compte);
    void addCompte(QList<Compte*> listCompte);
    void removeCompte(Compte* cpt);
    Compte* getCompteById(int id);
};



#endif // CLS_COMPTE_H
