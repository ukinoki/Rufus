#ifndef DLG_PAIEMENTDIRECT_H
#define DLG_PAIEMENTDIRECT_H

#include <QDialog>
#include "procedures.h"
#include "database.h"
#include "gbl_datas.h"
#include "icons.h"
#include "utils.h"

namespace Ui {
class dlg_paiementdirect;
}

/*!
 * \brief l'ensemble des informations concernant un paiments direct ( = en attente d'être enregistré )
 * rappel pour un acte
 * iduser           = le soignant responsable médical de l'acte
 * creepar          = le soignant qui a cree l'acte
 * userparent       = le soignant remplacé si iduser est un remplaçant
 * usercomptable    = le user qui comptabilise l'acte
 */

class dlg_paiementdirect : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_paiementdirect(QList<int> ListidActeAPasser, Procedures *procAPasser, int PaiementAModifer=0, QWidget *parent = Q_NULLPTR);
    ~dlg_paiementdirect();

private:
    Ui::dlg_paiementdirect      *ui;
    Procedures                  *proc;
    DataBase                    *db;

    bool                        FermeALaFin;
    bool                        InitOK;
    int                         gidComptableACrediter;
    int                         gidCompteBancaireParDefaut;
    QList<int>                  gListidActe;
    QMap<int, User*>            *m_listeComptables;
    QMap<int, User*>            *m_listeParents;
    QMap<int, Banque*>          *m_listeBanques;
    QStandardItemModel          *glistComptesEncaissmt;
    QStandardItemModel          *glistComptesEncaissmtAvecDesactive;
    User                        *m_userConnected, *UserComptableACrediter;

    void                        ReconstruitListeBanques();

};

#endif // DLG_PAIEMENTDIRECT_H
