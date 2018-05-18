USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ26;
DELIMITER |
    CREATE PROCEDURE MAJ26()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'idSuperViseur') as chp;
        IF tot=0
        THEN
        ALTER TABLE `salledattente`
        ADD COLUMN `idSuperViseur` INT NULL DEFAULT NULL COMMENT 'id soignant responsable de l’acte' AFTER `PosteExamen`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'idComptable') as chp;
        IF tot=0
        THEN
        ALTER TABLE `salledattente`
        ADD COLUMN `idComptable` INT NULL DEFAULT NULL COMMENT 'User qui enregistrera les honoraires' AFTER `idSuperViseur`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'idSalDat') as chp;
        IF tot=0
        THEN
        ALTER TABLE `salledattente`
        ADD COLUMN `idSalDat` INT NULL DEFAULT NULL COMMENT 'id de la salle d’attente' AFTER `idComptable`;
        END IF;
        END|
CALL MAJ26();
DROP PROCEDURE MAJ26;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 26;
UNLOCK TABLES;
