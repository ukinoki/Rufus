#ifndef CLS_LIEUXTRAVAIL_H
#define CLS_LIEUXTRAVAIL_H

#include "cls_item.h"

#include <QJsonObject>
#include <QObject>

class Etablissement : public Item
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
    explicit Etablissement(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);


    int getId() const;
    QString getNom() const;
    QString getAdresse1() const;
    QString getAdresse2() const;
    QString getAdresse3() const;
    int getCodePostal() const;
    QString getVille() const;
    QString getTelephone() const;


signals:

public slots:
};

#endif // CLS_LIEUXTRAVAIL_H
