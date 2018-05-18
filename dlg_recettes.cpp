/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_recettes.h"

dlg_recettes::dlg_recettes(QDate *DateDebut, QDate *Datefin, Procedures *ProcAPasser, QSqlQuery BilanRec, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionRecettes", parent)
{
    proc        = ProcAPasser;
    Debut       = *DateDebut;
    Fin         = *Datefin;
    gBilan      = BilanRec;
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
    TotalMontantlbl         = new UpLabel();
    TotalReclbl             = new UpLabel();
    DetailReclbl            = new UpLabel();
    QHBoxLayout *hboxsup    = new QHBoxLayout();
    QHBoxLayout *hboxinf    = new QHBoxLayout();
    QVBoxLayout *box        = new QVBoxLayout();
    gSupervcheckBox         = new QCheckBox();
    gSupervbox              = new UpComboBox();

    hboxsup->addWidget(gSupervbox);
    hboxsup->addSpacerItem((new QSpacerItem(60,5,QSizePolicy::Expanding)));
    hboxsup->addWidget(TotalMontantlbl);
    hboxsup->addSpacerItem((new QSpacerItem(130,5,QSizePolicy::Minimum)));
    hboxsup->addWidget(TotalReclbl);
    hboxsup->addSpacerItem((new QSpacerItem(5,5,QSizePolicy::Fixed)));
    hboxsup->setContentsMargins(0,0,0,0);
    hboxsup->setSpacing(0);
    hboxinf->addWidget(gSupervcheckBox);
    hboxinf->addSpacerItem((new QSpacerItem(60,10,QSizePolicy::Expanding)));
    hboxinf->addWidget(DetailReclbl);
    hboxinf->addSpacerItem((new QSpacerItem(5,5,QSizePolicy::Fixed)));
    hboxinf->setContentsMargins(0,0,0,0);
    hboxinf->setSpacing(0);
    box->addLayout(hboxsup);
    box->addLayout(hboxinf);
    box->setContentsMargins(0,0,0,0);
    box->setSpacing(0);


    gSupervcheckBox->setText(tr("Exclure les actes effectués par les remplaçants"));
    gSupervcheckBox->setChecked(false);
    QString Titre;
    if (Debut == Fin)
        Titre = tr("Bilan des recettes pour la journée du ") + Debut.toString(tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des recettes pour la période du ") + Debut.toString(tr("d MMMM yyyy")) + tr(" au ") + Fin.toString(tr("d MMMM yyyy"));
    setWindowTitle(Titre);
    AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonClose);
    if (proc->getListeSuperviseurs()->rowCount()>1)
        gSupervbox  ->addItem(tr("Tout le monde"),-1);
    for (int i=0; i<proc->getListeSuperviseurs()->rowCount(); i++)
        gSupervbox->addItem(proc->getListeSuperviseurs()->item(i,1)->text(), proc->getListeSuperviseurs()->item(i,0)->text());

    DefinitArchitetureTable();
    RemplitLaTable();
    Slot_FiltreTable();
    globallay->insertLayout(0,box);
    globallay->insertWidget(0,gBigTable);
    globallay->setSizeConstraint(QLayout::SetFixedSize);
    connect(CloseButton,        SIGNAL(clicked()),                  this,       SLOT(accept()));
    connect(PrintButton,        SIGNAL(clicked()),                  this,       SLOT(Slot_ImprimeEtat()));
    connect(gSupervbox,         SIGNAL(currentIndexChanged(int)),   this,       SLOT(Slot_FiltreTable()));
    connect(gSupervcheckBox,    SIGNAL(clicked(bool)),              this,       SLOT(Slot_FiltreTable()));
}

dlg_recettes::~dlg_recettes()
{
}

void dlg_recettes::Slot_FiltreTable()
{
    if (sender()==gSupervbox)
        gSupervcheckBox->setChecked(false);
    int idx         = gSupervbox->currentData().toInt();
    if (idx==-1)
    {
        for(int i=0; i<gBigTable->rowCount(); i++)
            gBigTable->setRowHidden(i,false);
        gSupervcheckBox->setVisible(false);
    }
    else
    {
        if (proc->getListeParents()->findItems(QString::number(idx)).size()>0)
        {
            gSupervcheckBox->setVisible(true);
            if (gSupervcheckBox->isChecked())
                for(int i=0; i<gBigTable->rowCount(); i++)
                    gBigTable->setRowHidden(i,gBigTable->item(i,8)->text() != QString::number(idx));
            else
                for(int i=0; i<gBigTable->rowCount(); i++)
                    gBigTable->setRowHidden(i,gBigTable->item(i,9)->text() != QString::number(idx));

        }
        else
        {
            gSupervcheckBox->setVisible(false);
            for(int i=0; i<gBigTable->rowCount(); i++)
                gBigTable->setRowHidden(i,gBigTable->item(i,8)->text() != QString::number(idx));
        }
    }
    PrintButton->setEnabled(gSupervbox->currentData().toInt()>-1  && gBigTable->rowNoHiddenCount()>0);
    CalculeTotal();
}

void dlg_recettes::Slot_ImprimeEtat()
{
    QString             Entete, Pied;
    bool AvecDupli   = false;
    bool AvecPrevisu = true;//proc->ApercuAvantImpression();
    bool AvecNumPage = false;

    //création de l'entête
    if (!proc->setDataOtherUser(gSupervbox->currentData().toInt()).value("Success").toBool())
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête") , tr("Annulation de l'impression"));
        return;
    }
    Entete = proc->ImpressionEntete(QDate::currentDate()).value("Norm");
    if (Entete == "") return;

    Entete.replace("{{PRENOM PATIENT}}"    , (gSupervbox->currentData().toInt()>0? proc->getLogin(gSupervbox->currentData().toInt()): tr("Bilan global")));
    Entete.replace("{{NOM PATIENT}}"       , "");
    Entete.replace("{{TITRE1}}"            , windowTitle());
    Entete.replace("{{TITRE}}"             , "");
    Entete.replace("{{DDN}}"               , TotalMontantlbl->text());

    // création du pied
    Pied = proc->ImpressionPied();
    if (Pied == "") return;

    // creation du corps de la remise
    QTextEdit *Etat_textEdit = new QTextEdit;
    QString test4 = "<html><head><style type=\"text/css\">p.p1 {font:70px; margin: 0px 0px 10px 100px;}"
                    "</style></head>"
                    "<body LANG=\"fr-FR\" DIR=\"LTR\">"
                    "<table width=\"510\" border=\"1\"  cellspacing=\"0\" cellpadding=\"2\">";
    int row = 1;
    for (int i = 0; i < gBigTable->rowCount();i++)
    {
        if (!gBigTable->isRowHidden(i) && gBigTable->item(i,3)->text()!= "")
        {
            test4 += "<tr>"
                    "<td width = \"30\"><span style=\"font-size:8pt\">" + QString::number(row) + "</span></td>"
                    "<td width=\"180\" ><span style=\"font-size:8pt\">" + gBigTable->item(i,2)->text() + "</span></td>"
                    "<td width=\"160\" ><span style=\"font-size:8pt\">" + gBigTable->item(i,3)->text() + "</span></td>"
                    "<td width=\"95\" ><span style=\"font-size:8pt\"><div align=\"right\">" + gBigTable->item(i,4)->text() + "</div></span></td>"
                    "</tr>";
            row++;
        }
    }
    test4 += "</table></body></html>";

    Etat_textEdit->setHtml(test4);

    proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
    delete Etat_textEdit;
}

