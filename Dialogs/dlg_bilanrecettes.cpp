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

#include "dlg_bilanrecettes.h"


dlg_bilanrecettes::dlg_bilanrecettes(QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionRecettes", parent)
{
    CalcBilan();
    if (!m_initok)
        return;

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    wdg_totalmontantlbl     = new UpLabel();
    wdg_totalrecetteslbl    = new UpLabel();
    wdg_totalapportslbl     = new UpLabel();
    wdg_grandtotallbl       = new UpLabel();
    QWidget* wdg = Q_NULLPTR;
    wdg_label               = new UpLabel(wdg ,tr("Actes effectués par "));
    wdg_hboxsup             = new QHBoxLayout();
    QHBoxLayout *hboxinf    = new QHBoxLayout();
    QVBoxLayout *box        = new QVBoxLayout();
    wdg_supervcombobox      = new UpComboBox();
    wdg_supervcombobox      ->setFixedWidth(130);

    wdg_totalrecetteslbl    ->setAlignment(Qt::AlignRight);
    wdg_grandtotallbl       ->setAlignment(Qt::AlignRight);
    wdg_totalmontantlbl     ->setAlignment(Qt::AlignRight);

    wdg_classmtupgrpbox     = new QGroupBox(tr("Classer par"));
    QFontMetrics fm         = QFontMetrics(qApp->font());
    int hauteurligne        = int(fm.height()*1.6);
    wdg_classmtupgrpbox     ->setFixedHeight((3*hauteurligne)+5);
    wdg_comptableradiobouton    = new UpRadioButton(tr("comptabilité"));
    wdg_superviseurradiobouton  = new UpRadioButton(tr("par soignant"));
    wdg_superviseurradiobouton  ->setChecked(true);
    QVBoxLayout *vbox       = new QVBoxLayout;
    vbox                    ->addWidget(wdg_superviseurradiobouton);
    vbox                    ->addWidget(wdg_comptableradiobouton);
    vbox                    ->setContentsMargins(8,0,8,0);
    wdg_classmtupgrpbox     ->setLayout(vbox);

    wdg_hboxsup ->addSpacerItem((new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed)));
    wdg_hboxsup ->addWidget(wdg_supervcombobox);
    wdg_hboxsup ->addSpacerItem((new QSpacerItem(30,5,QSizePolicy::Expanding)));
    wdg_hboxsup ->addWidget(wdg_totalmontantlbl);
    wdg_hboxsup ->addSpacerItem((new QSpacerItem(50,5,QSizePolicy::Minimum)));
    wdg_hboxsup ->addWidget(wdg_totalrecetteslbl);
    wdg_hboxsup ->addSpacerItem((new QSpacerItem(5,5,QSizePolicy::Fixed)));
    wdg_hboxsup ->setContentsMargins(0,0,0,0);
    wdg_hboxsup ->setSpacing(0);
    hboxinf     ->addSpacerItem((new QSpacerItem(0,10,QSizePolicy::Expanding)));
    hboxinf     ->addWidget(wdg_totalapportslbl);
    hboxinf     ->addSpacerItem((new QSpacerItem(0,10,QSizePolicy::Expanding)));
    hboxinf     ->addWidget(wdg_grandtotallbl);
    hboxinf     ->addSpacerItem((new QSpacerItem(5,5,QSizePolicy::Fixed)));
    hboxinf     ->setContentsMargins(0,0,0,0);
    hboxinf     ->setSpacing(0);
    box         ->addLayout(wdg_hboxsup);
    box         ->addSpacerItem((new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding)));
    box         ->addLayout(hboxinf);
    box         ->setContentsMargins(0,0,0,0);
    box         ->setSpacing(0);

    wdg_choixperiodebouton  = new UpPushButton(tr("Changer la période"));
    wdg_exportbouton        = new UpPushButton(tr("Exporter la table"));
    int h = 40;
    wdg_choixperiodebouton  ->setMinimumHeight(h);
    wdg_exportbouton        ->setMinimumHeight(h);
    wdg_choixperiodebouton  ->setIcon(Icons::icDate());
    wdg_exportbouton        ->setIcon(Icons::icSauvegarder());
    int l = 20;
    wdg_choixperiodebouton  ->setIconSize(QSize(l,l));
    wdg_exportbouton        ->setIconSize(QSize(l,l));

    AjouteWidgetLayButtons(wdg_exportbouton,false);
    AjouteWidgetLayButtons(wdg_choixperiodebouton, false);
    setStageCount(1);

    wdg_lblbox      = new QHBoxLayout();
    wdg_lblbox      ->addLayout(box);

    QString Titre;
    if (m_debut == m_fin)
        Titre = tr("Bilan des actes pour la journée du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des actes pour la période du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy")) + tr(" au ") + QLocale::system().toString(m_fin,tr("d MMMM yyyy"));
    setWindowTitle(Titre);
    AjouteLayButtons( UpDialog::ButtonPrint | UpDialog::ButtonClose );
    m_mode          = SUPERVISEUR;
    wdg_bigtable    = new UpTableView();

    dlglayout()     ->insertLayout(0,wdg_lblbox);
    dlglayout()     ->insertWidget(0,wdg_bigtable);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    RemplitLaTable();
    CalcSuperviseursEtComptables();
    FiltreTable(-1);
    connect(CloseButton,                &QPushButton::clicked,                                  this, [=] {close();});
    connect(PrintButton,                &QPushButton::clicked,                                  this, [&] {
                                                                                                            bool ok;
                                                                                                            if (proc->QuestionPdfOrPrint(this, ok))
                                                                                                                PrintReport(ok);
                                                                                                          });
    connect(wdg_choixperiodebouton,     &QPushButton::clicked,                                  this, [=] {NouvPeriode();});
    connect(wdg_exportbouton,           &QPushButton::clicked,                                  this, [=] {ExportTable();});
    connect(wdg_supervcombobox,         QOverload<int>::of(&QComboBox::currentIndexChanged),    this, [=] {FiltreTable(wdg_supervcombobox->currentData().toInt());});
    connect(wdg_comptableradiobouton,   &UpRadioButton::clicked,                                this, [=] {ChangeMode(COMPTABLE);});
    connect(wdg_superviseurradiobouton, &UpRadioButton::clicked,                                this, [=] {ChangeMode(SUPERVISEUR);});
}

