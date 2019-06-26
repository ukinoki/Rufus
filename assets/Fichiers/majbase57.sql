-- 15/06/2019
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ57;
DELIMITER |
    CREATE PROCEDURE MAJ57()
    BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'UtiliseTCP') as chp;
    IF tot=1
        THEN
            ALTER TABLE `ParametresSysteme`
            DROP COLUMN `UtiliseTCP`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'PortTCPServeur') as chp;
    IF tot=1
        THEN
            ALTER TABLE `ParametresSysteme`
            DROP COLUMN `PortTCPServeur`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'UtilisateursConnectes' AND COLUMN_NAME = 'NewIdModifSalDat') as chp;
    IF tot=1
        THEN
            ALTER TABLE `UtilisateursConnectes`
            DROP COLUMN `NewIdModifSalDat`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'UtilisateursConnectes' AND COLUMN_NAME = 'LastIdModifSalDat') as chp;
    IF tot=1
        THEN
            ALTER TABLE `UtilisateursConnectes`
            DROP COLUMN `LastIdModifSalDat`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'UtilisateursConnectes' AND COLUMN_NAME = 'IPAdress') as chp;
    IF tot=0
        THEN
            ALTER TABLE `UtilisateursConnectes`
            ADD COLUMN `IPAdress` VARCHAR(15) NULL AFTER `MACAdressePosteConnecte`;
    END IF;
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 57;
    END|
CALL MAJ57();
DROP PROCEDURE IF EXISTS MAJ57;

