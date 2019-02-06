/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cls_motif.h"

Motif::Motif(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Motif::id() const               { return m_id; }
QString Motif::motif() const        { return m_motif; }
QString Motif::raccourci() const    { return m_raccourci; }
QString Motif::couleur() const      { return m_couleur; }
int Motif::duree() const            { return m_duree; }
bool Motif::pardefaut() const       { return m_pardefaut; }
bool Motif::utiliser() const        { return m_utiliser; }
int Motif::noordre() const          { return m_noordre; }

void Motif::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_id);
    setDataString(data, "motif", m_motif);
    setDataString(data, "raccourci", m_raccourci);
    setDataString(data, "couleur", m_couleur);
    setDataInt(data, "duree", m_duree);
    setDataBool(data, "pardefaut", m_pardefaut);
    setDataBool(data, "utiliser", m_utiliser);
    setDataInt(data, "noordre", m_noordre);
}


Motifs::Motifs()
{
    m_motifs = new QMap<int, Motif*>();
}

QMap<int, Motif *> *Motifs::motifs() const
{
    return m_motifs;
}

void Motifs::addMotif(Motif *Motif)
{
    if( m_motifs->contains(Motif->id()) )
        return;
    m_motifs->insert(Motif->id(), Motif);
}

void Motifs::addMotif(QList<Motif*> listMotifs)
{
    QList<Motif*>::const_iterator it;
    for( it = listMotifs.constBegin(); it != listMotifs.constEnd(); ++it )
        addMotif( *it );
}

Motif* Motifs::getMotifById(int id)
{
    QMap<int, Motif*>::const_iterator itcpt = m_motifs->find(id);
    if( itcpt == m_motifs->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void Motifs::clearAll()
{
    QList<Motif*> listmotifs;
    for( QMap<int, Motif*>::const_iterator itmtf = m_motifs->constBegin(); itmtf != m_motifs->constEnd(); ++itmtf)
        delete itmtf.value();
    m_motifs->clear();
}

void Motifs::removeMotif(Motif *motf)
{
    QMap<int, Motif*>::const_iterator itmotf = m_motifs->find(motf->id());
    if( itmotf == m_motifs->constEnd() )
        return;
    m_motifs->remove(motf->id());
    delete motf;
}

