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

#include "dlg_bilanrecettes.h"
#include "gbl_datas.h"

dlg_bilanrecettes::dlg_bilanrecettes(QWidget *parent) :
    UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionRecettes", parent)
{
    InitOK = true;
    proc        = Procedures::I();
    db          = DataBase::I();

    CalcBilan();
    if (!InitOK)
        return;

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    gidUser     = db->getUserConnected();

    TotalMontantlbl         = new UpLabel();
    TotalReclbl             = new UpLabel();
    TotalApportlbl          = new UpLabel();
    GdTotalReclbl           = new UpLabel();
    gLabel                  = new UpLabel(Q_NULLPTR ,tr("Actes effectués par "));
    hboxsup                 = new QHBoxLayout();
    QHBoxLayout *hboxinf    = new QHBoxLayout();
    QVBoxLayout *box        = new QVBoxLayout();
    gSupervBox              = new UpComboBox();
    gSupervBox              ->setFixedWidth(130);

    TotalReclbl->setAlignment(Qt::AlignRight);
    GdTotalReclbl->setAlignment(Qt::AlignRight);
    TotalMontantlbl ->setAlignment(Qt::AlignRight);

    ClassmtupGrpBox         = new QGroupBox(tr("Classer par"));
    QFontMetrics fm         = QFontMetrics(qApp->font());
    int hauteurligne        = int(fm.height()*1.6);
    ClassmtupGrpBox         ->setFixedHeight((3*hauteurligne)+5);
    ComptableRadio          = new UpRadioButton(tr("comptabilité"));
    SupervRadio             = new UpRadioButton(tr("par soignant"));
    SupervRadio->setChecked(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(SupervRadio);
    vbox->addWidget(ComptableRadio);
    vbox->setContentsMargins(8,0,8,0);
    ClassmtupGrpBox->setLayout(vbox);

    hboxsup->addSpacerItem((new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed)));
    hboxsup->addWidget(gSupervBox);
    hboxsup->addSpacerItem((new QSpacerItem(30,5,QSizePolicy::Expanding)));
    hboxsup->addWidget(TotalMontantlbl);
    hboxsup->addSpacerItem((new QSpacerItem(50,5,QSizePolicy::Minimum)));
    hboxsup->addWidget(TotalReclbl);
    hboxsup->addSpacerItem((new QSpacerItem(5,5,QSizePolicy::Fixed)));
    hboxsup->setContentsMargins(0,0,0,0);
    hboxsup->setSpacing(0);
    hboxinf->addSpacerItem((new QSpacerItem(0,10,QSizePolicy::Expanding)));
    hboxinf->addWidget(TotalApportlbl);
    hboxinf->addSpacerItem((new QSpacerItem(0,10,QSizePolicy::Expanding)));
    hboxinf->addWidget(GdTotalReclbl);
    hboxinf->addSpacerItem((new QSpacerItem(5,5,QSizePolicy::Fixed)));
    hboxinf->setContentsMargins(0,0,0,0);
    hboxinf->setSpacing(0);
    box->addLayout(hboxsup);
    box->addSpacerItem((new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding)));
    box->addLayout(hboxinf);
    box->setContentsMargins(0,0,0,0);
    box->setSpacing(0);

    ChxPeriodButt   = new UpPushButton(tr("Changer la période"));
    ExportButt      = new UpPushButton(tr("Exporter la table"));
    int h = 40;
    ChxPeriodButt->setMinimumHeight(h);
    ExportButt->setMinimumHeight(h);
    ChxPeriodButt->setIcon(Icons::icDate());
    ExportButt->setIcon(Icons::icSauvegarder());
    int l = 20;
    ChxPeriodButt->setIconSize(QSize(l,l));
    ExportButt->setIconSize(QSize(l,l));
    AjouteWidgetLayButtons(ExportButt,false);
    AjouteWidgetLayButtons(ChxPeriodButt, false);
    setStageCount(1);

    glblbox = new QHBoxLayout();
    glblbox->addLayout(box);

    QString Titre;
    if (Debut == Fin)
        Titre = tr("Bilan des recettes pour la journée du ") + Debut.toString(tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des recettes pour la période du ") + Debut.toString(tr("d MMMM yyyy")) + tr(" au ") + Fin.toString(tr("d MMMM yyyy"));
    setWindowTitle(Titre);
    AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonClose);
    gMode = SUPERVISEUR;
    gBigTable = new UpTableView();

    RemplitLaTable();
    CalcSuperviseursEtComptables();
    FiltreTable(-1);
    dlglayout()->insertLayout(0,glblbox);
    dlglayout()->insertWidget(0,gBigTable);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(CloseButton,        &QPushButton::clicked,                                  this, [=] {accept();});
    connect(PrintButton,        &QPushButton::clicked,                                  this, [=] {ImprimeEtat();});
    connect(ChxPeriodButt,      &QPushButton::clicked,                                  this, [=] {NouvPeriode();});
    connect(ExportButt,         &QPushButton::clicked,                                  this, [=] {ExportTable();});
    connect(gSupervBox,         QOverload<int>::of(&QComboBox::currentIndexChanged),    this, [=] {FiltreTable(gSupervBox->currentData().toInt());});
    connect(ComptableRadio,     &UpRadioButton::clicked,                                this, [=] {ChangeMode(COMPTABLE);});
    connect(SupervRadio,        &UpRadioButton::clicked,                                this, [=] {ChangeMode(SUPERVISEUR);});
}

