#!/bin/bash
#
# Désinstallation TOTALE de MySQL 8.4.3 (installeur officiel Oracle .dmg)
# ATTENTION : supprime définitivement toutes les bases de données. Aucun backup.
# À lancer avec :  sudo bash uninstall_mysql.sh
#
set -u

echo "==> Désinstallation complète de MySQL"

if [ "$(id -u)" -ne 0 ]; then
  echo "Ce script doit être lancé en root :  sudo bash uninstall_mysql.sh" >&2
  exit 1
fi

echo "--> 1. Arrêt du serveur MySQL (LaunchDaemon)"
launchctl bootout system /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null \
  || launchctl unload -w /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null
# Tue tout mysqld encore en vie
pkill -f /usr/local/mysql/bin/mysqld 2>/dev/null
sleep 2

echo "--> 2. Suppression des fichiers d'installation"
rm -f  /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist
rm -rf /Library/PreferencePanes/MySQL.prefPane
rm -rf /usr/local/mysql                          # symlink
rm -rf /usr/local/mysql-8.4.3-macos14-x86_64     # contenu réel (binaires + data + bases)
rm -f  /etc/my.cnf
rm -f  /tmp/mysql.sock /tmp/mysql.sock.lock

echo "--> 3. Nettoyage du PATH (/etc/paths)"
if grep -q '^/usr/local/mysql/bin$' /etc/paths 2>/dev/null; then
  grep -v '^/usr/local/mysql/bin$' /etc/paths > /etc/paths.tmp && mv /etc/paths.tmp /etc/paths
  echo "    ligne /usr/local/mysql/bin retirée"
fi

echo "--> 4. Oubli des reçus d'installation (pkgutil)"
for p in com.mysql.launchd com.mysql.mysql com.mysql.prefpane com.oracle.oss.mysql.mysqld; do
  pkgutil --forget "$p" 2>/dev/null
done

echo "--> 5. Vérification"
echo -n "    mysql dans le PATH : " ; command -v mysql || echo "absent (OK)"
echo -n "    /usr/local/mysql*  : " ; ls -d /usr/local/mysql* 2>/dev/null || echo "absent (OK)"
echo -n "    prefPane           : " ; ls -d /Library/PreferencePanes/MySQL.prefPane 2>/dev/null || echo "absent (OK)"
echo -n "    LaunchDaemon       : " ; ls /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist 2>/dev/null || echo "absent (OK)"
echo -n "    process mysqld     : " ; pgrep -fl mysqld || echo "aucun (OK)"

echo "==> Terminé. (Le compte système _mysql d'Apple, UID 74, est conservé — il est natif à macOS.)"
echo "    Ouvre un nouveau terminal pour que le PATH soit rafraîchi."
