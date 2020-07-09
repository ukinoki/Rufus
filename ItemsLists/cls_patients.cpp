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

#include "cls_patients.h"

/*!
 * \brief Patients::Patients
 * Initialise la map Patient
 */
Patients::Patients(QObject *parent) : ItemsList(parent)
{
    map_patients              = new QMap<int, Patient*>();
    map_patientstable         = new QMap<int, Patient*>();
    map_patientssaldat        = new QMap<int, Patient*>();
    m_currentpatient        = new Patient();
    m_dossierpatientaouvrir = new Patient();
    m_full                  = false;
}

bool Patients::isfull()
{
    return m_full;
}

void Patients::setcurrentpatient(Patient *pat)
{
    if (m_currentpatient != Q_NULLPTR)
        delete m_currentpatient;
    if (pat == Q_NULLPTR)
        m_currentpatient = Q_NULLPTR;
    else
    {
        m_currentpatient = new Patient();
        DataBase::I()->loadPatientById(pat->id(), m_currentpatient, Item::LoadDetails);
    }
}

void Patients::setdossierpatientaouvrir(Patient *pat)
{
    if (m_dossierpatientaouvrir != Q_NULLPTR)
        delete m_dossierpatientaouvrir;
    if (pat == Q_NULLPTR)
        m_dossierpatientaouvrir = Q_NULLPTR;
    else
    {
        m_dossierpatientaouvrir = new Patient();
        DataBase::I()->loadPatientById(pat->id(), m_dossierpatientaouvrir, Item::LoadDetails);
    }
}

DonneesOphtaPatient* Patients::donneesophtacurrentpatient()
{
    setDonneesOphtaCurrentPatient();
    return DataBase::I()->donneesOphtaPatient();
}
void Patients::setDonneesOphtaCurrentPatient()
{
    if(DataBase::I()->donneesOphtaPatient()->id() != m_currentpatient->id())
        DataBase::I()->initDonnesOphtaPatient(m_currentpatient->id());
}

void Patients::actualiseDonneesOphtaCurrentPatient()
{
    DataBase::I()->initDonnesOphtaPatient(m_currentpatient->id());
}


/*! charge les données du patient corresondant à l'id * \brief Patients::getById
 * \param id l'id du patient recherché
 * \param all =false  -> ne charge que les données d'identité - =true -> charge les données sociales et médicales
 * \return Q_NULLPTR si aucun patient trouvé
 * \return Patient* le patient correspondant à l'id
 */
Patient* Patients::getById(int id, Item::LOADDETAILS loadDetails)
{
    if (id == 0)
        return Q_NULLPTR;
    Patient *pat = Q_NULLPTR;
    QMap<int, Patient*>::const_iterator itpat = map_patients->constFind(id);
    if (itpat == map_patients->constEnd())
    {
        pat = DataBase::I()->loadPatientById(id, pat, loadDetails);
        if (pat != Q_NULLPTR)
            map_patients->insert(pat->id(), pat);
    }
    else
    {
        pat = itpat.value();
        if (loadDetails == Item::LoadDetails)
        {
            QJsonObject jsonPatient = DataBase::I()->loadPatientAllData(id);
            if( jsonPatient.isEmpty() )
                return Q_NULLPTR;
            pat->resetdatas();
            pat->setData(jsonPatient);
        }
    }
    return pat;
}

void Patients::loadAll(Patient *pat, Item::UPDATE upd)
{
    if (pat == Q_NULLPTR)
        return;
    if (!pat->isalloaded() || upd == Item::Update)
    {
        QJsonObject jsonPatient = DataBase::I()->loadPatientAllData(pat->id());
        if( !jsonPatient.isEmpty() )
        {
            pat->resetdatas();
            pat->setData(jsonPatient);
        }
    }
}

void Patients::initListeSalDat(QList<int> listidaajouter)
{
    /*! on recrée la liste des patients en cours
     */
    QList<Patient*> listpatients = DataBase::I()->loadPatientsByListId(listidaajouter);
    //clearAll(map_patientssaldat);
    epurelist(map_patientssaldat, &listpatients);
    addList(map_patientssaldat, &listpatients, Item::Update);
}

void Patients::initListeTable(QString nom, QString prenom, bool filtre)
{
    /*! on recrée une liste des patients pour remplir la table
     */
    QList<Patient*> listpatients = DataBase::I()->loadPatientsAll(nom, prenom, filtre);
    m_full = (nom == "" && prenom == "");
    //clearAll(map_patientstable);
    epurelist(map_patientstable, &listpatients);
    addList(map_patientstable, &listpatients, Item::Update);
}

