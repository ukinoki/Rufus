#ifndef CLS_FLAGS_H
#define CLS_FLAGS_H

#include "database.h"

class Flags : public QObject

{
    Q_OBJECT
public:
    explicit                Flags(QObject *parent = Q_NULLPTR);
    ~Flags();

    void                    setTCP(bool tcp);

    int                     flagCorrespondants();               //!> flag de mise à jour de la liste des corresondants
    int                     flagMessages();                     //!> flag de vérification des messages
    int                     flagSallleDAttente();               //!> flag de mise à jour de la salled'attente
    void                    MAJflagCorrespondants();            //!<  MAJ du flag de la liste des correspondants
    void                    MAJflagMessages();                  //!<  MAJ du flag de la messagerie
    void                    MAJFlagSalleDAttente();             //!<  MAJ du flag de la salle d'attente

signals:
    void                    UpdCorrespondants();                //!< signal de mise à jour de la liste des correspondants
    void                    UpdMessages();                      //!< signal de mise à jour de la messagerie
    void                    UpdSalleDAttente();                 //!< signal de mise à jour de la salle d'attente

private:
    bool                    ok;
    bool                    TCP;
};


#endif // CLS_FLAGS_H
