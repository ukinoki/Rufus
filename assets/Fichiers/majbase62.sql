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
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'Cote') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            CHANGE COLUMN `Cote` `Cote` VARCHAR(1) NULL DEFAULT NULL ;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'idLigneProgramme') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            CHANGE COLUMN `idLigneProgramme` `idLigneProgramme` INT(11) NOT NULL AUTO_INCREMENT;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'TypesInterventions' AND COLUMN_NAME = 'idTypeIntervention') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`TypesInterventions`
            CHANGE COLUMN `idTypeIntervention` `idTypeIntervention` INT(11) NOT NULL AUTO_INCREMENT;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'TypesInterventions' AND COLUMN_NAME = 'DureeIntervention') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Ophtalmologie`.`TypesInterventions`
            ADD COLUMN `DureeIntervention` TIME NULL DEFAULT NULL AFTER `CodeIntervention`;
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
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'CorStatut') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            ADD COLUMN `CorStatut` VARCHAR(45) NULL DEFAULT NULL AFTER `CorPrenom`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'CorMail') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            ADD COLUMN `CorMail` VARCHAR(45) NULL DEFAULT NULL AFTER `CorStatut`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'ManFax') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            ADD COLUMN `ManFax` VARCHAR(17) NULL DEFAULT NULL AFTER `ManTelephone`;
        END IF;
    DROP TABLE IF EXISTS `SessionsOperatoires`;
    CREATE TABLE `Ophtalmologie`.`SessionsOperatoires` (
        `idSession` INT NOT NULL AUTO_INCREMENT,
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
