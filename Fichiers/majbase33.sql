USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ33;
DELIMITER |
    CREATE PROCEDURE MAJ33()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'NomLieu') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Actes`
            DROP COLUMN `NomLieu`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'idLieu') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Actes`
            ADD COLUMN `idLieu` INT(11) NULL DEFAULT NULL AFTER `NumCentre`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'idLieuParDefaut') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ParametresSysteme`
            ADD COLUMN `idLieuParDefaut` INT(11) NULL DEFAULT NULL AFTER `NumCentre`;
        END IF;
    END|
CALL MAJ33();
DROP PROCEDURE MAJ33;
LOCK TABLES `Actes` WRITE;
UPDATE `Actes` SET NumCentre = 1, idLieu = 1;
UNLOCK TABLES;
LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET NumCentre = 1, idLieuParDefaut = 1, VersionBase = 33;
UNLOCK TABLES;

