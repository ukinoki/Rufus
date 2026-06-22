# Déroulé RÉEL du démarrage de Rufus (état des lieux du code)

> **But de ce fichier.** Ceci est la *photographie de l'existant* : ce que le code
> fait **aujourd'hui** au lancement, reconstruit en suivant les sources (avec
> références `fichier:ligne`). Il n'est pas la cible. La cible — la *guideline*
> que l'on veut atteindre — est `INSTALLATION_ET_INITIALISATION_RUFUS.md`. En
> comparant les deux, on voit l'écart qui reste à combler.
>
> Quand le code change, **mettre à jour ce fichier** (il décrit le réel, pas le
> souhaité). Les références de lignes sont indicatives : elles bougent avec le
> code.

---

## 0. Le piège à défaire d'abord : il y a DEUX mots de passe, pas un

La plus grosse source de confusion. `.dbkey` / `gaxt78iy` et « login/mot de
passe » ne sont **pas** le même niveau :

| Niveau | Qui se connecte | Mot de passe utilisé | Où c'est résolu |
|---|---|---|---|
| **MySQL** (technique) | `adminrufus` / `adminrufusSSL` | aléatoire du cabinet (dans `~/.rufus/.dbkey`) **+** `gaxt78iy` conservé en 2ᵉ | cascade `MySQLInstaller::connecterAvecCandidats()` / `motDePasseSQL()` |
| **Application** (métier) | le praticien | son login/mot de passe perso | `dlg_identificationuser` → table `rufus.utilisateurs` |

- La cascade **`.dbkey` (aléatoire) → `gaxt78iy`** ouvre la **connexion MySQL** de
  bas niveau (le compte `adminrufus`). `gaxt78iy` (`MDP_SQL`) est le mot de passe
  générique de bootstrap, conservé 30 jours comme 2ᵉ mot de passe pour qu'un
  nouveau poste puisse se connecter avant de récupérer l'aléatoire.
- Le **login/mot de passe demandé à l'écran**, c'est l'**identification du
  praticien** (couche au-dessus), ou la saisie des paramètres dans le cas
  « base existante ».

Tout raisonnement sur le démarrage doit garder ces deux couches séparées.

---

## 1. Le vrai pivot : `rufus.ini`, PAS le serveur

Contrairement à l'intuition (« au démarrage, Rufus regarde d'abord s'il y a un
serveur MySQL »), le code **pivote sur la présence et la validité de
`rufus.ini`**. La détection du serveur MySQL n'intervient **qu'à l'intérieur** du
chemin « premier démarrage », lui-même déclenché par l'**absence** d'`ini`.

Conséquence structurante : `rufus.ini` est traité comme la **source de vérité**,
alors qu'il n'est en réalité qu'un *cache* de la réponse à la vraie question
métier (« ai-je un serveur, et une base Rufus dessus ? »).

---

## 2. Séquence au lancement

L'orchestration se joue en deux temps : le **constructeur `Procedures`** (créé en
premier via `Procedures::I()`), puis le **constructeur `Rufus`**.

### 2.1 `main()` — `main.cpp:75`
- Plateforme, filtre clavier, splash.
- `Rufus w;` (`main.cpp:130`) → déclenche tout le reste.

### 2.2 Constructeur `Procedures` — `procedures.cpp:29`
Appelé dès `proc = Procedures::I();` au tout début du constructeur `Rufus`
(`rufus.cpp:39`).

1. **Langue de l'interface** (`procedures.cpp:50‑126`)
   `rufus.ini` (`Param_Poste_Version`) fait foi ; sinon `QLocale` ; sinon
   sélecteur manuel (FR par défaut).

2. **`rufus.ini` ABSENT ?** (`procedures.cpp:128‑147`)
   Boucle sur `RecupererDemarrage(msg, msgInfo, true, true, true, **true**)` — le dernier
   paramètre `PremDemarrage = true` ajoute le bouton **« Premier démarrage »**.
   → cf. §3.