dlg_bilanrecettes::~dlg_bilanrecettes()
{
}

bool dlg_bilanrecettes::initOK() const
{
    return m_initok;
}

void dlg_bilanrecettes::CalcBilan(QWidget *parent)
{
    QMap<Utils::Period, QDate> DateMap = proc->ChoixDate(parent);
    if (!DateMap.isEmpty())
    {
        m_debut = DateMap[Utils::Debut];
        m_fin   = DateMap[Utils::Fin];
        Datas::I()->recettes->initListe(DateMap);
        if (Datas::I()->recettes->recettes()->size() == 0)
        {
            m_initok = false;
            UpMessageBox::Watch(parent,tr("Pas de recette enregistrée pour cette période"));
            return;
        }
    }
    else
    {
        m_initok = false;
        return;
    }
    m_initok = true;
    return;
}

void dlg_bilanrecettes::FiltreTable(int idx)
{
    if (m_mode==SUPERVISEUR)
    {
        if (idx==-1)
        {
            for(int i=0; i<m_recettesmodel->rowCount(); i++)
            {
                Recette* rec = getRecetteFromRow(i);
                if (rec == Q_NULLPTR)
                    wdg_bigtable->setRowHidden(i,true);
                else
                    wdg_bigtable->setRowHidden(i, rec->idacte() == -1);
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
                        wdg_bigtable->setRowHidden(i,true);
                    else
                        wdg_bigtable->setRowHidden(i, rec->iduser() != idx || rec->idacte() == -1);
                }
            }
        }
        PrintButton ->setEnabled(wdg_supervcombobox->currentData().toInt()>-1  && wdg_bigtable->rowNoHiddenCount()>0);
        wdg_bigtable->setColumnHidden(6,true);    // divers et autres recettes
        wdg_bigtable->setColumnHidden(7,true);    // apport pratcien
    }
    else if (m_mode==COMPTABLE)
    {
        QMap<int, User*>::iterator userFind = Datas::I()->users->comptablesActes()->find(idx);
        if( userFind != Datas::I()->users->comptablesActes()->end() )
            {
                for(int i=0; i<m_recettesmodel->rowCount(); i++)
                {
                    Recette* rec = getRecetteFromRow(i);
                    if (rec == Q_NULLPTR)
                        wdg_bigtable->setRowHidden(i,true);
                    else
                    {
                        bool a =  rec->idcomptable() != idx
                                            || (rec->encaissement() == 0.0
                                            && rec->encaissementautrerecette() == 0.0);
                         wdg_bigtable->setRowHidden(i, a);
                    }
                }
            }
        PrintButton ->setEnabled(wdg_bigtable->rowNoHiddenCount()>0);
        wdg_bigtable->setColumnHidden(6,false);    // divers et autres recettes
        wdg_bigtable->setColumnHidden(7,false);    // apport pratcien
    }
    wdg_exportbouton->setEnabled(wdg_bigtable->rowNoHiddenCount()>0);
    wdg_exportbouton->setVisible(!(m_mode==SUPERVISEUR && wdg_supervcombobox->currentData().toInt()==-1));
    wdg_bigtable->FixLargeurTotale();
    CalculeTotal();
}

