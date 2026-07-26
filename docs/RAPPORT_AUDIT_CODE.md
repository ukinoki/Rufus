# Rapport d'audit du code — faiblesses principales

> **But de ce fichier.** Recenser, en un seul endroit, les faiblesses relevées lors
> d'une revue systématique du code de Rufus (juillet 2026), classées par gravité et
> par thème, chacune avec sa **localisation**, son **effet concret** et une **piste de
> correction la plus simple possible**. Ce n'est pas une liste de reproches : le socle
> est sain pour un développement solo (voir la section *Points forts*). C'est une carte
> des points à surveiller, pensée pour un petit cabinet sur réseau local — pas pour une
> banque.
>
> **Méthode.** Le code a été passé au crible sur **7 axes** (robustesse, mémoire, SQL,
> multi-postes, architecture, maintenabilité, sécurité), puis les constats les plus
> lourds ont été **relus ligne à ligne** pour les confirmer. Les constats vérifiés de
> cette façon sont marqués **✓ vérifié**.
>
> **État du code audité** : sommet `RufusQt6` à `49439c5` (+ le correctif `ba1bf20`,
> voir *Déjà corrigé*). Les références `fichier:ligne` sont **indicatives** : elles
> bougent avec le code.
>
> **Gravité** : 🔴 haute · 🟠 moyenne · 🟡 basse — **relative au contexte réel**
> (petit cabinet, réseau local, un seul développeur).

---

## Synthèse — par où commencer

Le meilleur rapport bénéfice/effort, dans l'ordre :

| # | Action | Gravité | Effort | Où |
|---|--------|:---:|:---:|----|
| 1 | ~~Suppression en cours d'itération dans `CleanSalleDAttente`~~ **→ corrigé** | 🔴 | — | `rufus.cpp` |
| 2 | ~~Double `delete` dans `dlg_actesprecedents`~~ **→ corrigé** | 🔴 | — | `Dialogs/dlg_actesprecedents.cpp` |
| 3 | ~~**Faire remonter les erreurs SQL à l'écran**~~ **→ corrigé** (rend visibles ~750 requêtes muettes) | 🔴 | — | `Database/database.cpp:116` |
| 4 | `split().at()` non gardés sur les noms de fichiers d'imagerie | 🔴 | 1 helper | `importdocsexternes.cpp` |
| 5 | Verrou de dossier sauté dans le cas le plus fréquent | 🔴 | déplacer un bloc | `rufus.cpp:7667` |
| 6 | Copie d'image non vérifiée avant suppression de l'original | 🟠 | `bool` de retour | `utils.cpp:1183` |
| 7 | `commit()` jamais contrôlé | 🟠 | `bool` de retour | `Database/database.cpp:404` |
| 8 | Suppression d'un dossier patient sans transaction | 🔴 | copier un patron | `ItemsLists/cls_patients.cpp:169` |
| 9 | Espace manquant → verrou compta jamais posé (tiers payants) | 🔴 | 1 caractère | `Dialogs/dlg_paiementtiers.cpp:2093` |
| 10 | Sauvegarde du dossier médical en clair | 🔴 | archive chiffrée | `procedures.cpp:868` |

---

## Déjà corrigé

- ✓ **Suppression en cours d'itération dans `CleanSalleDAttente`** — commit `ba1bf20`.
  Le retrait d'un patient dont la fiche n'existe plus appelait `SupprimePatientEnCours`
  en pleine itération de la QMap parcourue (retrait de l'entrée + `delete`, puis `++it`
  relisait un élément détruit → lecture après libération, plantage aléatoire déclenché
  par le timer `VerifVerrouDossier` toutes les 60 s, sans action utilisateur). Corrigé
  en différant le retrait après la boucle, sur le modèle de `listpatcrstoremove` déjà
  présent dans la même fonction.

- ✓ **Double `delete` dans `dlg_actesprecedents`** — le constructeur ne libère plus
  `m_actes`/`m_listepaiements` : seul le destructeur s'en charge, ce qui supprime la
  double libération sur le chemin « patient sans consultation ».

- ✓ **Erreurs SQL invisibles** — `DataBase::erreurRequete` affiche désormais à l'écran
  le libellé transmis par l'appelant (nom de la fonction), le détail complet (requête +
  erreur MySQL) restant dans le log. Les échecs d'écriture ne sont plus silencieux. Le
  code mort `if (a)` (avec `a` figé à `false`) a été retiré au passage.

---

## 1. Robustesse & perte de données

### 🔴 `split().at()` non gardés sur les noms de fichiers d'imagerie ✓ vérifié
- **Où** : `importdocsexternes.cpp:203-204` (`split("_").at(4)` / `.at(3)`), `:460`
  (`if (split("__").size()>0) { …at(1) }` — `split()` renvoie toujours ≥ 1, le garde est
  donc toujours vrai et `.at(1)` sort des bornes dès qu'il n'y a pas de `__`), `:452, 487, 494`.
