/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_gestionbanques.h"
#include "ui_dlg_gestionbanques.h"
#include "icons.h"
#include "utils.h"
#include "database.h"

dlg_gestionbanques::dlg_gestionbanques(QWidget *parent, QString nouvbanqueabrege) :
    UpDialog(parent),
    ui(new Ui::dlg_gestionbanques)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    db                      = DataBase::I();

    gFermeApresValidation   = (nouvbanqueabrege != "");
    setWindowTitle(tr("Enregistrer une nouvelle banque"));
    setAttribute(Qt::WA_DeleteOnClose);

    dlglayout()               ->insertWidget(0, ui->Banqueframe);
    if (gFermeApresValidation)
    {
        NouvBanque();
        ui->NomAbregeupLineEdit->setText(nouvbanqueabrege);
        ui->NomAbregeupLineEdit->setFocusPolicy(Qt::NoFocus);
        dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    }
    else
    {
        int larg    = 350;
        int ncolvis = 12;
        int hautrow = int(QFontMetrics(qApp->font()).height()*1.3);
        int haut    = hautrow * ncolvis;
        uptablebanq = new UpTableWidget();
        uptablebanq->setFixedWidth(larg);
        uptablebanq->resize(larg, haut);
        uptablebanq->verticalHeader()->setVisible(false);
        uptablebanq->setFocusPolicy(Qt::StrongFocus);
        uptablebanq->setSelectionMode(QAbstractItemView::SingleSelection);
        uptablebanq->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        uptablebanq->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        uptablebanq->setGridStyle(Qt::DotLine);
        uptablebanq->setEditTriggers(QAbstractItemView::NoEditTriggers);
        uptablebanq->setSelectionBehavior(QAbstractItemView::SelectRows);
        uptablebanq->setColumnCount(2);
        uptablebanq->setColumnHidden(0,true);
        uptablebanq->setColumnWidth(1,uptablebanq->width()-2);
        uptablebanq->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
        uptablebanq->setHorizontalHeaderItem(1, new QTableWidgetItem(Icons::icEuro(),"Banques"));
        uptablebanq->horizontalHeader()->setVisible(true);
        uptablebanq->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
        uptablebanq->horizontalHeader()->setIconSize(QSize(25,25));
        uptablebanq->horizontalHeader()->setFixedHeight(hautrow);

        widgButtons = new WidgetButtonFrame(uptablebanq);
        widgButtons->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
        /*
        widgButtons->widgButtonParent()->setGeometry(10,10,larg, haut + widgButtons->height());
        widgButtons->widgButtonParent()->sizePolicy().setVerticalStretch(10);
        widgButtons->widgButtonParent()->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);
        int x = widgButtons->widgButtonParent()->sizeHint().width();
        int y = widgButtons->widgButtonParent()->sizeHint().height();
        qDebug() << (widgButtons->widgButtonParent()->sizeHint().isValid()? "Valide": "Null") << " - x = " + QString::number(x) + " - y = " + QString::number(y) + " - hauteur ligne = " + QString::number(haut + widgButtons->height()) + " Fixed";
        widgButtons->widgButtonParent()->sizePolicy().setVerticalPolicy(QSizePolicy::Preferred);
        x = widgButtons->widgButtonParent()->sizeHint().width();
        y = widgButtons->widgButtonParent()->sizeHint().height();
        qDebug() << (widgButtons->widgButtonParent()->sizeHint().isValid()? "Valide": "Null") << " - x = " + QString::number(x) + " - y = " + QString::number(y) + " - hauteur ligne = " + QString::number(haut + widgButtons->height()) + " Preferred";
        widgButtons->widgButtonParent()->sizePolicy().setVerticalPolicy(QSizePolicy::MinimumExpanding);
        x = widgButtons->widgButtonParent()->sizeHint().width();
        y = widgButtons->widgButtonParent()->sizeHint().height();
        qDebug() << (widgButtons->widgButtonParent()->sizeHint().isValid()? "Valide": "Null") << " - x = " + QString::number(x) + " - y = " + QString::number(y) + " - hauteur ligne = " + QString::number(haut + widgButtons->height()) + " MinimumExpanding";
        widgButtons->widgButtonParent()->sizePolicy().setVerticalPolicy(QSizePolicy::Minimum);
        x = widgButtons->widgButtonParent()->sizeHint().width();
        y = widgButtons->widgButtonParent()->sizeHint().height();
        qDebug() << (widgButtons->widgButtonParent()->sizeHint().isValid()? "Valide": "Null") << " - x = " + QString::number(x) + " - y = " + QString::number(y) + " - hauteur ligne = " + QString::number(haut + widgButtons->height()) + " Minimum";
        */
        dlglayout()               ->insertWidget(0,widgButtons->widgButtonParent());
        int r,t,l,b;
        dlglayout()               ->getContentsMargins(&r,&t,&l,&b);
        AjouteLayButtons(UpDialog::ButtonClose);
        resize(larg + r + l,
               haut + widgButtons->height() + ui->Banqueframe->height() + CloseButton->height() + t + b + (dlglayout()->spacing()*2));
        RemplirTableWidget();
        uptablebanq->setCurrentCell(0,1);
        AfficheBanque();
        connect(uptablebanq,        &UpTableWidget::itemSelectionChanged,   this,   &dlg_gestionbanques::AfficheBanque);
        connect(widgButtons,        &WidgetButtonFrame::choix,              this,   &dlg_gestionbanques::ChoixButtonFrame);
        connect(CloseButton,        &QPushButton::clicked,                  this,   &dlg_gestionbanques::accept);
        ui->AnnulModifupSmallButton ->setVisible(false);
        ui->OKModifupSmallButton    ->setVisible(false);
    }

    connect(ui->AnnulModifupSmallButton,    &QPushButton::clicked,    this,   &dlg_gestionbanques::AnnuleModifBanque);
    connect(ui->OKModifupSmallButton,       &QPushButton::clicked,    this,   &dlg_gestionbanques::ValideModifBanque);

    ui->NomBanqueupLineEdit ->setValidator(new QRegExpValidator(Utils::rgx_rx));
    ui->NomAbregeupLineEdit ->setValidator(new QRegExpValidator(Utils::rgx_MajusculeSeul));
}