dlg_bilanrecettes::~dlg_bilanrecettes()
{
}

bool dlg_bilanrecettes::getInitOK()
{
    return InitOK;
}

void dlg_bilanrecettes::CalcBilan()
{
    QMap<QString, QDate> DateMap = proc->ChoixDate();
    if (!DateMap.isEmpty())
    {
        Debut = DateMap["DateDebut"];
        Fin   = DateMap["DateFin"];
        Datas::I()->recettes->initListe(DateMap);
        if (Datas::I()->recettes->recettes()->size() == 0)
        {
            InitOK = false;
            UpMessageBox::Watch(this,tr("Pas de recette enregistrée pour cette période"));
            return;
        }
    }
    else
    {
        InitOK = false;
        return;
    }
    InitOK = true;
    return;
}

void dlg_bilanrecettes::FiltreTable(int idx)
{
    if (gMode==SUPERVISEUR)
    {
        if (idx==-1)
        {
            for(int i=0; i<m_recettesmodel->rowCount(); i++)
            {
                Recette* rec = getRecetteFromRow(i);
                if (rec == Q_NULLPTR)
                    gBigTable->setRowHidden(i,true);
                else
                    gBigTable->setRowHidden(i, rec->idacte() == -1);
            }
        }
        else
        {
            QMap<int, User*>::iterator userFind = Datas::I()->users->superviseurs()->find(idx);
            if( userFind != Datas::I()->users->superviseurs()->end() )
            {
                for(int i=0; i<m_recettesmodel->rowCount(); i++)
                {
                    Recette* rec = getRecetteFromRow(i);
                    if (rec == Q_NULLPTR)
                        gBigTable->setRowHidden(i,true);
                    else
                        gBigTable->setRowHidden(i, rec->iduser() != idx || rec->idacte() == -1);
                }
            }
        }
        PrintButton->setEnabled(gSupervBox->currentData().toInt()>-1  && gBigTable->rowNoHiddenCount()>0);
        gBigTable->setColumnHidden(6,true);    // divers et autres recettes
        gBigTable->setColumnHidden(7,true);    // apport pratcien
    }
    else if (gMode==COMPTABLE)
    {
            QMap<int, User*>::iterator userFind = Datas::I()->users->comptables()->find(idx);
            if( userFind != Datas::I()->users->comptables()->end() )
            {
                for(int i=0; i<m_recettesmodel->rowCount(); i++)
                {
                    Recette* rec = getRecetteFromRow(i);
                    if (rec == Q_NULLPTR)
                        gBigTable->setRowHidden(i,true);
                    else
                    {
                        bool a =  rec->idcomptable() != idx
                                            || (rec->encaissement() == 0.0
                                            && rec->encaissementautrerecette() == 0.0);
                         gBigTable->setRowHidden(i, a);
                    }
                }
            }
        PrintButton->setEnabled(gBigTable->rowNoHiddenCount()>0);
        gBigTable->setColumnHidden(6,false);    // divers et autres recettes
        gBigTable->setColumnHidden(7,false);    // apport pratcien
    }
    ExportButt->setEnabled(gBigTable->rowNoHiddenCount()>0);
    ExportButt->setVisible(!(gMode==SUPERVISEUR && gSupervBox->currentData().toInt()==-1));
    gBigTable->FixLargeurTotale();
    CalculeTotal();
}

Recette* dlg_bilanrecettes::getRecetteFromIndex(QModelIndex idx)
{
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_recettesmodel->itemFromIndex(idx));
    if (upitem != Q_NULLPTR)
        return dynamic_cast<Recette *>(upitem->item());
    else
        return Q_NULLPTR;
}

Recette* dlg_bilanrecettes::getRecetteFromRow(int row)
{
    QModelIndex idx = m_recettesmodel->index(row, 0);
    return getRecetteFromIndex(idx);

}