void dlg_recettes::CalculeTotal()
{
    double TotalMontant = 0;
    double TotalRecu    = 0;
    double TotalEsp     = 0;
    double TotalBanq    = 0;
    int    nbreActes    = 0;
    if (gBigTable->rowCount() > 0)
    {
        for (int k = 0; k < gBigTable->rowCount(); k++)
        {
            if (!gBigTable->isRowHidden(k))
            {
                if(gBigTable->item(k,3)->text()!= "")
                {
                    TotalMontant    += QLocale().toDouble(gBigTable->item(k,4)->text());
                    nbreActes++;
                }
                TotalRecu               += QLocale().toDouble(gBigTable->item(k,6)->text());
                if(gBigTable->item(k,5)->text()  == tr("Espèces"))
                        TotalEsp        += QLocale().toDouble(gBigTable->item(k,6)->text());
                else    TotalBanq       += QLocale().toDouble(gBigTable->item(k,6)->text());
            }
        }
        TotalMontantlbl ->setText(tr("Total ") + QString::number(nbreActes) + (nbreActes>1? tr(" actes ") : tr(" acte ")) + QLocale().toString(TotalMontant,'f',2));
        TotalReclbl     ->setText(tr("Total reçu ") + QLocale().toString(TotalRecu,'f',2));
        DetailReclbl    ->setText("(" + tr("Espèces") + ": " + QLocale().toString(TotalEsp,'f',2) + " - "
                                + tr("Banque")  + ": " + QLocale().toString(TotalBanq,'f',2) + ")");
        TotalMontantlbl ->setAlignment(Qt::AlignRight);
        TotalReclbl     ->setAlignment(Qt::AlignRight);
        DetailReclbl    ->setAlignment(Qt::AlignRight);
    }
}

