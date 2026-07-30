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

### Décision d'architecture — TRANCHÉE : neutraliser à la source

Masquer la case dans l'interface **et** fixer la valeur au chargement, dans `database.cpp`. Tout le
reste en découle, sans garde à semer aux points d'usage. Appliqué à :

- [x] **CMU** et **ALD** du patient — `loadSocialDataPatient` : `cotationsfrance() && valeurBase`,
      donc false hors France. Neutralise du même coup le calcul du montant de `RetrouveMontantActe`
      et l'affichage du dossier.
- [x] **OPTAM** — `loadUserData` et `loadUsers` : `!cotationsfrance() || valeurBase`, donc forcé
      hors France (le conventionnel est alors le montant optam). À l'enregistrement
      (`dlg_gestionusers`), OPTAM vaut 1 pour tout soignant non médecin ; seul un médecin peut ne pas
      l'être, et seulement en France.

### Points à traiter

- [x] **Codes barres** — la map (`AM`, `RPPS`) n'est plus posée hors France. Garde aux 3 seuls
      `setmapBarcodes()` du programme (`procedures.cpp:1639`, `:1679`, `:1724`) ; `TextPrinter`
      n'imprime rien si la map est vide. Garde mise là et non dans `User::mapBarCodes()` :
      **aucune classe de `Items/` ne dépend de `DataBase`**, il ne faut pas ouvrir cette porte.
- [x] **Carte Vitale** — bouton `VitaleupPushButton` masqué hors France (`rufus.cpp`, `InitWidgets()`).
      C'était la seule entrée : `SimulerLireCV` n'est câblé que par une ligne en commentaire
      (`rufus.cpp:439`). Le module (`lecteurvitale`, `fichevitale`) reste compilé mais inatteignable.
- [ ] **NNI** — déborde les deux sites déjà gardés : `Utils::rgx_NNI` (`utils.cpp:59`, format INSEE en
      dur), affichage, impression, colonne DSP.
- [ ] **ALD** — spécificité française. Le décompte brut trompe : les points de **décision** sont déjà
      gardés, un seul trou reste.
    - déjà couvert : `rufus.cpp:8850` et `dlg_programmationinterventions.cpp:1689`
      (`ALD = … && cotationsfrance()`) · la case de `dlg_impressions.cpp:1999` et celle de
      `dlg_identificationpatient.cpp:152` ne sont visibles qu'en France · l'affichage ALD/CMU de
      `rufus.cpp:7429`
    - **à faire** : `dlg_docsexternes.cpp:597`, `bool ALD = docmt->isALD()` n'est pas gardé — un
      document externe marqué ALD, réimprimé hors France, repasse par l'en-tête bizone.
    - résidus inertes, à nettoyer seulement si on veut : l'ordonnance bizone entière reste dans le
      code mais devient inatteignable (`Ressources::BodyOrdoALD()`/`HeaderOrdoALD()`, `ALDHeader`,
      `TailleEnTeteALD` et son réglage `Param_Imprimante/TailleEnTeteALD`, `HTML_LARGEUR_ENTETE_*_ALD`,
      le paramètre `bool ALD` qui traverse toute la chaîne d'impression) ; le drapeau patient
      (`CP_ALD_DSP`, `Patient::isald()`) continue d'être écrit à false, case masquée.
    - à laisser : `conversionbase.cpp` (12 occurrences), migration des anciennes bases.
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
