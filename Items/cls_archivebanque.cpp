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

#include "cls_archivebanque.h"

Archive::Archive(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Archive::idcompte() const                   { return m_idcompte;}
int Archive::iddepense() const                  { return m_iddepense;}
int Archive::idrecette() const                  { return m_idrecette;}
int Archive::idrecettespeciale() const          { return m_idrecettespeciale;}
int Archive::idremisecheque() const             { return m_idremisecheque;}
QDate Archive::lignedate() const                { return m_lignedate;}
QString Archive::lignelibelle() const           { return m_lignelibelle;}
double Archive::montant() const                 { return m_montant;}
QString Archive::lignetypeoperation() const     { return m_lignetypeoperation;}
QDate Archive::lignedateconsolidation() const   { return m_lignedateconsolidation;}
int Archive::idarchive() const                  { return m_idarchive;}

void Archive::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, "idligne", m_id);
    Utils::setDataInt(data, "idcompte", m_idcompte);
    Utils::setDataInt(data, "iddepense", m_iddepense);
    Utils::setDataInt(data, "idrecette", m_idrecette);
    Utils::setDataInt(data, "idrecettespeciale", m_idrecettespeciale);
    Utils::setDataInt(data, "idremisecheque", m_idremisecheque);
    Utils::setDataDate(data, "lignedate", m_lignedate);
    Utils::setDataString(data, "lignelibelle", m_lignelibelle);
    Utils::setDataDouble(data, "montant", m_montant);
    Utils::setDataString(data, "lignetypeoperation", m_lignetypeoperation);
    Utils::setDataDate(data, "lignedateconsolidation", m_lignedateconsolidation);
    Utils::setDataInt(data, "idarchive", m_idarchive);
    m_data = data;
}

Archives::Archives()
{
    m_archives = new QMap<int, Archive*>();
}

Archives::~Archives()
{
    clearAll();
    delete m_archives;
}

QMap<int, Archive *> *Archives::archives() const
{
    return m_archives;
}
void Archives::addArchive(Archive *archive)
{
    if( m_archives->contains(archive->id()) )
        return;
    m_archives->insert(archive->id(), archive);
}
void Archives::addArchive(QList<Archive*> listarchives)
{
    foreach (Archive* arch, listarchives)
        addArchive( arch );
}

void Archives::clearAll()
{
    QList<Archive*> listarchs;
    for( auto it = m_archives->begin(); it != m_archives->end(); )
    {
        delete it.value();
        it = m_archives->erase(it);
    }
}

void Archives::removeArchive(Archive *arch)
{
    if (arch == Q_NULLPTR)
        return;
    if( m_archives->find(arch->id()) == m_archives->end() )
        return;
    m_archives->remove(arch->id());
    delete arch;
}
