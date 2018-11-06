#include "dlg_paiementdirect.h"
#include "ui_dlg_paiementdirect.h"

dlg_paiementdirect::dlg_paiementdirect(QList<int> ListidActeAPasser, int Mode, Procedures *procAPasser, int PaiementAModifer, int idUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paiementdirect)
{
  ui->setupUi(this);
  ui->Loupelabel->setPixmap(Icons::pxLoupe().scaled(30,30)); //WARNING : icon scaled : pxLoupe 20,20
  ui->Loupelabel->setText("");
  connect(ui->AnnulupPushButton, &QPushButton::clicked, this, &dlg_paiementdirect::reject);
}

dlg_paiementdirect::~dlg_paiementdirect()
{
    delete ui;
}
