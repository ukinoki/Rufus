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

#include "cls_banques.h"

Banques::Banques(QObject *parent) : ItemsList(parent)
{
    map_banques = new QMap<int, Banque*>();
}

QMap<int, Banque *> *Banques::banques() const
{
    return map_banques;
}

/*!
 * \brief Banques::initListe
 * Charge l'ensemble des banques
 * et les ajoute à la classe Banques
 */
void Banques::initListe()
{
    QList<Banque*> listbanques = DataBase::I()->loadBanques();
    epurelist(map_banques, &listbanques);
    addList(map_banques, &listbanques);
}

Banque* Banques::getById(int id)
{
    QMap<int, Banque*>::const_iterator itcpt = map_banques->find(id);
    if( itcpt == map_banques->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void Banques::SupprimeBanque(Banque *bq)
{
    Supprime(map_banques, bq);
}

 Banque* Banques::CreationBanque(QString idBanqueAbrege, QString NomBanque, int CodeBanque)
{
    Banque *bq = Q_NULLPTR;

    QString idabrege        = (idBanqueAbrege == ""?    "null" : "'" + Utils::correctquoteSQL(idBanqueAbrege) + "'");
    QString nombq           = (NomBanque == ""?         "null" : "'" + Utils::correctquoteSQL(NomBanque) + "'");
    QString codebq          = (CodeBanque == 0?         "null" : QString::number(CodeBanque));
    QString req =     "INSERT INTO " TBL_BANQUES "(" CP_NOMABREGE_BANQUES ", " CP_NOMBANQUE_BANQUES ", " CP_CODE_BANQUES ")"
                      " VALUES (" + idabrege + ", " + nombq + "," + codebq +  ")";
    QString MsgErreur  = tr("Impossible de créer cette banque");
    DataBase::I()->locktable(TBL_BANQUES);
    if (!DataBase::I()->StandardSQL(req, MsgErreur))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    // Récupération de l'idMotif créé ------------------------------------
    int idbq = DataBase::I()->selectMaxFromTable(CP_IDBANQUE_BANQUES, TBL_BANQUES, m_ok, tr("Impossible de sélectionner les enregistrements"));
    if (!m_ok)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    DataBase::I()->unlocktables();
    bq = new Banque();
    bq->setid(idbq);
    bq->setnomabrege(idBanqueAbrege);
    bq->setnom(NomBanque);
    bq->setcode(CodeBanque);
    add(map_banques, bq);
    return bq;
}
