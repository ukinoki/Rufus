#ifndef CLS_LIEUXTRAVAIL_H
#define CLS_LIEUXTRAVAIL_H

#include "cls_item.h"

#include <QJsonObject>
#include <QObject>

class Site : public Item
{
    Q_OBJECT
private:
    //Adresse de travail
    int m_id;
    QString m_nom;
    QString m_adresse1;
    QString m_adresse2;
    QString m_adresse3;
    int m_codePostal;
    QString m_ville;
    QString m_telephone;
    QString m_fax;

public:
    explicit Site(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);


    int id() const;
    QString nom() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    int codePostal() const;
    QString ville() const;
    QString telephone() const;


signals:

public slots:
};

#endif // CLS_LIEUXTRAVAIL_H
