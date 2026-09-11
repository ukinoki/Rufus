# Donner une adresse fixe au poste serveur

Les postes clients rejoignent la base par l'**adresse du serveur** — celle qui
est inscrite dans Rufus, sur chaque poste client (Édition → Paramètres → onglet
« Ce poste » → « Réseau local », ligne « Emplacement »).

Or cette adresse, c'est la box qui la distribue automatiquement, et rien ne
l'oblige à donner la même qu'hier : après une coupure de courant ou un
redémarrage de la box, le serveur peut se réveiller avec une autre adresse. Les
postes clients, eux, cherchent toujours l'ancienne et ne trouvent plus rien.

Toute l'opération consiste donc à **figer l'adresse du serveur**, une fois pour
toutes. Dans les exemples ci-dessous, la box est en `192.168.1.1` et l'adresse
retenue pour le serveur est `192.168.1.240`.

> **Faites-vous aider.** Ce réglage n'a rien de spécifique à Rufus et n'importe
> quelle IA (ChatGPT, Claude, Gemini…) le connaît parfaitement. Donnez-lui le
> système du serveur et la marque de votre box : vous gagnerez du temps.

---

## 1. La méthode la plus simple : le faire faire par la box

Plutôt que de configurer l'ordinateur, on demande à la box de **toujours
attribuer la même adresse** à ce poste. Rien ne change sur le serveur, et le
réglage survit à une réinstallation du système.

1. Relever l'adresse actuelle du serveur (voir le § 2).
2. Sur n'importe quel poste, ouvrir `http://192.168.1.1` dans un navigateur et
   se connecter à la box (le mot de passe est généralement sur son étiquette).
3. Chercher la rubrique **DHCP** : « Baux statiques », « Adresses réservées »,
   « DHCP statique » — le nom change selon la marque.
4. Y ajouter le poste serveur, repéré par son nom dans la liste des appareils
   connectés, et valider.
5. Redémarrer le serveur, puis vérifier que son adresse n'a pas bougé.

Si votre box ne propose pas cette fonction, ou si vous n'avez pas son mot de
passe, passez aux réglages sur la machine (§ 3 à 5).

---

## 2. Avant de commencer : relever quatre nombres

Sur le **poste serveur**, notez l'adresse actuelle, le masque, la passerelle
(c'est la box) et le DNS (la box aussi, le plus souvent) :

| Système | Où regarder                                                        |
|---------|--------------------------------------------------------------------|
| Windows | touche Windows, taper `cmd`, Entrée, puis `ipconfig /all`           |
| macOS   | Réglages Système → Réseau → Wi-Fi ou Ethernet → **Détails…**        |
| Linux   | dans un terminal, `ip a` puis `ip route`                            |

**Attention à la carte réseau.** Le câble et le Wi-Fi sont deux cartes
distinctes, chacune avec son adresse : réglez celle par laquelle le serveur est
réellement connecté. Un serveur branché en câble est plus stable.

**Choisir l'adresse fixe.** On garde les trois premiers nombres de l'adresse
actuelle et on ne change que le dernier. Prenez-le élevé (240 à 250) : les box
distribuent en partant du bas, la collision devient très improbable. Vérifiez
qu'il est libre en le pinguant depuis un autre poste — `ping 192.168.1.240` ne
doit recevoir **aucune réponse**.

---

## 3. Le serveur est sous Windows

1. **Paramètres** → **Réseau et Internet** → **Ethernet** (ou **Wi-Fi** → le
   réseau connecté).
2. Ligne **Attribution IP** → **Modifier** → choisir **Manuel** et activer
   **IPv4**.
3. Renseigner :
   - Adresse IP : `192.168.1.240`
   - Masque de sous-réseau : `255.255.255.0`
   - Passerelle : `192.168.1.1`
   - DNS préféré : `192.168.1.1`
4. **Enregistrer**.

---

## 4. Le serveur est sous macOS

1. **Réglages Système** → **Réseau** → **Wi-Fi** (ou **Ethernet**) →
   **Détails…**
2. Onglet **TCP/IP** → « Configurer IPv4 » : choisir **Via DHCP avec adresse
   manuelle**.
3. Saisir la seule adresse `192.168.1.240`, puis **OK**.

Ce mode est le plus simple : masque, routeur et DNS continuent d'être fournis
par la box, il n'y a rien d'autre à taper. Le mode « Manuellement » oblige, lui,
à renseigner les quatre valeurs.

---

## 5. Le serveur est sous Linux (Ubuntu)

1. **Paramètres** → **Réseau** (ou **Wi-Fi**) → **roue dentée** de la connexion.
2. Onglet **IPv4** → cocher **Manuel** :
   - Adresse : `192.168.1.240`, Masque : `255.255.255.0`,
     Passerelle : `192.168.1.1`
   - DNS : décocher « Automatique » et saisir `192.168.1.1`
3. **Appliquer**, puis **éteindre et rallumer la connexion** avec son
   interrupteur : sans cela, l'ancienne adresse reste active.

---

## 6. Vérifier, puis reporter l'adresse dans Rufus

Redémarrez le serveur et réaffichez son adresse (§ 2) : elle doit être celle
que vous avez choisie. Depuis un poste client, `ping 192.168.1.240` doit
répondre.

Si l'adresse du serveur a changé, il faut la reporter **sur chaque poste
client** : Édition → Paramètres → onglet « Ce poste » → « Réseau local »,
ligne « Emplacement ». Redémarrer Rufus : il doit se connecter.

Rien à faire, en revanche, si vous avez simplement fait réserver par la box
l'adresse que le serveur avait déjà.

---

## En cas d'échec

**Le serveur n'a plus Internet après le réglage.** C'est presque toujours la
passerelle ou le DNS : vérifiez que l'adresse de la box figure bien dans les
deux, une case laissée vide suffit à couper l'accès.

**« Conflit d'adresse IP » ou adresse déjà utilisée.** Un autre appareil occupe
ce numéro : reprenez avec un dernier nombre différent.

**Le serveur répond au ping mais Rufus ne se connecte plus.** L'adresse n'a pas
été mise à jour sur les postes clients (§ 6).

**L'adresse retombe sur l'ancienne après redémarrage.** Sous Linux, la connexion
n'a pas été relancée ; sous Windows et macOS, le réglage a été posé sur l'autre
carte réseau que celle réellement utilisée.