Recette* dlg_bilanrecettes::getRecetteFromIndex(QModelIndex idx)
{
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_recettesmodel->itemFromIndex(idx));
    if (upitem != Q_NULLPTR)
        return qobject_cast<Recette *>(upitem->item());
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
    if (wdg_bigtable->selectionModel()->selectedIndexes().size() == 0)
        return Q_NULLPTR;
    QModelIndex idx  = wdg_bigtable->selectionModel()->selectedIndexes().at(0);
    return getRecetteFromIndex(idx);
}

void dlg_bilanrecettes::PrintReport(bool pdf)
{
    QString            textentete, textpied;

    User *userEntete = Q_NULLPTR;

    //création de l'entête
    if (m_mode==SUPERVISEUR)
        userEntete = Datas::I()->users->getById(wdg_supervcombobox->currentData().toInt());
    else
        userEntete = Datas::I()->users->getById(Datas::I()->users->userconnected()->id());

    if(userEntete == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête") , tr("Annulation de l'impression"));
        return;
    }
    textentete = proc->CalcEnteteImpression(db->ServerDate(), userEntete).value("Norm");
    if (textentete == "") return;

    // NOTE : POURQUOI mettre ici "PRENOM PATIENT" alors que ce sont les données d'un User qui sont utilisées ???
    // REP : parce qu'on utilise le même entête que pour les ordonnances et qu'on va substituer les champs patient dans cet entête.
    // on pourrait faire un truc plus élégant (un entête spécifique pour cet état p.e.) mais je n'ai pas eu le temps de tout faire.
    if (m_mode == SUPERVISEUR)
        textentete.replace("{{PRENOM PATIENT}}"    , (wdg_supervcombobox->currentData().toInt()>0? Datas::I()->users->getById(wdg_supervcombobox->currentData().toInt())->login(): tr("Bilan global")));
    else
        textentete.replace("{{PRENOM PATIENT}}"    , "");
    textentete.replace("{{NOM PATIENT}}"       , "");
    textentete.replace("{{TITRE1}}"            , windowTitle());
    textentete.replace("{{TITRE}}"             , "");
    textentete.replace("{{DDN}}"               , (m_mode == SUPERVISEUR? wdg_totalmontantlbl->text() : ""));

    // création du pied
    textpied = proc->CalcPiedImpression(userEntete);
    if (textpied == "") return;

    // creation du corps
    QString couleur = "<font color = \"" COULEUR_TITRES "\">";
    QString textcorps = "<html><head><style type=\"text/css\">p.p1 {font:70px; margin: 0px 0px 10px 100px;}"
                    "</style></head>"
                    "<body LANG=\"fr-FR\" DIR=\"LTR\">"
                    "<table width=\"100%\" border=\"0.3\"  cellspacing=\"0\" cellpadding=\"2\">";
    int row = 1;
    for (int i = 0; i < m_recettesmodel->rowCount();i++)
    {
        if (!wdg_bigtable->isRowHidden(i))
        {
            if (m_mode == SUPERVISEUR)
            {
                Recette* rec = getRecetteFromRow(i);
                if (rec != Q_NULLPTR)
                    if (rec->cotationacte() != "")
                    {
                        textcorps += "<tr>"
                                     "<td width=\"6%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</div></span></td>"                       //! no ligne
                                     "<td width=\"14%\"><span style=\"font-size:8pt\">" + rec->date().toString(tr("d MMM yyyy")) + "</span></font></td>"                        //! date
                                     "<td width=\"45%\"><span style=\"font-size:8pt\">" + rec->payeur() + "</span></td>"                                                        //! nom prenom
                                     "<td width=\"23%\"><span style=\"font-size:8pt\">" + rec->cotationacte() + "</span></td>"                                                  //! cotation
                                     "<td width=\"12%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QLocale().toString(rec->montant(),'f',2) + "</div></span></td>"  //! montant
                                     "</tr>";
                        row++;
                    }
            }
            else if (m_mode == COMPTABLE)
            {
                textcorps += "<tr>";
                Recette* rec = getRecetteFromRow(i);
                if (rec != Q_NULLPTR)
                {
                    if (rec->isapportpraticien())                                      //! ----   c'est un apport praticien
                    {
                        textcorps +=    "<td width=\"5%\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">"
                                + QString::number(row) + "</div></span></font></td>"                                                     //! no ligne
                                "<td width=\"12%\">" + couleur + "<span style=\"font-size:8pt\">"
                                + QLocale::system().toString(rec->date(),tr("d MMM yyyy")) + "</span></font></td>"                       //! date
                                "<td width=\"33%\">" + couleur + "<span style=\"font-size:8pt\">"
                                + rec->payeur() + "</span></font></td>"                                                                  //! libelle apport praticien
                                "<td width=\"22%\">" + couleur + "<span style=\"font-size:8pt\">"
                                + tr("apport praticien") + "</span></font></td>"                                                         //! apport praticien
                                "<td width=\"14%\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">"
                                + QLocale().toString(rec->encaissementautrerecette(), 'f',2) + "</div></span></font></td>"               //! montant
                                "<td width=\"14%\">" + couleur + "<span style=\"font-size:8pt\">"
                                + Utils::ConvertitModePaiement(rec->modepaiement()) + "</span></font></td>";                             //! mode de paiement
                    }
                    else if (rec->montant()==0.0 && !rec->isapportpraticien())          //! ----   c'est un tiers payant
                    {
                        textcorps += "<td width=\"5%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</span></td>"                                         //! no ligne
                                "<td width=\"12%\"><span style=\"font-size:8pt\">" + QLocale::system().toString(rec->date(),tr("d MMM yyyy")) + "</span></td>"                              //! date
                                "<td width=\"33%\"><span style=\"font-size:8pt\">" + rec->payeur() + "</span></td>"                                                                         //! nom prenom ou libelle si recette speciale
                                "<td width=\"22%\"><span style=\"font-size:8pt\">-</span></td>";                                                                                            //! vide (on est obligé de mettre un "-" parce que sinon la hauteur de ligne est fausse)
                        textcorps += "<td width=\"14%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QLocale().toString(rec->encaissement(), 'f', 2) + "</div></span></td>";      //! montant
                        textcorps += "<td width=\"14%\"><span style=\"font-size:8pt\">" + Utils::ConvertitModePaiement(rec->modepaiement()) + "</span></font></td>";                        //! mode de paiement

                    }
                    else                                                                 //! ----   c'est une recette
                    {
                        textcorps += "<td width=\"5%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</span></td>"                                         //! no ligne
                                "<td width=\"12%\"><span style=\"font-size:8pt\">" + QLocale::system().toString(rec->date(),tr("d MMM yyyy")) + "</span></td>"                              //! date
                                "<td width=\"33%\"><span style=\"font-size:8pt\">" + rec->payeur() + "</span></td>";                                                                        //! nom prenom ou libelle si recette speciale
                        if (rec->isautrerecette())
                            textcorps += "<td width=\"22%\"><span style=\"font-size:8pt\">" + tr("divers et autres recettes") + "</span></td>";
                        else
                            textcorps += "<td width=\"22%\"><span style=\"font-size:8pt\">" + rec->cotationacte() + "</span></td>";                                                         //! cotation
                        if (rec->isautrerecette())
                            textcorps += "<td width=\"14%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QLocale().toString(rec->encaissementautrerecette(), 'f', 2) + "</div></span></td>";
                        else
                            textcorps += "<td width=\"14%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QLocale().toString(rec->encaissement(), 'f', 2) + "</div></span></td>";  //! montant
                        textcorps += "<td width=\"14%\"><span style=\"font-size:8pt\">" + Utils::ConvertitModePaiement(rec->modepaiement()) + "</span></font></td>";                        //! mode de paiement
                    }
                    row++;
                }
            }
        }
    }
    textcorps += "</table>";
    if (m_mode==COMPTABLE)
    {
        textcorps += "<table width  =\"100%\" border=\"0\"  cellspacing=\"0\" cellpadding=\"2\">";

        textcorps += "<tr><tr><td width =\"20%\">" + couleur + "<span style=\"font-size:8pt\">" + tr("Total apports praticien") + "</span></font></td>";
        textcorps += "<td width =\"15%\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(m_totalapport,'f',2) + "</div></span></font></font></td>";

        textcorps += "<tr><td width =\"20%\"><span style=\"font-size:8pt\">" + tr("Total recettes") + "</span></td>";
        textcorps += "<td width =\"15%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(m_totalrecu,'f',2) + "</div></span></td>";
        textcorps += "<td width =\"41%\"><span style=\"font-size:8pt\">(Espèces = " + QString::number(m_totalrecuespeces,'f',2) + ", Banque = " + QString::number(m_totalrecubanque,'f',2) + ")</span></td>";

        textcorps += "<tr><td width =\"20%\"><span style=\"font-size:8pt\">" + tr("Total autres recettes") + "</span></td>";
        textcorps += "<td width =\"15%\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(m_totalautresrecettes,'f',2) + "</div></span></td>";
        textcorps += "<td width =\"41%\"><span style=\"font-size:8pt\">(Espèces = " + QString::number(m_totalautresrecettesespeces,'f',2) + ", Banque = " + QString::number(m_totalautresrecettesbanque,'f',2) + ")</span></td>";

        textcorps += "<tr><td width =\"20%\"><span style=\"font-size:8pt;font-weight:bold\">" + tr("Total général recettes") + "</span></td>";
        textcorps += "<td width =\"15%\"><span style=\"font-size:8pt;font-weight:bold\"><div align=\"right\">" + QString::number(m_grandtotalbanqu + m_grandtotalespeces,'f',2) + "</div></span></td>";
        textcorps += "<td width =\"41%\"><span style=\"font-size:8pt;font-weight:bold\">(Espèces = " + QString::number(m_grandtotalespeces,'f',2) + ", Banque = " + QString::number(m_grandtotalbanqu,'f',2) + ")</span></td>";
        textcorps += "</table>";
    }
    textcorps += "</body></html>";

    if (pdf)
    {
        QString dirname     = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0)) + "/" + tr("Comptabilité");
        QString filename    = userEntete->prenom() + " " + userEntete->nom() + " - " + windowTitle() + ".pdf";
        QString msgOK       = tr("fichier") +" " + QDir::toNativeSeparators(filename) + "\n" +
                              tr ("sauvegardé sur le bureau dans le dossier Comptabilité");
        bool a = proc->Cree_pdf(textcorps, textentete, textpied,
                            filename,
                            dirname);
        UpMessageBox::Watch(this, (a? tr("Enregistrement pdf") : tr("Echec enregistrement pdf")),
                                   a? msgOK : tr ("Impossible d'enregistret le fichier ") + QDir::toNativeSeparators(filename));
    }
    else
    {
        bool AvecDupli   = false;
        bool AvecNumPage = false;
        proc->Imprime_Etat(this, textcorps, textentete, textpied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecNumPage);
    }
}

