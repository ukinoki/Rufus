#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
#  reset-poste-linux.sh — remet un poste de TEST Linux dans un état "vierge de
#  Rufus", pour rejouer une installation à blanc sans avoir à restaurer Timeshift.
#
#  Retire ce que l'install de Rufus a posé :
#    1. les paquets de configurer_systeme (AppRun) : polices, GStreamer, mysql-client ;
#    2. (optionnel) le serveur MySQL + ses données (DESTRUCTIF) ;
#    3. l'appartenance au groupe dialout ;
#    4. les fichiers Rufus du home (~/.rufus, AppImage installée, raccourci, ~/Documents/Rufus).
#
#  ⚠️ Outil de TEST/DEV — pas livré aux utilisateurs. À lancer en root :
#        sudo ./reset-poste-linux.sh
#
#  NOTE de sûreté : pour les paquets (étape 1), on les marque "auto" puis
#  `apt autoremove --purge` → on NE retire QUE ceux dont plus rien ne dépend.
#  Ainsi on ne casse pas le bureau si, p.ex., GNOME a besoin d'un plugin GStreamer.
# ─────────────────────────────────────────────────────────────────────────────
set -u

if [ "$(id -u)" -ne 0 ]; then
    echo "À lancer en root :  sudo $0" >&2
    exit 1
fi

USER_REEL="${SUDO_USER:-$(id -un)}"
HOME_REEL="$(getent passwd "${USER_REEL}" | cut -d: -f6)"
echo "== Reset poste Rufus (TEST) pour l'utilisateur : ${USER_REEL} (${HOME_REEL}) =="

APT="apt-get -o DPkg::Lock::Timeout=300"

# ── 1. Paquets posés par configurer_systeme (AppRun) ──────────────────────────
PKGS="gsfonts ttf-mscorefonts-installer \
gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x \
gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-pulseaudio mysql-client"
echo "-- Étape 1 : retrait des paquets Rufus (autoremove ciblé, sans casser le bureau)"
apt-mark auto ${PKGS} 2>/dev/null || true
DEBIAN_FRONTEND=noninteractive ${APT} autoremove --purge -y

# ── 2. Serveur MySQL + données (DESTRUCTIF) ───────────────────────────────────
read -r -p "Retirer AUSSI le serveur MySQL et EFFACER ses données (/var/lib/mysql) ? [o/N] " rep
if [ "${rep}" = "o" ] || [ "${rep}" = "O" ]; then
    echo "-- Étape 2 : purge du serveur MySQL + données (calquée sur MySQLInstaller::uninstallMySQL)"
    systemctl stop mysql mysqld mariadb 2>/dev/null || true
    for pat in 'mysql-server.*' 'mysql-client.*' 'libmysqlclient.*' 'mysql-common' 'mysql.*' 'mariadb.*'; do
        DEBIAN_FRONTEND=noninteractive ${APT} purge -y "$pat" 2>/dev/null || true
    done
    DEBIAN_FRONTEND=noninteractive ${APT} autoremove --purge -y || true
    rm -rf /etc/mysql /var/lib/mysql /var/log/mysql /var/lib/mysql-files /var/lib/mysql-keyring
    echo "   (samba/wsdd et le dossier partagé sont LAISSÉS — comme le fait l'appli.)"
else
    echo "-- Étape 2 : serveur MySQL CONSERVÉ."
fi

# ── 3. Groupe dialout ─────────────────────────────────────────────────────────
echo "-- Étape 3 : retrait de ${USER_REEL} du groupe dialout"
gpasswd -d "${USER_REEL}" dialout 2>/dev/null || true

# ── 4. Fichiers Rufus dans le home ────────────────────────────────────────────
echo "-- Étape 4 : nettoyage du home"
rm -rf "${HOME_REEL}/.rufus" \
       "${HOME_REEL}/.local/bin/Rufus.AppImage" \
       "${HOME_REEL}/.local/share/applications/rufus.desktop" \
       "${HOME_REEL}/.local/share/icons/hicolor/256x256/apps/rufus.png" \
       "${HOME_REEL}/Documents/Rufus"
# Raccourci sur le bureau : on interroge xdg-user-dir (chemin réel, éventuellement
# localisé/personnalisé) en plus des noms usuels « Bureau » (FR) et « Desktop » (EN).
DESK_REEL="$(sudo -u "${USER_REEL}" xdg-user-dir DESKTOP 2>/dev/null)"
rm -f "${DESK_REEL}/rufus.desktop" \
      "${HOME_REEL}/Bureau/rufus.desktop" \
      "${HOME_REEL}/Desktop/rufus.desktop"
sudo -u "${USER_REEL}" update-desktop-database "${HOME_REEL}/.local/share/applications" 2>/dev/null || true
sudo -u "${USER_REEL}" gtk-update-icon-cache -f -t "${HOME_REEL}/.local/share/icons/hicolor" 2>/dev/null || true

# ── 5. Désépinglage de la barre des favoris (GNOME) ───────────────────────────
# gsettings a besoin de la session DBus de l'utilisateur réel (pas celle de root).
echo "-- Étape 5 : désépinglage de Rufus des favoris GNOME (best-effort)"
RUNTIME_DIR="/run/user/$(id -u "${USER_REEL}")"
sudo -u "${USER_REEL}" \
    env XDG_RUNTIME_DIR="${RUNTIME_DIR}" \
        DBUS_SESSION_BUS_ADDRESS="unix:path=${RUNTIME_DIR}/bus" \
    python3 - <<'PY' 2>/dev/null || true
import subprocess, ast
try:
    cur = subprocess.check_output(
        ['gsettings', 'get', 'org.gnome.shell', 'favorite-apps'], text=True).strip()
    lst = ast.literal_eval(cur) if cur.startswith('[') else []
    if 'rufus.desktop' in lst:
        lst = [x for x in lst if x != 'rufus.desktop']
        subprocess.run(['gsettings', 'set', 'org.gnome.shell', 'favorite-apps',
                        '[' + ', '.join("'%s'" % x for x in lst) + ']'])
except Exception:
    pass
PY

echo "== Reset terminé. Déconnecte/reconnecte la session (groupe dialout) avant de retester. =="
