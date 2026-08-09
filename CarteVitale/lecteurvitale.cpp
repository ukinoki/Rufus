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

#include "lecteurvitale.h"

#include <QByteArray>
#include <QStringList>
#include <vector>

// En-têtes PC/SC selon la plateforme. L'API (SCardEstablishContext, SCardTransmit…) est la même
// partout ; seuls l'en-tête et la bibliothèque à lier diffèrent (cf. RufusQt6.pro).
#ifdef Q_OS_WIN
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX          // évite que windows.h ne définisse les macros min/max (conflit Qt)
  #endif
  #include <windows.h>
  #include <winscard.h>
  #define SC_ListReaders SCardListReadersA   // variantes ANSI : on manipule des char*
  #define SC_Connect     SCardConnectA
#else
  #include <PCSC/wintypes.h>
  #include <PCSC/winscard.h>
  #define SC_ListReaders SCardListReaders
  #define SC_Connect     SCardConnect
#endif

// ============================================================================================
//  Couche APDU : dialogue bas niveau avec la carte
// ============================================================================================

namespace {

struct Reponse {
    bool               ok = false;      //!< la transmission a abouti (≠ succès applicatif)
    QByteArray         data;            //!< données hors mot d'état
    unsigned char      sw1 = 0, sw2 = 0;
};

// Envoie une APDU en gérant le protocole T=0 :
//  - 61 XX : la carte a XX octets en attente  -> GET RESPONSE
//  - 6C XX : longueur attendue différente      -> on rejoue avec XX
// Le lecteur intégré au clavier « hoquette » parfois : on réessaie jusqu'à 4 fois.
Reponse transmet(SCARDHANDLE card, const SCARD_IO_REQUEST *pci, QByteArray apdu)
{
    Reponse r;
    for (int essai = 0; essai < 4; ++essai)
    {
        BYTE  buf[512];
        DWORD lg = sizeof(buf);
        LONG rv = SCardTransmit(card, pci,
                                reinterpret_cast<const BYTE *>(apdu.constData()), (DWORD)apdu.size(),
                                nullptr, buf, &lg);
        if (rv != SCARD_S_SUCCESS)
            continue;                                   // hoquet : on retente
        r.ok = true;
        if (lg < 2)
            return r;
        unsigned char s1 = buf[lg - 2], s2 = buf[lg - 1];
        if (s1 == 0x6C && !apdu.isEmpty()) {            // mauvaise longueur : on rejoue avec la bonne
            apdu[apdu.size() - 1] = (char)s2;
            continue;
        }
        if (s1 == 0x61) {                               // données en attente : GET RESPONSE
            QByteArray gr;
            gr.append((char)0x00).append((char)0xC0).append((char)0x00).append((char)0x00).append((char)s2);
            return transmet(card, pci, gr);
        }
        r.data = QByteArray(reinterpret_cast<const char *>(buf), (int)lg - 2);
        r.sw1 = s1; r.sw2 = s2;
        return r;
    }
    return r;                                           // r.ok == false
}

inline bool estOK(const Reponse &r) { return r.ok && r.sw1 == 0x90 && r.sw2 == 0x00; }

// ============================================================================================
//  Couche TLV : décodage des données de la carte
//  Les données sont des objets TLV imbriqués (tag 1 octet, longueur 1 octet ici). Chaque porteur
//  est un modèle de tag 0x65 contenant : 0x80 nom, 0x81 prénom, 0x82 naissance (AAAAMMJJ),
//  0x90 pays, 0x93 NIR (15 caractères ASCII).
// ============================================================================================

// Cherche, dans une valeur TLV, le premier sous-tag == tag. Renvoie sa valeur.
bool sousTag(const unsigned char *b, int len, unsigned char tag, QByteArray &valeur)
{
    int i = 0;
    while (i + 2 <= len) {
        unsigned char t = b[i];
        int hdr = ((t & 0x1F) == 0x1F) ? 2 : 1;         // tag sur 2 octets si les 5 bits bas == 1
        if (i + hdr >= len) break;
        int l = b[i + hdr];
        int v = i + hdr + 1;
        if (v + l > len) break;
        if (t == tag) { valeur = QByteArray(reinterpret_cast<const char *>(b + v), l); return true; }
        i = v + l;
        if (t == 0) break;                              // bourrage de zéros en fin de fichier
    }
    return false;
}

// AAAAMMJJ -> JJ/MM/AAAA (renvoie la chaîne brute si le format n'est pas celui attendu).
QString formateDate(const QByteArray &aaaammjj)
{
    if (aaaammjj.size() != 8)
        return QString::fromLatin1(aaaammjj);
    const QString s = QString::fromLatin1(aaaammjj);
    return s.mid(6, 2) + "/" + s.mid(4, 2) + "/" + s.left(4);
}

// Parcourt un fichier et appelle, pour chaque modèle de porteur (tag 0x65), le foncteur reçu
// avec la valeur du modèle.
template <typename F>
void pourChaquePorteur(const QByteArray &fichier, F traite)
{
    const unsigned char *b = reinterpret_cast<const unsigned char *>(fichier.constData());
    int len = fichier.size(), i = 0;
    while (i + 2 <= len) {
        unsigned char t = b[i];
        int hdr = ((t & 0x1F) == 0x1F) ? 2 : 1;
        if (i + hdr >= len) break;
        int l = b[i + hdr];
        int v = i + hdr + 1;
        if (v + l > len) break;
        if (t == 0x65)
            traite(b + v, l);
        i = v + l;
        if (t == 0) break;
    }
}

} // namespace

