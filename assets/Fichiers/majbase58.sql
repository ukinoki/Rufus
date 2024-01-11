-- 19/10/2019
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
    END|

CALL MAJ58();
DROP PROCEDURE IF EXISTS MAJ58;

DROP TABLE IF EXISTS `Manufacturers`;
CREATE TABLE `Manufacturers` (
  `idManufacturer` int(11) NOT NULL,
  `ManName` varchar(45) NOT NULL,
  `ManAdresse1` varchar(80) DEFAULT NULL,
  `ManAdresse2` varchar(80) DEFAULT NULL,
  `ManAdresse3` varchar(80) DEFAULT NULL,
  `ManCodepostal` varchar(5) DEFAULT NULL,
  `ManVille` varchar(40) DEFAULT NULL,
  `ManTelephone` varchar(17) DEFAULT NULL,
  `ManPortable` varchar(17) CHARACTER SET latin1 DEFAULT NULL,
  `ManMail` varchar(45) CHARACTER SET latin1 DEFAULT NULL,
  `CorNom` varchar(45) DEFAULT NULL,
  `CorPrenom` varchar(45) DEFAULT NULL,
  `CorTelephone` varchar(17) DEFAULT NULL,
  PRIMARY KEY (`idManufacturer`)
) ENGINE=InnoDB;

Use `Ophtalmologie`;
DROP TABLE IF EXISTS `IOLs`;
CREATE TABLE `Ophtalmologie`.`IOLs` (
  `idIOL` INT NOT NULL AUTO_INCREMENT,
  `idmanufacturer` INT NULL,
  `modelname` VARCHAR(45) NULL,
  PRIMARY KEY (`idIOL`)
  ) ENGINE=InnoDB;

DROP TABLE IF EXISTS `ProgrammesOperatoires`;
CREATE TABLE `ProgrammesOperatoires` (
  `idLigneProgramme` INT NOT NULL,
  `ProgDate` DATETIME NULL,
  `idUser` INT NULL,
  `idPat` INT NULL,
  `idLieu` INT NULL,
  `TypeAnesthesie` VARCHAR(45) NULL,
  `TypeIntervention` INT NULL,
  `Cote` INT NULL,
  `idIOL` INT NULL,
  `PwIOL` DECIMAL(4,2) NULL,
  `CylIOL` DECIMAL(4,2) NULL,
  `Observation` VARCHAR(100) NULL,
  PRIMARY KEY (`idLigneProgramme`)
  ) ENGINE=InnoDB;

DROP TABLE IF EXISTS `TypesInterventions`;
CREATE TABLE `TypesInterventions` (
  `idTypeIntervention` INT NOT NULL,
  `TypeIntervention` VARCHAR(80) NULL,
  `CodeIntervention` VARCHAR(20) NULL,
  PRIMARY KEY (`idTypeIntervention`)
  ) ENGINE=InnoDB;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 58;
