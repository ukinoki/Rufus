# Rufus — Initialisation & sécurité des accès

> Document de référence (durable) sur **le démarrage de Rufus**, **la création
> d'une base patients**, et **le modèle de sécurité des accès à MySQL**.
> Public : développeurs reprenant Rufus. Branche de référence : **`RufusQt6`**
> (branche principale). L'ancienne branche Qt5 est archivée sous
> **`archive/Qt5-master`** et n'a plus rien de commun avec Rufus.

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

La fiche de l'installeur comporte des champs **login/mot de passe** : ils servent
à créer l'**utilisateur applicatif Rufus** (table `rufus.utilisateurs`, SHA1) ; en
mode *Verify* (un MySQL existe déjà), ils servent d'abord à se connecter au
**compte administrateur MySQL existant**. Les comptes MySQL `adminrufus`/
`adminrufusSSL`, eux, sont créés en tâche de fond avec un mot de passe aléatoire
(jamais affiché, stocké dans `~/.rufus/.dbkey`). Deux parcours : **Create** (pas de
MySQL → on l'installe) et **Verify** (MySQL déjà présent → on s'y connecte avec un
compte admin pour créer les comptes Rufus). En mode *Verify*, un bouton **« Supprimer
MySQL »** permet de **tout remettre à zéro** : après confirmation (perte de données),
le MySQL existant est désinstallé puis Rufus bascule sur le parcours *Create* pour
installer une base neuve.