- **Effet** : un `QList::at()` hors bornes **arrête brutalement** l'application (pas une
  boîte de dialogue). Le scan repasse **toutes les 2,5 s** dans le thread graphique
  (`rufus.cpp:6401`), donc Rufus replante 2,5 s après chaque redémarrage tant que le
  fichier fautif reste là.
- **Déclencheur** : un fichier `.jpg`/`.pdf` au nom non conforme dans le dossier d'échange
  (export manuel, capture d'écran, fichier temporaire du partage SMB, renommage).
- **Piste** : un helper unique `static QString champ(const QStringList &l, int i)` qui
  renvoie `""` si `i >= l.size()`, utilisé pour les ~15 accès ; quand le champ obligatoire
  (idPatient) est vide, passer par `EchecImport()` qui gère déjà le rejet proprement.

### 🔴 Observation perdue après un premier échec d'enregistrement
- **Où** : `rufus.cpp:6615` (enregistrement au `FocusOut` seulement si le texte diffère de
  `valeuravant`), `Components/uptextedit.cpp:169` (`setvaleuravant(toHtml())` au `FocusIn`),
  `ItemsLists/cls_itemslist.cpp:1572` (en cas d'échec, l'item mémoire est restauré mais rien
  n'est remis en file), `rufus.cpp:8218` (`FermeDossier` ne réenregistre aucun champ).
- **Effet** : après une micro-coupure, le médecin voit « Enregistrement impossible », clique
  OK, reclique dans le champ (→ `valeuravant` réaligné) puis en ressort : `valeuravant == toHtml()`
  → **aucune requête réémise**, à la fermeture non plus. **Toute l'observation est perdue**,
  sans second message.
- **Piste** : en cas d'échec dans `ItemsList::update()`, forcer `setmodified(true)` sur le
  widget émetteur (ou ne pas réaligner `valeuravant` tant qu'un échec est en attente), pour
  que le prochain `FocusOut` retente l'écriture.

### 🟠 Copie d'image non vérifiée avant suppression de l'original ✓ vérifié
- **Où** : `utils.cpp:1183` (`void Utils::copyWithPermissions(...)` — `file.copy(path)` dont le
  booléen est ignoré), `importdocsexternes.cpp:578` (copie, retour ignoré) puis `:585`
  (suppression de l'original + message de succès), `:714-715` (même schéma dans `EchecImport`,
  suppression **inconditionnelle**).
- **Effet** : si l'écriture échoue (disque plein, partage en lecture seule, destination déjà
  présente), la ligne est créée en base, l'original est effacé, et Rufus affiche « Enregistrement
  réussi ». **Le cliché est définitivement perdu**, découvert à la réouverture du dossier.
- **Piste** : faire renvoyer `bool` à `copyWithPermissions` (retour de `file.copy`), ne supprimer
  l'original que si la copie a réussi, sinon repasser par `EchecImport` et retirer la ligne insérée.

### 🟠 `commit()` jamais contrôlé ✓ vérifié
- **Où** : `Database/database.cpp:404` (`void DataBase::commit()` — les 3 requêtes `COMMIT` /
  `SET AUTOCOMMIT=1` / `UNLOCK TABLES` sont jetées ; idem `rollback()` `:412`),
  `Dialogs/dlg_paiementdirect.cpp:3255` (`db->commit(); return OK;` — dernière instruction).
- **Effet** : si la liaison tombe entre la dernière requête et le `COMMIT`, MySQL annule tout,
  mais `EnregistreRecette()` renvoie quand même `OK` : la boîte se ferme, le patient disparaît
  de l'écran, **rien n'a été écrit**. La recette du jour est fausse sans alerte.
- **Piste** : faire renvoyer `bool` à `commit()` et renvoyer `Impossible` au site d'appel si le
  commit échoue — le dialogue affiche alors déjà le bon message.

---

## 2. Mémoire & durée de vie des objets

### 🔴 Double `delete` dans `dlg_actesprecedents` ✓ vérifié
- **Où** : `Dialogs/dlg_actesprecedents.cpp:61-64` (le constructeur, branche « autre dossier »,
  libère `m_listepaiements` et `m_actes` sans les remettre à `Q_NULLPTR`), `:113-118` (le
  destructeur refait exactement la même chose sous la même condition `!m_iscurrentpatient`),
  déclenché par `rufus.cpp:3868` (`if (!Dlg_ActesPrecs->initOK()) { … delete Dlg_ActesPrecs; }`).
- **Effet** : **double libération → corruption du tas** : plantage immédiat ou, pire, différé et
  inexplicable. `m_actes`/`m_listepaiements` n'ont pas de parent Qt : rien ne rattrape.
- **Déclencheur** : clic droit → « Visualiser le dossier » sur un patient **sans consultation
  enregistrée** (cas prévu par l'auteur, le message existe).
- **Piste** : retirer les 4 lignes de nettoyage du constructeur — laisser le destructeur faire le
  ménage (il est déjà écrit pour ça), en lui ajoutant un test de nullité pour couvrir aussi le
  retour anticipé `pat == Q_NULLPTR` (`:30`).

### 🟠 `dlg_docsexternes` garde un pointeur que l'appelant détruit juste après `exec()`
- **Où** : `rufus.cpp:3890-3894` (`new DocsExternes` … `OuvrirDocsExternes(docs)` … `clearAll` +
  `delete docs`), `Dialogs/dlg_docsexternes.cpp:26-28` (mémorise le pointeur, `WA_DeleteOnClose`),
  `rufus.cpp:567-572` (`MAJDocsExternes` balaie les fiches sans le garde `isHidden()` présent en `:509`).
- **Effet** : `WA_DeleteOnClose` détruit la fiche **au tour de boucle suivant** ; entre le retour de
  `exec()` et cette destruction, la fiche est encore connectée au signal et au timer 10 s, mais son
  `m_docsexternes` pointe déjà dans la mémoire libérée → plantage rare, très dur à reproduire.
- **Cause de fond** : la convention de propriété **change selon l'appelant** (liste globale vivante
  vs liste jetable).
- **Piste** : ne pas mélanger les deux régimes — confier la liste à la fiche et la détruire depuis
  son destructeur seulement quand ce n'est pas la liste globale, comme le fait `dlg_actesprecedents`
  avec `m_iscurrentpatient`.

### 🟡 Listes `ItemsList` créées localement non libérées complètement
- **Où** : `rufus.cpp:2750-2762` (`acts = new Actes()` … sortie sans `delete acts` si aucun acte),
  `ItemsLists/cls_actes.h` (aucun destructeur → le `QMap` interne n'est jamais libéré). Le bon
  patron existe : `ItemsLists/cls_comptes.cpp:25-29` (`~Comptes() { clearAll(map_all); delete map_all; }`),
  présent dans seulement 3 des ~30 sous-classes.
- **Effet** : fuite réelle mais modeste (la plupart des listes sont des singletons de `Datas` qui
  vivent la session) ; surtout une **incohérence de convention** — on ne sait pas, en lisant une
  sous-classe, si elle nettoie derrière elle.
- **Piste** : mettre le `clearAll(map_…); delete map_…;` dans le destructeur de chaque sous-classe
  (copier `~Comptes()`), ce qui rend les `delete` des sites d'appel suffisants.

---

## 3. Base de données & SQL

### 🔴 Les échecs de requête SQL sont invisibles à l'écran — ✓ corrigé
- **Où** : `Database/database.cpp:116-131` (`erreurRequete` : `bool a = false; … if (a) { qDebug… }`
  — le `if(a)` est du code mort permanent, le seul canal restant est le fichier de log),
  `log.h:43-54` (n'écrit que dans un fichier, aucun `UpMessageBox`), `database.cpp:606-612`
  (`StandardSQL` renvoie bien un booléen, mais **~207 sites d'appel le jettent**, + 51
  `SupprRecordFromTable`, + 19 `UpdateTable*`/`InsertSQL*`).
- **Effet** : serveur qui tombe, table verrouillée, contrainte violée → l'écriture échoue et
  **l'interface se comporte comme si tout allait bien**. Cotation, paiement ou réfraction perdus
  en silence, découverts des semaines plus tard. **C'est la faiblesse qui rend toutes les autres
  indétectables à l'usage.**
- **Piste** : ajouter un `UpMessageBox::Watch()` dans `erreurRequete` juste après le `Logs::ERROR`
  — **une ligne** fait remonter à l'écran les ~750 requêtes du programme. Les appels vraiment « best
  effort » pourront ensuite passer par une variante `StandardSQLQuiet()`.

### 🔴 Suppression d'un dossier patient : 12 écritures, sans transaction ni vérification
- **Où** : `ItemsLists/cls_patients.cpp:169-192` (`SupprimePatient` enchaîne `DELETE`/`SupprRecordFromTable`
  sur 12 tables — actes, réfractions, biométries, tonométrie, etc. — sans `createtransaction`, sans
  retour testé), alors que l'infrastructure existe et est correcte (`database.cpp:390-417`).
- **Effet** : une coupure ou un verrou concurrent au milieu laisse un **dossier à moitié supprimé**
  (actes orphelins, ou patient sans ses données ophtalmo). Le réflexe transactionnel est acquis côté
  comptabilité (12 `createtransaction`, tous en compta), **pas** côté dossier médical.
- **Piste** : encadrer le corps de `SupprimePatient` par `createtransaction(...)` / `commit()` avec
  `rollback()` sur échec — le modèle est déjà écrit à l'identique dans `Dialogs/dlg_comptes.cpp:112-176`.

### ⓧ FAUX POSITIF écarté après vérification — « noms à apostrophe non échappés »
- Un premier passage avait signalé les dialogues d'identification (`dlg_identificationpatient`,
  `…corresp`, `…tiers`, `…manufacturer`) comme construisant leurs requêtes avec le nom brut, donc
  cassés par un « O'CONNOR ».
- **Relecture** : c'est faux. Le nom est **échappé à la capture**, avant toute concaténation —
  `dlg_identificationpatient.cpp:219-220` (`PatNom = Utils::correctquoteSQL(...)`) puis utilisé tel
  quel en `:274-275` ; idem `dlg_identificationcorresp.cpp:161-162`→`195`,
  `dlg_identificationtiers.cpp:121`→`138`, `dlg_identificationmanufacturer.cpp:164`→`182`.
  L'analyseur avait vu `+ PatNom +` sans voir que la variable contenait déjà la valeur échappée.
- **Conclusion** : aucun bug ici. `correctquoteSQL` est correctement appliqué (voir *Points forts*).
  Constat conservé, barré, pour mémoire — un rappel que tout constat doit être relu dans le code.

### 🟡 70 % du texte SQL vit hors de la couche `Database`
- **Où** : ~750 littéraux SQL au total, dont 225 dans `Database/database.cpp` et **~500 ailleurs**
  (rufus.cpp 134, dlg_paiementdirect 49, procedures 45, dlg_paiementtiers 45, dlg_remisecheques 30,
  dlg_gestionusers 21). Schémas recopiés :
  `ItemsLists/cls_correspondants.cpp:130-133` (4 lignes quasi identiques).
  *(Le comptage initial incluait `conversionbase.cpp` (63) — fichier désormais obsolète, exclu.)*
- **Effet** : un changement de schéma (colonne ajoutée, table renommée) oblige à fouiller une vingtaine
  de fichiers. L'usage systématique des macros `TBL_*`/`CP_*` limite déjà les dégâts (d'où la gravité
  basse).
- **Piste** : poursuivre la démarche déjà engagée (commits `3948ad3`, `f208b8b`), en priorité sur les
  trois plus gros foyers qui manipulent l'argent (paiements, remises de chèques).

---

## 4. Multi-postes & concurrence

### 🔴 Le verrou de dossier est court-circuité dans le cas le plus fréquent ✓ vérifié
- **Où** : `rufus.cpp:7667` (`if (indexOf(tabDossier) > 0 && currentpatient() != nullptr)` → seul
  `AutorDepartConsult(true)` est appelé), `:7680-7696` (la boucle de vérification du verrou n'existe
  que dans le `else`), `:7698` (`AfficheDossier` appelé après, sans nouveau contrôle).
- **Effet** : le contrôle « ce dossier est-il déjà ouvert par un autre soignant ? » ne s'exécute que
  si **aucun** dossier n'est ouvert sur le poste. Or le déroulé normal est « je ferme A, j'ouvre B » :
  le contrôle est alors **entièrement sauté**. Deux soignants peuvent ouvrir le même dossier sans message.
- **Piste** : sortir le bloc de vérification du `else` pour l'exécuter systématiquement avant
  `AfficheDossier`. Pour le TOCTOU résiduel, poser le statut par un `UPDATE … WHERE …` conditionnel et
  ne continuer que si `numRowsAffected() == 1`.

### 🔴 Aucune détection de conflit à l'écriture : les champs sont réécrits en entier depuis l'écran
- **Où** : `ItemsLists/cls_itemslist.cpp:1570` (`update … set … = <valeur> where <clause>` — aucune
  clause de version ni de date de dernière modif), `rufus.cpp:6628, 6639, 10446` (tout le texte du
  widget est réécrit), `:6416` (le seul timer de rafraîchissement d'un dossier ouvert ne recharge que
  les docs externes ; le contenu médical n'est jamais relu tant que le dossier reste ouvert).
- **Effet** : le dernier à quitter un champ **écrase intégralement** ce que l'autre a écrit, sur la base
  d'un instantané qui peut dater de toute la consultation — sans avertissement. Combiné au constat
  précédent, deux postes sur le même dossier = **perte silencieuse de texte clinique**.
- **Piste** : conditionner l'écriture à la valeur lue à l'ouverture — `UPDATE … WHERE champ = <valeur
  lue à l'affichage>` (déjà mémorisée dans `valeuravant()`, `rufus.cpp:6616`) et, si 0 ligne affectée,
  prévenir au lieu d'écraser.

### 🟠 Verrous de comptabilité relâchés dès qu'un même login sert sur deux postes
- **Où** : `rufus.cpp:280` (au démarrage : `delete from verrouscomptaactes where PosePar = <moi>` —
  supprime **tous** les verrous de cet utilisateur, y compris ceux d'un autre poste),
  `Dialogs/dlg_paiementdirect.cpp:3298` (`NettoieVerrousCompta` : `… where PosePar = <moi> or PosePar
  is null`), alors que le code sait qu'un user peut être sur plusieurs postes
  (`cls_postesconnectes.cpp:124-131`).
- **Effet** : verrou granulaire **par utilisateur, pas par poste**, alors que le partage d'un login
  « secrétariat » entre deux postes est la norme. La secrétaire B ferme son écran de paiement → les
  verrous de A sautent → les deux peuvent enregistrer un paiement sur le même acte.
- **Piste** : ajouter la colonne poste dans `verrouscomptaactes` et restreindre les `delete` à
  `PosePar = <moi> AND Poste = <ce poste>` ; clé primaire sur `idActe` pour que l'INSERT concurrent
  échoue au lieu de doubler.

### 🟠 Le nettoyage tourne en parallèle sur tous les postes et peut évincer un poste vivant
- **Où** : `rufus.cpp:211` (`t_timerVerifVerrou->start(60000)` sur chaque poste), `:6083` (poste déclaré
  mort au-delà de 240 s sans battement), `:6110-6115` (patient renvoyé « Arrivé » + verrous compta
  supprimés), `:229` (`t_timerSalDat->start(1000)` — un `select` par seconde et par poste).
- **Effet** : toute opération bloquante longue (sauvegarde, import volumineux, gel réseau) > 240 s fait
  considérer le poste comme planté par n'importe quel autre ; son patient en cours est renvoyé en salle
  d'attente et ses verrous sautent **alors que la consultation est ouverte**. Le coût du polling reste
  acceptable pour 3-5 postes ; c'est la **concurrence des écritures de nettoyage** qui pose problème.
- **Piste** : n'exécuter `VerifVerrouDossier`/`CleanSalleDAttente` que sur **un seul** poste (p. ex. le
  plus petit `idUser` connecté), et déplacer le battement de cœur hors du thread graphique — la classe
  `TimerController` (`timerthread.h`) fait déjà exactement cela pour la sauvegarde.

---

## 5. Architecture & dette conceptuelle

> Rien ici n'appelle une refonte, et l'auteur rembourse déjà cette dette (commits `3948ad3`,
> `f208b8b`). Le vrai risque n'est pas la qualité au quotidien mais la **transmission** du code.

### 🔴 `Rufus`, une fenêtre principale qui est en réalité toute l'application
- **Où** : `rufus.h:136` (199 membres/méthodes, 33 `#include <dlg_…>`), `rufus.cpp` (11 118 lignes,
  184 définitions, 950 `ui->`, 143 requêtes SQL), `rufus.cpp:2078` (`ExporteDocs`, 640 lignes),
  `:9525` (`Remplir_SalDat`, 503 lignes ; commentaire `:9535` : *« si vous trouvez plus simple, ne
  vous génez pas »*).
- **Effet** : un seul fichier concentre l'essentiel du métier (salle d'attente, compta, imagerie,
  TCP, impression, Vitale, réfraction…). Toute modification oblige à naviguer un fichier où interface,
  SQL, fichiers et réseau cohabitent. **Principale barrière d'entrée pour un repreneur.**
- **Piste** : pas de refonte. Extraire progressivement les blocs déjà autonomes et peu couplés à `ui->`
  (`ExporteDocs`/`ImportNouveauDocExterne` → `GestionDocsImagerie` ; `envoieTCPMessage`/`TraiteTCPMessage`
  → `MessagerieTCP`). Chaque extraction retire 500-800 lignes sans changer le comportement.

### 🔴 État global mutable via `Datas::I()` — 1 880 accès directs
- **Où** : `gbl_datas.h:68-109` (**40 pointeurs publics et nus**, sans accesseur ni `const`),
  `gbl_datas.cpp:30-71` (40 `new` jamais libérés), 1 880 `Datas::I()` dans 70 fichiers, `rufus.h:333-337`
  (état « courant » ambiant : `currentpatient()` etc.), `cls_patients.cpp:120` (177 appels `initListe*`,
  chacun une invalidation manuelle du cache).
- **Effet** : les fonctions ne déclarent pas ce sur quoi elles agissent (leurs vraies entrées sont
  `currentpatient()`/`currentacte()`, invisibles dans la signature). Corriger un bug oblige à reconstituer
  « qui a positionné le patient courant avant cet appel ». La cohérence des caches repose sur 177 appels
  manuels : en oublier un = affichage périmé silencieux. Rend tout test automatisé impraticable.
- **Piste** : ne pas supprimer le singleton. (1) passer les membres de `gbl_datas.h` en `private` avec
  accesseurs en lecture (empêche mécaniquement la réaffectation) ; (2) pour les nouvelles fonctions,
  passer `Patient*`/`Acte*` en paramètre plutôt que d'appeler `currentpatient()` dans le corps.

### 🔴 Aucune frontière entre couches : SQL et boîtes de dialogue partout
- **Où** : le SQL remonte dans l'UI (`dlg_paiementdirect.cpp:323`, 74 requêtes dans ce seul dialogue) ;
  l'UI redescend dans le modèle (`cls_patients.cpp:303`, `veriftelephone()` construit un dialogue modal
  complet) ; et jusque dans la base (`database.cpp:88, 314`) ; `cls_itemslist.h:10` inclut `upmessagebox.h`
  dans la classe de base de tous les modèles.
- **Effet** : aucun niveau où exécuter une opération métier sans qu'une fenêtre s'ouvre → aucun mode batch,
  aucun test, risque de blocage si une boîte part d'un timer de fond. La même règle métier est décrite
  moitié dans le modèle, moitié dans le dialogue.
- **Piste** : poursuivre la démarche engagée, avec une règle vérifiable : plus de `"SELECT`/`"INSERT` hors
  de `Database/`, plus de `UpMessageBox`/`UpDialog` dans `Database/` ni `ItemsLists/`. Les modèles renvoient
  un booléen ; l'appelant UI affiche. Premier candidat : `Patients::veriftelephone()`.

### 🟠 `Procedures`, le singleton fourre-tout, et sa contagion à la compilation
- **Où** : `procedures.h` (198 déclarations, 9 346 lignes, une section intitulée **« en vrac »** `:289`),
  `procedures.cpp:319` (`ab()` — outil de débogage figé dans l'API publique : *« jusque là, ça va »* /
  *« là, ça ne va pas »*), `:2560` (`RestaureBase(bool, bool, bool, …)`, 502 lignes, 3 booléens de mode).
  `procedures.h` est inclus par 30 fichiers et tire `QtSql`, `QSerialPort`, `QPrinter`, `nidek.h`, etc.
- **Effet** : destination par défaut de tout ce qui n'a pas trouvé sa place ; un dialogue de gestion des
  villes dépend, à la compilation, des protocoles Nidek/Topcon. Les signatures à 3 booléens sont illisibles
  au site d'appel pour une opération destructrice.
- **Piste** : découper selon les intitulés de sections déjà écrits par l'auteur (« les sauvegardes » +
  « première connexion » → `GestionBaseSysteme` ; ports série + protocoles → `AppareilsRefraction`).
  Supprimer `ab()` de l'en-tête public au passage.

