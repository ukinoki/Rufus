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
- **Résumé de classe en tête de chaque `.h`.** Après les `#include` et avant la
  déclaration de la classe, un bloc `/*! … */` résume **ce que fait la classe**
  et **le rôle des fonctions principales**, de façon à faire comprendre son
  **mécanisme** d'un coup d'œil (structures de données clés, enchaînement,
  interaction avec les appelants). Modèle à suivre : le bloc en tête de
  `Dialogs/dlg_impressions.h`. Forme exacte :
    - Bloc encadré par `/*!` et `*/`.
    - Chaque ligne commence par `* ` ; les **sous-chapitres** sont marqués par des
      **tabulations** (indentation croissante).
    - **Concis et précis** : les grandes lignes, pas des tartines interminables.
- **Dans le `.h`, ordonner et documenter les fonctions.**
    - Les **fonctions principales** sont précédées d'une **très courte** explication
      (mode *brief*).
    - Chaque fonction est **suivie d'une ligne vide**.
    - Les fonctions sont **groupées par rôle** (comme dans `mysqlinstaller.h`).
- **Sécurité** : MySQL avec mot de passe aléatoire fort, connexions chiffrées
  SSL entre postes. Voir `docs/INITIALISATION_ET_SECURITE.md`.

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
