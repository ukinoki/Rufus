Installation de la nouvelle version de Rufus — mot de passe MySQL sécurisé et version de la base

CE QUI CHANGE
1. La nouvelle version exige un serveur MySQL qui sait gérer le DOUBLE MOT DE PASSE.
   SEUIL DE VERSION (critère), figé dans macros.h et utilisé PARTOUT (runtime + installeurs) :
     - Windows / macOS : >= 8.4.3   (VERSION_MYSQL_MINI_WINMAC) — on y installe le 8.4 LTS ;
     - Linux / Ubuntu  : >= 8.0.14  (VERSION_MYSQL_MINI_LINUX)  — le 8.0 d'apt sait déjà le double mdp ;
     - MariaDB exclu dans tous les cas.
   Côté code, le seuil est résolu par MySQLInstaller::seuilVersionMySQL() (un seul point à changer).
   En pratique l'installeur pose du 8.4.9 (Oracle) sur macOS/Windows, et du 8.0.x (apt) sous Linux.
2. Elle remplace le mot de passe unique public (gaxt78iy) par un MOT DE PASSE ALÉATOIRE,
   propre au cabinet, pour les deux comptes MySQL de Rufus : adminrufus et adminrufusSSL.

OÙ EST STOCKÉ LE MOT DE PASSE ALÉATOIRE
- Dans un fichier caché PRIVÉ à chaque poste : ~/.rufus/.dbkey (jamais dans un dossier partagé).
- Un même poste peut viser plusieurs bases (une par mode) : le .dbkey a une ligne par mode,
  MONO= / LAN= / WAN=.
- Distribution du mot de passe d'un poste à l'autre : à l'ancienne (noté sur un papier) ou par CLÉ USB.
  On NE met PAS le mot de passe dans un dossier partagé réseau (lisible sur le WiFi du cabinet).

DEUX SITUATIONS APRÈS TÉLÉCHARGEMENT DE RUFUS
1. L'utilisateur crée une nouvelle base Rufus vierge (uniquement en monoposte).
2. L'utilisateur se connecte à une base existante (sur ce poste, ou sur un serveur réseau local/distant).


=====================================================================
LE FLUX
=====================================================================