void dlg_bilanrecettes::CalcSuperviseursEtComptables()
{
    wdg_hboxsup->removeWidget(wdg_label);
    wdg_lblbox->removeWidget(wdg_classmtupgrpbox);
    wdg_supervcombobox->clear();

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
                idcomptabletrouve = (rec->idcomptable() == Datas::I()->users->userconnected()->id());
        }
    }
    if( listiD.size() > 1 )
        wdg_supervcombobox->addItem(tr("Tout le monde"),-1);
    for( int i=0; i<listiD.size(); i++)
        if (Datas::I()->users->getById(listiD.at(i)) != Q_NULLPTR)
            wdg_supervcombobox->addItem(Datas::I()->users->getById( listiD.at(i))->login(), QString::number(listiD.at(i)) );

    if (idcomptabletrouve)
        wdg_lblbox->insertWidget(0, wdg_classmtupgrpbox);
    else
        wdg_hboxsup->insertWidget(0, wdg_label);
}

void dlg_bilanrecettes::CalculeTotal()
{
    m_totalmontant = 0;
    m_totalrecu    = 0;
    m_totalrecuespeces     = 0;
    m_totalrecubanque    = 0;
    int    nbreActes    = 0;
    wdg_totalapportslbl      ->setVisible(m_mode==COMPTABLE);
    wdg_grandtotallbl       ->setVisible(m_mode==COMPTABLE);
    if (m_recettesmodel->rowCount() > 0)
    {
        if (m_mode == SUPERVISEUR)
        {
            for (int k = 0; k < m_recettesmodel->rowCount(); k++)
            {
                if (!wdg_bigtable->isRowHidden(k))
                {
                    Recette * rec = getRecetteFromRow(k);
                    if(rec->montant()>0)
                    {
                        m_totalmontant    += rec->montant();
                        nbreActes++;
                    }
                    m_totalrecu           += rec->encaissement();
                    if(rec->modepaiement()  == ESP)
                        m_totalrecuespeces     += rec->encaissement();
                    else
                        m_totalrecubanque    += rec->encaissement();
                }
            }
            wdg_totalmontantlbl ->setText(tr("Total ") + QString::number(nbreActes) + (nbreActes>1? tr(" actes ") : tr(" acte ")) + QLocale().toString(m_totalmontant,'f',2));
            wdg_totalrecetteslbl     ->setText(tr("Total reçu ") + QLocale().toString(m_totalrecu,'f',2)
                                        + "\n(" + tr(ESPECES) + ": " + QLocale().toString(m_totalrecuespeces,'f',2) + " - "
                                        + tr("Banque")  + ": " + QLocale().toString(m_totalrecubanque,'f',2) + ")");
        }
        if (m_mode == COMPTABLE)
        {
            m_totalapport     = 0;
            m_totalautresrecettes  = 0;
            m_totalautresrecettesespeces   = 0;
            m_totalautresrecettesbanque  = 0;
            m_grandtotalespeces      = 0;
            m_grandtotalbanqu     = 0;
            for (int k = 0; k < m_recettesmodel->rowCount(); k++)
            {
                if (!wdg_bigtable->isRowHidden(k))
                {
                    nbreActes++;
                    Recette * rec           = getRecetteFromRow(k);
                    m_totalrecu               += rec->encaissement();
                    m_totalapport             += (rec->isapportpraticien()? rec->encaissementautrerecette() : 0.0);
                    m_totalautresrecettes          += (rec->isautrerecette()? rec->encaissementautrerecette() : 0.0);
                    if(rec->modepaiement()  == ESP)
                    {
                        m_totalrecuespeces         += rec->encaissement();
                        if (rec->isautrerecette())
                            m_totalautresrecettesespeces   += rec->encaissementautrerecette();
                    }
                    else
                    {
                        m_totalrecubanque           += rec->encaissement();
                        if (rec->isautrerecette())
                            m_totalautresrecettesbanque  += rec->encaissementautrerecette();
                    }
                }
            }
            m_grandtotalbanqu = m_totalrecubanque + m_totalautresrecettesbanque;
            m_grandtotalespeces  = m_totalrecuespeces + m_totalautresrecettesespeces;
            wdg_totalmontantlbl ->setText(tr("Total ") + QString::number(nbreActes) + (nbreActes>1? tr(" lignes ") : tr(" ligne ")));
            wdg_totalrecetteslbl     ->setText(tr("Total recettes ") + QLocale().toString(m_totalrecu,'f',2)
                                        + "\n(" + tr(ESPECES) + ": " + QLocale().toString(m_totalrecuespeces,'f',2) + " - "
                                        + tr("Banque")  + ": " + QLocale().toString(m_totalrecubanque,'f',2) + ")");
            wdg_totalapportslbl      ->setVisible(m_totalapport>0.0);
            wdg_grandtotallbl       ->setVisible(m_totalautresrecettes>0.0);
            if ((m_totalapport>0.0))
                wdg_totalapportslbl->setText(tr("Total apports praticien ") + QLocale().toString(m_totalapport,'f',2));
            if ((m_totalautresrecettes>0.0))
            {
                wdg_grandtotallbl->setText(tr("Total autres recettes ") + QLocale().toString(m_totalautresrecettes,'f',2)
                                       + "\n" + tr("Total général recettes ") + QLocale().toString(m_totalautresrecettes + m_totalrecu,'f',2)
                                       + "\n(" + tr(ESPECES) + ": " + QLocale().toString(m_grandtotalespeces,'f',2) + " - "
                                       + tr("Banque")  + ": " + QLocale().toString(m_grandtotalbanqu,'f',2) + ")");
            }
        }
    }
}

