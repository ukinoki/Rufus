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

#ifndef DLG_IMPRESSIONS_H
#define DLG_IMPRESSIONS_H

#include "procedures.h"
#include <QGraphicsOpacityEffect>
#include "gbl_datas.h"
#include "upstandarditemmodel.h"
#include "updelegate.h"

/*! La fiche dlg_impressions détermine la liste des documents à imprimer pour un patient donné et pour chaque document, UNIQUEMENT LE CORPS DU TEXTE
    *  l'appel de la fiche comporte 2 éléments : le patient concerné et l'intervention concernée si le document est en rapport avec une intervention chirurgicale
        * Chaque document à imprimer est caractérisé par une QMap<DATASAIMPRIMER, QString>
            * L'enum DATASAIMPRIMER a 5 valeurs qui caractérisent chaque document
                * d_Titre              -> le titre qui sera inséré dans la fiche docsexternes et dans la conclusion
                * d_Prescription       -> précise si le document est une prescription - le formatage n'est pas le même
                * d_Dupli              -> la nécessité ou non d'mprimer un dupli
                * d_Texte              -> le corps du document à imprimer
                * d_Administratif      -> document administratif ou pas
        * La fiche peut renvoyer plusieurs documents. L'ensemble des documents à imprimer est réuni dans une autre QMap<int, QMap<DATASAIMPRIMER, QString>>
                * int                           -> le numéro d'orde du document: 1, 2, 3, 4, 5...etc...
                * QMap<DATASAIMPRIMER, QString> -> le document à imprimer

    * l'en-tête et le pied de page varient suivant l'utilisateur et ne sont pas déterminés par la fiche dlg_impressions
    * ils sont déterminés par la fiche demandeuse de l'impression - rufus.cpp ou dlg_programmationinterventions, en appelant les fonctions correspondantes dans procedures.cpp
    * la fiche dlg_impressions ne détermine que le contenu du document
        *  mais pas l'identification de l'utilisateur qui va figurer dans l'entête du texte,
        *  ni l'identification du patient quand elle figure dans l'en tête du dcoument (comme c'est le cas pour une ordonnance)
        *  ni la date du document
*/




namespace Ui {
class dlg_impressions;
}

