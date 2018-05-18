USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ39;
DELIMITER |
    CREATE PROCEDURE MAJ39()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='cotations' AND COLUMN_NAME = 'MontantConv') as chp;
        IF tot=1
        THEN
            ALTER TABLE `cotations`
            CHANGE COLUMN `MontantConv` `MontantOPTAM` DECIMAL(6,2) NULL DEFAULT NULL ,
            ADD COLUMN `MontantNonOPTAM` DECIMAL(6,2) NULL DEFAULT NULL AFTER `MontantOPTAM`;
            UPDATE `cotations` SET MontantNonOPTAM = MontantOPTAM WHERE CCAM >1;
        END IF;
     END|
CALL MAJ39();
DROP PROCEDURE MAJ39;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 39;
UNLOCK TABLES;

