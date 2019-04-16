#include "cls_motifs.h"

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

void Motifs::removeMotif(Motif *motif)
{
    if (motif == Q_NULLPTR)
        return;
    QMap<int, Motif*>::const_iterator itmotf = m_motifs->find(motif->id());
    if( itmotf == m_motifs->constEnd() )
        return;
    m_motifs->remove(motif->id());
    delete motif;
}

/*!
 * \brief Motifs::initListeMotifs
 * Charge l'ensemble des motifs
 * et les ajoute à la classe Motifss
 */
void Motifs::initListe()
{
    clearAll();
    QList<Motif*> listmotifs = DataBase::getInstance()->loadMotifs();
    QList<Motif*>::const_iterator itmtf;
    for( itmtf = listmotifs.constBegin(); itmtf != listmotifs.constEnd(); ++itmtf )
    {
        Motif *mtf = const_cast<Motif*>(*itmtf);
        addMotif(mtf);
    }
}

