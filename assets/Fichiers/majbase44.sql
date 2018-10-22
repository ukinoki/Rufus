USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ44;
DELIMITER |
    CREATE PROCEDURE MAJ44()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='autresrecettes' AND COLUMN_NAME = 'TireurCheque') as chp;
        IF tot=0
        THEN
            ALTER TABLE `autresrecettes`
            ADD COLUMN `TireurCheque` VARCHAR(45) NULL DEFAULT NULL AFTER `BanqueCheque`,
            ADD COLUMN `DateEnregistrement` DATE NULL DEFAULT NULL COMMENT 'date de la remise en cas de cheque' AFTER `TireurCheque`;
        END IF;
        update `autresrecettes` set idremise = 0 where paiement = 'C';
    END|
CALL MAJ44();
DROP PROCEDURE IF EXISTS MAJ44;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 44;
