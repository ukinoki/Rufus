# Créer l'AppImage d'installation de Rufus

Ce script **fabrique juste le fichier `.AppImage`**. Il n'installe rien sur ton
ordinateur.

---

## La toute première fois seulement

Ouvre un **Terminal**, colle cette ligne, tape Entrée (mot de passe demandé) :

```
sudo apt-get install -y build-essential libfuse2 libmariadb3 libmysqlclient21 libgl1-mesa-dev libxkbcommon-dev libpcsclite-dev
```

---

## Pour fabriquer l'AppImage

1. Ouvre un **Terminal**.
2. Tape `bash ` (le mot *bash* suivi d'un **espace**) — **ne tape pas Entrée**.
3. **Glisse le fichier `creer-appimage-installeur.sh`** depuis le gestionnaire de
   fichiers **dans le Terminal** : son chemin s'écrit tout seul.
4. Tape **Entrée**. Attends quelques minutes (ça compile).

---

## Où est mon AppImage ?

À la fin, le Terminal affiche :

```
  AppImage créée : /…/Rufus-JJ-MM-AAAA-x86_64.AppImage
```

C'est **ce fichier `Rufus-…-x86_64.AppImage`** (dans le dossier Rufus). C'est lui
que tu copies sur une clé / que tu installes sur un poste.

*(Pour installer Rufus sur un poste, ça se passe plus tard, en lançant cette
AppImage-là — ce n'est pas le rôle de ce script.)*
