# CLAUDE.md — socle de développement de Rufus

> Ce fichier est lu au démarrage de chaque session. Il fixe le **pourquoi** du
> projet (la vision qui guide les décisions) et le **comment** (les conventions
> concrètes du dépôt). Quand un choix se présente, il se tranche à la lumière de
> ce document.

---

## 0. RÈGLE N°1 — <ins>**FAIRE SIMPLE**</ins>

> ## <ins>**ON ESSAIE DE FAIRE DU SIMPLE.**</ins>

Et surtout : **quand on croit avoir trouvé, on regarde une deuxième fois en
prenant du recul.** On remet la solution dans son contexte réel — on est sur de
**petits cabinets médicaux**, pas une banque d'affaires, inutile de se blinder
comme si des millions étaient en jeu — et on se demande : *est-ce qu'on ne peut
pas faire plus simple ?*

Souvent la vraie réponse n'est pas de raffiner la solution compliquée, mais de
**changer de tuyau**. Exemple canonique : faire circuler les images par la
connexion MySQL déjà en place plutôt que de monter un VPN. La voie parallèle
simple bat la voie savante.

Contre-exemple à ne jamais reproduire : imposer `mysql_native_password` « par
sécurité » a coûté **trois jours** et cassé la sécurisation sur MySQL 8.4, là où
un simple `IDENTIFIED BY` (sans clause de plugin) faisait le travail. La clause
de trop est le défaut par défaut — on la traque.

**La complexité n'est pas de la rigueur, c'est une dette.** À qualité égale, le
code le plus court gagne toujours : moins de code = moins de risque.

### Fil-piège (règle *comptable*, pas une maxime)

Un principe ne se déclenche pas au bon moment ; un compteur, si. Donc, concret :

1. **Avant d'écrire une solution** : *« ça marche déjà quelque part dans le
   code ? »* Si oui → **on le copie**, on ne réinvente pas (ex. la fiche de
   progression : le multiviewer / la copie de fichiers l'affichaient déjà, il
   fallait partir de là, pas inventer trois variantes).
2. **2ᵉ essai raté sur le même point = STOP.** On n'écrit pas une 3ᵉ variante :
   on **revient à la dernière version qui marchait**, ou on copie l'analogue qui
   marche. Deux échecs, c'est comptable — contrairement à « faire simple ».
3. **« Ça marche » = observé, jamais supposé.** Tant que ce n'est pas constaté,
   c'est « pas encore vérifié », pas « fait ».
4. **Diagnostiquer = mesurer, pas conjecturer.** Devant un bug, ne pas raisonner
   par « ce qui se passe le plus souvent » : **modéliser** le mécanisme de bout en
   bout (quelle grandeur détermine physiquement le résultat ?) et **instrumenter**
   pour le constater (un `qDebug`, une trace, une valeur extraite du fichier
   produit). Deux hypothèses opposées se tranchent par **une** mesure, pas par
   une intuition. Cas d'école : la taille de la signature à l'impression — cinq
   suppositions successives sur le DPI/`QTextDocument` ratées, réglées d'un coup
   en lisant les matrices du PDF et un `qDebug` du rendu.
5. **UNE hypothèse, puis on trace.** Corollaire comptable du point 4, parce que
   « mesurer plutôt que conjecturer » ne se déclenche jamais tout seul : on a
   droit à **une** hypothèse. Si elle ne tombe pas juste, on n'en propose pas une
   deuxième — on **pose les `qDebug`**. Lire le code plus longtemps ne remplace
   pas une trace : le code dit ce qu'il fait, pas ce qui s'est passé.
   Corollaire : **une absence n'est une preuve que si l'instrument était en
   place**. Avant de conclure « ce n'est pas ça » parce qu'une trace n'apparaît
   pas, vérifier qu'elle était bien dans le binaire exécuté et qu'elle aurait
   parlé.
   Cas d'école : le bouton qui relançait l'installation en boucle — quatre
   hypothèses (bouton par défaut, clic égaré, fonction ré-entrée, lambda
   pendante) toutes fausses, et douze `qDebug` ont nommé la cause en deux essais
   (Entrée déclenchait sur le relâchement, qui retombait sur la fiche du dessous).

> Cette règle est si souvent enfreinte qu'un *hook* (`.claude/settings.json`) la
> réinjecte à chaque tour : le document seul ne suffisait pas.