// ============================================================================================
//  LecteurVitale
// ============================================================================================

LecteurVitale::LecteurVitale(QObject *parent) : QObject(parent)
{
}

QString LecteurVitale::nirLisible(const QString &nir)
{
    if (nir.size() != 15)
        return nir;
    // sexe(1) an(2) mois(2) dépt(2) commune(3) ordre(3) clé(2)
    return nir.mid(0, 1) + " " + nir.mid(1, 2) + " " + nir.mid(3, 2) + " " + nir.mid(5, 2)
         + " " + nir.mid(7, 3) + " " + nir.mid(10, 3) + " " + nir.mid(13, 2);
}

bool LecteurVitale::lire(QString &err)
{
    m_porteurs.clear();

    SCARDCONTEXT ctx;
    if (SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &ctx) != SCARD_S_SUCCESS) {
        err = tr("Service de lecture de carte indisponible.");
        return false;
    }

    // Liste des lecteurs (chaîne multi-terminée par des \0).
    DWORD lgReaders = 0;
    if (SC_ListReaders(ctx, nullptr, nullptr, &lgReaders) != SCARD_S_SUCCESS || lgReaders == 0) {
        SCardReleaseContext(ctx);
        err = tr("Aucun lecteur de carte détecté.");
        return false;
    }
    std::vector<char> readers(lgReaders);
    SC_ListReaders(ctx, nullptr, readers.data(), &lgReaders);
    QList<QByteArray> noms;
    for (const char *p = readers.data(); *p; p += qstrlen(p) + 1)
        noms.append(QByteArray(p));

    // On essaie chaque fente : la Vitale peut être dans l'une ou l'autre (lecteur bi-fente), et
    // une CPS éventuelle dans l'autre est simplement ignorée (elle n'a pas la racine MF_VITALE).
    QStringList diag;                                   // raisons techniques d'échec, ajoutées au message si rien n'est lu
    for (const QByteArray &nom : noms)
    {
        SCARDHANDLE card;
        DWORD proto = 0;
        LONG rc = SC_Connect(ctx, nom.constData(), SCARD_SHARE_SHARED,
                             SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &card, &proto);
        if (rc != SCARD_S_SUCCESS) {                    // pas de carte dans cette fente, ou lecteur occupé
            diag << QString("connexion « %1 » : 0x%2").arg(QString::fromLatin1(nom)).arg((quint32)rc, 8, 16, QChar('0'));
            continue;
        }

        const SCARD_IO_REQUEST *pci = (proto == SCARD_PROTOCOL_T1) ? SCARD_PCI_T1 : SCARD_PCI_T0;
        SCardBeginTransaction(card);

        // Racine : si le SELECT MF échoue, ce n'est pas une Vitale (p.ex. une CPS) -> fente suivante.
        Reponse mf = transmet(card, pci, QByteArray::fromHex("00A4000002" "3F00"));
        if (estOK(mf))
        {
            // 1) Identité (nom / prénom / naissance) : racine, fichier court n°2.
            Reponse id = transmet(card, pci, QByteArray::fromHex("00B0820000"));
            if (id.ok && !id.data.isEmpty())
                pourChaquePorteur(id.data, [&](const unsigned char *v, int l) {
                    Porteur pt;
                    QByteArray val;
                    if (sousTag(v, l, 0x80, val)) pt.nom           = QString::fromLatin1(val).trimmed();
                    if (sousTag(v, l, 0x81, val)) pt.prenom        = QString::fromLatin1(val).trimmed();
                    if (sousTag(v, l, 0x82, val)) pt.dateNaissance = formateDate(val);
                    m_porteurs.append(pt);
                });
            else
                diag << QString("identité « %1 » : %2").arg(QString::fromLatin1(nom),
                            id.ok ? QString("SW %1%2 (%3 octets)").arg(id.sw1, 2, 16, QChar('0')).arg(id.sw2, 2, 16, QChar('0')).arg(id.data.size())
                                  : QString("transmission KO"));

            // 2) NIR : application « VITALE » (AID D2 50 00 00 02 56 49 54 41 4C 45), fichier court n°2.
            transmet(card, pci, QByteArray::fromHex("00A404000BD250000002564954414C45"));
            Reponse nir = transmet(card, pci, QByteArray::fromHex("00B0820000"));
            if (nir.ok && !nir.data.isEmpty()) {
                int i = 0;
                pourChaquePorteur(nir.data, [&](const unsigned char *v, int l) {
                    QByteArray val;
                    if (sousTag(v, l, 0x93, val) && i < m_porteurs.size())
                        m_porteurs[i].nir = QString::fromLatin1(val).trimmed();
                    ++i;
                });
            }
        }
        else
            diag << QString("SELECT MF « %1 » : %2").arg(QString::fromLatin1(nom),
                        mf.ok ? QString("SW %1%2").arg(mf.sw1, 2, 16, QChar('0')).arg(mf.sw2, 2, 16, QChar('0'))
                              : QString("transmission KO"));

        SCardEndTransaction(card, SCARD_LEAVE_CARD);
        SCardDisconnect(card, SCARD_RESET_CARD);        // remet la carte à zéro pour la lecture suivante

        if (!m_porteurs.isEmpty())
            break;                                       // on a trouvé la Vitale
    }

    SCardReleaseContext(ctx);

    if (m_porteurs.isEmpty()) {
        err = tr("Aucune carte Vitale lisible dans le lecteur.");
        if (!diag.isEmpty())                            // détail technique : indispensable pour diagnostiquer un échec rare
            err += " [" + diag.join(" ; ") + "]";
        return false;
    }
    return true;
}
