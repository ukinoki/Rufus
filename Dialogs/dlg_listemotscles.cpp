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

#include "dlg_listemotscles.h"

dlg_listemotscles::dlg_listemotscles(Patient *pat, QWidget *parent) :
    UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionMotsCles", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    m_currentpatient    = pat;
    db                 = DataBase::I();

    wdg_bigtable              = new QTableView();

    RemplirTableView();
    wdg_bigtable->verticalHeader()->setVisible(false);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_bigtable->setGridStyle(Qt::NoPen);
    wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_bigtable->setColumnWidth(0,300);
    wdg_bigtable->setFixedWidth(wdg_bigtable->columnWidth(0)+2);
    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    wdg_bigtable->setMinimumHeight(hauteurligne*20);
    wdg_bigtable->setSizeIncrement(0,hauteurligne);
    wdg_bigtable->setMouseTracking(true);
    wdg_buttonframe = new WidgetButtonFrame(wdg_bigtable);
    wdg_buttonframe->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());

    AjouteLayButtons();
    setFixedWidth(wdg_bigtable->width() + dlglayout()->contentsMargins().left()*2);
    setModal(true);
    setSizeGripEnabled(false);
    setWindowTitle(tr("Liste des mots-clés"));

    m_listidmotsclesdepart << "-1";

    connect(wdg_bigtable,          SIGNAL(pressed(QModelIndex)),   this,   SLOT(Slot_Enablebuttons()));
    connect(OKButton,       SIGNAL(clicked(bool)),          this,   SLOT(Slot_OK()));
    connect(wdg_buttonframe,    SIGNAL(choix(int)),             this,   SLOT(Slot_ChoixButtonFrame(int)));
    wdg_buttonframe->wdg_modifBouton->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
}

dlg_listemotscles::~dlg_listemotscles()
{
}

void dlg_listemotscles::Slot_ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        CreationModifMC(Creation);
        break;
    case 0:
        CreationModifMC(Modif);
        break;
    case -1:
        SupprMC();
        break;
    default:
        break;
    }
}

void dlg_listemotscles::CreationModifMC(Mode mode)
{
    dlg_ask                      = new UpDialog(this);
    QWidget     *widg               = new QWidget(dlg_ask);
    UpLineEdit  *Line               = new UpLineEdit(dlg_ask);
    QCompleter  *MCListCompleter    = new QCompleter(m_listemotscles);

    widg            ->setLayout(new QVBoxLayout);
    widg->layout()  ->setContentsMargins(0,10,0,0);
    widg->layout()  ->addWidget(Line);
    Line            ->setFixedSize(300,21);
    dlg_ask->dlglayout()  ->insertWidget(0,widg);
    dlg_ask->dlglayout()  ->setSizeConstraint(QLayout::SetFixedSize);

    dlg_ask      ->AjouteLayButtons();
    dlg_ask      ->setWindowTitle(tr("Entrez un nouveau mot-clé"));

    connect(dlg_ask->OKButton,       SIGNAL(clicked(bool)),this,SLOT(Slot_VerifMC()));

    Line            ->setMaxLength(60);
    MCListCompleter ->setCaseSensitivity(Qt::CaseInsensitive);
    MCListCompleter ->setCompletionMode(QCompleter::InlineCompletion);
    Line            ->setCompleter(MCListCompleter);

    if (mode == Modif)
    {
        Line        ->setText(m_model->itemFromIndex(m_selectionmodel->currentIndex())->text());
        dlg_ask  ->setMode("Modif");
    }
    else
        dlg_ask  ->setMode("Creation");
    dlg_ask->exec();
    delete dlg_ask;
}

void dlg_listemotscles::Slot_VerifMC()
{
    QString nouvMC= dlg_ask->findChildren<UpLineEdit*>().at(0)->text();
    if (nouvMC == "")
    {
        dlg_message(tr("Vous n'avez pas rempli le mot clé"),2000,false);
        return;
    }
    if (m_listemotscles.contains(nouvMC, Qt::CaseInsensitive))
    {
        dlg_message(tr("Ce mot-clé existe déjà"),2000,false);
        if (m_model->findItems(nouvMC, Qt::MatchExactly).size()>0)
            wdg_bigtable->scrollTo(m_model->findItems(nouvMC, Qt::MatchExactly).at(0)->index());
        return;
    }
    if (dlg_ask->mode()=="Creation")
    {
        QString req = "insert into " TBL_MOTSCLES " (MotCle) values('" + Utils::correctquoteSQL(nouvMC) + "')";
        db->StandardSQL(req);
    }
    else if (dlg_ask->mode()=="Modif")
    {
        QString req = "update " TBL_MOTSCLES " set MotCle = '" + Utils::correctquoteSQL(nouvMC) + "' where MotCle = '"
                   + m_model->itemFromIndex(m_selectionmodel->currentIndex())->text() + "'";
        db->StandardSQL(req);
    }
    db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(m_currentpatient->id()));
    QStringList listidMc;
    for (int i=0; i< m_model->rowCount(); i++)
        if(m_model->item(i,0)->checkState() == Qt::Checked)
            listidMc << m_model->item(i,0)->accessibleDescription();
    if (listidMc.size()>0)
    {
        QString req = "insert into " TBL_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
        req += "(" + QString::number(m_currentpatient->id()) + ", " + listidMc.at(0) + ")";
        for (int j=1; j<listidMc.size(); j++)
            req += ", (" + QString::number(m_currentpatient->id()) + ", " + listidMc.at(j) + ")";
        db->StandardSQL(req);
    }
    RemplirTableView();
    dlg_ask->accept();
}

