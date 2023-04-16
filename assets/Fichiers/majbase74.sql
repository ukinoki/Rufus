-- 20/03/2023

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ74;
DELIMITER |
CREATE PROCEDURE MAJ74()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'Codepostaltiers') as chp;
        IF tot=1
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            CHANGE COLUMN `Codepostaltiers` `Codepostaltiers` VARCHAR(10) CHARACTER SET 'utf8' NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tiers' AND COLUMN_NAME = 'Villetiers') as chp;
        IF tot=1
        THEN
            ALTER TABLE `ComptaMedicale`.`tiers`
            CHANGE COLUMN `Villetiers` `Villetiers` VARCHAR(45) CHARACTER SET 'utf8' NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'AutresVilles' AND COLUMN_NAME = 'CodePostal') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`AutresVilles`
            CHANGE COLUMN `CodePostal` `CodePostal` VARCHAR(10) CHARACTER SET 'utf8' NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Villes' AND COLUMN_NAME = 'codePostal') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`Villes`
            CHANGE COLUMN `codePostal` `CodePostal` VARCHAR(10) CHARACTER SET 'utf8' NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'ManCodepostal') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            CHANGE COLUMN `ManCodepostal` `ManCodepostal` VARCHAR(10) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'ManVille') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            CHANGE COLUMN `ManVille` `ManVille` VARCHAR(45) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'donneessocialespatients' AND COLUMN_NAME = 'PatCodepostal') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`donneessocialespatients`
            CHANGE COLUMN `PatCodepostal` `PatCodepostal` VARCHAR(10) NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'donneessocialespatients' AND COLUMN_NAME = 'PatVille') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`donneessocialespatients`
            CHANGE COLUMN `PatVille` `PatVille` VARCHAR(45) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'correspondants' AND COLUMN_NAME = 'CorCodePostal') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`correspondants`
            CHANGE COLUMN `CorCodepostal` `CorCodepostal` VARCHAR(10) NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'correspondants' AND COLUMN_NAME = 'CorVille') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`correspondants`
            CHANGE COLUMN `CorVille` `CorVille` VARCHAR(45) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'LieuCodePostal') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`LieuxExercice`
            CHANGE COLUMN `LieuCodePostal` `LieuCodePostal` VARCHAR(10) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Villes' AND COLUMN_NAME = 'ville') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`Villes`
            CHANGE COLUMN `ville` `Nom` VARCHAR(45) CHARACTER SET 'utf8' NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Villes' AND COLUMN_NAME = 'pk') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`Villes`
            CHANGE COLUMN `pk` `id` INT(11) NOT NULL AUTO_INCREMENT FIRST;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'VillesFrance') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`ParametresSysteme`
            ADD COLUMN `VillesFrance` INT(1) NULL DEFAULT 1 AFTER `DirBkup`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'CotationsFrance') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`ParametresSysteme`
            ADD COLUMN `CotationsFrance` INT(1) NULL DEFAULT 1 AFTER `VillesFrance`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ComptaFrance') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`ParametresSysteme`
            ADD COLUMN `ComptaFrance` INT(1) NULL DEFAULT 1 AFTER `CotationsFrance`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 74;
END|

CALL MAJ74();
DROP PROCEDURE IF EXISTS MAJ74;
