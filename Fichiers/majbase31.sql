USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ31;
DELIMITER |
    CREATE PROCEDURE MAJ31()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='utilisateurs' AND COLUMN_NAME = 'OPTAM') as chp;
        IF tot=0
        THEN
            ALTER TABLE `utilisateurs`
            ADD COLUMN `OPTAM` INT NULL DEFAULT NULL AFTER `UserSecteur`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='utilisateurs' AND COLUMN_NAME = 'UserSecteur') as chp;
        IF tot=0
        THEN
            ALTER TABLE `utilisateurs`
            CHANGE COLUMN `UserSecteur` `UserSecteur` TINYINT(1) NULL DEFAULT NULL COMMENT '1= Secteur 1\n2= Secteur2\n3= Secteur3';
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='ccam' AND COLUMN_NAME = 'SecteurIINONCAS') as chp;
        IF tot=1
        THEN
            ALTER TABLE `ccam`
            CHANGE COLUMN `SecteurIINONCAS` `NoOPTAM`  decimal(6,2) NULL DEFAULT NULL;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='ccam' AND COLUMN_NAME = 'SecteurI') as chp;
        IF tot=1
        THEN
            ALTER TABLE `ccam`
            CHANGE COLUMN `SecteurI` `OPTAM`  decimal(6,2) NULL DEFAULT NULL;
        END IF;
    END|
CALL MAJ31();
DROP PROCEDURE MAJ31;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 31;
UNLOCK TABLES;
