# Fabriquer l'AppImage-installeur de Rufus (Linux) — mode d'emploi

> Objectif : produire **toi-même**, sur ta machine Ubuntu, l'AppImage qui installe
> Rufus — sans passer par GitHub. C'est exactement ce que fait le workflow
> automatique, mais en local et sous ton contrôle.

Tout tient en **une commande**. Le reste de ce document explique les prérequis et
ce qui se passe, pour un débutant.

---

## 1. Ce que tu obtiens

Un seul fichier : `Rufus-<date>-x86_64.AppImage`.

C'est une **AppImage auto-installante** : quand on la lance (double-clic) sur un
poste, elle :
1. contrôle et installe au besoin les composants système nécessaires (polices,
   multimédia, client MySQL, **et la couche PC/SC `pcscd`/`libccid` pour la carte
   Vitale**) ;
2. recopie Rufus et l'**inscrit dans les menus** d'Ubuntu, comme un vrai logiciel.

Le « cerveau » de cette auto-installation est le fichier **`AppRun`** (dans ce
dossier). Le script ci-dessous ne fait que **fusionner** cet `AppRun` avec Rufus
et empaqueter le tout.

---

## 2. Prérequis (à faire UNE fois)

1. **Qt 6.10.2** installé (la même version que pour compiler Rufus, via
   l'installeur Qt classique). Le script a besoin de `qmake`.

2. Quelques paquets système, pour compiler et pour fabriquer/lancer une AppImage :

   ```bash
   sudo apt-get update
   sudo apt-get install -y build-essential libfuse2 \
     libmariadb3 libmysqlclient21 libgl1-mesa-dev libxkbcommon-dev
   ```

3. Un **accès Internet** au premier lancement : le script télécharge une fois pour
   toutes trois petits outils (`linuxdeploy`, son plugin Qt, `appimagetool`) et les
   met en cache dans `~/.cache/rufus-appimage-tools/`. Les fois suivantes, il les
   réutilise (pas de nouveau téléchargement).

---

## 3. Fabriquer l'AppImage

Depuis le dossier du dépôt (là où tu veux voir apparaître le fichier final) :

```bash
./build_tools/Linux/creer-appimage-installeur.sh
```

Si `qmake` n'est pas trouvé, ajoute Qt au PATH d'abord (adapte le chemin à ton
install) :

```bash
export PATH="$HOME/Qt/6.10.2/gcc_64/bin:$PATH"
./build_tools/Linux/creer-appimage-installeur.sh
```

Le script affiche chaque étape. À la fin :

```
  Terminé : /…/Rufus-10-06-2026-x86_64.AppImage
```

### Variante : tu as déjà compilé Rufus dans Qt Creator

Pas besoin de **recompiler** : donne au script le chemin du binaire `Rufus` produit
par Qt Creator (il est dans ton dossier de build, du genre
`build-RufusQt6-Desktop_Qt_6_10_2-Release/Rufus`) :

```bash
./build_tools/Linux/creer-appimage-installeur.sh /chemin/vers/Rufus
```

> Même dans ce cas, **Qt doit rester accessible** (`qmake` dans le PATH) : il sert à
> embarquer Qt dans l'AppImage, pas seulement à compiler.

---

## 4. Installer Rufus à partir de l'AppImage produite

Rien de spécial : **lance l'AppImage** (double-clic dans le gestionnaire de
fichiers, ou dans un terminal `./Rufus-<date>-x86_64.AppImage`). Elle s'occupe de
tout (libs + menu). Après ça, Rufus apparaît dans les applications d'Ubuntu.

---

## 5. Que fait le script, en clair

1. **Compile** Rufus (`qmake` + `make`) dans un dossier de build à part
   (`build-appimage/`, jetable — supprimé et recréé à chaque exécution).
2. **Assemble** l'« AppDir » : le binaire, l'icône, le raccourci `.desktop`, les
   traductions `rufus_*.qm`.
3. **Embarque Qt** et le **pilote MySQL** avec `linuxdeploy` + son plugin Qt.
4. **Remplace** l'`AppRun` par défaut par le **nôtre** (celui qui fait
   l'auto-installation).
5. **Empaquette** avec `appimagetool` → `Rufus-<date>-x86_64.AppImage`.

> Le script **ne touche pas** à ton installation Qt : il écarte le temps du build
> les pilotes SQL inutiles, puis les **remet en place** en sortant (même en cas
> d'erreur).

---

## 6. Si ça coince

- **« qmake introuvable »** → ajoute Qt au PATH (voir §3) ou passe un binaire déjà
  compilé (§3, variante).
- **« AppImage anormalement petite »** → le bundling Qt a échoué : vérifie que Qt
  6.10.2 est bien installé et que le PATH pointe dessus.
- **L'AppImage ne se lance pas du tout** → il manque `libfuse2` sur le poste
  (`sudo apt-get install -y libfuse2`). C'est le seul prérequis qui ne peut pas
  s'auto-installer (c'est ce qui permet de lancer une AppImage).
- **Détail des opérations d'install** sur un poste → journal dans
  `~/.rufus/install-systeme.log`.

---

## 7. Le dossier `old/`

Il contient l'**ancien** procédé (`create-rufus-appimage.sh`, `install-rufus.sh`,
`howto.txt`, `rufus.desktop`). Il est **périmé** : il utilisait `linuxdeployqt` et
ne produisait **pas** une AppImage auto-installante. Ne t'en sers plus — il n'est
gardé que pour mémoire.
