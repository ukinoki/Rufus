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

#include "cls_motscles.h"

MotsCles::MotsCles(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, MotCle*>();
}

QMap<int, MotCle*>* MotsCles::motscles() const
{
    return map_all;
}

MotCle* MotsCles::getById(int id, bool reload)
{
    QMap<int, MotCle*>::const_iterator itref = map_all->constFind(id);
    if( itref == map_all->constEnd() )
    {
        MotCle * itm = DataBase::I()->loadMotCleById(id);
        if (itm != Q_NULLPTR)
            add( map_all, itm );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<MotCle*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        MotCle* itm = DataBase::I()->loadMotCleById(id);
        if (itm)
        {
            itref.value()->setData(itm->datas());
            delete itm;
        }
    }
    return itref.value();
}

/*!
 * \brief MotsCles::initListe
 * Charge l'ensemble des motscles
 * et les ajoute à la classe MotsCles
 */
void MotsCles::initListe()
{
    QList<MotCle*> listMotsCles = DataBase::I()->loadMotsCles();
    epurelist(map_all, &listMotsCles);
    addList(map_all, &listMotsCles, Item::Update);
}

QCompleter* MotsCles::completer()
{
    QStandardItemModel *model = new QStandardItemModel();
    for (auto it = map_all->constBegin(); it != map_all->constEnd(); ++it)
    {
        MotCle *mc = const_cast<MotCle*>(it.value());
        model->appendRow(new QStandardItem(mc->motcle()));
    }
    model->sort(0);
    m_completer->setModel(model);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::InlineCompletion);
    return m_completer;
}

void MotsCles::SupprimeMotCle(MotCle* motcle)
{
    Supprime(map_all, motcle);
}

MotCle* MotsCles::CreationMotCle(QHash<QString, QVariant> sets)
{
    MotCle *motcle = Q_NULLPTR;
    int idmotcle = 0;
    DataBase::I()->locktables(QStringList() << TBL_MOTSCLES);
    idmotcle = DataBase::I()->selectMaxFromTable(CP_ID_MOTCLE, TBL_MOTSCLES, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idmotcle;
        sets[CP_ID_MOTCLE] = idmotcle;
        result = DataBase::I()->InsertSQLByBinds(TBL_MOTSCLES, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce mot-clé dans la base!"));
        return motcle;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_MOTCLE] = idmotcle;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_TEXT_MOTCLE)              data[champ] = itset.value().toString();
    }
    motcle = new MotCle(data);
    if (motcle != Q_NULLPTR)
        map_all->insert(motcle->id(), motcle);
    return motcle;
}

bool MotsCles::isThisMCusedForOtherPatients(MotCle* mc, int idpat)
{
    if (!mc)
        return false;
    bool ok;
    QList<QVariantList> listid = QList<QVariantList>();
    QString req = "select " CP_IDPATIENT_JOINTURESMOTSCLES " from " TBL_MOTSCLESJOINTURES " where " CP_IDPATIENT_JOINTURESMOTSCLES " <> " + QString::number(idpat) + " and " CP_IDMOTCLE_JOINTURESMOTSCLES " = " + QString::number(mc->id());
    //qDebug() << req;
    listid = DataBase::I()->StandardSelectSQL(req, ok);
    return (listid.size()>0);
}
