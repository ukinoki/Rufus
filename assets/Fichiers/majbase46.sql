USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ46;
DELIMITER |
    CREATE PROCEDURE MAJ46()
    BEGIN
        DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'AdresseTCPServeur') as chp;
            IF tot=0
            THEN
                ALTER TABLE `ParametresSysteme`
                ADD COLUMN `AdresseTCPServeur` VARCHAR(15) NULL DEFAULT NULL COMMENT 'L’adresse du serveur TCP sur le réseau' AFTER `AdresseServeurDistant`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'AccesDistant') as chp;
            IF tot=0
            THEN
                ALTER TABLE `UtilisateursConnectes`
                ADD COLUMN `AccesDistant` INT(1) NULL DEFAULT NULL COMMENT '1 = acces distant' AFTER `NomPosteConnecte`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'LastIdModifSalDat') as chp;
            IF tot=0
            THEN
                ALTER TABLE `UtilisateursConnectes`
                DROP COLUMN `LastIdModifSalDat`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'NewIdModifSalDat') as chp;
            IF tot=0
            THEN
                ALTER TABLE `UtilisateursConnectes`
                DROP COLUMN `NewIdModifSalDat`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'MACAdressePosteConnecte') as chp;
            IF tot=0
            THEN
                ALTER TABLE `UtilisateursConnectes`
                ADD COLUMN `MACAdressePosteConnecte` VARCHAR(45) NULL DEFAULT NULL AFTER `NomPosteConnecte`,
                ADD UNIQUE INDEX `MACAdressePosteConnecte_UNIQUE` (`MACAdressePosteConnecte` ASC);
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='flags' AND COLUMN_NAME = 'flagMGdistant') as chp;
            IF tot=0
            THEN
                ALTER TABLE `flags`
                ADD COLUMN `flagMGdistant` INT(1) NULL DEFAULT NULL COMMENT '1 = le flagMG a été mis à jour par un poste distant' AFTER `MAJflagMG`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='flags' AND COLUMN_NAME = 'MAJflagPatients') as chp;
            IF tot=1
            THEN
                ALTER TABLE `flags`
                CHANGE COLUMN `MAJflagPatients` `MAJflagSalDat` INT(11) NULL DEFAULT NULL ;
            END IF;
        DROP TABLE IF EXISTS `gestionfichiers`;
    END|
    UPDATE `ParametresSysteme` SET VersionBase = 46;
CALL MAJ46();
DROP PROCEDURE IF EXISTS MAJ46;

