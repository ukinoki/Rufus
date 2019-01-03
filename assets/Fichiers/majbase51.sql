-- 25/12/2018
USE `ComptaMedicale`;
DROP TABLE IF EXISTS `Factures`;
CREATE TABLE `Factures` (
    `idFacture` INT NOT NULL AUTO_INCREMENT,
    `DateFacture` date DEFAULT NULL,
    `LienFichier` VARCHAR(250) NULL,
    `Intitule` VARCHAR(250) NULL,
    `Echeancier` INT(1) NULL,
    `idDepense` INT NULL,
    `pdf` LONGBLOB NULL,
    `jpg` LONGBLOB NULL,
    PRIMARY KEY (`idFacture`)
    ) ENGINE=InnoDB;
USE  `Images`;
DROP TABLE IF EXISTS `FacturesASupprimer`;
CREATE TABLE `FacturesASupprimer` (
    `LienFichier` VARCHAR(250) NULL
    ) ENGINE=InnoDB;
DROP TABLE IF EXISTS `EchangeImages`;
CREATE TABLE `EchangeImages` (
    `idImpression` INT NOT NULL,
    `pdf` LONGBLOB NULL,
    `jpg` LONGBLOB NULL,
    `Compression` INT(1) NULL,
    `Facture` INT(1) NULL
    ) ENGINE=InnoDB;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 51;

