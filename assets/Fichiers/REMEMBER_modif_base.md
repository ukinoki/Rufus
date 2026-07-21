# Modifications à apporter au prochain majbase (pense-bête)

Ce fichier recense les changements de schéma à intégrer dans le **prochain**
script `majbaseXX.sql` (avec le bump de `VERSION_BASE` dans `macros.h`).
Il n'est **pas** exécuté automatiquement : c'est une simple liste d'attente
pour ne pas perdre les petites modifs qui ne justifient pas, à elles seules,
une nouvelle version de base.

## En attente

- **Table `utilisateurs`, nouveau champ `SignatureAuto` (bool / tinyint).**
  Raison : le drapeau « signer automatiquement les courriers et ordonnances »
  est pour l'instant stocké dans `rufus.ini` (`Param_Poste_SignatureAuto`),
  donc **lié au poste** et non à l'utilisateur — sur un autre poste il faut le
  régler à nouveau. Le déplacer dans `utilisateurs` le ferait suivre
  l'utilisateur partout (cohérent avec la signature elle-même, déjà en base
  dans `utilisateurs.Signature`). Une fois le champ créé : le lire/écrire dans
  `dlg_gestionusers` (checkbox `SignatureAutoCheckBox`) et dans `dlg_impressions`
  à la place de la clé rufus.ini.

- **Table `Ophtalmologie.IOLs`, colonne `modelname` : passer de `varchar(45)`
  à `varchar(60)`.**
  Raison : certains noms d'implants du fichier IOLexport (IOLCon) dépassent
  45 caractères — ex. « Clareon Vivity Toric AutonoMe CNAET2-T6/CCAET2-T6 »
  (49 car.) dans IOLexport 2.3. MySQL tourne en `STRICT_TRANS_TABLES` : un nom
  trop long fait **échouer** l'INSERT au lieu d'être tronqué.
  En attendant, l'import tronque à 45 (`dlg_listeiols.cpp`, `ImportListeIOLS`,
  `iol.setmodele(...left(45))`). Une fois la colonne élargie à 60, relever la
  troncature de `.left(45)` à `.left(60)`.

- **Table `ParametresSysteme`, nouveau champ `VersionCCAM`.**
  Raison : mémoriser en base la version de la nomenclature CCAM chargée, sur le
  modèle des champs de version déjà présents (`VersionBase`, `VersionBaseIOL`).
  Une fois le champ créé : ajouter la macro `CP_VERSIONCCAM_PARAMSYSTEME`
  (`"VersionCCAM"`) dans `macros.h`, à côté des autres colonnes de
  `ParametresSysteme`, puis le lire/écrire là où la version CCAM est gérée.
