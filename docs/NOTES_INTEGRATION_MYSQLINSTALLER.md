# Intégration de mysqlinstaller dans Rufus — notes de conception

> Première étape de la reprise de Rufus : intégrer l'utilitaire autonome
> **ukinoki/mysqlinstaller-for-rufus** comme **module natif** de Rufus, avec
> mot de passe MySQL **aléatoire par cabinet**, et ergonomie `up*`.
>
> Branche : `claude/wizardly-edison-f9mnax` (basée sur `RufusQt6`).

---

## 1. Ce qui a été décidé (résumé des échanges)

### Architecture
- L'installeur devient une **classe native** `MySQLInstaller` (dossier
  `MySQLInstaller/`), **pas** un exécutable séparé. Maintenance unique, dans Rufus.
- Lancée depuis **« Nouvelle base patients vierge »** de `Procedures::PremierDemarrage()`.
  Contrat de retour :
  - base MySQL présente **et conforme** aux exigences Rufus → `true` (Rufus continue) ;
  - base absente, l'utilisateur **accepte** l'installation → install + config → `true` ;
  - l'utilisateur **annule / échec** → `false`, et **Rufus reste sur l'écran
    `PremierDemarrage()`** (ne quitte pas, ne continue pas).

### Comptes MySQL (sécurité)
- Il n'existe que **2 comptes MySQL** : `adminrufus` (local/réseau) et
  `adminrufusSSL` (distant, `REQUIRE SSL`). Les utilisateurs Rufus, eux, vivent
  dans la table `rufus.utilisateurs` et sont validés **par Rufus**, pas par MySQL.
- **Plus de compte temporaire** : l'installeur intégré crée **directement**
  `adminrufus` + `adminrufusSSL`. La faille de l'ancien « compte temporaire jamais
  réutilisé mais toujours présent » disparaît.
- **Maurice ne saisit aucun identifiant *MySQL***. La fiche a bien des champs
  login/mdp, mais ils servent à l'**utilisateur applicatif Rufus** (table
  `rufus.utilisateurs`, SHA1) — sauf en mode *Verify* (B2) où ils servent à se
  connecter au **compte administrateur MySQL existant** (voir §10).

### Mot de passe MySQL aléatoire par installation (la mesure clé)
Aujourd'hui `MDP_SQL = "gaxt78iy"` est **codé en dur, identique pour toutes les
installations, donc public via GitHub**. C'est LA faille.

Nouvelle logique :
- À la **création d'une base**, on génère un **mot de passe aléatoire fort** (12
  caractères alphanumériques), on crée `adminrufus`/`adminrufusSSL` avec, et on le
  stocke dans un **fichier CACHÉ** `~/.rufus/.dbkey` (`PATH_FILE_DBKEY`, clé
  `Param_MDPSQL`) — **PAS dans `rufus.ini`** (voir encadré ci-dessous).
- À la connexion, Rufus lit ce mot de passe ; **s'il est absent OU vide, repli
  automatique sur `MDP_SQL` (`gaxt78iy`)**.

> **Pourquoi pas dans `rufus.ini` ?** La **réinitialisation pour tests** = supprimer
> `rufus.ini` puis relancer. Si le mot de passe y était, on perdrait l'accès aux
> comptes MySQL existants. Le fichier `~/.rufus/.dbkey` est **hors de
> `~/Documents/Rufus`** → survit à la suppression de `rufus.ini`, et est caché.

Pourquoi c'est sûr et non disruptif (cf. règle de migration ci-dessous).

---

## 2. Règle de migration (IMPORTANTE) — pas de mise à jour simultanée

Le risque redouté : « tous les postes d'une base partagée doivent changer de
version et de `rufus.ini` en même temps ». **On l'évite** en *découplant* la
migration de la mise à jour :

```
mdp = (~/.rufus/.dbkey)[Param_MDPSQL]         (fichier caché, hors ~/Documents/Rufus)
si mdp est vide  OU  la connexion échoue :
    mdp = MDP_SQL  ("gaxt78iy")               (repli "legacy", toujours dans le code)
se connecter avec mdp
```

- **Base existante** : pas de clé `Param_MDPSQL` → repli sur `gaxt78iy` →
  **fonctionne exactement comme avant**. Ancienne et nouvelle version coexistent.
