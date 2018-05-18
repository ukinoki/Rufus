USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ34;
DELIMITER |
    CREATE PROCEDURE MAJ34()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'FormatDoc') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Impressions`
            CHANGE COLUMN `FormatDoc` `FormatDoc` VARCHAR(12) NULL DEFAULT NULL COMMENT 'Prescription de lunettes = PrescrLun\nPrescription medicale = Prescription\nCourrier emis = Courrier\nDocument scan = DocRecu\nDocument imagerie = Imagerie\nBilan orthoptique = BO';
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'LienFichierDistant') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Impressions`
            ADD COLUMN `LienFichierDistant` VARCHAR(250) NULL DEFAULT NULL COMMENT 'Pointe un lien distant vers fichier image ou video externe';
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'AdresseServeurLocal') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ParametresSysteme`
            ADD COLUMN `AdresseServeurLocal` VARCHAR(45) NULL AFTER `SansCompta`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'AdresseServeurDistant') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ParametresSysteme`
            ADD COLUMN `AdresseServeurDistant` VARCHAR(45) NULL AFTER `AdresseServeurLocal`;
        END IF;
        UPDATE `ParametresSysteme` SET VersionBase = 34;
    END|
CALL MAJ34();
DROP PROCEDURE MAJ34;

