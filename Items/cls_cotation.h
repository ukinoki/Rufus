#ifndef CLS_COTATION_H
#define CLS_COTATION_H

#include <QObject>
#include "cls_item.h"

class Cotation : public Item
{
private:
    int m_id, m_iduser, m_frequence, m_idcotation;
    QString m_typeacte, m_descriptif;
    bool m_ccam;
    double m_montantoptam, m_montantnonoptam, m_montantpratique;

public:
    explicit Cotation(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);

    int id() const;
    int idcotation() const;
    QString typeacte() const;
    double montantoptam() const;
    double montantnonoptam() const;
    double montantpratique() const;
    bool isCCAM() const;
    int idUser() const;
    int frequence() const;
    QString descriptif() const;
};


class Cotations
{
private:
    QMap<int, Cotation*> *m_cotations;          //!< la liste des cotations
    QMap<int, Cotation*> *m_cotationsbyuser;    //!< la liste des cotations pratiquées par un utilisateur

public:
    explicit Cotations();

    QMap<int, Cotation *> *cotations() const;
    QMap<int, Cotation *> *cotationsbyuser() const;

    void addCotation(Cotation *cotation);
    void addCotationByUser(Cotation *cotation);
    void addCotation(QList<Cotation*> listcotations);
    void removeCotation(Cotation* cotation);
    Cotation* getCotationById(int id);
};

#endif // CLS_COTATION_H
