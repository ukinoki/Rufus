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

#include "cls_comptes.h"

Comptes::Comptes(QObject * parent) : ItemsList(parent)
{
    m_comptes = new QMap<int, Compte*>();
}

Comptes::~Comptes()
{
    clearAll(m_comptes);
    delete m_comptes;
}

QMap<int, Compte*>* Comptes::comptes() const
{
    return m_comptes;
}

void Comptes::addList(QList<Compte*> listCompte)
{
    QList<Compte*>::const_iterator it;
    for( it = listCompte.constBegin(); it != listCompte.constEnd(); ++it )
    {
        Compte* item = const_cast<Compte*>(*it);
        add( m_comptes, item->id(), item );
    }
}

Compte* Comptes::getById(int id)
{
    QMap<int, Compte*>::const_iterator itcpt = m_comptes->find(id);
    if( itcpt == m_comptes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void Comptes::reloadCompte(Compte *compte)
{
    compte->setData(DataBase::I()->loadCompteById(compte->id()));
}

void Comptes::initListe()
{
    QList<Compte*> listcomptes;
    addList(DataBase::I()->loadComptesAll());
}

void Comptes::SupprimeCompte(Compte *cpt)
{
    DataBase::I()->SupprRecordFromTable(cpt->id(), CP_IDCOMPTE_COMPTES, TBL_COMPTES);
    remove(m_comptes, cpt);

}
Compte* Comptes::CreationCompte(int idBanque, int idUser, QString IBAN, QString IntituleCompte, QString NomCompteAbrege, double SoldeSurDernierReleve, bool Partage, bool Desactive)
{
    Compte *cpt = Q_NULLPTR;
    bool ok;
    QString idbq            = (idBanque == 0?               "null" : QString::number(idBanque));
    QString idusr           = (idUser == 0?                 "null" : QString::number(idUser));
    QString iban            = (IBAN == ""?                  "null" : "'" + Utils::correctquoteSQL(IBAN) + "'");
    QString intitule        = (IntituleCompte == ""?        "null" : "'" + Utils::correctquoteSQL(IntituleCompte) + "'");
    QString nomcpt          = (NomCompteAbrege == ""?       "null" : "'" + Utils::correctquoteSQL(NomCompteAbrege) + "'");
    QString solde           = QString::number(SoldeSurDernierReleve, 'f',2);
    QString partage         = (!Partage?                    "null" : "1");
    QString desactive       = (!Desactive?                  "null" : "1");
    QString req =     "INSERT INTO " TBL_COMPTES
            " (idBanque, idUser, IBAN, IntituleCompte, NomCompteAbrege, SoldeSurDernierReleve, Partage, Desactive)"
            " VALUES (" +   idbq + "," +
            idusr + "," +
            iban + "," +
            intitule   + "," +
            nomcpt   + "," +
            solde   + "," +
            partage   + "," +
            desactive + ")";
    QString MsgErreur           = tr("Impossible de créer ce compte");
    DataBase::I()->locktables(QStringList() << TBL_COMPTES);
    if (!DataBase::I()->StandardSQL(req, MsgErreur))
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    // Récupération de l'idMotif créé ------------------------------------
    int idcpt = DataBase::I()->selectMaxFromTable(CP_IDCOMPTE_COMPTES, TBL_COMPTES, ok, tr("Impossible de sélectionner les enregistrements"));
    DataBase::I()->unlocktables();
    QJsonObject jData{};
    jData["id"]             = idcpt;
    jData["idbanque"]       = idBanque;
    jData["iduser"]         = idUser;
    jData["IBAN"]           = IBAN;
    jData["IntituleCompte"] = IntituleCompte;
    jData["nom"]            = NomCompteAbrege;
    jData["solde"]          = SoldeSurDernierReleve;
    jData["partage"]        = Partage;
    jData["desactive"]      = Desactive;
    cpt = new Compte(jData);
    add(m_comptes, cpt->id(), cpt);
    return cpt;
}
