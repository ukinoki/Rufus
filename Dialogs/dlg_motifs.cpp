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

#include "dlg_motifs.h"
#include "ui_dlg_motifs.h"

dlg_motifs::dlg_motifs(QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionMotifs", parent),
    ui(new Ui::dlg_motifs)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_motifs = Datas::I()->motifs->motifs();

    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());

    widgButtons             = new WidgetButtonFrame(ui->MotifsupTableWidget);
    widgButtons             ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::MoinsButton);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    OKButton                ->setText(tr("Enregistrer\nles modifications"));
    CancelButton            ->setText(tr("Annuler"));
    setStageCount(1);

    ui->MotifsupTableWidget ->setPalette(QPalette(Qt::white));
    ui->MotifsupTableWidget ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->MotifsupTableWidget ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->MotifsupTableWidget ->verticalHeader()->setVisible(false);
    ui->MotifsupTableWidget ->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->MotifsupTableWidget ->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MotifsupTableWidget ->setAcceptDrops(true);
    ui->MotifsupTableWidget ->setDragEnabled(true);
    ui->MotifsupTableWidget ->setDragDropOverwriteMode(false);
    ui->MotifsupTableWidget ->setDragDropMode(QAbstractItemView::DragDrop);
    ui->MotifsupTableWidget ->setDefaultDropAction(Qt::IgnoreAction);
    ui->MotifsupTableWidget ->setColumnCount(9);
    ui->MotifsupTableWidget ->setColumnWidth(0,30);      // checkbox Utiliser
    ui->MotifsupTableWidget ->setColumnWidth(1,179);     // Motif
    ui->MotifsupTableWidget ->setColumnHidden(2,true);   // colonne Raccourci -  masquee
    ui->MotifsupTableWidget ->setColumnWidth(3,0);       // idMotifsRDV - pas masquee pour pouvoir lire l'id au moment du drag
    ui->MotifsupTableWidget ->setColumnHidden(4,true);   // Couleur - masquee
    ui->MotifsupTableWidget ->setColumnHidden(5,true);   // Duree - masquee
    ui->MotifsupTableWidget ->setColumnWidth(6,80);      // Par Defaut
    ui->MotifsupTableWidget ->setColumnWidth(7,20);      // Couleur
    ui->MotifsupTableWidget ->setColumnWidth(8,0);       // NoOrdre
    ui->MotifsupTableWidget ->setStyleSheet("QTableView {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");
    ui->MotifsupTableWidget ->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->MotifsupTableWidget ->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Motifs")));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(4, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(6, new QTableWidgetItem(tr("Par Defaut")));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(7, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->setHorizontalHeaderItem(8, new QTableWidgetItem(""));
    ui->MotifsupTableWidget ->horizontalHeader()->setVisible(true);
    ui->MotifsupTableWidget ->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->MotifsupTableWidget ->setGridStyle(Qt::DotLine);
    ui->MotifsupTableWidget ->FixLargeurTotale();

    QVBoxLayout *vlay       = new QVBoxLayout;
    vlay                    ->setContentsMargins(0,0,0,0);
    vlay                    ->setSpacing(0);
    vlay                    ->addWidget(ui->Detailsframe);
    vlay                    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout *hlay       = new QHBoxLayout;
    hlay                    ->setContentsMargins(0,0,0,0);
    hlay                    ->setSpacing(5);
    hlay                    ->insertWidget(0,widgButtons->widgButtonParent());
    hlay                    ->addLayout(vlay);

    globallay               ->insertLayout(0,hlay);
    int r,t,l,b;
    globallay               ->getContentsMargins(&r,&t,&l,&b);
    setFixedWidth(ui->MotifsupTableWidget->width() + hlay->spacing() + ui->Detailsframe->width() + r + l);

    ui->MotifupLineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->RaccourciupLineEdit ->setValidator(new QRegExpValidator(Utils::rgx_rx,this));

    RemplirTableWidget();
    ui->MotifsupTableWidget ->selectRow(0);
    Slot_ActualiseDetails();

    connect(OKButton,                                   SIGNAL(clicked()),              this,           SLOT (Slot_EnregistreMotifs()));
    connect(CancelButton,                               SIGNAL(clicked()),              this,           SLOT (reject()));
    connect(ui->MotifsupTableWidget,                    SIGNAL(itemSelectionChanged()), this,           SLOT(Slot_ActualiseDetails()));
    connect(ui->CouleurpushButton,                      SIGNAL(clicked(bool)),          this,           SLOT(Slot_ModifCouleur()));
    connect(ui->DefautupCheckBox,                       SIGNAL(clicked(bool)),          this,           SLOT(Slot_ModifPD()));
    connect(ui->UtiliserupCheckBox,                     SIGNAL(clicked(bool)),          this,           SLOT(Slot_ModifUtil()));
    connect(ui->MotifupLineEdit,                        SIGNAL(textEdited(QString)),    this,           SLOT(Slot_ModifMotif(QString)));
    connect(ui->RaccourciupLineEdit,                    SIGNAL(textEdited(QString)),    this,           SLOT(Slot_ModifRaccouci(QString)));
    connect(widgButtons,                                SIGNAL(choix(int)),             this,           SLOT(Slot_ChoixButtonFrame(int)));
    connect(ui->MotifsupTableWidget,                    SIGNAL(dropsignal(QByteArray)), this,           SLOT(Slot_DropMotif(QByteArray)));
}

