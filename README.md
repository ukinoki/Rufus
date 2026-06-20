<img align=left src="https://www.rufusvision.org/uploads/9/8/0/3/98038824/rufus2_11.png" width='80' alt='Icon'>

Rufus est un logiciel médical libre et gratuit de gestion des activités de soin en ophtalmologie et en orthoptie, 
basé sur le modèle client-serveur et publié en open source sous licence GPL. Son développement est assuré sous Qt6 et la base de données fait appel à MySQL.

# Objectifs
Développer un outil logiciel d'activité de soins en santé oculaire, libre, utilisable par la communauté des acteurs et facile à faire évoluer par tous.
Fédérer une communauté et réaliser un développement collaboratif.

# Installation
Sous MacOs, télécharger le fichier rufus.dmg, l'ouvrir par un double clic et le déplacer à l'aide du finder, dans le dossier Applications de Macos.
Sous Linux, télécharger le fichier rufus.Appimage et copier le dans le dossier "/usr/bin". Effectuez dans le terminal un "chmod a+x" sur l'appimage afin de la rendre exécutable ou faire un clic droit sur le fichier, choisir Propriétés et, dans l'onglet Permissions, cocher la case autoriser l'éxécution du fichier comme un programme.

# Documentation
La documentation de référence est regroupée dans le dossier [`docs/`](docs/) :
- [`docs/INSTALLATION_ET_INITIALISATION_RUFUS.md`](docs/INSTALLATION_ET_INITIALISATION_RUFUS.md) — processus d'installation et d'initialisation de Rufus sur les 3 environnements (Windows, macOS, Linux) : pré-contrôle et installation MySQL, mot de passe sécurisé, connexions chiffrées (SSL), et tâches de l'installeur Linux (composants système, menus, raccourcis).
- [`docs/INITIALISATION_ET_SECURITE.md`](docs/INITIALISATION_ET_SECURITE.md) — principes de fonctionnement : démarrage de Rufus, création d'une base patients, modèle de sécurité des accès MySQL.
- [`docs/NOTES_INTEGRATION_MYSQLINSTALLER.md`](docs/NOTES_INTEGRATION_MYSQLINSTALLER.md) — notes d'intégration du module MySQLInstaller.

Voir aussi `CLAUDE.md` (socle de développement, à la racine) et le dossier `build_tools/` (publication des releases).

# Contact
Vous souhaitez participer au projet, vous avez des commentaires particuliers ou des suggestions ?
serge.laine2@sfr.fr
