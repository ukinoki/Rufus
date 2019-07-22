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
    addList(m_lignespaiements, DataBase::I()->loadlignespaiementsByPatient(pat));
}

void LignesPaiements::SupprimeActeLignesPaiements(Acte* act)
{
    if (act == Q_NULLPTR)
        return;
    DataBase::I()->StandardSQL("DELETE FROM " TBL_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(act->id()));
    QList<LignePaiement*> listlignesasupprimer = QList<LignePaiement*>();
    for (auto itlign = m_lignespaiements->begin() ; itlign != m_lignespaiements->end();)
    {
        LignePaiement *lign = const_cast<LignePaiement*>(itlign.value());
        if (lign->idacte() == act->id())
        {
            itlign = m_lignespaiements->erase(itlign);
            delete lign;
        }
        else
            ++ itlign;
    }
}

