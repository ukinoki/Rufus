-- 06/04/2020
USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ63;
DELIMITER |
    CREATE PROCEDURE MAJ63()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'AdresseTiers') as chp;
        IF tot=1
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            CHANGE COLUMN `AdresseTiers` `Adresse1Tiers` VARCHAR(80) NULL DEFAULT NULL ;
	        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'Adresse2Tiers') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            ADD COLUMN `Adresse2Tiers` VARCHAR(80) NULL DEFAULT NULL AFTER `Adresse1Tiers`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'Adresse3Tiers') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            ADD COLUMN `Adresse3Tiers` VARCHAR(80) NULL DEFAULT NULL AFTER `Adresse2Tiers`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'MailTiers') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            ADD COLUMN `MailTiers` VARCHAR(45) NULL DEFAULT NULL AFTER `FaxTiers`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'WebsiteTiers') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            ADD COLUMN `WebsiteTiers` VARCHAR(120) NULL DEFAULT NULL AFTER `MailTiers`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
       (SELECT COLUMN_KEY
       FROM INFORMATION_SCHEMA.COLUMNS
       WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'EDOF') as chp;
       IF tot=0
       THEN
           ALTER TABLE `Ophtalmologie`.`IOLs`
           ADD COLUMN `Toric` INT(1) NULL DEFAULT NULL AFTER `Jaune`,
           ADD COLUMN `EDOF` INT(1) NULL DEFAULT NULL AFTER `Toric`;
           END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 63;
END|

CALL MAJ63();
DROP PROCEDURE IF EXISTS MAJ63;

