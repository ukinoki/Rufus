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

#include "dlg_commentaires.h"
#include "ui_dlg_commentaires.h"
#include "icons.h"


dlg_commentaires::dlg_commentaires(QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionCommentaires", parent),
    ui(new Ui::dlg_commentaires)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    proc        = Procedures::I();
    db          = DataBase::getInstance();
    gidUser     = db->getUserConnected()->id();

    widgButtons = new WidgetButtonFrame(ui->ComupTableWidget);
    widgButtons->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    dlglayout()     ->insertWidget(0,ui->upTextEdit);
    dlglayout()     ->insertWidget(0,widgButtons->widgButtonParent());
    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    CancelButton    ->disconnect();

    setWindowTitle(tr("Liste des commentaires prédéfinis de ") + db->getUserConnected()->getLogin());

    // Initialisation des slots.
    connect (OKButton,                  &QPushButton::clicked,                  [=] {Validation();});
    connect (CancelButton,              &QPushButton::clicked,                  [=] {Annulation();});
    connect (ui->upTextEdit,            &QTextEdit::textChanged,                [=] {EnableOKPushbutton();});
    connect (ui->upTextEdit,            &UpTextEdit::dblclick,                  [=] {dblClicktextEdit();});
    connect(widgButtons,                &WidgetButtonFrame::choix,              [=] {ChoixButtonFrame(widgButtons->Reponse());});


    // Mise en forme du formulaire
    ui->ComupTableWidget->setColumnCount(5);
    ui->ComupTableWidget->setColumnWidth(0,30);      // Check
    ui->ComupTableWidget->setColumnWidth(1,380);     // Resume
    ui->ComupTableWidget->setColumnHidden(2,true);   // idComment
    ui->ComupTableWidget->setColumnHidden(3,true);   // idUser
    ui->ComupTableWidget->setColumnWidth(4,30);      // DefautIcon
    ui->ComupTableWidget->setPalette(QPalette(Qt::white));
    ui->ComupTableWidget->setGridStyle(Qt::NoPen);
    ui->ComupTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->ComupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ComupTableWidget->verticalHeader()->setVisible(false);
    ui->ComupTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->ComupTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Icons::icImprimer(),""));
    ui->ComupTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("TITRES DES COMMENTAIRES")));
    ui->ComupTableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    ui->ComupTableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    ui->ComupTableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(Icons::icCheckOblig(),""));
    ui->ComupTableWidget->horizontalHeader()->setVisible(true);
    ui->ComupTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    ui->ComupTableWidget->horizontalHeader()->setIconSize(QSize(30,30));

    gTimerEfface    = new QTimer(this);
    ui->upTextEdit  ->installEventFilter(this);
    ui->ComupTableWidget->installEventFilter(this);

    gModeSurvol = true;
    Remplir_TableView();
    if (ui->ComupTableWidget->rowCount() > 0)
        ConfigMode(Selection);
    else
        ConfigMode(Creation);
}

dlg_commentaires::~dlg_commentaires()
{
    proc = Q_NULLPTR;
    delete proc;
    delete ui;
}

void dlg_commentaires::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void dlg_commentaires::ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        ConfigMode(Creation);
        break;
    case 0:
        Modif_Com();
        break;
    case -1:
        Del_Com();
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton ANNULER.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_commentaires::Annulation()
{
    if (gMode == Creation || gMode == Modification)
    {
        int     row = -1;
        UpLineEdit *line;
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(i,1));
            if (line->focusPolicy() == Qt::WheelFocus)
            {
                row = line->getRowTable();
                break;
            }
        }
        Remplir_TableView();
        if (ui->ComupTableWidget->rowCount() == 0)
            ConfigMode(Creation);
        else
        {
            ConfigMode(Selection);
            LineSelect(row);
        }
        line = Q_NULLPTR;
        delete line;
    }
    else
        reject();
}