> Les connexions clientes **avec identifiants** (`mysql`/`mysqladmin`) sont forcées en
> **TCP vers `127.0.0.1:3306`** (`LOCAL_TCP_ARGS`), comme le pilote Qt de Rufus et MySQL
> Workbench. Cela évite qu'un compte défini pour `@'127.0.0.1'`/`@'%'` (ou masqué par un
> compte anonyme `''@'localhost'`) soit refusé sur le socket Unix local. Seul `mysql -u
> root` via `pkexec` (mode Create, authentification socket d'Ubuntu) reste en socket.

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
- À la **création d'une base**, on génère un **mot de passe aléatoire** (12 car.
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

## 6. Diagnostic « sécurité de la connexion » & migration du mot de passe

> Cette section fige un raisonnement **subtil mais important**, pour ne pas avoir à
> le reconstruire dans quelques mois. L'essentiel est **conçu, pas encore
> implémenté** (sauf le format de `.dbkey`, §6.1, déjà en place).

### 6.1 Format de `~/.rufus/.dbkey` : mot de passe BRUT, sans étiquette

Le fichier ne contient **que le mot de passe**, sur une ligne, **sans clé ni
section** (pas de `MDPSQL=`, pas de `[Connexion]`). Un fichier au contenu opaque
ne révèle pas à quoi il sert : seuls les initiés savent que `~/.rufus/.dbkey`
porte le mot de passe MySQL d'`adminrufus`. Lu/écrit par
`MySQLInstaller::motDePasseSQL()` / `stockerMotDePasse()` (l'ancien format INI
`QSettings` reste lisible par rétro-compatibilité : détecté par la présence d'un
`=`, que le mot de passe aléatoire alphanumérique ne contient jamais).

### 6.2 Diagnostic au démarrage : une machine à états à 3 signaux

Plutôt que de **détecter « est-ce une mise à jour ? »** (heuristique fragile), on
**classe l'état du poste** à partir de trois signaux. Le 3ᵉ — *une base Rufus
joignable répond-elle ENCORE au mot de passe public ?* — est ce qui rend le
diagnostic fiable (il distingue « ancienne version » de « clé manquante »).

| `rufus.ini` | `.dbkey` | base joignable avec… | État | Action |
|:---:|:---:|---|---|---|
| absent | — | — | **premier démarrage** | `PremierDemarrage()` (nbp / base existante) |
| présent | absent | `gaxt78iy` ✅ | **ancienne version, NON sécurisée** | proposer la sécurisation (bandeau / bouton) |
| présent | présent | mdp aléatoire ✅ | **sécurisée** | rien à faire |
| présent | absent | `gaxt78iy` ❌ mais base existe | **clé manquante** (poste désynchronisé d'un cabinet déjà sécurisé) | récupérer la clé (§6.3) |
| présent | — | base injoignable | problème de connexion | déjà géré |

Points clés :
- Le 3ᵉ contrôle s'appuie sur `baseRufusComplete()` : quand `.dbkey` est absent,
  il se connecte **via le repli `gaxt78iy`** ; son succès prouve donc *à la fois*
  qu'une vraie base Rufus existe **et** qu'elle accepte encore le mot de passe public.
- **On n'écrit JAMAIS `gaxt78iy` dans `.dbkey`.** L'état « non sécurisée » EST
  défini par « `.dbkey` absent + `gaxt78iy` marche » : écrire `gaxt78iy` dans le
  fichier détruirait ce signal (on ne distinguerait plus « sécurisée » de « legacy »).
  `.dbkey` ne reçoit qu'un mot de passe **aléatoire**, et seulement à la sécurisation.
- **Emplacement** : ce diagnostic vit sur le **chemin de démarrage normal** (après
  la connexion `adminrufus` réussie), **pas** dans `MySQLInstaller` (qui ne tourne
  que sur « nouvelle base vierge », donc `rufus.ini` absent).
- La migration **logiciel seul** reste donc **silencieuse et sans friction** (le
  repli legacy connecte tout seul) : aucune question « est-ce une mise à jour ? ».

### 6.3 Migration opt-in du mot de passe (conçu, non implémenté)

Sécuriser un cabinet existant = remplacer `gaxt78iy` par un mot de passe aléatoire.
C'est **explicite et volontaire** (jamais automatique). La question préalable
détermine **deux parcours bien distincts** :

> **« Ce poste est-il le SEUL à utiliser Rufus ? »**

#### Cas A — mono-poste (≈ 90 % des installations)

Le cas le plus fréquent (Rufus est très souvent en strict mono-poste). Si la
réponse est **oui**, la rotation est **triviale, immédiate, sans clé USB** :
1. Avertir que **les versions antérieures de Rufus ne pourront plus se connecter**
   (sans conséquence ici, mais honnête).
2. Générer le nouveau mot de passe,
   `ALTER USER 'adminrufus'/'adminrufusSSL' … IDENTIFIED BY '<aléatoire>'`,
   écrire `.dbkey` localement. **Terminé.**

Pas de propagation, pas de distribution : un seul `.dbkey`, sur ce poste.

#### Cas B — multi-poste (≈ 10 %)

Si **non** (plusieurs postes partagent la base), tous doivent recevoir le nouveau
mot de passe sous peine d'être bloqués. Parcours :
1. **Proposer la migration à intervalles réguliers** (tant que `.dbkey` est absent
   et que `gaxt78iy` fonctionne = migration jamais faite), en **avertissant** qu'elle
   devra être faite sur **tous les autres postes**, sinon ils seront bloqués.
2. Si l'admin accepte (derrière `MDP_ADMINISTRATEUR`) : générer le nouveau mot de
   passe, `ALTER USER … IDENTIFIED BY '<aléatoire>'`, écrire `.dbkey` sur ce poste,
   puis demander d'**insérer une clé USB** sur laquelle le nouveau mot de passe est
   **gravé** (vecteur de distribution aux autres postes).
3. **Sur chaque autre poste**, tant qu'il n'a pas de `.dbkey`, un **bouton « Récupérer
   un mot de passe »** dans `dlg_param` (onglet *Général*) : au clic, Rufus demande la
   clé USB et **récupère le mot de passe** (écrit son `.dbkey`).

> *Note d'implémentation (n'altère pas le parcours ci-dessus) :* MySQL 8 sait porter
> **deux mots de passe** simultanément — `ALTER USER … IDENTIFIED BY '<new>' RETAIN
> CURRENT PASSWORD;` garde `gaxt78iy` valide pendant le déploiement USB (aucun poste
> coupé), puis `… DISCARD OLD PASSWORD;` une fois tous les postes migrés. À retenir
> si l'on veut une bascule **sans blocage** plutôt que le blocage assumé de l'étape 1.

### 6.4 Oubli / perte du mot de passe : la trappe de secours

**Discours utilisateur (à dessein un peu effrayant)** : *« Ce mot de passe est sur
cette clé USB. Gardez-la précieusement : sans elle, les autres postes ne pourront
plus se connecter. »* C'est un **mensonge pédagogique utile** — il pousse à la
prudence — mais **techniquement, ce n'est jamais irréversible** :

- **Les données ne sont jamais perdues.** Le mot de passe protège l'accès via
  `adminrufus`, pas les données : qui a un accès **système** à la machine hébergeant
  MySQL peut tout relire (`mysqldump` en root).
- **Le mot de passe de `adminrufus` est réattribuable** par qui contrôle le serveur :
  - **Ubuntu** : `root@localhost` via `auth_socket` → `sudo mysql` puis
    `ALTER USER 'adminrufus'@'%' IDENTIFIED BY '<nouveau>'`. C'est **exactement** le
    mécanisme déjà utilisé par l'installeur (`pkexec mysql -u root`, chemin
    `createUser` Linux) ;
  - autres OS : `mysqld --skip-grant-tables`, ou le compte root MySQL initial.

→ Rufus pourrait donc offrir, **sur la machine hôte uniquement**, un bouton
**« Réinitialiser le mot de passe MySQL »** : élévation `pkexec`, réassignation de
`adminrufus`, réécriture du `.dbkey`. Aucune clé USB nécessaire pour récupérer *sur
le serveur* (la USB ne sert qu'à *propager* aux autres postes). Cette trappe reste
**réservée à l'administrateur** : on ne la met pas en avant dans l'UX courante, pour
ne pas saper l'incitation à conserver la clé.

---

## 7. Pointeurs dans le code

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
