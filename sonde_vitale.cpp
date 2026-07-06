// sonde_vitale.cpp — sonde de diagnostic PC/SC pour la carte Vitale
// -----------------------------------------------------------------------------
// But : cartographier ce que la carte accepte de renvoyer SANS Pyxvital, pour
// décider ce qui est décodable (identité, NIR, ayants droit).
//
// Ce n'est PAS du code destiné à Rufus : c'est un outil jetable qui sert à
// observer la carte de test. On envoie quelques APDU candidates et on dumpe la
// réponse en hexa + ASCII : si la zone administrative est ouverte, on doit voir
// apparaître le nom en clair dans la colonne ASCII.
//
// Compilation :
//   Linux   : g++ sonde_vitale.cpp -o sonde_vitale -lpcsclite
//             (paquets : pcscd libpcsclite-dev  ; service : sudo systemctl start pcscd)
//   macOS   : g++ sonde_vitale.cpp -o sonde_vitale -framework PCSC
//   Windows : cl sonde_vitale.cpp winscard.lib      (ou g++ ... -lwinscard sous MinGW)
//
// Usage : brancher le lecteur + la Vitale de test, puis ./sonde_vitale
// -----------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

#ifdef _WIN32
  #include <winscard.h>
  // Windows expose les variantes ...A (ANSI) ; on force le char* pour rester
  // homogène avec pcsc-lite qui, lui, n'a pas de suffixe.
  #define SCListReaders SCardListReadersA
  #define SCConnect     SCardConnectA
  #define SCStatus      SCardStatusA
#else
  #include <PCSC/winscard.h>
  #include <PCSC/wintypes.h>
  #define SCListReaders SCardListReaders
  #define SCConnect     SCardConnect
  #define SCStatus      SCardStatus
#endif

// ---- petit utilitaire : dump hexa + ASCII ----------------------------------
static void dump(const char* titre, const BYTE* buf, DWORD len)
{
    printf("  %s (%lu octets) :\n", titre, (unsigned long)len);
    for (DWORD i = 0; i < len; i += 16) {
        printf("    %04lx  ", (unsigned long)i);
        for (DWORD j = 0; j < 16; ++j) {
            if (i + j < len) printf("%02X ", buf[i + j]);
            else             printf("   ");
        }
        printf(" |");
        for (DWORD j = 0; j < 16 && i + j < len; ++j) {
            BYTE c = buf[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");
    }
}

// ---- envoi d'une APDU + affichage ------------------------------------------
static void envoie(SCARDHANDLE card, const SCARD_IO_REQUEST* pci,
                   const char* libelle, const std::vector<BYTE>& apdu)
{
    BYTE  reponse[512];
    DWORD lg = sizeof(reponse);
    printf("\n> %s\n  APDU :", libelle);
    for (BYTE b : apdu) printf(" %02X", b);
    printf("\n");

    LONG rv = SCardTransmit(card, pci, apdu.data(), (DWORD)apdu.size(),
                            NULL, reponse, &lg);
    if (rv != SCARD_S_SUCCESS) {
        printf("  ERREUR SCardTransmit : 0x%08lX\n", (unsigned long)rv);
        return;
    }
    if (lg >= 2) {
        BYTE sw1 = reponse[lg - 2], sw2 = reponse[lg - 1];
        printf("  Statut SW : %02X %02X  %s\n", sw1, sw2,
               (sw1 == 0x90 && sw2 == 0x00) ? "(OK)" :
               (sw1 == 0x6D)                ? "(INS non supportée)" :
               (sw1 == 0x6E)                ? "(CLA non supportée)" :
               (sw1 == 0x69)                ? "(condition non satisfaite / accès refusé)" :
               (sw1 == 0x6A)                ? "(fichier/param introuvable)" : "");
    }
    if (lg > 2) dump("Données", reponse, lg - 2);
}

int main()
{
    SCARDCONTEXT ctx;
    LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &ctx);
    if (rv != SCARD_S_SUCCESS) {
        printf("SCardEstablishContext a échoué : 0x%08lX\n", (unsigned long)rv);
        printf("(Sous Linux : le service pcscd tourne-t-il ? sudo systemctl start pcscd)\n");
        return 1;
    }

    // ---- liste des lecteurs -------------------------------------------------
    DWORD lgReaders = 0;
    rv = SCListReaders(ctx, NULL, NULL, &lgReaders);
    if (rv != SCARD_S_SUCCESS || lgReaders == 0) {
        printf("Aucun lecteur détecté (0x%08lX).\n", (unsigned long)rv);
        SCardReleaseContext(ctx);
        return 1;
    }
    std::vector<char> readers(lgReaders);
    SCListReaders(ctx, NULL, readers.data(), &lgReaders);

    printf("Lecteurs détectés :\n");
    const char* p = readers.data();
    std::string premier;
    while (*p) {
        printf("  - %s\n", p);
        if (premier.empty()) premier = p;
        p += strlen(p) + 1;
    }
    printf("\n=> Utilisation du lecteur : %s\n", premier.c_str());

    // ---- connexion à la carte ----------------------------------------------
    SCARDHANDLE card;
    DWORD proto = 0;
    rv = SCConnect(ctx, premier.c_str(), SCARD_SHARE_SHARED,
                   SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &card, &proto);
    if (rv != SCARD_S_SUCCESS) {
        printf("SCardConnect a échoué : 0x%08lX  (carte insérée ?)\n", (unsigned long)rv);
        SCardReleaseContext(ctx);
        return 1;
    }
    const SCARD_IO_REQUEST* pci =
        (proto == SCARD_PROTOCOL_T1) ? SCARD_PCI_T1 : SCARD_PCI_T0;

    // ---- ATR ----------------------------------------------------------------
    {
        BYTE atr[64]; DWORD lgAtr = sizeof(atr), state = 0, aproto = 0;
        char nom[256]; DWORD lgNom = sizeof(nom);
        if (SCStatus(card, nom, &lgNom, &state, &aproto, atr, &lgAtr) == SCARD_S_SUCCESS)
            dump("ATR de la carte", atr, lgAtr);
    }

    // ---- APDU candidates ----------------------------------------------------
    // On tâtonne : la Vitale historique répond à BC B0 (lecture directe par
    // adresse) ; on tente aussi le SELECT ISO du fichier maître. Les statuts SW
    // ci-dessus disent lesquelles la carte accepte.
    envoie(card, pci, "SELECT MF (fichier maître ISO 7816)",
           {0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00});

    envoie(card, pci, "Vitale : READ direct BC B0 @0000, 256 octets",
           {0xBC, 0xB0, 0x00, 0x00, 0x00});

    envoie(card, pci, "Vitale : READ direct BC B0 @0100, 256 octets",
           {0xBC, 0xB0, 0x01, 0x00, 0x00});

    envoie(card, pci, "Vitale : READ direct BC B0 @0200, 256 octets",
           {0xBC, 0xB0, 0x02, 0x00, 0x00});

    // ---- fin ----------------------------------------------------------------
    SCardDisconnect(card, SCARD_LEAVE_CARD);
    SCardReleaseContext(ctx);
    printf("\nTerminé. Copie-colle toute cette sortie pour analyse.\n");
    return 0;
}