---

## 1. Ce qu'est Rufus

Logiciel médical **libre et gratuit** (GPL) de gestion des soins en
**ophtalmologie et orthoptie**, en architecture **client-serveur local**
(Qt + MySQL). Développé en **Qt6** (branche principale `RufusQt6`) ; l'ère Qt5
est révolue et archivée sous `archive/Qt5-master`.

Particularité fondatrice : **développé par un ophtalmologiste qui code**, et non
par une équipe d'informaticiens traduisant les demandes de praticiens.
La double compétence est la source de tous les avantages
du logiciel — et la grille de lecture de toutes ses décisions.

---

## 2. Philosophie produit (le socle — non négociable)

1. **Le métier d'abord, le normatif jamais en maître.** Rufus se consacre à
   l'acte de soin, pas à la conformité administrative changeante. L'affranchissement
   des normes locales permet d'éditer une version internationale du programme
   pour rester sur le cœur de métier.

2. **Local, jamais le cloud. Les données ne quittent pas le cabinet.** Aucune
   exfiltration vers des serveurs tiers, aucune monétisation possible de données patients
   (même « anonymisées »). C'est un **principe non négociable** et, accessoirement,
   la meilleure réponse RGPD qui soit : on est plus propre que bien des solutions
   « conformes » qui, elles, hébergent tout ailleurs. À formuler comme un
   argument, pas comme une contrainte : *« vos données ne quittent jamais votre
   cabinet »*.

3. **L'ergonomie pensée par l'utilisateur final.** Les bonnes idées ne se
   *demandent* pas dans un cahier des charges, elles se *ressentent* en
   consultation (ex. le multiviewer d'imagerie, l'export IOL). La boucle courte
   idée → code → usage réel sur ses propres patients → correction est l'avantage
   décisif. Le préserver.

4. **Cible : le praticien, pas l'établissement.** En France comme ailleurs, le
   soignant veut faire son métier sans surveillance et sans perdre de temps en
   saisie normative. Marché stratégique majeur : **les pays en développement**
   (pas de budget pour un EMR commercial, pas de norme bloquante, besoin réel) —
   déjà une traction organique en Afrique et en Amérique du Sud. Ce terrain
   compte **autant** que la France.

5. **Priorité produit n°1 : abaisser la barrière d'installation/configuration.**
   Le principal frein à l'adoption est l'install et la config du serveur. Tout
   ce qui rend l'installation « ça se fait tout seul » convertit directement les
   visiteurs en utilisateurs. C'est le levier le plus rentable. (Travaux récents
   en ce sens : installeur MySQL intégré, mot de passe sécurisé automatique, SSL
   transparent.)

---

## 3. Points d'attention techniques (dette & rigueur)

- **Un seul cerveau tient l'architecture.** Le risque réel n'est pas le marché,
  c'est la **dette conceptuelle** (« le plat de spaghetti »). Privilégier les
  flux **simples et lisibles** à l'exhaustivité des cas. Quand une logique
  devient un labyrinthe de cas particuliers, c'est un signal : simplifier plutôt
  qu'ajouter.
- Commenter le **pourquoi** d'un choix non évident (les commentaires existants du
  code le font déjà — garder ce style, en français).
- **Lisibilité d'abord.** Tout ce qui suit ne vise qu'un but : qu'on comprenne le
  code sans le dérouler en entier. Règle transverse sur la **forme** du commentaire :
  utiliser **`/*! … */`** (et `/*!< … */` en fin de ligne). Dans l'éditeur Qt, `//`
  et `/* */` **nus** s'affichent en **vert**, peu lisible ; `/*!` donne un **bleu
  foncé** bien plus lisible. Donc on **bannit `//` et `/*` nus** au profit de `/*!`.
