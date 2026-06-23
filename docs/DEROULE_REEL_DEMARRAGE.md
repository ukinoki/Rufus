# Déroulé du démarrage de Rufus — fil de guidage du code

> **But de ce fichier.** Version **condensée et unifiée** du démarrage : le *fil de
> guidage* qui doit refléter ce que le code fait réellement, fonction par fonction.
> Le **schéma exhaustif** (toutes les itérations 1a/1b/1c/1d cas par cas) reste dans
> `initialisation Rufus.txt`, à la racine du dépôt. Ici, on **factorise** : depuis le
> remaniement, les modes monoposte / réseau local / accès distant partagent une seule
> fonction `Connexion_A_La_Base()`, et les différences se réduisent à quelques `if`.
>
> Quand le code change, **mettre à jour ce fichier**. Les références `fichier:ligne`
> sont indicatives (elles bougent avec le code).

---

## 0. Le piège à défaire d'abord : il y a DEUX mots de passe, pas un

La plus grosse source de confusion. `.dbkey` / `gaxt78iy` et « login/mot de passe »
ne sont **pas** le même niveau :

| Niveau | Qui se connecte | Mot de passe | Où c'est résolu |
|---|---|---|---|
| **MySQL** (technique) | `adminrufus` / `adminrufusSSL` | aléatoire du cabinet (`~/.rufus/.dbkey`) **+** `gaxt78iy` conservé en 2ᵉ | cascade `MySQLInstaller::connecterAvecCandidats()` / `motDePasseSQL()` |
| **Application** (métier) | le praticien | son login/mot de passe perso | `dlg_identificationuser` → table `rufus.utilisateurs` |

- La cascade **`.dbkey` (aléatoire) → `gaxt78iy`** ouvre la **connexion MySQL** de bas
  niveau (compte `adminrufus`). `gaxt78iy` (`MDP_SQL`) est le mot de passe générique de
  bootstrap, conservé ~30 j comme 2ᵉ mot de passe pour qu'un nouveau poste se connecte
  avant d'avoir récupéré l'aléatoire.
- Le **login/mot de passe demandé à l'écran** = l'**identification du praticien**
  (couche au-dessus), faite en TOUT DERNIER (`IdentificationUser`), une fois la base
  ouverte. **Aucune** demande de login praticien avant ce point.

---

## 1. Les 6 paramètres qui font varier le démarrage

C'est **chaque** démarrage (pas seulement le premier) qui les contrôle, dans un ordre
qui dépend du mode :

1. présence d'un `rufus.ini` valide (≥ 1 mode de connexion paramétré) ;
2. présence d'un `.dbkey` ;
3. présence d'un serveur ;
4. présence d'une base Rufus sur ce serveur ;
5. conformité de la version MySQL (≥ `VERSION_MYSQL_MINI` = 8.0.14, hors MariaDB) ;
6. présence de clés SSL valides (accès distant uniquement).

## 2. Les 4 fonctions qui gèrent tout

| Fonction | Rôle |
|---|---|
| `Procedures::RecupererDemarrage()` | **Le carrefour de récupération** : appelé chaque fois que « ça rate ». |
| `Procedures::PremierDemarrage()` | Derrière le bouton « Nouvelle base patients vierge » du carrefour. |
| `Procedures::Connexion_A_La_Base()` | **Le cœur** : connexion MySQL + contrôles, UNIFIÉ pour les 3 modes. |
| `Procedures::IdentificationUser()` | Identification du **praticien**, UNIQUEMENT (login métier). |

---

## 3. Séquence au lancement (vue d'ensemble)

