-- 16/05/2024

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ78;
DELIMITER |
CREATE PROCEDURE MAJ78()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'salledattente' AND COLUMN_NAME = 'DateRDV') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`salledattente`
            ADD COLUMN `DateRDV` DATE NULL DEFAULT NULL AFTER `HeureStatut`;
        END IF;
	SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'salledattente' AND COLUMN_NAME = 'MessageRetour') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`salledattente`
            ADD COLUMN `MessageRetour` VARCHAR(300) NULL DEFAULT NULL AFTER `HeureArrivee`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 78;
END|

CALL MAJ78();
DROP PROCEDURE IF EXISTS MAJ78;