dlg_motifs::~dlg_motifs()
{
    delete ui;
}

void dlg_motifs::Slot_ActualiseDetails()
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    Motif *mtf = getMotifFromRow(row);

    ui->MotifupLineEdit->setText(mtf->motif());
    ui->RaccourciupLineEdit->setText(mtf->raccourci());
    QString background = "background:#" + mtf->couleur();
    ui->MotifupLineEdit->setStyleSheet(background);
    bool checkUt    = false;
    bool checkUtTog = false;
    if (UpchkFromTableW(ui->MotifsupTableWidget,row,0)!=Q_NULLPTR)
    {
        checkUt     = UpchkFromTableW(ui->MotifsupTableWidget,row,0)->isChecked();
        checkUtTog  = UpchkFromTableW(ui->MotifsupTableWidget,row,0)->Toggleable();
    }
    bool checkPD    = false;
    bool checkPDTog = false;
    if (UpchkFromTableW(ui->MotifsupTableWidget,row,6)!=Q_NULLPTR)
    {
        checkPD     = UpchkFromTableW(ui->MotifsupTableWidget,row,6)->isChecked();
        checkPDTog  = UpchkFromTableW(ui->MotifsupTableWidget,row,6)->Toggleable();
    }
    ui->UtiliserupCheckBox  ->setChecked(checkUt);
    ui->UtiliserupCheckBox  ->setToggleable(checkUtTog);
    ui->DefautupCheckBox    ->setChecked(checkPD);
    ui->DefautupCheckBox    ->setToggleable(checkPDTog);
}

void dlg_motifs::Slot_ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        CreeMotif();
        break;
    case -1:
        SupprimMotif();
        break;
    default:
        break;
    }
}

void dlg_motifs::Slot_DropMotif(QByteArray data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    int id = 0;
    int anc = 0;
    int nouv = 0;
    bool a = true;
    while (!stream.atEnd() && a)
    {
        int                 row, col;
        QMap<int,QVariant>  roleDataMap;
        stream >> row >> col >> roleDataMap;
        if (col==3)
            id = roleDataMap[0].toInt();
        if (col==8)
            anc = roleDataMap[0].toInt();
    }
    QPoint pos = ui->MotifsupTableWidget->viewport()->mapFromGlobal(cursor().pos());
    int r = ui->MotifsupTableWidget->rowAt(pos.y());
    //qDebug() <<  "y = " + QString::number(pos.y()) + " - row = " + QString::number(r) + " - idmotif = " + QString::number(id);
    nouv = r+1;
    if (nouv==0)
    {
        nouv = ui->MotifsupTableWidget->rowCount()+1;
        if (anc==ui->MotifsupTableWidget->rowCount())
            return;
    }
    if (anc != nouv)
        DeplaceVersRow(id, anc, nouv);
}

