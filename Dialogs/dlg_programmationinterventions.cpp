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

#include "dlg_programmationinterventions.h"

dlg_programmationinterventions::dlg_programmationinterventions(Patient *pat, QWidget *parent) : UpDialog(PATH_FILE_INI, "PositionsFiches/PositionProgramIntervention",parent)
{
    m_currentpatient = pat;
    setWindowTitle(tr("Programmer une intervention pour ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());
    UpTableView *wdg_tableprogramme = new UpTableView();
    QTreeView *wdg_dates            = new QTreeView();
    wdg_listmedecinscombo           = new QComboBox();
    QHBoxLayout *choixmedecinLay    = new QHBoxLayout();
    QHBoxLayout *programmLay        = new QHBoxLayout();

    wdg_tableprogramme  ->setFocusPolicy(Qt::NoFocus);
    wdg_tableprogramme  ->setPalette(QPalette(Qt::white));
    wdg_tableprogramme  ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tableprogramme  ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tableprogramme  ->verticalHeader()->setVisible(false);
    wdg_tableprogramme  ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_tableprogramme  ->setGridStyle(Qt::SolidLine);
    wdg_tableprogramme  ->verticalHeader()->setVisible(false);
    wdg_tableprogramme  ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_tableprogramme  ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    wdg_dates           ->setMaximumWidth(100);

    UpLabel* lblmedecins = new UpLabel;
    lblmedecins->setText(tr("Programme opératoire de "));
    choixmedecinLay     ->addWidget(lblmedecins);
    choixmedecinLay     ->addWidget(wdg_listmedecinscombo);
    choixmedecinLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixmedecinLay     ->setSpacing(5);
    choixmedecinLay     ->setContentsMargins(0,0,0,0);

    wdg_buttondateframe     = new WidgetButtonFrame(wdg_dates);
    wdg_buttondateframe     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::MoinsButton);
    connect (wdg_buttondateframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixDateFrame);
    programmLay     ->addWidget(wdg_buttondateframe->widgButtonParent());

    wdg_buttoninterventionframe     = new WidgetButtonFrame(wdg_tableprogramme);
    wdg_buttoninterventionframe     ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    connect (wdg_buttoninterventionframe,   &WidgetButtonFrame::choix,  this,   &dlg_programmationinterventions::ChoixInterventionFrame);

    programmLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    programmLay     ->addWidget(wdg_buttoninterventionframe->widgButtonParent());
    programmLay     ->setSpacing(5);
    programmLay     ->setContentsMargins(0,0,0,0);
    programmLay     ->setStretch(0,2);
    programmLay     ->setStretch(1,0);
    programmLay     ->setStretch(2,6);

    dlglayout()     ->insertLayout(0, programmLay);
    dlglayout()     ->insertLayout(0, choixmedecinLay);

    AjouteLayButtons(UpDialog::ButtonOK);
    connect(OKButton,     &QPushButton::clicked,    this, [=] {close();});
    setModal(true);
    dlglayout()->setStretch(0,1);
    dlglayout()->setStretch(1,15);

    QStandardItemModel *md_medecins = new QStandardItemModel();
    foreach (User* usr, *Datas::I()->users->actifs())
        if (usr->isMedecin())
        {
            QList<QStandardItem *> items;
            items << new QStandardItem(usr->login())
                  << new QStandardItem(QString::number(usr->id()));
            md_medecins->appendRow(items);
        }
    md_medecins->sort(0, Qt::DescendingOrder);
    for (int i=0; i< md_medecins->rowCount(); ++i)
    {
        wdg_listmedecinscombo->addItem(md_medecins->item(i,0)->text());             //! le login
        wdg_listmedecinscombo->setItemData(i, md_medecins->item(i,1)->text());      //! l'id en data
    }
    connect(wdg_listmedecinscombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &dlg_programmationinterventions::ChoixMedecin);
    if (Datas::I()->users->userconnected()->isMedecin())
    {
        wdg_listmedecinscombo->findData(Datas::I()->users->userconnected()->id());
        wdg_listmedecinscombo->setEnabled(false);
    }
    Datas::I()->interventions->initListebyUserId(wdg_listmedecinscombo->currentData().toInt());
}

dlg_programmationinterventions::~dlg_programmationinterventions()
{
}

void dlg_programmationinterventions::AfficheChoixIOL(int state)
{
    wdg_IOL->setVisible(state == Qt::Checked);
}

void dlg_programmationinterventions::ChoixDateFrame()
{
    switch (wdg_buttondateframe->Choix()) {
    case WidgetButtonFrame::Plus:
        proc->ab();
        break;
    case WidgetButtonFrame::Moins:
        proc->ab();
        break;
    default:
        break;
    }
}

void dlg_programmationinterventions::ChoixInterventionFrame()
{
    switch (wdg_buttoninterventionframe->Choix()) {
    case WidgetButtonFrame::Plus:
        CreerIntervention();
        break;
    case WidgetButtonFrame::Modifier:
        proc->ab();
        break;
    case WidgetButtonFrame::Moins:
        proc->ab();
        break;
    }
}

void dlg_programmationinterventions::ChoixMedecin(int idx)
{
    m_currentuser = Datas::I()->users->getById(wdg_listmedecinscombo->itemData(idx).toInt());
    Datas::I()->interventions->initListebyUserId(m_currentuser->id());
    UpMessageBox::Watch(this, "Docteur " + m_currentuser->prenom() + " " + m_currentuser->nom());
}

void dlg_programmationinterventions::CreerIntervention()
{
    UpDialog *dlg_intervention = new UpDialog(this);
    dlg_intervention->setWindowTitle(tr("programmer une intervention pour ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());

    QHBoxLayout *choixdateLay    = new QHBoxLayout();
    UpLabel* lbldate = new UpLabel;
    lbldate         ->setText(tr("Date"));
    QDateEdit *dateedit = new QDateEdit(m_currentdate);
    choixdateLay    ->addWidget(lbldate);
    choixdateLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixdateLay    ->addWidget(dateedit);
    choixdateLay    ->setSpacing(5);
    choixdateLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixheureLay    = new QHBoxLayout();
    UpLabel* lblheure = new UpLabel;
    lblheure        ->setText(tr("Heure"));
    QTimeEdit *timeedit = new QTimeEdit();
    choixheureLay   ->addWidget(lblheure);
    choixheureLay   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixheureLay   ->addWidget(timeedit);
    choixheureLay   ->setSpacing(5);
    choixheureLay   ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixinterventionLay    = new QHBoxLayout();
    UpLabel* lblinterv = new UpLabel;
    lblinterv               ->setText(tr("Type d'intervention"));
    QComboBox *intercombo = new QComboBox();
    choixinterventionLay    ->addWidget(lblinterv);
    choixinterventionLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixinterventionLay    ->addWidget(intercombo);
    choixinterventionLay    ->setSpacing(5);
    choixinterventionLay    ->setContentsMargins(0,0,0,0);

    QHBoxLayout *checkIOLLay    = new QHBoxLayout();
    QCheckBox *IOLchk = new QCheckBox(tr("Utiliser un implamt"));
    IOLchk          ->setCheckState(Qt::Unchecked);
    checkIOLLay     ->addWidget(IOLchk);
    checkIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    checkIOLLay     ->setSpacing(5);
    checkIOLLay     ->setContentsMargins(0,0,0,0);

    wdg_IOL         = new QWidget();
    QVBoxLayout *box_IOLlay  = new QVBoxLayout();
    box_IOLlay->setSpacing(0);

    QHBoxLayout *choixIOLLay    = new QHBoxLayout();
    UpLabel* lblIOL = new UpLabel;
    lblIOL          ->setText(tr("Implant"));
    QComboBox *IOLcombo = new QComboBox();
    choixIOLLay     ->addWidget(lblIOL);
    choixIOLLay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixIOLLay     ->addWidget(IOLcombo);
    choixIOLLay     ->setSpacing(5);
    choixIOLLay     ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixPwrIOLLay    = new QHBoxLayout();
    UpLabel* lblPwrIOL = new UpLabel;
    lblPwrIOL       ->setText(tr("Puissance"));
    UpDoubleSpinBox *PwrIOLspinbox = new UpDoubleSpinBox();
    PwrIOLspinbox   ->setRange(-10.00, 35.00);
    PwrIOLspinbox   ->setSingleStep(0.50);
    PwrIOLspinbox   ->setValue(21.00);
    choixPwrIOLLay  ->addWidget(lblPwrIOL);
    choixPwrIOLLay  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixPwrIOLLay  ->addWidget(PwrIOLspinbox);
    choixPwrIOLLay  ->setSpacing(5);
    choixPwrIOLLay  ->setContentsMargins(0,0,0,0);

    QHBoxLayout *choixCylIOLLay    = new QHBoxLayout();
    UpLabel* lblCylIOL = new UpLabel;
    lblCylIOL       ->setText(tr("Cylindre"));
    UpDoubleSpinBox *CylIOLspinbox = new UpDoubleSpinBox();
    CylIOLspinbox   ->setRange(0.00, 10.00);
    CylIOLspinbox   ->setSingleStep(0.50);
    CylIOLspinbox   ->setValue(0.00);
    choixCylIOLLay  ->addWidget(lblCylIOL);
    choixCylIOLLay  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixCylIOLLay  ->addWidget(CylIOLspinbox);
    choixCylIOLLay  ->setSpacing(5);
    choixCylIOLLay  ->setContentsMargins(0,0,0,0);

    connect(IOLchk, &QCheckBox::stateChanged, this, &dlg_programmationinterventions::AfficheChoixIOL);

    box_IOLlay      ->insertLayout(0, choixCylIOLLay);
    box_IOLlay      ->insertLayout(0, choixPwrIOLLay);
    box_IOLlay      ->insertLayout(0, choixIOLLay);
    wdg_IOL         ->setLayout(box_IOLlay);
    wdg_IOL         ->setVisible(false);

    dlg_intervention->dlglayout()   ->insertWidget(0, wdg_IOL);
    dlg_intervention->dlglayout()   ->insertLayout(0, checkIOLLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixinterventionLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixheureLay);
    dlg_intervention->dlglayout()   ->insertLayout(0, choixdateLay);
    dlg_intervention->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_intervention->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);

    dlg_intervention->exec();
}

