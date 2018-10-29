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
#include "gbl_datas.h"

dlg_recettes::dlg_recettes(Procedures *ProcAPasser, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionRecettes", parent)
{
    InitOK = true;
    proc        = ProcAPasser;
    db          = DataBase::getInstance();

    CalcBilan();
    if (!InitOK)
        return;

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    gidUser     = db->getUserConnected();

    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
    TotalMontantlbl         = new UpLabel();
    TotalReclbl             = new UpLabel();
    TotalApportlbl          = new UpLabel();
    GdTotalReclbl           = new UpLabel();
    gLabel                  = new UpLabel(Q_NULLPTR ,tr("Actes effectués par "));
    hboxsup                 = new QHBoxLayout();
    QHBoxLayout *hboxinf    = new QHBoxLayout();
    QVBoxLayout *box        = new QVBoxLayout();
    gSupervBox              = new UpComboBox();

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

    glblbox = new QHBoxLayout();
    glblbox->addLayout(box);

    QString Titre;
    if (Debut == Fin)
        Titre = tr("Bilan des recettes pour la journée du ") + Debut.toString(tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des recettes pour la période du ") + Debut.toString(tr("d MMMM yyyy")) + tr(" au ") + Fin.toString(tr("d MMMM yyyy"));
    setWindowTitle(Titre);
    AjouteLayButtons(UpDialog::ButtonPrint | UpDialog::ButtonClose);
    CalcSuperviseursEtComptables();
    gMode = SUPERVISEUR;

    DefinitArchitetureTable();
    RemplitLaTable();
    FiltreTable(-1);
    globallay->insertLayout(0,glblbox);
    globallay->insertWidget(0,gBigTable);
    globallay->setSizeConstraint(QLayout::SetFixedSize);
    connect(CloseButton,        &QPushButton::clicked,                                  this, [=] {accept();});
    connect(PrintButton,        &QPushButton::clicked,                                  this, [=] {ImprimeEtat();});
    connect(ChxPeriodButt,      &QPushButton::clicked,                                  this, [=] {NouvPeriode();});
    connect(ExportButt,         &QPushButton::clicked,                                  this, [=] {ExportTable();});
    connect(gSupervBox,         QOverload<int>::of(&QComboBox::currentIndexChanged),    this, [=] {FiltreTable(gSupervBox->currentData().toInt());});
    connect(ComptableRadio,     &UpRadioButton::clicked,                                this, [=] {ChangeMode(COMPTABLE);});
    connect(SupervRadio,        &UpRadioButton::clicked,                                this, [=] {ChangeMode(SUPERVISEUR);});
}

dlg_recettes::~dlg_recettes()
{
}

bool dlg_recettes::getInitOK()
{
    return InitOK;
}

QList<QList<QVariant> > dlg_recettes::CalcBilan()
{
    QMap<QString, QDate> DateMap = proc->ChoixDate();
    if (!DateMap.isEmpty())
    {
        Debut = DateMap["DateDebut"];
        Fin   = DateMap["DateFin"];
        //---------------------------------------------- Tous les actes effectués par tout le monde durant la période, sauf les impayés et les gratuits
        QString req =
        "select res1.idActe, res1.actedate, res1.nom, res1.actecotation, res1.acteMontant, res1.actemonnaie, res1.TypePaiement,"
        " res1.Tiers, Paye, res1.iduser, res1.userparent, res1.usercomptable, null as montantautresrecettes, null as typeautresrecettes from\n "
        "(\n"
            "select\n"
            " act.idActe, actedate, concat(patnom, ' ', patprenom) as nom, actecotation, acteMontant, acteMonnaie, TypePaiement, Tiers, iduser, userparent, usercomptable from \n"
            NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ\n"
            " where act.idPat = pat.idpat\n"
            " and act.idActe = typ.idacte\n"
            " and actedate >= '" + Debut.toString("yyyy-MM-dd") + "'\n"
            " and actedate <= '" + Fin.toString("yyyy-MM-dd") + "'\n"
            " order by actedate, nom\n"
        ")\n"
        " as res1\n"
        " left outer join\n"
        " (\n"
            "select rec.idrecette, paye, lig.idActe from \n"
            NOM_TABLE_LIGNESPAIEMENTS " lig, " NOM_TABLE_RECETTES " rec, " NOM_TABLE_TYPEPAIEMENTACTES " typ2\n"
            " where lig.idrecette = rec.idrecette\n"
            " and lig.idActe = typ2.idacte\n"
            " and TypePaiement <> 'T'\n"
            " and TypePaiement <> 'G'\n"
            " and datepaiement >= '" + Debut.toString("yyyy-MM-dd") + "'\n"
            " and datepaiement <= '" + Fin.toString("yyyy-MM-dd") + "'\n"
        ")\n"
        " as res3 on res1.idacte = res3.idActe\n";

        //----------------------------------------------- et tous les tiers payants encaissés durant cette même période
        req +=
        " union\n"

        " select null as idActe, DatePaiement as actedate, NomTiers as nom, null as actecotation, null as acteMontant, Monnaie as acteMonnaie, ModePaiement as TypePaiement,"
        " null as Tiers, Montant as paye, iduser, iduser as userparent, iduser as usercomptable, null as montantautresrecettes, null as typeautresrecettes from \n"
        NOM_TABLE_RECETTES
        "\n where TiersPayant = 'O'\n"
        " and DatePaiement >= '" + Debut.toString("yyyy-MM-dd") + "'\n"
        " and DatePaiement <= '" + Fin.toString("yyyy-MM-dd") + "'\n"

        " union\n"

        " select null as idActe, DateRecette as actedate, Libelle as nom, null as actecotation, null as acteMontant, Monnaie as acteMonnaie,"
        " Paiement as TypePaiement, null as Tiers, null as paye, null as iduser, null as userparent, iduser as usercomptable,"
        " montant as montantautresrecettes, Typerecette as typeautresrecettes from \n" NOM_TABLE_RECETTESSPECIALES
        " \nwhere"
        " DateRecette >= '" + Debut.toString("yyyy-MM-dd") + "'\n"
        " and DateRecette<= '" + Fin.toString("yyyy-MM-dd") + "'\n"
        " order by actedate, nom";

        //proc->Edit(req);
        //p... ça c'est de la requête

        gBilan =  db->StandardSelectSQL(req);

        if (gBilan.size() == 0)
        {
            InitOK = false;
            UpMessageBox::Watch(this,tr("Pas de recette enregistrée pour cette période"));
            return gBilan;
        }
    }
    else
    {
        InitOK = false;
        gBilan.clear();
        return gBilan;
    }

    InitOK = true;
    return gBilan;
}
void dlg_recettes::FiltreTable(int idx)
{
    if (gMode==SUPERVISEUR)
    {
        if (idx==-1)
        {
            for(int i=0; i<gBigTable->rowCount(); i++)
                gBigTable->setRowHidden(i,gBigTable->item(i,0)->text() == "0");
        }
        else
        {
            QMultiMap<int, User*>::iterator userFind = Datas::I()->users->superviseurs()->find(idx);
            if( userFind != Datas::I()->users->superviseurs()->end() )
            {
                for(int i=0; i<gBigTable->rowCount(); i++)
                {
                    //qDebug() << gBigTable->item(i,8)->text() + " - " + QString::number(idx) + " - " + gBigTable->item(i,0)->text() + " ;";
                    gBigTable->setRowHidden(i,gBigTable->item(i,8)->text() != QString::number(idx) || gBigTable->item(i,0)->text() == "0");
                }
            }
        }
        PrintButton->setEnabled(gSupervBox->currentData().toInt()>-1  && gBigTable->rowNoHiddenCount()>0);
        gBigTable->setColumnHidden(7,true);     // divers et autres recettes
        gBigTable->setColumnHidden(12,true);    // apport pratcien
    }
    else if (gMode==COMPTABLE)
    {
            QMultiMap<int, User*>::iterator userFind = Datas::I()->users->comptables()->find(idx);
            if( userFind != Datas::I()->users->comptables()->end() )
            {
                for(int i=0; i<gBigTable->rowCount(); i++)
                {
                    //qDebug() << gBigTable->item(i,10)->text() + " - " + QString::number(idx) + " - "
                    //+ gBigTable->item(i,6)->text() + " ; " + QString::number(QLocale().toDouble(gBigTable->item(i,6)->text()));
                    gBigTable->setRowHidden(i,
                                            gBigTable->item(i,10)->text() != QString::number(idx)
                                            || (QLocale().toDouble(gBigTable->item(i,6)->text()) == 0.0
                                            && QLocale().toDouble(gBigTable->item(i,7)->text()) == 0.0
                                            && QLocale().toDouble(gBigTable->item(i,12)->text()) == 0.0));
                }
            }
        PrintButton->setEnabled(gBigTable->rowNoHiddenCount()>0);
        gBigTable->setColumnHidden(7,false);     // divers et autres recettes
        gBigTable->setColumnHidden(12,false);    // apport pratcien
    }
    ExportButt->setEnabled(gBigTable->rowNoHiddenCount()>0);
    ExportButt->setVisible(!(gMode==SUPERVISEUR && gSupervBox->currentData().toInt()==-1));
    gBigTable->FixLargeurTotale();
    CalculeTotal();
}

void dlg_recettes::ImprimeEtat()
{
    QString            Entete, Pied;
    bool AvecDupli   = false;
    bool AvecPrevisu = true;
    bool AvecNumPage = false;
    User *userEntete;

    //création de l'entête
    if (gMode==SUPERVISEUR)
        userEntete = proc->setDataOtherUser(gSupervBox->currentData().toInt());
    else
        userEntete = proc->setDataOtherUser(gidUser->id());

    if(userEntete == nullptr)
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête") , tr("Annulation de l'impression"));
        return;
    }
    Entete = proc->ImpressionEntete(QDate::currentDate(), userEntete).value("Norm");
    if (Entete == "") return;

    // NOTE : POURQUOI mettre ici "PRENOM PATIENT" alors que ceux sont les données d'un User qui sont utilisées ???
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
    Pied = proc->ImpressionPied();
    if (Pied == "") return;

    // creation du corps de la remise
    QString couleur = "<font color = \"" + proc->CouleurTitres + "\">";
    double c = CORRECTION_td_width;
    QTextEdit *Etat_textEdit = new QTextEdit;
    QString test4 = "<html><head><style type=\"text/css\">p.p1 {font:70px; margin: 0px 0px 10px 100px;}"
                    "</style></head>"
                    "<body LANG=\"fr-FR\" DIR=\"LTR\">"
                    "<table width=\"" + QString::number(int(c*510)) + "\" border=\"1\"  cellspacing=\"0\" cellpadding=\"2\">";
    int row = 1;
    for (int i = 0; i < gBigTable->rowCount();i++)
    {
        if (!gBigTable->isRowHidden(i))
        {
            if (gMode == SUPERVISEUR)
            {
                if (gBigTable->item(i,3)->text()!= "")
                {
                    test4 += "<tr>"
                             "<td width=\"" + QString::number(int(c*30))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</div></span></td>"              // no ligne
                             "<td width=\"" + QString::number(int(c*180)) + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,2)->text() + "</span></td>"                                 // nom prenom
                             "<td width=\"" + QString::number(int(c*160)) + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,3)->text() + "</span></td>"                                 // cotation
                             "<td width=\"" + QString::number(int(c*95))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + gBigTable->item(i,4)->text() + "</div></span></td>"      // montant
                             "</tr>";
                    row++;
                }
            }
            else if (gMode == COMPTABLE)
            {
                test4 += "<tr>";
                if (QLocale().toDouble(gBigTable->item(i,12)->text())!=0.0)                      // ------------------------------------------------------------------------------------   c'est un apport praticien
                {
                    test4 +=    "<td width=\"" + QString::number(int(c*25))  + "\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</div></span></font></td>"             // no ligne
                                "<td width=\"" + QString::number(int(c*60))  + "\">" + couleur + "<span style=\"font-size:8pt\">" + gBigTable->item(i,1)->text() + "</span></font></td>"                                // date
                                "<td width=\"" + QString::number(int(c*180)) + "\">" + couleur + "<span style=\"font-size:8pt\">" + gBigTable->item(i,2)->text() + "</span></font></td>"                                // libelle apport praticien
                                "<td width=\"" + QString::number(int(c*115)) + "\">" + couleur + "<span style=\"font-size:8pt\">" + tr("apport praticien") + "</span></font></td>"                                      // apport praticien
                                "<td width=\"" + QString::number(int(c*50))  + "\">" + couleur + "<span style=\"font-size:8pt\"><div align=\"right\">" + gBigTable->item(i,12)->text() + "</div></span></font></td>"    // montant
                                "<td width=\"" + QString::number(int(c*75))  + "\">" + couleur + "<span style=\"font-size:8pt\">" + gBigTable->item(i,5)->text() + "</span></font></td>";                               // mode de paiement
                }
                else if (QLocale().toDouble(gBigTable->item(i,4)->text())==0.0 && QLocale().toDouble(gBigTable->item(i,7)->text())==0.0) // --------------------------------------------   c'est un tiers payant
                {
                    test4 += "<td width=\"" + QString::number(int(c*25))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</span></td>"                    // no ligne
                             "<td width=\"" + QString::number(int(c*60))  + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,1)->text() + "</span></td>"                                 // date
                             "<td width=\"" + QString::number(int(c*180)) + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,2)->text() + "</span></td>"                                 // nom prenom ou libelle si recette speciale
                             "<td width=\"" + QString::number(int(c*115)) + "\"><span style=\"font-size:8pt\">-</span></td>";                                                                   // vide (on est obligé de mettre un "-" parce que sinon la hauteur de ligne est fausse)
                    test4 += "<td width=\"" + QString::number(int(c*50))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + gBigTable->item(i,6)->text() + "</div></span></td>";     // montant
                    test4 += "<td width=\"" + QString::number(int(c*75))  + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,5)->text() + "</span></td>";                                // mode de paiement

               }
                else                                                                            // -------------------------------------------------------------------------------------   c'est une recette
                {
                    test4 += "<td width=\"" + QString::number(int(c*25))  + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + QString::number(row) + "</span></td>"                    // no ligne
                             "<td width=\"" + QString::number(int(c*60))  + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,1)->text() + "</span></td>"                                 // date
                             "<td width=\"" + QString::number(int(c*180)) + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,2)->text() + "</span></td>";                                // nom prenom ou libelle si recette speciale
                    if (QLocale().toDouble(gBigTable->item(i,7)->text())!=0.0)                                                                                                                  // cotation ou type recette si recette spéciale
                        test4 += "<td width=\"" + QString::number(int(c*115)) + "\"><span style=\"font-size:8pt\">" + tr("divers et autres recettes") + "</span></td>";
                    else
                        test4 += "<td width=\"" + QString::number(int(c*115)) + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,3)->text() + "</span></td>";
                    if (QLocale().toDouble(gBigTable->item(i,7)->text())!=0.0)                                                                                                                  // montant
                        test4 += "<td width=\"" + QString::number(int(c*50)) + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + gBigTable->item(i,7)->text() + "</div></span></td>";
                    else
                        test4 += "<td width=\"" + QString::number(int(c*50)) + "\"><span style=\"font-size:8pt\"><div align=\"right\">" + gBigTable->item(i,6)->text() + "</div></span></td>";
                    test4 += "<td width=\"" + QString::number(int(c*75))  + "\"><span style=\"font-size:8pt\">" + gBigTable->item(i,5)->text() + "</span></td>";                                // mode de paiement
               }
                row++;
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
}