Recette* dlg_bilanrecettes::getRecetteFromSelectionInTable()
{
    if (gBigTable->selectionModel()->selectedIndexes().size() == 0)
        return Q_NULLPTR;
    QModelIndex idx  = gBigTable->selectionModel()->selectedIndexes().at(0);
    return getRecetteFromIndex(idx);
}

void dlg_bilanrecettes::ImprimeEtat()
{
    QString            Entete, Pied;
    bool AvecDupli   = false;
    bool AvecPrevisu = true;
    bool AvecNumPage = false;

    User *userEntete = Q_NULLPTR;

    //création de l'entête
    if (gMode==SUPERVISEUR)
        userEntete = Datas::I()->users->getById(gSupervBox->currentData().toInt(), Item::LoadDetails);
    else
        userEntete = Datas::I()->users->getById(gidUser->id(), Item::LoadDetails);

    if(userEntete == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête") , tr("Annulation de l'impression"));
        return;
    }
    Entete = proc->ImpressionEntete(QDate::currentDate(), userEntete).value("Norm");
    if (Entete == "") return;

    // NOTE : POURQUOI mettre ici "PRENOM PATIENT" alors que ce sont les données d'un User qui sont utilisées ???
    // REP : parce qu'on utilise le même entête que pour les ordonnances et qu'on va substituer les champs patient dans cet entête.
    // on pourrait faire un truc plus élégant (un entête spécifique pour cet état p.e.) mais je n'ai pas eu le temps de tout faire.
    if (gMode == SUPERVISEUR)
        Entete.replace("{{PRENOM PATIENT}}"    , (gSupervBox->currentData().toInt()>0? Datas::I()->users->getLoginById(gSupervBox->currentData().toInt()): tr("Bilan global")));
    else
        Entete.replace("{{PRENOM PATIENT}}"    , "");
    Entete.replace("{{NOM PATIENT}}"       , "");
    Entete.replace("{{TITRE1}}"            , windowTitle());
    Entete.replace("{{TITRE}}"             , "");
    Entete.replace("{{DDN}}"               , (gMode == SUPERVISEUR? TotalMontantlbl->text() : ""));

    // création du pied
    Pied = proc->ImpressionPied(userEntete);
    if (Pied == "") return;

    // creation du corps de la remise
    QString couleur = "<font color = \"" COULEUR_TITRES "\">";
    double c = CORRECTION_td_width;
    QTextEdit *Etat_textEdit = new QTextEdit;
    QString test4 = "<html><head><style type=\"text/css\">p.p1 {font:70px; margin: 0px 0px 10px 100px;}"
                    "</style></head>"
                    "<body LANG=\"fr-FR\" DIR=\"LTR\">"
                    "<table width=\"" + QString::number(int(c*510)) + "\" border=\"1\"  cellspacing=\"0\" cellpadding=\"2\">";
    int row = 1;
    for (int i = 0; i < m_recettesmodel->rowCount();i++)
    {
        if (!gBigTable->isRowHidden(i))
        {
            if (gMode == SUPERVISEUR)
            {
                Recette* rec = getRecetteFromRow(i);
                if (rec != Q_NULLPTR)
                    if (rec->cotationacte() != "")
                    {
                        test4 += "<tr>"
                                 "<td width=\"" + QString::number(int(c*30))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</div></span></td>"                      //! no ligne
                                "<td width=\"" + QString::number(int(c*60))  + "\"><span style=\"font-size:8pt\">" + rec->date().toString(tr("d MMM yyyy")) + "</span></font></td>"                         //! date
                                 "<td width=\"" + QString::number(int(c*160)) + "\"><span style=\"font-size:8pt\">" + rec->payeur() + "</span></td>"                                                        //! nom prenom
                                 "<td width=\"" + QString::number(int(c*140)) + "\"><span style=\"font-size:8pt\">" + rec->cotationacte() + "</span></td>"                                                  //! cotation
                                 "<td width=\"" + QString::number(int(c*95))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QLocale().toString(rec->montant(),'f',2) + "</div></span></td>"  //! montant
                                 "</tr>";
                        row++;
                    }
            }
            else if (gMode == COMPTABLE)
            {
                test4 += "<tr>";
                Recette* rec = getRecetteFromRow(i);
                if (rec != Q_NULLPTR)
                {
                    if (rec->isapportpraticien())                                      //! ----   c'est un apport praticien
                    {
                        test4 +=    "<td width=\"" + QString::number(int(c*25))  + "\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">"
                                + QString::number(row) + "</div></span></font></td>"                                                     //! no ligne
                                "<td width=\"" + QString::number(int(c*60))  + "\">" + couleur + "<span style=\"font-size:8pt\">"
                                + rec->date().toString(tr("d MMM yyyy")) + "</span></font></td>"                                         //! date
                                "<td width=\"" + QString::number(int(c*180)) + "\">" + couleur + "<span style=\"font-size:8pt\">"
                                + rec->payeur() + "</span></font></td>"                                                                  //! libelle apport praticien
                                "<td width=\"" + QString::number(int(c*115)) + "\">" + couleur + "<span style=\"font-size:8pt\">"
                                + tr("apport praticien") + "</span></font></td>"                                                         //! apport praticien
                                "<td width=\"" + QString::number(int(c*50))  + "\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">"
                                + QLocale().toString(rec->encaissementautrerecette(), 'f',2) + "</div></span></font></td>"               //! montant
                                "<td width=\"" + QString::number(int(c*75))  + "\">" + couleur + "<span style=\"font-size:8pt\">"
                                + Utils::ConvertitModePaiement(rec->modepaiement()) + "</span></font></td>";                             //! mode de paiement
                    }
                    else if (rec->montant()==0.0 && !rec->isapportpraticien())          //! ----   c'est un tiers payant
                    {
                        test4 += "<td width=\"" + QString::number(int(c*25))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">"
                                + QString::number(row) + "</span></td>"                                                                 //! no ligne
                                "<td width=\"" + QString::number(int(c*60))  + "\"><span style=\"font-size:8pt\">"
                                + rec->date().toString(tr("d MMM yyyy")) + "</span></td>"                                               //! date
                                "<td width=\"" + QString::number(int(c*180)) + "\"><span style=\"font-size:8pt\">"
                                + rec->payeur() + "</span></td>"                                                                        //! nom prenom ou libelle si recette speciale
                                "<td width=\"" + QString::number(int(c*115)) + "\"><span style=\"font-size:8pt\">-</span></td>";                       //! vide (on est obligé de mettre un "-" parce que sinon la hauteur de ligne est fausse)
                        test4 += "<td width=\"" + QString::number(int(c*50))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">"
                                + QLocale().toString(rec->encaissement(), 'f', 2) + "</div></span></td>";                               //! montant
                        test4 += "<td width=\"" + QString::number(int(c*75))  + "\"><span style=\"font-size:8pt\">"
                                + Utils::ConvertitModePaiement(rec->modepaiement()) + "</span></font></td>";                            //! mode de paiement

                    }
                    else                                                                                        //! ----   c'est une recette
                    {
                        test4 += "<td width=\"" + QString::number(int(c*25))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">"
                                + QString::number(row) + "</span></td>"                                                                 //! no ligne
                                "<td width=\"" + QString::number(int(c*60))  + "\"><span style=\"font-size:8pt\">"
                                + rec->date().toString(tr("d MMM yyyy")) + "</span></td>"                                               //! date
                                "<td width=\"" + QString::number(int(c*180)) + "\"><span style=\"font-size:8pt\">"
                                + rec->payeur() + "</span></td>";                                                                       //! nom prenom ou libelle si recette speciale
                        if (rec->isautrerecette())
                                test4 += "<td width=\"" + QString::number(int(c*115)) + "\"><span style=\"font-size:8pt\">"
                                + tr("divers et autres recettes") + "</span></td>";
                        else
                            test4 += "<td width=\"" + QString::number(int(c*115)) + "\"><span style=\"font-size:8pt\">"
                                + rec->cotationacte() + "</span></td>";
                        if (rec->isautrerecette())
                            test4 += "<td width=\"" + QString::number(int(c*50)) + "\"><span style=\"font-size:8pt\"><div align=\"right\">"
                                    + QLocale().toString(rec->encaissementautrerecette(), 'f', 2) + "</div></span></td>";
                        else
                            test4 += "<td width=\"" + QString::number(int(c*50)) + "\"><span style=\"font-size:8pt\"><div align=\"right\">"
                                    + QLocale().toString(rec->encaissement(), 'f', 2) + "</div></span></td>";
                        test4 += "<td width=\"" + QString::number(int(c*75))  + "\"><span style=\"font-size:8pt\">"
                                + Utils::ConvertitModePaiement(rec->modepaiement()) + "</span></font></td>";                            //! mode de paiement
                    }
                    row++;
                }
            }
        }
    }
    test4 += "</table>";
    if (gMode==COMPTABLE)
    {
        test4 +=    "<table width  =\"" + QString::number(int(c*510)) + "\" border=\"0\"  cellspacing=\"0\" cellpadding=\"2\">";
        test4 +=    "<tr>"
                    "<tr><td width =\"" + QString::number(int(c*125)) + "\">" + couleur + "<span style=\"font-size:8pt\">" + tr("Total apports praticien") + "</span></font></td>"
                                                                                                                                                             "<td width     =\"" + QString::number(int(c*125)) + "\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(TotalApport,'f',2) + "</div></span></font></font></td>";
        test4 +=    "<tr><td width =\"" + QString::number(int(c*125)) + "\"><span style=\"font-size:8pt\">" + tr("Total recettes") + "</span></td>"
                    "<td width     =\"" + QString::number(int(c*125)) + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(TotalRecu,'f',2) + "</div></span></td>"
                    "<td width     =\"" + QString::number(int(c*255)) + "\"><span style=\"font-size:8pt\">(Espèces = " + QString::number(TotalRecEsp,'f',2) + ", Banque = " + QString::number(TotalRecBanq,'f',2) + ")</span></td>";
        test4 +=    "<tr><td width =\"" + QString::number(int(c*125)) + "\"><span style=\"font-size:8pt\">" + tr("Total autres recettes") + "</span></td>"
                    "<td width     =\"" + QString::number(int(c*125)) + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(TotalAutresRec,'f',2) + "</div></span></td>"
                    "<td width     =\"" + QString::number(int(c*255)) + "\"><span style=\"font-size:8pt\">(Espèces = " + QString::number(TotalAutresRecEsp,'f',2) + ", Banque = " + QString::number(TotalAutresRecBanq,'f',2) + ")</span></td>";
        test4 +=    "<tr><td width =\"" + QString::number(int(c*125)) + "\"><span style=\"font-size:8pt;font-weight:bold\">" + tr("Total général recettes") + "</span></td>"
                    "<td width     =\"" + QString::number(int(c*125)) + "\"><span style=\"font-size:8pt;font-weight:bold\"><div align=\"right\">" + QString::number(GdTotalBanq + GdTotalEsp,'f',2) + "</div></span></td>"
                    "<td width     =\"" + QString::number(int(c*255)) + "\"><span style=\"font-size:8pt;font-weight:bold\">(Espèces = " + QString::number(GdTotalEsp,'f',2) + ", Banque = " + QString::number(GdTotalBanq,'f',2) + ")</span></td>";
        test4 += "</table>";
    }
    test4 += "</body></html>";

    Etat_textEdit->setHtml(test4);

    proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
    delete Etat_textEdit;
    Etat_textEdit = Q_NULLPTR;
}

