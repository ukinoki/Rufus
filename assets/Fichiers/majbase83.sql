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
    -- ParametresSysteme.VersionNGAP : date de la version de NGAP chargee
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'VersionNGAP') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `VersionNGAP` DATE NULL DEFAULT '2026-07-01' AFTER `VersionCCAM`;
        END IF;
    -- ParametresSysteme.ValeurRNO : valeur de la lettre-cle RNO
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ValeurRNO') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `ValeurRNO` DOUBLE NULL DEFAULT 28 AFTER `VersionNGAP`;
        END IF;
    -- ParametresSysteme.ValeurAMYMetropole : valeur de la lettre-cle AMY en metropole
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ValeurAMYMetropole') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `ValeurAMYMetropole` DOUBLE NULL DEFAULT 2.60 AFTER `ValeurRNO`;
        END IF;
    -- ParametresSysteme.ValeurAMYDOM : valeur de la lettre-cle AMY dans les DOM
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ValeurAMYDOM') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `ValeurAMYDOM` DOUBLE NULL DEFAULT 2.72 AFTER `ValeurAMYMetropole`;
        END IF;
    -- Ophtalmologie.IOLs.modelname : 45 -> 100 (noms d'implants IOL plus longs)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'modelname' AND CHARACTER_MAXIMUM_LENGTH < 100) as chp;
        IF tot=1
            THEN
                ALTER TABLE `Ophtalmologie`.`IOLs`
                CHANGE COLUMN `modelname` `modelname` VARCHAR(100) NULL DEFAULT NULL;
        END IF;
    -- cotations.Tip : varchar -> text (libelles NGAP longs + personnalisation multiligne,
    -- plus de limite de longueur a gerer)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'Tip' AND DATA_TYPE = 'varchar') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`cotations`
                CHANGE COLUMN `Tip` `Tip` TEXT NULL DEFAULT NULL;
        END IF;
    -- tables de personnalisation par utilisateur du montant pratiqué (cotations et ccam)
    CREATE TABLE IF NOT EXISTS `rufus`.`jointurescotations` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresccam` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCCAM` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresassociations` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointurescotationsautres` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 83;
END|
CALL MAJ83();
DROP PROCEDURE IF EXISTS MAJ83;
