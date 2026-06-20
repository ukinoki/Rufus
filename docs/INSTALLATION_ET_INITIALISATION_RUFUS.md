Installation et initialisation de Rufus — référence des 3 environnements (Windows, macOS, Linux)

Ce document décrit le PROCESSUS d'installation et d'initialisation de Rufus : pré-contrôle et
installation de MySQL, mot de passe sécurisé et version de la base, connexions chiffrées (SSL), et
— sous Linux — les tâches de l'AppRun (composants système, inscription dans les menus, raccourcis).
Pour les PRINCIPES de fonctionnement (démarrage, création de base, modèle de sécurité des accès),
voir le document voisin docs/INITIALISATION_ET_SECURITE.md.

CE QUI CHANGE
1. La nouvelle version exige un serveur MySQL qui sait gérer le DOUBLE MOT DE PASSE.
   SEUIL DE VERSION (critère), figé dans macros.h et utilisé PARTOUT (runtime + installeurs) :
     - COMMUN à tous les OS : >= 8.0.14 (VERSION_MYSQL_MINI) — première version qui gère le double
       mot de passe (RETAIN CURRENT PASSWORD). L'OS du serveur n'entre PAS en jeu : seule compte la
       version MySQL du serveur, pas la plateforme qui l'héberge ni celle du poste client.
     - MariaDB exclu dans tous les cas.
   Côté code, le seuil est résolu par MySQLInstaller::seuilVersionMySQL() (un seul point à changer).
   En pratique l'installeur POSE du 8.4.9 (Oracle, LTS) sur macOS/Windows et du 8.0.x (apt) sous Linux ;
   mais un serveur déjà >= 8.0.14 est accepté tel quel (aucune migration forcée).
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
   Avant l'installation de Rufus, un pré-programme "installeur", vérifie la version en cours de MySQL et prévient l'utilisateur 
   de la nécessité éventuelle d'une mise à jour de MySQL.
   But : ne pas mettre le poste serveur dans une situation où il serait bloqué plusieurs minutes, 
   cause des manoeuvres de sauvegarde/restauration nécessaires.
   
   a. Ne se fait QUE si un Rufus préexistant, ou une base Rufus, existe déjà sur ce poste.
   b. Si la base MySQL est SUR ce poste (rubrique BDD_POSTE de rufus.ini avec ACTIVE=YES)
      et que sa version ne remplit pas le critère :
        Message :
        " Ce poste héberge la base de données patients."
        " Cette nouvelle version de Rufus améliore la une sécurité de l'accès aux données du serveur
        " en mettant en place une gestion plus élaborée du mot de passe général." 
        " Elle nécessite une mise à jour du serveur MySQL."
        " Pour cela, Rufus devra désinstaller MySQL puis réinstaller une version plus récente."
        " Rufus fera donc une sauvegarde de votre base patients pour pouvoir la restaurer ensuite."
        " Cette installation peut-être longue et durer plusieurs minutes en fonction de la taille de votre base Rufus"
        IMPORTANT: s'il existe d'autres postes sur le réseau local qui utilisent rufus,
        ils devront IMPÉRATIVEMENT être mis à jour vers cette nouvelle version dans un délai d'un mois.
        Sans cette mise à jour, au-delà de ce délai, ils ne pourront plus utiliser Rufus.
        2 boutons : 1. Annuler, je ferais ça à un autre moment   2. OK, continuer
        Prévoir de lire la version de langue utilisée dans Rufus.ini et adapter ce texte à la langue utilisée.
        En cas de clic sur Annuler, l'installeur stoppe et Rufus n'est pas installé
   c. Si ce poste est un CLIENT réseau (BDD_LOCAL ou BDD_DISTANT avec ACTIVE=YES) :
      on installe sans rien signaler (ce n'est pas lui qui héberge la base).

   Note technique : l'installeur tourne en mode élevé (admin/root). « Le dossier personnel » n'y
   pointe donc PAS vers l'utilisateur connecté. Pour trouver le bon rufus.ini, il faut viser le home
   de l'UTILISATEUR DE LA SESSION (macOS : utilisateur de la console ; Windows : profil de l'utilisateur,
   pas de l'admin ; Linux : éviter /root sous sudo). Le souci n'est pas le droit de lecture (rufus.ini
   est lisible par tous), mais de viser le bon fichier.

   IMPLÉMENTÉ (macOS preinstall + Windows RufusVision.iss) : les deux installeurs lisent désormais
   rufus.ini pour déterminer le RÔLE du poste, et n'utilisent l'heuristique « mysqld local » qu'en
   REPLI (rufus.ini absent ou muet) :
     - macOS  : ~/Documents/Rufus/Rufus.ini via CONSOLE_HOME (utilisateur de la console) ;
     - Windows: %USERPROFILE%\Documents\Rufus\Rufus.ini ({userprofile} = utilisateur connecté).
     - BDD_POSTE Active=YES  -> SERVEUR : on contrôle la version locale ; si insuffisante, message 1.b
       (consentement) ; Annuler -> l'installeur stoppe, l'ancien Rufus reste en place.
     - BDD_LOCAL/BDD_DISTANT Active=YES -> CLIENT : on installe en silence (point 1.c), sans contrôle.
   Le rôle vient de rufus.ini (fait foi) ; mysqld --version ne sert qu'à lire la VERSION du socle.

   LANGUE DES MESSAGES (implémenté) : le dialogue de consentement 1.b est affiché dans la langue
   du poste, lue dans rufus.ini ([Param_Poste] Version, = Param_Poste_Version côté code). Langues
   traduites : fr, en, es, pt (br -> pt) ; repli fr pour toute autre valeur ou si absente.
   On ne traduit QUE ce dialogue : c'est le seul montré quand rufus.ini est lisible (donc quand la
   langue est connue). Les dialogues de REPLI (rôle INCONNU, c.-à-d. pas de rufus.ini exploitable)
   restent en français : sans rufus.ini, on n'a pas la langue. Le cas CLIENT, lui, n'affiche rien.
   macOS : le texte est passé en arguments à osascript (on run argv), ce qui lève la contrainte
   « pas d'apostrophe » de l'ancienne version. Windows : libellés de boutons localisés, le résultat
   reste lu par index (= 100), indépendant de la langue.


