#ifndef CLS_COMPTE_H
#define CLS_COMPTE_H

#include <QObject>
#include "cls_item.h"

class Compte : public Item
{
private:
    int m_id;
    QString m_nom, m_iban, m_intitulecompte, m_nombanque;
    bool m_desactive;

public:
    explicit Compte(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);


    int id() const;
    QString nom() const;
    bool isDesactive() const;
    QString iban() const;
    QString intitulecompte() const;
    QString nombanque() const;
};


class Comptes
{
private:
    QMultiMap<int, Compte*> m_comptes; //!< la liste des comptes actifs
    QMultiMap<int, Compte*> m_comptesAll; //!< la liste de tous les comptes

public:
    explicit Comptes();

    QMultiMap<int, Compte *> comptes() const;
    QMultiMap<int, Compte *> comptesAll() const;

    void addCompte(Compte *compte);
    void addCompte(QList<Compte*> listCompte);
};



#endif // CLS_COMPTE_H