void dlg_bilanrecettes::ChangeMode(enum Mode mode)
{
    QString Titre;
    m_mode = mode;
    wdg_supervcombobox      ->setVisible(m_mode == SUPERVISEUR);
    int hauteurrow      = int(QFontMetrics(qApp->font()).height()*1.3);
    wdg_bigtable->horizontalHeader()->setFixedHeight(hauteurrow*(m_mode==SUPERVISEUR? 1 : 2));
    if (m_mode == SUPERVISEUR)
    {
        FiltreTable(wdg_supervcombobox->currentData().toInt());
        if (m_debut == m_fin)
            Titre = tr("Bilan des actes pour la journée du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy"));
        else
            Titre = tr("Bilan des actes pour la période du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy")) + tr(" au ") + QLocale::system().toString(m_fin,tr("d MMMM yyyy"));
        setWindowTitle(Titre);
    }
    if (m_mode == COMPTABLE)
    {
        FiltreTable(Datas::I()->users->userconnected()->id());
        if (m_debut == m_fin)
            Titre = tr("Bilan des recettes pour la journée du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy"));
        else
            Titre = tr("Bilan des recettes pour la période du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy")) + tr(" au ") + QLocale::system().toString(m_fin,tr("d MMMM yyyy"));
        setWindowTitle(Titre);
    }
}

