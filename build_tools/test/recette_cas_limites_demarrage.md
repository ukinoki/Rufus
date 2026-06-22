# Recette : provoquer les cas limites du démarrage de Rufus (tests manuels)

> But : exercer à la main, sur UN seul poste, les branchements du flux de démarrage
> refondu (carrefour `RecupererDemarrage`, cohérence de base, récupération du mot de
> passe, clés SSL…) sans attendre d'avoir un vrai 2ᵉ poste réseau.
>
> Référence du flux : `docs/INSTALLATION_ET_INITIALISATION_RUFUS.md` et `initialisation Rufus.txt`.

---

## ⚠️ AVERTISSEMENT — DONNÉE PATIENT

1. **Ne JAMAIS faire ça sur la base de production.** Utiliser un poste/Mac de test, ou au minimum :
2. **Dump complet AVANT toute manip** (restaurable à la main) :
   ```sh
   MDP=$(grep '^MONO=' ~/.rufus/.dbkey | cut -d= -f2)   # mot de passe aléatoire du .dbkey (mode monoposte)
   /usr/local/mysql/bin/mysqldump -u adminrufus -p"$MDP" \
       --databases rufus comptabilite imagerie ophtalmologie > ~/rufus_TEST_dump.sql
   # (adapter les 4 noms de bases si besoin)
   ```
   Restauration : `… mysql -u adminrufus -p"$MDP" < ~/rufus_TEST_dump.sql`
3. Préférer **RENAME** à DROP (réversible).

### Fichiers clés (macOS)
| Fichier | Rôle |
|---|---|
| `~/Documents/Rufus/Rufus.ini` | configuration du poste |
| `~/.rufus/.dbkey` | mot de passe MySQL du cabinet (lignes `MONO=` / `LAN=` / `WAN=`) |
| `~/.rufus/.rufus.ini` | sauvegarde de Rufus.ini (écrite à chaque OUVERTURE réussie) |

Astuce : `MDP=$(grep '^MONO=' ~/.rufus/.dbkey | cut -d= -f2)` récupère le mot de passe courant.

---

## Cas sûrs / réversibles (à faire en premier — ils exercent le cœur du flux)

### A. rufus.ini absent → carrefour / premier démarrage
```sh
mv ~/Documents/Rufus/Rufus.ini ~/Documents/Rufus/Rufus.ini.SAVE
```
Attendu : au lancement, le **carrefour** `RecupererDemarrage` (Nouvelle base vierge / Reconstruire / Restaurer Rufus.ini / Quitter).
Restaurer : `mv ~/Documents/Rufus/Rufus.ini.SAVE ~/Documents/Rufus/Rufus.ini`

### B. rufus.ini présent mais invalide (aucun mode de connexion valable)
Dans `Rufus.ini`, passer `Active=YES` de `[BDD_POSTE]` à `NO` (ou casser le `Port`).
Attendu : carrefour **sans** le bouton « Nouvelle base vierge ». Remettre `YES` ensuite.

### C. Connexion impossible (serveur éteint / mauvais port)
```sh
sudo /usr/local/mysql/support-files/mysql.server stop
```
Attendu : « Connexion à la base impossible » → carrefour. Rallumer ensuite (`… start`).
(Variante : `Port=3308` dans Rufus.ini.)

### D. .dbkey périmé → invite SIMPLE de récupération du mot de passe
Il faut que les DEUX échouent (aléatoire ET gaxt78iy). On change le mdp serveur SANS garder
gaxt78iy, et on met un faux aléatoire dans .dbkey :
```sh
MDP=$(grep '^MONO=' ~/.rufus/.dbkey | cut -d= -f2)
/usr/local/mysql/bin/mysql -u adminrufus -p"$MDP" -e \
  "ALTER USER 'adminrufus'@'%' IDENTIFIED WITH mysql_native_password BY 'nouveau123';"
sed -i '' 's/^MONO=.*/MONO=fauxmdp/' ~/.rufus/.dbkey      # macOS sed (BSD)
```
Attendu : faux aléatoire refusé (→ .dbkey effacé), gaxt78iy refusé → **invite simple « donnez-moi le
mot de passe »** → saisir `nouveau123` → connexion + réécriture du .dbkey. Se remet d'aplomb tout seul.

### E. Base incohérente → message de cohérence (RÉVERSIBLE via RENAME)
```sh
MDP=$(grep '^MONO=' ~/.rufus/.dbkey | cut -d= -f2)
/usr/local/mysql/bin/mysql -u adminrufus -p"$MDP" -e \
  "RENAME TABLE rufus.utilisateurs TO rufus.utilisateurs_SAVE;"
```
Attendu : connexion OK mais sonde de cohérence (`SELECT 1 FROM rufus.utilisateurs`) échoue →
**« Base de données incohérente »** → (monoposte) carrefour avec **« Restaurer la base »**.
Restaurer : `… -e "RENAME TABLE rufus.utilisateurs_SAVE TO rufus.utilisateurs;"`

### F. Sauvegarde de Rufus.ini à l'OUVERTURE réussie
```sh
rm ~/.rufus/.rufus.ini
```
Lancer Rufus normalement (succès) → `ls -l ~/.rufus/.rufus.ini` : il **réapparaît** (horodatage frais).

### G. Bouton « Restaurer Rufus.ini » conditionné à l'existence du backup
- Backup présent (après F) + cas A → le bouton **apparaît**.
- `rm ~/.rufus/.rufus.ini` PUIS cas A → le bouton **n'apparaît pas**.

---

## Cas « distant » — durs à simuler sur un seul poste

### H. #2 — pré-contrôle des clés SSL (approximation)
Dans `Rufus.ini`, activer `[BDD_DISTANT]` (serveur même = `127.0.0.1`) et faire pointer
`Dossier_ClesSSL` vers un dossier VIDE.
Attendu : au démarrage, l'invite **« Indiquez le dossier des clés SSL »** puis, si rien,
« Clés SSL introuvables » → carrefour. (N'exerce que le BRANCHEMENT, pas une vraie connexion chiffrée.)

### I. #5 — rappel de récupération de l'aléatoire (poste distant)
Fiable seulement avec un VRAI 2ᵉ poste (mode distant + base sécurisée + sans l'aléatoire local).
Simulable en bricolant, mais fragile → à garder pour le jour où deux machines sont disponibles.

---

## Conseil
Commencer par **D, E, F, G** (monoposte, sûrs et réversibles) : ce sont eux qui exercent le
vrai cœur du nouveau flux. A, B, C sont triviaux. H, I concernent l'accès distant.
