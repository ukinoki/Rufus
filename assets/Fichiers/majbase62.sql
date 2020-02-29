-- 27/02/2020
USE `Ophtalmologie`;
DROP PROCEDURE IF EXISTS MAJ62;
DELIMITER |
    CREATE PROCEDURE MAJ62()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'ProgDate') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            DROP COLUMN `ProgDate`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'idLieu') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            CHANGE COLUMN `idLieu` `idSession` INT(11) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'idUser') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            DROP COLUMN `idUser`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'ProgHeure') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            ADD COLUMN `ProgHeure` TIME(0) NULL AFTER `idLigneProgramme`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'LieuCouleur') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`LieuxExercice`
            ADD COLUMN `LieuCouleur` VARCHAR(6) NULL AFTER `LieuFax`;
        END IF;
    DROP TABLE IF EXISTS `SessionsOperatoires`;
    CREATE TABLE `Ophtalmologie`.`SessionsOperatoires` (
        `idSession` INT NOT NULL,
        `idUser` INT NULL,
        `idAide` INT NULL,
        `DateSession` DATE NULL,
        `idLieu` INT NULL,
        PRIMARY KEY (`idSession`)
        ) ENGINE=InnoDB;
END|

CALL MAJ62();
DROP PROCEDURE IF EXISTS MAJ62;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 62;
