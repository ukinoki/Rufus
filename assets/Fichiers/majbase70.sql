-- 11/06/2022

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ70;
DELIMITER |
CREATE PROCEDURE MAJ70()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'AfficheDocsPublics') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            ADD COLUMN `AfficheDocsPublics` TINYINT(1) NULL DEFAULT 1 COMMENT 'Affiche les documents rendus publics par leur createur dans la fiche Impressions' AFTER `UserBarCode`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 70;
END|

CALL MAJ70();
DROP PROCEDURE IF EXISTS MAJ70;

