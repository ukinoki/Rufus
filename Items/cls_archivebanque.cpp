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

    setDataInt(data, "idligne", m_id);
    setDataInt(data, "idcompte", m_idcompte);
    setDataInt(data, "iddepense", m_iddepense);
    setDataInt(data, "idrecette", m_idrecette);
    setDataInt(data, "idrecettespeciale", m_idrecettespeciale);
    setDataInt(data, "idremisecheque", m_idremisecheque);
    setDataDate(data, "lignedate", m_lignedate);
    setDataString(data, "lignelibelle", m_lignelibelle);
    setDataDouble(data, "montant", m_montant);
    setDataString(data, "lignetypeoperation", m_lignetypeoperation);
    setDataDate(data, "lignedateconsolidation", m_lignedateconsolidation);
    setDataInt(data, "idarchive", m_idarchive);
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
    QList<Archive*>::const_iterator it;
    for( it = listarchives.constBegin(); it != listarchives.constEnd(); ++it )
        addArchive( *it );
}

void Archives::clearAll()
{
    QList<Archive*> listarchs;
    for( QMap<int, Archive*>::const_iterator itbq = m_archives->constBegin(); itbq != m_archives->constEnd(); ++itbq)
        delete itbq.value();
    m_archives->clear();
}

void Archives::removeArchive(Archive *arch)
{
    if (arch == Q_NULLPTR)
        return;
    QMap<int, Archive*>::const_iterator itarch = m_archives->find(arch->id());
    if( itarch == m_archives->constEnd() )
        return;
    m_archives->remove(arch->id());
    delete arch;
}
