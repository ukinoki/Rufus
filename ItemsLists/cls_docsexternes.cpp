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

#include "cls_docsexternes.h"

DocsExternes::DocsExternes(QObject *parent) : ItemsList(parent)
{
    map_docsexternes      = new QMap<int, DocExterne*>();
    m_nouveaudocument   = false;
    m_patient           = Q_NULLPTR;
}

QMap<int, DocExterne *> *DocsExternes::docsexternes()
{
    return map_docsexternes;
}

/*!
 * \brief docsexternes::getById
 * \param id l'idimpr du Document recherché
 * \return Q_NULLPTR si aucun Document trouvée
 * \return DocExterne* le Document correspondant à l'id
 */
DocExterne* DocsExternes::getById(int id, Item::LOADDETAILS loadDetails, ADDTOLIST addToList)
{
    QMap<int, DocExterne*>::const_iterator itdoc = map_docsexternes->constFind(id);
    DocExterne *result;
    if( itdoc == map_docsexternes->constEnd() )
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
        add( map_docsexternes, result );
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
        if (doc == Q_NULLPTR)
            continue;;
        if(!map_docsexternes->contains(doc->id()))
            m_nouveaudocument = true;
        add(map_docsexternes, doc);
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
    QList<DocExterne*> list = DataBase::I()->loadDoscExternesByPatient(pat);
    epurelist(map_docsexternes, &list);
    addList(list);
}

void DocsExternes::actualise()
{
    m_nouveaudocument = false;
    QList<DocExterne*> list = DataBase::I()->loadDoscExternesByPatient(m_patient);
    addList(list);
}

void DocsExternes::SupprimeDocumentExterne(DocExterne *doc)
{
    if (doc)
    {
        DataBase::I()->SupprRecordFromTable(doc->id(), CP_ID_ECHGIMAGES, TBL_ECHANGEIMAGES);
        Supprime(map_docsexternes, doc);
    }
}

DocExterne* DocsExternes::CreationDocumentExterne(QHash<QString, QVariant> sets)
{
    bool ok;
    DocExterne *doc = Q_NULLPTR;
    int iddoc = 0;
    DataBase::I()->locktable(TBL_DOCSEXTERNES);
    iddoc = DataBase::I()->selectMaxFromTable(CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES, ok);
    bool result = (ok);
    if (result)
    {
        ++ iddoc;
        sets[CP_ID_DOCSEXTERNES] = iddoc;
        result = DataBase::I()->InsertSQLByBinds(TBL_DOCSEXTERNES, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce document dans la base!"));
        return doc;
    }

    QJsonObject  data = QJsonObject{};
    data[CP_ID_DOCSEXTERNES] = iddoc;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDUSER_DOCSEXTERNES)                 data[champ] = itset.value().toInt();
        else if (champ == CP_IDPAT_DOCSEXTERNES)             data[champ] = itset.value().toInt();
        else if (champ == CP_TYPEDOC_DOCSEXTERNES)           data[champ] = itset.value().toString();
        else if (champ == CP_SOUSTYPEDOC_DOCSEXTERNES)       data[champ] = itset.value().toString();
        else if (champ == CP_TITRE_DOCSEXTERNES)             data[champ] = itset.value().toString();
        else if (champ == CP_TEXTENTETE_DOCSEXTERNES)        data[champ] = itset.value().toString();
        else if (champ == CP_TEXTCORPS_DOCSEXTERNES)         data[champ] = itset.value().toString();
        else if (champ == CP_TEXTORIGINE_DOCSEXTERNES)       data[champ] = itset.value().toString();
        else if (champ == CP_TEXTPIED_DOCSEXTERNES)          data[champ] = itset.value().toString();
        else if (champ == CP_DATE_DOCSEXTERNES)              data[champ] = QDateTime(itset.value().toDate(), itset.value().toTime()).toMSecsSinceEpoch();
        else if (champ == CP_COMPRESSION_DOCSEXTERNES)       data[champ] = itset.value().toInt();
        else if (champ == CP_LIENFICHIER_DOCSEXTERNES)       data[champ] = itset.value().toString();
        else if (champ == CP_ALD_DOCSEXTERNES)               data[champ] = itset.value().toInt();
        else if (champ == CP_IDEMETTEUR_DOCSEXTERNES)        data[champ] = itset.value().toInt();
        else if (champ == CP_FORMATDOC_DOCSEXTERNES)         data[champ] = itset.value().toString();
        else if (champ == CP_IMPORTANCE_DOCSEXTERNES)        data[champ] = itset.value().toInt();
        else if (champ == CP_EMISORRECU_DOCSEXTERNES)        data[champ] = itset.value().toInt();
        else if (champ == CP_IDLIEU_DOCSEXTERNES)            data[champ] = itset.value().toInt();
        else if (champ == CP_IDREFRACTION_DOCSEXTERNES)      data[champ] = itset.value().toInt();

    }
    doc = new DocExterne(data);
    return doc;
}
