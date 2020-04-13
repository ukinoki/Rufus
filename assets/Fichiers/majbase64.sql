-- 09/04/2020
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ64;
DELIMITER |
    CREATE PROCEDURE MAJ64()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'DistribuePar') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Manufacturers`
            ADD COLUMN `DistribuePar` INT(11) NULL DEFAULT NULL AFTER `ManInactif`;
        END IF;
END|
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 64;
CALL MAJ64();
DROP PROCEDURE IF EXISTS MAJ62;
DROP PROCEDURE IF EXISTS MAJ64;