void dlg_motifs::DeplaceVersRow(int id, int anc, int nouv)
{
    //qDebug() <<  "id = " + QString::number(id) + " - anc = " + QString::number(anc) + " - nouv = " + QString::number(nouv);
    int row                 = anc  -1;
    int Rrow                = nouv -1;
    bool UtilCheckSender    = false;
    bool PDCheckSender      = false;
    QString MotifSender(""), RaccourciSender(""), idSender(""), CouleurSender(""), DureeSender("");

    if (UpchkFromTableW(ui->MotifsupTableWidget,row,0) != Q_NULLPTR)
        UtilCheckSender = UpchkFromTableW(ui->MotifsupTableWidget,row,0)->isChecked();
    else return;
    if (UpchkFromTableW(ui->MotifsupTableWidget,row,6) != Q_NULLPTR)
        PDCheckSender = UpchkFromTableW(ui->MotifsupTableWidget,row,6)->isChecked();
    else return;

    int rowtodel      = UpchkFromTableW(ui->MotifsupTableWidget,row,0)->getRowTable();
    MotifSender       = ui->MotifsupTableWidget->item(row,1)->text();
    RaccourciSender   = ui->MotifsupTableWidget->item(row,2)->text();
    idSender          = ui->MotifsupTableWidget->item(row,3)->text();
    CouleurSender     = ui->MotifsupTableWidget->item(row,4)->text();
    DureeSender       = ui->MotifsupTableWidget->item(row,5)->text();

    ui->MotifsupTableWidget->insertRow(Rrow);

    int col = 0;                                                                            //0 - Utiliser
    QWidget *w         = new QWidget(ui->MotifsupTableWidget);
    UpCheckBox *Check  = new UpCheckBox(w);
    QHBoxLayout *l     = new QHBoxLayout();
    Check->setChecked(UtilCheckSender);
    Check->setRowTable(Rrow);
    Check->setFocusPolicy(Qt::NoFocus);
    connect(Check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_Utiliser(bool)));
    l->setAlignment(Qt::AlignCenter);
    l->addWidget(Check);
    l->setContentsMargins(0,0,0,0);
    w->setLayout(l);
    ui->MotifsupTableWidget->setCellWidget(Rrow,col,w);

    col++;                                                                                  //1 - Motif
    QTableWidgetItem *pItem0 = new QTableWidgetItem();
    pItem0->setText(MotifSender);
    ui->MotifsupTableWidget->setItem(Rrow,col,pItem0);

    col++;                                                                                  //2 - Raccourci
    QTableWidgetItem *pItem1 = new QTableWidgetItem();
    pItem1->setText(RaccourciSender);
    ui->MotifsupTableWidget->setItem(Rrow,col,pItem1);

    col++;                                                                                  //3 - idMotifsRDV
    QTableWidgetItem *pItem2 = new QTableWidgetItem();
    pItem2->setText(QString::number(id));
    ui->MotifsupTableWidget->setItem(Rrow,col,pItem2);

    col++;                                                                                  //4 - Couleur
    QTableWidgetItem *pItem3 = new QTableWidgetItem();
    pItem3->setText(CouleurSender);
    ui->MotifsupTableWidget->setItem(Rrow,col,pItem3);

    col++;                                                                                  //5 - Duree
    QTableWidgetItem *pItem4 = new QTableWidgetItem();
    pItem4->setText(DureeSender);
    ui->MotifsupTableWidget->setItem(Rrow,col,pItem4);

    col++;                                                                                  //6 - ParDefaut
    QWidget *w0         = new QWidget(ui->MotifsupTableWidget);
    UpCheckBox *Check0  = new UpCheckBox(w0);
    QHBoxLayout *l0     = new QHBoxLayout();
    Check0->setChecked(PDCheckSender);
    Check0->setRowTable(Rrow);
    Check0->setFocusPolicy(Qt::NoFocus);
    connect(Check0, SIGNAL(clicked(bool)), this, SLOT(Slot_ParDefaut()));
    l0->setAlignment( Qt::AlignCenter );
    l0->addWidget(Check0);
    l0->setContentsMargins(0,0,0,0);
    w0->setLayout(l0);
    ui->MotifsupTableWidget->setCellWidget(Rrow,col,w0);

    col++;                                                                                  //7 - Couleur
    QWidget *w1         = new QWidget(ui->MotifsupTableWidget);
    UpLabel *Lbl1       = new UpLabel(w1);
    QHBoxLayout *l1     = new QHBoxLayout();
    QString background = "background:#" + CouleurSender;
    Lbl1->setStyleSheet(background);
    Lbl1->setRow(Rrow);
    l1->addWidget(Lbl1);
    l1->setContentsMargins(0,0,0,0);
    w1->setLayout(l1);
    ui->MotifsupTableWidget->setCellWidget(Rrow,col,w1);

    col++;                                                                                  //8 - NoOrdre
    QTableWidgetItem *pItem5 = new QTableWidgetItem();
    pItem5->setText(QString::number(nouv));
    ui->MotifsupTableWidget->setItem(Rrow,col,pItem5);

    QFontMetrics fm(qApp->font());
    ui->MotifsupTableWidget->setRowHeight(Rrow,int(fm.height()*1.3));
    for (int i=0; i<ui->MotifsupTableWidget->rowCount(); i++)
        if (UpchkFromTableW(ui->MotifsupTableWidget,i,0)->getRowTable() == rowtodel)
        {
            ui->MotifsupTableWidget->removeRow(i);
            break;
        }
    ui->MotifsupTableWidget->clearSelection();
    for (int i=0; i<ui->MotifsupTableWidget->rowCount(); i++)
        if (ui->MotifsupTableWidget->item(i,3)->text() == idSender)
        {
            ui->MotifsupTableWidget->selectRow(i);
            break;
        }
    Slot_ActualiseDetails();
    RecalculeLesRows();
    OKButton->setEnabled(true);
}