void dlg_listemotscles::SupprMC()
{
    UpMessageBox msgbox;
    msgbox.setText(tr("Suppression de mot clé!"));
    msgbox.setInformativeText(tr("Voulez vous vraiment supprimer le mot-clé\n") + m_model->itemFromIndex(m_selectionmodel->currentIndex())->text());
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton AnnulBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer le mot-clé"));
    msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    //msgbox.setDefaultButton(AnnulBouton);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        QString req = "delete from " TBL_MOTSCLES " where idmotcle = " + m_model->itemFromIndex(m_selectionmodel->currentIndex())->accessibleDescription();
        db->StandardSQL(req);
        RemplirTableView();
    }
}

void dlg_listemotscles::Slot_Enablebuttons()
{
    wdg_buttonframe->wdg_modifBouton->setEnabled(m_selectionmodel->hasSelection());
    wdg_buttonframe->wdg_moinsBouton->setEnabled(m_selectionmodel->hasSelection());
}

void dlg_listemotscles::Slot_OK()
{
    db->StandardSQL("delete from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(m_currentpatient->id()));
    QStringList listidMc;
    for (int i=0; i< m_model->rowCount(); i++)
        if(m_model->item(i,0)->checkState() == Qt::Checked)
            listidMc << m_model->item(i,0)->accessibleDescription();
    if (listidMc.size()>0)
    {
        QString req = "insert into " TBL_MOTSCLESJOINTURES " (idpat, idmotcle) values ";
        req += "(" + QString::number(m_currentpatient->id()) + ", " + listidMc.at(0) + ")";
        for (int j=1; j<listidMc.size(); j++)
            req += ", (" + QString::number(m_currentpatient->id()) + ", " + listidMc.at(j) + ")";
        db->StandardSQL(req);
    }
    accept();
}

void dlg_listemotscles::DisableLines()
{
    m_selectionmodel->clearSelection();
}

QStringList dlg_listemotscles::listMCDepart() const
{
    return m_listidmotsclesdepart;
}

void dlg_listemotscles::RemplirTableView()
{
    bool ok;
    QString req = "select idMotcle from " TBL_MOTSCLESJOINTURES " where idpat = " + QString::number(m_currentpatient->id());
    QList<QVariantList> idmotclelist = db->StandardSelectSQL(req, ok);
    QStringList listidMC;
    bool a = m_listidmotsclesdepart.contains("-1");
    m_listidmotsclesdepart.clear();
    if (idmotclelist.size()>0)
    {
        for (int i=0; i<idmotclelist.size(); i++)
        {
            listidMC << idmotclelist.at(i).at(0).toString();
            if (a)
                m_listidmotsclesdepart << idmotclelist.at(i).at(0).toString();
        }
    }
    req = "select idmotcle, motcle from " TBL_MOTSCLES " order by motcle";
    QList<QVariantList> motclelist = db->StandardSelectSQL(req, ok);
    QStandardItem       *pitem;
    m_model = dynamic_cast<QStandardItemModel*>(wdg_bigtable->model());
    if (m_model)
        m_model->clear();
    else
        m_model = new QStandardItemModel(this);
    m_selectionmodel = new QItemSelectionModel(m_model);

    m_listemotscles.clear();
    pitem   = new QStandardItem(tr("Mot-clé"));
    pitem->setEditable(false);
    m_model->setHorizontalHeaderItem(0,pitem);

    for (int i=0; i<motclelist.size(); i++)
    {
        pitem   = new QStandardItem(motclelist.at(i).at(1).toString());
        pitem->setAccessibleDescription(motclelist.at(i).at(0).toString());
        //pitem->setEditable(false);
        pitem->setCheckable(true);
        if (listidMC.contains(motclelist.at(i).at(0).toString()))
            pitem->setCheckState(Qt::Checked);
        else
            pitem->setCheckState(Qt::Unchecked);
        m_model->appendRow(pitem);
        m_listemotscles << motclelist.at(i).at(1).toString();
    }
    wdg_bigtable->setModel(m_model);
    wdg_bigtable->setSelectionModel(m_selectionmodel);

    QFontMetrics fm(qApp->font());
    int hauteurligne = int(fm.height()*1.1);
    for (int i=0; i<m_model->rowCount(); i++)
        wdg_bigtable->setRowHeight(i,hauteurligne);
    wdg_bigtable->horizontalHeader()->setFixedHeight(hauteurligne);
    wdg_bigtable->setColumnWidth(0,300);
}
