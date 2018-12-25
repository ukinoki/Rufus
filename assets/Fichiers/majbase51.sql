-- 25/12/2018
USE `ComptaMedicale`;
DROP TABLE IF EXISTS `Factures`;
CREATE TABLE `Factures` (
    `idFacture` INT NOT NULL AUTO_INCREMENT,
    `LienFichier` VARCHAR(250) NULL,
    `Intitule` VARCHAR(250) NULL,
    `Echeancier` INT(1) NULL,
    `idUser` INT NULL,
    `pdf` LONGBLOB NULL,
    `jpg` LONGBLOB NULL,
    PRIMARY KEY (`idFacture`)
    ) ENGINE=InnoDB;
USE  `Images`;
DROP TABLE IF EXISTS `FacturesASupprimer`;
CREATE TABLE `FacturesASupprimer` (
    `idFacture` INT NOT NULL,
    PRIMARY KEY (`idFacture`)
    ) ENGINE=InnoDB;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 51;

