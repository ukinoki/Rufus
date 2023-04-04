-- 03/04/2023


ALTER TABLE `rufus`.`utilisateurs` 
CHANGE COLUMN `Soignant` `Soignant` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = ophtalmo\n2 = orthoptiste\n3 = autre\n4 = Non soignant\n5 = societe comptable\n6 = neutre (poste ne modifiant pas les donnees et sans affichage des donnees medicales)' ,
CHANGE COLUMN `UserEnregHonoraires` `UserEnregHonoraires` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = liberal\n2 = salarie\n3 = retrocession\n4 = pas de comptabilite\n5 = liberal en SEL' ;


USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ75;
DELIMITER |
CREATE PROCEDURE MAJ75()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'idCompteEncaissHonoraires') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `idCompteEncaissHonoraires`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserAdresse1') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserAdresse1`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserAdresse2') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserAdresse2`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserAdresse3') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserAdresse3`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserCodePostal') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserCodePostal`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserVille') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserVille`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserPoste') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserPoste`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserFax') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserFax`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserWeb') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `UserWeb`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 75;
END|

CALL MAJ75();
DROP PROCEDURE IF EXISTS MAJ75;
