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

#include "cls_actes.h"

Actes::Actes(QObject *parent) : ItemsList(parent)
{
    map_actes = new QMap<int, Acte*>();
    m_actesmodel = Q_NULLPTR;
}

QMap<int, Acte *> *Actes::actes() const
{
    return map_actes;
}

void Actes::setcurrentacte(Acte *act)
{
    m_currentacte = act;
}

/*!
 * \brief Actes::initListe
 * Charge l'ensemble des actes
 * et les ajoute à la classe Actes
 * upd = NoUpdate par defaut
 * quelesid = false par défaut
 */
void Actes::initListeByPatient(Patient *pat, Item::UPDATE upd, bool quelesid)
{
    if (pat == Q_NULLPTR)
        return;
    if (upd == Item::NoUpdate)
        clearAll(map_actes);
    QList<Acte*> listActes;
    if (quelesid)
        listActes = DataBase::I()->loadIdActesByPat(pat);
    else
        listActes = DataBase::I()->loadActesByPat(pat);
    addList(map_actes, &listActes, upd);
}

void Actes::sortActesByDate()  /*! cette fonction n'est pour l'instant pas utilisée.
                                 * elle est prévue pour réorganiser le tri des actes en fonction de leur date et pas en fonction de leur id
                                 * parce qu'il arrive (rarement) qu'on saisisse un acte a posteriori dont la date sera antérieure à celle du dernier acte
                                 * si on continue à défiler par id, cet acte n'apparaîtra pas en ordre chronologique mais en dernier. */
{
    // toute la manip qui suit sert à remetre les actes par ordre chronologique - si vous trouvez plus simple, ne vous génez pas
    if (m_actesmodel != Q_NULLPTR)
        delete m_actesmodel;
    m_actesmodel = new QStandardItemModel(this);
    for (auto it = map_actes->constBegin(); it != map_actes->constEnd(); ++it)
    {
        Acte *act = const_cast<Acte*>(it.value());
        QList<QStandardItem *> items;
        UpStandardItem *itemact = new UpStandardItem(QString::number(act->id()), act);
        items << new UpStandardItem(act->date().toString("yyyymmss"))
              << new UpStandardItem(act->heure().toString("HHmm"))
              << itemact;
        m_actesmodel->appendRow(items);
    }
    m_heuresortmodel = new QSortFilterProxyModel();
    m_heuresortmodel->setSourceModel(m_actesmodel);
    m_heuresortmodel->sort(1);

    m_actesortmodel = new QSortFilterProxyModel();
    m_actesortmodel->setSourceModel(m_heuresortmodel);
    m_actesortmodel->sort(0);
}

Acte* Actes::getActeFromRow(int row)
{
    QModelIndex psortindx = m_actesortmodel->index(row, 2);
    return getActeFromIndex(psortindx);
}

Acte* Actes::getActeFromIndex(QModelIndex idx)
{
    QModelIndex heureindx   = m_actesortmodel->mapToSource(idx);                      //  -> m_heuresortmodel
    QModelIndex pindx       = m_heuresortmodel->mapToSource(heureindx);               //  -> m_actesmodel
    UpStandardItem *item = dynamic_cast<UpStandardItem *>(m_actesmodel->itemFromIndex(pindx));
    if (item != Q_NULLPTR)
        return qobject_cast<Acte *>(item->item());
    else
        return Q_NULLPTR;
}

/*!
 * \brief Actes::getById -> charge un acte à partir de son id
 * \param id
 * \param details si l'acte n'est pas dans liste et si details = LoadDetails => va chercher l'acte dans la BDD sinon, renvoie Q_NULLPTR
 * \return
 * +++++ cette fonction n'ajoute pas l'acte à la map_actes quelquesoit son résultat
 */
Acte* Actes::getById(int id, Item::LOADDETAILS details)
{
    Acte * act = Q_NULLPTR;
    QMap<int, Acte*>::const_iterator itact = map_actes->constFind(id);
    if( itact == map_actes->constEnd() )
    {
        if (details == Item::LoadDetails)
            act = DataBase::I()->loadActeById(id);
    }
    else
        act = const_cast<Acte*>(itact.value());
    return act;
}

