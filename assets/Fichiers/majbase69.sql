-- 22/04/2022

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ69;
DELIMITER |
CREATE PROCEDURE MAJ69()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'Glasspop') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `LAN`, `Implemente`) VALUES ('NIDEK', 'Glasspop', 'Refracteur', '1', '1');
		END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'RT-6100') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `LAN`, `Implemente`) VALUES ('NIDEK', 'RT-6100', 'Refracteur', '1', '1');
		END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'LM-7') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `Serie`, `LAN`, `Implemente`) VALUES ('NIDEK', 'LM-7', 'Fronto', '1', '1', '1');
                END IF;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 69;
END|

CALL MAJ69();
DROP PROCEDURE IF EXISTS MAJ69;
