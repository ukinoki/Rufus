# Se connecter au cabinet depuis l'extérieur (redirection de port)

Rufus sait travailler **en accès distant** : le poste du domicile se connecte à
la base restée au cabinet. Mais depuis l'extérieur, on ne voit aucune des
machines du cabinet — on ne voit que la **box**, qui porte une seule adresse
publique et qui, par défaut, refuse tout ce qui vient de dehors.

La redirection de port est la consigne que l'on donne à la box : *« ce qui
arrive sur le port 3306, passe-le au serveur »*. Sans elle, le poste distant
frappe à une porte qui ne s'ouvre jamais.

> **Un préalable.** Le serveur doit d'abord avoir une **adresse fixe sur le
> réseau local** (voir la fiche *Donner une adresse fixe au poste serveur*).
> Une redirection pointe vers une adresse : si celle du serveur change un
> matin, la redirection envoie les connexions à une autre machine.

> **Faites-vous aider.** Ce réglage n'a rien de spécifique à Rufus et n'importe
> quelle IA (ChatGPT, Claude, Gemini…) le connaît parfaitement. Donnez-lui la
> marque de votre box et le système du serveur : vous gagnerez du temps.

Dans les exemples ci-dessous, la box est en `192.168.1.1` et le serveur en
`192.168.1.240`.

---

## 1. Connaître l'adresse publique du cabinet

Depuis un poste du cabinet, ouvrez un navigateur et cherchez « mon adresse IP » :
le premier site venu vous l'affiche. C'est elle que le poste distant devra
appeler.

**Le problème, c'est qu'elle change.** L'opérateur la renouvelle de temps à
autre, et le jour où elle change, le poste distant ne trouve plus rien. Deux
réponses :

- **Le nom de domaine dynamique (DynDNS)** — la plupart des box en offrent un
  gratuitement (rubrique « DynDNS » ou « Nom de domaine »). La box prévient le
  service à chaque changement d'adresse. On obtient un nom du genre
  `moncabinet.freeboxos.fr`, que l'on saisira dans Rufus **à la place** de
  l'adresse. C'est la bonne solution : on l'écrit une fois, elle ne bouge plus.
- **Une adresse IP fixe**, à demander à l'opérateur — souvent proposée sur les
  offres professionnelles, parfois payante.

---

## 2. Créer la redirection sur la box

1. Ouvrir `http://192.168.1.1` dans un navigateur et se connecter à la box (le
   mot de passe est généralement sur son étiquette).
2. Chercher la rubrique **« Redirections de ports »**, « NAT/PAT » ou « Gestion
   des ports » — le nom change selon la marque.
3. Créer une règle :

   | Champ (les intitulés varient)       | Valeur          |
   |-------------------------------------|-----------------|
   | Protocole                           | TCP             |
   | Port externe (ou port source)       | `3306`          |
   | Adresse IP de destination           | `192.168.1.240` |
   | Port interne (ou port destination)  | `3306`          |
   | Règle active                        | oui             |

4. Valider. Certaines box demandent un redémarrage pour appliquer la règle.

Rufus n'accepte que **3306 ou 3307** comme port de connexion : la redirection
doit donc aboutir sur l'un des deux. Si votre box autorise un port externe
différent du port interne, `3307` vers `3306` convient très bien.

---

## 3. Laisser passer le pare-feu du serveur

La box transmet la connexion, encore faut-il que le serveur l'accepte.

| Système du serveur | À faire                                                       |
|--------------------|---------------------------------------------------------------|
| Linux (Ubuntu)     | rien : Rufus ouvre le port à l'installation (`ufw allow 3306`) |
| Windows            | créer une règle d'entrée (ci-dessous)                          |
| macOS              | rien, sauf si vous avez activé le pare-feu (ci-dessous)        |

**Windows** — Menu Démarrer, taper « Pare-feu Windows Defender avec
fonctions avancées de sécurité » → **Règles de trafic entrant** → **Nouvelle
règle** → **Port** → TCP, port spécifique `3306` → **Autoriser la connexion** →
cocher les trois profils → nommer la règle `MySQL Rufus`.

**macOS** — Réglages Système → Réseau → **Pare-feu**. S'il est désactivé (cas
par défaut), il n'y a rien à faire. S'il est activé : **Options…** → `+` →
ajouter `mysqld` et le passer à « Autoriser les connexions entrantes ».

---

## 4. Vérifier — depuis l'extérieur, pas du cabinet

Le test doit se faire **hors du réseau du cabinet** : sur un portable connecté
au partage de connexion d'un téléphone, par exemple. Testé depuis le cabinet, il
échoue le plus souvent alors que tout est correct, car beaucoup de box ne savent
pas se rappeler elles-mêmes.

- Depuis un portable Windows, dans PowerShell :
  `Test-NetConnection moncabinet.freeboxos.fr -Port 3306`
- Depuis un portable macOS ou Linux, dans un terminal :
  `nc -vz moncabinet.freeboxos.fr 3306`

La réponse doit indiquer que la connexion réussit (`TcpTestSucceeded : True`, ou
`succeeded!`).

---

## 5. Renseigner le poste distant dans Rufus

Édition → Paramètres → onglet « Ce poste » → **« Accès distant »** :

- **Emplacement** : le nom de domaine dynamique (ou l'adresse publique)
- **Port** : celui de la redirection
- **Mot de passe** : celui de la base
- **Emplacement des clés client SSL** : le dossier où vous avez copié les clés

Ces clés s'exportent **depuis le serveur** sur une clé USB : Édition →
Paramètres → onglet « Ce poste » → « Monoposte » → **Exporter les clés client
SSL**. Sans elles, la connexion distante est refusée.

---

## Ce que vous ouvrez, et ce qui protège

Une redirection de port rend le serveur joignable depuis n'importe où dans le
monde : c'est le prix de l'accès à distance, et il faut le savoir. Rufus impose
pour les connexions distantes un compte MySQL qui **exige les clés SSL** : le
mot de passe seul, même volé, ne sert à rien sans elles, et les échanges sont
chiffrés.

Deux réflexes malgré tout : refermer la redirection quand elle ne sert plus, et
la refaire si vous changez de box.

---

## En cas d'échec

Reprenez dans cet ordre — chaque étape ne se pose que si la précédente répond.

**Le test depuis l'extérieur ne répond pas.** Vérifiez successivement que la
règle est bien enregistrée *et active* sur la box, que l'adresse de destination
est toujours celle du serveur, puis le pare-feu du serveur (§ 3).

**Le serveur fonctionne-t-il seulement en réseau local ?** Si les postes du
cabinet eux-mêmes ne se connectent pas, le problème n'est pas la redirection :
inutile de chercher côté box.

**Ça marchait, ça ne marche plus.** L'adresse publique du cabinet a changé :
c'est exactement ce que le nom de domaine dynamique évite (§ 1).

**Rien ne passe, quoi que vous fassiez.** Certains opérateurs font partager une
même adresse publique à plusieurs abonnés (fréquent sur les box 4G/5G et
certaines offres fibre) : aucune redirection n'est alors possible. Demandez-leur
une adresse IPv4 dédiée.
