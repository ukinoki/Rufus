-- 16/12/2018
USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ50;
DELIMITER |
    CREATE PROCEDURE MAJ50()
    BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Depenses' AND COLUMN_NAME = 'idFacture') as chp;
    IF tot=0
        THEN
            ALTER TABLE `Depenses`
            ADD COLUMN `idFacture` INT(11) NULL DEFAULT NULL AFTER `Nocheque`;
    END IF;
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