=====================================================================
1 bis. INSTALLEUR LINUX (AppImage / AppRun) — AU-DELÀ DU CONTRÔLE MYSQL
=====================================================================
   Sous Linux, l'« installeur » n'est pas un programme séparé : c'est l'AppRun embarqué dans
   l'AppImage (build_tools/Linux/AppRun). Au TOUT PREMIER lancement (double-clic sur le fichier
   .AppImage, AVANT que le poste ne soit installé), il joue une séquence d'installation complète ;
   ensuite, lancé depuis le menu, il démarre directement l'application.

   Comment il sait s'il doit (ré)installer : tant qu'il n'existe pas de raccourci de menu
   (~/.local/share/applications/rufus.desktop), il rejoue la séquence ; sa présence = « déjà
   installé » → lancement direct. (Désactivable en exportant RUFUS_NO_INSTALL=1.)

   Ordre de la séquence d'installation :
     0.  Centrage des fenêtres : zenity n'ayant pas d'option de position, on active le temps de
         l'install le réglage GNOME « center-new-windows », puis on REND à l'utilisateur son
         réglage d'origine (no-op hors GNOME). Toutes les fenêtres ci-dessous sont ainsi centrées.
     0b. CHOIX DE LA LANGUE (1re fenêtre) : Français / English / Español / Português, présélectionné
         d'après la locale du système, repli FR. Tous les textes de l'install sont ensuite localisés
         (table definir_textes : FR par défaut, surchargé par langue).
     1.  Configuration système (libs + dialout) — voir A ci-dessous.
     2.  Inscription dans les menus — voir B.
     3.  Fenêtre finale (raccourcis + lancement) — voir C.

   Pré-contrôle MySQL : avant tout cela, et UNIQUEMENT en cas de MISE À JOUR (un Rufus/base
   préexiste ET on lance une AppImage autre que celle déjà installée), l'AppRun applique la même
   règle qu'au point 1 (serveur local ≥ 8.0.14, MariaDB exclu ; client réseau → on laisse passer).
   Une installation neuve ne déclenche aucun contrôle.

   --------------------------------------------------------------
   A. CONFIGURATION SYSTÈME : LES COMPOSANTS QUE L'APPIMAGE NE PEUT PAS EMBARQUER
   --------------------------------------------------------------
   Certains composants ne peuvent pas être embarqués proprement dans l'AppImage et doivent être
   posés sur le système, via UN SEUL appel pkexec (apt), avec le mot de passe administrateur :

     - POLICES : gsfonts + ttf-mscorefonts-installer (l'EULA mscorefonts est pré-acceptée par
       debconf). Indispensables à l'affichage ET à l'impression correcte des documents.
     - LECTURE VIDÉO (GStreamer) : gstreamer1.0-plugins-base / -good / -bad / -ugly, -libav,
       -tools, -x, -alsa, -gl, -gtk3, -pulseaudio. Pour la relecture des vidéos de l'imagerie.
     - CLIENT MYSQL : mysql-client. Nécessaire aux sauvegardes/restaurations (mysqldump / mysql).
     - GROUPE « dialout » : l'utilisateur est ajouté au groupe dialout (usermod -aG dialout) pour
       accéder aux appareils de mesure sur PORT SÉRIE. NB : la prise d'effet demande une
       déconnexion/reconnexion de la session.

   La liste des paquets est centralisée dans une seule variable (PAQUETS_REQUIS), partagée par la
   détection de présence, le bloc pkexec et le message d'installation manuelle (un seul endroit à
   maintenir). L'appel apt utilise DPkg::Lock::Timeout=300 (patiente si unattended-upgrades tient
   le verrou) et installe PAQUET PAR PAQUET (un paquet indisponible n'interrompt pas le reste).

   DÉTECTION DE PRÉSENCE (MAJ / réinstallation) : avant de proposer quoi que ce soit, on vérifie
   que TOUS les paquets requis sont déjà installés (dpkg-query) ET que l'utilisateur est déjà dans
   dialout. Si tout est en place, on SAUTE l'étape : pas de question, pas d'apt, pas de mot de
   passe admin. Plus de marqueur « .systeme_ok ».

   ÉCHEC : si la configuration échoue VRAIMENT, l'installation s'arrête et explique pourquoi
   (authentification annulée/refusée, pas d'accès réseau aux dépôts…), avec renvoi vers le journal
   détaillé ~/.rufus/install-systeme.log. Un paquet OPTIONNEL manquant (p. ex. un plugin GStreamer
   absent du dépôt) est journalisé mais NON bloquant. Sont bloquants : l'échec d'authentification
   pkexec et l'absence d'accès aux dépôts.

   PRÉREQUIS NON AUTO-INSTALLABLE : libfuse2 est nécessaire pour LANCER l'AppImage elle-même
   (problème de l'œuf et la poule) → il reste documenté sur la page de téléchargement, pas posé ici.

   PARTAGE RÉSEAU DE L'IMAGERIE : en poste CLIENT d'un réseau local, l'accès au dossier d'imagerie
   partagé reste une manipulation MANUELLE (montage du partage) — l'installeur ne l'automatise pas.

   --------------------------------------------------------------
   B. INSTALLATION DE L'APPLICATION ET INSCRIPTION DANS LES MENUS
   --------------------------------------------------------------
   Une fois le système configuré, l'AppRun « installe » Rufus comme le ferait un setup, SANS le
   demander (démarche standard) :
     - COPIE de l'AppImage dans ~/.local/bin/Rufus.AppImage (chmod +x) ;
     - ICÔNE dans ~/.local/share/icons/hicolor/256x256/apps/rufus.png ;
     - RACCOURCI DE MENU ~/.local/share/applications/rufus.desktop (Exec pointant sur l'AppImage
       installée, catégories Office;MedicalSoftware), puis rafraîchissement des caches
       (update-desktop-database, gtk-update-icon-cache).
   Cette étape (copie ~60 Mo + caches) prend quelques secondes : elle se déroule SOUS une barre de
   progression « Finalisation… » pour ne pas laisser l'écran vide.

   --------------------------------------------------------------
   C. FENÊTRE FINALE : INSTALLATION CONFIRMÉE + RACCOURCIS + LANCEMENT
   --------------------------------------------------------------
   À la fin, une boîte de dialogue annonce que le programme a bien été installé :
   « Rufus a été correctement installé sur ce poste et dans les menus, et le système a été
   configuré pour qu'il puisse fonctionner. Que souhaitez-vous faire ? », avec TROIS cases à
   cocher :
     - Créer un raccourci sur le BUREAU .................. PRÉCOCHÉE
       (copie du .desktop sur le bureau, vrai chemin via xdg-user-dir ; sous GNOME, le lanceur est
        marqué « de confiance » via gio, sinon il resterait grisé) ;
     - Inscrire dans la barre des FAVORIS ............... PRÉCOCHÉE
       (gsettings org.gnome.shell favorite-apps ; GNOME uniquement) ;
     - Exécuter Rufus maintenant ........................ NON COCHÉE.
   Rufus n'est lancé QUE si la dernière case est cochée — même logique qu'un installeur Windows
   (ISS). Le réglage de centrage est restitué à l'utilisateur AVANT ce lancement.

   Limites assumées : le centrage des fenêtres et l'épinglage aux favoris ne fonctionnent que sous
   GNOME (bureau par défaut d'Ubuntu) ; sous KDE/XFCE ces gestes sont des no-op silencieux.

   OUTIL DE TEST associé : build_tools/Linux/test/reset-poste-linux.sh remet un poste de TEST
   « vierge de Rufus » (retrait ciblé des paquets sans casser le bureau, sortie de dialout,
   suppression du home ~/.rufus / AppImage installée / raccourci menu / raccourci bureau /
   désépinglage des favoris / ~/Documents/Rufus, et option destructive de purge du serveur MySQL).


2. LANCEMENT DU PROGRAMME APRÈS INSTALLATION

  I. IL Y A UN rufus.ini   (déroulé CHRONOLOGIQUE, AVANT l'écran d'identification)

     1. CONNEXION à la base selon rufus.ini.
        On essaie les mots de passe candidats : celui du .dbkey (du mode courant) PUIS gaxt78iy.
        Si aucun ne marche -> message + VerifParamConnexion (corriger les paramètres, réécrire rufus.ini).

     2. CONTRÔLE DE LA VERSION DE MYSQL — AVANT TOUTE AUTRE CHOSE.
        (SELECT VERSION() sur la connexion qu'on vient d'ouvrir.)
        - Version NON conforme (< 8.0.14, ou MariaDB) ET ce poste HÉBERGE la base (monoposte / BDD_POSTE) :
              -> on lance la PROCÉDURE DE MISE À JOUR DE LA BASE (voir plus bas), puis on relance Rufus.
        - Ce poste est un CLIENT réseau (BDD_LOCAL / BDD_DISTANT) : il ne peut PAS migrer (ce n'est pas
              lui qui héberge la base). On AFFICHE un message « serveur MySQL à mettre à jour depuis le
              poste serveur », puis on continue normalement ; c'est le poste serveur (monoposte) qui migrera.
        - Version conforme -> on continue.

     3. SÉCURISATION (structure de contrôle) — seulement sur une base à la version conforme, et
        UNIQUEMENT depuis un poste LOCAL (monoposte ou réseau local) : JAMAIS depuis un accès DISTANT
        (WAN). Un poste distant ne doit pas « s'approprier » l'aléatoire — sinon, via la deadline, il
        pourrait verrouiller les postes locaux qui ne le détiennent pas.
        On regarde AVEC QUEL mot de passe la connexion a réussi, ET si adminrufus a déjà un 2e mot de passe :
           SELECT User_attributes->>'$.additional_password' IS NOT NULL
           FROM mysql.user WHERE User='adminrufus' AND Host='%';   -> 1 = base déjà sécurisée
          - Connecté avec le RANDOM (.dbkey) ............ rien à faire, ce poste a déjà le bon mot de passe.
          - Connecté avec GAXT78IY et PAS de 2e mot de passe .... la base n'est pas sécurisée :
                CE POSTE la sécurise (pose un random, garde gaxt78iy en 2e mot de passe avec RETAIN
                CURRENT PASSWORD), écrit son .dbkey, et invite fortement à NOTER/SAUVER le mot de passe.
          - Connecté avec GAXT78IY et un 2e mot de passe EXISTE .... base déjà sécurisée par un autre
                poste, mais celui-ci n'a pas le random : on DEMANDE le random (saisie ou clé USB),
                en précisant que le mot de passe actuellement utilisé arrive à échéance
                à la date d'introduction du mot de passe sécurisé + 30.
        Cette structure évite toute « course » entre postes : dès qu'un poste a sécurisé, les autres
        voient « déjà sécurisé » et demandent le mot de passe au lieu d'en recréer un.

     4. DEADLINE DE SUPPRESSION DE GAXT78IY.
        Quand la base est sécurisée, deadline = 30 jours après la date de sécurisation, lue côté serveur :
           SELECT password_last_changed FROM mysql.user WHERE User='adminrufus' AND Host='%';
        À la deadline, gaxt78iy est retiré (DISCARD OLD PASSWORD) SEULEMENT si les 3 conditions sont
        réunies : (a) ce poste détient le random (.dbkey ≠ gaxt78iy, sinon il se couperait l'accès) ;
        (b) adminrufus a BIEN un 2e mot de passe (base sécurisée) — on ne retire JAMAIS l'unique mot de
        passe ; (c) la deadline (sécurisation + 30 j) est dépassée. Au-delà, un poste sans le random ne
        peut plus se connecter : il devra le récupérer (papier/USB).

     5. Si ce poste vient de sécuriser (il a créé le .dbkey), on invite fortement :
          - à noter/sauver le mot de passe en lieu sûr (papier ou clé USB) ;
          - si le serveur est en réseau : à mettre à jour Rufus sur TOUS les autres postes (l'ancienne
            version ne marchera plus après la deadline) et à leur fournir le mot de passe (papier/USB).
          - afficher un message
          " IMPORTANT: un mot de passe sécurisé vient d'être mis en place.
            s'il existe d'autres postes sur le réseau local qui utilisent rufus,
            ils devront IMPÉRATIVEMENT être mis à jour vers cette nouvelle version de Rufus dans un délai d'un mois.
            Sans cette mise à jour, au-delà de ce délai, ils ne pourront plus utiliser Rufus."
"

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

=====================================================================
PLUGIN D'AUTHENTIFICATION : mysql_native_password (et pas caching_sha2_password)
=====================================================================
   Les comptes adminrufus / adminrufusSSL sont créés avec le plugin mysql_native_password,
   PAS avec caching_sha2_password (le plugin par défaut de MySQL 8).

   Pourquoi : en monoposte, le driver Qt (QMYSQL) ouvre une connexion TCP NON chiffrée vers
   localhost, sans récupérer la clé publique RSA du serveur. Avec caching_sha2_password, dès que
   le cache d'authentification du serveur est vide (serveur fraîchement installé, ou simplement
   redémarré / poste rebooté), la connexion est REFUSÉE (« Authentication plugin caching_sha2_password
   requires secure connection »). Symptôme observé : après une mise à jour du socle (réinstall +
   restauration), Rufus ne se reconnecte plus, alors que le mot de passe du .dbkey est pourtant le bon.
   La restauration, elle, passe : elle utilise le CLIENT mysql en ligne de commande, qui sait négocier
   la clé publique — contrairement au driver Qt.

   mysql_native_password fonctionne sur une connexion TCP en clair et survit aux redémarrages.
   - MySQL 8.0 (Linux/apt, anciens serveurs) : plugin actif par défaut → rien à faire.
   - MySQL 8.4 (Windows/macOS, installé par Rufus) : plugin DÉSACTIVÉ par défaut → Rufus écrit
     « mysql_native_password=ON » dans my.ini (Windows) / my.cnf (macOS) AVANT de créer les comptes.
   Ce réglage n'est appliqué que sur une installation NEUVE 8.4 ; on ne touche jamais le my.cnf d'un
   serveur 8.0 préexistant (où cette option n'existe pas et empêcherait le serveur de démarrer).

   Code : MySQLInstaller::createUser / createUserAvecAdmin / securiserBaseSiNecessaire (clause
   « IDENTIFIED WITH mysql_native_password BY … »), et activation du plugin dans installMySQL (my.ini
   Windows) et prepareCreateModeMacOS (my.cnf macOS).

=====================================================================
À FAIRE — ÉTAPE 7 DE LA CONFIG SERVEUR : PRÉPARER LES CONNEXIONS CHIFFRÉES (SSL)
=====================================================================
   (En cours d'implémentation.)

   Lors de l'installation/configuration du serveur MySQL, il manque une 7e étape : préparer le
   serveur pour les connexions chiffrées (SSL), nécessaires aux accès DISTANTS (WAN) via les
   comptes adminrufusSSL (REQUIRE SSL).

   APPROCHE RETENUE : certificats AUTO-GÉNÉRÉS PAR MYSQL (pas d'openssl).
   MySQL est lié à SA PROPRE OpenSSL (DLL/libs livrées avec le serveur). À l'--initialize
   (--initialize-insecure), il dépose dans le datadir : ca.pem, ca-key.pem, server-cert.pem,
   server-key.pem, client-cert.pem, client-key.pem (+ private/public_key.pem). Le serveur les
   utilise tout seul → SSL actif SANS toucher au my.ini. AUCUN openssl système requis (ce qui
   échoue sous Windows 11, c'est seulement l'utilitaire mysql_ssl_rsa_setup qui appelle la
   commande openssl externe — on ne l'utilise pas). On ne pose PAS require_secure_transport
   (casserait le monoposte local en clair) : le chiffrement reste imposé PAR COMPTE via
   adminrufusSSL REQUIRE SSL.

   Les 5 étapes (dans l'ordre) :
   1. SERVEUR — s'assurer que SSL est actif. Cas nominal : rien à ajouter au my.ini (MySQL prend
      ca.pem/server-cert.pem/server-key.pem du datadir). Garde-fous : ne JAMAIS toucher un serveur
      préexistant ; pas de require_secure_transport.
   2. RÉCOLTE DES CLÉS CLIENT. Après une install neuve, copier depuis le datadir vers le dossier
      des clés du serveur (PATH_DIR_RUFUS + "/DossierClesSSL") UNIQUEMENT les clés CLIENT :
      ca.pem -> RENOMMÉ ca-cert.pem, client-cert.pem, client-key.pem (noms attendus par
      database.cpp). On ne récolte JAMAIS server-key.pem / server-cert.pem / ca-key.pem.
   3. EXPORT UTILISATEUR — CÔTÉ SERVEUR (dlg_param.cpp, onglet « Ce poste » / tabMono). Bouton pour
      EXPORTER vers une clé USB les SEULES clés client (la copie conservée par le serveur), afin de
      les porter sur les postes distants.
      SÉCURITÉ : l'extraction des clés client est un geste sensible (elles permettent à une machine
      distante de se connecter). Elle doit se faire DEPUIS LE SERVEUR (poste maîtrisé), pas via un
      bouton de confort sur un poste distant quelconque. -> le bouton est sur tabMono, JAMAIS sur
      tabDistant. (Correction d'une 1re version placée à tort sur tabDistant : retirée.)
      Côté poste distant : on garde le simple sélecteur de dossier « … » (Dossier_ClesSSL) pour
      pointer les clés apportées par clé USB ; pas d'import « automatique ».
      Implémenté : bouton « Exporter les clés client SSL (clé USB) » sur l'onglet Monoposte
      (tabMono), visible seulement si ce poste héberge la base. Il copie depuis
      PATH_DIR_CLESSSL_SERVEUR les 3 clés client vers la clé USB choisie ; avertit si les clés ne
      sont pas (encore) disponibles. Slot dlg_param::ExporterClesSSLversUSB().
   4. REMPLACEMENT D'UN ANCIEN SERVEUR : conserver/réinjecter les anciennes clés (voir ci-dessous).
      Implémenté dans reinstallerSocleMySQLpourMigration() : sauvegarderClesSSLMigration() copie les
      6 .pem du datadir (CA + serveur + client) vers ~/.rufus/ssl_migration AVANT uninstallMySQL()
      (qui détruit le datadir) ; après réinstallation, restaurerClesSSLMigration() les réinjecte dans
      le nouveau datadir (droits/propriétaire rétablis), redémarre MySQL et réactualise la copie
      client (PATH_DIR_CLESSSL_SERVEUR). Le serveur sert ainsi le MÊME CA -> aucun poste distant à
      retoucher. Datadir root-only (Linux/macOS) : sauvegarde/réinjection CÔTÉ ROOT (script élevé) ;
      Windows : en C++. Helper mysqlDataDir() pour le chemin du datadir selon l'OS.
   5. macOS et Linux/apt : même logique (datadir auto-génère les certs), récolte identique.
      Implémenté. Subtilité : sur macOS (datadir _mysql, client-key.pem en 0600) et Linux
      (/var/lib/mysql appartenant à mysql), la récolte se fait CÔTÉ ROOT dans le script élevé
      (oracleInitStartScript pour macOS ; script pkexec apt pour Linux), avec restitution à
      l'utilisateur (chown). Sur Windows, le datadir est lisible par le processus : récolte en
      C++ (recolterClesClientSSL). Destination commune : PATH_DIR_CLESSSL_SERVEUR.

   Petite subtilité — REMPLACEMENT D'UN ANCIEN SERVEUR (mise à jour du socle) :
      Si on remplace une ancienne version de MySQL, CONSERVER les anciennes clés et les réinjecter
      dans la nouvelle installation, plutôt que d'en générer de nouvelles. Sinon, il faudrait
      redéployer les clés client sur TOUS les postes en accès distant. Garder les anciennes évite
      cette corvée et ne casse aucun poste distant existant.
