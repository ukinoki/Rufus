-- 02/1/2024

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ80;
DELIMITER |
CREATE PROCEDURE MAJ80()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'AppareilsRefraction' AND COLUMN_NAME = 'Commentaire') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`AppareilsRefraction`
            ADD COLUMN `Commentaire` BLOB NULL AFTER `Implemente`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Impressions' AND COLUMN_NAME = 'Cote') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Impressions`
            ADD COLUMN `Cote` INT(1) NULL DEFAULT NULL COMMENT '1 = R 2 = G' AFTER `Importance`;
        END IF;
    update  `rufus`.`Impressions` set Cote = 1 where SousTypeDoc like '% OD';
    update  `rufus`.`Impressions` set Cote = 2 where SousTypeDoc like '% OG';
    SELECT COUNT(*) INTO tot FROM
        (SELECT idAppareil
        FROM AppareilsRefraction
        WHERE Marque = 'SHIN-NIPPON' AND Modele = 'DR-900') as chp;
        IF tot=0
        THEN
            INSERT INTO `rufus`.`AppareilsRefraction` (`Marque`, `Modele`, `Type`, `Serie`, `Implemente`) VALUES ('SHIN-NIPPON', 'DR-900', 'Refracteur', '1', '1');
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'UserBarCode') as chp;
        IF tot=1
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            CHANGE COLUMN `UserBarCode` `UserLogo` BLOB NULL DEFAULT NULL;
        ELSEIF tot=0
        THEN
            ALTER TABLE `rufus`.`utilisateurs`
            ADD COLUMN `UserLogo` BLOB NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'SiteLogo') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`LieuxExercice`
            ADD COLUMN `SiteLogo` BLOB NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Impressions' AND COLUMN_NAME = 'PdfOrigin') as chp;
        IF tot=0
        THEN
            ALTER TABLE `rufus`.`Impressions`
            ADD COLUMN `PdfOrigin` MEDIUMBLOB AFTER `TextOrigine`;
        END IF;

UPDATE `rufus`.`AppareilsRefraction` SET LAN = 1, Implemente = 1 WHERE Marque = 'TOPCON' and Modele = 'CV-5000';
UPDATE `rufus`.`AppareilsRefraction` SET LAN = 1, Implemente = 1 WHERE Marque = 'TOPCON' and Modele = 'CV-3000';
UPDATE `rufus`.`AppareilsRefraction` SET LAN = 1, Implemente = 1 WHERE Marque = 'TOPCON' and Modele = 'CV-2500';
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 80;
END|
CALL MAJ80();
DROP PROCEDURE IF EXISTS MAJ80;
