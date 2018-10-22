#ifndef CLS_CORRESPONDANTS_H
#define CLS_CORRESPONDANTS_H

#include <QObject>
#include <cls_correspondant.h>

class Correspondants
{
    QMap<int, Correspondant*> *m_Correspondants;    //!<Collection de tous les correspondants
    QMap<int, Correspondant*> *m_MGCorrespondants;  //!<Collection de tous les généralistes

public:
    //GETTER
    QMap<int, Correspondant *> *getCorrespondants()     const;
    QMap<int, Correspondant *> *getMGCorrespondants()   const;


    Correspondants();

    bool addCorrespondant(Correspondant *Correspondant);
    bool addMGCorrespondant(Correspondant *Correspondant);
    Correspondant* getCorrespondantById(int id);
    void VideLesListes();
};


#endif // CLS_CORRESPONDANTS_H