```
main()  (main.cpp:75)
 ├─ splash
 ├─ Rufus w;                       → constructeur Rufus
 │   ├─ Procedures::I()            → constructeur Procedures :
 │   │     • langue de l'interface (rufus.ini Param_Poste_Version)
 │   │     • rufus.ini ABSENT ou INVALIDE → boucle RecupererDemarrage()   (cf. §6)
 │   ├─ ListeModesAcces().size()   (rufus.cpp:43)
 │   │     0   → « rufus.ini endommagé », exit (cas théorique, déjà géré en amont)
 │   │     1   → Connexion_A_La_Base()                         (cf. §4)
 │   │     ≥2  → FicheChoixConnexion() puis Connexion_A_La_Base()
 │   ├─ (connexion OK) SauvegardeIni(), init widgets/menus/tables…
 │   └─ w.show()                   → la fenêtre principale s'affiche
 └─ Procedures::I()->ControleSocleMySQLApresAffichage()  (main.cpp:138)   (cf. §7)
     → mise à jour DIFFÉRÉE du socle MySQL, APRÈS l'affichage
```

Point clé : tout sauf la migration du socle se joue **avant** `w.show()`, dans
`Connexion_A_La_Base()`. La migration, longue, est volontairement **différée à après
l'affichage** (Rufus tourne en mode dégradé en attendant).

---

## 4. `Connexion_A_La_Base()` — le cœur unifié — `procedures.cpp:3083`

Une seule fonction pour les 3 modes. L'ordre des contrôles, et les quelques branches
spécifiques à un mode, sont signalés ci-dessous.

1. **`initParametresConnexionSQL(server, port)`** — serveur = `localhost` (monoposte)
   ou l'IP du `rufus.ini` (réseau/distant).

2. **[DISTANT] Pré-contrôle des clés SSL.** Le compte `adminrufusSSL` exige `REQUIRE
   SSL`. Clés absentes (`client-key.pem` + `client-cert.pem`) → on **demande leur
   dossier** (à copier du serveur sur une clé USB), on recontrôle ; toujours rien →
   message + **carrefour** `RecupererDemarrage()`. Inutile de tenter une connexion qui
   échouerait.

3. **[MONOPOSTE] Pré-contrôle de la présence du serveur** (`serveurLocalPresent()`).
   Pas de serveur installé → **carrefour**, mais **boutons « Rufus.ini » masqués**
   (`RecupIni=false, ReconstruitIni=false`) : le `rufus.ini` est correct, c'est le
   SERVEUR qui manque. Restent « Nouvelle base patients vierge » (installe le socle) et
   « Quitter ». Inutile de tenter une connexion.

4. **Connexion MySQL en cascade** (`connecterAvecCandidats(DB_RUFUS)` : `.dbkey` PUIS
   `gaxt78iy`).
   - **Échec d'AUTHENTIFICATION** (base sécurisée ailleurs / `.dbkey` périmé) → on
     propose DIRECTEMENT de récupérer le bon mot de passe (`RecupererMotDePasseMySQL` :
     saisie ou clé USB), puis on réessaie. Invite SIMPLE — on ne balade pas l'utilisateur
     dans le carrefour pour juste un mot de passe.
   - **Toujours en échec** (mdp non récupéré, ou échec NON-auth : serveur injoignable,
     mauvais port…) → **carrefour** `RecupererDemarrage()` AVEC le bouton de
     **connexion/création d'une base** (cf. §6). Sans ce bouton, un `rufus.ini` correct
     pointant vers un serveur/base disparu menait à une IMPASSE (on ne pouvait que
     reconstruire/restaurer `rufus.ini`). Boutons : connexion + restaurer `rufus.ini`
     (si sauvegarde) + quitter.
   - **[DISTANT] clés SSL absentes** (pré-contrôle §4.2 resté infructueux) → on NE passe PAS
     par le carrefour : un seul mode paramétré → `VerifParamConnexion()` (inclut le dossier
     des clés SSL) puis relance ; plusieurs modes → on reboucle sur `FicheChoixConnexion()`.