void dlg_bilanrecettes::CalcSuperviseursEtComptables()
{
    hboxsup->removeWidget(gLabel);
    glblbox->removeWidget(ClassmtupGrpBox);
    gSupervBox->clear();

    QList <int> listiD; // la liste des superviseurs
    bool idcomptabletrouve = false;

    for (int i=0; i< m_recettesmodel->rowCount(); i++)
    {
        Recette* rec = getRecetteFromRow(i);
        if (rec == Q_NULLPTR)
            continue;
        else
        {
            if (!listiD.contains(rec->iduser()) && rec->iduser() > 0)
                listiD << rec->iduser();
            if (!idcomptabletrouve)
                idcomptabletrouve = (rec->idcomptable() == gidUser->id());
        }
    }
    if( listiD.size() > 1 )
        gSupervBox->addItem(tr("Tout le monde"),-1);
    for( int i=0; i<listiD.size(); i++)
        gSupervBox->addItem(Datas::I()->users->getLoginById( listiD.at(i)), QString::number(listiD.at(i)) );

    if (idcomptabletrouve)
        glblbox->insertWidget(0, ClassmtupGrpBox);
    else
        hboxsup->insertWidget(0, gLabel);
}

void dlg_bilanrecettes::CalculeTotal()
{
    TotalMontant = 0;
    TotalRecu    = 0;
    TotalRecEsp     = 0;
    TotalRecBanq    = 0;
    int    nbreActes    = 0;
    TotalApportlbl      ->setVisible(gMode==COMPTABLE);
    GdTotalReclbl       ->setVisible(gMode==COMPTABLE);
    if (m_recettesmodel->rowCount() > 0)
    {
        if (gMode == SUPERVISEUR)
        {
            for (int k = 0; k < m_recettesmodel->rowCount(); k++)
            {
                if (!gBigTable->isRowHidden(k))
                {
                    Recette * rec = getRecetteFromRow(k);
                    if(rec->montant()>0)
                    {
                        TotalMontant    += rec->montant();
                        nbreActes++;
                    }
                    TotalRecu           += rec->encaissement();
                    if(rec->modepaiement()  == "E")
                        TotalRecEsp     += rec->encaissement();
                    else
                        TotalRecBanq    += rec->encaissement();
                }
            }
            TotalMontantlbl ->setText(tr("Total ") + QString::number(nbreActes) + (nbreActes>1? tr(" actes ") : tr(" acte ")) + QLocale().toString(TotalMontant,'f',2));
            TotalReclbl     ->setText(tr("Total reçu ") + QLocale().toString(TotalRecu,'f',2)
                                        + "\n(" + tr("Espèces") + ": " + QLocale().toString(TotalRecEsp,'f',2) + " - "
                                        + tr("Banque")  + ": " + QLocale().toString(TotalRecBanq,'f',2) + ")");
        }
        if (gMode == COMPTABLE)
        {
            TotalApport     = 0;
            TotalAutresRec  = 0;
            TotalAutresRecEsp   = 0;
            TotalAutresRecBanq  = 0;
            GdTotalEsp      = 0;
            GdTotalBanq     = 0;
            for (int k = 0; k < m_recettesmodel->rowCount(); k++)
            {
                if (!gBigTable->isRowHidden(k))
                {
                    nbreActes++;
                    Recette * rec           = getRecetteFromRow(k);
                    TotalRecu               += rec->encaissement();
                    TotalApport             += (rec->isapportpraticien()? rec->encaissementautrerecette() : 0.0);
                    TotalAutresRec          += (rec->isautrerecette()? rec->encaissementautrerecette() : 0.0);
                    if(rec->modepaiement()  == "E")
                    {
                        TotalRecEsp         += rec->encaissement();
                        if (rec->isautrerecette())
                            TotalAutresRecEsp   += rec->encaissementautrerecette();
                    }
                    else
                    {
                        TotalRecBanq           += rec->encaissement();
                        if (rec->isautrerecette())
                            TotalAutresRecBanq  += rec->encaissementautrerecette();
                    }
                }
            }
            GdTotalBanq = TotalRecBanq + TotalAutresRecBanq;
            GdTotalEsp  = TotalRecEsp + TotalAutresRecEsp;
            TotalMontantlbl ->setText(tr("Total ") + QString::number(nbreActes) + (nbreActes>1? tr(" lignes ") : tr(" ligne ")));
            TotalReclbl     ->setText(tr("Total recettes ") + QLocale().toString(TotalRecu,'f',2)
                                        + "\n(" + tr("Espèces") + ": " + QLocale().toString(TotalRecEsp,'f',2) + " - "
                                        + tr("Banque")  + ": " + QLocale().toString(TotalRecBanq,'f',2) + ")");
            TotalApportlbl      ->setVisible(TotalApport>0.0);
            GdTotalReclbl       ->setVisible(TotalAutresRec>0.0);
            if ((TotalApport>0.0))
                TotalApportlbl->setText(tr("Total apports praticien ") + QLocale().toString(TotalApport,'f',2));
            if ((TotalAutresRec>0.0))
            {
                GdTotalReclbl->setText(tr("Total autres recettes ") + QLocale().toString(TotalAutresRec,'f',2)
                                       + "\n" + tr("Total général recettes ") + QLocale().toString(TotalAutresRec + TotalRecu,'f',2)
                                       + "\n(" + tr("Espèces") + ": " + QLocale().toString(GdTotalEsp,'f',2) + " - "
                                       + tr("Banque")  + ": " + QLocale().toString(GdTotalBanq,'f',2) + ")");
            }
        }
    }
}

