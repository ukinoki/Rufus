-- 22/05/2020

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ68;
DELIMITER |
CREATE PROCEDURE MAJ68()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Sessions' AND COLUMN_NAME = 'DateSession') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`Sessions`
            DROP COLUMN `DateSession`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Sessions' AND COLUMN_NAME = 'DebutSession') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Sessions`
            ADD COLUMN `DebutSession` DATETIME NULL AFTER `idLieu`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Sessions' AND COLUMN_NAME = 'FinSession') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Sessions`
            ADD COLUMN `FinSession` DATETIME NULL AFTER `DebutSession`;
        END IF;
    DROP TABLE IF EXISTS `JournalSessions`;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 68;
END|

CALL MAJ68();
DROP PROCEDURE IF EXISTS MAJ68;

