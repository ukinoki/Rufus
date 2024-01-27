-- 26/06/2024

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ77;
DELIMITER |
CREATE PROCEDURE MAJ77()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'listeappareils' AND COLUMN_NAME = 'NomAppareil') as chp;
        IF tot=1
        THEN
			INSERT INTO `rufus`.`listeappareils` (`TitreExamen`, `NomAppareil`) VALUES ('RNM', 'TOPCON TRC-NW400');
			INSERT INTO `rufus`.`listeappareils` (`TitreExamen`, `NomAppareil`) VALUES ('RNM', 'ESSILOR Retina 550');
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 77;
END|

CALL MAJ77();
DROP PROCEDURE IF EXISTS MAJ77;