void dlg_bilanrecettes::ExportTable()
{
    QString ExportEtat;
    QString sep = "\t";                                                                                                             // séparateur
    if (UpMessageBox::Question(this,
                               tr("Exportation de la table des recettes"),
                               tr("Voulez-vous exporter la table des recettes?\n(Jeu de caractères Unicode (UTF8), Format CSV, langue anglais(USA), champs séparés par tabulation)"))
            != UpSmallButton::STARTBUTTON)
        return;
    ExportEtat.append(tr("Date") + sep + tr("Nom") + sep + tr("Type acte") + sep + tr("Montant") + sep + tr("Mode de paiement") + sep + tr("Reçu") + sep + tr("Divers et autres recettes") + sep + tr("Apport praticien") + "\n");
    for (int i=0;i< m_recettesmodel->rowCount(); i++)
    {
        if (!wdg_bigtable->isRowHidden(i))
        {
            Recette * rec           = getRecetteFromRow(i);
            QString date = rec->date().toString("dd/MM/yyyy");
            ExportEtat.append(date + sep);                                                                  // Date
            ExportEtat.append(rec->payeur() + sep);                                                         // NomPrenom
            ExportEtat.append(rec->cotationacte() + sep);                                                   // Cotation
            ExportEtat.append(QString::number(rec->montant()) + sep);                                       // Montant
            QString A = (rec->modepaiement() == TRS? rec->typetiers() : rec->modepaiement());               // Mode de paiement
            ExportEtat.append(Utils::ConvertitModePaiement(A) + sep);
            ExportEtat.append(QString::number(rec->encaissement()) + sep);                                  // Reçu
            double diversrec = (rec->isautrerecette()? rec->encaissementautrerecette() : 0.0);
            ExportEtat.append(QString::number(diversrec) + sep);                                            // divers et eutres recettes
            double apportprat = (rec->isapportpraticien()? rec->encaissementautrerecette() : 0.0);
            ExportEtat.append(QString::number(apportprat) + sep);                                           // Apports praticien
            ExportEtat.append("\n");
        }
    }
    QString ExportFileName = PATH_DIR_RUFUS + "/"
                            + (m_mode == COMPTABLE? tr("Recettes") + " " + Datas::I()->users->userconnected()->login() : tr("Actes") + " " + wdg_supervcombobox->currentText())
                            + " " + tr("du") + " " + QLocale::system().toString(m_debut,"d MMM yyyy") + " " + tr("au") + " " + QLocale::system().toString(m_fin,"d MMM yyyy")
                            + ".csv";
    QFile   ExportFile(ExportFileName);
    bool exportOK = true;
    QString msg = tr("Nom du fichier d'export") + "\n" + ExportFileName;
    Utils::removeWithoutPermissions(ExportFile);
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
    CalcBilan(this);
    if (!m_initok)
    {
        m_debut   = debutavant;
        m_fin     = finavant;
        m_initok  = true;
        return;
    }

    QString Titre;
    if (m_debut == m_fin)
        Titre = tr("Bilan des recettes pour la journée du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des recettes pour la période du ") + QLocale::system().toString(m_debut,tr("d MMMM yyyy")) + tr(" au ") + QLocale::system().toString(m_fin,tr("d MMMM yyyy"));
    setWindowTitle(Titre);

    RemplitLaTable();
    CalcSuperviseursEtComptables();
    wdg_superviseurradiobouton->click();
}

