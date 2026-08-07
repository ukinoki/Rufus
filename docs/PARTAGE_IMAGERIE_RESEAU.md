# Mettre les images de Rufus en réseau

Rufus range les images, vidéos et factures dans un dossier unique, sur le poste
qui héberge la base. Les autres postes du cabinet doivent **voir ce dossier
comme un dossier ordinaire** de leur propre ordinateur : Rufus ne parle pas le
langage du réseau, il ouvre des fichiers.

Toute l'opération consiste donc à *rattacher* le dossier du serveur au poste
client, puis à indiquer à Rufus où il est.

> **Faites-vous aider.** Ces manipulations sont les mêmes pour tous les
> logiciels qui partagent des fichiers, et n'importe quelle IA (ChatGPT, Claude,
> Gemini…) les connaît parfaitement. Copiez-lui le message d'erreur que vous
> obtenez, en précisant le système du serveur et celui du poste client : vous
> gagnerez beaucoup de temps.

---

## 1. Côté serveur : rien à faire

Rufus partage le dossier tout seul, à l'installation, et vérifie à **chaque
démarrage** qu'il l'est toujours. S'il ne l'est plus, il le signale et propose
de le rétablir.

Le nom sous lequel le dossier apparaît sur le réseau dépend du système :

| Système du serveur | Nom du partage  | Dossier réel                     |
|--------------------|-----------------|----------------------------------|
| Windows            | `RufusImagerie` | `C:\Users\Public\Rufus\Imagerie` |
| Linux (Ubuntu)     | `Rufus`         | `/Users/Shared/Rufus`            |
| macOS              | `Shared`        | `/Users/Shared/Rufus`            |

**Ce nom n'est pas celui du dossier.** Sur Windows en particulier, cherchez
`RufusImagerie` et non `Imagerie` : c'est la première cause de « je ne le trouve
pas sur le réseau ».

Il vous faut aussi **l'adresse du serveur** — celle-là même que les postes
clients utilisent déjà pour se connecter à la base (Édition → Paramètres →
onglet « Réseau local », ligne « Emplacement »). Dans les exemples ci-dessous
elle est notée `192.168.1.25`.

### Un compte pour ouvrir la session

Un serveur **Windows** refuse les connexions anonymes, quels que soient les
droits posés sur le partage. Les postes clients devront donc s'identifier avec
un compte Windows existant du serveur (son nom d'utilisateur et son mot de
passe de session). Les serveurs Linux et macOS installés par Rufus, eux,
acceptent l'accès invité : aucun identifiant n'est demandé.

---

## 2. Côté poste client

### Le poste client est sous Windows

1. Ouvrir l'Explorateur de fichiers.
2. Dans la barre d'adresse, taper `\\192.168.1.25\RufusImagerie` puis Entrée.
3. Saisir l'identifiant et le mot de passe du serveur si on les demande, en
   cochant « Mémoriser mes informations d'identification ».
4. Clic droit sur le dossier → **Connecter un lecteur réseau**, choisir une
   lettre (par exemple `Z:`) et cocher « Se reconnecter à l'ouverture de
   session ».

Le dossier à indiquer à Rufus sera alors `Z:\`.

### Le poste client est sous macOS

1. Dans le Finder : menu **Aller** → **Se connecter au serveur…** (`⌘K`).
2. Saisir `smb://192.168.1.25/RufusImagerie`, puis **Se connecter**.
3. Saisir l'identifiant et le mot de passe du serveur, en cochant « Conserver ce
   mot de passe dans mon trousseau ».
4. Pour que la connexion se refasse à chaque démarrage : **Réglages Système** →
   **Général** → **Ouverture** → `+`, et choisir le volume monté.

Le dossier à indiquer à Rufus sera `/Volumes/RufusImagerie`.

### Le poste client est sous Linux (Ubuntu)

Le gestionnaire de fichiers n'affiche pas toujours le serveur dans « Réseau » :
cette découverte automatique est peu fiable et son absence ne veut rien dire.
On monte donc le partage directement.

**a. Installer l'outil de montage**

```bash
sudo apt install -y cifs-utils
```

**b. Créer le point de montage**

```bash
sudo mkdir -p /mnt/rufusimagerie
```

**c. Enregistrer les identifiants du serveur** (inutile si le serveur est Linux
ou macOS)

```bash
sudo tee /etc/rufus-smb.cred > /dev/null <<'EOF'
username=NOM_DU_COMPTE_WINDOWS
password=SON_MOT_DE_PASSE
EOF
sudo chmod 600 /etc/rufus-smb.cred
```

Le `chmod 600` est important : il réserve la lecture de ce fichier à
l'administrateur du poste.

**d. Monter à chaque démarrage** — ajouter cette ligne à la fin de
`/etc/fstab` (`sudo nano /etc/fstab`), en remplaçant `1000` par ce que renvoie
`id -u` si ce n'est pas cette valeur :

```
//192.168.1.25/RufusImagerie /mnt/rufusimagerie cifs credentials=/etc/rufus-smb.cred,uid=1000,gid=1000,vers=3.0,_netdev,nofail 0 0
```

`_netdev` attend que le réseau soit disponible, `nofail` évite qu'un serveur
éteint empêche le poste de démarrer.

**e. Vérifier sans redémarrer**

```bash
sudo mount -a
ls /mnt/rufusimagerie
touch /mnt/rufusimagerie/essai && rm /mnt/rufusimagerie/essai && echo "écriture OK"
```

Le test d'écriture n'est pas facultatif : Rufus écrit dans ce dossier, un
montage en lecture seule ne suffit pas.

Le dossier à indiquer à Rufus sera `/mnt/rufusimagerie`.

---

## 3. Dans Rufus, sur le poste client

**Édition → Paramètres → onglet « Ce poste » → « Réseau local »**, ligne du
dossier de stockage des images : y porter le chemin obtenu ci-dessus (`Z:\`,
`/Volumes/RufusImagerie` ou `/mnt/rufusimagerie`).

Redémarrer Rufus, ouvrir un dossier patient contenant des images : elles
doivent s'afficher.

---

## En cas d'échec

Reprenez dans cet ordre — chaque étape ne se pose que si la précédente répond.

**Le poste client voit-il le serveur ?** Si Rufus s'y connecte déjà pour la base
de données, la réponse est oui : inutile de tester le réseau. Mais attention,
la base et le partage de fichiers empruntent deux portes différentes (3306 et
445) : l'une peut être ouverte et l'autre fermée par le pare-feu.

**Le partage répond-il ?** Sous Linux, `smbclient -L //192.168.1.25 -N` :

- pas de réponse du tout → le pare-feu du serveur bloque le partage de
  fichiers ;
- `NT_STATUS_ACCESS_DENIED` → le serveur répond mais refuse l'anonyme :
  recommencez avec `-U nom_du_compte`, il demandera le mot de passe ;
- la liste des partages s'affiche → le serveur est joignable, le problème est
  dans le montage.

**Le montage échoue-t-il ?** `mount` ne donne qu'un numéro d'erreur ; c'est le
système qui dit pourquoi :

```bash
sudo dmesg | tail -20
```

`STATUS_LOGON_FAILURE` désigne un identifiant ou un mot de passe erroné, et rien
d'autre.

**Les images ne s'affichent pas, sans message d'erreur ?** Vérifiez d'abord que
le montage est vivant, avant de soupçonner Rufus :

```bash
mount | grep rufusimagerie
```

Un dossier vide et un montage absent se ressemblent beaucoup.
