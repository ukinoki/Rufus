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

#include "cls_docsexternes.h"

DocsExternes::DocsExternes(QObject *parent) : ItemsList(parent)
{
    m_docsexternes = new QMap<int, DocExterne*>();
}

QMap<int, DocExterne *> *DocsExternes::docsexternes()
{
    return m_docsexternes;
}

/*!
 * \brief docsexternes::getById
 * \param id l'idimpr du Document recherché
 * \return Q_NULLPTR si aucun Document trouvée
 * \return DocExterne* le Document correspondant à l'id
 */
DocExterne* DocsExternes::getById(int id, Item::LOADDETAILS loadDetails, ADDTOLIST addToList)
{
    QMap<int, DocExterne*>::const_iterator itdoc = m_docsexternes->find(id);
    DocExterne *result;
    if( itdoc == m_docsexternes->constEnd() )
            result = new DocExterne();
    else
    {
        result = itdoc.value();
        if (loadDetails == Item::NoLoadDetails)
            return result;
        addToList = ItemsList::NoAddToList;
    }

    if( !result->isAllLoaded() )
    {
        QJsonObject jsonDocExterne = DataBase::I()->loadDocExterneData(id);
        if( jsonDocExterne.isEmpty() )
        {
            delete result;
            return Q_NULLPTR;
        }
        else
            result->setData(jsonDocExterne);
    }
    if( addToList == ItemsList::AddToList)
        add( m_docsexternes, result->id(), result );
    return result;
}

bool DocsExternes::NouveauDocument()
{
    return m_nouveaudocument;
}

void DocsExternes::setNouveauDocumentFalse()
{
    m_nouveaudocument = false;
}

void DocsExternes::addList(QList<DocExterne*> listdocs)
{
    for(QList<DocExterne*>::const_iterator it = listdocs.constBegin(); it != listdocs.constEnd(); ++it )
    {
        DocExterne *doc = const_cast<DocExterne*>(*it);
        if(!m_docsexternes->contains(doc->id()))
            m_nouveaudocument = true;
        add(m_docsexternes, doc->id(), doc);
    }
}

/*!
 * \brief DocsExternes::initListeByPatient
 * Charge l'ensemble des documents externes pour un patient
 * et les ajoute à la classe Patients
 */
void DocsExternes::initListeByPatient(Patient *pat)
{
    clearAll(m_docsexternes);
    addList(DataBase::I()->loadDoscExternesByPatient(pat));
}


void DocsExternes::SupprimeDocument(DocExterne *doc)
{
    DataBase::I()->StandardSQL("delete from " TBL_REFRACTION " where idrefraction = (select idrefraction from " TBL_IMPRESSIONS
                    " where idimpression = " + QString::number(doc->id()) + ")");
    DataBase::I()->StandardSQL("delete from " TBL_ECHANGEIMAGES " where idimpression = " + QString::number(doc->id()));
    DataBase::I()->SupprRecordFromTable(doc->id(), CP_IDIMPRESSION_IMPRESSIONS, TBL_IMPRESSIONS);
    remove(m_docsexternes, doc);
}

DocExterne* DocsExternes::CreationDocument(int idUser, int idPat, QString TypeDoc, QString SousTypeDoc, QString Titre,
                                           QString TextEntete, QString TextCorps, QString TextOrigine, QString  TextPied, QDateTime DateImpression,
                                           QFile file, QString lienversfichier, int idRefraction, bool ALD, int UserEmetteur,
                                           int EmisRecu, QString FormatDoc, int idLieu, int Importance)
{
    DocExterne *doc = Q_NULLPTR;
    QString idusr           = QString::number(idUser);
    QString idpat           = QString::number(idPat);
    QVariant typdoc         = (TypeDoc == ""?               QVariant(QVariant::String) : (TypeDoc));
    QVariant sstypdoc       = (SousTypeDoc == ""?           QVariant(QVariant::String) : (SousTypeDoc));
    QVariant titre          = (Titre == ""?                 QVariant(QVariant::String) : (Titre));
    QVariant entete         = (TextEntete == ""?            QVariant(QVariant::String) : (TextEntete));
    QVariant corps          = (TextCorps == ""?             QVariant(QVariant::String) : (TextCorps));
    QVariant txtorigin      = (TextOrigine == ""?           QVariant(QVariant::String) : (TextOrigine));
    QVariant pied           = (TextPied == ""?              QVariant(QVariant::String) : (TextPied));
    QString date            = (DateImpression.isValid()?    QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") : DateImpression.toString("yyyy-MM-dd HH:mm:ss"));
    QVariant lienfichier    = (lienversfichier == ""?       QVariant(QVariant::String) : (lienversfichier));
    QVariant idref          = (idRefraction == 0?           QVariant(QVariant::String) : QString::number(idRefraction));
    QVariant ald            = (!ALD?                        QVariant(QVariant::String) : "1");
    QString idemetteur      = QString::number(UserEmetteur);
    QString emisrecu        = QString::number(EmisRecu);
    QVariant formatdoc      = (FormatDoc == ""?             QVariant(QVariant::String) : (FormatDoc));
    QString idlieu          = QString::number(idLieu);
    QString importance      = QString::number(Importance);

    bool ok;
    DataBase::I()->locktables(QStringList() << TBL_IMPRESSIONS);
    int idimpr = DataBase::I()->selectMaxFromTable(CP_IDIMPRESSION_IMPRESSIONS,  TBL_IMPRESSIONS, ok)+1;
    QString suffixe = "";
    QVariant ba = QVariant(QVariant::ByteArray);
    if (file.exists())
    {
        suffixe = QFileInfo(file).suffix().toLower();
        if (suffixe == "jpeg")
            suffixe = JPG;
        QString commentechec ("");
        if (suffixe != PDF && suffixe != JPG)
        {
            commentechec = tr("format invalide") + " -> " + suffixe;
            DataBase::I()->unlocktables();
            return doc;
        }
        ba = file.readAll();
    }
    QHash<QString,QVariant> listbinds;
    listbinds["idimpression"] =     idimpr;
    listbinds["iduser"] =           DataBase::I()->getUserConnected()->id();
    listbinds["idpat"] =            idpat;
    listbinds["typeDoc"] =          typdoc;
    listbinds["soustypedoc"] =      SousTypeDoc;
    listbinds["titre"] =            titre;
    listbinds["TextEntete"] =       entete;
    listbinds["TextCorps"] =        corps;
    listbinds["textorigine"] =      txtorigin;
    listbinds["TextPied"] =         pied;
    listbinds["dateimpression"] =   date;
    listbinds["lienversfichier"] =  lienfichier;
    listbinds["useremetteur"] =     DataBase::I()->getUserConnected()->id();
    listbinds["ald"] =              ald;
    if (suffixe != "")
        listbinds[suffixe] =        ba;
    listbinds["emisrecu"] =         emisrecu;
    listbinds["formatdoc"] =        formatdoc;
    listbinds["idlieu"] =           idlieu;


    bool result = DataBase::I()->InsertSQLByBinds(TBL_IMPRESSIONS, listbinds);
    if (!result)
    {
        DataBase::I()->unlocktables();
        return doc;
    }
    return doc;
}

