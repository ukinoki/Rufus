USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ17;
DELIMITER |
        CREATE PROCEDURE MAJ17()
        BEGIN
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'SansCompta') as chp;
            IF tot=0
            THEN
                ALTER TABLE `ParametresSysteme`
                ADD COLUMN `SansCompta` int(1) NULL DEFAULT NULL;
            END IF;
        END|
CALL MAJ17();
DROP PROCEDURE MAJ17;--

DROP TABLE IF EXISTS `ParametresMachines`;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 17;
UNLOCK TABLES;