void Patients::initListeByDDN(QDate DDN)
{
    QList<Patient*> listpatients = (DDN == QDate()? DataBase::I()->loadPatientsAll() : DataBase::I()->loadPatientsByDDN(DDN));
    m_full = (DDN == QDate());
    //clearAll(map_patientstable);
    epurelist(map_patientstable, &listpatients);
    addList(map_patientstable, &listpatients, Item::Update);
}

void Patients::initListeIdInterventions(Patient *pat)
{
    if (!pat)
        return;
    pat->setListidinterventions(DataBase::I()->loadIdInterventionsByPatientId(pat->id()));
}

void Patients::SupprimePatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    //!. Suppression des bilans orthoptiques
    DataBase::I()->StandardSQL("DELETE FROM " TBL_BILANORTHO " WHERE idbilanortho in (SELECT idActe from " TBL_ACTES " where idPat = " + QString::number(pat->id()) + ")");
    //!. Suppression des actes
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_ACTES);
    //!. Suppression des documents émis
    DataBase::I()->SupprRecordFromTable(pat->id(), CP_IDPAT_DOCSEXTERNES, TBL_DOCSEXTERNES);
    //!. Suppression des mots cles utilisés
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_MOTSCLESJOINTURES);

    //! Suppression dans la base OPhtalmologie
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_REFRACTIONS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_DONNEES_OPHTA_PATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_BIOMETRIES);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_TONOMETRIE);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_PACHYMETRIE);

    //!. Suppression du patient
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_PATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_DONNEESSOCIALESPATIENTS);
    DataBase::I()->SupprRecordFromTable(pat->id(), "idPat", TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    auto it = map_patientssaldat->find(pat->id());
    if (it != map_patientssaldat->end())
    {
        if (it.value() != pat)
            delete it.value();
        map_patientssaldat->remove(pat->id());
    }
    it = map_patientstable->find(pat->id());
    if (it != map_patientstable->end())
    {
        if (it.value() != pat)
            delete it.value();
        map_patientstable->remove(pat->id());
    }
    it = map_patients->find(pat->id());
    if (it != map_patients->end())
    {
        if (it.value() != pat)
            delete it.value();
        map_patients->remove(pat->id());
    }
    int id = pat->id();
    if (pat != m_currentpatient && pat != m_dossierpatientaouvrir)
        delete pat;
    if (m_currentpatient->id() == id)
       m_currentpatient->resetdatas();
    if (m_dossierpatientaouvrir->id() == id)
        m_dossierpatientaouvrir->resetdatas();
}

void Patients::updatePatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    pat->setData(DataBase::I()->loadPatientAllData(pat->id()));
}

void Patients::updateCorrespondant(Patient *pat, Correspondant::typecorrespondant type, Correspondant *cor)
{
    if (pat == Q_NULLPTR)
        return;
    int id = (cor != Q_NULLPTR ? cor->id() : 0);
    QString field;
    switch (type) {
    case Correspondant::MG:
        field = CP_IDMG_RMP;
        pat->setmg(id);
        break;
    case Correspondant::Spe1:
        field = CP_IDSPE1_RMP;
        pat->setspe1(id);
        break;
    case Correspondant::Spe2:
        pat->setspe2(id);
        field = CP_IDSPE2_RMP;
    }
    QString idsql = (cor != Q_NULLPTR ? QString::number(cor->id()) : "null");
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + field + " = " + idsql +
                " where idpat = " + QString::number(pat->id()));
}

