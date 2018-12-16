-- 16/12/2018
USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ49;
DELIMITER |
    CREATE PROCEDURE MAJ49()
    BEGIN
        DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT COLUMN_KEY
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME = 'EchangeImages' AND COLUMN_NAME = 'Facture') as chp;
        IF tot=0
            THEN
                ALTER TABLE `Images`.`EchangeImages`
                ADD COLUMN `Facture` INT(1) NULL DEFAULT NULL AFTER `Compression`;
        END IF;
        DROP TABLE IF EXISTS `Factures`;
        CREATE TABLE `Factures` (
            `idFacture` INT NOT NULL AUTO_INCREMENT,
            `LienFichier` VARCHAR(150) NULL,
            `Echeancier` INT(1) NULL,
            PRIMARY KEY (`idFacture`)
            ) ENGINE=InnoDB;
        UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 49;
    END|
CALL MAJ49();
DROP PROCEDURE IF EXISTS MAJ49;