void dlg_motifs::RecalculeLesRows()
{
    for (int i=0; i<ui->MotifsupTableWidget->rowCount(); i++)
    {
        UpchkFromTableW(ui->MotifsupTableWidget,i,0)->setRowTable(i);
        UpchkFromTableW(ui->MotifsupTableWidget,i,6)->setRowTable(i);
        QWidget *w = dynamic_cast<QWidget*>(ui->MotifsupTableWidget->cellWidget(i,7));
        if (w)
        {
            QList<UpLabel*> listlbl = w->findChildren<UpLabel*>();
            if (listlbl.size()>0)
                listlbl.at(0)->setRow(i);
        }
        ui->MotifsupTableWidget->item(i,8)->setText(QString::number(i+1));
    }
}

void dlg_motifs::Slot_ModifMotif(QString txt)
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    ui->MotifsupTableWidget->item(row,1)->setText(txt);
    OKButton->setEnabled(true);
}

void dlg_motifs::Slot_ModifRaccouci(QString txt)
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    ui->MotifsupTableWidget->item(row,2)->setText(txt);
    OKButton->setEnabled(true);
}

void dlg_motifs::Slot_ModifCouleur()
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    QString couleurenreg = ui->MotifsupTableWidget->item(row,4)->text();
    QColor colordep = QColor("#FF" + couleurenreg);
    QColorDialog *dlg = new QColorDialog(colordep, this);
    dlg->exec();

    QColor colorfin = dlg->selectedColor();
    delete dlg;
    if (!colorfin.isValid())
        return;
    QString couleur = colorfin.name();
    QString background = "background:" + couleur;
    ui->MotifupLineEdit->setStyleSheet(background);
    QWidget *w = dynamic_cast<QWidget*>(ui->MotifsupTableWidget->cellWidget(row,7));
    if (w)
    {
        QList<UpLabel*> listlbl = w->findChildren<UpLabel*>();
        if (listlbl.size()>0)
            listlbl.at(0)->setStyleSheet(background);
    }
    couleur = couleur.replace("#","");
    ui->MotifsupTableWidget->item(row,4)->setText(couleur);
    OKButton->setEnabled(true);
}

void dlg_motifs::Slot_ModifPD()
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    for (int i=0; i< ui->MotifsupTableWidget->rowCount(); i++)
    {
        if (UpchkFromTableW(ui->MotifsupTableWidget,i,6)!=Q_NULLPTR)
        {
            UpchkFromTableW(ui->MotifsupTableWidget,i,6)->setToggleable(i!=row);
            UpchkFromTableW(ui->MotifsupTableWidget,i,6)->setChecked(i==row);
            if (UpchkFromTableW(ui->MotifsupTableWidget,i,0)!=Q_NULLPTR)
            {
                UpchkFromTableW(ui->MotifsupTableWidget,i,0)->setToggleable(i!=row);
                if (i==row)
                    UpchkFromTableW(ui->MotifsupTableWidget,i,0)->setChecked(true);
            }
        }
    }
    ui->DefautupCheckBox->setToggleable(false);
    ui->UtiliserupCheckBox->setChecked(true);
    ui->UtiliserupCheckBox->setToggleable(false);
    OKButton->setEnabled(true);
}

