-- 23/04/2020

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ66;
DELIMITER |
CREATE PROCEDURE MAJ66()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'idRufus') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            ADD COLUMN `idRufus` INT(11) NULL DEFAULT NULL AFTER `DistribuePar`;
        END IF;

UPDATE `rufus`.`Manufacturers` SET `idRufus` = `idManufacturer`;        

DROP TABLE IF EXISTS `Commercials`;
CREATE TABLE `rufus`.`Commercials` (
  `idCommercial` INT NOT NULL AUTO_INCREMENT,
  `Nom` VARCHAR(45) NULL,
  `Prenom` VARCHAR(45) NULL,
  `Statut` VARCHAR(45) NULL,
  `Mail` VARCHAR(45) NULL,
  `Telephone` VARCHAR(17) NULL,
  `idManufacturer` INT NULL,
  PRIMARY KEY (`idCommercial`));
  
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 66;
END|

CALL MAJ66();
DROP PROCEDURE IF EXISTS MAJ66;