// ----------------------------------------------------------------------------------
// On a survole une ligne de comm. On affice le détail
// ----------------------------------------------------------------------------------
QString dlg_commentaires::CalcToolTip(QString ab)
{
    ab.replace(QRegExp("\n\n[\n]*"),"\n");
    if (ab.size()>300)
    {
        ab = ab.left(300);
        if (ab.endsWith("\n"))
            ab = ab.left(ab.size()-1);
        else if (!ab.endsWith("."))
        {
            if (ab.endsWith(" "))
                ab = ab.left(ab.size()-1) + "...";
            else
            {
                int a = ab.lastIndexOf(" ");
                int b = ab.lastIndexOf(".");
                int c = b;
                if (a>b) c = a;
                ab = ab.left(c) + "...";
            }
        }
    }
    int cassure = 50;
    QStringList listhash1;
    listhash1 = ab.split("\n");
    QString ResumeItem = "";
    for (int i=0; i<listhash1.size(); i++)
    {
        QString abc = listhash1.at(i);
        if (listhash1.at(i).size() > cassure)
        {
            QStringList listhash;
            listhash = listhash1.at(i).split(" ");
            abc = listhash.at(0);
            int count = abc.size() + 1;
            for (int j=1; j<listhash.size(); j++)
            {
                count += listhash.at(j).size();
                if (count>cassure)
                {
                    abc += "\n";
                    count = listhash.at(j).size() + 1;
                }
                else
                {
                    abc += " ";
                    count += 1;
                }
                abc += listhash.at(j);
            }
        }
        ResumeItem += abc;
        if (i <(listhash1.size()-1)) ResumeItem += "\n";
    }
    return ResumeItem;
}

// ----------------------------------------------------------------------------------
// Annulation du commentaire selectionne
// ----------------------------------------------------------------------------------
void dlg_commentaires::Del_Com()
{
    bool a = false;
    UpLineEdit *line = new UpLineEdit(this);
    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line->hasSelectedText()) {a= true; break;}
    }
    if (a)
    {
        DisableLines();
        SupprimmCommentaire(line->getRowTable());
    }
}

void dlg_commentaires::dblClicktextEdit()
{
    if (gMode == Selection)
    {
        bool a = false;
        UpLineEdit *line = new UpLineEdit(this);
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
            if (line->hasSelectedText()) {a= true; break;}
        }
        if (a)
        {
            int idUser = ui->ComupTableWidget->item(line->getRowTable(),4)->text().toInt();
            if (idUser == db->getUserConnected()->id())
                ConfigMode(Modification,line->getRowTable());
        }
        line = Q_NULLPTR;
        delete line;
    }
}

// ----------------------------------------------------------------------------------
// On a clique sur une ligne de comm. On active ou desactive les icones Modif, Annul, ...
// ----------------------------------------------------------------------------------
void dlg_commentaires::EnableOKPushbutton()
{
    if (gMode != Selection)
    {
        UpLineEdit *line = new UpLineEdit(this);
        bool a = false;
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) {a = true; break;}
        }
        if (a == false)
            OKButton->setEnabled(false);
        else
        {
            if (line->text().size() == 0)
                OKButton->setEnabled(false);
            else if (ui->upTextEdit->toPlainText().size() == 0)
                OKButton->setEnabled(false);
            else
                OKButton->setEnabled(true);
        }
        line = Q_NULLPTR;
        delete line;
    }
    else
        OKButton->setEnabled(true);
}

void dlg_commentaires::MenuContextuel(UpLineEdit *line)
{
    QMenu *menuContextuel               = new QMenu(this);
    QAction *pAction_ModifCommentaire   = new QAction();
    QAction *pAction_SupprCommentaire   = new QAction();
    QAction *pAction_CreerCommentaire   = new QAction();
    QAction *pAction_ParDefautCom       = new QAction();

    LineSelect(line->getRowTable());
    pAction_CreerCommentaire                = menuContextuel->addAction(Icons::icCreer(), tr("Créer un commentaire"));
    connect (pAction_CreerCommentaire,      &QAction::triggered,    [=] {ChoixMenuContextuel("CreerCom");});
    bool a = false;
    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line->hasSelectedText())
            if (ui->ComupTableWidget->item(line->getRowTable(),3)->text().toInt() == gidUser)
            {a =true; break;}
    }
    if (a)
    {
        pAction_ModifCommentaire                = menuContextuel->addAction(Icons::icEditer(), tr("Modifier ce commentaire"));
        pAction_SupprCommentaire                = menuContextuel->addAction(Icons::icPoubelle(), tr("Supprimer ce commentaire"));
        menuContextuel->addSeparator();
        UpLabel *lbldef                         = static_cast<UpLabel*>(ui->ComupTableWidget->cellWidget(line->getRowTable(),4));
        if (lbldef->pixmap()!=Q_NULLPTR)
            pAction_ParDefautCom                = menuContextuel->addAction(Icons::icBlackCheck(), tr("Par défaut"));
        else
            pAction_ParDefautCom                = menuContextuel->addAction("Par défaut") ;
        pAction_ParDefautCom->setToolTip(tr("si cette option est cochée\nle commentaire sera systématiquement imprimé"));

        connect (pAction_ModifCommentaire,      &QAction::triggered,    [=] {ChoixMenuContextuel("ModifierCom");});
        connect (pAction_SupprCommentaire,      &QAction::triggered,    [=] {ChoixMenuContextuel("SupprimerCom");});
        connect (pAction_CreerCommentaire,      &QAction::triggered,    [=] {ChoixMenuContextuel("CreerCom");});
        connect (pAction_ParDefautCom,          &QAction::triggered,    [=] {ChoixMenuContextuel("ParDefautCom");});
    }
    line = Q_NULLPTR;
    delete line;
    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
}

