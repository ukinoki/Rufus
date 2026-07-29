# Internationalisation — feuille de route

Trois chantiers, dans cet ordre : **cotations hors France**, **villes / codes postaux**, **sans compta**.
Branche de travail : `RufusQt6`.

---

## 0. Fait

- [x] `loadCotationsByUser` — 4 requêtes, une par type, chacune sur sa table de jointure
- [x] `cotations.MontantPratique` abandonnée, macro retirée ; le pratiqué ne vit plus que dans les jointures
- [x] `ActeCotationcomboBox` — `UpStandardItemModel`, la cotation en `rufusitem` de chaque item
- [x] `dlg_gestioncotations` — mode « autre » imposé et sélecteur masqué (modification **et** version internationale)
- [x] `loadMontantsPratiquesByUser` — hors France, seule la jointure des « autres » (type 4) est lue

**Rien de tout cela n'a été compilé ni exécuté** (pas de Qt dans le conteneur de développement).

---

## 1. Cotations hors France (`cotationsfrance() == false`)

### Bloquant, à faire en premier

- [ ] **`loadCotations()`** — `Database/database.cpp:2916` filtre `where Typeacte = 4`, or le type est
      porté par `Typecotation`. `Typeacte` est le code de l'acte : MySQL convertit `'BZLB001'` en 0, donc
      la map de référence ressort **vide** et tout le reste est invisible.

### Décision d'architecture à trancher avant d'attaquer le reste

- [ ] Masquer l'interface **ne suffit pas** : `secteurconventionnel()`, `isOPTAM()`, `numPS()`,
      `AMnumberforSite()` restent lus dans `procedures.cpp`, `cls_user.cpp`, `database.cpp`.
      Deux voies : **neutraliser à la source** (les getters rendent une valeur neutre hors France) ou
      **garder chaque point d'usage** (beaucoup plus de code). C'est le seul vrai choix du chantier.

### Points à traiter

- [ ] **Codes barres** — point de coupe unique : `User::mapBarCodes()`, `Items/cls_user.cpp:201`.
      Elle seule construit la map (`AM`, `RPPS`) ; tout l'aval (`TextPrinter/code128`,
      `Procedures::ImpressionDocument`, `dlg_docsexternes`) devient inerte si la map est vide.
- [x] **Carte Vitale** — bouton `VitaleupPushButton` masqué hors France (`rufus.cpp`, `InitWidgets()`).
      C'était la seule entrée : `SimulerLireCV` n'est câblé que par une ligne en commentaire
      (`rufus.cpp:439`). Le module (`lecteurvitale`, `fichevitale`) reste compilé mais inatteignable.
- [ ] **NNI** — déborde les deux sites déjà gardés : `Utils::rgx_NNI` (`utils.cpp:59`, format INSEE en
      dur), affichage, impression, colonne DSP.
- [ ] **CMU** — dispositif français, sert encore au calcul du montant : `rufus.cpp:4568`
      (`RetrouveMontantActe`), `rufus.cpp:7429-7434`.
- [ ] **`loadCotationsByUser`** n'a toujours aucun appelant — à brancher ou à retirer.

### Déjà conditionné (pour mémoire, 31 sites)

`dlg_gestionusers` (20 : RPPS, secteur, OPTAM) · `dlg_identificationpatient` (NNI) ·
`dlg_impressions` (ALD) · `dlg_programmationinterventions` (2) · `fichevitale` ·
`procedures.cpp:4228` (garde de la MAJ des cotations, couvre tout le bloc de démarrage)

### À confirmer, probablement à garder

Tiers payants (`TiersPayants`, `tiers.TiersPayant = 'O'`) : la notion d'organisme tiers existe hors de
France. Les « caisses » du code sont des encaissements de compta, pas des CPAM.

---

## 2. Villes et codes postaux

### Décision en attente

- [ ] **Source des données.** Une intégration par pays (WFS européens, type Cartociudad espagnol) ne
      couvre pas le Mexique ni le Chili. Alternative : **une seule source mondiale au format unique**
      (GeoNames, un fichier par pays, CC-BY, dumps gratuits sans compte).
      **Non vérifié** — le réseau du conteneur refuse le domaine. À contrôler depuis le poste :
      télécharger `MX.zip` et `CL.zip` et regarder la couverture réelle.

### Ensuite

- [ ] Paramètre **`Pays`** dans `ParametresSysteme` (comme `VillesFrance`, `Cotations`, `SansCompta`) :
      propriété du cabinet, donc de la base — la langue du poste reste dans `rufus.ini`.
      Changement de schéma → à noter dans `assets/Fichiers/REMEMBER_modif_base.md`.
- [ ] Choix du pays au **PremierDemarrage** et dans **`dlg_param`**.
- [ ] **`rgx_CP` dérivé du pays** — aujourd'hui `static const` figé au lancement sur
      `QLocale().territory()` avec un `if Madagascar` en dur (`utils.h:113`) : il ne peut pas suivre un
      choix fait dans la fiche paramètres.
- [ ] **Import** reconstruisant la table `villes` pour le pays choisi.

### Existant à réutiliser (ne pas réinventer)

`villesfrance()` · table `AutresVilles` · `loadAutresVilles()` · `Villes::CUSTOM`
(`ItemsLists/cls_villes.cpp:79`) · `villecpwidget.cpp:81` bascule déjà sur `villesfrance()`

---

## 3. Sans compta

- [ ] **Tout est à écrire.** Le stockage existe (`ParametresSysteme.SansCompta`, `setsanscompta()`,
      `sanscompta()`) mais **aucun endroit du code ne lit `sanscompta()`**.
- [ ] **À trancher d'abord** : `assets/Fichiers/rufus.sql:409` documente **quatre** états
      (0 avec compta / 1 sans compta ni cotation / 2 sans compta avec cotation / 3 avec compta sans
      cotation), alors que `ParametresSysteme` le stocke en **booléen**. La distinction compta/cotation
      est déjà perdue côté code — à régler avant d'écrire quoi que ce soit, sinon on le paiera partout.
- [ ] Voir aussi `User::STATUT_COMPTABLE::NoCompta`, qui existe par utilisateur.

---

## Décisions en attente (récapitulatif)

1. `loadCotations()` : je corrige `Typeacte` → `Typecotation` ?
2. Hors France : neutraliser à la source ou garder chaque point d'usage ?
3. Codes postaux : GeoNames ou intégration par pays ?
4. Sans compta : booléen ou les quatre états de la colonne ?
