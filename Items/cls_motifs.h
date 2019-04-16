#ifndef CLS_MOTIFS_H
#define CLS_MOTIFS_H

#include "cls_motif.h"
#include "database.h"

class Motifs
{
private:
    QMap<int, Motif*> *m_motifs; //!< la liste des Motifs

public:
    explicit Motifs();

    QMap<int, Motif *> *motifs() const;

    void addMotif(Motif *Motif);
    void addMotif(QList<Motif*> listMotifs);
    void removeMotif(Motif* motif);
    Motif* getMotifById(int id);
    void clearAll();
    void initListe();
};


#endif // CLS_MOTIFS_H