void dlg_commentaires::ChoixMenuContextuel(QString choix)
{
    QPoint pos = ui->ComupTableWidget->viewport()->mapFromGlobal(findChildren<QMenu*>().at(0)->pos());
    int row (-1);
    UpLineEdit *line = new UpLineEdit();
    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line->hasSelectedText())
        {
            row = line->getRowTable();
            break;
        }
    }
    row = line->getRowTable();
    if (choix  == "ModifierCom")
    {
        if (row > -1)
            ConfigMode(Modification,row);
    }
    else if (choix  == "SupprimerCom")
    {
        if (row > -1)
        SupprimmCommentaire(row);
    }
    else if (choix  == "ParDefautCom")
    {
        if (row > -1)
        {
            UpLabel *lbldef = static_cast<UpLabel*>(ui->ComupTableWidget->cellWidget(row,4));
            QString b = "null";
            if (lbldef->pixmap()!=Q_NULLPTR)
                lbldef->clear();
            else
            {
                b = "1";
                lbldef->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled
            }
            lbldef = Q_NULLPTR;
            delete lbldef;
            db->StandardSQL ("update " NOM_TABLE_COMMENTAIRESLUNETTES " set ParDefautComment = " + b +
                             " where idCommentLunet = " + ui->ComupTableWidget->item(row,2)->text());
        }
    }
    else if (choix  == "CreerCom")
    {
        row = ui->ComupTableWidget->rowAt(pos.y());
        ConfigMode(Creation,row);
    }
    line = Q_NULLPTR;
    delete line;
}

// ----------------------------------------------------------------------------------
// Modification du commentaire dans la base.
// ----------------------------------------------------------------------------------
void dlg_commentaires::Modif_Com()
{
    UpLineEdit *line = new UpLineEdit();
    bool a = false;
    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line->hasSelectedText()) {a= true; break;}
    }
    int row = line->getRowTable();
    line = Q_NULLPTR;
    delete line;
    if (a)
        ConfigMode(Modification,row);
}