- **Nouvelle base** : mot de passe aléatoire d'emblée → **sécurisée par construction**.
- **Rotation opt-in** (cabinet existant qui veut sécuriser) : prévue plus tard,
  via un bouton « Régénérer le mot de passe » (protégé par le mdp admin). Comme
  `adminrufus` a `WITH GRANT OPTION`, n'importe quel poste connecté peut faire
  `ALTER USER 'adminrufus'@'%' IDENTIFIED BY '<nouveau>'`. **Pas implémenté ici.**

Le `MDP_SQL` du code devient donc un **repli de transition** (à retirer dans une
version lointaine, une fois l'adoption large).

> Conséquence rassurante : tous les remplacements `MDP_SQL → MySQLInstaller::motDePasseSQL()`
> sont **iso-comportement pour les bases existantes** (le repli renvoie `gaxt78iy`).

---

## 3. Modèle de sécurité (rappel, pour le futur)

- Vérité de fond : un client lourd qui se connecte **directement** à MySQL **doit**
  détenir un mot de passe utilisable. On ne peut pas le rendre vraiment secret
  pour qui a accès à un poste. SHA1 impossible (il faut le vrai mdp pour se
  connecter). Seul un **serveur applicatif** intermédiaire l'éliminerait (gros
  chantier, hors périmètre).
- Trois niveaux d'exposition : **mondiale** (mdp public sur GitHub — supprimée par
  le mdp aléatoire), **réseau** (atténuée par pare-feu + `REQUIRE SSL` distant),
  **machine déjà compromise** (irréductible : les données patients y sont déjà).
- L'accès **distant** est déjà bien protégé : `adminrufusSSL REQUIRE SSL` → le mdp
  seul ne suffit pas sans les clés SSL clientes.
- Pistes ultérieures (non faites) : `chmod 600` sur `rufus.ini`, restriction des
  privilèges/host de `adminrufus`, chiffrement des sauvegardes via le mdp admin.

---

## 4. Ce qui a été fait dans cette intégration

| Élément | Fichier(s) | État |
|---|---|---|
| Module installeur natif | `MySQLInstaller/mysqlinstaller.{h,cpp}` | **nouveau** |
| Enregistrement build | `MySQLInstaller/mysqlinstaller.pri`, `RufusQt6.pro` | fait |
| Clé ini mot de passe | `macros.h` → `Param_MDPSQL` | fait |
| Repli legacy + génération + stockage | `MySQLInstaller::motDePasseSQL()` / `genererMotDePasse()` / `stockerMotDePasse()` | dans le module |
| Câblage « Nouvelle base vierge » | `procedures.cpp::PremierDemarrage()` | voir §5 |
| Bascule des points de consommation `MDP_SQL` | voir §6 | voir §6 |

---

## 5. Câblage dans PremierDemarrage()

Dans `procedures.cpp::PremierDemarrage()`, branche `protoc == BaseVierge`,
**avant** `VerifParamConnexion(...)` : appel à `MySQLInstaller::run()`.
- `run() == false` → on **reboucle** sur l'écran de choix (la fiche
  `PremierDemarrage` reste affichée), au lieu de continuer ou de quitter.
- `run() == true` → suite normale (`VerifParamConnexion`, `RestaureBase`, etc.).

---

## 6. Points de consommation de MDP_SQL — à basculer sur le repli

