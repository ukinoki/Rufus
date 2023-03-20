#ifndef CLS_VILLE_H
#define CLS_VILLE_H

#include "cls_item.h"

class Ville : public Item
{
private:
    //!< m_id l'id de la ville en base
    QString m_codePostal;   //!< le code postal de la ville
    QString m_nom;          //!< le nom de la ville

public:
    explicit Ville(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    QString codepostal() const;
    QString nom() const;
    void setcodepostal(const QString &codePostal);
    void setnom(const QString &nom);
    void setData(QJsonObject data);
};
#endif // CLS_VILLE_H