// ----------------------------------------------------------------------------------
// Clic sur le bouton OK.
// L'action depend de ce qu'on est en train de faire (creation modife, selection)
// ----------------------------------------------------------------------------------
void dlg_commentaires::Validation()
{
    bool a              = false;
    UpLineEdit *line    = Q_NULLPTR;

    if (gMode == Creation)
    {
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) {a = true; break;}
        }
        int row = line->getRowTable();
        line = Q_NULLPTR;
        delete line;
        if (a)
            InsertCommentaire(row);
    }
    else if (gMode == Modification)
    {
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(i,1));
            if (line->isEnabled()) {a = true; break;}
        }
        int row = line->getRowTable();
        line = Q_NULLPTR;
        delete line;
        if (a)
            UpdateCommentaire(row);
    }
    else if (gMode == Selection)
    {
        UpCheckBox *Check   = Q_NULLPTR;
        QWidget *Widg       = Q_NULLPTR;
        gReponseResumePrescription = "";
        for (int i =0 ; i < ui->ComupTableWidget->rowCount(); i++)
        {
            Widg =  dynamic_cast<QWidget*>(ui->ComupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                Check = Widg->findChildren<UpCheckBox*>().at(0);
                if (Check->isChecked())
                {
                    if (gReponseResumePrescription != "") gReponseResumePrescription += "\n";
                    line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(Check->getRowTable(),1));
                    gReponseResumePrescription  += line->getData().toString();
                    gReponseResumeDossier       += " - " + line->text();
                }
            }
        }
        line = Q_NULLPTR;
        delete line;
        Widg = Q_NULLPTR;
        delete Widg;
        Check = Q_NULLPTR;
        delete Check;
        accept();
    }
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_commentaires::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        if (obj->objectName() == "textEdit_Com")
        {
            if (ui->upTextEdit->toPlainText() != textDocument)
                OKButton->setEnabled(true);
        }
    }
    if(event->type() == QEvent::MouseMove)
    {
        if (obj == ui->upTextEdit)
            if (gMode == Selection)
            {
                QRect rect = QRect(ui->upTextEdit->pos(),ui->upTextEdit->size());
                QPoint pos = mapFromParent(cursor().pos());
                if (rect.contains(pos) && ui->upTextEdit->toPlainText() != "")
                    EffaceWidget(ui->upTextEdit, false);
            }
    }
    if (event->type() == QEvent::KeyPress)
        if (obj->inherits("UpTableWidget"))
        {
            QKeyEvent       *keyEvent = static_cast<QKeyEvent*>(event);
            UpTableWidget   *table    = static_cast<UpTableWidget *>(obj);
            UpLineEdit      *line     = Q_NULLPTR;
            if (keyEvent->key() == Qt::Key_Up)
            {
                for (int i=0; i<table->rowCount(); i++)
                {
                    line = static_cast<UpLineEdit *>(table->cellWidget(i,1));
                    if (line->selectedText() != "")
                    {
                        if (line->getRowTable() > 0){
                            LineSelect(line->getRowTable()-1);
                            QPoint posdebut = mapFrom(this, table->cellWidget(0,0)->pos());
                            QPoint poscell  = mapFrom(this, table->cellWidget(line->getRowTable(),0)->pos());
                            if (posdebut.y() < -1 && poscell.y() < table->height()-(line->height()*7))
                                table->viewport()->scroll(0,line->height());
                        }
                        break;
                    }
                }
            }
            if (keyEvent->key() == Qt::Key_Down)
            {
                for (int i=0; i<table->rowCount(); i++)
                {
                    line = static_cast<UpLineEdit *>(table->cellWidget(i,1));
                    if (line->selectedText() != "")
                    {
                        if (line->getRowTable() < table->rowCount()-1){
                            LineSelect(line->getRowTable()+1);
                            QPoint poscell = mapFrom(this, table->cellWidget(line->getRowTable(),0)->pos());
                            QPoint posfin = mapFrom(this, table->cellWidget(table->rowCount()-1,0)->pos());
                            if (posfin.y() > table->height()-(line->height()*2) && poscell.y() > line->height()*5)
                                table->viewport()->scroll(0,-line->height());
                        }
                        break;
                    }
                }
            }
            keyEvent = Q_NULLPTR;
            delete keyEvent;
            table    = Q_NULLPTR;
            delete table;
            line = Q_NULLPTR;
            delete line;
        }
    return QWidget::eventFilter(obj, event);
}

void dlg_commentaires::keyPressEvent(QKeyEvent * event )
{
    switch (event->key()) {
    case Qt::Key_Escape:
    {
        Annulation();
        break;
    }
    default:
        break;
    }
}

// ----------------------------------------------------------------------------------
// Recherche de doublon après création ou modification de document
// ----------------------------------------------------------------------------------
bool dlg_commentaires::ChercheDoublon(QString str, int row)
{
    bool a = false;
    switch (gMode) {
    case Creation:
    case Modification:
        for (int i=0;  i<ui->ComupTableWidget->rowCount(); i++)
        {
            UpLineEdit *line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(i,1));
            if  (line != Q_NULLPTR)
                if (line->text().toUpper() == str.toUpper() && line->getRowTable()!=row)
                {
                    a = true;
                    QString b = "vous";
                    int iduser = ui->ComupTableWidget->item(i,3)->text().toInt();
                    if (iduser != gidUser)
                        b = Datas::I()->users->getUserById(iduser)->getLogin();
                    UpMessageBox::Watch(this, tr("Il existe déjà un commentaire portant ce nom créé par ") + b);
                    break;
                }
            line = Q_NULLPTR;
            delete line;
        }
        break;
    default:
        break;
    }
    return a;
}

