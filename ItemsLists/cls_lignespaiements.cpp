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

#include "cls_lignespaiements.h"

LignesPaiements::LignesPaiements(QObject *parent) : ItemsList(parent)
{
    m_lignespaiements = new QMap<QString, LignePaiement*>();
}

QMap<QString, LignePaiement *>* LignesPaiements::lignespaiements() const
{
    return m_lignespaiements;
}

void LignesPaiements::addList(QList<LignePaiement*> listlignes)
{
    QList<LignePaiement*>::const_iterator it;
    for( it = listlignes.constBegin(); it != listlignes.constEnd(); ++it )
    {
        LignePaiement* item = const_cast<LignePaiement*>(*it);
        add( m_lignespaiements, item );
    }
}

LignePaiement* LignesPaiements::getById(QString stringid)
{
    QMap<QString, LignePaiement*>::const_iterator itcpt = m_lignespaiements->find(stringid);
    if( itcpt == m_lignespaiements->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

/*!
 * \brief LignesPaiements::initListe
 * Charge l'ensemble des lignes de paiement pour les actes concernant un patient
 * et les ajoute à la classe LignesPaiements
 */
void LignesPaiements::initListeByPatient(Patient *pat)
{
    if (pat == Q_NULLPTR)
        return;
    clearAll(m_lignespaiements);
    addList(DataBase::I()->loadlignespaiementsByPatient(pat));
}

void LignesPaiements::SupprimeActeLignesPaiements(Acte* act)
{
    if (act == Q_NULLPTR)
        return;
    DataBase::I()->StandardSQL("DELETE FROM " TBL_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(act->id()));
    QList<LignePaiement*> listlignesasupprimer = QList<LignePaiement*>();
    for (QMap<QString, LignePaiement*>::const_iterator itlign = m_lignespaiements->constBegin() ; itlign != m_lignespaiements->constEnd(); ++itlign)
    {
        LignePaiement *lign = const_cast<LignePaiement*>(itlign.value());
        listlignesasupprimer << lign;
    }
    if (listlignesasupprimer.size() > 0)
        for (int i=0; i<listlignesasupprimer.size(); ++i)
            remove(m_lignespaiements, listlignesasupprimer.at(i));
}