void dlg_recettes::CalcSuperviseursEtComptables()
{
    hboxsup->removeWidget(gLabel);
    glblbox->removeWidget(ClassmtupGrpBox);
    gSupervBox->clear();

    QList <int> listiD; // la liste des superviseurs - gBilan.value(9)
    bool idcomptabletrouve = false;

    for (int i=0; i< gBilan.size(); i++)
    {
        int iduser = gBilan.at(i).at(9).toInt();
        if (!listiD.contains(iduser) && iduser > 0)
            listiD << iduser;
        if (!idcomptabletrouve)
            idcomptabletrouve = (gBilan.at(i).at(11).toInt() == gidUser->id());
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

void dlg_recettes::CalculeTotal()
{
    TotalMontant = 0;
    TotalRecu    = 0;
    TotalRecEsp     = 0;
    TotalRecBanq    = 0;
    int    nbreActes    = 0;
    TotalApportlbl      ->setVisible(gMode==COMPTABLE);
    GdTotalReclbl       ->setVisible(gMode==COMPTABLE);
    if (gBigTable->rowCount() > 0)
    {
        if (gMode == SUPERVISEUR)
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
                        TotalRecEsp        += QLocale().toDouble(gBigTable->item(k,6)->text());
                    else    TotalRecBanq       += QLocale().toDouble(gBigTable->item(k,6)->text());
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
            for (int k = 0; k < gBigTable->rowCount(); k++)
            {
                if (!gBigTable->isRowHidden(k))
                {
                    nbreActes++;
                    TotalRecu               += QLocale().toDouble(gBigTable->item(k,6)->text());
                    TotalApport             += QLocale().toDouble(gBigTable->item(k,12)->text());
                    TotalAutresRec          += QLocale().toDouble(gBigTable->item(k,7)->text());
                    if(gBigTable->item(k,5)->text()  == tr("Espèces"))
                    {
                        TotalRecEsp            += QLocale().toDouble(gBigTable->item(k,6)->text());
                        TotalAutresRecEsp   += QLocale().toDouble(gBigTable->item(k,7)->text());
                    }
                    else
                    {
                        TotalRecBanq           += QLocale().toDouble(gBigTable->item(k,6)->text());
                        TotalAutresRecBanq  += QLocale().toDouble(gBigTable->item(k,7)->text());
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

void dlg_recettes::ChangeMode(enum gMode Mode)
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

void dlg_recettes::DefinitArchitetureTable()
{
    gBigTable = new UpTableWidget();

    int ColCount = 13;
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
    LabelARemplir << tr("Divers et\nautres recettes");
    LabelARemplir << "";
    LabelARemplir << "";
    LabelARemplir << "";
    LabelARemplir << "";
    LabelARemplir << tr("Apport praticien");

    gBigTable->setHorizontalHeaderLabels(LabelARemplir);
    gBigTable->horizontalHeader()->setVisible(true);
    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    gBigTable->setColumnWidth(li,0);                                                // 0 - idActe
    li++;
    gBigTable->setColumnWidth(li,100);                                              // 1 - Date affichage européen
    li++;
    gBigTable->setColumnWidth(li,240);                                              // 2 - Nom
    li++;
    gBigTable->setColumnWidth(li,140);                                              // 3 - TypeActe
    li++;
    gBigTable->setColumnWidth(li,85);                                               // 4 - Montant
    li++;
    gBigTable->setColumnWidth(li,130);                                              // 5 - Mode de paiement
    li++;
    gBigTable->setColumnWidth(li,85);                                               // 6 - Reçu
    li++;
    gBigTable->setColumnWidth(li,120);                                              // 7 - Divers et autres recettes
    li++;
    gBigTable->setColumnWidth(li,0);                                                // 8 - idsuperviseur
    li++;
    gBigTable->setColumnWidth(li,0);                                                // 9 - idparent
    li++;
    gBigTable->setColumnWidth(li,0);                                                // 10 - idcomptable
    li++;
    gBigTable->setColumnWidth(li,0);                                                // 11 - date format MySQL
    li++;
    gBigTable->setColumnWidth(li,120);                                              // 12 - Apport praticien

    gBigTable->setColumnHidden(0,true);
    gBigTable->setColumnHidden(ColCount-5,true);
    gBigTable->setColumnHidden(ColCount-4,true);
    gBigTable->setColumnHidden(ColCount-3,true);
    gBigTable->setColumnHidden(ColCount-2,true);

    gBigTable->setGridStyle(Qt::SolidLine);
    gBigTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void dlg_recettes::ExportTable()
{
    QByteArray ExportEtat;
    QString sep = "\t";                                                                                                             // séparateur
    if (UpMessageBox::Question(this,
                               tr("Exportation de la table des recettes"),
                               tr("Voulez-vous exporter la table des recettes?\n(Format CSV, langue anglais(USA), champs séparés par tabulation)"))
            != UpSmallButton::STARTBUTTON)
        return;
    ExportEtat.append(tr("Date") + sep + tr("Nom") + sep + tr("Type acte") + sep + tr("Montant") + sep + tr("Mode de paiement") + sep + tr("Reçu") + sep + tr("Divers et autres recettes") + sep + tr("Apport praticien") + "\n");
    for (int i=0;i< gBigTable->rowCount(); i++)
    {
        if (!gBigTable->isRowHidden(i))
        {
            QString date = QDate::fromString(gBigTable->item(i,11)->text(), "yyyy-MM-dd").toString("dd/MM/yyyy");
            ExportEtat.append(date + sep);                                                                                              // Date - col = 11
            ExportEtat.append(gBigTable->item(i,2)->text() + sep);                                                                      // NomPrenom - col = 2
            ExportEtat.append(gBigTable->item(i,3)->text() + sep);                                                                      // Cotation - col = 3;
            double montant = QLocale().toDouble(gBigTable->item(i,4)->text());
            ExportEtat.append(QString::number(montant) + sep);                                                                          // Montant - col = 4
            ExportEtat.append(gBigTable->item(i,5)->text() + sep);                                                                      // Mode de paiement - col = 5
            double recu = QLocale().toDouble(gBigTable->item(i,6)->text());
            ExportEtat.append(QString::number(recu) + sep);                                                                             // Reçu- col = 6
            double diversrec = QLocale().toDouble(gBigTable->item(i,7)->text());
            ExportEtat.append(QString::number(diversrec) + sep);                                                                        // divers et eutres recettes - col = 7
            double apportprat = QLocale().toDouble(gBigTable->item(i,12)->text());
            ExportEtat.append(QString::number(apportprat) + sep);                                                                       // Apports praticien- col = 12
            ExportEtat.append("\n");
        }
    }
    QString ExportFileName = QDir::homePath() + NOMDIR_RUFUS + "/"
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

void dlg_recettes::NouvPeriode()
{
    QDate debutavant, finavant;
    QList<QList<QVariant>> bilanavant;
    CalcBilan();
    if (!InitOK)
    {
        Debut   = debutavant;
        Fin     = finavant;
        gBilan  = bilanavant;
        InitOK  = true;
        return;
    }

    QString Titre;
    if (Debut == Fin)
        Titre = tr("Bilan des recettes pour la journée du ") + Debut.toString(tr("d MMMM yyyy"));
    else
        Titre = tr("Bilan des recettes pour la période du ") + Debut.toString(tr("d MMMM yyyy")) + tr(" au ") + Fin.toString(tr("d MMMM yyyy"));
    setWindowTitle(Titre);

    CalcSuperviseursEtComptables();

    RemplitLaTable();
    SupervRadio->click();
}

void dlg_recettes::RemplitLaTable()
{
    QTableWidgetItem    *pItem0,*pItem1,*pItem2,*pItem3,*pItem4,*pItem5,*pItem6,*pItem7, *pItem8, *pItem9, *pItem10, *pItem11, *pItem12;
    QString             A;
    int hauteurrow      = int(QFontMetrics(qApp->font()).height()*1.3);
    gBigTable->clearContents();

    gBigTable->setRowCount(gBilan.size());

    gBilan.first();
    for (int i = 0; i < gBilan.size(); i++)
    {
        int col = 0;
        QList<QVariant> rec = gBilan.at(i);

        A = rec.at(0).toString();                                                             // idActe - col = 0
        pItem0 = new QTableWidgetItem();
        pItem0->setText(A);
        gBigTable->setItem(i,col,pItem0);
        col++;

        A = rec.at(1).toDate().toString(tr("d MMM yyyy"));                                     // Date - col = 1
        pItem1 = new QTableWidgetItem();
        pItem1->setText(A);
        pItem1->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem1);
        col++;

        pItem2 = new QTableWidgetItem();
        pItem2->setText(rec.at(2).toString());                                                // NomPrenom - col = 2
        gBigTable->setItem(i,col,pItem2);
        col++;

        pItem3 = new QTableWidgetItem();
        pItem3->setText(rec.at(3).toString());                                                // Cotation - col = 3;
        gBigTable->setItem(i,col,pItem3);
        col++;

        if (rec.at(5).toString() == "F")
            A = QLocale().toString(rec.at(4).toDouble()/6.55957,'f',2);// Montant en F converti en euros
        else
            A = QLocale().toString(rec.at(4).toDouble(),'f',2);                               // Montant - col = 4
        pItem4 = new QTableWidgetItem();
        pItem4->setText(A);
        pItem4->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem4);
        col++;

        A = rec.at(6).toString();                                                             // Mode de paiement - col = 5
        QString B = A;
        if (A == "T")
            B = rec.at(7).toString();
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


        double C = rec.at(8).toDouble();                                                      // Reçu- col = 6
        pItem6 = new QTableWidgetItem();
        pItem6->setText(QLocale().toString(C,'f',2));
        pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem6);
        col++;

        pItem7 = new QTableWidgetItem() ;                                                           //DIvers et autres recettes - col = 7
        pItem7->setText(("0,00"));
        if (rec.at(13).toString() == tr("Divers et autres recettes"))
            pItem7->setText(QLocale().toString(rec.at(12).toDouble(),'f',2));
        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem7);
        col++;

        pItem8 = new QTableWidgetItem();
        pItem8->setText(rec.at(9).toString());                                                // iduser - col = 8 - superviseur;
        gBigTable->setItem(i,col,pItem8);
        col++;

        pItem9 = new QTableWidgetItem();
        pItem9->setText(rec.at(10).toString());                                               // idparent - col = 9;
        gBigTable->setItem(i,col,pItem9);
        col++;

        pItem10 = new QTableWidgetItem();
        pItem10->setText(rec.at(11).toString());                                               // idcomptable - col = 10;
        gBigTable->setItem(i,col,pItem10);
        col++;

        pItem11 = new QTableWidgetItem();
        pItem11->setText(rec.at(1).toString());                                               // date format MySQL (yyyy-MM-dd) - col = 11;
        gBigTable->setItem(i,col,pItem11);

        gBigTable->setRowHeight(i,hauteurrow);

        col++;

        pItem12 = new QTableWidgetItem() ;                                                           // Apports praticien - col = 12
        pItem12->setText(("0,00"));
        if (rec.at(13).toString() == tr("Apport praticien"))
            pItem12->setText(QLocale().toString(rec.at(12).toDouble(),'f',2));
        pItem12->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        gBigTable->setItem(i,col,pItem12);
    }
    int nbrowsAAfficher = 30;
    gBigTable->setFixedHeight(nbrowsAAfficher*hauteurrow+2);
}
