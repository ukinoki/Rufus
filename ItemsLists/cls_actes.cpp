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
void Actes::initListeByPatient(Patient *pat)
{
    clearAll();
    QMap<int, Acte*> listActes = DataBase::I()->loadActesByPat(pat);
    QMap<int, Acte*>::const_iterator itact;
    for( itact = listActes.constBegin(); itact != listActes.constEnd(); ++itact )
    {
        Acte *act = const_cast<Acte*>(*itact);
        add( act );
    }
}

void Actes::add(Acte *acte)
{
    if( m_actes->contains(acte->id()) )
        return;
    m_actes->insert(acte->id(), acte);
}

void Actes::addList(QList<Acte*> listActes)
{
    QList<Acte*>::const_iterator it;
    for( it = listActes.constBegin(); it != listActes.constEnd(); ++it )
        add( *it );
}

void Actes::clearAll()
{
    for( QMap<int, Acte*>::const_iterator itact = m_actes->constBegin(); itact != m_actes->constEnd(); ++itact)
        delete itact.value();
    m_actes->clear();
}

void Actes::remove(Acte *acte)
{
    if (acte == Q_NULLPTR)
        return;
    m_actes->remove(acte->id());
    delete acte;
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
    QString requete = "UPDATE " NOM_TABLE_ACTES
                      " SET ActeCotation = " + cotsql +
                      ", ActeMontant = " + QString::number(montant) +
                      " WHERE idActe = " + QString::number(act->id());
    DataBase::I()->StandardSQL(requete);
    act->setcotation(Cotation);
    act->setmontant(montant);
}

void Actes::updateActeData(Acte *act, QString nomchamp, QVariant value)
{
    QString newvalue;
    if (nomchamp == CP_MOTIFACTES)
    {
        act->setmotif(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_TEXTEACTES)
    {
        act->settexte(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_CONCLUSIONACTES)
    {
        act->setconclusion(value.toString());
        newvalue = "'" + Utils::correctquoteSQL(value.toString()) + "'";
    }
    else if (nomchamp == CP_ACTEDATEACTES)
    {
        act->setdate(value.toDate());
        newvalue = "'" + value.toDate().toString("yyyy-MM-dd") + "'";
    }
    else if (nomchamp == CP_COURRIERAFAIREACTES)
    {
        act->setcourrierafaire(value.toString()== "T" || value.toString()== "1");
        newvalue = ((value.toString() == "T" || value.toString()== "1")? "'T'" : "null");
    }
    QString requete = "UPDATE " NOM_TABLE_ACTES
                      " SET " + nomchamp + " = " + newvalue +
                      " WHERE idActe = " + QString::number(act->id());
    DataBase::I()->StandardSQL(requete);
}

void Actes::SupprimeActe(Acte* act)
{
    DataBase::I()->StandardSQL("DELETE FROM " NOM_TABLE_ACTES " WHERE idActe = " + QString::number(act->id()));
    remove(act);
}

Acte* Actes::CreationActe(Patient *pat, int idcentre)
{
    if (pat == Q_NULLPTR)
        return Q_NULLPTR;
    bool ok;
    User* usr = DataBase::I()->getUserConnected();
    QString rempla = (usr->getEnregHonoraires()==3? "1" : "null");
    QString creerrequete =
            "INSERT INTO " NOM_TABLE_ACTES
            " (idPat, idUser, ActeDate, ActeHeure, CreePar, UserComptable, UserParent,SuperViseurRemplacant, NumCentre, idLieu)"
            " VALUES (" +
            QString::number(pat->id()) + ", " +
            QString::number(usr->getIdUserActeSuperviseur()) + ", "
            "NOW(), "
            "NOW(), " +
            QString::number(usr->id()) + ", " +
            QString::number(usr->getIdUserComptable()) + ", " +
            QString::number(usr->getIdUserParent()) + ", " +
            rempla + ", " +
            QString::number(idcentre) + ", " +
            QString::number(usr->getSite()->id()) +")";
    //qDebug() << creerrequete;
    DataBase::I()->locktables(QStringList() << NOM_TABLE_ACTES);
    if (!DataBase::I()->StandardSQL(creerrequete,tr("Impossible de créer cette consultation dans ") + NOM_TABLE_ACTES))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    int idacte = DataBase::I()->selectMaxFromTable("idActe", NOM_TABLE_ACTES, ok, tr("Impossible de retrouver l'acte qui vient d'être créé"));
    if (!ok || idacte == 0)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    DataBase::I()->unlocktables();
    return DataBase::I()->loadActeById(idacte);
}
