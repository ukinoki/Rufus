# `rufuslastversion.xml` — notification de nouvelle version

Référence du fichier XML que **Rufus consulte à chaque ouverture** pour
prévenir l'utilisateur qu'une nouvelle version du logiciel est disponible.

- **Producteur** : fichier hébergé en ligne, à l'URL de la macro
  `LIEN_XML_RUFUSLASTVERSION` (`macros.h`).
  Variante de test : `LIEN_XML_RUFUSLASTVERSIONTEST`.
- **Consommateur** : `Rufus::VerifLastVersion()` (`rufus.cpp`), qui télécharge
  le fichier, lit la section correspondant à l'OS courant et, si la version
  locale est plus ancienne, affiche un message d'information.
- **Gabarit prêt à remplir** : [`RufusLastVersion.template.xml`](./RufusLastVersion.template.xml)
  (même dossier). Exemples réels archivés : `RufusLastVersion*.xml`.

> ⚠️ Ce fichier ne déclenche **aucune** mise à jour automatique : il est
> purement **informatif** et renvoie vers la page Téléchargements du site.

---

## Version du logiciel = date de sortie

La version de Rufus est sa **date de publication**, fixée dans `rufus.cpp` :

```cpp
qApp->setApplicationVersion("05-06-2026/1");   // jj-MM-aaaa / sous-révision
```

`VerifLastVersion()` ne compare que la **date** (la partie avant le `/`) à la
balise `<Date>` du XML. C'est indépendant de `VERSION_BASE` (la version du
*schéma* de la base de données), qui suit son propre cycle.

---

## Structure du fichier

```xml
<?xml version="1.0" encoding="UTF-16"?>
<?xml-stylesheet type="text/xsl" href="RKT_style.xsl"?>   <!-- cosmétique, optionnel -->
<Version>
    <macos>                                   <!-- une section par OS -->
        <Date>2025/03/21</Date>               <!-- format yyyy/MM/dd -->
        <Base>
            <VersionBase>82</VersionBase>     <!-- informatif : NON lu par le code -->
            <UPDBase>No</UPDBase>             <!-- "Yes"/"No" -->
            <CompatibleWithPrecedent>Yes</CompatibleWithPrecedent>  <!-- "Yes"/"No" -->
        </Base>
        <Comment>                             <!-- une sous-section par langue -->
            <FR>texte FR, HTML autorisé (ex. &lt;br/&gt;)</FR>
            <EN>texte EN</EN>
            <ES>texte ES</ES>
            <PT>texte PT (Portugal)</PT>
            <BR>texte BR (brésilien)</BR>
            <IT>texte IT (italien)</IT>
        </Comment>
    </macos>
    <windows> <!-- mêmes enfants : Date, Base, Comment --> </windows>
    <linux>   <!-- idem --> </linux>
</Version>
```

### `<Comment>` multilingue (FR / EN / ES / PT / BR / IT)

`VerifLastVersion()` affiche la sous-section qui correspond à la **langue du
poste** (`ParametresSysteme::version()` — celle qui a déjà chargé le `.qm`,
valeurs `FR`/`EN`/`ES`/`PT`/`BR`/`IT`). Repli automatique sur `FR`, puis `EN`,
si la langue du poste n'a pas de sous-section.

> **Compatibilité ascendante** : si `<Comment>` ne contient **aucune** de ces
> sous-sections (ancien format = texte libre directement dans `<Comment>`), ce
> texte brut est affiché tel quel. Inutile donc de migrer les anciens fichiers.

### Règles imposées par le parser (à respecter scrupuleusement)