// ----------------------------------------------------------------------------------
// Configuration de la fiche en fonction du mode de fonctionnement
// ----------------------------------------------------------------------------------
void dlg_commentaires::ConfigMode(int mode, int row)
{
    gMode = mode;

    if (gMode != Selection)
        gTimerEfface->disconnect();
    gOp.setOpacity(1);
    ui->upTextEdit->setGraphicsEffect(&gOp);
    ui->upTextEdit->setVisible               (gMode != Selection);

    if (mode == Selection)
    {
        EnableLines();
        widgButtons->setEnabled(true);
        ui->ComupTableWidget->setEnabled(true);
        ui->ComupTableWidget->setFocus();
        ui->ComupTableWidget->setStyleSheet("");
        widgButtons->modifBouton->setEnabled(false);
        widgButtons->moinsBouton->setEnabled(false);
        ui->upTextEdit->setEnabled(true);
        ui->upTextEdit->clear();
        ui->upTextEdit->setFocusPolicy(Qt::NoFocus);
        ui->upTextEdit->setStyleSheet("");

        CancelButton->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        CancelButton->setImmediateToolTip(tr("Annuler et fermer la fiche"));
        OKButton->setImmediateToolTip(tr("Imprimer\nla sélection"));

        int nbCheck = 0;
        for (int i =0 ; i < ui->ComupTableWidget->rowCount(); i++)
        {
            QWidget *Widg =  dynamic_cast<QWidget*>(ui->ComupTableWidget->cellWidget(i,0));
            if (Widg)
            {
                UpCheckBox *Check = Widg->findChildren<UpCheckBox*>().at(0);
                Check->setEnabled(true);
                if (Check->isChecked()) nbCheck ++;
                Check = Q_NULLPTR;
                delete Check;
            }
            Widg = Q_NULLPTR;
            delete Widg;
        }
        OKButton->setEnabled(true);
    }

    if (mode == Modification)
    {
        DisableLines();
        UpLineEdit *line = static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(row,1));
        line->setEnabled(true);
        line->setFocusPolicy(Qt::WheelFocus);
        line->setFocus();
        line->selectAll();
        connect(line,   &QLineEdit::textEdited, [=] {EnableOKPushbutton();});

        ui->upTextEdit->setText(line->getData().toString());
        ui->ComupTableWidget->setEnabled(true);
        ui->ComupTableWidget->setStyleSheet("");
        widgButtons->setEnabled(false);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setImmediateToolTip(tr("Enregistrer"));
        line = Q_NULLPTR;
        delete line;
    }
    else if (mode == Creation)
    {
        if (ui->ComupTableWidget->rowCount() > 0)
            DisableLines();
        ui->ComupTableWidget->insertRow(row);
        QWidget * w = new QWidget(ui->ComupTableWidget);
        UpCheckBox *Check = new UpCheckBox(w);
        Check->setCheckState(Qt::Unchecked);
        Check->setRowTable(row);
        Check->setFocusPolicy(Qt::NoFocus);
        Check->setEnabled(false);
        QHBoxLayout *l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Check);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);
        ui->ComupTableWidget->setCellWidget(row,0,w);
        UpLineEdit *upLine0 = new UpLineEdit;
        upLine0->setText("Nouveau Commentaire");                          // resume
        upLine0->setRowTable(row);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::WheelFocus);
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        upLine0->selectAll();
        ui->ComupTableWidget->setCellWidget(row,1,upLine0);
        QTableWidgetItem    *pItem1 = new QTableWidgetItem;
        QTableWidgetItem    *pItem2 = new QTableWidgetItem;
        int col = 2;
        pItem1->setText("0");                          // idComment
        ui->ComupTableWidget->setItem(row,col,pItem1);
        col++; //3
        pItem2->setText(QString::number(gidUser));     // idUser
        ui->ComupTableWidget->setItem(row,col,pItem2);
        col++; //4
        UpLabel*lbl = new UpLabel(ui->ComupTableWidget);
        lbl->setAlignment(Qt::AlignCenter);
        ui->ComupTableWidget->setCellWidget(row,col,lbl);
        connect(upLine0,   &QLineEdit::textEdited, this, &dlg_commentaires::EnableOKPushbutton);
        ui->ComupTableWidget->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));

        widgButtons->moinsBouton->setEnabled(false);
        ui->upTextEdit->clear();
        ui->upTextEdit->setEnabled(true);
        ui->upTextEdit->setFocusPolicy(Qt::WheelFocus);
        ui->upTextEdit->setStyleSheet("border: 2px solid rgb(251, 51, 61);");

        CancelButton->setUpButtonStyle(UpSmallButton::BACKBUTTON);
        CancelButton->setEnabled(true);
        CancelButton->setImmediateToolTip(tr("Revenir au mode\nsélection de commentaire"));
        OKButton->setEnabled(false);
        OKButton->setImmediateToolTip(tr("Enregistrer\nle commentaire"));
        upLine0->setFocus();

        w = Q_NULLPTR;
        l = Q_NULLPTR;
        delete w;
        delete l;
        pItem1 = Q_NULLPTR;
        pItem2 = Q_NULLPTR;
        delete pItem1;
        delete pItem2;
        Check = Q_NULLPTR;
        upLine0 = Q_NULLPTR;
        lbl = Q_NULLPTR;
        delete Check;
        delete upLine0;
        delete lbl;
    }
}

