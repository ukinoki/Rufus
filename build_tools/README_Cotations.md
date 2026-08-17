# `Cotations.xml` — mise à jour de la CCAM et de la NGAP

Référence du fichier XML que **Rufus consulte une fois par jour** pour mettre à
jour la table `ccam` et les cotations NGAP de la base.

- **Producteur** : le script [`genere_cotations_xml.py`](./genere_cotations_xml.py)
  (même dossier), à partir du fichier CCAM officiel de la CNAM (`.xls`).
- **Hébergement** : dépôt GitHub **`ukinoki/additional_files`**, à la racine.
  URL lue par le programme : macro `LIEN_XML_COTATIONS` (`macros.h`) →
  `https://raw.githubusercontent.com/ukinoki/additional_files/main/Cotations.xml`.
- **Consommateur** : `DataBase::verifMajCotations()` (`Database/database.cpp`),
  qui télécharge le fichier, compare `<CCAM><Version>` à `parametres()->versionCCAM()`
  et, si elle est supérieure, **reconstruit entièrement** la table `ccam` puis
  répercute les montants sur les cotations de type 1 (CCAM) et 2 (associations).
- **Copies de travail** : `assets/CotationsFrance/` du dépôt Rufus (le `.xls`
  source, le `Cotations.xml` produit, le PDF de la NGAP). Ce dossier n'est pas
  ce que lit le programme — il ne lit **que** le dépôt `additional_files`.

---

## Structure du fichier

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<Cotations>
	<CCAM>
		<Version>84</Version>                    <!-- version CCAM, un entier -->
	</CCAM>
	<NGAP>
		<Version>2026-07-01</Version>            <!-- date d'application, yyyy-MM-dd -->
		<AMY>
			<ValeurMetropole>2.60</ValeurMetropole>
			<ValeurDOM>2.72</ValeurDOM>
		</AMY>
	</NGAP>
	<Acte>                                       <!-- ~8 260 actes CCAM -->
		<code>BGQP002</code>
		<nom>Examen du fond d'œil par biomicroscopie avec verre de contact</nom>
		<optam>29.07</optam>                     <!-- secteur 1 / adhérent OPTAM -->
		<nonoptam>29.07</nonoptam>               <!-- hors secteur 1 -->
		<origin>ccam</origin>
	</Acte>
	<Acte>                                       <!-- 22 actes NGAP (AMY) -->
		<code>AMY8</code>
		<nom>Mesure de l'acuité visuelle et de la réfraction…</nom>
		<indice>8</indice>                       <!-- montant = indice x ValeurMetropole -->
		<origin>ngap</origin>
	</Acte>
</Cotations>
```

Les deux familles d'actes cohabitent dans la **même** balise `<Acte>` ; c'est
`<origin>` qui les distingue (`ccam` ou `ngap`). Les actes CCAM viennent en
premier, dans l'ordre du `.xls`, les NGAP à la fin.

---

## 1. Récupérer le fichier CCAM de la CNAM

Le site de l'Assurance Maladie publie à chaque version un classeur Excel du
type `CCAM_V84_VF.xls` (« CCAM descriptive à usage PMSI » / base des tarifs).
Le déposer dans `assets/CotationsFrance/` du dépôt Rufus, en gardant le nom
d'origine (il porte le numéro de version).

Le script n'attend rien de particulier de la mise en page : il repère la ligne
d'en-tête (`Code`, `Texte`, les 2 colonnes `Tarif…`) et le numéro de version
dans le titre. Les colonnes ont déjà bougé d'une version à l'autre (la V84 a
4 colonnes vides à gauche que la V83 n'avait pas), c'est prévu.

## 2. Régénérer le XML

```bash
pip install xlrd                          # une fois : lecture des .xls (BIFF)
cd <dépôt Rufus>
python3 build_tools/genere_cotations_xml.py \
        assets/CotationsFrance/CCAM_V84_VF.xls \
        assets/CotationsFrance/Cotations.xml
