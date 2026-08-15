# Mettre à jour la liste des implants et leurs images

IOLCon publie un export XML des implants, mais **sans les images** : elles ne sont
visibles que dans le tableau de son site. Le script `maj_images_iolcon.py` les
récupère et les injecte dans le XML, que Rufus sait désormais lire (balise `<Image>`
dans `<Lens>`, cf. `IOLs::ImportListeIOLS`).

Tout se passe sur le Mac. Compter une dizaine de minutes, l'essentiel en attente.

## 1. Préparer l'outil (une seule fois)

```
pip3 install Pillow
```

## 2. Récupérer le nouvel export XML

Le télécharger depuis IOLCon et le déposer dans `assets/`, sous son nom de version
(`IOLexport2.4.xml` par exemple).

## 3. Enregistrer la page du tableau

Sur le site IOLCon, ouvrir le tableau des implants — celui qui les liste tous, une
ligne par implant, avec la vignette dans la deuxième colonne. Puis, dans Safari :
**Fichier → Enregistrer sous → Format : Page web, complète**.

Le fichier `.html` obtenu suffit : il ne contient pas les images, mais il porte
l'adresse de chacune et l'identifiant IOLCon de l'implant à qui elle appartient.
C'est ce qui permet de les apparier sans se tromper.

## 4. Lancer le script

```
cd ~/Documents/GitHub/Rufus/assets/importation_iolcon
python3 maj_images_iolcon.py ~/Desktop/page_iolcon.html ../IOLexport2.4.xml
```

Il affiche le nombre d'implants, d'images injectées et d'implants restés sans image
(une cinquantaine n'en ont aucune sur le site), puis le poids final. Les images
téléchargées restent dans `cache_images_iolcon/` : une deuxième exécution ne
retélécharge rien. Relancer le script ne duplique jamais les balises `<Image>`.

Si tu vois défiler des lignes `echec:`, c'est en général que la racine du site a
changé — la préciser avec `-b https://…`.

## 5. Vérifier et publier

Le XML doit peser quelques mégaoctets, pas quelques dizaines : les images sont
ramenées à 240 px de côté et comprimées sous 8 Ko, la plus grande fenêtre
d'affichage de Rufus faisant 160×180 px. Ces deux seuils sont en tête du script
(`COTE_MAX`, `POIDS_MAX`).

Pousser ensuite le XML sur le dépôt **et** le déposer à l'adresse que Rufus
interroge (`LIEN_XML_IOLCONLASTVERSION` dans `macros.h`), sans quoi les postes ne
verront pas la mise à jour.

Ne pas verser `cache_images_iolcon/` au dépôt : il est reconstruit à la demande.
