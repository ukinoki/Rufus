-- 26/06/2022

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ71;
DELIMITER |
CREATE PROCEDURE MAJ71()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Metadocuments' AND COLUMN_NAME = 'TextMetaDocument') as chp;)
        IF tot=1
        THEN
            ALTER TABLE `Metadocuments` 
			DROP COLUMN `TextMetaDocument`;
		END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 71;
END|

CALL MAJ71();
DROP PROCEDURE IF EXISTS MAJ71;