- **Résumé de classe en tête de chaque `.h`.** Après les `#include` et avant la
  déclaration de la classe, un bloc `/*! … */` résume **ce que fait la classe** et
  **le rôle des fonctions principales**, pour saisir son **mécanisme** d'un coup
  d'œil (structures de données clés, enchaînement, interaction avec les appelants).
  Modèle : le bloc en tête de `Dialogs/dlg_impressions.h`. Forme exacte :
    - Ouvert par `/*!`, fermé par `*/` ; **pas de `*` de marge** en début de ligne
      (style Doxygen) — inutile et fatigant pour l'œil.
    - **Un `*` = un vrai item** (bullet), à son niveau d'indentation ; chaque **cran
      de hiérarchie = exactement UNE tabulation de plus**. Une ligne qui n'est que la
      *suite* d'un item est simplement **indentée sous son texte, sans `*`** (pas
      d'étoile orpheline qui ramène l'œil au début de ligne pour rien).
    - **Concis et précis** : les grandes lignes, pas des tartines interminables.
- **Getters / setters (et petits accesseurs triviaux) : définis *inline dans le
  `.h`***, corps en accolades sur la ligne (modèle `Items/cls_intervention.h`) —
  pour ne PAS encombrer le `.cpp` de définitions évidentes. Le `.cpp` ne contient
  que les fonctions qui font vraiment quelque chose.
### Commentaires : plafond DUR (règle comptable, comme le §0)

> **1 ligne.** Un commentaire interne tient sur **une** ligne. Un *brief* tient en
> **une phrase**. Deux lignes = exception qu'on doit pouvoir justifier. **Trois lignes
> et plus = interdit : on coupe, on ne « résume » pas.**

Ce n'est pas un idéal, c'est un compteur : avant de valider un bloc, on compte les
lignes. Corollaires, à appliquer sans discuter :

- Un commentaire qui **redit ce que le code dit déjà** se supprime, il ne se raccourcit pas.
- On garde **le pourquoi**, jamais le comment ni l'historique. Pas de récit du bug
  d'avant, pas de « INDISPENSABLE », pas de mise en garde à rallonge : le fait suffit.
- **Pas de MAJUSCULES d'insistance** ni de gras dans le code : si c'est important, c'est
  déjà écrit.
- Un pavé de 5 lignes n'est jamais « nécessaire » : c'est le signe que le code, lui,
  n'est pas assez clair — c'est le code qu'on corrige.

- **Documenter les fonctions — au plus léger.**
    - Le *brief* d'une fonction vit dans le **`.cpp`**, au-dessus de la définition,
      au **format Doxygen** (modèle `ItemsLists/cls_itemslist.h`) :
      ```
      /*!
       * \brief Classe::fonction
       * description concise de ce que fait la fonction
       * \param nom  rôle du paramètre
       */
      ```
    - **On ne met un brief que s'il AIDE** : si le rôle est évident au nom + à la
      signature (getter/setter, fonction ultra-courte), **pas de brief du tout**.
    - **Description : 1 phrase** en général ; 2-3 lignes **seulement si c'est
      indispensable** (fonction vraiment complexe).
    - Quand il y a un brief, **un `\param` par paramètre** (systématique).
    - Dans le `.h`, une fonction n'a un *brief* que si elle est **complexe**. Si elle
      est assez claire, un **commentaire d'une seule ligne** en forme `/*!< … */`,
      **sur la même ligne** que la déclaration, suffit.
    - Ces commentaires courts d'une ligne s'**alignent en colonne** d'une ligne à
      l'autre, à la manière de `Items/cls_acte.h` — mais en forme `/*!< … */`. On
      **cale toute la colonne** (nom **et** commentaire) sur la déclaration la plus
      large du groupe, avec autant de tabulations/espaces que nécessaire.
    - **Brief sur plusieurs lignes** (fonction complexe) : bloc `/*! … */` **au-dessus**
      de la déclaration, avec un `*` de marge **aligné** sur chaque ligne — ce `*` sert
      justement à **dégager le bloc** du code alentour. Un tel bloc en début de ligne se
      **sépare de la fonction précédente par une ligne vide** (sinon il colle et l'œil ne
      voit plus où commence quoi).
    - **Regroupement (`.h`)** : fonctions **groupées par rôle** (comme dans
      `mysqlinstaller.h`) et, **dans un même groupe, sans ligne vide** — les
      fonctions qui vont ensemble (getters/setters, surtout quand leur rôle est
      évident) restent collées, ce qui garde les colonnes de commentaires alignées.
    - **Espacement (`.cpp`)** : une **ligne vide après chaque fonction**.
- **À l'intérieur d'une fonction (`.cpp`), commentaires extrêmement brefs**, sauf
  quand un *pourquoi* non évident l'impose.
