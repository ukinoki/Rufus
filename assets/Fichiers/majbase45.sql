USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ45;
DELIMITER |
    CREATE PROCEDURE MAJ45()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='lignescomptes' AND COLUMN_NAME = 'idRemCheq') as chp;
        IF tot=0
        THEN
            ALTER TABLE `lignescomptes` 
            ADD COLUMN `idRemCheq` INT NULL DEFAULT NULL COMMENT 'id de la remise de cheques correspondante' AFTER `idRecSpec`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='archivesbanques' AND COLUMN_NAME = 'idRemCheq') as chp;
        IF tot=0
        THEN
            ALTER TABLE `archivesbanques`
            ADD COLUMN `idRemCheq` INT NULL DEFAULT NULL COMMENT 'id de la remise de cheques correspondante' AFTER `idRecSpec`;
        END IF;
        update `lignescomptes` set idRemCheq = -1 where LigneTypeOperation = 'Remise de chèques';
        update `archivesbanques` set idRemCheq = -1 where LigneTypeOperation = 'Remise de chèques';
    END|
CALL MAJ45();
DROP PROCEDURE IF EXISTS MAJ45;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 45;
