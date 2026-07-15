/* (C) 2026 LAINE SERGE
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

#ifndef LECTEURVITALE_H
#define LECTEURVITALE_H

#include <QObject>
#include <QString>
#include <QList>

//-----------------------------------------------------------------------------------------------------
// Lecture DIRECTE de la carte Vitale, via PC/SC, SANS intermédiaire (ni Pyxvital, ni CPS).
//
// La carte Vitale 2 est une carte ISO 7816 : on ouvre la racine (MF_VITALE) pour l'identité
// (nom / prénom / date de naissance), puis l'application « VITALE » pour le NIR. Ces zones sont
// lisibles en clair, sans authentification. Les données restent dans le cabinet : rien ne sort.
//
// La couche PC/SC est native : winscard (Windows), framework PCSC (macOS), pcsc-lite (Linux).
//-----------------------------------------------------------------------------------------------------
class LecteurVitale : public QObject
{
    Q_OBJECT

public:
    explicit LecteurVitale(QObject *parent = nullptr);

    // Un porteur = l'assuré ou un ayant droit présent sur la carte.
    struct Porteur {
        QString nom;
        QString prenom;
        QString dateNaissance;   //!< format JJ/MM/AAAA
        QString nir;             //!< 15 caractères (13 + clé), brut ; vide si absent
    };

    //! Lit la carte Vitale présente dans un lecteur PC/SC.
    //! Renvoie true si au moins l'identité a pu être lue ; sinon err décrit l'échec.
    bool lire(QString &err);

    QList<Porteur> porteurs() const { return m_porteurs; }

    //! Met en forme un NIR brut « 268081315501324 » en « 2 68 08 13 155 013 24 » (lisibilité).
    static QString nirLisible(const QString &nir);

private:
    QList<Porteur> m_porteurs;
};

#endif // LECTEURVITALE_H
