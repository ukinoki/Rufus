#ifndef CLS_DEPENSES_H
#define CLS_DEPENSES_H

#include <QObject>
#include "cls_depense.h"

class Depenses : public QObject
{
public:
    QHash<int, Depense*> *m_Depenses;    //!< Collection de toutes les depenses pour unuser et une  année donnée - le QHash plutôt que le QMap pour garder le tri par date

public:
    //GETTER
    QHash<int, Depense *> *getDepenses()     const;

    Depenses(QObject *parent = Q_NULLPTR);

    bool addDepense(Depense *Depense);
    Depense* getDepenseById(int id);

};

#endif // CLS_DEPENSES_H
