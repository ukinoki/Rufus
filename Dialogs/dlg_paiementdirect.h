#ifndef DLG_PAIEMENTDIRECT_H
#define DLG_PAIEMENTDIRECT_H

#include <QDialog>
#include "procedures.h"

namespace Ui {
class dlg_paiementdirect;
}

/*!
 * \brief l'ensemble des informations concernant un paiments direct ( = en attente d'être enregistré )
 */

class dlg_paiementdirect : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_paiementdirect(QList<int> ListidActeAPasser, int Mode, Procedures *procAPasser, int PaiementAModifer=0, int idUser=0, QWidget *parent = Q_NULLPTR);
    ~dlg_paiementdirect();

private:
    Ui::dlg_paiementdirect *ui;
};

#endif // DLG_PAIEMENTDIRECT_H
