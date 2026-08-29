#!/bin/bash
# Installe MySQL 5.7.44 sur Ubuntu pour TESTER la migration Rufus. Poste de test uniquement.
# Emplacement /usr/local/mysql : celui que MySQLInstaller::oraclePrefix() reconnaît sous Linux.
set -e

VERSION=5.7.44
ARCHIVE=mysql-${VERSION}-linux-glibc2.12-x86_64
URL=https://dev.mysql.com/get/Downloads/MySQL-5.7/${ARCHIVE}.tar.gz

# Dossier d'imagerie volontairement DANS le home : c'est le cas que Rufus doit rattraper
SECURE_DIR=${SECURE_DIR:-/home/$SUDO_USER/RufusTest}

[ "$EUID" -eq 0 ] || { echo "À lancer avec sudo."; exit 1; }
[ "$(uname -m)" = "x86_64" ] || { echo "Archive disponible en x86_64 seulement."; exit 1; }

echo "== dépendances =="
apt-get update -qq
# libaio1 renommée libaio1t64 sur Ubuntu 24.04
apt-get install -y libaio1 2>/dev/null || apt-get install -y libaio1t64
apt-get install -y libnuma1 wget

echo "== téléchargement =="
cd /tmp
[ -f ${ARCHIVE}.tar.gz ] || wget -q --show-progress "$URL"

echo "== installation dans /usr/local/mysql =="
systemctl stop mysql 2>/dev/null || true
rm -rf /usr/local/${ARCHIVE} /usr/local/mysql
tar xzf ${ARCHIVE}.tar.gz -C /usr/local
ln -s /usr/local/${ARCHIVE} /usr/local/mysql

id -u mysql >/dev/null 2>&1 || useradd -r -s /bin/false mysql
mkdir -p /usr/local/mysql/data /var/run/mysqld "$SECURE_DIR/Rufus/Imagerie"
chown -R mysql:mysql /usr/local/mysql/ /var/run/mysqld
chmod -R 777 "$SECURE_DIR"

echo "== my.cnf =="
cat > /etc/my.cnf <<EOF
[mysqld]
basedir=/usr/local/mysql
datadir=/usr/local/mysql/data
socket=/var/run/mysqld/mysqld.sock
pid-file=/var/run/mysqld/mysqld.pid
port=3306
bind-address=0.0.0.0
secure_file_priv=$SECURE_DIR
sql_mode=STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION

[client]
socket=/var/run/mysqld/mysqld.sock
EOF

echo "== initialisation du datadir (root sans mot de passe) =="
/usr/local/mysql/bin/mysqld --initialize-insecure --user=mysql \
    --basedir=/usr/local/mysql --datadir=/usr/local/mysql/data >/dev/null

echo "== service systemd nommé mysql (pour que Rufus puisse l'arrêter) =="
cat > /etc/systemd/system/mysql.service <<'EOF'
[Unit]
Description=MySQL 5.7 (test Rufus)
After=network.target

[Service]
User=mysql
Group=mysql
ExecStart=/usr/local/mysql/bin/mysqld --defaults-file=/etc/my.cnf
Restart=on-failure
LimitNOFILE=10000

[Install]
WantedBy=multi-user.target
EOF
systemctl daemon-reload
systemctl enable --now mysql
sleep 5

echo "== compte adminrufus, comme une ancienne installation Rufus =="
/usr/local/mysql/bin/mysql -u root --socket=/var/run/mysqld/mysqld.sock <<'EOF'
CREATE USER IF NOT EXISTS 'adminrufus'@'localhost' IDENTIFIED BY 'gaxt78iy';
CREATE USER IF NOT EXISTS 'adminrufus'@'127.0.0.1' IDENTIFIED BY 'gaxt78iy';
CREATE USER IF NOT EXISTS 'adminrufus'@'%' IDENTIFIED BY 'gaxt78iy';
GRANT ALL PRIVILEGES ON *.* TO 'adminrufus'@'localhost' WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON *.* TO 'adminrufus'@'127.0.0.1' WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON *.* TO 'adminrufus'@'%' WITH GRANT OPTION;
FLUSH PRIVILEGES;
EOF

echo
echo "MySQL $(/usr/local/mysql/bin/mysql -u root --socket=/var/run/mysqld/mysqld.sock -N -B -e 'SELECT VERSION()') installé."
echo "secure_file_priv = $SECURE_DIR   (imagerie de test : $SECURE_DIR/Rufus/Imagerie)"
echo "adminrufus / gaxt78iy — root sans mot de passe."
echo
echo "Pour tout retirer :"
echo "  sudo systemctl disable --now mysql"
echo "  sudo rm -rf /usr/local/mysql /usr/local/${ARCHIVE} /etc/my.cnf /etc/systemd/system/mysql.service"
echo "  sudo systemctl daemon-reload"
