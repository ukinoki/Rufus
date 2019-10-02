-- 25/09/2019
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ58;
DELIMITER |
    CREATE PROCEDURE MAJ58()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'flags' AND COLUMN_NAME = 'flagUserDistant') as chp;
    IF tot=0
        THEN
            ALTER TABLE `flags`
            ADD COLUMN `flagUserDistant` INT AFTER `MAJflagMG`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'pachymetries' AND COLUMN_NAME = 'pachyDate') as chp;
    IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`pachymetries`
            CHANGE COLUMN `pachyDate` `pachyDate` DATETIME NULL DEFAULT NULL ;
	END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tonometries' AND COLUMN_NAME = 'TODate') as chp;
    IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`tonometries`
            CHANGE COLUMN `TODate` `TODate` DATETIME NULL DEFAULT NULL ;
	END IF;
	UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 58;
    END|

CALL MAJ58();
DROP PROCEDURE IF EXISTS MAJ58;

