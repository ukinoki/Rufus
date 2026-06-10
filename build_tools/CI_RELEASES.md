# Publication automatique des releases (GitHub Actions)

Le workflow [`.github/workflows/release.yml`](../.github/workflows/release.yml)
construit et publie les **trois paquets d'installation** de Rufus à chaque
release :

| OS | Paquet | Outil |
|---|---|---|
| Windows | `Rufus-<version>-Setup.exe` | `windeployqt` + **Inno Setup** (`build_tools/Windows/RufusVision.iss`) |
| Linux | `Rufus-<version>-x86_64.AppImage` (auto-installante) | `linuxdeployqt` + `appimagetool` + `build_tools/Linux/AppRun` |
| macOS | `Rufus-<version>.dmg` (**universel x86_64 + arm64**) | `macdeployqt` + `build_tools/macOS/create-rufus-dmg.sh` |

> **macOS — binaire universel obligatoire.** On ne fait PAS l'impasse sur les Macs
> Intel. Le job tourne sur `macos-14` (Apple Silicon, runners bien plus disponibles
> que `macos-13`) mais compile en **universel** via
> `qmake … "QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64"` (Qt macOS officiel est universel ;
> la tranche x86_64 est cross-compilée). Le `.dmg` obtenu tourne sur Intel **et**
> Apple Silicon. **macOS est aussi le plus simple à produire à la main** (`macdeployqt`
> + 2 lignes `codesign` via `CODESIGN_ID`, cf. `create-rufus-dmg.sh`) : si les runners
> macOS de la CI font défaut, le faire localement n'a rien de pénalisant.

## Version

La version **= la date de publication**, source unique : `rufus.cpp`
(`qApp->setApplicationVersion("JJ-MM-AAAA/n")`). Le workflow l'extrait
automatiquement et l'injecte dans les noms d'artefacts, le `.iss` et la Release.
**Avant une release**, mettre cette date à jour dans `rufus.cpp`.

## Déclenchement

- **Tag `v*`** (ex. `git tag v10-06-2026 && git push origin v10-06-2026`) →
  build **+ publication d'une Release GitHub** avec les 3 paquets attachés.
- **Bouton « Run workflow »** (onglet *Actions*) → build + artefacts
  téléchargeables, **sans** créer de Release (utile pour tester).

> ⚠️ Le `workflow_dispatch` (bouton manuel) n'apparaît dans l'UI *Actions* que si
> le fichier de workflow est présent sur la **branche par défaut** (`RufusQt6`).
> Il faut donc fusionner ce workflow dans `RufusQt6` pour disposer du bouton ; le
> déclenchement par **tag** fonctionne dès que le tag pointe sur un commit qui
> contient le workflow.

## L'AppImage Linux est un vrai installeur

Au premier double-clic sur `Rufus-*.AppImage`, l'`AppRun`
(`build_tools/Linux/AppRun`) propose d'**ajouter Rufus au menu d'Ubuntu** :
copie de l'AppImage dans `~/.local/bin`, création du `.desktop` et de l'icône.
Les lancements suivants (depuis le menu) démarrent directement l'application.
Comportement désactivable avec `RUFUS_NO_INSTALL=1`.

## Binaires non signés

Les paquets ne sont **pas signés** : à la première ouverture, l'utilisateur voit
un avertissement **SmartScreen** (Windows) ou **Gatekeeper** (macOS, clic droit →
*Ouvrir*) qu'il contourne. La signature Authenticode (Windows) et la
signature + notarisation Apple (macOS) pourront être ajoutées plus tard via des
*secrets* GitHub.

## Points à ajuster au premier run (CI à rôder)

Cette première version encode le process connu ; quelques détails ne peuvent se
valider que sur les *runners* réels :

1. **Pilote SQL MySQL de Qt** (`qsqlmysql`) sur les 3 OS : le point le plus
   sensible. Stratégie **A puis B**, sans aucun binaire committé.
   - **A (par défaut)** : on installe la **bibliothèque client** MariaDB/MySQL sur
     chaque runner (apt `libmariadb-dev`/`default-libmysqlclient-dev`, brew
     `mariadb-connector-c`, `choco`), et on laisse `windeployqt --sql` /
     `macdeployqt` / `linuxdeployqt -extra-plugins=sqldrivers` embarquer le plugin
     **fourni par le Qt officiel** (Qt 6.10 le livre généralement). À confirmer au
     1er run que le plugin est présent et se charge.
   - **B (repli, si A échoue)** : compiler le plugin contre le Qt installé —
     ```sh
     git clone --depth 1 --branch v$QT_VERSION https://code.qt.io/qt/qtbase.git
     qt-cmake -S qtbase/src/plugins/sqldrivers -B sqlbuild \
       -DMySQL_INCLUDE_DIR=<inc> -DMySQL_LIBRARY=<lib>
     cmake --build sqlbuild
     # copier le qsqlmysql produit dans <Qt>/plugins/sqldrivers/ avant le déploiement
     ```
     Insensible aux versions, se régénère tout seul. À activer si le 1er run montre
     l'absence du plugin.
2. **Nom du binaire produit** (Linux/macOS) : les scopes `OSX`/`LINUX` du `.pro`
   ne sont pas des scopes qmake standard ; le workflow localise donc le binaire /
   le `.app` dynamiquement. À confirmer.
3. **Client MySQL Windows** (`mysql.exe`, `mysqldump.exe`, `libmysql.dll`) :
   chemin d'installation `choco` à confirmer et à copier dans `Deploy\Applications`
   comme le fait `CreateDeploy.ps1`.
4. **Version de Qt** (`QT_VERSION`) : alignée sur **6.10.2** (version de
   compilation locale). À vérifier que `install-qt-action`/aqt la propose bien.
