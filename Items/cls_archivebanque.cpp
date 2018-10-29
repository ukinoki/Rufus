#include "cls_archivebanque.h"

Archive::Archive(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Archive::id() const                         { return m_idligne;}
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

    setDataInt(data, "idligne", m_idligne);
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
}

QMap<int, Archive *> Archives::archives() const
{
    return m_archives;
}
void Archives::addArchive(Archive *archive)
{
    if( m_archives.contains(archive->id()) )
        return;
    m_archives.insert(archive->id(), archive);
}
void Archives::addArchive(QList<Archive*> listarchives)
{
    QList<Archive*>::const_iterator it;
    for( it = listarchives.constBegin(); it != listarchives.constEnd(); ++it )
        addArchive( *it );
}