`MySQLInstaller::motDePasseSQL()` remplace l'usage direct de `MDP_SQL` aux
endroits qui **se connectent** ou **sauvegardent**. Liste exhaustive (relevée le
jour de l'intégration) :

| Fichier:ligne | Rôle | Action |
|---|---|---|
| `Dialogs/dlg_paramconnexion.cpp:209` | `Password = MDP_SQL;` (test base) | → `motDePasseSQL()` |
| `Dialogs/dlg_paramconnexion.cpp:255` | `connectToDataBase(DB_RUFUS, LOGIN_SQL, MDP_SQL)` | → `motDePasseSQL()` |
| `procedures.cpp:661,663,665,667,670` | scripts `mysqldump` de sauvegarde | → `motDePasseSQL()` |
| `procedures.cpp:914` | args de connexion (`-p`) | → `motDePasseSQL()` |
| `procedures.cpp:2362,2364` | `RestaureBase` : `CREATE USER ... IDENTIFIED BY MDP_SQL` | → `motDePasseSQL()` |
| `procedures.cpp` (~3326) | `connectToDataBase(DB_RUFUS)` (défaut) | → `connectToDataBase(DB_RUFUS, LOGIN_SQL, motDePasseSQL())` |
| `Dialogs/dlg_identificationuser.cpp:124` | login au démarrage : `connectToDataBase(DB_RUFUS)` (défaut) | → idem (**critique** : sans ça, une base neuve ne se connecterait pas) |
| `Dialogs/dlg_paramconnexion.cpp:213` | connexion en mode **Poste** | → connexion **toujours via `adminrufus`/`motDePasseSQL()`** (voir ci-dessous) |
| `Database/database.h:125` | **défaut** `password = MDP_SQL` de `connectToDataBase` | **laissé en MDP_SQL** mais **plus aucun appelant ne s'appuie sur le défaut** (les 4 sites passent désormais le mdp explicitement). |

> 🔑 **Changement sémantique (élimination du compte temporaire)** : la connexion
> MySQL passe **toujours** par le compte fixe `adminrufus` (`dlg_paramconnexion.cpp:213`,
> mode Poste). Auparavant, la branche « base vierge » se connectait avec les
> **identifiants saisis** (l'ancien compte MySQL temporaire). Désormais ces
> identifiants ne sont que l'**identité applicative Rufus** (table `utilisateurs`,
> créée par `CreerPremierUser`), jamais un compte MySQL. C'est l'élimination de la
> faille du « compte temporaire ». **À valider en priorité** (cf §7).

> ⚠️ **Ces remplacements sont iso-comportement pour les bases existantes** (repli =
> `gaxt78iy`, et `adminrufus` y existe déjà avec ce mot de passe). Le principal
> risque est une **erreur de compilation** — voir §7.

---

## 7. ⚠️ Limites — à valider sur TON build (impératif avant de t'y fier)

Je n'ai **pas pu** compiler ni exécuter dans l'environnement de développement :
- pas de vrai serveur MySQL, pas d'interface graphique ;
- pas de build complet de Rufus (poppler, multimedia, toute la couche `Datas`…).
  Les composants `up*` tirent `Item`/`Utils`/`Icons`, donc même le module seul ne
  compile pas isolément sans une grande partie du projet.

**À faire de ton côté :**
1. **Compiler** `RufusQt6.pro` (qmake6) et corriger les éventuelles erreurs de
   compilation — surtout sur les fichiers cœur modifiés (`procedures.cpp`,
   `dlg_paramconnexion.cpp`) et le module.
2. **Tester le premier démarrage** « Nouvelle base vierge » sur une machine Ubuntu
   propre (ta cible) : détection MySQL, install apt, config (dossier partagé,
   Samba, `secure_file_priv`, PATH), création `adminrufus`/`adminrufusSSL`, écriture
   de `Param_MDPSQL` dans le fichier caché `~/.rufus/.dbkey`, puis connexion.
3. **Tester la non-régression** d'une base **existante** (sans `Param_MDPSQL`) :
   doit se connecter via le repli `gaxt78iy`, comme avant.
4. Vérifier le déroulé **multi-postes** (base partagée) : un 2e poste qui se
   connecte à une base déjà créée doit obtenir `Param_MDPSQL`… (mécanisme de
   transmission inter-postes **non implémenté** — voir §2 rotation/clé de connexion ;
   pour l'instant, en pratique, le repli legacy couvre les bases non migrées).

---

## 8. Choix d'implémentation à connaître

- **Pas de port du sélecteur de langue** de l'installeur (Rufus gère déjà l'i18n).
- **Pas de saisie login/mdp** (comptes fixes + mdp aléatoire).
- L'écran de l'installeur est une **checklist** (6 cases `UpCheckBox` cochées en
  direct), construite sur `UpDialog` ; messages/confirmations via `UpMessageBox`
  (`Information` / `Watch` / `Question`) ; progression via un `MySQLProgressDialog`
  interne (porté du `ProgressDialog` autonome, autonome lui aussi).
- Les comptes sont créés en `@'%'` (réseau), conformément à `RestaureBase`.
- Composants `up*` : **non modifiés** (prudence — utilisés partout). Les « erreurs
  de conception » repérées seront listées séparément pour décision, pas changées à
  l'aveugle.

---

## 9. Précision (à trancher) — login/mdp = utilisateur APPLICATIF Rufus

Clarification de l'auteur : **les champs login/mdp restent** dans l'installeur
intégré à Rufus — mais ils ne créent **pas** un compte MySQL. Ils servent à créer
l'**utilisateur applicatif Rufus** qui installe la base : login + mot de passe
stockés dans **`rufus.utilisateurs`** (mdp en **SHA1**), et que le praticien
utilisera ensuite pour se connecter au logiciel.

Répartition :
- **`MySQLInstaller` (engine)** : ne crée QUE les comptes MySQL `adminrufus` +
  `adminrufusSSL` (mot de passe aléatoire dans le fichier caché `~/.rufus/.dbkey`). ✓ déjà le cas.
- **Champs login/mdp (utilisateur Rufus)** : à **conserver** dans le parcours
  d'installation intégré. Aujourd'hui ils sont collectés par
  `dlg_paramconnexion` et `Procedures::CreerPremierUser(login, MDP)` crée déjà
  l'utilisateur applicatif (SHA1). **Le fond est donc en place.**

> **Question ouverte (UX, à trancher)** : faut-il **déplacer** les champs login/mdp
> *dans* le dialogue de l'installeur (`MySQLInstallerDialog`), ou les **laisser**
> dans `dlg_paramconnexion` (qui porte aussi le choix Poste/Local/Distant) ?
> - *Laisser dans dlg_paramconnexion* : zéro travail, déjà fonctionnel.
> - *Déplacer dans l'installeur* : parcours plus unifié, mais il faut alors retirer
>   la saisie login/mdp de `dlg_paramconnexion` pour la branche « base vierge » et
>   faire remonter les valeurs jusqu'à `CreerPremierUser` (refonte du flux).

### TODO dépôt autonome (hors de cette session, scope = ukinoki/rufus)
Dans **`ukinoki/mysqlinstaller-for-rufus`** : **supprimer** les champs login/mdp
(`CredentialsDialog`) — l'utilitaire autonome ne crée plus que les comptes MySQL
`adminrufus`/`adminrufusSSL` avec un mot de passe aléatoire écrit dans un
fichier caché `~/.rufus/.dbkey` sur la machine. (À faire dans une session sur ce dépôt.)


---

## 10. Flux `run()` implémenté (create / verify) — référence

Clic « Nouvelle base patients vierge » (nbp, **toujours monoposte/local**) →
`MySQLInstaller::run()` → `bool`. `dlg_paramconnexion` **n'est plus appelé** pour
nbp (ses effets « Poste » — mode/localhost/port/rufus.ini — sont relogés en dur
dans `PremierDemarrage`).

- **A. Pas de MySQL** → message « il faut installer MySQL, OK ? » (Non = `false`) →
  fiche **Create** (login/mdp = futur user Rufus) → install MySQL + `adminrufus`/
  `adminrufusSSL` (mdp aléatoire → `.dbkey`) + config → `true`.
- **Version trop ancienne** → `askUpdateConfirmation` → désinstallation → chemin Create.
- **B1. Base Rufus déjà complète** (`adminrufus` se connecte **+** base `rufus` avec
  ≥ 1 table) → message « une base Rufus existe déjà » + `return false` (ce n'est pas
  le rôle de nbp ; passer par « base existante »).
- **B2. MySQL présent mais pas une base Rufus** → fiche **Verify** : login/mdp d'un
  **compte admin MySQL existant**, bouton « Se connecter » → connexion → création
  `adminrufus`/`adminrufusSSL` via ce compte (**gestion du manque de droit
  `CREATE USER`** → message + `false`) → fiche **« Créer le compte »** (futur user
  Rufus) → `true`.

Getters `loginRufus()` / `mdpRufus()` → exposent le user applicatif ; `PremierDemarrage`
appelle `RestaureBase(true,true)` (structure + schémas) **puis**
`CreerPremierUser(loginRufus(), mdpRufus())` (SHA1).

### Réintégré du dépôt autonome
- `runCmdElevated` (macOS) : timeout `osascript` porté à **900000 ms** (corrige le
  « uninstall fantôme » — `waitProcessResponsive` tuait l'élévation au défaut 30 s,
  d'où un faux « succès »). Repris du commit `d9917a7` de mysqlinstaller-for-rufus.

### TODO (plus tard)
- Si MySQL détecté et base corrompue : *Verify* devrait proposer **« garder le
  serveur »** ou **« tout réinitialiser »** (→ suppression complète de MySQL puis
  bascule en mode Create). Non implémenté.
- Reset *total* (régénérer les comptes) = supprimer aussi `~/.rufus/.dbkey`.