1. PREMIÈRE TÂCHE — VÉRIFICATION DE LA VERSION DE MYSQL (faite par l'installeur)
   But : ne pas créer d'irréversibilité. Il ne faut pas écraser un Rufus qui marche si la migration
   risque de bloquer la base (serveur trop ancien pour le double mot de passe).

   a. Ne se fait QUE si un Rufus préexistant, ou une base Rufus, existe déjà sur ce poste.
   b. Si la base MySQL est SUR ce poste (rubrique BDD_POSTE de rufus.ini avec ACTIVE=YES)
      et que sa version ne remplit pas le critère :
        Message :
        " Ce poste héberge la base de données patients."
        " Cette nouvelle version de Rufus nécessite une mise à jour du serveur MySQL."
        " Pour cela, Rufus devra désinstaller MySQL puis réinstaller une version plus récente."
        " Rufus fera donc une sauvegarde de votre base patients pour pouvoir la restaurer ensuite."
        2 boutons : 1. Annuler   2. OK, continuer
   c. Si ce poste est un CLIENT réseau (BDD_LOCAL ou BDD_DISTANT avec ACTIVE=YES) :
      on installe sans rien signaler (ce n'est pas lui qui héberge la base).

   Note technique : l'installeur tourne en mode élevé (admin/root). « Le dossier personnel » n'y
   pointe donc PAS vers l'utilisateur connecté. Pour trouver le bon rufus.ini, il faut viser le home
   de l'UTILISATEUR DE LA SESSION (macOS : utilisateur de la console ; Windows : profil de l'utilisateur,
   pas de l'admin ; Linux : éviter /root sous sudo). Le souci n'est pas le droit de lecture (rufus.ini
   est lisible par tous), mais de viser le bon fichier.


2. LANCEMENT DU PROGRAMME APRÈS INSTALLATION

  I. IL Y A UN rufus.ini   (déroulé CHRONOLOGIQUE, AVANT l'écran d'identification)

     1. CONNEXION à la base selon rufus.ini.
        On essaie les mots de passe candidats : celui du .dbkey (du mode courant) PUIS gaxt78iy.
        Si aucun ne marche -> message + VerifParamConnexion (corriger les paramètres, réécrire rufus.ini).

     2. CONTRÔLE DE LA VERSION DE MYSQL — AVANT TOUTE AUTRE CHOSE.
        (SELECT VERSION() sur la connexion qu'on vient d'ouvrir.)
        - Version NON conforme (< 8.0.14, ou MariaDB) ET ce poste HÉBERGE la base (monoposte / BDD_POSTE) :
              -> on lance la PROCÉDURE DE MISE À JOUR DE LA BASE (voir plus bas), puis on relance Rufus.
        - Ce poste est un CLIENT réseau (BDD_LOCAL / BDD_DISTANT) : ce n'est pas lui qui gère le serveur,
              on n'agit pas (au pire un avertissement) ; c'est le poste serveur qui migrera.
        - Version conforme -> on continue.

     3. SÉCURISATION (structure de contrôle) — seulement sur une base à la version conforme.
        On regarde AVEC QUEL mot de passe la connexion a réussi, ET si adminrufus a déjà un 2e mot de passe :
           SELECT User_attributes->>'$.additional_password' IS NOT NULL
           FROM mysql.user WHERE User='adminrufus' AND Host='%';   -> 1 = base déjà sécurisée
          - Connecté avec le RANDOM (.dbkey) ............ rien à faire, ce poste a déjà le bon mot de passe.
          - Connecté avec GAXT78IY et PAS de 2e mot de passe .... la base n'est pas sécurisée :
                CE POSTE la sécurise (pose un random, garde gaxt78iy en 2e mot de passe avec RETAIN
                CURRENT PASSWORD), écrit son .dbkey, et invite fortement à NOTER/SAUVER le mot de passe.
          - Connecté avec GAXT78IY et un 2e mot de passe EXISTE .... base déjà sécurisée par un autre
                poste, mais celui-ci n'a pas le random : on DEMANDE le random (saisie ou clé USB).
        Cette structure évite toute « course » entre postes : dès qu'un poste a sécurisé, les autres
        voient « déjà sécurisé » et demandent le mot de passe au lieu d'en recréer un.

     4. DEADLINE DE SUPPRESSION DE GAXT78IY.
        Quand la base est sécurisée, deadline = 30 jours après la date de sécurisation, lue côté serveur :
           SELECT password_last_changed FROM mysql.user WHERE User='adminrufus' AND Host='%';
        À la deadline, si gaxt78iy existe encore -> DISCARD OLD PASSWORD (UNIQUEMENT depuis un poste qui
        détient le random, sinon il se couperait l'accès). Au-delà, un poste sans le random ne peut plus
        se connecter : il devra le récupérer.

     5. Si ce poste vient de sécuriser (il a créé le .dbkey), on invite fortement :
          - à noter/sauver le mot de passe en lieu sûr (papier ou clé USB) ;
          - si le serveur est en réseau : à mettre à jour Rufus sur TOUS les autres postes (l'ancienne
            version ne marchera plus après la deadline) et à leur fournir le mot de passe (papier/USB).

     6. ÉCRAN D'IDENTIFICATION (dlg_identificationuser), puis ouverture de la session.
        Le contrôle de la version de la BASE Rufus (majbase) se fait APRÈS l'identification.


  II. IL N'Y A PAS DE rufus.ini

     A. NOUVELLE BASE VIERGE — uniquement en monoposte
        1. Y a-t-il déjà un MySQL installé ?
           A. Oui :
              1. Y a-t-il déjà une base Rufus ? (on tente une connexion avec gaxt78iy)
                 a. Oui -> " Une base Rufus existe sur cet ordinateur. La conserver ou la supprimer ? "
                       Conserver  -> retour à PremierDemarrage()
                       Supprimer  -> on continue
                       Annuler    -> retour à PremierDemarrage()
                 b. Non -> on continue
              2. La version de MySQL convient-elle ?
                 a. Non -> message de confirmation avant de désinstaller MySQL et de réinstaller.
                 b. Oui -> on continue
           B. Pas de MySQL installé -> on continue
        2. Installation et configuration de MySQL, avec un mot de passe ALÉATOIRE
           (adminrufus + adminrufusSSL), gaxt78iy gardé en 2e mot de passe.
        3. On écrit le mot de passe aléatoire dans le .dbkey PRIVÉ (~/.rufus/.dbkey).
        4. Avant le message final, on invite fortement à noter/sauver le mot de passe (papier ou clé USB).

     B. CONNEXION À UNE BASE EXISTANTE
        On applique la même STRUCTURE DE CONTRÔLE qu'au point I :
        on regarde avec quel mot de passe on s'est connecté et s'il y a un 2e mot de passe, et :
           - random -> rien à faire ;
           - gaxt78iy sans 2e mdp -> ce poste sécurise (si la version convient), écrit son .dbkey,
             invite à noter/sauver et, en réseau, à mettre à jour + fournir le mot de passe aux autres ;
           - gaxt78iy avec 2e mdp -> demander le random (saisie ou clé USB).
        En monoposte, si la version de MySQL ne convient pas -> PROCÉDURE DE MISE À JOUR DE LA BASE.

        Cas particulier de l'ACCÈS DISTANT :
           - On se connecte avec gaxt78iy si on n'a pas encore le random ; ce n'est pas un gros risque
             car la connexion distante exige déjà les clés SSL.
           - Le poste distant garde son mot de passe dans SON ~/.rufus/.dbkey (il ne va jamais le lire
             ailleurs).
           - Le poste distant ne CRÉE PAS le mot de passe aléatoire (il n'est pas serveur).
           - Tant qu'il n'a pas le random, il invite régulièrement l'utilisateur à le récupérer, en
             rappelant la deadline (gaxt78iy sera supprimé).


=====================================================================
PROCÉDURE DE MISE À JOUR DE LA BASE (version trop ancienne)
=====================================================================
   Message :
   " Cette nouvelle version de Rufus nécessite une mise à jour de la base de données patients MySQL."
   " Pour cela, Rufus doit désinstaller MySQL puis réinstaller une version plus récente."
   " Il faut donc sauvegarder votre base patients pour pouvoir la restaurer ensuite."
   " Voulez-vous faire cette sauvegarde, ou continuer ?"
   " Si vous continuez sans sauvegarde, vos données patients seront définitivement perdues."
      2 boutons :
        1. Faire la sauvegarde d'abord
        2. Ma sauvegarde est faite ET testée, on peut continuer

   Déroulé (avec garde-fous pour ne JAMAIS perdre les données) :
   1. Sauvegarde : se connecter avec adminrufus/gaxt78iy et appeler
      Procedures::Backup(m_parametres->dirbkup(), true, false, false, false)  -> on sauvegarde la base seule.
      (Nota : si le dossier de sauvegarde ne peut pas être créé, ouvrir une boîte pour en choisir un.)
   2. VÉRIFIER LE BACKUP AVANT TOUTE DESTRUCTION :
        - fichier non vide et terminé par « -- Dump completed » ;
        - présence des 4 bases (DB_RUFUS / DB_COMPTA / DB_IMAGES / DB_OPHTA) ;
        - vérifier qu'il reste assez d'espace disque ;
        - (idéal) compter les lignes des tables clés AVANT, pour comparer APRÈS restauration.
   3. NE PAS EFFACER l'ancien data dir : le DÉPLACER de côté et le garder.
   4. Désinstaller MySQL puis installer la nouvelle version.
   5. Restaurer la base depuis la sauvegarde.
      (Nota : ajouter à Procedures::RestaureBase() un paramètre indiquant le fichier de restauration,
       sans demander de choix à l'utilisateur.)
   6. VÉRIFIER la restauration (les tables/comptes attendus sont là ; comparer les comptages de lignes).
      - Si OK -> supprimer l'ancien data dir mis de côté ; créer/mettre à jour rufus.ini ;
        créer le mot de passe aléatoire (adminrufus + adminrufusSSL) en gardant gaxt78iy en 2e mot de passe ;
        écrire le .dbkey.
      - Si ÉCHEC -> ne rien supprimer : on dispose encore de la sauvegarde ET de l'ancien data dir.
   7. (Utile) tenir un petit fichier d'état (dump OK / désinstall OK / install OK / restore OK) pour
      savoir où on en était si Rufus est interrompu en cours de route.

   Après une réinstallation complète, adminrufus est recréé avec un NOUVEAU random + gaxt78iy en 2e mot
   de passe : les postes LAN/distants qui avaient l'ancien random retombent sur gaxt78iy, voient « base
   sécurisée par un autre poste » et redemandent le nouveau random (papier/USB) avant la deadline.