dlg_gestionbanques::~dlg_gestionbanques()
{
    delete ui;
}

void dlg_gestionbanques::AfficheBanque()
{
    UpLabel* lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(uptablebanq->currentRow(),1));
    int idBanque = uptablebanq->item(lbl->Row(),0)->text().toInt();
    auto itbq = Datas::I()->banques->banques()->find(idBanque);
    if (itbq != Datas::I()->banques->banques()->constEnd())
    {
        Banque *banq = const_cast<Banque*>(itbq.value());
        ui->NomBanqueupLineEdit->setText(banq->nom());
        ui->NomAbregeupLineEdit->setText(banq->nomabrege());
    }
    widgButtons->moinsBouton->setEnabled(true);
    foreach (Compte *cpt, Datas::I()->comptes->comptes()->values())
        if (cpt->idBanque() == idBanque)
        {
            widgButtons->moinsBouton->setEnabled(false);
            return;
        }
}

void dlg_gestionbanques::AnnuleModifBanque()
{
    if (gFermeApresValidation)
        reject();
    else
    {
        RemetEnNorm();
        AfficheBanque();
    }
}

void dlg_gestionbanques::ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        NouvBanque();
        break;
    case 0:
        ModifBanque();
        break;
    case -1:
        SupprBanque();
        break;
    default:
        break;
    }
}

void dlg_gestionbanques::NouvBanque()
{
    ui->AnnulModifupSmallButton->setVisible(true);
    ui->OKModifupSmallButton->setVisible(true);
    ui->Banqueframe->setEnabled(true);
    ui->NomBanqueupLineEdit->clear();
    ui->NomAbregeupLineEdit->clear();
    if (!gFermeApresValidation)
    {
        uptablebanq->setEnabled(false);
        widgButtons->setEnabled(false);
    }
    gMode = Nouv;
}

void dlg_gestionbanques::ModifBanque()
{
    ui->AnnulModifupSmallButton->setVisible(true);
    ui->OKModifupSmallButton->setVisible(true);
    ui->Banqueframe->setEnabled(true);
    uptablebanq->setEnabled(false);
    widgButtons->setEnabled(false);
    gMode = Modif;
}

