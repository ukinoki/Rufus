# CLAUDE.md — socle de développement de Rufus

> Ce fichier est lu au démarrage de chaque session. Il fixe le **pourquoi** du
> projet (la vision qui guide les décisions) et le **comment** (les conventions
> concrètes du dépôt). Quand un choix se présente, il se tranche à la lumière de
> ce document.

---

## 1. Ce qu'est Rufus

Logiciel médical **libre et gratuit** (GPL) de gestion des soins en
**ophtalmologie et orthoptie**, en architecture **client-serveur local**
(Qt + MySQL). Migration en cours **Qt5 → Qt6** (branche de travail `RufusQt6`).

Particularité fondatrice : **développé par un ophtalmologiste qui code**, et non
par une équipe d'informaticiens traduisant les demandes de praticiens. La même
personne tient le scalpel et le clavier. C'est la source de tous les avantages
du logiciel — et la grille de lecture de toutes ses décisions.

---

## 2. Philosophie produit (le socle — non négociable)

1. **Le métier d'abord, le normatif jamais en maître.** Rufus se consacre à
   l'acte de soin, pas à la conformité administrative changeante. Les éditeurs
   commerciaux s'épuisent à courir après des normes locales qui bougent sans
   cesse ; Rufus s'en affranchit délibérément pour rester sur son cœur de métier.

2. **Local, jamais le cloud. Les données ne quittent pas le cabinet.** Aucune
   exfiltration vers des serveurs tiers, aucune monétisation de données patients
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
- **Sécurité** : MySQL avec mot de passe aléatoire fort, connexions chiffrées
  SSL entre postes. Voir `docs/INITIALISATION_ET_SECURITE.md`.

---

## 4. Conventions concrètes du dépôt

- **Branche de travail** : `RufusQt6`. Commiter et pousser dessus. Messages de
  commit en **français**, clairs et descriptifs.
- **Version du logiciel = date de sortie**, fixée dans `rufus.cpp` :
  `qApp->setApplicationVersion("jj-MM-aaaa/révision")`. Indépendante de
  `VERSION_BASE` (schéma de la base, `macros.h`).
- **Langue du poste** : portée par `rufus.ini` (`Param_Poste_Version`), exposée
  via `ParametresSysteme::version()` → `"FR"`/`"EN"`/`"ES"`/`"PT"`/`"BR"`. C'est
  elle qui charge le `.qm`. **Ne pas l'écraser** avec la version (partagée) de la
  base. Quand on a besoin de la langue à l'exécution, elle est **déjà connue** :
  ne pas relire `rufus.ini` ni `QLocale`.
- **Traductions** disponibles (`rufus_*.ts`/`.qm`) : fr, en, es, pt (Portugal),
  br (brésilien), + ca, it, ro. Repli de langue par défaut : **FR**, puis EN
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