3. **`rufus.ini` PRÉSENT mais paramètres de connexion INVALIDES ?**
   (`procedures.cpp:148‑186`)
   Test `k` : au moins un mode (Poste / Réseau local / Distant) actif avec un
   port plausible (3306/3307) et, pour les modes distants, un serveur renseigné.
   Si invalide → boucle sur `RecupererDemarrage(..., **false**)` (PremDemarrage = **false**,
   donc **PAS** de bouton « Premier démarrage » : seulement Restaurer /
   Reconstruire / Quitter).

4. Divers (imprimante, appareils de réfraction, `ReconstruitListeModesAcces()`).

### 2.3 Constructeur `Rufus` — `rufus.cpp:22`
On n'arrive ici **que si `rufus.ini` existe et est exploitable** (sinon §3 a
déjà bouclé ou quitté).

- `proc->ListeModesAcces().size()` (`rufus.cpp:43`) :
  - **0** → « Rufus.ini endommagé », `exit(0)`.
  - **1** → `db->setModeacces(...)` puis `Connexion_A_La_Base()` (`rufus.cpp:49`).
  - **≥ 2** → `FicheChoixConnexion()` (choix du mode) puis
    `Connexion_A_La_Base()` (`rufus.cpp:56‑58`).

### 2.4 `Connexion_A_La_Base()` — `procedures.cpp:3060`
Le chemin **normal** (ini valide) :
1. `initParametresConnexionSQL(server, port)` → connexion **MySQL** (cascade
   `.dbkey` / `gaxt78iy` côté `adminrufus`).
2. `IdentificationUser()` → login **praticien** (`dlg_identificationuser`).
3. Lieu d'exercice, création de session, `max_allowed_packet`, etc.

---

## 3. La branche « premier démarrage » (uniquement si `rufus.ini` absent)

`RecupererDemarrage(..., PremDemarrage=true)` (`procedures.cpp:4557`) affiche les boutons :
**[Premier démarrage] [Restaurer le .ini] [Reconstruire le .ini] [Quitter]**.

- **Quitter** → `exit(0)`.
- **Restaurer** → recopie un `rufus.ini` sauvegardé puis **relance** Rufus.
- **Reconstruire** → `VerifParamConnexion(true)` pour ressaisir les paramètres.
- **Premier démarrage** → `PremierDemarrage()` (`procedures.cpp:4328`).

### 3.1 `PremierDemarrage()` — `procedures.cpp:4328`
Demande : *« Base patients vierge »* ou *« Base existante sur le serveur »*.

#### A. « Base vierge » → `MySQLInstaller::run()` (`mysqlinstaller.cpp:~945`)
C'est **ici**, et seulement ici, qu'on inspecte l'état réel du serveur. Les trois
situations métier vivent toutes dans cette fonction :

```
isMySQLInstalled() == false
        → CAS 1 : aucun serveur
          askYesNo → assurerDroitsAdmin → faireCreate(cfg)
          (installe MySQL puis le configure : comptes, secure_file_priv, SSL…)

isMySQLInstalled() == true
        startMySQL() si besoin
        ├─ baseRufusComplete() == true
        │      → CAS 3 : serveur + base Rufus déjà présente
        │        Boîte : Conserver / Supprimer / Annuler
        │          • Conserver / Annuler → return false  ──► retour PremierDemarrage()
        │                                  (l'utilisateur doit alors choisir « Base existante »)
        │          • Supprimer → efface les bases Rufus puis continue
        └─ baseRufusComplete() == false
               → CAS 2 : serveur présent, pas de base Rufus
                 on continue la configuration
```

Après un `run()` qui aboutit (cas 1 ou 2, ou cas 3 « Supprimer ») :
`PremierDemarrage()` poursuit (`procedures.cpp:4406‑4448`) :
- connexion MySQL en cascade (`connecterAvecCandidats`),
- `RestaureBase(true, true)` → **crée** toute la structure de la base,
- `CreerPremierUser(login, MDP)` → 1ᵉʳ utilisateur applicatif,
- `PremierParametrageMateriel()` → écrit `rufus.ini` + dossiers,
- boîte « Redémarrage nécessaire » (affiche le mot de passe à conserver),
- **redémarrage automatique** puis `exit(0)`.

