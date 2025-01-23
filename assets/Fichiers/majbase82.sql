-- 18/01/2025

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ82;
DELIMITER |
CREATE PROCEDURE MAJ82()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UsersBarCode1') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UsersBarCode1`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UseNum') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`utilisateurs`
                ADD COLUMN `UseNum` INT NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'jointuresLieux' AND COLUMN_NAME = 'AMnumber') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`jointuresLieux`
                ADD COLUMN `AMnumber` BIGINT NULL DEFAULT NULL AFTER `idLieu`;
        END IF;
    UPDATE `rufus`.`utilisateurs` SET UseNum = 1;
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 82;
END|
CALL MAJ82();
DROP PROCEDURE IF EXISTS MAJ82;