void dlg_motifs::Slot_ModifUtil()
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    if (UpchkFromTableW(ui->MotifsupTableWidget,row,0)!=Q_NULLPTR)
        UpchkFromTableW(ui->MotifsupTableWidget,row,0)->setChecked(ui->UtiliserupCheckBox->isChecked());
    OKButton->setEnabled(true);
}

void dlg_motifs::Slot_ParDefaut()
{
    UpCheckBox *check = dynamic_cast<UpCheckBox*>(sender());
    if (!check)
        return;
    int row = check->getRowTable();
    for (int i=0; i< ui->MotifsupTableWidget->rowCount(); i++)
    {
        if (UpchkFromTableW(ui->MotifsupTableWidget,i,6)!=Q_NULLPTR)
        {
            UpchkFromTableW(ui->MotifsupTableWidget,i,6)->setToggleable(i!=row);
            UpchkFromTableW(ui->MotifsupTableWidget,i,6)->setChecked(i==row);
            if (UpchkFromTableW(ui->MotifsupTableWidget,i,0)!=Q_NULLPTR)
            {
                UpchkFromTableW(ui->MotifsupTableWidget,i,0)->setToggleable(i!=row);
                if (i==row)
                    UpchkFromTableW(ui->MotifsupTableWidget,i,0)->setChecked(true);
            }
        }
    }
    check->setToggleable(false);
    ui->DefautupCheckBox->setChecked(true);
    ui->DefautupCheckBox->setToggleable(false);

    ui->UtiliserupCheckBox->setChecked(true);
    ui->UtiliserupCheckBox->setToggleable(false);
    OKButton->setEnabled(true);
    Slot_ActualiseDetails();
}

void dlg_motifs::Slot_Utiliser(bool a)
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    UpCheckBox *chk = dynamic_cast<UpCheckBox*>(sender());
    if (!chk)
        return;
    if (row == chk->getRowTable())
        ui->UtiliserupCheckBox->setChecked(a);
    OKButton->setEnabled(true);
}

void dlg_motifs::SupprimMotif()
{
    if (ui->MotifsupTableWidget->selectedRanges().size()==0)
        return;
    int row = ui->MotifsupTableWidget->selectedRanges().at(0).topRow();
    int rowdest=0;
    bool PD = false;
    if (row == ui->MotifsupTableWidget->rowCount()+1)
        rowdest = row - 1;
    else
        rowdest = row;
    if (UpchkFromTableW(ui->MotifsupTableWidget,row,6)!=Q_NULLPTR)
        PD = UpchkFromTableW(ui->MotifsupTableWidget,row,6)->isChecked();

    ui->MotifsupTableWidget->removeRow(row);

    if (UpchkFromTableW(ui->MotifsupTableWidget,rowdest,6)!=Q_NULLPTR)
        if (PD)
        {
            UpchkFromTableW(ui->MotifsupTableWidget,rowdest,6)->setChecked(true);
            UpchkFromTableW(ui->MotifsupTableWidget,rowdest,6)->setToggleable(false);
            if (UpchkFromTableW(ui->MotifsupTableWidget,rowdest,0)!=Q_NULLPTR)
            {
                UpchkFromTableW(ui->MotifsupTableWidget,rowdest,0)->setChecked(true);
                UpchkFromTableW(ui->MotifsupTableWidget,rowdest,0)->setToggleable(false);
            }
        }
    for (int i=0; i<ui->MotifsupTableWidget->rowCount(); i++)
    {
        if (UpchkFromTableW(ui->MotifsupTableWidget,i,0)!=Q_NULLPTR)
            UpchkFromTableW(ui->MotifsupTableWidget,i,0)->setRowTable(i);
        if (UpchkFromTableW(ui->MotifsupTableWidget,i,6)!=Q_NULLPTR)
            UpchkFromTableW(ui->MotifsupTableWidget,i,6)->setRowTable(i);
    }

    ui->MotifsupTableWidget->selectRow(rowdest);
    Slot_ActualiseDetails(); // setrangeselected ne déclenche pas le slot
    OKButton->setEnabled(true);
}

