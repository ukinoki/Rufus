# La fiche Paramètres (`dlg_param`)

Ce document explique **comment fonctionne** la fiche des paramètres : pourquoi
elle est découpée en trois onglets, comment fonctionnent les trois cadenas, et
quand les réglages sont réellement écrits. Il s'adresse à qui doit y ajouter un
paramètre ou y corriger un comportement.

Fichiers : `Dialogs/dlg_param.cpp` / `.h` / `.ui`.
Ouverte par `Rufus::OuvrirParametres()` (menu Édition → Paramètres).

---

## 1. Pourquoi trois onglets

Le découpage ne suit pas les thèmes (« réseau », « affichage », « impression »)
mais **la portée du réglage** : *à qui appartient ce que je modifie ?* Trois
réponses possibles, donc trois onglets — et, ce qui compte autant, **trois
verrous différents**, parce qu'on ne protège pas de la même façon une préférence
personnelle et un réglage qui engage tout le cabinet.

| Onglet | Le réglage appartient à… | Où il est écrit | Qui a le droit | Effet d'une erreur |
|---|---|---|---|---|
| **Utilisateur** (`UserParamtab`) | la personne connectée | la base (tables `utilisateurs`, jointures de cotations) | **son propre** mot de passe | limitée à l'utilisateur |
| **Ce poste** (`PosteParamtab`) | l'ordinateur devant lequel on est | `rufus.ini` et `.dbkey` — **jamais** la base | mot de passe **administrateur** | ce poste ne se connecte plus |
| **Général** (`GeneralParamtab`) | la base, donc tout le cabinet | la base | mot de passe **administrateur**, et RufusAdmin ne doit pas tourner | tout le monde est touché |

### Onglet « Utilisateur » — *moi*

Identité (nom, prénom, titre), login et mot de passe, police d'écran, lieux
d'exercice et numéros AM, et surtout **ses cotations** : la liste des actes qu'il
facture et ses montants pratiqués. Deux médecins du même cabinet n'ont ni les
mêmes tarifs ni les mêmes actes : ces données sont attachées à l'utilisateur, via
ses tables de jointure.

Le verrou est **son mot de passe à lui** (`Utils::VerifMDP(currentuser()->password())`) :
personne d'autre n'est concerné, un mot de passe administrateur serait
disproportionné. Corollaire respecté partout dans la page : un remplaçant
(`idparent() != id()`) voit les cotations de son parent mais **ne les modifie
pas** — les boutons +/−/modifier et l'édition du montant pratiqué restent fermés.

### Onglet « Ce poste » — *cette machine*

Mode de connexion à la base (monoposte / réseau local / accès distant), port
MySQL, mot de passe MySQL, clés SSL, instruments de réfraction branchés sur les
ports COM ou lisant un dossier d'échange, dossiers d'imagerie **tels que ce poste
les voit**, marges d'imprimante, ville et code postal par défaut, priorité
d'import des documents.

Rien de tout cela n'a de sens ailleurs que sur cet ordinateur : le poste du
secrétariat n'a pas l'autoréfractomètre, et le dossier d'imagerie du serveur
s'appelle `C:\Users\Public\Rufus\Imagerie` sur le serveur et `Z:\Imagerie` sur le
poste voisin. D'où le stockage dans `rufus.ini` (`proc->settings()`) et, pour les
mots de passe MySQL, dans le `.dbkey` — **pas dans la base**.

Le verrou est le **mot de passe administrateur** bien qu'il ne s'agisse que d'un
poste : une faute de frappe dans l'adresse du serveur ou le mot de passe MySQL
coupe la machine de la base au prochain démarrage.

### Onglet « Général » — *la base*

Gestion des utilisateurs, des lieux d'exercice, des motifs de consultation, des
banques, mot de passe administrateur, sauvegarde et restauration de la base,
langue, appareils déclarés sur le site, mode de comptabilité, cotations
françaises, base des villes, lieu par défaut du serveur, numéros de version.