QMap<int, Acte*>::const_iterator Actes::getLast()
{
    return actes()->constFind(actes()->lastKey());
}

QMap<int, Acte*>::const_iterator Actes::getAt(int idx)
{
    return actes()->constFind(actes()->keys().at(idx) );
}

QMap<int, Acte *> *Actes::listCourriersByUser(int iduser)
{
    QMap<int, Acte *> *listactes = new QMap<int, Acte *>;
    QString req = "select " CP_ID_ACTES " from " TBL_ACTES " where " CP_COURRIERAFAIRE_ACTES " = 'T' and " CP_IDUSER_ACTES " = " + QString::number(iduser);
    QList<QVariantList> acts  = DataBase::I()->StandardSelectSQL(req, m_ok);
    if (m_ok)
        for (int i=0; i<acts.size(); i++)
        {
            Acte *act = getById(acts.at(i).at(0).toInt());
            if (act)
                listactes->insert(act->id(), act);
        }
    return listactes;
}

void Actes::updateActe(Acte* acte)
{
    if (acte == Q_NULLPTR)
        return;
    acte->setData(DataBase::I()->loadActeAllData(acte->id()));
}

void Actes::SupprimeActe(Acte* act)
{
    Supprime(map_actes, act);
}

Acte* Actes::CreationActe(Patient *pat, User* usr, int idcentre, int idlieu)
{
    if (pat == Q_NULLPTR)
        return Q_NULLPTR;
    Acte *act = Q_NULLPTR;
    QString rempla = (usr->isRemplacant()? "1" : "null");
    QString comptable = (usr->idcomptableactes() > 0? QString::number(usr->idcomptableactes()) : "null");
    QString creerrequete =
            "INSERT INTO " TBL_ACTES
            " (" CP_IDPAT_ACTES ", " CP_IDUSER_ACTES ", " CP_DATE_ACTES ", " CP_HEURE_ACTES ", " CP_IDUSERCREATEUR_ACTES ", "
               CP_IDUSERCOMPTABLE_ACTES ", " CP_IDUSERPARENT_ACTES ", " CP_SUPERVISEURREMPLACANT_ACTES ", " CP_NUMCENTRE_ACTES ", " CP_IDLIEU_ACTES ")"
            " VALUES (" +
            QString::number(pat->id()) + ", " +
            QString::number(usr->idsuperviseur()) + ", "
            "NOW(), "
            "NOW(), " +
            QString::number(usr->id()) + ", " +
            comptable + ", " +
            QString::number(usr->idparent()) + ", " +
            rempla + ", " +
            QString::number(idcentre) + ", " +
            QString::number(idlieu) +")";
    //qDebug() << creerrequete;
    DataBase::I()->locktable(TBL_ACTES);
    if (!DataBase::I()->StandardSQL(creerrequete,tr("Impossible de créer cette consultation dans ") + TBL_ACTES))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    int idacte = DataBase::I()->selectMaxFromTable(CP_ID_ACTES, TBL_ACTES, m_ok, tr("Impossible de retrouver l'acte qui vient d'être créé"));
    DataBase::I()->unlocktables();
    if (!m_ok)
        return Q_NULLPTR;
    act = new Acte();
    act->setid(idacte);
    act->setidpatient(pat->id());
    act->setidsuperviseur(usr->idsuperviseur());
    QDateTime dt = DataBase::I()->ServerDateTime();
    act->setdate(dt.date());
    act->setheure(dt.time());
    act->setidcreateur(usr->id());
    act->setidcomptable(usr->idcomptableactes());
    act->setidparent(usr->idparent());
    act->seteffectueparremplacant(rempla == "1");
    act->setnumcentre(idcentre);
    act->setidlieu(idlieu);
    add(map_actes, act);
    return act;
}
