#!/bin/bash
# Retire tout ce qu'a posé installer-mysql57-test.sh, et le MySQL d'apt s'il y en a un.
# Poste de test uniquement : DÉTRUIT les bases présentes.
set -u

VERSION=5.7.44
ARCHIVE=mysql-${VERSION}-linux-glibc2.12-x86_64

[ "$EUID" -eq 0 ] || { echo "À lancer avec sudo."; exit 1; }

read -p "Effacer MySQL et TOUTES ses bases sur ce poste ? [oui/non] " rep
[ "$rep" = "oui" ] || { echo "Abandon."; exit 0; }

echo "== arrêt du service =="
systemctl disable --now mysql 2>/dev/null || true
pkill -x mysqld 2>/dev/null || true
sleep 2

echo "== installation manuelle (/usr/local/mysql) =="
rm -rf /usr/local/mysql /usr/local/${ARCHIVE}
rm -f /etc/systemd/system/mysql.service
systemctl daemon-reload

echo "== installation apt éventuelle =="
apt-get remove --purge -y 'mysql-server*' 'mysql-client*' 'mysql-common' 'mariadb-*' 2>/dev/null || true
apt-get autoremove -y 2>/dev/null || true

echo "== fichiers résiduels =="
rm -f  /etc/my.cnf
rm -rf /etc/mysql /var/lib/mysql /var/log/mysql /var/run/mysqld
rm -f  /etc/paths.d/mysql /etc/profile.d/mysql.sh

echo "== compte système =="
userdel mysql 2>/dev/null || true
groupdel mysql 2>/dev/null || true

echo
echo "Terminé. Le dossier d'imagerie de test n'est PAS effacé :"
echo "  rm -rf ~/RufusTest   (à faire à la main si besoin)"
echo
command -v mysqld >/dev/null && echo "ATTENTION : un mysqld subsiste dans le PATH." || echo "Plus aucun mysqld dans le PATH."
