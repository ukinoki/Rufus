/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_banque.h"
#include "ui_dlg_banque.h"
#include "icons.h"

dlg_banque::dlg_banque(QSqlDatabase gdb, QWidget *parent, QString nouvbanqueabrege) :
    UpDialog(parent),
    ui(new Ui::dlg_banque)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    db                      = gdb;

    gFermeApresValidation   = (nouvbanqueabrege != "");
    setWindowTitle(tr("Enregistrer une nouvelle banque"));
    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());

    globallay               ->insertWidget(0, ui->Banqueframe);
    if (gFermeApresValidation)
    {
        NouvBanque();
        ui->NomAbregeupLineEdit->setText(nouvbanqueabrege);
        ui->NomAbregeupLineEdit->setFocusPolicy(Qt::NoFocus);
        globallay->setSizeConstraint(QLayout::SetFixedSize);
    }
    else
    {
        int larg    = 350;
        int ncolvis = 12;
        int hautrow = QFontMetrics(qApp->font()).height()*1.3;
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
        globallay               ->insertWidget(0,widgButtons->widgButtonParent());
        int r,t,l,b;
        globallay               ->getContentsMargins(&r,&t,&l,&b);
        AjouteLayButtons(UpDialog::ButtonClose);
        resize(larg + r + l,
               haut + widgButtons->height() + ui->Banqueframe->height() + CloseButton->height() + t + b + (globallay->spacing()*2));
        RemplirTableView();
        uptablebanq->setCurrentCell(0,1);
        AfficheBanque();
        connect(uptablebanq,        &UpTableWidget::itemSelectionChanged,   [=] {AfficheBanque();});
        connect(widgButtons,        &WidgetButtonFrame::choix,              [=] {ChoixButtonFrame(widgButtons->Reponse());});
        connect(CloseButton,        &QPushButton::clicked,                  [=] {accept();});
        ui->AnnulModifupSmallButton ->setVisible(false);
        ui->OKModifupSmallButton    ->setVisible(false);
    }

    connect(ui->AnnulModifupSmallButton,    &QPushButton::clicked,    [=] {AnnuleModifBanque();});
    connect(ui->OKModifupSmallButton,       &QPushButton::clicked,    [=] {ValideModifBanque();});

    QRegExp rx              = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
    ui->NomBanqueupLineEdit ->setValidator(new QRegExpValidator(rx));
    QRegExp val             = QRegExp("[A-Z]*");
    ui->NomAbregeupLineEdit ->setValidator(new QRegExpValidator(val));
}

dlg_banque::~dlg_banque()
{
    delete ui;
}

void dlg_banque::AfficheBanque()
{
    UpLabel* lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(uptablebanq->currentRow(),1));
    int idBanque = uptablebanq->item(lbl->getRow(),0)->text().toInt();
    QString req = "select idBanque, NomBanque, idBanqueAbrege from \n"
                  NOM_TABLE_BANQUES " where  idBanque = " + QString::number(idBanque);
    QSqlQuery quer(req,db);
    TraiteErreurRequete(quer,req,"");
    if (quer.size()>0)
    {
        quer.first();
        ui->NomBanqueupLineEdit->setText(quer.value(1).toString());
        ui->NomAbregeupLineEdit->setText(quer.value(2).toString());
    }
    widgButtons->moinsBouton->setEnabled(true);
    req = "select idBanque from " NOM_TABLE_COMPTES " where idBanque = " + quer.value(0).toString();
    QSqlQuery quer1(req,db);
    if (quer1.size()>0)
        widgButtons->moinsBouton->setEnabled(false);
    lbl = 0;
    delete lbl;
}

void dlg_banque::AnnuleModifBanque()
{
    if (gFermeApresValidation)
        reject();
    else
    {
        RemetEnNorm();
        AfficheBanque();
    }
}

void dlg_banque::ChoixButtonFrame(int i)
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

void dlg_banque::NouvBanque()
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

void dlg_banque::ModifBanque()
{
    ui->AnnulModifupSmallButton->setVisible(true);
    ui->OKModifupSmallButton->setVisible(true);
    ui->Banqueframe->setEnabled(true);
    uptablebanq->setEnabled(false);
    widgButtons->setEnabled(false);
    gMode = Modif;
}

void dlg_banque::SupprBanque()
{
    UpLabel* lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(uptablebanq->currentRow(),1));
    int idBanque = uptablebanq->item(lbl->getRow(),0)->text().toInt();
    UpMessageBox msgbox;
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer"));
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    msgbox.setText(tr("Supprimer la banque ") + lbl->text() + "?");
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != OKBouton)
       return;
    delete OKBouton;
    delete NoBouton;
    if (QSqlQuery("select idcompte from " NOM_TABLE_COMPTES " where idbanque = " + QString::number(idBanque), db).size()>0)
    {
        UpMessageBox::Watch(this, tr("Impossible de supprimer la banque ") + lbl->text(), tr("Elle est utilisée par d'autres utilisateurs"));
        return;
    }
    QString rq = "delete from " NOM_TABLE_BANQUES " where idbanque = " + QString::number(idBanque);
    QSqlQuery quer(rq,db);
    TraiteErreurRequete(quer,rq,"");
    RemplirTableView();
    AfficheBanque();
}