// --------------------------------------------------------------------------------------------------
// SetEnabled = false et disconnect toute sles lignes des UpTableWidget - SetEnabled = false checkBox
// --------------------------------------------------------------------------------------------------
void dlg_commentaires::DisableLines()
{
    QWidget *Widg       = Q_NULLPTR;
    UpCheckBox *Check   = Q_NULLPTR;
    UpLineEdit *line    = Q_NULLPTR;
    widgButtons->setEnabled(false);
    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        Widg = dynamic_cast<QWidget*>(ui->ComupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(false);
        }
        line = dynamic_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line != Q_NULLPTR) {
            line->deselect();
            line->setEnabled(false);
            line->setFocusPolicy(Qt::NoFocus);
            line->disconnect();
        }
    }
    Widg    = Q_NULLPTR; delete Widg;
    Check   = Q_NULLPTR; delete Check;
    line    = Q_NULLPTR; delete line;
}

// -------------------------------------------------------------------------------------------
// SetEnabled = true, connect toutes les lignes des UpTableWidget - SetEnabled = true checkBox
// -------------------------------------------------------------------------------------------
void dlg_commentaires::EnableLines()
{
    QWidget    *Widg    = Q_NULLPTR;
    UpCheckBox *Check   = Q_NULLPTR;
    UpLineEdit *line    = Q_NULLPTR;
    widgButtons->setEnabled(true);
    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        Widg =  dynamic_cast<QWidget*>(ui->ComupTableWidget->cellWidget(i,0));
        if (Widg)
        {
            Check = Widg->findChildren<UpCheckBox*>().at(0);
            Check->setEnabled(true);
        }
        line = dynamic_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line != Q_NULLPTR) {
            line->deselect();
            line->setEnabled(true);
            line->setFocusPolicy(Qt::NoFocus);
            connect(line,       &QWidget::customContextMenuRequested,   [=] {MenuContextuel(line);});
            connect(line,       &QLineEdit::textEdited,                 [=] {EnableOKPushbutton();});
            if (ui->ComupTableWidget->item(i,3)->text().toInt() == gidUser)
                connect(line,   &UpLineEdit::mouseDoubleClick,          [=] {if (gMode == Selection) ConfigMode(Modification, line->getRowTable());});
            connect(line,       &UpLineEdit::mouseRelease,              [=] {LineSelect(line->getRowTable());});
        }
    }
    Widg    = Q_NULLPTR; delete Widg;
    Check   = Q_NULLPTR; delete Check;
    line    = Q_NULLPTR; delete line;
}

// ----------------------------------------------------------------------------------
// Effacement progressif d'un Widget.
// ----------------------------------------------------------------------------------
void dlg_commentaires::EffaceWidget(QWidget* widg, bool AvecOuSansPause)
{
    QTime DebutTimer     = QTime::currentTime();
    gOp.setOpacity(1);
    widg->setGraphicsEffect(&gOp);
    widg->setVisible(true);
    widg->setAutoFillBackground(true);
    gTimerEfface->disconnect();
    gTimerEfface->start(70);
    connect(gTimerEfface, &QTimer::timeout, this, [=]
    {
        QRect rect = QRect(widg->pos(),widg->size());
        QPoint pos = mapFromParent(cursor().pos());
        int Pause = (AvecOuSansPause? 4000: 0);
        if (DebutTimer.msecsTo(QTime::currentTime()) > Pause)
        {
            if (!rect.contains(pos))
            {
                gOp.setOpacity(gOp.opacity()*0.9);
                widg->setGraphicsEffect(&gOp);
                if (gOp.opacity() < 0.10)
                {
                    gTimerEfface->stop();
                    widg->setVisible(false);
                }
            }
            else
            {
                gOp.setOpacity(1);
                widg->setGraphicsEffect(&gOp);
            }
        }
    });
}

// ----------------------------------------------------------------------------------
// Creation du commentaire dans la base.
// ----------------------------------------------------------------------------------
void dlg_commentaires::InsertCommentaire(int row)
{
    // controle validate des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));

    // Creation du commentaire dans la table
    if (ChercheDoublon(line->text(),row))
    {
        LineSelect(row);
        line = Q_NULLPTR;
        delete line;
        return;
    }

    QString requete = "INSERT INTO " NOM_TABLE_COMMENTAIRESLUNETTES
            " (TextComment, ResumeComment, idUser, Pardefautcomment ) "
            " VALUES ('" + Utils::correctquoteSQL(ui->upTextEdit->toPlainText()) +
            "','" + Utils::correctquoteSQL(line->text().left(100)) +
            "'," + QString::number(gidUser) + ", null)";
    db->StandardSQL(requete, tr("Erreur d'enregistrement du commentaire dans ") + NOM_TABLE_COURRIERS);
    Remplir_TableView();

    if (ui->ComupTableWidget->rowCount() == 0)
        ConfigMode(Creation);
    else
    {
        ConfigMode(Selection);
        QString resume = line->text();
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
             line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(i,1));
             if (line->text() == resume)
             {
                LineSelect(line->getRowTable());
                QModelIndex index = ui->ComupTableWidget->model()->index(line->getRowTable(),1);
                ui->ComupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                break;
             }
         }
    }
    line = Q_NULLPTR;
    delete line;
 }