void dlg_bilanrecettes::ChangeMode(enum gMode Mode)
{
    gMode = Mode;
    gSupervBox      ->setVisible(gMode == SUPERVISEUR);
    int hauteurrow      = int(QFontMetrics(qApp->font()).height()*1.3);
    gBigTable->horizontalHeader()->setFixedHeight(hauteurrow*(gMode==SUPERVISEUR? 1 : 2));
    if (gMode == SUPERVISEUR)
        FiltreTable(gSupervBox->currentData().toInt());
    if (gMode == COMPTABLE)
        FiltreTable(gidUser->id());
}

void dlg_bilanrecettes::ExportTable()
{
    QByteArray ExportEtat;
    QString sep = "\t";                                                                                                             // séparateur
    if (UpMessageBox::Question(this,
                               tr("Exportation de la table des recettes"),
                               tr("Voulez-vous exporter la table des recettes?\n(Jeu de caractères Unicode (UTF8), Format CSV, langue anglais(USA), champs séparés par tabulation)"))
            != UpSmallButton::STARTBUTTON)
        return;
    ExportEtat.append(tr("Date") + sep + tr("Nom") + sep + tr("Type acte") + sep + tr("Montant") + sep + tr("Mode de paiement") + sep + tr("Reçu") + sep + tr("Divers et autres recettes") + sep + tr("Apport praticien") + "\n");
    for (int i=0;i< m_recettesmodel->rowCount(); i++)
    {
        if (!gBigTable->isRowHidden(i))
        {
            Recette * rec           = getRecetteFromRow(i);
            QString date = rec->date().toString("dd/MM/yyyy");
            ExportEtat.append(date + sep);                                                                  // Date
            ExportEtat.append(rec->payeur() + sep);                                                         // NomPrenom
            ExportEtat.append(rec->cotationacte() + sep);                                                   // Cotation
            ExportEtat.append(QString::number(rec->montant()) + sep);                                       // Montant
            ExportEtat.append(Utils::ConvertitModePaiement(rec->modepaiement()) + sep);                     // Mode de paiement
            ExportEtat.append(QString::number(rec->encaissement()) + sep);                                  // Reçu
            double diversrec = (rec->isautrerecette()? rec->encaissementautrerecette() : 0.0);
            ExportEtat.append(QString::number(diversrec) + sep);                                            // divers et eutres recettes
            double apportprat = (rec->isapportpraticien()? rec->encaissementautrerecette() : 0.0);
            ExportEtat.append(QString::number(apportprat) + sep);                                           // Apports praticien
            ExportEtat.append("\n");
        }
    }
    QString ExportFileName = QDir::homePath() + DIR_RUFUS + "/"
                            + (gMode == COMPTABLE? tr("Recettes") + " " + gidUser->getLogin() : tr("Actes") + " " + gSupervBox->currentText())
                            + " " + tr("du") + " " + Debut.toString("d MMM yyyy") + " " + tr("au") + " " + Fin.toString(tr("d MMM yyyy"))
                            + ".csv";
    QFile   ExportFile(ExportFileName);
    bool exportOK = true;
    QString msg = tr("Nom du fichier d'export") + "\n" + ExportFileName;
    ExportFile.remove();
    if (ExportFile.open(QIODevice::Append))
    {
        QTextStream out(&ExportFile);
        out << ExportEtat;
        ExportFile.close();
    }
    else
        exportOK = false;
    UpMessageBox::Watch(this, (exportOK? tr("Exportation réussie") : tr("Echec exportation")), (exportOK? msg : tr("Les données n'ont pas pu être exportées")));
}