```

Le script réécrit le fichier passé en 2ᵉ argument (un 3ᵉ argument optionnel
permet d'écrire ailleurs et de comparer avant de remplacer). Il affiche en
fin de course la version et le nombre d'actes :

```
assets/CotationsFrance/Cotations.xml : CCAM version 84, 8261 actes CCAM + 22 actes NGAP
```

Ce qu'il fait :

- **partie CCAM** : reconstruite depuis le `.xls`. Une ligne est un acte si son
  code vaut 4 lettres + 3 chiffres (`^[A-Z]{4}[0-9]{3}$`) ; tout le reste
  (chapitres, notes, mentions `[F, P, S, U]`) est ignoré. Un tarif vide ou
  « Non pris en charge » devient `0.00` ;
- **partie NGAP** : **recopiée telle quelle** depuis le `Cotations.xml`
  existant — elle ne vient pas du `.xls` mais du PDF de la NGAP
  (`assets/CotationsFrance/NGAP-01012026.pdf`) et se modifie à la main.

> Vérification faite au moment de l'écrire : régénéré depuis le `.xls` de la
> V83, le script reproduisait à l'octet près le `Cotations.xml` de la V83.
> Régénérer la version précédente et la comparer à ce que le dépôt contient
> déjà est le contrôle à refaire après chaque évolution du script.

## 3. Relire le diff

`git diff` sur `assets/CotationsFrance/Cotations.xml` montre exactement ce que
la nouvelle version CCAM apporte (la V84 : 4 actes dentaires ajoutés, un
libellé corrigé, un tarif). Un diff énorme est un signal d'alarme : c'est plus
probablement la mise en page du `.xls` qui a changé qu'un bouleversement de la
CCAM.

## 4. Déposer sur GitHub

Le fichier que lit Rufus est celui du dépôt **`ukinoki/additional_files`**,
distinct du dépôt Rufus. Deux dépôts, donc deux dépôts à mettre à jour :

```bash
# a) le dépôt Rufus : le .xls source, le XML produit et, si besoin, le script
cd <dépôt Rufus>
git add assets/CotationsFrance/CCAM_V84_VF.xls assets/CotationsFrance/Cotations.xml
git commit -m "CCAM version 84"
git push -u origin RufusQt6

# b) le dépôt additional_files : c'est LUI que le programme télécharge
git clone https://github.com/ukinoki/additional_files
cp <dépôt Rufus>/assets/CotationsFrance/Cotations.xml additional_files/
cd additional_files
git commit -am "CCAM version 84 (applicable au 31/07/2026)"
git push -u origin main
```

Le dépôt `additional_files` ne contient que ce fichier, à sa racine, sur la
branche `main` — pas de sous-dossier, pas d'autre branche : l'URL de
`macros.h` est en dur.

Contrôle final, une fois poussé (le CDN de GitHub met parfois une minute) :

```bash
curl -s https://raw.githubusercontent.com/ukinoki/additional_files/main/Cotations.xml | head -5
```

La balise `<Version>` doit afficher le nouveau numéro. Côté poste utilisateur,
la mise à jour n'est visible qu'au **prochain jour** : `verifMajCotations()` ne
relit le fichier qu'une fois par jour (date mémorisée dans `rufus.ini`, clé
`…DateCotations`). Pour tester tout de suite, supprimer cette clé.

---

## Mettre à jour la NGAP (indépendant de la CCAM)

La NGAP ne suit pas le rythme de la CCAM. À la main, dans le XML :

- `<NGAP><Version>` : la date d'application (`yyyy-MM-dd`) ;
- `<ValeurMetropole>` / `<ValeurDOM>` : la valeur de la lettre-clé AMY ;
- les `<Acte>` en `origin=ngap` : `<indice>` porte le coefficient, le montant
  est calculé par Rufus (`indice x ValeurMetropole`).

`verifMajCotations()` applique la mise à jour NGAP si la date du fichier est
postérieure à celle en base **ou** si une valeur d'AMY diffère — une
revalorisation de l'AMY seule est donc prise en compte sans changer la date.
