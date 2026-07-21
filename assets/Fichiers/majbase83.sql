-- 21/07/2026

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ83;
DELIMITER |
CREATE PROCEDURE MAJ83()
BEGIN
    DECLARE tot INT DEFAULT 1;
    -- utilisateurs.SignatureAuto : drapeau "signer automatiquement les courriers
    -- et ordonnances", deplace de rufus.ini vers l'utilisateur (suit l'utilisateur)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'SignatureAuto') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`utilisateurs`
                ADD COLUMN `SignatureAuto` TINYINT(1) NULL DEFAULT NULL AFTER `Signature`;
        END IF;
    -- ParametresSysteme.VersionCCAM : version de la nomenclature CCAM chargee
    -- (sur le modele de VersionBaseIOL)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'VersionCCAM') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `VersionCCAM` DOUBLE NULL DEFAULT NULL AFTER `VersionBaseIOL`;
        END IF;
    -- Ophtalmologie.IOLs.modelname : 45 -> 60 (noms d'implants IOLexport > 45 car.)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'modelname' AND CHARACTER_MAXIMUM_LENGTH < 60) as chp;
        IF tot=1
            THEN
                ALTER TABLE `Ophtalmologie`.`IOLs`
                CHANGE COLUMN `modelname` `modelname` VARCHAR(60) NULL DEFAULT NULL;
        END IF;
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 83;
END|
CALL MAJ83();
DROP PROCEDURE IF EXISTS MAJ83;
