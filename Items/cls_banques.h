#ifndef CLS_BANQUES_H
#define CLS_BANQUES_H

#include "database.h"
#include "cls_banque.h"

class Banques
{
private:
    QMap<int, Banque*> *m_banques = Q_NULLPTR; //!< la liste des Banques

public:
    explicit Banques();

    QMap<int, Banque *> *banques() const;

    void    addBanque(Banque *banque);
    void    addBanque(QList<Banque*> listbanques);
    void    removeBanque(Banque* banque);
    void    clearAll();
    Banque* getBanqueById(int id);
    void    initListe();
};

#endif // CLS_BANQUES_H