void dlg_motifs::CreeMotif()
{
    ui->MotifsupTableWidget->clearSelection();

    int row = ui->MotifsupTableWidget->rowCount();
    ui->MotifsupTableWidget->insertRow(row);
    QJsonObject jmotif{};
    jmotif["id"] = 0;
    jmotif["motif"] = tr("Nouveau motif");
    jmotif["raccourci"] = "NM";
    jmotif["couleur"] = "FFFFFF";
    jmotif["duree"] = 0;
    jmotif["pardefaut"] = false;
    jmotif["utiliser"] = true;
    jmotif["noordre"] = row+1;
    Motif *motif = new Motif(jmotif);
    Datas::I()->motifs->addMotif(motif);
    SetMotifToRow(motif,row);

    QFontMetrics fm(qApp->font());
    ui->MotifsupTableWidget->setRowHeight(row,int(fm.height()*1.3));
    ui->MotifsupTableWidget->selectRow(row);
    OKButton->setEnabled(true);
}

void dlg_motifs::MAJMotifs()
{
    Datas::I()->motifs->clearAll();
    QList<Motif*> listMotifs = DataBase::getInstance()->loadMotifs();
    for(QList<Motif*>::const_iterator itmtf = listMotifs.constBegin(); itmtf != listMotifs.constEnd(); ++itmtf )
    {
        Motif *mt = const_cast<Motif*>(*itmtf);
        Datas::I()->motifs->addMotif( mt );
    }
}

void dlg_motifs::Slot_EnregistreMotifs()
{
    //verifier la cohérence
    for (int i =0; i<ui->MotifsupTableWidget->rowCount(); i++)
    {
        if (ui->MotifsupTableWidget->item(i,1)->text()=="" || ui->MotifsupTableWidget->item(i,2)->text()=="")
        {
            ui->MotifsupTableWidget->selectRow(i);
            UpMessageBox::Watch(this,tr("Renseignements incomplets pour ce motif"));
            return;
        }
    }
    //vider la table MotifsRDV
    DataBase::getInstance()->StandardSQL("delete from " NOM_TABLE_MOTIFSRDV);
    //la remplir avec les nouvelles valeurs
    QString req = "insert into " NOM_TABLE_MOTIFSRDV " (Utiliser, Motif, raccourci, couleur, ParDefaut, NoOrdre) Values\n";
    for (int j=0; j<ui->MotifsupTableWidget->rowCount(); j++)
    {
        if (j>0)
            req += ",\n";
        req += "(";
        QString a = "NULL";
        if (UpchkFromTableW(ui->MotifsupTableWidget,j,0)!=Q_NULLPTR)
            if (UpchkFromTableW(ui->MotifsupTableWidget,j,0)->isChecked())
                a = "1";
        req += a + ",";
        req += "'" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->MotifsupTableWidget->item(j,1)->text())) +"',";
        req += "'" + Utils::correctquoteSQL(Utils::trimcapitilize(ui->MotifsupTableWidget->item(j,2)->text())) +"',";
        req += "'" + ui->MotifsupTableWidget->item(j,4)->text() +"',";
        a = "NULL";
        if (UpchkFromTableW(ui->MotifsupTableWidget,j,6)!=Q_NULLPTR)
            if (UpchkFromTableW(ui->MotifsupTableWidget,j,6)->isChecked())
                a = "1";
        req += a + ",";
        req += QString::number(j+1) + ")";
    }
    DataBase::getInstance()->StandardSQL(req);
    MAJMotifs();
    accept();
}

void dlg_motifs::RemplirTableWidget()
{
    QFontMetrics fm(qApp->font());
    ui->MotifsupTableWidget->horizontalHeader()->setFixedHeight(int(fm.height()*1.3));

    //Remplissage Table MotifsupTableWidget
    ui->MotifsupTableWidget->clearContents();
    QList<Motif*> listMotifs;

    int i=0;
    ui->MotifsupTableWidget->setRowCount(m_motifs->size());
    for (QMap<int, Motif*>::const_iterator itmtf = m_motifs->constBegin() ; itmtf != m_motifs->constEnd() ; ++itmtf)
    {
        Motif *mtf = const_cast<Motif*>(*itmtf);
        SetMotifToRow(mtf, i);
        ++i;
    }
}

UpCheckBox* dlg_motifs::UpchkFromTableW(QTableWidget *Table, int row, int col)
{
    QWidget *w = dynamic_cast<QWidget*>(Table->cellWidget(row,col));
    if (w)
    {
        QList<UpCheckBox*> listcheckBox = w->findChildren<UpCheckBox*>();
        if (listcheckBox.size()>0)
            return listcheckBox.at(0);
    }
    return Q_NULLPTR;
}