#### B. « Base existante » → `VerifParamConnexion()` (`procedures.cpp:4361‑4372`)
Se connecte à un serveur/base **déjà là** (saisie des paramètres), écrit
`rufus.ini`, affiche « Connexion réussie », **redémarre automatiquement**.

---

## 4. Où vivent les 3 situations, et l'entortillement avec `rufus.ini`

| Situation | Traitée où | Atteignable seulement si… |
|---|---|---|
| **1. Pas de serveur** → installer + configurer | `MySQLInstaller::run()` cas `!installed` | `rufus.ini` absent → Premier démarrage → Base vierge |
| **2. Serveur, pas de base Rufus** → configurer | `run()` cas `baseRufusComplete()==false` | idem |
| **3. Serveur + base Rufus** → s'y connecter | `run()` cas `baseRufusComplete()==true` (Conserver) **puis** « Base existante » | idem |

**Le nœud :** ces trois cas sont **tous** enfermés dans la branche
« premier démarrage », donc **inaccessibles dès que `rufus.ini` existe et est
valide**. Avec un `rufus.ini` exploitable, on file droit en
`Connexion_A_La_Base` sans jamais re-sonder serveur ni base.

---

## 5. Points de friction connus (à arbitrer dans la cible)

> Ces points décrivent des limites de l'existant. Ils sont la matière du travail
> de convergence vers `INSTALLATION_ET_INITIALISATION_RUFUS.md`.

1. **La détection du serveur est inaccessible si `rufus.ini` existe.**
   Un `rufus.ini` valide pointant vers un serveur mort ou une base disparue
   échoue **sec**, sans auto-réparation ni proposition de réinstaller. Le seul
   moyen d'atteindre `run()` est de n'avoir **pas** d'`ini`.

2. **« Base existante » fait re-saisir ce que `run()` savait déjà.**
   Quand `run()` détecte une base complète et qu'on clique « Conserver », on est
   renvoyé à l'écran de premier démarrage pour re-choisir « Base existante » →
   `VerifParamConnexion`, alors que la base venait d'être identifiée.
   Aller-retour inutile.

3. **Deux boucles `RecupererDemarrage` quasi jumelles** (`ini` absent vs `ini` invalide)
   qui ne diffèrent que par le bouton « Premier démarrage ». Logique dupliquée.

4. **L'ordre « `ini` d'abord » est l'inverse de l'intuition métier.**
   Si la vraie question est « ai-je un serveur, et une base Rufus dessus ? »,
   alors `rufus.ini` n'est qu'un *cache* de la réponse, pas la source de vérité.
   On pilote sur le cache, pas sur l'état réel — d'où l'impression de
   « plat de spaghetti » à cet endroit.

---

## 6. Fonctions clés (index pour naviguer)

| Rôle | Fonction | Emplacement |
|---|---|---|
| Orchestration init | `Procedures::Procedures()` | `procedures.cpp:29` |
| `ini` absent/invalide → choix | `Procedures::RecupererDemarrage()` | `procedures.cpp:4557` |
| Premier démarrage (vierge/existante) | `Procedures::PremierDemarrage()` | `procedures.cpp:4328` |
| Détection serveur + base (3 cas) | `MySQLInstaller::run()` | `mysqlinstaller.cpp:~945` |
| Connexion normale (ini valide) | `Procedures::Connexion_A_La_Base()` | `procedures.cpp:3060` |
| Connexion MySQL en cascade | `MySQLInstaller::connecterAvecCandidats()` | `mysqlinstaller.cpp:560` |
| Mot de passe MySQL du mode | `MySQLInstaller::motDePasseSQL()` | `mysqlinstaller.cpp:509` |
| Config `secure_file_priv` (my.cnf + restart) | `MySQLInstaller::ensureSecureFilePriv()` | `mysqlinstaller.cpp:2644` |