class dlg_impressions : public QDialog
{
    Q_OBJECT
public:
    /*!
     * \brief dlg_impressions
     * \param pat = le patient concerné
     * \param intervention si ce paramètre n'est pas null, la fiche est appelée depuis la programmation des interventions et les champs
        DATEINTERVENTION, HEUREINTERVENTION, COTEINTERVENTION, SITEINTERVENTION et TYPEINTERVENTION seront complétés automatiquement
     * \param parent
     */
    explicit                                    dlg_impressions(Patient *pat, Intervention* intervention = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    ~dlg_impressions();
    Ui::dlg_impressions                         *ui;
    enum DATASAIMPRIMER                         {d_Texte, d_Titre, d_Prescription, d_Dupli, d_Administratif};                     Q_ENUM(DATASAIMPRIMER)
    enum Mode                                   {Selection,CreationDOC,ModificationDOC,CreationDOSS,ModificationDOSS};  Q_ENUM(Mode)
    User*                                       userentete();
    QMap<int, QMap<DATASAIMPRIMER, QString> >   mapdocsaimprimer() const;

private:
    DataBase                                    *db             = DataBase::I();
    User*                                       currentuser() { return Datas::I()->users->userconnected(); }
    Procedures                                  *proc           = Procedures::I();
    Impression                                  *m_currentdocument = Q_NULLPTR;
    DossierImpression                           *m_currentdossier = Q_NULLPTR;
    Patient                                     *m_currentpatient;
    Intervention                                *m_currentintervention = Q_NULLPTR;
    User                                        *m_userentete = Q_NULLPTR;
    QList<Correspondant*>                       m_listedestinataires;
    QMap<int, QMap<DATASAIMPRIMER, QString> >   map_docsaimprimer;
    QDate                                       m_currentdate = db->ServerDate();
    Mode                        m_mode;
    double                      m_opacity;
    QMap<QString,QString>       map_champs;
    QStringList                 m_listid;
    QStringList                 m_listtexts;   /*! pour un document donné, il peut y avoir plusieurs impressions s'il y a plusieurs destinataires
                                                -> le texte varie légèrement d'un destinataire à l'autre en ce qui concerne le nom du correspondant
                                                -> la variable correspond à la liste des textes */
    QGraphicsOpacityEffect      *m_opacityeffect;
    QTimer                      *t_timerefface;

    UpDialog                    *dlg_ask;
    UpDialog                    *dlg_askdialog;
    UpDialog                    *dlg_askcorrespondant;
    WidgetButtonFrame           *wdg_docsbuttonframe, *wdg_dossiersbuttonframe;

    QHash<QString, QVariant>    m_dossierlistbinds;
    UpStandardItemModel         *m_dossiersmodel = Q_NULLPTR;
    QHash<QString, QVariant>    m_docslistbinds;
    UpStandardItemModel         *m_docsmodel = Q_NULLPTR;
    QString                     m_textdocdelegate = "";
    QString                     m_textdossierdelegate = "";
    QMenu                       *m_menucontextuel_doc = Q_NULLPTR;

    bool                        event(QEvent *event);
    void                        closeEvent      (QCloseEvent *event);
    bool                        eventFilter     (QObject *obj, QEvent *event);
    void                        keyPressEvent   (QKeyEvent * event );
    void                        AfficheDocsPublicsAutresUtilisateurs(bool affiche = true);
    void                        AfficheTexteDocument(Impression *doc);
    void                        Annulation();
    void                        ChoixButtonFrame(WidgetButtonFrame *);
    void                        ChoixMenuContextuelTexteDocument(QString);
    void                        ChoixMenuContextuelDocument(QString);
    void                        ChoixMenuContextuelDossier(QString);
    QString                     AskDialog(QString titre);
    void                        CheckPublicEditablAdmin(QCheckBox *check);
    bool                        ChercheDoublon(QString, int row);
    void                        ChoixCorrespondant(QList<Correspondant*> listcor);
    void                        CocheLesDocs(int iddoss, bool A);
    void                        ConfigMode(Mode mode);
    void                        dblClicktextEdit();
    void                        DisableLines();
    QString                     DocumentToolTip(Impression *doc);
    QString                     DossierToolTip(DossierImpression *dossier);
    void                        EffaceWidget(QWidget* widg, bool AvecOuSansPause = true);
    void                        EnableDocsButtons(Impression *doc = Q_NULLPTR);
    void                        EnableDossiersButtons(DossierImpression *dossier = Q_NULLPTR);
    void                        EnableLines();
    void                        EnableOKPushButton(QModelIndex idx = QModelIndex());
    void                        FiltreListe();
    Impression*                 getDocumentFromIndex(QModelIndex idx);
    DossierImpression*          getDossierFromIndex(QModelIndex idx);
    bool                        hasDocumentPrive(DossierImpression *dossier);              //!> vérifie si un dossier incorpore des documents privés
    bool                        EnregistreDocument(Impression *doc);
    bool                        EnregistreDossier(DossierImpression *dossier);
    void                        ListidCor();
    void                        MenuContextuelDocuments();
    void                        MenuContextuelDossiers();
    void                        MenuContextuelTexteDocument();
    void                        MetAJour(QString texte, bool pourVisu = true);
    void                        OrdoAvecDupli(bool);
    bool                        RecopieDocument(Impression *doc);
    void                        Remplir_TableView();
    void                        selectAllDocuments();
    void                        selectcurrentDocument(Impression *doc, QAbstractItemView::ScrollHint hint = QAbstractItemView::EnsureVisible);
    void                        selectcurrentDossier(DossierImpression *dossier, QAbstractItemView::ScrollHint hint = QAbstractItemView::EnsureVisible);
    void                        SetDocumentToRow(Impression *doc, int row, bool resizecolumn = true);
    void                        SetDossierToRow(DossierImpression *dossier, int row, bool resizecolumn = true);
    void                        SupprimmeDocument(Impression* doc);
    void                        SupprimmeDossier(DossierImpression *dossier);
    void                        OKpushButtonClicked();
    void                        VerifCoherencedlg_ask();
    bool                        VerifDocumentPublic(Impression *doc, bool msg = true);
    void                        VerifDossiers();

private:
    QString TITRUSER            = tr(STR_TITRUSER);
    QString NOMPAT              = tr(STR_NOMPAT);
    QString DATEDOC             = tr(STR_DATEDOC);
    QString DDNPAT              = tr(STR_DDNPAT);
    QString TITREPAT            = tr(STR_TITREPAT);
    QString AGEPAT              = tr(STR_AGEPAT);
    QString PRENOMPAT           = tr(STR_PRENOMPAT);
    QString SEXEPAT             = tr(STR_SEXEPAT);
    QString MGPAT               = tr(STR_MGPAT);
    QString MGPATTITRE          = tr(STR_MGPATTITRE);
    QString POLITESSEMG         = tr(STR_POLITESSEMG);
    QString PRENOMMG            = tr(STR_PRENOMMG);
    QString NOMMG               = tr(STR_NOMMG);
    QString REFRACT             = tr(STR_REFRACT);
    QString KERATO              = tr(STR_KERATO);
    QString POLITESSECOR        = tr(STR_POLITESSECOR);
    QString CORPAT              = tr(STR_CORPAT);
    QString PRENOMCOR           = tr(STR_PRENOMCOR);
    QString NOMCOR              = tr(STR_NOMCOR);
    QString PRENOMUSER          = tr(STR_PRENOMUSER);
    QString NOMUSER             = tr(STR_NOMUSER);
    QString TELEPHONE           = tr(STR_TELEPHONE);

    QString ANESTHINTERVENTION  = tr(STR_ANESTHINTERVENTION);
    QString PROVENANCE          = tr(STR_PROVENANCE);
    QString TYPESEJOUR          = tr(STR_TYPESEJOUR);
    QString COTE                = tr(STR_COTE);
    QString SITE                = tr(STR_SITE);
    QString DATEINTERVENTION    = tr(STR_DATEINTERVENTION);
    QString HEUREINTERVENTION   = tr(STR_HEUREINTERVENTION);
    QString COTEINTERVENTION    = tr(STR_COTEINTERVENTION);
    QString SITEINTERVENTION    = tr(STR_SITEINTERVENTION);
    QString TYPEINTERVENTION    = tr(STR_TYPEINTERVENTION);

    QString NOCOR               = tr(STR_NOCOR);

    QString TITREDATEINTERVENTION    = tr("Date d'intervention");
    QString TITREHEUREINTERVENTION   = tr("Heure d'intervention");
    QString TITRECOTEINTERVENTION    = tr("Côté de l'intervention");
    QString TITREANESTHINTERVENTION  = tr("Type d'anesthésie");
    QString TITRESITEINTERVENTION    = tr("Lieu d'intervention");
    QString TITRETYPEINTERVENTION    = tr("Type d'intervention");

};

#endif // DLG_DOCUMENTS_H