Ces réglages sont **lus au démarrage** et partagés par tous les postes : d'où les
avertissements « ne sera pris en compte qu'au prochain démarrage » (cotations
France) ou le redémarrage imposé (mode comptabilité, langue). Et d'où une
protection supplémentaire : le cadenas **refuse de s'ouvrir si une session
RufusAdmin est active** (`Datas::I()->postesconnectes->admin()`) — deux fiches
modifiant en même temps les mêmes paramètres généraux, c'est le meilleur moyen
d'en perdre la moitié.

### La règle, quand on ajoute un paramètre

Une seule question : **qui possède ce réglage ?**

- il ne vaut que pour moi → onglet Utilisateur, écriture en base sur mon `iduser` ;
- il ne vaut que pour cette machine → onglet Ce poste, écriture dans `rufus.ini` ;
- il vaut pour la base → onglet Général, écriture en base.

Se tromper de rangement, ce n'est pas une faute d'ergonomie : c'est écrire dans
la base un réglage qui devait rester local (et l'imposer à tous les postes), ou
l'inverse (et le perdre à chaque nouveau poste).

---

## 2. Les trois cadenas

Tout s'ouvre **fermé**. Le constructeur désactive en bloc les widgets des trois
pages (`enableCotations(false)`, `EnableWidgContent(...)`, `setEnabled(false)`) ;
seuls les trois cadenas restent cliquables.

Le mécanisme est le même pour les trois :

1. `eventFilter()` capte le relâchement de souris sur un `UpLabel` et émet
   `click(obj)` ;
2. `click` est relié à `EnableModif(QWidget*)`, qui reconnaît le cadenas à son
   pointeur et regarde **son pixmap** : cadenas fermé → on demande le mot de
   passe ; cadenas ouvert → on referme (et pour « Ce poste », on enregistre au
   passage) ;
3. l'état ouvert/fermé se relit ensuite sur le pixmap, et pilote le
   `setEnabled()` de tous les widgets de la page.

Le mot de passe n'est demandé qu'une fois par session de la fiche :
`m_MDPadminverifie` et `m_MDPuserverifie` mémorisent la vérification. Le cadenas
administrateur est partagé par « Ce poste » et « Général » — ouvrir l'un dispense
de ressaisir pour l'autre.

Deux détails utiles :

- Les cases à cocher et boutons radio de `Principalframe` sont mis en
  `setToggleable(false)` : ils affichent un état, ils ne se cliquent pas.
- Table des cotations verrouillée : un filtre d'événements est **posé** sur le
  viewport et la zone des boutons pour avaler le clic et afficher « Page
  verrouillée, cliquez sur le cadenas » (`m_msgVerrouCotations`) ; il est
  **retiré** au déverrouillage (`enableCotations`).

---

## 3. Onglet « Utilisateur » en détail

- **Données personnelles** : affichées par `AfficheParamUser()`, modifiées par
  `dlg_gestionusers` en mode `MODIFUSER` (bouton « Modifier mes données »). Le
  retour lève `m_donneesusermodifiees`.
- **Police d'écran** : `dlg_fontdialog`, écriture immédiate en base.
- **Lieux d'exercice** : `ReconstruitListeLieuxExerciceUser()` remplit la table ;
  le petit bouton de chaque ligne enregistre le **numéro AM** propre au site
  (`fixAMnumberforSite`, table `jointureslieux`).
- **Cotations** : `Cotationswidget` est **masqué** pour les secrétaires et les
  « autres fonctions », qui ne facturent pas. La table elle-même
  (`cotationsUpTableView`, modèle `m_modelCotations`) et son mécanisme sont
  documentés à part → **`docs/COTATIONS_README.md`** et le bloc en tête de
  `dlg_param.h`. Un bouton « ? » affiche l'aide de la table (`whatsThis`), et
  reste actif page verrouillée, volontairement.

Tout, dans cet onglet, s'écrit **immédiatement** en base.

---

## 4. Onglet « Ce poste » en détail

### Pourquoi trois sous-onglets Monoposte / Réseau local / Accès distant

Un même poste peut être configuré pour les trois modes à la fois : le portable du
médecin est serveur au cabinet et client à distance depuis chez lui. Chaque mode a
donc son jeu complet de réglages (actif ou non, adresse, port, mot de passe MySQL,
dossiers), rangés sous la clé `Utils::getBaseFromMode(mode)` de `rufus.ini`. Le
sous-onglet ouvert au démarrage de la fiche est celui du **mode courant**
(`db->ModeAccesDataBase()`).

Décocher un mode le désactive (`ACTIVE=NO`) mais **conserve** ses réglages et son
mot de passe : on peut le réactiver plus tard sans tout ressaisir.

### Mots de passe MySQL

Les trois champs sont équipés par `ConfigureChampMDPMySQL()` : un œil pour
afficher/masquer, un bouton pour recopier le mot de passe sur une clé USB (afin
de le porter sur un autre poste).

Garde-fou important : **seul** le champ du mode courant est surveillé. C'est celui
qui contient le mot de passe ayant réellement servi à se connecter ; si on le
modifie, un message propose « Oups » (rétablir) ou la confirmation explicite. Pour
les deux autres modes, un mot de passe non vérifiable est légitime, on ne dit
rien.

### Clés SSL — trois boutons à ne pas confondre

| Bouton | Où | Ce qu'il fait |
|---|---|---|
| Exporter les clés client SSL | Monoposte (serveur) | copie sur clé USB les clés **client** du serveur hébergé ici, à déployer sur les postes distants |
| Exporter les clés client SSL | Accès distant | copie sur clé USB les clés d'accès **à un serveur distant** dont ce poste dispose, pour équiper un autre poste distant |
| Créer de nouvelles clés SSL | Monoposte (serveur) | **destructif** : régénère les clés du serveur, invalide toutes celles déjà distribuées, redémarre MySQL puis Rufus |

Un même poste peut détenir les deux jeux : c'est pourquoi les deux exports
coexistent.

### Instruments de réfraction

Pour chaque appareil (frontofocomètre, autoréfractomètre, réfracteur, tonomètre) :
un modèle et un « port », qui peut être un port COM, une box, ou un **dossier
d'échange**. `EnableComOrNetworkWidgetsAppareilRefraction()` fait apparaître les
widgets correspondants et écrit les clés `rufus.ini` du cas choisi en effaçant
celles de l'autre. `RecalcAvailablesPorts()` retire des autres listes le port COM
déjà pris — un port COM ne sert qu'un appareil.

### Reste de l'onglet

Dossiers de documents par appareil (une table par mode), marges d'imprimante avec
son bouton « Oups! » de remise à zéro, priorité d'import des documents (rafraîchie
toutes les 5 s par `VerifPosteImportDocs`), ville et code postal par défaut.

