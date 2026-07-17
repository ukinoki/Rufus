# Fabriquer l'AppImage-installeur de Rufus — mémo

Tout tient avec **3 fichiers de ce dossier** :

| Fichier | Rôle |
|---|---|
| **`AppRun`** | L'**installeur** : embarqué dans l'AppImage, il s'exécute au lancement et installe Rufus (libs nécessaires + raccourci dans les menus). |
| **`creer-appimage-installeur.sh`** | Le **fabricant** : compile Rufus, y fusionne l'`AppRun`, et produit l'AppImage. |
| **`LISEZMOI-creer-appimage.md`** | Ce mémo. |

Les outils (`linuxdeploy`, son plugin Qt, `appimagetool`) sont téléchargés
automatiquement au premier lancement et gardés en cache — rien à installer soi-même.

---

## Prérequis (une fois)

```bash
sudo apt-get install -y build-essential libfuse2 \
  libmariadb3 libmysqlclient21 libgl1-mesa-dev libxkbcommon-dev
```

Et **Qt accessible** (qmake dans le PATH). Si besoin :
```bash
export PATH="$HOME/Qt/6.10.2/gcc_64/bin:$PATH"     # adapte le chemin à ton install
```

## Fabriquer

```bash
./build_tools/Linux/creer-appimage-installeur.sh
```

À la fin : `Rufus-<date>-x86_64.AppImage` à la racine du dépôt.

> Déjà compilé dans Qt Creator ? Passe le binaire pour éviter de recompiler :
> `./build_tools/Linux/creer-appimage-installeur.sh /chemin/vers/Rufus`

## Installer sur un poste

Lance l'AppImage produite (double-clic, ou `./Rufus-<date>-x86_64.AppImage`).
Elle s'installe toute seule.

## Si ça coince

- **qmake introuvable** → `export PATH=...` (voir Prérequis).
- **L'AppImage ne démarre pas** → il manque `libfuse2` sur le poste
  (`sudo apt-get install -y libfuse2`) : c'est le seul prérequis qui ne peut pas
  s'auto-installer.
- Journal d'installation sur un poste : `~/.rufus/install-systeme.log`.

*(Le dossier `old/` contient l'ancien procédé, périmé — à ignorer.)*
