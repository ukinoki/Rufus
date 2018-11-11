-- 11/11/2018
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ47;
DELIMITER |
    CREATE PROCEDURE MAJ47()
    BEGIN
        DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='flags' AND COLUMN_NAME = 'flagMGdistant') as chp;
            IF tot=1
            THEN
                ALTER TABLE `flags`
                DROP COLUMN `flagMGdistant`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='flags' AND COLUMN_NAME = 'MAJflagMessages') as chp;
            IF tot=0
            THEN
                ALTER TABLE `flags`
                ADD COLUMN `MAJflagMessages` INT(11) NULL DEFAULT NULL AFTER `MAJflagSalDat`;
            END IF;
        UPDATE `rufus`.`utilisateurs` SET `UserLogin`='Admin' WHERE `UserNom`='Admin';
    END|
    UPDATE `ParametresSysteme` SET VersionBase = 47;
CALL MAJ47();
DROP PROCEDURE IF EXISTS MAJ47;