---

## 5. Onglet « Général » en détail

- **Appareils connectés au site** : ajout/suppression en base (`+` / `−`),
  immédiat.
- **Gestion des utilisateurs / lieux / motifs / banques** : ouvrent leurs fiches
  respectives. Après `dlg_gestionusers` en mode ADMIN, un message rappelle que les
  utilisateurs connectés devront relancer Rufus.
- **Mot de passe administrateur** : `ModifMDPAdmin()` (ancien + nouveau +
  confirmation), stocké en SHA1.
- **Sauvegarde / restauration** : jours, heure et dossier de la sauvegarde
  automatique, sauvegarde immédiate, restauration, réinitialisation. Tout écrit
  immédiatement en base et relance `proc->ParamAutoBackup()`. Le bloc est
  désactivé en accès distant (on ne sauvegarde pas une base qu'on ne voit pas en
  local). Le mécanisme de la sauvegarde lui-même est décrit dans `procedures.h`.
- **Emplacement du serveur** : le site où se trouve physiquement le serveur
  (`idlieupardefaut`).
- **Base des villes** : villes de la base ou liste personnalisée (essentielle hors
  de France).
- **Cotations françaises** et **mode comptabilité** : réglages de base, appliqués
  au prochain démarrage — d'où la confirmation, et le redémarrage immédiat pour la
  comptabilité.
