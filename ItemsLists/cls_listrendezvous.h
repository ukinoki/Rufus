#ifndef CLS_LISTRENDEZVOUS_H
#define CLS_LISTRENDEZVOUS_H

#include "cls_rendezvous.h"
#include "cls_itemslist.h"

class ListRendezVous : public ItemsList
{
private:
    QMap<int, RendezVous*> *map_all = Q_NULLPTR;      //!< la liste de tous les RendezVous

public:
    explicit ListRendezVous(QObject *parent = Q_NULLPTR);

    QMap<int, RendezVous*> *listRendezVous() const;

    RendezVous* getByIdPatient(int id);

    //!> actions sur les enregistrements
    void              SupprimeRendezVous(RendezVous *RendezVous);
    void              AddRendezVous(RendezVous *RendezVous);
};

#endif // CLS_LISTRENDEZVOUS_H
