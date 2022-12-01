/* (C) 2020 LAINE SERGE
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

    m_fermeapresvalidation   = (nouvbanqueabrege != "");
    setWindowModality(Qt::WindowModal);
    dlglayout()               ->insertWidget(0, ui->Banqueframe);
    if (m_fermeapresvalidation)
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
        wdg_bigtable = new UpTableWidget();
        wdg_bigtable->setFixedWidth(larg);
        wdg_bigtable->resize(larg, haut);
        wdg_bigtable->verticalHeader()->setVisible(false);
        wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
        wdg_bigtable->setSelectionMode(QAbstractItemView::SingleSelection);
        wdg_bigtable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        wdg_bigtable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        wdg_bigtable->setGridStyle(Qt::DotLine);
        wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        wdg_bigtable->setSelectionBehavior(QAbstractItemView::SelectRows);
        wdg_bigtable->setColumnCount(2);
        wdg_bigtable->setColumnHidden(0,true);
        wdg_bigtable->setColumnWidth(1,wdg_bigtable->width()-2);
        wdg_bigtable->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
        wdg_bigtable->setHorizontalHeaderItem(1, new QTableWidgetItem(Icons::icEuro(),"Banques"));
        wdg_bigtable->horizontalHeader()->setVisible(true);
        wdg_bigtable->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
        wdg_bigtable->horizontalHeader()->setIconSize(QSize(25,25));
        wdg_bigtable->horizontalHeader()->setFixedHeight(hautrow);

        wdg_buttonframe = new WidgetButtonFrame(wdg_bigtable);
        wdg_buttonframe->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
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
        dlglayout()               ->insertWidget(0,wdg_buttonframe->widgButtonParent());
        int r,t,l,b;
        dlglayout()               ->getContentsMargins(&r,&t,&l,&b);
        AjouteLayButtons(UpDialog::ButtonClose);
        resize(larg + r + l,
               haut + wdg_buttonframe->height() + ui->Banqueframe->height() + CloseButton->height() + t + b + (dlglayout()->spacing()*2));
        RemplirTableWidget();
        wdg_bigtable->setCurrentCell(0,1);
        AfficheBanque();
        connect(wdg_bigtable,       &UpTableWidget::itemSelectionChanged,   this,   &dlg_gestionbanques::AfficheBanque);
        connect(wdg_buttonframe,    &WidgetButtonFrame::choix,              this,   &dlg_gestionbanques::ChoixButtonFrame);
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
    UpLabel* lbl = static_cast<UpLabel*>(wdg_bigtable->cellWidget(wdg_bigtable->currentRow(),1));
    int idBanque = wdg_bigtable->item(lbl->Row(),0)->text().toInt();
    auto itbq = Datas::I()->banques->banques()->constFind(idBanque);
    if (itbq != Datas::I()->banques->banques()->constEnd())
    {
        Banque *banq = const_cast<Banque*>(itbq.value());
        ui->NomBanqueupLineEdit->setText(banq->nom());
        ui->NomAbregeupLineEdit->setText(banq->nomabrege());
    }
    wdg_buttonframe->wdg_moinsBouton->setEnabled(true);
    for (auto it = Datas::I()->comptes->comptes()->constBegin(); it != Datas::I()->comptes->comptes()->constEnd(); ++it)
    {
        Compte *cpt = const_cast<Compte*>(it.value());
        if (cpt->idBanque() == idBanque)
        {
            wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
            return;
        }
    }
}

void dlg_gestionbanques::AnnuleModifBanque()
{
    if (m_fermeapresvalidation)
        reject();
    else
    {
        RemetEnNorm();
        AfficheBanque();
    }
}

void dlg_gestionbanques::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        NouvBanque();
        break;
    case WidgetButtonFrame::Modifier:
        ModifBanque();
        break;
    case WidgetButtonFrame::Moins:
        SupprBanque();
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
    if (!m_fermeapresvalidation)
    {
        wdg_bigtable->setEnabled(false);
        wdg_buttonframe->setEnabled(false);
    }
    m_mode = Nouv;
}

void dlg_gestionbanques::ModifBanque()
{
    ui->AnnulModifupSmallButton->setVisible(true);
    ui->OKModifupSmallButton->setVisible(true);
    ui->Banqueframe->setEnabled(true);
    wdg_bigtable->setEnabled(false);
    wdg_buttonframe->setEnabled(false);
    m_mode = Modif;
}

void dlg_gestionbanques::SupprBanque()
{
    UpLabel* lbl = static_cast<UpLabel*>(wdg_bigtable->cellWidget(wdg_bigtable->currentRow(),1));
    int idBanque = wdg_bigtable->item(lbl->Row(),0)->text().toInt();
    UpMessageBox msgbox(this);
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
    if (ui->NomBanqueupLineEdit->text() == "")
        msg = tr("le nom de la banque");
    else if (ui->NomAbregeupLineEdit->text() == "")
        msg = tr("le nom abrégé de la banque");
    if (msg != "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas précisé ") + msg);
        return;
    }

    if (m_mode == Nouv)
    {
        for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
        {
            Banque *bq = const_cast<Banque*>(it.value());
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
        if (m_fermeapresvalidation)
        {
            UpMessageBox::Watch(this,tr("La banque ") + nombanque + tr(" a été enregistrée"));
            accept();
            return;
        }
    }

    else if (m_mode == Modif)
    {
        UpLabel* lbl = static_cast<UpLabel*>(wdg_bigtable->cellWidget(wdg_bigtable->currentRow(),1));
        int idBanque = wdg_bigtable->item(lbl->Row(),0)->text().toInt();
        Banque * bqamodifier = Datas::I()->banques->getById(idBanque);
        for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
        {
            Banque *banq = const_cast<Banque*>(it.value());
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
        QHash<QString, QVariant> sets = QHash<QString, QVariant>();
        sets[CP_NOMABREGE_BANQUES] = ui->NomAbregeupLineEdit->text();
        sets[CP_NOMBANQUE_BANQUES] = nombanque;
        Datas::I()->banques->UpdateBanque(bqamodifier, sets);
    }
    RemplirTableWidget();
    UpLabel *lbl;
    for (int i=0; i<wdg_bigtable->rowCount(); i++)
    {
        lbl = static_cast<UpLabel*>(wdg_bigtable->cellWidget(i,1));
        if (lbl->text() == nombanque)
        {
            wdg_bigtable->setCurrentCell(i,1);
            break;
        }
    }
    AfficheBanque();
    RemetEnNorm();
}

void dlg_gestionbanques::RemetEnNorm()
{
    m_mode = Norm;
    ui->AnnulModifupSmallButton->setVisible(false);
    ui->OKModifupSmallButton->setVisible(false);
    ui->Banqueframe->setEnabled(false);
    wdg_bigtable->setEnabled(true);
    wdg_buttonframe->setEnabled(true);
}

void dlg_gestionbanques::RemplirTableWidget()
{
    QTableWidgetItem    *pitem0;
    UpLabel             *label1;
    if (Datas::I()->banques->banques()->size() > 0)
    {
        if (m_model)
            delete m_model;
        m_model = new QStandardItemModel;
        for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
        {
            Banque *bq = const_cast<Banque*>(it.value());
            m_model->appendRow(QList<QStandardItem*>() << new QStandardItem(bq->nom()) << new QStandardItem(QString::number(bq->id())));
        }
        m_model->sort(0);
        wdg_bigtable->setRowCount(m_model->rowCount());
        for (int i=0; i < m_model->rowCount(); ++i)
        {
            pitem0 = new QTableWidgetItem;
            label1 = new UpLabel;
            pitem0->setText(m_model->item(i,1)->text());
            label1->setText(m_model->item(i,0)->text());
            label1->setRow(i);
            wdg_bigtable->setItem(i,0,pitem0);
            wdg_bigtable->setCellWidget(i,1,label1);
            wdg_bigtable->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
        }
    }
}
