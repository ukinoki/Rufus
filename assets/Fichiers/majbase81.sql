-- 02/12/2024

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ81;
DELIMITER |
CREATE PROCEDURE MAJ81()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'DirImagerie') as chp;
        IF tot=1
        THEN
			ALTER TABLE `rufus`.`ParametresSysteme` 
			DROP COLUMN `DirImagerie`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 81;
END|
CALL MAJ81();
DROP PROCEDURE IF EXISTS MAJ81;