- **Aligner le code aux tabulations** — même finalité : la lisibilité (et, l'auteur
  l'assume, une manie personnelle assumée ; le résultat est réellement plus lisible).
    - **`.h`** : colonnes alignées d'une ligne à l'autre — **type**, **nom**,
      `= valeur`, puis commentaire `/*!< … */` — et membres **regroupés par rôle**
      sous un intitulé (`/*! les items */`, `/*! les widgets */`…). Modèle :
      `Dialogs/dlg_programmationinterventions.h`.
    - **`.cpp`** : **toujours au moins une tabulation entre l'objet et sa méthode**
      (`objet⇥->methode(...)`) — comme un espace entre le sujet et le verbe — et
      **aligner les `->`** d'une ligne à l'autre autant que possible ; **regrouper en
      blocs** les appels portant sur la même chose (un bloc par objet/but), séparés
      par une ligne vide.

---

## 4. Conventions concrètes du dépôt

- **Branche de travail** : `RufusQt6`. Commiter et pousser **directement dessus**.
  Messages de commit en **français**, clairs et descriptifs. **Ne pas laisser le
  travail sur les branches éphémères que les sessions web créent automatiquement**
  (du type `claude/...`) : elles sont jetables et réapparaissent toute seules ;
  tout doit atterrir sur `RufusQt6`. En fin de session, rapatrier les commits sur
  `RufusQt6` (rebase) et pousser là.
- **Version du logiciel = date de sortie**, fixée dans `rufus.cpp` :
  `qApp->setApplicationVersion("jj-MM-aaaa/révision")`. Indépendante de
  `VERSION_BASE` (schéma de la base, `macros.h`).
- **Modifs de schéma en attente** : les petits changements de base qui ne
  justifient pas à eux seuls un nouveau `majbase` sont notés dans
  `assets/Fichiers/REMEMBER_modif_base.md`. À vider dans le prochain
  `majbaseXX.sql` (avec bump de `VERSION_BASE`). Y jeter un œil avant de créer
  un majbase.
- **Langue du poste** : portée par `rufus.ini` (`Param_Poste_Version`), exposée
  via `ParametresSysteme::version()` → `"FR"`/`"EN"`/`"ES"`/`"PT"`/`"BR"`. C'est
  elle qui charge le `.qm`. **Ne pas l'écraser** avec la version (partagée) de la
  base. Quand on a besoin de la langue à l'exécution, elle est **déjà connue** :
  ne pas relire `rufus.ini` ni `QLocale`.
- **Traductions** disponibles (`rufus_*.ts`/`.qm`) : fr, en, es, pt (Portugal),
  br (brésilien), it (italien). Repli de langue par défaut : **FR**, puis EN
  (>90 % d'utilisateurs francophones).
- **Notification de nouvelle version** : `build_tools/RufusLastVersion.xml`
  (consulté à chaque ouverture par `Rufus::VerifLastVersion()`). Format **UTF-16
  avec BOM**, `<Comment>` avec une sous-section par langue. Détails et règles du
  parser dans `build_tools/README_RufusLastVersion.md`.

---

## 5. Posture attendue de l'assistant

- Être un **interlocuteur franc**, pas un béni-oui-oui : signaler les risques et
  les pendants d'une décision, même quand l'orientation est déjà prise — tout en
  respectant les choix produit du socle ci-dessus.
- Garder à l'esprit que ce projet dit **pourquoi** on code autant que **quoi**.
  Les décisions techniques servent la vision, pas l'inverse.
- **Adapter le registre à l'auteur.** L'auteur de Rufus est ophtalmologiste, pas
  informaticien, mais il maîtrise Qt (il a écrit l'essentiel du code) : les
  notions de classe, héritage, signal/slot, ou `ScrollBarAsNeeded` sont acquises
  et n'ont pas à être réexpliquées. En revanche, **bannir le jargon
  d'implémentation non défini** (« rasteriser », « transient », « idempotent »,
  « blast radius »…) : soit l'éviter, soit le définir en une demi-phrase à la
  première occurrence. Viser des explications **courtes, claires et sobres** —
  *ce qui se conçoit bien s'énonce brièvement*. Il doit toujours pouvoir
  comprendre ce qui est fait pour le corriger à sa main : une explication qu'il
  renonce à lire est une explication ratée.
