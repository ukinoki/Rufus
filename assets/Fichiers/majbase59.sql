-- 21/12/2019
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ59;
DELIMITER |
    CREATE PROCEDURE MAJ59()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'Manwebsite') as chp;
    IF tot=0
        THEN
            ALTER TABLE `Manufacturers`
            ADD COLUMN `Manwebsite` varchar(120) AFTER `ManPortable`;
    END IF;
    END|

CALL MAJ59();
DROP PROCEDURE IF EXISTS MAJ59;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 59;