5. **Cohérence de la CONFIGURATION MySQL** (post-connexion). Règle de fond : la config du
   serveur ne peut être **corrigée** que là où on a la main dessus → **uniquement en
   monoposte**. Un client peut détecter, mais défère (comme pour la base endommagée et la
   version).
   - **[MONOPOSTE]** vérification SILENCIEUSE et bon marché des items de config (PATH,
     dossier partagé, `secure_file_priv`, lecture/écriture, privilèges adminrufus, clés SSL
     serveur) ; si **tout va bien → rien** (cas courant). Si un item est cassé → on RÉPARE
     (réutilise `MySQLInstaller::executerEtapesConfig()` avec `m_freshInstall=false` : il
     rejoue les `ensure*`/`setup*` SANS réinstaller ni recréer d'utilisateur).
   - **[RÉSEAU LOCAL]** on vérifie seulement l'accès au **dossier partagé d'imagerie** (son
     propre montage réseau) → si inaccessible, message (montage manuel). La config SERVEUR
     n'est ni vérifiée ni corrigée ici (le client ne peut pas la corriger).
   - **[DISTANT]** rien : un poste distant ne corrige aucune config serveur, et ses clés SSL
     ont déjà été pré-contrôlées avant la connexion (§4.2).
   - NB : `dlg_identificationuser` contrôle DÉJÀ (plus tard, à l'identification)
     `secure_file_priv` et `sql_mode` et alerte en cas d'anomalie — sans corriger.

6. **Cohérence de la base** (`SELECT 1 FROM utilisateurs`). Base présente mais
   altérée → messageBox **« Base de données endommagée »** :
   - **monoposte (hôte)** : carrefour avec bouton « Restaurer la base » → `RestaureBase()` ;
   - **client (réseau/distant)** : message seul (« la restauration ne peut se faire qu'à
     partir du poste serveur ») — un client ne répare jamais la base partagée.

7. **Version du socle MySQL.** Si < 8.0.14, on **ne fait RIEN ici** : on MÉMORISE le
   besoin (`m_socleMySQLAMettreAJour = true`). Message et migration sont **différés** à
   après l'affichage (cf. §7).

8. **Entretien du mot de passe** (`MySQLInstaller::entretienApresConnexion()`) — cf. §5.

9. **`IdentificationUser()`** — login praticien (`dlg_identificationuser`), UNIQUEMENT.

10. Lieu d'exercice, création de session, `max_allowed_packet`, etc.

---

## 5. Entretien du mot de passe — `entretienApresConnexion()` — `mysqlinstaller.cpp:2106`

Branches **auto-gardées** (chacune teste ses conditions) et **mutuellement exclusives**
selon le mot de passe avec lequel CE poste s'est connecté (`motDePasseSQL()`) :

```
securiserBaseSiNecessaire()       générique + base NON sécurisée + LOCAL + ≥8.0.14
                                   → pose un aléatoire (gaxt78iy conservé en 2ᵉ), écrit .dbkey
                                   → renvoie true si elle vient de sécuriser (évite un message en double)

connecté avec l'ALÉATOIRE, gaxt78iy encore présent :
   supprimerGaxt78iySiEchue()      deadline (sécurisation + 30 j) PASSÉE
                                   → retire gaxt78iy (après confirmation) + message
   avertirEffacementImminent()     deadline NON atteinte (et pas juste sécurisé)
                                   → simple avertissement informatif

connecté avec le GÉNÉRIQUE, base SÉCURISÉE (un aléatoire existe) :
   proposerRecuperationAleatoire() TOUS modes
                                   → message d'échéance + bouton « Renseigner le nouveau
                                     mot de passe » (réutilise RecupererMotDePasseMySQL :
                                     saisie / clé USB → .dbkey)
```

Règle : on ne **sécurise** (poser l'aléatoire) JAMAIS depuis un poste en accès distant
(sinon, via la deadline, il pourrait verrouiller les postes locaux). La **récupération**
d'un aléatoire existant, elle, est possible depuis n'importe quel poste.

---

## 6. Le carrefour — `RecupererDemarrage()` — `procedures.cpp:4750`

Appelé chaque fois que « ça rate ». Jusqu'à 3 boutons (+ « Quitter »), selon le contexte
passé par l'appelant (qui seul connaît le rôle du poste et l'état de la connexion) :

| Bouton | Paramètre | Action |
|---|---|---|
| **Connexion/Création d'une base patients** *(monoposte / indéterminé)* | `PremDemarrage` | → `PremierDemarrage()` |
| **Connexion à une base patients** *(réseau local / distant)* | `PremDemarrage` | → `VerifParamConnexion()` puis relance |
| **Reconstruire le fichier Rufus.ini** | `ReconstruitIni` | → `VerifParamConnexion()` |
| **Restaurer la base** *(poste hôte)* | `RestaurerBase` | → `RestaureBase()` |
| **Restaurer le fichier Rufus.ini** *(sinon)* | `RecupIni` | → recopie `~/.rufus/.rufus.ini` (seulement si la sauvegarde existe) |
| **Abandonner et quitter** | — | `exit(0)` |

Le **même** bouton `PremDemarrage` s'adapte au mode (`db->ModeAccesDataBase()`) : il CRÉE ou
connecte en monoposte (via `PremierDemarrage`), il connecte seulement en client réseau (via
`VerifParamConnexion`). Pas de paramètre supplémentaire : le mode est déjà connu.

Toute récupération réussie **relance** Rufus (`startDetached` + `exit(0)`).
`PremierDemarrage()` (base vierge / base existante) vit derrière le 1ᵉʳ bouton ; c'est là
— et là seulement — qu'on inspecte l'état réel du serveur (`MySQLInstaller::run()` :
pas de serveur → installe ; serveur sans base → configure ; serveur + base → conserver /
supprimer).

---

## 7. Migration du socle DIFFÉRÉE — `ControleSocleMySQLApresAffichage()` — `procedures.cpp:3301`

Appelée depuis `main.cpp:138`, **après `w.show()`**. Si `m_socleMySQLAMettreAJour` :

1. `qApp->processEvents()` force la **peinture** de la fenêtre principale (même mécanisme
   que `ShowMessage`) — sinon le dialogue modal surgirait sur un fond vide ;
2. **monoposte (hôte)** → `MettreAJourSocleMySQL()` ;
   **client** → message « mise à jour à faire depuis le poste serveur ».

`MettreAJourSocleMySQL()` (`procedures.cpp:2763`) propose **2 chemins** (schéma à 3
boutons de `VerifVersionBase`) :

- **Sauvegarder les données et mettre à jour** : sauvegarde + validation par Rufus, puis
  désinstall → téléchargement MySQL ≥ 8.0.14 → paramétrage → restauration **automatique**
  depuis cette sauvegarde → relance ;
- **Poursuivre, la sauvegarde a été faite** : on saute la sauvegarde ; à la restauration,
  Rufus **demande le dossier** de la sauvegarde de l'utilisateur (`RestaureBase` avec
  chemin vide) ;
- **Annuler** : on ne touche à rien (Rufus continue en mode dégradé sur l'ancien socle).

---

## 8. Fonctions clés (index pour naviguer)

| Rôle | Fonction | Emplacement |
|---|---|---|
| Orchestration init | `Procedures::Procedures()` | `procedures.cpp:30` |
| Carrefour de récupération | `Procedures::RecupererDemarrage()` | `procedures.cpp:4750` |
| Premier démarrage (vierge/existante) | `Procedures::PremierDemarrage()` | `procedures.cpp:4509` |
| Détection serveur + base (3 cas) | `MySQLInstaller::run()` | `mysqlinstaller.cpp` |
| **Cœur** : connexion + contrôles unifiés | `Procedures::Connexion_A_La_Base()` | `procedures.cpp:3083` |
| Migration socle différée | `Procedures::ControleSocleMySQLApresAffichage()` | `procedures.cpp:3301` |
| Migration du socle (2 chemins) | `Procedures::MettreAJourSocleMySQL()` | `procedures.cpp:2763` |
| Identification praticien | `Procedures::IdentificationUser()` | `procedures.cpp:3741` |
| Pré-contrôle présence serveur (monoposte) | `MySQLInstaller::serveurLocalPresent()` | `mysqlinstaller.cpp:1939` |
| Connexion MySQL en cascade | `MySQLInstaller::connecterAvecCandidats()` | `mysqlinstaller.cpp:561` |
| Entretien du mot de passe | `MySQLInstaller::entretienApresConnexion()` | `mysqlinstaller.cpp:2106` |
| Récupération mdp (saisie / clé USB) | `dlg_paramconnexion::RecupererMotDePasseMySQL()` | `Dialogs/dlg_paramconnexion.cpp` |
