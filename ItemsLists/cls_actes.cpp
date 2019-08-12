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

#include "cls_actes.h"

Actes::Actes(QObject *parent) : ItemsList(parent)
{
    m_actes = new QMap<int, Acte*>();
    m_actesmodel = Q_NULLPTR;
}

QMap<int, Acte *> *Actes::actes() const
{
    return m_actes;
}

/*!
 * \brief Actes::initListe
 * Charge l'ensemble des actes
 * et les ajoute à la classe Actes
 */
void Actes::initListeByPatient(Patient *pat, Item::UPDATE upd, bool quelesid)
{
    if (pat == Q_NULLPTR)
        return;
    if (upd == Item::NoUpdate)
        clearAll(m_actes);
    QList<Acte*> listActes;
    if (quelesid)
        listActes = DataBase::I()->loadIdActesByPat(pat);
    else
        listActes = DataBase::I()->loadActesByPat(pat);
    addList(m_actes, &listActes, upd);
}

void Actes::sortActesByDate()  /*! cette fonction et les 2 qui suivent ne sont pour l'instant pas utilisées.
                                 * elles sont prévues pour réorganiser le tri des actes en fonction de leur date et pas en fonction de leur id
                                 * parce qu'il arrive (rarement) qu'on saisisse un acte a posteriori dont la date sera antérieure à celle du dernier acte
                                 * si on continue à défiler par id, cet acte n'apparaîtra pas en ordre chronologique mais en dernier. */
{
    // toute la manip qui suit sert à remetre les actes par ordre chronologique - si vous trouvez plus simple, ne vous génez pas
    if (m_actesmodel == Q_NULLPTR)
        m_actesmodel = new QStandardItemModel();
    else
        m_actesmodel->clear();
    foreach (Acte* act, m_actes->values())
    {
        QList<QStandardItem *> items;
        UpStandardItem *itemact = new UpStandardItem(QString::number(act->id()));
        itemact->setitem(act);
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
        return dynamic_cast<Acte *>(item->item());
    else
        return Q_NULLPTR;
}

Acte* Actes::getById(int id, ADDTOLIST add)
{
    QMap<int, Acte*>::const_iterator itact = m_actes->find(id);
    if( itact == m_actes->constEnd() )
    {
        Acte * act = Q_NULLPTR;
        if (add == AddToList)
            act = DataBase::I()->loadActeById(id);
        return act;
    }
    return itact.value();
}

QMap<int, Acte*>::const_iterator Actes::getLast()
{
    return actes()->find(actes()->lastKey());
}

QMap<int, Acte*>::const_iterator Actes::getAt(int idx)
{
    return actes()->find( actes()->keys().at(idx) );
}

void Actes::updateActe(Acte* acte)
{
    if (acte == Q_NULLPTR)
        return;
    acte->setData(DataBase::I()->loadActeAllData(acte->id()));
}

void Actes::setMontantCotation(Acte *act, QString Cotation, double montant)
{
    if ( act == Q_NULLPTR )
        return;
    //on modifie la table Actes avec la nouvelle cotation
    QString cotsql = Cotation;
    if (cotsql == "")
    {
        cotsql = "null";
        montant = 0.00;
    }
    else
        cotsql = "'" + Utils::correctquoteSQL(Cotation) + "'";
    QString requete = "UPDATE " TBL_ACTES
                      " SET " CP_COTATION_ACTES " = " + cotsql + ", " CP_MONTANT_ACTES " = " + QString::number(montant) +
                      " WHERE " CP_IDACTE_ACTES " = " + QString::number(act->id());
    DataBase::I()->StandardSQL(requete);
    act->setcotation(Cotation);
    act->setmontant(montant);
}

void Actes::SupprimeActe(Acte* act)
{
    Supprime(m_actes, act);
}

Acte* Actes::CreationActe(Patient *pat, int idcentre)
{
    if (pat == Q_NULLPTR)
        return Q_NULLPTR;
    Acte *act = Q_NULLPTR;
    bool ok;
    User* usr = DataBase::I()->getUserConnected();
    QString rempla = (usr->modeenregistrementhonoraires() == User::Retrocession? "1" : "null");
    QString creerrequete =
            "INSERT INTO " TBL_ACTES
            " (idPat, idUser, ActeDate, ActeHeure, CreePar, UserComptable, UserParent, SuperViseurRemplacant, NumCentre, idLieu)"
            " VALUES (" +
            QString::number(pat->id()) + ", " +
            QString::number(usr->idsuperviseur()) + ", "
            "NOW(), "
            "NOW(), " +
            QString::number(usr->id()) + ", " +
            QString::number(usr->idcomptable()) + ", " +
            QString::number(usr->idparent()) + ", " +
            rempla + ", " +
            QString::number(idcentre) + ", " +
            QString::number(usr->idsitedetravail()) +")";
    //qDebug() << creerrequete;
    DataBase::I()->locktables(QStringList() << TBL_ACTES);
    if (!DataBase::I()->StandardSQL(creerrequete,tr("Impossible de créer cette consultation dans ") + TBL_ACTES))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    int idacte = DataBase::I()->selectMaxFromTable("idActe", TBL_ACTES, ok, tr("Impossible de retrouver l'acte qui vient d'être créé"));
    if (!ok || idacte == 0)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    DataBase::I()->unlocktables();
    act = new Acte();
    act->setid(idacte);
    act->setidpatient(pat->id());
    act->setiduser(usr->idsuperviseur());
    act->setdate(QDate::currentDate());
    act->setheure(QTime::currentTime());
    act->setidusercreateur(usr->id());
    act->setidusercomptable(usr->idcomptable());
    act->setiduserparent(usr->idparent());
    act->seteffectueparremplacant(rempla == "1");
    act->setnumcentre(idcentre);
    act->setidlieu(usr->idsitedetravail());
    add(m_actes, act);
    return act;
}
