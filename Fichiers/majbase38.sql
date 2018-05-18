USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ38;
DELIMITER |
    CREATE PROCEDURE MAJ38()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'TextCorps') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Impressions`
            CHANGE COLUMN `TextCorps` `TextCorps` MEDIUMBLOB NULL DEFAULT NULL ;
        END IF;
     END|
CALL MAJ38();
DROP PROCEDURE MAJ38;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 38;
UNLOCK TABLES;

