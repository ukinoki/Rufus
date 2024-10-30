-- 30/10/2024

USE `Ophtalmologie`;
DROP PROCEDURE IF EXISTS MAJ79;
DELIMITER |
CREATE PROCEDURE MAJ79()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'idiolcon') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `idiolcon` INT NULL DEFAULT NULL AFTER `idmanufacturer`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'AddIntermediate') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `AddIntermediate` DOUBLE NULL DEFAULT NULL AFTER `CylindrePas`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'AddNear') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `AddNear` DOUBLE NULL DEFAULT NULL AFTER `AddIntermediate`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'HofferQ') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `HofferQ` DOUBLE NULL DEFAULT NULL AFTER `Holladay1sf`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'BarrettLF') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
                        ADD COLUMN `BarrettLF` DOUBLE NULL DEFAULT NULL AFTER `HofferQ`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'BarrettDF') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
                        ADD COLUMN `BarrettDF` DOUBLE NULL DEFAULT NULL AFTER `BarrettLF`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Olsen') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
                        ADD COLUMN `Olsen` DOUBLE NULL DEFAULT NULL AFTER `BarrettDF`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'SinglePiece') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `SinglePiece` INT NULL DEFAULT NULL AFTER `TypeImage`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'HaptiqueMateriau') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `HaptiqueMateriau` VARCHAR(45) NULL DEFAULT NULL AFTER `Materiau`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'ResultsULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `ResultsULIB` INT NULL DEFAULT NULL AFTER `TypeImplant`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'CsteAEchoULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `CsteAEchoULIB` DOUBLE NULL DEFAULT NULL AFTER `ResultsULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'CsteAOptiqueULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `CsteAOptiqueULIB` DOUBLE NULL DEFAULT NULL AFTER `CsteAEchoULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Haigisa0ULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Haigisa0ULIB` DOUBLE NULL DEFAULT NULL AFTER `CsteAOptiqueULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Haigisa1ULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Haigisa1ULIB` DOUBLE NULL DEFAULT NULL AFTER `Haigisa0ULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Haigisa2ULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Haigisa2ULIB` DOUBLE NULL DEFAULT NULL AFTER `Haigisa1ULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Holladay1sfULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Holladay1sfULIB` DOUBLE NULL DEFAULT NULL AFTER `Haigisa2ULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'HofferQULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `HofferQULIB` DOUBLE NULL DEFAULT NULL AFTER `Holladay1sfULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'BarrettLFULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
                        ADD COLUMN `BarrettLFULIB` DOUBLE NULL DEFAULT NULL AFTER `HofferQULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'BarrettDFULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
                        ADD COLUMN `BarrettDFULIB` DOUBLE NULL DEFAULT NULL AFTER `BarrettLFULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'OlsenULIB') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
                        ADD COLUMN `OlsenULIB` DOUBLE NULL DEFAULT NULL AFTER `BarrettDFULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'ResultsOptimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `ResultsOptimized` INT NULL DEFAULT NULL AFTER `OlsenULIB`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'CsteAOptiqueOptimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `CsteAOptiqueOptimized` DOUBLE NULL DEFAULT NULL AFTER `ResultsOptimized`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Haigisa0Optimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Haigisa0Optimized` DOUBLE NULL DEFAULT NULL AFTER `CsteAOptiqueOptimized`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Haigisa1Optimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Haigisa1Optimized` DOUBLE NULL DEFAULT NULL AFTER `Haigisa0Optimized`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Haigisa2Optimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Haigisa2Optimized` DOUBLE NULL DEFAULT NULL AFTER `Haigisa1Optimized`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'Holladay1sfOptimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `Holladay1sfOptimized` DOUBLE NULL DEFAULT NULL AFTER `Haigisa2Optimized`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'HofferQOptimized') as chp;
        IF tot=0
        THEN
			ALTER TABLE `Ophtalmologie`.`IOLs` 
			ADD COLUMN `HofferQOptimized` DOUBLE NULL DEFAULT NULL AFTER `Holladay1sfOptimized`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'VersionBaseIOL') as chp;
        IF tot=0
        THEN
			ALTER TABLE `rufus`.`ParametresSysteme` 
			ADD COLUMN `VersionBaseIOL` DOUBLE NULL DEFAULT 1 AFTER `VersionBase`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 79, VersionBaseIOL = 1;
END|

CALL MAJ79();
DROP PROCEDURE IF EXISTS MAJ79;