| Élément | Contrainte | Détail |
|---|---|---|
| Racine | `<Version>` | `documentElement` du document |
| Sections OS | tags **exacts** `macos`, `windows`, `linux` | Rufus mappe `osx → macos`, `ubuntu`/`"" → linux`, puis ne lit QUE la section dont le tag == OS courant |
| `<Date>` | format **`yyyy/MM/dd`** | parsé par `QDate::fromString(...,"yyyy/MM/dd")` ; un format invalide ⇒ date nulle ⇒ **aucune alerte** |
| `<UPDBase>` | littéral **`Yes`** pour vrai | `text() == "Yes"` ; toute autre valeur = `No` |
| `<CompatibleWithPrecedent>` | littéral **`Yes`** pour vrai | idem |
| `<VersionBase>` | libre | **décoratif** : présent pour l'humain, jamais lu par le code |
| `<Comment>` | sous-sections `FR`/`EN`/`ES`/`PT`/`BR`/`IT` (ou texte brut) | la langue du poste est affichée ; repli `FR`→`EN` ; `<br/>` et HTML simples acceptés. Texte brut sans sous-section = ancien format, toujours géré |
| Encodage | **UTF-16** (avec BOM) | tous les fichiers de production le sont ; surtout pas UTF-8 |

---

## Message affiché à l'utilisateur

Si `Date(locale) < Date(XML)`, Rufus affiche un message dont le contenu dépend
des deux booléens :

| `UPDBase` | `CompatibleWithPrecedent` | Message |
|:---:|:---:|---|
| `No` | (ignoré) | « N'impose pas de mise à jour de la base, compatible avec la version précédente. » |
| `Yes` | `Yes` | « Impose une mise à jour de la base, mais les postes restés en version actuelle continuent de fonctionner. » |
| `Yes` | `No` | « Impose une mise à jour de la base ; **tous** les postes utilisant cette base devront aussi évoluer. » |

Le contenu de `<Comment>` est ajouté au message, suivi d'un lien vers
`www.rufusvision.org`.

---

## Procédure de publication d'une nouvelle version

1. Compiler/publier la nouvelle version du logiciel (binaires sur le site).
2. Éditer ce XML (à partir du gabarit) :
   - `<Date>` = date de sortie, format `yyyy/MM/dd`, **pour chaque OS** publié ;
   - `<UPDBase>` = `Yes` **uniquement** si la version embarque une nouvelle
     `VERSION_BASE` (nouveau `majbaseNN.sql`) ; sinon `No` ;
   - `<CompatibleWithPrecedent>` = `No` si les anciens postes ne peuvent plus
     travailler sur la base mise à jour, `Yes` sinon ;
   - `<Comment>` = description des évolutions, **une sous-section par langue**
     (`<FR>`, `<EN>`, `<ES>`, `<PT>`, `<BR>`, `<IT>`) ;
   - (`<VersionBase>` : mettre le numéro courant pour la lisibilité, sans effet).
3. **Enregistrer en UTF-16** puis **ré-héberger** à l'URL `LIEN_XML_RUFUSLASTVERSION`.

> Conversion UTF-8 → UTF-16 (avec BOM) en ligne de commande :
> ```sh
> iconv -f UTF-8 -t UTF-16 source.xml > rufuslastversion.xml
> ```

---

## Note — lien avec la sécurité du mot de passe MySQL

Le champ `CompatibleWithPrecedent` est aussi le bon vecteur pour signaler, le
jour venu, une version qui **forcerait la rotation du mot de passe MySQL
aléatoire** en multi-poste : ce serait un cas `CompatibleWithPrecedent = No`
(les postes restés en ancienne version, donc sur l'ancien mot de passe,
seraient éjectés). La version qui **introduit** simplement `MySQLInstaller` +
le repli *legacy*, elle, reste `CompatibleWithPrecedent = Yes` (les bases
existantes continuent de fonctionner sans changement). Voir
`../docs/INITIALISATION_ET_SECURITE.md`.

---

## `RKT_style.xsl` (manquant)

Les fichiers déclarent une feuille de style `RKT_style.xsl` (instruction
`<?xml-stylesheet ?>`) servant uniquement à **afficher joliment le XML dans un
navigateur**. Elle n'est **pas** nécessaire au fonctionnement de Rufus (qui
parse le XML directement) et n'est pas présente dans le dépôt. Si besoin d'un
rendu navigateur, elle peut être recréée séparément ; son absence n'a aucun
impact fonctionnel.