// ----------------------------------------------------------------------------------
// On sélectionne une ligne. On affiche le détail et on met en édition
// ----------------------------------------------------------------------------------
void dlg_commentaires::LineSelect(int row)
{
    if (ui->ComupTableWidget->rowCount() == 0) return;
    if (row < 0) row = 0;
    if (row > ui->ComupTableWidget->rowCount()-1) row = ui->ComupTableWidget->rowCount()-1;

    UpLineEdit *line = Q_NULLPTR;
    line = dynamic_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(row,1));
    if (!line)
    {
        line = Q_NULLPTR;
        delete line;
        return;
    }

    for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        UpLineEdit *line0 = dynamic_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (line0) line0->deselect();
        line0 = Q_NULLPTR;
        delete line0;
    }
    if (gMode == Selection)
    {
        ui->upTextEdit->setVisible(true);
        ui->upTextEdit->setText(line->getData().toString());
        EffaceWidget(ui->upTextEdit);
        widgButtons->modifBouton    ->setEnabled(ui->ComupTableWidget->item(row,3)->text().toInt() == gidUser);
        widgButtons->moinsBouton    ->setEnabled(ui->ComupTableWidget->item(row,3)->text().toInt() == gidUser);
    }
    line->selectAll();
    line = Q_NULLPTR;
    delete line;
}

// ----------------------------------------------------------------------------------
// Remplissage de tree view avec les commentaires de la base.
// ----------------------------------------------------------------------------------
void dlg_commentaires::Remplir_TableView()
{
    UpLineEdit          *upLine0;
    QTableWidgetItem    *pItem1;
    QTableWidgetItem    *pItem2;
    QWidget * w;
    QHBoxLayout *l;
    UpCheckBox *Check;
    UpLabel*lbl2;
    int i;
    QFont disabledFont = qApp->font();
    disabledFont.setItalic(true);
    QPalette palette;
    palette.setColor(QPalette::Text,Qt::black);

    //Remplissage Table Documents
    for (int i = 0; i<ui->ComupTableWidget->rowCount(); i++)
    {
        upLine0 = dynamic_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(i,1));
        if (upLine0 != Q_NULLPTR)
            upLine0->disconnect();
    }

    ui->ComupTableWidget->clearContents();
    QString Remplirtablerequete = "SELECT ResumeComment, ParDefautComment, TextComment, idCommentLunet, idUser"
              " FROM "  NOM_TABLE_COMMENTAIRESLUNETTES
              " WHERE idUser = " + QString::number(gidUser) +
                (proc->UserSuperviseur() != gidUser? " Or idUser = " + QString::number(proc->UserSuperviseur()) : "") +
                ((proc->UserParent() != proc->UserSuperviseur())&&(proc->UserParent() != gidUser)? " Or idUser = " + QString::number(proc->UserParent()) : "") +
              " ORDER BY ResumeComment";
    bool ok;
    QList<QList<QVariant>> listcom = db->StandardSelectSQL(Remplirtablerequete, ok);
    if (!ok)
        return;
    ui->ComupTableWidget->setRowCount(listcom.size());
    for (i = 0; i < listcom.size(); i++)
    {
        pItem1  = new QTableWidgetItem() ;
        upLine0 = new UpLineEdit() ;
        pItem2  = new QTableWidgetItem() ;

        int col = 0;
        w = new QWidget(ui->ComupTableWidget);
        Check = new UpCheckBox(w);
        Check->setChecked(listcom.at(i).at(1).toInt() == 1);
        Check->setRowTable(i);
        Check->setFocusPolicy(Qt::NoFocus);
        connect(Check,   &QCheckBox::clicked,   this,   &dlg_commentaires::EnableOKPushbutton);
        l = new QHBoxLayout();
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Check);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);
        ui->ComupTableWidget->setCellWidget(i,col,w);

        col++; //1
        upLine0->setText(listcom.at(i).at(0).toString());                          // resume
        upLine0->setRowTable(i);
        upLine0->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
        upLine0->setFocusPolicy(Qt::NoFocus);
        upLine0->setImmediateToolTip(CalcToolTip(listcom.at(i).at(2).toString()));
        upLine0->setData(listcom.at(i).at(2).toString());
        if (listcom.at(i).at(4).toInt() != gidUser)
        {
            upLine0->setFont(disabledFont);
            upLine0->setPalette(palette);
        }
        upLine0->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->ComupTableWidget->setCellWidget(i,col,upLine0);

        col++; //2
        pItem1->setText(listcom.at(i).at(3).toString());                           // idCommentaire
        ui->ComupTableWidget->setItem(i,col,pItem1);
        col++; //3
        pItem2->setText(listcom.at(i).at(4).toString());                           // idUser
        ui->ComupTableWidget->setItem(i,col,pItem2);
        col++; //4                                                                 // DefautCheck
        lbl2 = new UpLabel(ui->ComupTableWidget);
        lbl2->setAlignment(Qt::AlignCenter);
        if (listcom.at(i).at(1).toInt()==1)
            lbl2->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled
        ui->ComupTableWidget->setCellWidget(i,col,lbl2);
        ui->ComupTableWidget->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.3));
    }
}