void dlg_recettes::DefinitArchitetureTable()
{
    gBigTable = new UpTableWidget();

    int ColCount = 10;
    gBigTable->setPalette(QPalette(Qt::white));
    gBigTable->setEditTriggers(QAbstractItemView::AnyKeyPressed
                               |QAbstractItemView::DoubleClicked
                               |QAbstractItemView::EditKeyPressed
                               |QAbstractItemView::SelectedClicked);
    gBigTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gBigTable->verticalHeader()->setVisible(false);
    gBigTable->setFocusPolicy(Qt::StrongFocus);
    gBigTable->setColumnCount(ColCount);
    gBigTable->setSelectionMode(QAbstractItemView::SingleSelection);

    QStringList LabelARemplir;
    LabelARemplir << "";
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Nom");
    LabelARemplir << tr("Type acte");
    LabelARemplir << tr("Montant");
    LabelARemplir << tr("Mode de paiement");
    LabelARemplir << tr("Reçu");
    LabelARemplir << tr("Recettes spéciales");
    LabelARemplir << "";
    LabelARemplir << "";

    gBigTable->setHorizontalHeaderLabels(LabelARemplir);
    gBigTable->horizontalHeader()->setVisible(true);
    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    gBigTable->setColumnWidth(li,0);                                                // idActe
    li++;
    gBigTable->setColumnWidth(li,100);                                              // Date affichage européen
    li++;
    gBigTable->setColumnWidth(li,240);                                              // Nom
    li++;
    gBigTable->setColumnWidth(li,140);                                              // TypeActe
    li++;
    gBigTable->setColumnWidth(li,85);                                               // Montant
    li++;
    gBigTable->setColumnWidth(li,130);                                              // Mode de paiement
    li++;
    gBigTable->setColumnWidth(li,85);                                               // Reçu
    li++;
    gBigTable->setColumnWidth(li,80);                                               // Recettes spéciales
    li++;
    gBigTable->setColumnWidth(li,0);                                                // idsuperviseur
    li++;
    gBigTable->setColumnWidth(li,0);                                                // idparent

    gBigTable->setColumnHidden(0,true);
    gBigTable->setColumnHidden(ColCount-3,true);
    gBigTable->setColumnHidden(ColCount-2,true);
    gBigTable->setColumnHidden(ColCount-1,true);
    gBigTable->FixLargeurTotale();

    gBigTable->setGridStyle(Qt::SolidLine);
    gBigTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void dlg_recettes::RemplitLaTable()
{
    QTableWidgetItem    *pItem0,*pItem1,*pItem2,*pItem3,*pItem4,*pItem5,*pItem6,*pItem7, *pItem8, *pItem9;
    QString             A;
    int hauteurrow      = QFontMetrics(qApp->font()).height()*1.3;

    gBigTable->setRowCount(gBilan.size());

    gBilan.first();
    for (int i = 0; i < gBilan.size(); i++)
    {
        int col = 0;

        A = gBilan.value(0).toString();                                                             // idActe - col = 0
        pItem0 = new QTableWidgetItem();
        pItem0->setText(A);
        gBigTable->setItem(i,col,pItem0);
        col++;

        A = gBilan.value(1).toDate().toString(tr("d MMM yyyy"));                                     // Date - col = 1
        pItem1 = new QTableWidgetItem();
        pItem1->setText(A);
        pItem1->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem1);
        col++;

        pItem2 = new QTableWidgetItem();
        pItem2->setText(gBilan.value(2).toString());                                                // NomPrenom - col = 2
        gBigTable->setItem(i,col,pItem2);
        col++;

        pItem3 = new QTableWidgetItem();
        pItem3->setText(gBilan.value(3).toString());                                                // Cotation - col = 3;
        gBigTable->setItem(i,col,pItem3);
        col++;

        if (gBilan.value(5).toString() == "F")
            A = QLocale().toString(gBilan.value(4).toDouble()/6.55957,'f',2);// Montant en F converti en euros
        else
            A = QLocale().toString(gBilan.value(4).toDouble(),'f',2);                               // Montant - col = 4
        pItem4 = new QTableWidgetItem();
        pItem4->setText(A);
        pItem4->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem4);
        col++;

        A = gBilan.value(6).toString();                                                             // Mode de paiement - col = 5
        QString B = A;
        if (A == "T")
            B = gBilan.value(7).toString();
        if (B == "CB")
            B = tr("Carte Bancaire");
        else if (B == "E")
            B = tr("Espèces");
        else if (B== "C")
            B = tr("Chèque");
        else if (B == "G")
            B = tr("Gratuit");
        else if (B == "I")
            B = tr("Impayé");
        else if(B == "V")
            B = tr("Virement");
        pItem5 = new QTableWidgetItem();
        pItem5->setText(B);
        gBigTable->setItem(i,col,pItem5);
        col++;


        double C = gBilan.value(8).toDouble();                                                      // Reçu- col = 6
        pItem6 = new QTableWidgetItem();
        pItem6->setText(QLocale().toString(C,'f',2));
        pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem6);
        col++;

        A = gBilan.value(1).toDate().toString("yyyy-MM-dd");                                        // Recettes spéciales - col = 7
        pItem7 = new QTableWidgetItem() ;
        pItem7->setText(A);
        gBigTable->setItem(i,col,pItem7);
        col++;

        pItem8 = new QTableWidgetItem();
        pItem8->setText(gBilan.value(9).toString());                                                // iduser - col = 8;
        gBigTable->setItem(i,col,pItem8);
        col++;

        pItem9 = new QTableWidgetItem();
        pItem9->setText(gBilan.value(10).toString());                                               // idparent - col = 9;
        gBigTable->setItem(i,col,pItem9);

        gBigTable->setRowHeight(i,hauteurrow);

        gBilan.next();
    }
    int nbrowsAAfficher = 30;
    gBigTable->setFixedHeight(nbrowsAAfficher*hauteurrow+2);
}
