USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ30;
DELIMITER |
    CREATE PROCEDURE MAJ30()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'SousLaResponsabiliteDe') as chp;
        IF tot=1
        THEN
            ALTER TABLE `UtilisateursConnectes`
            CHANGE COLUMN `SousLaResponsabiliteDe` `UserSuperviseur` INT NULL DEFAULT NULL;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'UserPourLeCompteDe') as chp;
        IF tot=1
        THEN
            ALTER TABLE `UtilisateursConnectes`
            CHANGE COLUMN `UserPourLeCompteDe` `UserComptable` INT NULL DEFAULT NULL;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'UserParent') as chp;
        IF tot=0
        THEN
            ALTER TABLE `UtilisateursConnectes`
            ADD COLUMN `UserParent` INT NULL DEFAULT NULL COMMENT 'Id du user pour qui l acte est effectué\n(User remplacé si le superviseur est remplacant - user superviseur sinon)' AFTER `UserComptable`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'PourLeCompteDe') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Actes`
            CHANGE COLUMN `PourLeCompteDe` `UserComptable` INT NULL DEFAULT NULL COMMENT 'id du user qui comptabilise cet acte\n-1 = acte sans compta' ;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'UserParent') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Actes`
            ADD COLUMN `UserParent` INT NULL DEFAULT NULL COMMENT 'Id du user pour qui l acte est effectué\n(User remplacé si le superviseur est remplacant - user superviseur sinon)' AFTER `UserComptable`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'SuperViseurRemplacant') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Actes`
            ADD COLUMN `SuperViseurRemplacant` INT NULL DEFAULT NULL COMMENT '1= acte effectue par un remplacant' AFTER `UserParent`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'NomLieu') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Actes`
            ADD COLUMN `NomLieu` VARCHAR(80) NULL DEFAULT NULL AFTER `NumCentre`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'Medecin') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Utilisateurs`
            ADD COLUMN `Medecin` INT(1) NULL DEFAULT NULL AFTER `Soignant`;
        END IF;
        UPDATE `Actes` SET UserParent = idUser, UserComptable = idUser;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'idComptable') as chp;
        IF tot=1
        THEN
            ALTER TABLE `salledattente`
            DROP COLUMN `idComptable`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'verrouillepar') as chp;
        IF tot=1
        THEN
            ALTER TABLE `salledattente`
            DROP COLUMN `verrouillepar`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='LieuxExercice' AND COLUMN_NAME = 'NomLieu') as chp;
        IF tot=1
        THEN
            ALTER TABLE `LieuxExercice`
            CHANGE COLUMN `NomLieu` `NomLieu` VARCHAR(80) NULL DEFAULT NULL;
        ELSE
            ALTER TABLE `LieuxExercice`
            ADD COLUMN `NomLieu` VARCHAR(80) NULL DEFAULT NULL AFTER `idLieu`;
        END IF;
        END|
CALL MAJ30();
DROP PROCEDURE MAJ30;

USE `Comptamedicale`;
DROP TABLE IF EXISTS `ComptesJointures`;
DROP TABLE IF EXISTS `verrouscompta`;
DROP TABLE IF EXISTS `verrouscomptapaiements`;

ALTER TABLE ComptaMedicale.autresrecettes COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.banques COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.Comptes COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.lignescomptes COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.Depenses COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.lignespaiements COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.lignesrecettes COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.remisecheques COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.rubriques2035 COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.tiers COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.typepaiementactes COLLATE 'utf8_general_ci';
ALTER TABLE ComptaMedicale.verrouscomptaactes COLLATE 'utf8_general_ci';

ALTER TABLE Ophtalmologie.commentaireslunettes COLLATE 'utf8_general_ci';
ALTER TABLE Ophtalmologie.biometries COLLATE 'utf8_general_ci';
ALTER TABLE Ophtalmologie.DonneesOphtaPatients COLLATE 'utf8_general_ci';
ALTER TABLE Ophtalmologie.pachymetries COLLATE 'utf8_general_ci';
ALTER TABLE Ophtalmologie.refractions COLLATE 'utf8_general_ci';
ALTER TABLE Ophtalmologie.speculaires COLLATE 'utf8_general_ci';
ALTER TABLE Ophtalmologie.tonometries COLLATE 'utf8_general_ci';


ALTER TABLE Rufus.Actes COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.appareilsconnectescentre COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.AppareilsRefraction COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.ccam COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.correspondants COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.donneessocialespatients COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.flags COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.gestionfichiers COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.Impressions COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.jointuresdocuments COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.jointuresLieux COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.LieuxExercice COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.listeappareils COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.listetiers COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.Messagerie COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.MessagerieJointures COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.Metadocuments COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.MotifsRDV COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.MotsCles COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.MotsClesJointures COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.ParametresSysteme COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.renseignementsmedicauxpatients COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.salledattente COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.specialitesmedicales COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.Utilisateurs COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.UtilisateursConnectes COLLATE 'utf8_general_ci';
ALTER TABLE Rufus.Villes COLLATE 'utf8_general_ci';

UPDATE `Depenses` SET COMPTE = NULL WHERE ModePaiement = 'E';

ALTER DATABASE `Rufus` CHARACTER SET utf8;
ALTER DATABASE `Ophtalmologie` CHARACTER SET utf8;
ALTER DATABASE `Comptamedicale` CHARACTER SET utf8;

USE `Rufus`;
DROP TABLE IF EXISTS `listespecialites`;

UPDATE `ParametresSysteme` SET VersionBase = 30;
