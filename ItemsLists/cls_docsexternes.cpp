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
    m_docsexternes      = new QMap<int, DocExterne*>();
    m_nouveaudocument   = false;
    m_patient           = Q_NULLPTR;
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
        add( m_docsexternes, result );
    return result;
}

bool DocsExternes::NouveauDocumentExterne()
{
    return m_nouveaudocument;
}

void DocsExternes::setNouveauDocumentExterneFalse()
{
    m_nouveaudocument = false;
}

void DocsExternes::addList(QList<DocExterne*> listdocs)
{
    foreach (DocExterne* doc, listdocs)
    {
        if(!m_docsexternes->contains(doc->id()))
            m_nouveaudocument = true;
        add(m_docsexternes, doc);
    }
}

/*!
 * \brief DocsExternes::initListeByPatient
 * Charge l'ensemble des documents externes pour un patient
 * et les ajoute à la classe Patients
 */
void DocsExternes::initListeByPatient(Patient *pat)
{
    m_patient = pat;
    QList<DocExterne*> listdocs = DataBase::I()->loadDoscExternesByPatient(pat);
    epurelist(m_docsexternes, &listdocs);
    addList(listdocs);
}

void DocsExternes::actualise()
{
    m_nouveaudocument = false;
    addList(DataBase::I()->loadDoscExternesByPatient(m_patient));
}

void DocsExternes::SupprimeDocumentExterne(DocExterne *doc)
{
    if (doc == Q_NULLPTR)
        return;
    DataBase::I()->StandardSQL("delete from " TBL_ECHANGEIMAGES " where idimpression = " + QString::number(doc->id()));
    Supprime(m_docsexternes, doc);
}

DocExterne* DocsExternes::CreationDocumentExterne(QHash<QString, QVariant> sets)
{
    DocExterne *doc = Q_NULLPTR;
    DataBase::I()->locktables(QStringList() << TBL_DOCSEXTERNES);
    bool result = DataBase::I()->InsertSQLByBinds(TBL_DOCSEXTERNES, sets);
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce document dans la base!"));
        DataBase::I()->unlocktables();
        return doc;
    }
    // Récupération de l'iddocument créé ------------------------------------
    int iddoc = 0;
    QHash<QString, QVariant>::const_iterator itx = sets.find(CP_IDIMPRESSION_IMPRESSIONS);
    if (itx != sets.constEnd())
        iddoc = itx.value().toInt();
    else
    {
        bool ok;
        iddoc = DataBase::I()->selectMaxFromTable(CP_IDIMPRESSION_IMPRESSIONS, TBL_DOCSEXTERNES, ok, tr("Impossible de sélectionner les enregistrements"));
        if (!ok)
        {
            DataBase::I()->unlocktables();
            return Q_NULLPTR;
        }
    }
    DataBase::I()->unlocktables();
    if (iddoc == 0)
        return Q_NULLPTR;
    QJsonObject  data = QJsonObject{};
    data[CP_IDIMPRESSION_IMPRESSIONS] = iddoc;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDUSER_IMPRESSIONS)                 data[champ] = itset.value().toInt();
        else if (champ == CP_IDPAT_IMPRESSIONS)             data[champ] = itset.value().toInt();
        else if (champ == CP_TYPEDOC_IMPRESSIONS)           data[champ] = itset.value().toString();
        else if (champ == CP_SOUSTYPEDOC_IMPRESSIONS)       data[champ] = itset.value().toString();
        else if (champ == CP_TITRE_IMPRESSIONS)             data[champ] = itset.value().toString();
        else if (champ == CP_TEXTENTETE_IMPRESSIONS)        data[champ] = itset.value().toString();
        else if (champ == CP_TEXTCORPS_IMPRESSIONS)         data[champ] = itset.value().toString();
        else if (champ == CP_TEXTORIGINE_IMPRESSIONS)       data[champ] = itset.value().toString();
        else if (champ == CP_TEXTPIED_IMPRESSIONS)          data[champ] = itset.value().toString();
        else if (champ == CP_DATE_IMPRESSIONS)              data[champ] = QDateTime(itset.value().toDate(), itset.value().toTime()).toMSecsSinceEpoch();
        else if (champ == CP_COMPRESSION_IMPRESSIONS)       data[champ] = itset.value().toInt();
        else if (champ == CP_LIENFICHIER_IMPRESSIONS)       data[champ] = itset.value().toString();
        else if (champ == CP_ALD_IMPRESSIONS)               data[champ] = itset.value().toInt();
        else if (champ == CP_IDEMETTEUR_IMPRESSIONS)        data[champ] = itset.value().toInt();
        else if (champ == CP_FORMATDOC_IMPRESSIONS)         data[champ] = itset.value().toString();
        else if (champ == CP_IMPORTANCE_IMPRESSIONS)        data[champ] = itset.value().toInt();
        else if (champ == CP_EMISORRECU_IMPRESSIONS)        data[champ] = itset.value().toInt();
        else if (champ == CP_IDLIEU_IMPRESSIONS)            data[champ] = itset.value().toInt();
    }
    doc = new DocExterne(data);
    return doc;
}
