# Les cotations dans Rufus

Ce document résume le fonctionnement du système de cotations (facturation des
actes) tel qu'il a été refondu : **une seule table affichée**, **une fiche de
saisie**, et **4 catégories** d'actes, chacune adossée à sa propre table de
jointure.

---

## 1. Vue d'ensemble

- **Affichage** : `cotationsUpTableView` dans `dlg_param` — 3 colonnes : *Acte*,
  *Conventionnel*, *Pratiqué*. La case à cocher de chaque ligne indique que
  l'utilisateur courant **utilise** la cotation (il a une ligne de jointure).
- **Saisie** : `dlg_gestioncotations` (création / modification d'une cotation) ;
  `dlg_choixccam` (choix d'un acte dans la table officielle CCAM, appelée par la
  précédente).

---

## 2. Modèle de données

### Table `cotations` (PARTAGÉE entre tous les utilisateurs)

| Champ            | Rôle                                                            |
|------------------|-----------------------------------------------------------------|
| `Typeacte`       | le code de l'acte (clé « métier »)                              |
| `MontantOptam`   | conventionnel OPTAM ; pour un « autre » (4), c'est LE conventionnel |
| `MontantNonOptam`| conventionnel non OPTAM (null pour un « autre »)                |
| `MontantPratique`| hérité, peu utilisé — le pratiqué « qui compte » vit dans la jointure |
| `Typecotation`   | **1** CCAM, **2** association, **3** NGAP, **4** autre          |
| `Tip`            | libellé / description (multiligne)                             |

> `CCAM` et `idUser` sont conservés **uniquement** pour compatibilité avec les
> postes antérieurs au 15‑07‑2026. On ne s'en sert plus.

### Les 4 tables de jointure (personnalisation PAR utilisateur)

Toutes clées sur `idCotation` + `idUser`.

| Table                      | Montants stockés                          |
|----------------------------|-------------------------------------------|
| `jointuresccam`            | `MontantPratique`                         |
| `jointuresassociations`    | `MontantPratique`                         |
| `jointuresNGAP`            | *(aucun)* — voir NGAP ci-dessous          |
| `jointuresautrescotations` | `MontantConventionnel` **et** `MontantPratique` |

Le montant pratiqué d'un utilisateur pour une cotation se lit dans **sa** ligne
de jointure (`DataBase::loadMontantsPratiquesByUser` réunit les 4 tables ;
`Cotations::loadUserCotations` marque `used()` et affecte les montants).

---

## 3. Les 4 catégories

### 1 — CCAM
Un acte CCAM isolé. Le code est un **code CCAM** (`Utils::rgx_cotationCCAM` =
`[A-Z]{4}[0-9]{4}`), présent dans la table officielle `ccam`. Les montants
(OPTAM / non‑OPTAM) sont **repris de la CCAM** et non modifiables. Le pratiqué
par défaut = le conventionnel selon le secteur OPTAM du **parent** (titulaire).

### 2 — Association CCAM
**Deux** codes CCAM. Les montants sont **calculés** : *1er acte plein + moitié
du 2e* (idem OPTAM et non‑OPTAM). Le code est stocké « code1+code2 » ; à la
relecture on ré‑extrait les 2 codes par le motif CCAM (le séparateur d'origine
pouvant varier).

### 3 — NGAP
Actes en lettres‑clés (AMY…). **Le pratiqué vaut toujours le conventionnel** :
- `jointuresNGAP` n'a donc **pas** de `MontantPratique` ;
- dans la table, la case *Pratiqué* est **vide** et **non éditable** ;
- à la lecture, `loadUserCotations` fixe `montantpratique = montantOptam` pour un
  NGAP.

La seule action possible sur un NGAP est de le **mettre / retirer** de sa
jointure via la **case à cocher**.

### 4 — Autre
Tout le reste (ni CCAM, ni association, ni NGAP). **Entièrement saisi** par
l'utilisateur : conventionnel (rangé dans `cotations.MontantOptam` **et**
`jointuresautrescotations.MontantConventionnel`), pratiqué, libellé. Pas de
completer CCAM.

**Déduction du type** (à la migration v83, `exporteJointures`) : d'après le
`Typeacte` — présent dans `ccam` → 1 ; motif « code CCAM + suffixe » → 2 ;
commence par `AMY` → 3 ; sinon → 4.

---

## 4. Création (`dlg_gestioncotations`)

La table `cotations` étant **partagée**, on évite les doublons :

- **CCAM (1) / association (2)** : dédup par **code exact** (même type). Si l'acte
  existe déjà → on ne le duplique pas, on rafraîchit `MontantOptam` /
  `MontantNonOptam` ; sinon insertion. Puis la **jointure** du user est créée
  (ou son `MontantPratique` mis à jour).
- **Autre (4)** : dédup en comparant les codes **en MAJUSCULES** (codes libres,
  insensibles à la casse ; même type). Existant → maj `MontantOptam`
  (conventionnel) + `MontantPratique` ; sinon insertion. La jointure
  `jointuresautrescotations` est créée / mise à jour (ses **2 montants**).

Après validation, la table est rechargée (la ligne apparaît **cochée** via
`used()`) et **scrollée** sur la cotation.

---

## 5. Modification

Réservée au type **4** (« autre ») : un acte CCAM/NGAP a ses montants figés et ne
se modifie pas. La fiche s'ouvre case « autre » cochée, les 2 cases CCAM
désactivées ; les montants sont chargés depuis **la jointure du user**.
L'enregistrement fait un **update par `idcotation`** : la cotation partagée
(code + conventionnel + libellé) et la jointure du user (ses 2 montants).

---

## 6. Table des cotations dans `dlg_param`

- **Boutons** (`+` / `modifier` / `-`) et **menu clic droit** : actifs **seulement
  si l'utilisateur est son propre parent** (`idparent() == id()`). Un remplaçant
  ne modifie pas les cotations du parent qu'il remplace.
  - `+` : nouvelle cotation ;
  - `modifier` : type **4** seulement ;
  - `-` : type **1, 2 ou 4** (pas NGAP) et si **aucun autre** utilisateur ne
    l'utilise → retire la cotation de sa jointure **et** de `cotations`.
- **Montant pratiqué éditable en place** (double‑clic sur la case ou entrée du
  menu), **sauf NGAP** : à la sortie de l'édition, `MontantPratique` est mis à
  jour dans la jointure du type.
- **NGAP** : seule action possible = **cocher / décocher** la case (mettre /
  retirer de la jointure).

---

## 7. Conventions de code

- **Macros de champs** : `CP_NOMCHAMP_NOMTABLE`, groupées par table dans
  `macros.h` (suffixes `JOINTCCAM`, `JOINTASSOCIATIONS`, `JOINTNGAP`,
  `JOINTAUTRESCOTATIONS`).
- **Regex CCAM centralisé** : `Utils::rgx_cotationCCAM` = `[A-Z]{4}[0-9]{4}`.

---

## 8. Reste à faire

- `dlg_param::MAJCotation` (handler de la **case à cocher**) est encore un *stub* :
  cocher / décocher ne crée / ne retire pas encore réellement la ligne de
  jointure. C'est le dernier maillon pour que la gestion par case à cocher (seul
  levier pour les NGAP) soit effective.
