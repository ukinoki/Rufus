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
        WHERE TABLE_NAME = 'flags' AND COLUMN_NAME = 'MAJflagUserDistant') as chp;
    IF tot=0
        THEN
            ALTER TABLE `flags`
            ADD COLUMN `flagUserDistant` INT AFTER `MAJflagMG`;
    END IF;
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 58;
    END|

CALL MAJ58();
DROP PROCEDURE IF EXISTS MAJ58;

