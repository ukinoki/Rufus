-- 16/11/2018
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ48;
DELIMITER |
    CREATE PROCEDURE MAJ48()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT COLUMN_KEY
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME = 'UtilisateursConnectes' AND COLUMN_NAME = 'NomPosteConnecte' AND COLUMN_KEY = 'UNI') as chp;
            IF tot=1
            THEN
                ALTER TABLE `rufus`.`UtilisateursConnectes`
		DROP INDEX `NomPosteConnecte_UNIQUE` ;
            END IF;
    END|
    UPDATE `ParametresSysteme` SET VersionBase = 48;
CALL MAJ48();
DROP PROCEDURE IF EXISTS MAJ48;

