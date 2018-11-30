#ifndef CLS_BANQUE_H
#define CLS_BANQUE_H

#include <QObject>
#include "cls_item.h"

/*!
 * \brief Banque class
 * l'ensemble des informations concernant une banque
 */


class Banque : public Item
{
private:
    int m_id, m_codebanque;
    QString m_idbanqueabrege, m_nombanque;

public:
    explicit Banque(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);

    int id() const;
    int CodeBanque() const;
    QString NomBanqueAbrege() const;
    QString NomBanque() const;
};


class Banques
{
private:
    QMap<int, Banque*> *m_banques; //!< la liste des Banques

public:
    explicit Banques();

    QMap<int, Banque *> *banques() const;

    void addBanque(Banque *banque);
    void addBanque(QList<Banque*> listbanques);
    void removeBanque(Banque* banque);
    Banque* getBanqueById(int id);
};

#endif // CLS_BANQUE_H
