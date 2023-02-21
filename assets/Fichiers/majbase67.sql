-- 22/05/2020

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ67;
DELIMITER |
CREATE PROCEDURE MAJ67()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT NomAppareil
        FROM listeappareils
        WHERE NomAppareil = 'ION Imaging') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`listeappareils` (`TitreExamen`, `NomAppareil`) VALUES ('ImagerieSA', 'ION Imaging');
        END IF;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 67;
END|

CALL MAJ67();
DROP PROCEDURE IF EXISTS MAJ67;

