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
    m_banques = new QMap<int, Banque*>();
}

QMap<int, Banque *> *Banques::banques() const
{
    return m_banques;
}

/*!
 * \brief Banques::initListE_BANQUES
 * Charge l'ensemble des banques
 * et les ajoute à la classe Banques
 */
void Banques::initListe()
{
    clearAll(m_banques);
    addList(DataBase::I()->loadBanques());
}

void Banques::addList(QList<Banque*> listbanques)
{
    QList<Banque*>::const_iterator it;
    for( it = listbanques.constBegin(); it != listbanques.constEnd(); ++it )
    {
        Banque* item = const_cast<Banque*>(*it);
        add( m_banques, item->id(), item );
    }
}

Banque* Banques::getById(int id)
{
    QMap<int, Banque*>::const_iterator itcpt = m_banques->find(id);
    if( itcpt == m_banques->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void Banques::SupprimeBanque(Banque *bq)
{
    DataBase::I()->SupprRecordFromTable(bq->id(), CP_IDBANQUE_BANQUES, TBL_BANQUES);
    remove(m_banques, bq);
}

 Banque*    Banques::CreationBanque(QString idBanqueAbrege, QString NomBanque, int CodeBanque)
{
    Banque *bq = Q_NULLPTR;
    bool ok;
    QString idabrege        = (idBanqueAbrege == ""?    "null" : "'" + Utils::correctquoteSQL(idBanqueAbrege) + "'");
    QString nombq           = (NomBanque == ""?         "null" : "'" + Utils::correctquoteSQL(NomBanque) + "'");
    QString codebq          = (CodeBanque == 0?         "null" : QString::number(CodeBanque));
    QString req =     "INSERT INTO " TBL_BANQUES
                        " (Motif,Raccourci, Couleur, Duree, ParDefaut, Utiliser, NoOrdre)"
                        " VALUES (" +   idabrege + "," +
                                        nombq + "," +
                                        codebq +  ")";
    QString MsgErreur  = tr("Impossible de créer cette banque");
    DataBase::I()->locktables(QStringList() << TBL_BANQUES);
    if (!DataBase::I()->StandardSQL(req, MsgErreur))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    // Récupération de l'idMotif créé ------------------------------------
    int idbq = DataBase::I()->selectMaxFromTable(CP_IDBANQUE_BANQUES, TBL_BANQUES, ok, tr("Impossible de sélectionner les enregistrements"));
    DataBase::I()->unlocktables();
    QJsonObject jData{};
    jData["id"] = idbq;
    jData["idbanqueabrege"] = idBanqueAbrege;
    jData["nombanque"] = NomBanque;
    jData["codebanque"] = CodeBanque;
    bq = new Banque(jData);
    if (bq != Q_NULLPTR)
        add(m_banques, bq->id(), bq);
    return bq;
}