// ----------------------------------------------------------------------------------
// Supprime commentaire
// ----------------------------------------------------------------------------------
void dlg_commentaires::SupprimmCommentaire(int row)
{
    if (row == -1)
        return;
    DisableLines();

    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer le commentaire") + "\n" + static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(row,1))->text().toUpper() + "?";
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + db->getUserConnected()->getLogin() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton;
    OKBouton.setImmediateToolTip(tr("Supprimer le commentaire"));
    UpSmallButton NoBouton;
    NoBouton.setImmediateToolTip(tr("Annuler"));
    msgbox.addButton(&NoBouton,UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.setDefaultButton(&NoBouton);
    msgbox.exec();
    if (msgbox.clickedButton()  == &OKBouton)
    {
        int idCom = ui->ComupTableWidget->item(row,2)->text().toInt();
        Msg = tr("Impossible de supprimer le commentaire") + "\n" + static_cast<UpLineEdit*>(ui->ComupTableWidget->cellWidget(row,1))->text().toUpper() + "\n ... " + tr("et je ne sais pas pourquoi...");
        db->SupprRecordFromTable(idCom, "idcommentlunet", NOM_TABLE_COMMENTAIRESLUNETTES, Msg);
        Remplir_TableView();
    }
    if (ui->ComupTableWidget->rowCount() == 0)
        ConfigMode(Creation);
    else
    {
        ConfigMode(Selection);
        LineSelect(row);
    }
}

// ----------------------------------------------------------------------------------
// Modification du commentaire dans la base.
// ----------------------------------------------------------------------------------
void dlg_commentaires::UpdateCommentaire(int row)
{
    // recherche de l'enregistrement modifié
    // controle validate des champs
    UpLineEdit *line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(row,1));
    line->setText(Utils::trimcapitilize(line->text(), true, false, false));

    if (ChercheDoublon(line->text(),row))
    {
        LineSelect(row);
        line = Q_NULLPTR;
        delete line;
        return;
    }

    QString idAmodifier = ui->ComupTableWidget->item(row,2)->text();
    QString req = "UPDATE " NOM_TABLE_COMMENTAIRESLUNETTES
            " SET TextComment = '" + Utils::correctquoteSQL(ui->upTextEdit->toPlainText()) +
            "', ResumeComment = '" + Utils::correctquoteSQL(line->text().left(100)) +
            "' WHERE  idCommentLunet = " + idAmodifier;
    db->StandardSQL(req, tr("Erreur de mise à jour du document dans ") +  NOM_TABLE_COMMENTAIRESLUNETTES);
    Remplir_TableView();

    if (ui->ComupTableWidget->rowCount() == 0)
        ConfigMode(Creation);
    else
    {
        ConfigMode(Selection);
        QString resume = line->text();
        for (int i=0; i<ui->ComupTableWidget->rowCount(); i++)
        {
            line = static_cast<UpLineEdit *>(ui->ComupTableWidget->cellWidget(i,1));
            if (line->text() == resume)
            {
                LineSelect(line->getRowTable());
                QModelIndex index = ui->ComupTableWidget->model()->index(line->getRowTable(),1);
                ui->ComupTableWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
                QWidget *Widg =  dynamic_cast<QWidget*>(ui->ComupTableWidget->cellWidget(i,0));
                if (Widg)
                {
                    Widg->findChildren<UpCheckBox*>().at(0)->setChecked(true);
                    OKButton->setEnabled(true);
                }
                break;
            }
        }
    }
    line = Q_NULLPTR;
    delete line;
}
