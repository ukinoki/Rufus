# Rufus — Initialisation & sécurité des accès

> Document de référence (durable) sur **le démarrage de Rufus**, **la création
> d'une base patients**, et **le modèle de sécurité des accès à MySQL**.
> Public : développeurs reprenant Rufus. Branche de référence : **`RufusQt6`**
> (la branche `master` est obsolète et n'a plus rien de commun avec Rufus).

---

## 1. Démarrage & détection du premier lancement

Au lancement, Rufus cherche le fichier **`~/Documents/Rufus/rufus.ini`**
(`PATH_FILE_INI`). Ce fichier contient le paramétrage **du poste** : mode d'accès
au serveur, ports, chemins, imprimantes, etc.

- **`rufus.ini` absent** → Rufus en conclut qu'il s'agit du **premier démarrage**
  et lance `Procedures::PremierDemarrage()`.
- **`rufus.ini` présent** → Rufus lit le paramétrage et se connecte.

### `Procedures::PremierDemarrage()`
Propose trois choix :
1. **Retour au menu d'accueil** (annule).
2. **Base patients existante sur le serveur** → `VerifParamConnexion(...)` puis
   connexion ; le poste rejoint une base déjà créée.
3. **Nouvelle base patients vierge** → création d'une base neuve sur le serveur
   MySQL du poste (voir §2 et §3).

---

## 2. L'installeur MySQL intégré (module `MySQLInstaller`)

Historiquement, Rufus **supposait** qu'un serveur MySQL était déjà installé et
configuré (il se contentait de localiser les binaires via
`Procedures::setDirSQLExecutable()`). Désormais, au clic **« Nouvelle base
patients vierge »**, Rufus lance le module natif **`MySQLInstaller`**
(`MySQLInstaller/mysqlinstaller.{h,cpp}`), porté de l'utilitaire autonome
*ukinoki/mysqlinstaller-for-rufus*.

**Rôle** : garantir, de façon transparente pour un utilisateur non informaticien
(« Maurice »), qu'un serveur MySQL **conforme aux exigences de Rufus** est prêt.

**Contrat** : `bool MySQLInstaller::run()`
- serveur présent **et conforme** → `true` (Rufus continue) ;
- serveur absent, l'utilisateur **accepte** l'installation → install + config → `true` ;
- **annulation / échec** → `false` ; Rufus **reste sur `PremierDemarrage()`**.

**Ce que l'installeur vérifie/configure** (checklist en 6 points, cases `up*`
cochées en direct) :
0. MySQL (≥ version minimale) installé — sinon proposition d'installation ;
1. chemin de `mysql` dans le `PATH` ;
2. dossier partagé créé et partagé (échange d'images avec les appareils) ;
3. `secure_file_priv` pointant sur ce dossier ;
4. lecture/écriture MySQL vérifiée (le serveur écrit puis relit un fichier test) ;
5. droits de l'utilisateur confirmés (`ALL PRIVILEGES` + `GRANT OPTION`).

Spécificités par OS (compilation conditionnelle) : Ubuntu (`apt`, `pkexec`,
`ufw`, AppArmor, Samba + wsdd), Windows (ZIP + service, UAC), macOS (DMG Oracle,
`osascript`). Cible principale : **Ubuntu ≥ 22.04** (compatibilité de machines
anciennes, > 10 ans).

L'installeur **ne demande aucun identifiant MySQL** : les comptes sont fixes et
le mot de passe est géré automatiquement (voir §4).

---

## 3. Modèle des comptes & des utilisateurs

Il faut distinguer **deux notions** :

### a) Comptes MySQL (niveau serveur) — il n'y en a que deux
- **`adminrufus`** (`LOGIN_SQL`) : connexions locales / poste / réseau local.
- **`adminrufusSSL`** : connexions **distantes**, avec `REQUIRE SSL` (cryptage
  obligatoire ; le poste doit posséder les clés SSL clientes).

Les deux ont `GRANT ALL PRIVILEGES ON *.* ... WITH GRANT OPTION` et **le même mot
de passe**. Ils sont créés à la création de la base (par l'installeur, et/ou
`Procedures::RestaureBase()`).

### b) Utilisateurs Rufus (niveau application) — dans la base
Les praticiens **ne sont pas** des comptes MySQL. Ils vivent dans la table
**`rufus.utilisateurs`** (login + mot de passe en **SHA1**) et c'est **Rufus** qui
valide leur connexion (`dlg_identificationuser`), *après* s'être connecté à MySQL
via `adminrufus`. Le mot de passe administrateur applicatif (`MDP_ADMINISTRATEUR`,
« bob » par défaut) ouvre le paramétrage (sauvegardes, topologie, imprimantes…).

> Séquence de connexion : Rufus se connecte d'abord à MySQL avec `adminrufus`,
> **puis** vérifie l'utilisateur saisi dans `rufus.utilisateurs`.

---

## 4. Mot de passe MySQL : aléatoire par cabinet + repli legacy

### Le problème
`MDP_SQL` (`"gaxt78iy"`) était **codé en dur, identique pour toutes les
installations**, donc **public** (code open source sur GitHub). N'importe qui
pouvait se connecter à n'importe quelle base Rufus joignable.

### La solution
- À la **création d'une base**, on génère un **mot de passe aléatoire** (24 car.
  alphanumériques, `genererMotDePasse()`), on crée `adminrufus`/`adminrufusSSL`
  avec, et on le **stocke dans un fichier CACHÉ** `~/.rufus/.dbkey`
  (`PATH_FILE_DBKEY`, clé `Param_MDPSQL`) — **PAS dans `rufus.ini`**.
