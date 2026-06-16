#!/usr/bin/env bash
# =============================================================================
#  OUTIL DE TEST — installe MySQL 8.0.13 (< seuil Rufus 8.0.14) sur Ubuntu 22.04+
#  afin de TESTER la procédure de mise à jour du socle MySQL (migration).
#
#  8.0.13 est choisi à dessein : c'est UN patch sous le seuil, et le double mot de
#  passe (ALTER USER … RETAIN CURRENT PASSWORD) n'apparaît qu'en 8.0.14 → 8.0.13
#  est donc RÉELLEMENT non conforme, pas juste « marqué » comme tel.
#
#  Le script :
#    1. installe libssl1.1 (absent de 22.04, requis par les binaires 8.0.13) ;
#    2. télécharge + installe MySQL 8.0.13 (DEB bundle des archives Oracle) ;
#    3. démarre le serveur ;
#    4. crée adminrufus / adminrufusSSL @'%' avec le mot de passe legacy gaxt78iy
#       (comme un poste préparé pour une ANCIENNE version de Rufus).
#
#  Ensuite : lance l'ANCIENNE version de Rufus pour créer la base de test, puis la
#  NOUVELLE version → elle doit détecter 8.0.13 < 8.0.14 et proposer la migration.
#
#  ⚠️  À N'EXÉCUTER QUE SUR UNE MACHINE / VM DE TEST. Jamais en production.
#  Nécessite : sudo, accès internet, architecture amd64.
# =============================================================================

set -euo pipefail

MYSQL_VER="8.0.13"
UBUNTU_BUILD="18.04"          # build des binaires 8.0.13 (lié à libssl1.1)
GAXT="gaxt78iy"               # MDP_SQL legacy attendu par Rufus
ROOTPW="rootRufusTest"        # mot de passe root MySQL (test uniquement)
ARCH="$(dpkg --print-architecture)"
BUNDLE="mysql-server_${MYSQL_VER}-1ubuntu${UBUNTU_BUILD}_${ARCH}.deb-bundle.tar"
ARCHIVE_URL="https://downloads.mysql.com/archives/get/p/23/file/${BUNDLE}"
LIBSSL_URL="http://archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2_${ARCH}.deb"

WORKDIR="$(mktemp -d)"
echo "### Répertoire de travail : $WORKDIR"
cd "$WORKDIR"

# --- 0. Garde-fou : ne rien casser si un serveur est déjà installé ----------
if command -v mysqld >/dev/null 2>&1; then
    echo "ERREUR : un serveur MySQL/MariaDB est déjà présent."
    echo "         Purge-le d'abord, p.ex. :"
    echo "         sudo apt-get remove --purge 'mysql-server*' 'mysql-community*' 'mariadb-server*'"
    echo "         sudo rm -rf /var/lib/mysql /etc/mysql && sudo apt-get autoremove --purge -y"
    exit 1
fi

# --- 1. libssl1.1 (cohabite avec libssl3) -----------------------------------
if ! dpkg -l 2>/dev/null | grep -q '^ii  libssl1.1 '; then
    echo "### Installation de libssl1.1 (archive focal)"
    wget -q "$LIBSSL_URL" -O libssl1.1.deb
    sudo dpkg -i libssl1.1.deb
fi

# --- 2. Dépendances usuelles ------------------------------------------------
sudo apt-get update -qq
sudo apt-get install -y libaio1 libmecab2 wget tar >/dev/null

# --- 3. Téléchargement du DEB bundle 8.0.13 ---------------------------------
echo "### Téléchargement de $BUNDLE"
if [ ! -f "$BUNDLE" ]; then
    wget -q "$ARCHIVE_URL" -O "$BUNDLE" || true
fi
# Vérifie que c'est bien une archive tar valide (sinon : téléchargement manuel)
if ! tar -tf "$BUNDLE" >/dev/null 2>&1; then
    echo
    echo "ÉCHEC du téléchargement automatique."
    echo "Télécharge manuellement « $BUNDLE » depuis :"
    echo "  https://downloads.mysql.com/archives/community/"
    echo "  (Version 8.0.13, OS « Ubuntu Linux », build ${UBUNTU_BUILD}, « DEB Bundle »)"
    echo "Place le fichier dans : $WORKDIR"
    echo "puis relance ce script (il réutilisera le fichier présent)."
    exit 1
fi
mkdir -p deb && tar -xf "$BUNDLE" -C deb

# --- 4. Préconfiguration du mot de passe root (évite l'invite debconf) -------
echo "mysql-community-server mysql-community-server/root-pass password $ROOTPW"    | sudo debconf-set-selections
echo "mysql-community-server mysql-community-server/re-root-pass password $ROOTPW" | sudo debconf-set-selections

# --- 5. Installation des paquets (on n'installe QUE le nécessaire) -----------
echo "### Installation de MySQL $MYSQL_VER"
sudo DEBIAN_FRONTEND=noninteractive dpkg -i \
    deb/mysql-common_*.deb \
    deb/mysql-community-client-core_*.deb \
    deb/mysql-community-client_*.deb \
    deb/mysql-client_*.deb \
    deb/mysql-community-server-core_*.deb \
    deb/mysql-community-server_*.deb \
    deb/mysql-server_*.deb || true
sudo DEBIAN_FRONTEND=noninteractive apt-get -f install -y   # résout les deps restantes

# --- 6. Démarrage -----------------------------------------------------------
sudo systemctl enable mysql >/dev/null 2>&1 || true
sudo systemctl start mysql 2>/dev/null || sudo service mysql start
sleep 3
echo -n "### Version installée : " ; mysqld --version

# --- 7. Comptes adminrufus / adminrufusSSL (mdp legacy gaxt78iy) ------------
echo "### Création de adminrufus / adminrufusSSL (mdp : $GAXT)"
sudo mysql -uroot -p"$ROOTPW" <<SQL
CREATE USER IF NOT EXISTS 'adminrufus'@'%'    IDENTIFIED WITH mysql_native_password BY '$GAXT';
ALTER  USER               'adminrufus'@'%'    IDENTIFIED WITH mysql_native_password BY '$GAXT';
GRANT ALL PRIVILEGES ON *.* TO 'adminrufus'@'%' WITH GRANT OPTION;
CREATE USER IF NOT EXISTS 'adminrufusSSL'@'%' IDENTIFIED WITH mysql_native_password BY '$GAXT' REQUIRE SSL;
ALTER  USER               'adminrufusSSL'@'%' IDENTIFIED WITH mysql_native_password BY '$GAXT' REQUIRE SSL;
GRANT ALL PRIVILEGES ON *.* TO 'adminrufusSSL'@'%' WITH GRANT OPTION;
FLUSH PRIVILEGES;
SQL

echo
echo "============================================================================="
echo "  TERMINÉ."
echo "  MySQL $MYSQL_VER installé (root / $ROOTPW)."
echo "  Comptes adminrufus / adminrufusSSL créés (mdp : $GAXT)."
echo
echo "  Étapes suivantes :"
echo "   1. Lance l'ANCIENNE version de Rufus → crée la base patients de test."
echo "   2. Lance la NOUVELLE version de Rufus → elle doit détecter 8.0.13 < 8.0.14"
echo "      et proposer la mise à jour du socle MySQL (migration)."
echo
echo "  (Accès réseau : par défaut MySQL n'écoute que sur localhost. Pour un test LAN,"
echo "   ajoute 'bind-address = 0.0.0.0' dans /etc/mysql/mysql.conf.d/mysqld.cnf puis"
echo "   redémarre : sudo systemctl restart mysql.)"
echo "============================================================================="