void dlg_gestionbanques::SupprBanque()
{
    UpLabel* lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(uptablebanq->currentRow(),1));
    int idBanque = uptablebanq->item(lbl->Row(),0)->text().toInt();
    UpMessageBox msgbox;
    UpSmallButton OKBouton(tr("Supprimer"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.setText(tr("Supprimer la banque ") + lbl->text() + "?");
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
       return;
    foreach (Compte *cpt, *Datas::I()->comptes->comptes())
        if (cpt->idBanque() == idBanque)
        {
            UpMessageBox::Watch(this, tr("Impossible de supprimer la banque ") + lbl->text(), tr("Elle est utilisée par d'autres utilisateurs"));
            return;
        }
    Datas::I()->banques->SupprimeBanque(Datas::I()->banques->getById(idBanque));
    RemplirTableWidget();
    AfficheBanque();
}

void dlg_gestionbanques::ValideModifBanque()
{
    QString msg = "";
    QString nombanque = Utils::trimcapitilize(ui->NomBanqueupLineEdit->text());
    QString req;
    if (ui->NomBanqueupLineEdit->text() == "")
        msg = tr("le nom de la banque");
    else if (ui->NomAbregeupLineEdit->text() == "")
        msg = tr("le nom abrégé de la banque");
    if (msg != "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas précisé ") + msg);
        return;
    }

    if (gMode == Nouv)
    {
        foreach (Banque* bq, Datas::I()->banques->banques()->values())
        {
            if (bq->nom().toUpper() == ui->NomBanqueupLineEdit->text().toUpper())
            {
                UpMessageBox::Watch(this,tr("Cette banque est déjà enregistrée!"));
                return;
            }
            if (bq->nomabrege() == ui->NomAbregeupLineEdit->text())
            {
                UpMessageBox::Watch(this,tr("Cette abréviation est déjà utilisée!"));
                ui->NomAbregeupLineEdit->setFocus();
                return;
            }
        }
        Banque *banq = Datas::I()->banques->CreationBanque(ui->NomAbregeupLineEdit->text(),   //! idBanqueAbrege
                                                            nombanque);                        //! NomBanque
        if (banq == Q_NULLPTR)
            return;
        if (gFermeApresValidation)
        {
            UpMessageBox::Watch(this,tr("La banque ") + nombanque + tr(" a été enregistrée"));
            accept();
            return;
        }
    }

    else if (gMode == Modif)
    {
        UpLabel* lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(uptablebanq->currentRow(),1));
        int idBanque = uptablebanq->item(lbl->Row(),0)->text().toInt();
        Banque * bqamodifier = Datas::I()->banques->getById(idBanque);
        foreach (Banque* banq, Datas::I()->banques->banques()->values())
        {
            if (banq != bqamodifier && banq->nom().toUpper() == ui->NomBanqueupLineEdit->text().toUpper())
            {
                msg = tr("il y a déjà un organisme bancaire enregistré avec ce nom");
                UpMessageBox::Watch(this,msg);
                return;
            }
            if (banq != bqamodifier && banq->nomabrege() == ui->NomAbregeupLineEdit->text())
            {
                msg = tr("il y a déjà un organisme bancaire enregistré avec cette abréviation");
                UpMessageBox::Watch(this,msg);
                return;
            }
        }
        ItemsList::update(bqamodifier, CP_NOMABREGE_BANQUES, ui->NomAbregeupLineEdit->text());
        ItemsList::update(bqamodifier, CP_NOMBANQUE_BANQUES, nombanque);
    }
    RemplirTableWidget();
    UpLabel *lbl;
    for (int i=0; i<uptablebanq->rowCount(); i++)
    {
        lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(i,1));
        if (lbl->text() == nombanque)
        {
            uptablebanq->setCurrentCell(i,1);
            break;
        }
    }
    AfficheBanque();
    RemetEnNorm();
}

void dlg_gestionbanques::RemetEnNorm()
{
    gMode = Norm;
    ui->AnnulModifupSmallButton->setVisible(false);
    ui->OKModifupSmallButton->setVisible(false);
    ui->Banqueframe->setEnabled(false);
    uptablebanq->setEnabled(true);
    widgButtons->setEnabled(true);
}

void dlg_gestionbanques::RemplirTableWidget()
{
    QTableWidgetItem    *pitem0;
    UpLabel             *label1;
    int rowcount = Datas::I()->banques->banques()->size();
    int i = 0;
    if (rowcount > 0)
    {
        uptablebanq->setRowCount(rowcount);
        foreach (Banque* bq, Datas::I()->banques->banques()->values())
        {
            pitem0 = new QTableWidgetItem;
            label1 = new UpLabel;
            pitem0->setText(QString::number(bq->id()));
            label1->setText(bq->nom());
            label1->setRow(i);
            uptablebanq->setItem(i,0,pitem0);
            uptablebanq->setCellWidget(i,1,label1);
            uptablebanq->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
            ++i;
        }
    }
}