void dlg_banque::ValideModifBanque()
{
    QString msg = "";
    QString nombanque = fMAJPremiereLettre(ui->NomBanqueupLineEdit->text());
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
        for (int i = 0; i<gListBanques.size(); i++)
        {
            if (gListBanques.at(i).toUpper() == ui->NomBanqueupLineEdit->text().toUpper())
            {
                UpMessageBox::Watch(this,tr("Cette banque est déjà enregistrée!"));
                return;
            }
        }
        req = "select idbanqueabrege from " NOM_TABLE_BANQUES;
        QSqlQuery quer1(req,db);
        TraiteErreurRequete(quer1,req,"");
        if (quer1.size()>0)
        {
            quer1.first();
            for (int i=0; i<quer1.size(); i++)
            {
                if (quer1.value(0).toString() == ui->NomAbregeupLineEdit->text())
                {
                    msg = tr("il y a déjà un organisme bancaire enregistré avec cette abréviation");
                    UpMessageBox::Watch(this,msg);
                    return;
                }
                quer1.next();
            }
        }
        req = " select idbanqueabrege from " NOM_TABLE_BANQUES " where idbanqueabrege = " + ui->NomAbregeupLineEdit->text();
        QSqlQuery quer(req,db);
        if(quer.size()>0)
        {
            UpMessageBox::Watch(this,tr("Cette abréviation est déjà utilisée!"));
            ui->NomAbregeupLineEdit->setFocus();
            return;
        }
        req = "insert into " NOM_TABLE_BANQUES " (idbanqueabrege, nombanque) values ("
                "'" + ui->NomAbregeupLineEdit->text() + "', "
                "'" + CorrigeApostrophe(nombanque) + "')";
        QSqlQuery quer0(req,db);
        TraiteErreurRequete(quer0,req,"");
        if (gFermeApresValidation)
        {
            UpMessageBox::Watch(this,tr("La banque ") + nombanque + tr(" a été enregistrée"));
            accept();
        }
    }

    else if (gMode == Modif)
    {
        UpLabel* lbl = static_cast<UpLabel*>(uptablebanq->cellWidget(uptablebanq->currentRow(),1));
        int idBanque = uptablebanq->item(lbl->getRow(),0)->text().toInt();
        req = "select nombanque from " NOM_TABLE_BANQUES " where idbanque <> " + QString::number(idBanque);
        QSqlQuery quer0(req,db);
        TraiteErreurRequete(quer0,req,"");
        if (quer0.size()>0)
        {
            quer0.first();
            for (int i=0; i<quer0.size(); i++)
            {
                if (quer0.value(0).toString().toUpper() == nombanque.toUpper())
                {
                    msg = tr("il y a déjà un organisme bancaire enregistré avec ce nom");
                    UpMessageBox::Watch(this,msg);
                    return;
                }
                quer0.next();
            }
        }
        req = "select idbanqueabrege from " NOM_TABLE_BANQUES " where idbanque <> " + QString::number(idBanque);
        QSqlQuery quer1(req,db);
        TraiteErreurRequete(quer1,req,"");
        if (quer1.size()>0)
        {
            quer1.first();
            for (int i=0; i<quer1.size(); i++)
            {
                if (quer1.value(0).toString() == ui->NomAbregeupLineEdit->text())
                {
                    msg = tr("il y a déjà un organisme bancaire enregistré avec cette abréviation");
                    UpMessageBox::Watch(this,msg);
                    return;
                }
                quer1.next();
            }
        }
        req = "update " NOM_TABLE_BANQUES " set "
              " nombanque = '" + CorrigeApostrophe(nombanque) + "', " +
                " idbanqueabrege = '" + ui->NomAbregeupLineEdit->text() + "'"
                " where idBanque = " + QString::number(idBanque);
        QSqlQuery quer2(req,db);
        TraiteErreurRequete(quer2,req,"");
    }
    RemplirTableView();
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
    lbl = 0;
    delete lbl;
    AfficheBanque();
    RemetEnNorm();
}

void dlg_banque::RemetEnNorm()
{
    gMode = Norm;
    ui->AnnulModifupSmallButton->setVisible(false);
    ui->OKModifupSmallButton->setVisible(false);
    ui->Banqueframe->setEnabled(false);
    uptablebanq->setEnabled(true);
    widgButtons->setEnabled(true);
}

void dlg_banque::RemplirTableView()
{
    QTableWidgetItem    *pitem0;
    UpLabel             *label1;
    QString req = "select idBanque, NomBanque from " NOM_TABLE_BANQUES " order by nomBanque";
    QSqlQuery quer(req,db);
    TraiteErreurRequete(quer,req,"");
    if (quer.size() > 0)
    {
        uptablebanq->setRowCount(quer.size());
        quer.first();
        for (int i=0; i<quer.size(); i++)
        {
            pitem0 = new QTableWidgetItem;
            label1 = new UpLabel;
            pitem0->setText(quer.value(0).toString());
            label1->setText(quer.value(1).toString());
            label1->setRow(i);
            uptablebanq->setItem(i,0,pitem0);
            uptablebanq->setCellWidget(i,1,label1);
            uptablebanq->setRowHeight(i,QFontMetrics(qApp->font()).height()*1.3);
            quer.next();
        }
    }
}

QString dlg_banque::CorrigeApostrophe(QString RechAp)
{
    return RechAp.replace("'","\\'");
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Traite et affiche le signal d'erreur d'une requete -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool dlg_banque::TraiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage)
{
    if (query.lastError().type() != QSqlError::NoError)
    {
        UpMessageBox::Watch(0, ErrorMessage, tr("\nErreur\n") + query.lastError().text() +  tr("\nrequete = ") + requete);
        return true;
    }
    else return false;
}