Patient* Patients::CreationPatient(QHash<QString, QVariant> sets)
{
    Patient *pat = Q_NULLPTR;
    DataBase::I()->locktables(QStringList() << TBL_PATIENTS << TBL_DONNEESSOCIALESPATIENTS << TBL_RENSEIGNEMENTSMEDICAUXPATIENTS );
    sets[CP_DATECREATION_PATIENTS] = DataBase::I()->ServerDateTime().date();
    sets[CP_IDCREATEUR_PATIENTS]   = DataBase::I()->idUserConnected();
    bool result = DataBase::I()->InsertSQLByBinds(TBL_PATIENTS, sets);
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce patient dans la base!"));
        DataBase::I()->unlocktables();
        return pat;
    }
    // Récupération de l'idpatient créé ------------------------------------
    int id = -1;
    QHash<QString, QVariant>::const_iterator itx = sets.constFind(CP_IDPAT_PATIENTS);
    if (itx != sets.constEnd())
        id = itx.value().toInt();
    else
    {
        bool ok;
        id = DataBase::I()->selectMaxFromTable(CP_IDPAT_PATIENTS, TBL_PATIENTS, ok, tr("Impossible de sélectionner les enregistrements"));
        if (!ok)
        {
            DataBase::I()->unlocktables();
            return Q_NULLPTR;
        }
    }
    QJsonObject  data = QJsonObject{};
    data[CP_IDPAT_PATIENTS] = id;
    data[CP_IDCREATEUR_PATIENTS] = DataBase::I()->idUserConnected();
    data[CP_DATECREATION_PATIENTS] = QDate::currentDate().toString("yyyy-MM-dd");
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_NOM_PATIENTS)                   data[champ] = itset.value().toString();
        else if (champ == CP_PRENOM_PATIENTS)           data[champ] = itset.value().toString();
        else if (champ == CP_DDN_PATIENTS)              data[champ] = itset.value().toDate().toString("yyyy-MM-dd");
        else if (champ == CP_SEXE_PATIENTS)             data[champ] = itset.value().toString();
        else if (champ == CP_DATECREATION_PATIENTS)     data[champ] = itset.value().toDate().toString("yyyy-MM-dd");
        else if (champ == CP_IDCREATEUR_PATIENTS)       data[champ] = itset.value().toInt();
        else if (champ == CP_IDLIEU_PATIENTS)           data[champ] = itset.value().toInt();
        else if (champ == CP_COMMENTAIRE_PATIENTS)      data[champ] = itset.value().toString();
    }
    pat = new Patient(data);
    QString req = "INSERT INTO " TBL_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    DataBase::I()->StandardSQL(req,tr("Impossible de créer les données sociales"));
    req = "INSERT INTO " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    DataBase::I()->StandardSQL(req,tr("Impossible de créer les renseignements médicaux"));
    DataBase::I()->unlocktables();
    return pat;
}

bool Patients::veriftelephone(Patient *pat)
{
    bool retour = false;
    UpDialog            *dlg_telephone = new UpDialog();
    dlg_telephone->setAttribute(Qt::WA_DeleteOnClose);
    dlg_telephone->setWindowTitle(tr("No de téléphone"));

    UpLabel* lbl    = new UpLabel;
    lbl             ->setText(pat->nom() + " " + pat->prenom() + "\n" + tr("n'a pas de n°de téléphone enregistré") + "\n" + tr("Entrez au moins un n°"));
    lbl             ->setAlignment(Qt::AlignCenter);

    QHBoxLayout *telLay    = new QHBoxLayout();
    UpLabel* lbltel = new UpLabel;
    lbltel          ->setText(tr("Telephone"));
    QLineEdit *linetel = new QLineEdit();
    linetel         ->setFixedSize(QSize(120,24));
    linetel         ->setValidator(new QRegExpValidator(Utils::rgx_telephone));
    telLay          ->addWidget(lbltel);
    telLay          ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    telLay          ->addWidget(linetel);
    telLay          ->setSpacing(5);
    telLay          ->setContentsMargins(0,0,0,0);

    QHBoxLayout *portLay    = new QHBoxLayout();
    UpLabel* lblport = new UpLabel;
    lblport         ->setText(tr("Portable"));
    QLineEdit *lineport = new QLineEdit();
    lineport        ->setFixedSize(QSize(120,24));
    lineport        ->setValidator(new QRegExpValidator(Utils::rgx_telephone));
    portLay         ->addWidget(lblport);
    portLay         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    portLay         ->addWidget(lineport);
    portLay         ->setSpacing(5);
    portLay         ->setContentsMargins(0,0,0,0);

    dlg_telephone->dlglayout()  ->insertLayout(0, telLay);
    dlg_telephone->dlglayout()  ->insertLayout(0, portLay);
    dlg_telephone->dlglayout()  ->insertWidget(0, lbl);
    dlg_telephone->dlglayout()  ->setSpacing(5);
    dlg_telephone->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_telephone->OKButton, &QPushButton::clicked, dlg_telephone, [&]
    {
        QString portable    = lineport->text();
        QString fixe        = linetel->text();
        if  (portable == "" && fixe == "")
            return;
        if  (fixe != "")
            ItemsList::update(pat, CP_TELEPHONE_DSP, fixe);
        if  (portable != "")
            ItemsList::update(pat, CP_PORTABLE_DSP, portable);
        dlg_telephone->close();
        retour = true;
    });
    lineport->setFocus();
    dlg_telephone->exec();
    return retour;
};