- **Langue** : écrit à la fois `Param_Poste_Version` (rufus.ini) et la version en
  base ; effective au redémarrage.
- **Versions** affichées : Rufus, schéma de base, base IOL.

---

## 6. Quand les réglages sont-ils écrits ?

Deux régimes cohabitent, et c'est la principale chose à savoir avant de toucher à
cette fiche.

**Immédiat** — tout ce qui va en base (onglets Utilisateur et Général), plus les
dossiers de documents et les dossiers d'échange des instruments, écrits dans
`rufus.ini` dès qu'ils sont choisis.

**Différé** — le reste de l'onglet « Ce poste ». Toute modification d'un
`QLineEdit`, `QComboBox`, `QCheckBox`, `QRadioButton` ou `QSpinBox` de
`PosteParamtab` lève le drapeau `m_modifposte` (connexions posées en masse dans
`ConnectSignals()`). L'écriture n'a lieu que dans **`Valide_Modifications()`**,
appelée quand on referme le cadenas ou quand on ferme la fiche. Cette fonction
commence par vérifier :

- deux appareils sur le même port COM ;
- un appareil sans port, ou un port sans appareil ;
- un dossier d'échange inexistant.

En cas d'anomalie, elle bascule sur l'onglet fautif, y place le focus et renvoie
`false` — la fiche ne se ferme pas. Un mot de passe MySQL manquant, en revanche,
ne bloque **plus** : il peut manquer légitimement, on se contente d'informer et on
enregistre tout le reste.

À la fermeture (`FermepushButtonClicked`) : si `m_modifposte` est levé, on propose
d'enregistrer ; si on refuse et que la base des villes a été changée entretemps,
elle est remise dans son état d'origine. Puis `VerifDirStockageImagerie()` vérifie
qu'un dossier de stockage d'imagerie valide est renseigné dès qu'au moins un
appareil a un dossier de documents — sinon la fermeture est refusée.

La fiche ne fait **jamais** `accept()` : elle se ferme toujours par `reject()`.

---

## 7. Ce qui remonte à Rufus

Deux drapeaux seulement, lus par `Rufus::OuvrirParametres()` après l'`exec()` :

- `DataUserModifiees()` → le titre de la fenêtre principale est refait ;
- `CotationsModifiees()` → les cotations sont rechargées, et si l'utilisateur n'en
  a aucune, on retombe sur celles de son parent.

---

## 8. Pièges connus (déjà payés une fois)

- **Ne pas forcer la hauteur de `ParamConnexiontabWidget`** dans le constructeur :
  un `setFixedHeight()` écrase min *et* max du `.ui` et fait déborder le bas de
  l'onglet sur le cadre des instruments — à l'exécution seulement, Designer ne le
  montre pas.
- **Ne pas déplacer `PosteLayout` sur `PosteParamtab`** : la somme des hauteurs
  minimales (~770 px) ne tient pas dans la page d'onglet ; le layout doit rester
  sur son wrapper `ParamWidget`.
- **Combos de ports** : leur liste est reconstruite avec
  `clearItems(true)` + `insertItemsRespectCurrent()` pour ne pas émettre
  `currentTextChanged` et déclencher en cascade tout le paramétrage de l'appareil.
- **Modèle des cotations** : ne jamais le détruire pendant le traitement de son
  propre `itemChanged` — les rechargements sont différés par
  `QTimer::singleShot(0, …)`.
- **Régénération des clés SSL** : `proc->SuspendreTimersFond()` avant, sinon un
  timer tente une écriture SQL pendant le redémarrage de MySQL et fait surgir un
  « Enregistrement impossible » derrière la fiche, donc inaccessible.
