-- 22/04/2022

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ72;
DELIMITER |
CREATE PROCEDURE MAJ72()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'TL-6100') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `Serie`, `LAN`, `Implemente`) VALUES ('TOMEY', 'TL-6100', 'Fronto ', '1', NULL, NULL);
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'AL6500') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`) VALUES ('RODENSTOCK', 'AL6500', 'Fronto', '53', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'AL6700') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`) VALUES ('RODENSTOCK', 'AL6700', 'Fronto', '53', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'TL-6000') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `Serie`, `LAN`, `Implemente`) VALUES ('TOMEY', 'TL-6000', 'Fronto ', '1', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'AL6600') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `LAN`, `Implemente`) VALUES ('RODENSTOCK', 'AL6600', 'Fronto', '56', '1', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'RC-6600') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `LAN`, `Implemente`) VALUES ('TOMEY', 'RC-6000', 'Autoref', NULL, '1', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'CX2000') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `LAN`, `Implemente`) VALUES ('RODENSTOCK', 'CX2000', 'Autoref', '58', '1', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'TOP-1000') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `LAN`, `Implemente`) VALUES ('TOMEY', 'TOP-1000', 'Tonometre', NULL, '1', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'Topascope') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `LAN`, `Implemente`) VALUES ('RODENSTOCK', 'Topascope', 'Tonometre', '60', '1', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'TAP-2000') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `LAN`, `Implemente`) VALUES ('TOMEY', 'TAP-2000', 'Refracteur', NULL, '1', NULL, '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT Modele
        FROM AppareilsRefraction
        WHERE Modele = 'Phoromat 2000') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `SimilaireA`, `Serie`, `Implemente`) VALUES ('RODENSTOCK', 'Phoromat 2000', 'Refracteur', '62', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT NomAppareil
        FROM listeappareils
        WHERE NomAppareil = 'Spectralis SHIFT') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`listeappareils` (`TitreExamen`, `NomAppareil`) VALUES ('0CT', 'Spectralis SHIFT');
	END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'salledattente' AND COLUMN_NAME = 'idPat') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`salledattente` 
            DROP PRIMARY KEY;
        END IF;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 72;
END|

CALL MAJ72();
DROP PROCEDURE IF EXISTS MAJ72;
