#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
#  reset-poste-linux.sh — remet un poste de TEST Linux dans un état "vierge de
#  Rufus", pour rejouer une installation à blanc sans avoir à restaurer Timeshift.
#
#  Retire ce que l'install de Rufus a posé :
#    1. les paquets de configurer_systeme (AppRun) : polices, GStreamer, mysql-client ;
#    2. (optionnel) le serveur MySQL + ses données (DESTRUCTIF) ;
#    3. l'appartenance au groupe dialout ;
#    4. les fichiers Rufus du home (~/.rufus, AppImage installée, raccourci, ~/Documents/Rufus) ;
#    5. le raccourci du bureau et l'épinglage dans les favoris GNOME.
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
    for pat in 'mysql-server.*' 'mysql-client.*' 'mysql-common' 'mysql.*' 'mariadb.*'; do
        DEBIAN_FRONTEND=noninteractive ${APT} purge -y "$pat" 2>/dev/null || true
    done
    DEBIAN_FRONTEND=noninteractive ${APT} autoremove --purge -y || true

    # Un lien d'unité orphelin fait échouer les scripts du paquet (« Link has been severed »)
    # et interdit toute réinstallation ultérieure.
    rm -f /etc/systemd/system/mysql.service /etc/systemd/system/multi-user.target.wants/mysql.service
    rm -rf /var/lib/systemd/deb-systemd-helper-enabled/mysql.service.dsh-also
    systemctl daemon-reload 2>/dev/null || true

    # Paquet resté coincé (postinst en échec) : apt n'en sort plus, on neutralise ses scripts.
    RESTE=$(dpkg-query -f '${binary:Package} ${db:Status-Status}\n' -W 'mysql-server*' 2>/dev/null \
            | grep -v ' not-installed$' | cut -d' ' -f1)
    if [ -n "${RESTE}" ]; then
        echo "   paquets coincés : ${RESTE} -> purge forcée"
        for p in ${RESTE}; do
            rm -f "/var/lib/dpkg/info/${p}.preinst"  "/var/lib/dpkg/info/${p}.postinst" \
                  "/var/lib/dpkg/info/${p}.prerm"    "/var/lib/dpkg/info/${p}.postrm"
        done
        dpkg --purge --force-all ${RESTE} 2>/dev/null || true
    fi

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
       "${HOME_REEL}/Documents/Rufus"
rm -rf "${HOME_REEL}/.local/share/icons/hicolor/256x256/apps/rufus.png"

# ── 5. Raccourci du bureau + favoris GNOME (posés par AppRun) ─────────────────
echo "-- Étape 5 : retrait du raccourci du bureau et des favoris"
# gsettings et xdg-user-dir ont besoin du bus de session de l'utilisateur.
CMD_USER="sudo -u ${USER_REEL} env DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u "${USER_REEL}")/bus"

BUREAU="$(${CMD_USER} xdg-user-dir DESKTOP 2>/dev/null)"
[ -z "${BUREAU}" ] && { [ -d "${HOME_REEL}/Bureau" ] && BUREAU="${HOME_REEL}/Bureau" || BUREAU="${HOME_REEL}/Desktop"; }
rm -f "${BUREAU}/rufus.desktop"

FAV="$(${CMD_USER} gsettings get org.gnome.shell favorite-apps 2>/dev/null)" || FAV=""
case "${FAV}" in
    *"'rufus.desktop'"*)
        NOUV="$(printf '%s' "${FAV}" | sed "s/, \?'rufus\.desktop'//; s/'rufus\.desktop', \?//; s/'rufus\.desktop'//")"
        ${CMD_USER} gsettings set org.gnome.shell favorite-apps "${NOUV}" 2>/dev/null || true ;;
esac

${CMD_USER} update-desktop-database "${HOME_REEL}/.local/share/applications" 2>/dev/null || true
${CMD_USER} gtk-update-icon-cache -f -t "${HOME_REEL}/.local/share/icons/hicolor" 2>/dev/null || true

echo "== Reset terminé. Déconnecte/reconnecte la session (groupe dialout) avant de retester. =="
