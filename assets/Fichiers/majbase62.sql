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
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'TypeAnesthesie') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            CHANGE COLUMN `TypeAnesthesie` `TypeAnesthesie` VARCHAR(1) NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'idActe') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            ADD COLUMN `idActe` INT(11) NULL DEFAULT NULL AFTER `Observation`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ProgrammesOperatoires' AND COLUMN_NAME = 'Incident') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Ophtalmologie`.`ProgrammesOperatoires`
            ADD COLUMN `Incident` LONGTEXT NULL DEFAULT NULL AFTER `idActe`;
    END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'SessionsOperatoires') as chp1;
        IF tot=1
        THEN
            SELECT COUNT(*) INTO tot FROM
                (SELECT COLUMN_KEY
                FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME = 'SessionsOperatoires' AND COLUMN_NAME = 'Incident') as chp2;
                IF tot=0
                THEN
                    ALTER TABLE `Ophtalmologie`.`SessionsOperatoires`
                    ADD COLUMN `Incident` LONGTEXT NULL DEFAULT NULL AFTER `idLieu`;
                END IF;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'DiametreOptique') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Ophtalmologie`.`IOLs`
            ADD COLUMN `DiametreOptique` DOUBLE NULL DEFAULT NULL AFTER `modelname`,
            ADD COLUMN `DiametreAll` DOUBLE NULL DEFAULT NULL AFTER `DiametreOptique`,
            ADD COLUMN `ACD` DOUBLE NULL DEFAULT NULL AFTER `DiametreAll`,
            ADD COLUMN `PuissanceMin` DOUBLE NULL DEFAULT NULL AFTER `ACD`,
            ADD COLUMN `PuissanceMax` DOUBLE NULL DEFAULT NULL AFTER `PuissanceMin`,
            ADD COLUMN `PuissancePas` DOUBLE NULL DEFAULT NULL AFTER `PuissanceMax`,
            ADD COLUMN `CylindreMin` DOUBLE NULL DEFAULT NULL AFTER `PuissancePas`,
            ADD COLUMN `CylindreMax` DOUBLE NULL DEFAULT NULL AFTER `CylindreMin`,
            ADD COLUMN `CylindrePas` DOUBLE NULL DEFAULT NULL AFTER `CylindreMax`,
            ADD COLUMN `CsteAOptique` DOUBLE NULL DEFAULT NULL AFTER `CylindrePas`,
            ADD COLUMN `CsteAEcho` DOUBLE NULL DEFAULT NULL AFTER `CsteAOptique`,
            ADD COLUMN `Haigisa0` DOUBLE NULL DEFAULT NULL AFTER `CsteAEcho`,
            ADD COLUMN `Haigisa1` DOUBLE NULL DEFAULT NULL AFTER `Haigisa0`,
            ADD COLUMN `Haigisa2` DOUBLE NULL DEFAULT NULL AFTER `Haigisa1`,
            ADD COLUMN `Holladay1sf` DOUBLE NULL DEFAULT NULL AFTER `Haigisa2`,
            ADD COLUMN `DiametreInjecteur` DOUBLE NULL DEFAULT NULL AFTER `Holladay1sf`,
            ADD COLUMN `Image` BLOB NULL DEFAULT NULL AFTER `DiametreInjecteur`,
            ADD COLUMN `Materiau` VARCHAR(45) NULL DEFAULT NULL AFTER `Image`,
            ADD COLUMN `Remarque` VARCHAR(100) NULL DEFAULT NULL AFTER `Materiau`;
            END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'IOLInactif') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Ophtalmologie`.`IOLs`
            ADD COLUMN `IOLInactif` INT(1) NULL DEFAULT NULL AFTER `Remarque`;
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
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'ManInactif') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            ADD COLUMN `ManInactif` INT(1) NULL DEFAULT NULL AFTER `CorTelephone`;
        END IF;
    DROP TABLE IF EXISTS `SessionsOperatoires`;
    CREATE TABLE `Ophtalmologie`.`SessionsOperatoires` (
        `idSession` INT NOT NULL AUTO_INCREMENT,
        `idUser` INT NULL,
        `idAide` INT NULL,
        `DateSession` DATE NULL,
        `idLieu` INT NULL,
        `Incident` LONGTEXT NULL,
        PRIMARY KEY (`idSession`)
        ) ENGINE=InnoDB;
END|

CALL MAJ62();
DROP PROCEDURE IF EXISTS MAJ62;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 62;
