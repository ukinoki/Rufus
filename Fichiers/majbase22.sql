USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ22;
DELIMITER |
    CREATE PROCEDURE MAJ22()
    BEGIN
        DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'autre') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Impressions`
            ADD COLUMN `autre` LONGBLOB NULL AFTER `jpg`,
            ADD COLUMN `formatautre` VARCHAR(5) NULL DEFAULT NULL AFTER `autre`;
        END IF;
    END|
CALL MAJ22();
DROP PROCEDURE MAJ22;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 22;
UNLOCK TABLES;
