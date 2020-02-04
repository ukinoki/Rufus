-- 31/01/2020
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ61;
DELIMITER |
    CREATE PROCEDURE MAJ61()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'MDPAdmin') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`ParametresSysteme`
            CHANGE COLUMN `MDPAdmin` `MDPAdmin` VARCHAR(45) NULL DEFAULT NULL ;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserMDP') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            CHANGE COLUMN `UserMDP` `UserMDP` VARCHAR(45) NULL DEFAULT NULL ;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserLogin') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            CHANGE COLUMN `UserLogin` `UserLogin` VARCHAR(15) NULL DEFAULT NULL ;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'DateCreationMDP') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            ADD COLUMN `DateCreationMDP` DATE NULL DEFAULT NULL AFTER `DateDerniereConnexion`;
            UPDATE `rufus`.`utilisateurs` SET DateCreationMDP = NOW();
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserBarCode') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            ADD COLUMN `UserBarCode` BLOB NULL DEFAULT NULL AFTER `DateCreationMDP`;
        END IF;

END|

CALL MAJ61();
DROP PROCEDURE IF EXISTS MAJ61;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 61;