void dlg_bilanrecettes::NouvPeriode()
{
    QDate debutavant, finavant;
    CalcBilan();
    if (!InitOK)
    {
        Debut   = debutavant;
        Fin     = finavant;
        InitOK  = true;
        return;
    }

    QString Titre;
    if (Debut == Fin)
        Titre = tr("Bilan des recettes pour la journée du ") + Debut.toString(tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des recettes pour la période du ") + Debut.toString(tr("d MMMM yyyy")) + tr(" au ") + Fin.toString(tr("d MMMM yyyy"));
    setWindowTitle(Titre);

    RemplitLaTable();
    CalcSuperviseursEtComptables();
    SupervRadio->click();
}

void dlg_bilanrecettes::RemplitLaTable()
{
    UpStandardItem *pitem0, *pitem1, *pitem2, *pitem3, *pitem4, *pitem5,*pitem6,*pitem7;
    m_recettesmodel = dynamic_cast<QStandardItemModel*>(gBigTable->model());
    if (m_recettesmodel != Q_NULLPTR)
        m_recettesmodel->clear();
    else
        m_recettesmodel = new QStandardItemModel;
    QMapIterator<int, Recette*> itrec(*Datas::I()->recettes->recettes());
    while (itrec.hasNext())
    {
        Recette *rec = itrec.next().value();

        pitem0 = new UpStandardItem(rec->date().toString(tr("d MMM yyyy")));                        // Date - col = 0
        pitem0->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        pitem0->setItem(rec);

        pitem1 = new UpStandardItem(rec->payeur());                                                 // NomPrenom - col = 1
        pitem1->setItem(rec);

        pitem2 = new UpStandardItem(rec->cotationacte());                                           // Cotation - col = 2;
        pitem2->setItem(rec);

        QString A;
        if (rec->monnaie() == "F")
            A = QLocale().toString(rec->montant()/6.55957,'f',2);                                   // Montant en F converti en euros
        else
            A = QLocale().toString(rec->montant(),'f',2);                                           // Montant - col = 3
        pitem3 = new UpStandardItem(A);
        pitem3->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        pitem3->setItem(rec);

        A = rec->modepaiement();                                                                    // Mode de paiement - col = 4
        QString B = A;
        if (A == "T")
            B = rec->typetiers();
        pitem4 = new UpStandardItem(Utils::ConvertitModePaiement(B));
        pitem4->setItem(rec);

        double C = rec->encaissement();                                                             // Reçu- col = 5
        pitem5 = new UpStandardItem(QLocale().toString(C,'f',2));
        pitem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        pitem5->setItem(rec);

        pitem6 = new UpStandardItem("0,00");                                                        // Divers et autres recettes - col = 6
        if (rec->isautrerecette())
            pitem6->setText(QLocale().toString(rec->encaissementautrerecette(),'f',2));
        pitem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        pitem6->setItem(rec);

        pitem7 = new UpStandardItem("0,00");                                                       // Apport praticien - col = 7
        if (rec->isapportpraticien())
            pitem7->setText(QLocale().toString(rec->encaissementautrerecette(),'f',2));
        pitem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        pitem7->setItem(rec);

        m_recettesmodel->appendRow(QList<QStandardItem*>() << pitem0 << pitem1 << pitem2 << pitem3 << pitem4 << pitem5 << pitem6 << pitem7);
    }
    int hauteurrow      = int(QFontMetrics(qApp->font()).height()*1.3);
    int nbrowsAAfficher = 30;
    gBigTable->setFixedHeight(nbrowsAAfficher*hauteurrow+2);

    gBigTable->setModel(m_recettesmodel);

    gBigTable->setPalette(QPalette(Qt::white));
    gBigTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    gBigTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gBigTable->verticalHeader()->setVisible(false);
    gBigTable->setFocusPolicy(Qt::StrongFocus);
    gBigTable->setSelectionMode(QAbstractItemView::SingleSelection);

    QStandardItem *itdate = new QStandardItem();
    itdate->setText(tr("Date"));
    itdate->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_recettesmodel->setHorizontalHeaderItem(0,itdate);
    QStandardItem *itnom = new QStandardItem();
    itnom->setText(tr("Nom"));
    itnom->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_recettesmodel->setHorizontalHeaderItem(1,itnom);
    QStandardItem *ittypeacte = new QStandardItem();
    ittypeacte->setText(tr("Type acte"));
    ittypeacte->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_recettesmodel->setHorizontalHeaderItem(2,ittypeacte);
    QStandardItem *itmontant = new QStandardItem();
    itmontant->setText(tr("Montant"));
    itmontant->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_recettesmodel->setHorizontalHeaderItem(3,itmontant);
    QStandardItem *itmodepaiement = new QStandardItem();
    itmodepaiement->setText(tr("Mode de paiement"));
    itmodepaiement->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_recettesmodel->setHorizontalHeaderItem(4,itmodepaiement);
    QStandardItem *itpaye = new QStandardItem();
    itpaye->setText(tr("Reçu"));
    itpaye->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_recettesmodel->setHorizontalHeaderItem(5,itpaye);
    QStandardItem *itdivers = new QStandardItem();
    itdivers->setText(tr("Divers et\nautres recettes"));
    itdivers->setTextAlignment(Qt::AlignCenter);
    m_recettesmodel->setHorizontalHeaderItem(6,itdivers);
    QStandardItem *itapports = new QStandardItem();
    itapports->setText(tr("Apports\npraticiens"));
    itdivers->setTextAlignment(Qt::AlignCenter);
    m_recettesmodel->setHorizontalHeaderItem(7,itapports);

    gBigTable->horizontalHeader()->setVisible(true);
    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    gBigTable->setColumnWidth(li,100);                                              // 0 - Date affichage européen
    li++;
    gBigTable->setColumnWidth(li,240);                                              // 1 - Nom
    li++;
    gBigTable->setColumnWidth(li,140);                                              // 2 - TypeActe
    li++;
    gBigTable->setColumnWidth(li,85);                                               // 3 - Montant
    li++;
    gBigTable->setColumnWidth(li,130);                                              // 4 - Mode de paiement
    li++;
    gBigTable->setColumnWidth(li,85);                                               // 5 - Reçu
    li++;
    gBigTable->setColumnWidth(li,120);                                              // 6 - Divers et autres recettes
    li++;
    gBigTable->setColumnWidth(li,120);                                              // 7 - Apport praticien

    gBigTable->setGridStyle(Qt::SolidLine);
    QFontMetrics fm(qApp->font());
    for (int j=0; j<Datas::I()->recettes->recettes()->size(); j++)
         gBigTable->setRowHeight(j,int(fm.height()*1.3));

    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    //gBigTable->horizontalHeader()->setFixedHeight(int(fm.height()*2));
}

