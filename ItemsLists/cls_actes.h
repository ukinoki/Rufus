#ifndef CLS_ACTES_H
#define CLS_ACTES_H

#include "database.h"
#include "cls_acte.h"

class Actes

{
private:
    QMap<int, Acte*> *m_actes = Q_NULLPTR; //!< une liste d'actes

public:
    explicit Actes();

    QMap<int, Acte *> *actes() const;

    void    add(Acte *acte);
    void    addList(QList<Acte*> listActes);
    void    remove(Acte* acte);
    void    clearAll();
    Acte*   getById(int id);
    void    initListeByPatient(Patient *pat);
};


#endif // CLS_ACTES_H