Motif* dlg_motifs::getMotifFromRow(int row)
{
    Motif *mtf = Datas::I()->motifs->getMotifById(ui->MotifsupTableWidget->item(row,3)->text().toInt());
    return mtf;
}

void dlg_motifs::SetMotifToRow(Motif *mtf, int row)
{
    //+++ ne pas utiliser insertRow() qui est très lent au fur et à mesure qu'on vide et remplit la table
    int col = 0;                                                                            //0 - Utiliser
    QWidget *w         = new QWidget(ui->MotifsupTableWidget);
    UpCheckBox *Check  = new UpCheckBox(w);
    QHBoxLayout *l     = new QHBoxLayout();
    bool a = mtf->utiliser();
    Check->setChecked(a);
    Check->setRowTable(row);
    Check->setFocusPolicy(Qt::NoFocus);
    connect(Check,  SIGNAL(clicked(bool)),  this,   SLOT(Slot_Utiliser(bool)));
    l->setAlignment(Qt::AlignCenter);
    l->addWidget(Check);
    l->setContentsMargins(0,0,0,0);
    w->setLayout(l);
    ui->MotifsupTableWidget->setCellWidget(row,col,w);

    col++;                                                                                  //1 - Motif
    QTableWidgetItem *pItem0 = new QTableWidgetItem();
    pItem0->setText(mtf->motif());
    ui->MotifsupTableWidget->setItem(row,col,pItem0);

    col++;                                                                                  //2 - Raccourci
    QTableWidgetItem *pItem1 = new QTableWidgetItem();
    pItem1->setText(mtf->raccourci());
    ui->MotifsupTableWidget->setItem(row,col,pItem1);

    col++;                                                                                  //3 - idMotifsRDV
    QTableWidgetItem *pItem2 = new QTableWidgetItem();
    pItem2->setText(QString::number(mtf->id()));
    ui->MotifsupTableWidget->setItem(row,col,pItem2);

    col++;                                                                                  //4 - Couleur
    QTableWidgetItem *pItem3 = new QTableWidgetItem();
    pItem3->setText(mtf->couleur());
    ui->MotifsupTableWidget->setItem(row,col,pItem3);

    col++;                                                                                  //5 - Duree
    QTableWidgetItem *pItem4 = new QTableWidgetItem();
    pItem4->setText(QString::number(mtf->duree()));
    ui->MotifsupTableWidget->setItem(row,col,pItem4);

    col++;                                                                                  //6 - ParDefaut
    QWidget *w0         = new QWidget(ui->MotifsupTableWidget);
    UpCheckBox *Check0  = new UpCheckBox(w0);
    QHBoxLayout *l0     = new QHBoxLayout();
    bool b = mtf->pardefaut();
    Check0->setChecked(b);
    if (b)
    {
        Check->setChecked(true);
        Check0->setToggleable(false);
        Check->setToggleable(false);
    }
    Check0->setRowTable(row);
    Check0->setFocusPolicy(Qt::NoFocus);
    connect(Check0, SIGNAL(clicked(bool)), this, SLOT(Slot_ParDefaut()));
    l0->setAlignment( Qt::AlignCenter );
    l0->addWidget(Check0);
    l0->setContentsMargins(0,0,0,0);
    w0->setLayout(l0);
    ui->MotifsupTableWidget->setCellWidget(row,col,w0);

    col++;                                                                                  //7 - Couleur
    QWidget *w1         = new QWidget(ui->MotifsupTableWidget);
    UpLabel *Lbl1       = new UpLabel(w1);
    QHBoxLayout *l1     = new QHBoxLayout();
    QString background = "background:#" + mtf->couleur();
    Lbl1->setStyleSheet(background);
    Lbl1->setRow(row);
    l1->addWidget(Lbl1);
    l1->setContentsMargins(0,0,0,0);
    w1->setLayout(l1);
    ui->MotifsupTableWidget->setCellWidget(row,col,w1);

    col++;                                                                                  //8 - NoOrdre
    QTableWidgetItem *pItem5 = new QTableWidgetItem();
    pItem5->setText(QString::number(mtf->noordre()));
    ui->MotifsupTableWidget->setItem(row,col,pItem5);

    QFontMetrics fm(qApp->font());
    ui->MotifsupTableWidget->setRowHeight(row,int(fm.height()*1.3));
}