- À la connexion, on lit ce mot de passe via **`MySQLInstaller::motDePasseSQL()`** ;
  **s'il est absent → repli automatique sur `MDP_SQL` (`gaxt78iy`)**.

> **Pourquoi un fichier caché à part, et non `rufus.ini` ?** La méthode de
> **réinitialisation pour les tests** consiste à **supprimer `rufus.ini` puis
> relancer**. Si le mot de passe y était, on perdrait l'accès aux comptes MySQL
> existants (qui gardent, eux, le mot de passe aléatoire). Le fichier `~/.rufus/.dbkey`
> vit **hors de `~/Documents/Rufus`**, survit donc à cette suppression, et est un
> peu plus à l'abri (caché). *(Nicety future : poser l'attribut « caché » Windows.)*

### Migration sans douleur (pas de mise à jour simultanée)
```
mdp = (~/.rufus/.dbkey)[Param_MDPSQL]
si mdp vide OU connexion échoue :  mdp = MDP_SQL   (repli "legacy")
```
- **Base existante** (pas de clé) → repli `gaxt78iy` → **fonctionne comme avant**.
  Ancienne et nouvelle version coexistent ; les postes migrent à leur rythme.
- **Nouvelle base** → mot de passe aléatoire **d'emblée** → sécurisée par construction.
- **Rotation opt-in** (pour sécuriser un cabinet existant) : prévue plus tard via
  un bouton « Régénérer le mot de passe » (protégé par le mdp admin) ; comme
  `adminrufus` a `WITH GRANT OPTION`, tout poste connecté peut faire
  `ALTER USER 'adminrufus'@'%' IDENTIFIED BY '<nouveau>'`.

`MDP_SQL` est donc devenu un **repli de transition**, à retirer dans une version
lointaine une fois l'adoption large.

---

## 5. Modèle de sécurité (synthèse & limites)

**Vérité de fond** : un *client lourd* qui se connecte **directement** à MySQL
**doit** détenir un mot de passe utilisable. On ne peut donc pas le rendre
réellement secret vis-à-vis de quelqu'un qui a **accès à un poste**. Le stocker en
SHA1 est impossible (il faut le vrai mot de passe pour se connecter). Seule une
**refonte en serveur applicatif** (les postes ne détiendraient plus le mot de
passe) l'éliminerait — **hors périmètre** actuel.

L'objectif réaliste est donc de **réduire les niveaux d'exposition** :

| Niveau | Description | Mitigation |
|---|---|---|
| **Mondiale** | mot de passe public sur GitHub, identique partout | **supprimée** par le mot de passe aléatoire par cabinet (§4) |
| **Réseau (LAN)** | qui lit un `rufus.ini` du réseau peut se connecter | pare-feu ; distant déjà couvert par `adminrufusSSL REQUIRE SSL` (mdp seul inutile sans les clés SSL) |
| **Machine compromise** | accès complet à un poste | **irréductible** : les données patients y sont déjà |

**Pistes ultérieures** (non implémentées) : `chmod 600` sur `rufus.ini` ;
restriction des privilèges/host de `adminrufus` ; chiffrement des sauvegardes
(`.sql` en clair aujourd'hui) via une clé dérivée du mot de passe admin de Maurice.

---

## 6. Pointeurs dans le code

| Sujet | Où |
|---|---|
| Détection 1er démarrage / `rufus.ini` | `Procedures::PremierDemarrage()` (`procedures.cpp`) |
| Choix « Nouvelle base vierge » → installeur | `procedures.cpp`, branche `BaseVierge` |
| Module installeur MySQL | `MySQLInstaller/mysqlinstaller.{h,cpp}` |
| Mot de passe SQL (génération / stockage / repli) | `MySQLInstaller::genererMotDePasse / stockerMotDePasse / motDePasseSQL` |
| Constantes comptes & mdp | `macros.h` : `LOGIN_SQL`, `MDP_SQL`, `Param_MDPSQL`, `MDP_ADMINISTRATEUR` |
| Création des comptes à la base | `Procedures::RestaureBase()` (`procedures.cpp`) |
| Connexion / test | `Dialogs/dlg_paramconnexion.cpp`, `Database/database.{h,cpp}` |
| Détail de l'intégration + à-valider | `MySQLInstaller/NOTES_INTEGRATION_MYSQLINSTALLER.md` |
