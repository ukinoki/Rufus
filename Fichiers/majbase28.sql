USE `Comptamedicale`;
DROP PROCEDURE IF EXISTS MAJ28;
DELIMITER |
    CREATE PROCEDURE MAJ28()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Comptes' AND COLUMN_NAME = 'Desactive') as chp1;
        IF tot=0
        THEN
            ALTER TABLE `Comptes`
            ADD COLUMN `Desactive` TINYINT(1) NULL DEFAULT NULL AFTER `Partage`;
        END IF;
        END|
CALL MAJ28();
DROP PROCEDURE MAJ28;

USE `Rufus`;
LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 28;
UNLOCK TABLES;
