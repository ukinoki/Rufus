# Privacy Policy — Rufus

_Last updated: 2026-06-23_

Rufus is free and open-source software (GPLv3) for the management of care in
ophthalmology and orthoptics. This document describes how Rufus handles your data.

## Summary

**Your data never leaves your premises.** Rufus is a local, client–server
application. There is no cloud, no telemetry, no analytics, and no tracking. The
developer of Rufus receives **no** personal or patient data.

## Where your data is stored

- All patient and practice data is stored in a **MySQL database hosted by you** —
  on the same computer (single-workstation mode), on another computer of your local
  network, or on a remote server **that you own and control**.
- Imaging files are stored on **your** server's disk.
- Connections between workstations (local network or remote access) are
  **encrypted (SSL/TLS)** and go only to **your own** server. Rufus never routes
  your data through any third-party service.

## What Rufus does NOT do

- It does **not** send personal or patient data to the developer or to any third
  party.
- It contains **no** telemetry, usage analytics, advertising, or tracking.
- It does **not** monetise data, even "anonymised".

## The only outbound network connections Rufus makes

For transparency, Rufus connects to the internet in just two cases, and **neither
transmits any personal or patient data**:

1. **Update check** — at startup, Rufus reads a small version file on the project's
   website/repository to tell you whether a newer version exists. This is a plain
   download; nothing about you or your patients is sent.
2. **MySQL server download (first install only)** — when setting up the database
   engine, Rufus may download the MySQL server binaries from the project's GitHub
   release page. Again, this is a download only.

## Your responsibilities as the data host

Because you host the database, you are the data controller. You are responsible for
the physical and network security of your server, your backups, and access control
(Rufus protects database access with a strong, per-practice random password and
encrypted connections).

## Source code

Rufus is open source under the GNU General Public License v3. The full source code
is available at <https://github.com/ukinoki/Rufus>, and information and downloads at
<https://www.rufusvision.org>.

## Contact

For any question about this policy: serge.laine2@sfr.fr

---

# Politique de confidentialité — Rufus

_Dernière mise à jour : 23/06/2026_

Rufus est un logiciel libre et gratuit (GPLv3) de gestion des soins en ophtalmologie
et en orthoptie. Ce document décrit la manière dont Rufus traite vos données.

## En résumé

**Vos données ne quittent jamais votre cabinet.** Rufus est une application locale,
en architecture client-serveur. Aucun cloud, aucune télémétrie, aucune statistique
d'usage, aucun pistage. L'auteur de Rufus ne reçoit **aucune** donnée personnelle ou
patient.

## Où vos données sont stockées

- Toutes les données patients et du cabinet sont stockées dans une **base de données
  MySQL que vous hébergez** — sur le même ordinateur (mode monoposte), sur un autre
  poste de votre réseau local, ou sur un serveur distant **qui vous appartient et que
  vous contrôlez**.
- Les fichiers d'imagerie sont stockés sur le disque de **votre** serveur.
- Les connexions entre postes (réseau local ou accès distant) sont **chiffrées
  (SSL/TLS)** et ne vont que vers **votre propre** serveur. Rufus ne fait jamais
  transiter vos données par un service tiers.

## Ce que Rufus NE fait PAS

- Il n'envoie **aucune** donnée personnelle ou patient à l'auteur ni à un tiers.
- Il ne contient **aucune** télémétrie, statistique d'usage, publicité ou pistage.
- Il ne monétise **aucune** donnée, même « anonymisée ».

## Les seules connexions sortantes de Rufus

Par transparence, Rufus ne se connecte à internet que dans deux cas, et **aucun ne
transmet de donnée personnelle ou patient** :

1. **Vérification de mise à jour** — au démarrage, Rufus lit un petit fichier de
   version sur le site/dépôt du projet pour vous indiquer si une version plus récente
   existe. C'est un simple téléchargement ; rien vous concernant n'est envoyé.
2. **Téléchargement du serveur MySQL (à la première installation)** — lors de la mise
   en place du moteur de base de données, Rufus peut télécharger les binaires du
   serveur MySQL depuis la page de *releases* GitHub du projet. Là encore, il s'agit
   d'un téléchargement uniquement.

## Vos responsabilités en tant qu'hébergeur des données

Comme vous hébergez la base, vous en êtes le responsable de traitement. Vous êtes
responsable de la sécurité physique et réseau de votre serveur, de vos sauvegardes et
du contrôle des accès (Rufus protège l'accès à la base par un mot de passe aléatoire
fort, propre au cabinet, et par des connexions chiffrées).

## Code source

Rufus est un logiciel libre sous licence GNU General Public License v3. Le code source
complet est disponible sur <https://github.com/ukinoki/Rufus>, et les informations et
téléchargements sur <https://www.rufusvision.org>.

## Contact

Pour toute question sur cette politique : serge.laine2@sfr.fr
