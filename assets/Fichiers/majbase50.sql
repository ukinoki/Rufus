-- 16/12/2018
USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ50;
DELIMITER |
    CREATE PROCEDURE MAJ50()
    BEGIN
    DROP TABLE IF EXISTS `Factures`;
    CREATE TABLE `Factures` (
        `idFacture` INT NOT NULL AUTO_INCREMENT,
        `LienFichier` VARCHAR(250) NULL,
        `Echeancier` INT(1) NULL,
        `idUser` INT NULL,
        PRIMARY KEY (`idFacture`)
        ) ENGINE=InnoDB;
        UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 50;
    END|
CALL MAJ50();
DROP PROCEDURE IF EXISTS MAJ50;