void dlg_bilanrecettes::RemplitLaTable()
{
    UpStandardItem *pitem0, *pitem1, *pitem2, *pitem3, *pitem4, *pitem5,*pitem6,*pitem7;
    if (m_recettesmodel != Q_NULLPTR)
        delete m_recettesmodel;
    m_recettesmodel = new QStandardItemModel(this);
    foreach (Recette *rec, *Datas::I()->recettes->recettes())
    {
        pitem0 = new UpStandardItem(QLocale::system().toString(rec->date(),tr("d MMM yyyy")), rec);                   // Date - col = 0
        pitem0->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

        pitem1 = new UpStandardItem(rec->payeur(), rec);                                            // NomPrenom - col = 1

        pitem2 = new UpStandardItem(rec->cotationacte(), rec);                                      // Cotation - col = 2;

        QString A;
        if (rec->monnaie() == "F")
            A = QLocale().toString(rec->montant()/6.55957,'f',2);                                   // Montant en F converti en euros
        else
            A = QLocale().toString(rec->montant(),'f',2);                                           // Montant - col = 3
        pitem3 = new UpStandardItem(A, rec);
        pitem3->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

        A = (rec->modepaiement() == TRS? rec->typetiers() : rec->modepaiement());                   // Mode de paiement - col = 4
        pitem4 = new UpStandardItem(Utils::ConvertitModePaiement(A), rec);

        double C = rec->encaissement();                                                             // Reçu- col = 5
        pitem5 = new UpStandardItem(QLocale().toString(C,'f',2), rec);
        pitem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

        pitem6 = new UpStandardItem("0,00", rec);                                                   // Divers et autres recettes - col = 6
        if (rec->isautrerecette())
            pitem6->setText(QLocale().toString(rec->encaissementautrerecette(),'f',2));
        pitem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

        pitem7 = new UpStandardItem("0,00", rec);                                                   // Apport praticien - col = 7
        if (rec->isapportpraticien())
            pitem7->setText(QLocale().toString(rec->encaissementautrerecette(),'f',2));
        pitem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

        m_recettesmodel->appendRow(QList<QStandardItem*>() << pitem0 << pitem1 << pitem2 << pitem3 << pitem4 << pitem5 << pitem6 << pitem7);
    }
    int hauteurrow      = int(QFontMetrics(qApp->font()).height()*1.3);
    int nbrowsAAfficher = 30;
    wdg_bigtable->setFixedHeight(nbrowsAAfficher*hauteurrow+2);

    wdg_bigtable->setModel(m_recettesmodel);

    wdg_bigtable->setPalette(QPalette(Qt::white));
    wdg_bigtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_bigtable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_bigtable->verticalHeader()->setVisible(false);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setSelectionMode(QAbstractItemView::SingleSelection);

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

    wdg_bigtable->horizontalHeader()->setVisible(true);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    wdg_bigtable->setColumnWidth(li,100);                                              // 0 - Date affichage européen
    li++;
    wdg_bigtable->setColumnWidth(li,240);                                              // 1 - Nom
    li++;
    wdg_bigtable->setColumnWidth(li,140);                                              // 2 - TypeActe
    li++;
    wdg_bigtable->setColumnWidth(li,85);                                               // 3 - Montant
    li++;
    wdg_bigtable->setColumnWidth(li,130);                                              // 4 - Mode de paiement
    li++;
    wdg_bigtable->setColumnWidth(li,85);                                               // 5 - Reçu
    li++;
    wdg_bigtable->setColumnWidth(li,120);                                              // 6 - Divers et autres recettes
    li++;
    wdg_bigtable->setColumnWidth(li,120);                                              // 7 - Apport praticien

    wdg_bigtable->setGridStyle(Qt::SolidLine);
    QFontMetrics fm(qApp->font());
    for (int j=0; j<Datas::I()->recettes->recettes()->size(); j++)
         wdg_bigtable->setRowHeight(j,int(fm.height()*1.3));

    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    //gBigTable->horizontalHeader()->setFixedHeight(int(fm.height()*2));
}

