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

#include "cls_comptes.h"

Comptes::Comptes(QObject * parent) : ItemsList(parent)
{
    map_all = new QMap<int, Compte*>();
}

Comptes::~Comptes()
{
    clearAll(map_all);
    delete map_all;
}

QMap<int, Compte*>* Comptes::comptes() const
{
    return map_all;
}

Compte* Comptes::getById(int id, bool reload)
{
    QMap<int, Compte*>::const_iterator itcpt = map_all->constFind(id);
    if( itcpt == map_all->constEnd() )
    {
        Compte *cpt = DataBase::I()->loadCompteById(id);
        if (cpt)
            add( map_all, cpt );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<Compte*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        Compte *cpt = DataBase::I()->loadCompteById(id);
        if (cpt)
        {
            itcpt.value()->setData(cpt->datas());
            delete cpt;
        }
    }
    return itcpt.value();
}

void Comptes::initListe()
{
    QList<Compte*> listcomptes = DataBase::I()->loadComptesAll();
    epurelist(map_all, &listcomptes);
    addList(map_all, &listcomptes);
}

void Comptes::SupprimeCompte(Compte *cpt)
{
    Supprime(map_all, cpt);
}

QMap<int, bool> Comptes::initListeComptesByIdUser(int id)
{
    QMap<int, bool> mapcomptes;
    foreach (Compte *cpt, *map_all)
        if (cpt->idUser() == id)
            mapcomptes.insert(cpt->id(), cpt->isDesactive());
    return mapcomptes;
}


Compte* Comptes::CreationCompte(int idBanque, int idUser, QString IBAN, QString IntituleCompte, QString NomCompteAbrege, double SoldeSurDernierReleve, bool Partage, bool Desactive)
{
    Compte *cpt = Q_NULLPTR;
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
    int idcpt = DataBase::I()->selectMaxFromTable(CP_ID_COMPTES, TBL_COMPTES, m_ok, tr("Impossible de sélectionner les enregistrements"));
    if (!m_ok)
    {
        DataBase::I()->unlocktables();
        return Q_NULLPTR;
    }
    DataBase::I()->unlocktables();
    cpt = new Compte();
    cpt->setid(idcpt);
    cpt->setidbanque(idBanque);
    cpt->setiduser(idUser);
    cpt->setiban(IBAN);
    cpt->setintitulecompte(IntituleCompte);
    cpt->setnomabrege(NomCompteAbrege);
    cpt->setsolde(SoldeSurDernierReleve);
    cpt->setpartage(Partage);
    cpt->setdesactive(Desactive);
    add(map_all, cpt);
    return cpt;
}
