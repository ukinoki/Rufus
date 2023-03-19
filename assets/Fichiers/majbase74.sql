-- 20/03/2023


ALTER TABLE `rufus`.`LieuxExercice` 
	CHANGE COLUMN `LieuCodePostal` `LieuCodePostal` VARCHAR(10) NULL DEFAULT NULL ;
ALTER TABLE `rufus`.`correspondants` 
	CHANGE COLUMN `CorCodepostal` `CorCodepostal` VARCHAR(10) NULL DEFAULT NULL ,
	CHANGE COLUMN `CorVille` `CorVille` VARCHAR(45) NULL DEFAULT NULL ;
ALTER TABLE `rufus`.`donneessocialespatients` 
	CHANGE COLUMN `PatCodepostal` `PatCodepostal` VARCHAR(10) NULL DEFAULT NULL ,
	CHANGE COLUMN `PatVille` `PatVille` VARCHAR(45) NULL DEFAULT NULL ;
ALTER TABLE `rufus`.`Manufacturers` 
	CHANGE COLUMN `ManCodepostal` `ManCodepostal` VARCHAR(10) NULL DEFAULT NULL ,
	CHANGE COLUMN `ManVille` `ManVille` VARCHAR(45) NULL DEFAULT NULL ;
ALTER TABLE `rufus`.`utilisateurs` 
	CHANGE COLUMN `UserCodePostal` `UserCodePostal` VARCHAR(10) NULL DEFAULT NULL ;
ALTER TABLE `ComptaMedicale`.`tiers` 
	CHANGE COLUMN `Codepostaltiers` `Codepostaltiers` VARCHAR(10) CHARACTER SET 'utf8' NULL DEFAULT NULL ,
	CHANGE COLUMN `Villetiers` `Villetiers` VARCHAR(45) CHARACTER SET 'utf8' NULL DEFAULT NULL ;
ALTER TABLE `rufus`.`AutresVilles` 
	CHANGE COLUMN `CodePostal` `CodePostal` VARCHAR(10) CHARACTER SET 'utf8' NULL DEFAULT NULL;
ALTER TABLE `rufus`.`Villes` 
	CHANGE COLUMN `codePostal` `CodePostal` VARCHAR(10) CHARACTER SET 'utf8' NULL DEFAULT NULL;

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ74;
DELIMITER |
CREATE PROCEDURE MAJ74()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Villes' AND COLUMN_NAME = 'ville') as chp;
        IF tot=1
        THEN
			ALTER TABLE `rufus`.`Villes` 
			CHANGE COLUMN `ville` `Nom` VARCHAR(45) CHARACTER SET 'utf8' NULL DEFAULT NULL ;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 74;
END|

CALL MAJ74();
DROP PROCEDURE IF EXISTS MAJ74;