---

## 6. Maintenabilité, duplication & outillage

### ⚖ Pas de test automatisé ; la CI ne compile qu'à la publication — *décision assumée*
- **Fait** : `grep QTest/QtTest/gtest` → **zéro résultat** sur 125 000 lignes ; le seul « test » est une
  procédure **manuelle** (`build_tools/test/recette_cas_limites_demarrage.md`).
  `.github/workflows/release.yml` se déclenche **uniquement sur tag `v*`** — pas sur `push`/`pull_request`.
- **Décision de l'auteur (juillet 2026)** : on **ne met pas** de compilation à chaque push. Trois OS qui
  recompilent de zéro (~40 min de machines) pour, l'immense majorité du temps, du Qt pur sans compilation
  conditionnelle, ne se justifie pas — coût énergétique disproportionné pour un cabinet, et l'auteur
  vérifie lui-même à chaque push, plus vite et sur davantage que « ça compile ». Conforme au §0 du
  CLAUDE.md (faire simple, pas une banque).
- **Tests** : même verdict, faible intérêt sur du code figé depuis des années (`utils.cpp` n'a pas bougé).
  Un test ne paie que sur du code qui **change** ou qu'on **redoute de toucher** — à réserver, le cas
  échéant, à une logique subtile *et* évolutive (un calcul de montant retravaillé), pas au socle stable.
- **Risque accepté, consigné** : une régression de compilation reste théoriquement possible entre deux
  releases ; elle est couverte par la vérification manuelle de l'auteur à chaque push.

### 🔴 Duplication massive entre les deux dialogues de paiement, avec divergences déjà buguées ✓ vérifié
- **Où** : `dlg_paiementdirect.cpp` (3 615 l.) et `dlg_paiementtiers.cpp` (3 082 l.) partagent **33
  méthodes de même nom** et **959 lignes strictement identiques**. Bug réel :
  `dlg_paiementtiers.cpp:2093` — `"select" CP_IDACTE_VERROUCOMPTA` (espace manquant) produit
  `selectidActe` → requête invalide → `m_ok` faux → **le verrou compta n'est jamais posé côté tiers
  payants**, alors qu'il fonctionne côté direct (`dlg_paiementdirect.cpp:1996`).
- **Effet** : tout correctif sur un dialogue doit être reporté à la main dans l'autre, rien ne le signale.
  Le bug ci-dessus prouve que le report n'a pas eu lieu : deux copies du même verrou, une seule marche.
- **Piste** : corriger l'espace manquant. Puis extraire les fonctions strictement identiques et sans état
  d'UI (`PoseVerrouCompta`, `RetireVerrouCompta`, `NettoieVerrousCompta`, `ConvertitDoubleMontant`) dans
  une classe commune ou dans `Database/` — mécanique, ~150 lignes en moins.

### 🔴 Fonctions et fichiers monolithiques — une fonction de 1 575 lignes à 245 branches
- **Où** : `ItemsLists/cls_itemslist.cpp:8` (`ItemsList::update`, **1 575 lignes**, **245** `if (field == …)`,
  27 pointeurs bruts déclarés d'entrée). **68 fonctions dépassent 200 lignes** (`Nidek::RegleRefracteurXML`
  873 l., `dlg_identificationiol` 837 l., `IOLs::ImportListeIOLS` 743 l., `ExporteDocs` 637 l.…).
  `macros.h` : 1 386 lignes pour **1 088 `#define`** (tout le schéma SQL en préprocesseur, invisible à l'IDE).
- **Effet** : impossible de se constituer une carte mentale progressive ; pour modifier la mise à jour d'un
  champ, il faut entrer dans une fonction qui traite 27 types à la fois. Aucun outil ne répond à « où cette
  table est-elle écrite ? » autrement que par recherche textuelle.
- **Piste** : ne pas toucher à `rufus.cpp` (trop risqué sans tests). Le découpage de **`ItemsList::update`**
  serait mécanique (chaque bloc `if (type == …)` → une méthode `updateXxx()`, par déplacement pur), mais
  **décision de l'auteur : on le laisse tel quel** — stable depuis des années, touché seulement pour ajouter
  un champ (« if it ain't broke… »). L'observation reste comme repère de lisibilité pour un repreneur.

### 🟠 Aiguillage matériel par comparaison de chaînes — une branche morte depuis l'origine ✓ vérifié
- **Où** : `Protocols/genericprotocol.h` (aucune méthode `virtual` — pas de polymorphisme entre `Nidek`,
  `Topcon`, `Tomey`, `ShinNippon`), d'où **105 comparaisons `name == "…"`** dans `procedures.cpp`.
  `procedures.cpp:6390, 6435` : `"TOMEY TAP-6000"` alors que la table (`assets/Fichiers/rufus.sql:90`) et le
  chemin de lecture (`:6041, 6999`) utilisent `"TAP-2000"` → **la branche de réglage de ce réfracteur est
  inatteignable**. Lambda `initvariables` recopiée à l'identique dans 3 fichiers.
- **Effet** : ajouter/renommer un appareil oblige à retrouver toutes les occurrences dans 3 fichiers, sans
  aide du compilateur ; une faute de frappe donne une branche morte silencieuse, indétectable pour qui n'a
  pas le matériel.
- **Piste** : corriger les deux `TAP-6000` → `TAP-2000`. Puis remplacer les 105 littéraux par des constantes
  `macros.h` : une faute de frappe redevient une erreur de compilation.

---

## 7. Sécurité & données médicales

> Jugée à l'échelle d'un cabinet sur réseau local. La promesse *« vos données ne quittent jamais votre
> cabinet »* est **globalement tenue dans le code** (aucun appel sortant ne transporte de donnée patient).
> Le point dur n'est pas réseau mais physique : la sauvegarde.

### 🔴 Les sauvegardes contiennent tout le dossier médical en clair ✓ vérifié
- **Où** : `procedures.cpp:868, 877` (`mysqldump … > rufus.sql` / `user.sql`, en clair), `:787, 801`
  (copie brute des images et vidéos), `:632` (destination libre : disque externe, NAS, dossier synchronisé).
  Recherche `encrypt|AES|gpg|7z|openssl enc` → **aucun chiffrement**.
- **Effet** : le scénario de fuite le plus probable pour un cabinet n'est pas l'intrusion réseau mais la
  **perte ou le vol du support de sauvegarde**. Un `.sql` en clair suffit à reconstituer nom, date de
  naissance, NIR, motifs, comptes rendus et images de toute la patientèle — sur un support qui, par
  construction, quitte le cabinet. C'est le trou dans la promesse.
- **Piste** : router la sortie de `mysqldump` dans une archive protégée par mot de passe
  (`| 7z a -si -p<mdp>` ou `openssl enc -aes-256-cbc`), avec un mot de passe de sauvegarde saisi une fois
  dans les paramètres et stocké comme le `.dbkey`. À défaut, un avertissement explicite.

### 🟠 Mot de passe admin « bob » par défaut, mots de passe users en SHA1 nu ✓ vérifié
- **Où** : `macros.h:940` (`#define MDP_ADMINISTRATEUR "bob"`), `database.cpp:3615` (repli permanent sur
  `SHA1("bob")`, jamais forcé à changer), `procedures.cpp:604` (c'est ce mot de passe qui protège
  sauvegarde/restauration), `utils.cpp:967` (`SHA1` sans sel ni itération), `database.cpp:1496` (branche
  « mot de passe en clair » toujours acceptée), `dlg_gestionusers.cpp:541` (5 caractères alphanumériques mini).
- **Effet** : menace **interne**. Tous les postes se connectent avec le même compte MySQL tout-puissant →
  n'importe qui peut lire la table des utilisateurs ; un SHA1 non salé de 5-15 caractères se casse en
  secondes. « bob », publié dans le code GPL, ouvre la restauration de base sur toute install non modifiée.
- **Piste** : (1) imposer le changement de `MDP_ADMINISTRATEUR` au premier démarrage ; (2) remplacer
  `calcSHA1` par un dérivé salé (`SHA256(sel + mdp)`, sel par utilisateur), SHA1 en repli le temps de migrer.

### 🟠 Le mot de passe MySQL aléatoire est réexposé en clair
- **Où** : `procedures.cpp:1121, 1143` (`-p<mdp>` sur la ligne de commande → visible dans `ps`),
  `:868` + `macros.h:1038` (écrit en clair dans `RufusScriptBackup.sh`, permissions par défaut),
  `database.cpp:94` (`InfosConnexionSQL()` l'affiche en clair dans une fenêtre),
  `MySQLInstaller/mysqlinstaller.cpp:745` (`.dbkey` écrit sans `setPermissions` restrictif).
- **Effet** : le bon travail de migration vers un mot de passe aléatoire par cabinet perd une partie de son
  bénéfice — le secret ressort à chaque sauvegarde/restauration. Sur un poste serveur partagé en SMB, cela
  suffit à se connecter à MySQL avec un client externe et contourner les droits applicatifs.
- **Piste** : remplacer `-p<mdp>` par un `--defaults-extra-file=` temporaire en 0600 supprimé après usage
  (règle d'un coup `ps`, script et historique shell) ; appliquer `QFile::setPermissions(…, ReadOwner|WriteOwner)`
  après écriture du `.dbkey`.

### 🟡 Requêtes en échec (avec données patients) journalisées en clair sur disque
- **Où** : `database.cpp:121` (`Logs::ERROR(… + requete)` — la requête complète, donc les `INSERT`/`UPDATE`
  d'identité), `log.h:45, 117` (fichier `~/Documents/Rufus/Logs`), `utils.cpp:1211`
  (`rendDossierAccessibleAuServeurSQL` applique 0777 aux dossiers d'imagerie).
- **Effet** : noms, dates de naissance et contenus finissent en texte brut dans `Documents`, hors du
  périmètre de la base et de ses contrôles d'accès, embarqués dans toute copie du profil. Risque limité
  (purge à 7 jours, seules les requêtes en échec), mais de la donnée de santé stockée sans nécessité.
- **Piste** : dans `erreurRequete`, ne journaliser que le verbe + la table (60 premiers caractères) et le
  code d'erreur ; réserver la requête complète à une variable d'environnement de débogage.

---

## Points forts (pour ne pas fausser le tableau)

- **Parenté Qt correcte** pour les widgets et dialogues — pas de fuite mémoire massive ; le socle
  `ItemsList` (`add`, `epurelist`, `Supprime`) est cohérent et documenté.
- **Transactions comptables correctes** et systématiquement appliquées côté argent.
- **Échappement SQL disponible et largement utilisé** (`correctquoteSQL`, 267 usages).
- **Aucune exfiltration** : les seuls appels sortants sont des téléchargements (cotations depuis GitHub) ;
  aucune donnée patient n'est émise. La promesse « les données ne quittent pas le cabinet » est tenue.
- **Migration réussie vers un mot de passe MySQL aléatoire par cabinet** (`QRandomGenerator::securelySeeded()`),
  documentée et menée sans casse — vrai travail de fond.
- **6 traductions complètes** (fr, en, es, pt, br, it, ~3 392 chaînes chacune) ; **API Qt obsolètes quasi
  absentes** (4 occurrences résiduelles).
- **La dette architecturale est déjà en cours de remboursement** par extractions successives (commits
  `3948ad3`, `f208b8b`).

---

## Plan d'action recommandé (ordre de priorité)

1. ~~**Les deux plantages certains**~~ — *fait* : `CleanSalleDAttente` et `dlg_actesprecedents`.
2. ~~**Rendre les erreurs SQL visibles**~~ — *fait* : le multiplicateur qui révèle tout le reste.
3. **Le verrou de dossier sauté** (déplacer un bloc hors du `else`).
4. **Les corrections d'un caractère à fort effet** : espace manquant `selectidActe`, `TAP-6000` → `TAP-2000`.
5. **La sauvegarde chiffrée** : le plus grand gain de sécurité pour le moindre effort.
6. **Au fil de l'eau** : transactions côté dossier médical, `bool` de retour sur `copy`/`commit`.
