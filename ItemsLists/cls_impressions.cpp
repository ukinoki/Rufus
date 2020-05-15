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

#include "cls_impressions.h"

/*
 * Impressions
*/

Impressions::Impressions(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, Impression*>();
}

QMap<int, Impression *> *Impressions::impressions() const
{
    return map_all;
}

Impression* Impressions::getById(int id, bool reload)
{
    QMap<int, Impression*>::const_iterator itdoc = map_all->find(id);
    if( itdoc == map_all->constEnd() )
    {
        Impression* impr = DataBase::I()->loadImpressionById(id);
        if (impr)
            add(map_all, impr, Item::Update);
        return impr;
    }
    else if (reload)
    {
        Impression* impr = DataBase::I()->loadImpressionById(id);
        if (impr)
        {
            itdoc.value()->setData(impr->datas());
            delete impr;
        }
    }
    return itdoc.value();
}

/*!
 * \brief Impressions::initListe
 * Charge l'ensemble des impressions accessibles à l'utilisateur en cours
 * et les ajoute à la classe Impressions
 */
void Impressions::initListe()
{
    QList<Impression *> listimpressions = DataBase::I()->loadImpressions();
    epurelist(map_all, &listimpressions);
    addList(map_all, &listimpressions, Item::Update);
    m_isfull = true;
}

void Impressions::SupprimeImpression(Impression* impr)
{
    Supprime(map_all, impr);
}

Impression* Impressions::CreationImpression(QHash<QString, QVariant> sets)
{
    Impression *impr = Q_NULLPTR;
    int idimpr = 0;
    DataBase::I()->locktables(QStringList() << TBL_IMPRESSIONS);
    idimpr = DataBase::I()->selectMaxFromTable(CP_ID_IMPRESSIONS, TBL_IMPRESSIONS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idimpr;
        sets[CP_ID_IMPRESSIONS] = idimpr;
        result = DataBase::I()->InsertSQLByBinds(TBL_IMPRESSIONS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce document d'impression dans la base!"));
        return impr;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_IMPRESSIONS] = idimpr;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_TEXTE_IMPRESSIONS)              data[champ] = itset.value().toString();
        else if (champ == CP_RESUME_IMPRESSIONS)        data[champ] = itset.value().toString();
        else if (champ == CP_CONCLUSION_IMPRESSIONS)    data[champ] = itset.value().toString();
        else if (champ == CP_IDUSER_IMPRESSIONS)        data[champ] = itset.value().toInt();
        else if (champ == CP_DOCPUBLIC_IMPRESSIONS)     data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_PRESCRIPTION_IMPRESSIONS)  data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_EDITABLE_IMPRESSIONS)      data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_MEDICAL_IMPRESSIONS)       data[champ] = (itset.value().toInt() == 1);
    }
    impr = new Impression(data);
    if (impr != Q_NULLPTR)
        map_all->insert(impr->id(), impr);
    return impr;
}


/*
 * Dossiers impression
*/

DossiersImpressions::DossiersImpressions()
{
    map_all = new QMap<int, DossierImpression*>();
}

QMap<int, DossierImpression *> *DossiersImpressions::dossiersimpressions() const
{
    return map_all;
}

DossierImpression* DossiersImpressions::getById(int id, bool reload)
{
    QMap<int, DossierImpression*>::const_iterator itdoc = map_all->find(id);
    if( itdoc == map_all->constEnd() )
    {
        DossierImpression* dossier = DataBase::I()->loadDossierImpressionById(id);
        if (dossier)
            add(map_all, dossier, Item::Update);
        return dossier;
    }
    else if (reload)
    {
        DossierImpression* dossier = DataBase::I()->loadDossierImpressionById(id);
        if (dossier)
        {
            itdoc.value()->setData(dossier->datas());
            delete dossier;
        }
    }
    return itdoc.value();
}

/*!
 * \brief DossiersImpressions::initListe
 * Charge l'ensemble des dossiers d'impressions accessibles à l'utilisateur en cours
 * et les ajoute à la classe DossiersImpressions
 */
void DossiersImpressions::initListe()
{
    QList<DossierImpression *> listdossiers = DataBase::I()->loadDossiersImpressions();
    epurelist(map_all, &listdossiers);
    addList(map_all, &listdossiers);
    m_isfull = true;
}

void DossiersImpressions::SupprimeDossierImpression(DossierImpression* impr)
{
    Supprime(map_all, impr);
}

DossierImpression* DossiersImpressions::CreationDossierImpression(QHash<QString, QVariant> sets)
{
    DossierImpression *impr = Q_NULLPTR;
    int idimpr = 0;
    DataBase::I()->locktables(QStringList() << TBL_DOSSIERSIMPRESSIONS);
    idimpr = DataBase::I()->selectMaxFromTable(CP_ID_DOSSIERIMPRESSIONS, TBL_DOSSIERSIMPRESSIONS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idimpr;
        sets[CP_ID_DOSSIERIMPRESSIONS] = idimpr;
        result = DataBase::I()->InsertSQLByBinds(TBL_DOSSIERSIMPRESSIONS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce document d'impression dans la base!"));
        return impr;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_DOSSIERIMPRESSIONS] = idimpr;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_TEXTE_DOSSIERIMPRESSIONS)              data[champ] = itset.value().toString();
        else if (champ == CP_RESUME_DOSSIERIMPRESSIONS)        data[champ] = itset.value().toString();
        else if (champ == CP_IDUSER_DOSSIERIMPRESSIONS)        data[champ] = itset.value().toInt();
        else if (champ == CP_PUBLIC_DOSSIERIMPRESSIONS)        data[champ] = (itset.value().toInt() == 1);
    }
    impr = new DossierImpression(data);
    if (impr != Q_NULLPTR)
        map_all->insert(impr->id(), impr);
    return impr;
}


