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
    -- ParametresSysteme.ValeurAMYMetropole : valeur de la lettre-cle AMY en metropole
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ValeurAMYMetropole') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `ValeurAMYMetropole` decimal(9,2) NULL DEFAULT 2.60 AFTER `VersionNGAP`;
        END IF;
    -- ParametresSysteme.ValeurAMYDOM : valeur de la lettre-cle AMY dans les DOM
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ValeurAMYDOM') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `ValeurAMYDOM` decimal(9,2) NULL DEFAULT 2.72 AFTER `ValeurAMYMetropole`;
        END IF;
    -- Ophtalmologie.IOLs.modelname : 45 -> 100 (noms d'implants IOL plus longs).
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = 'Ophtalmologie' AND TABLE_NAME = 'IOLs' AND COLUMN_NAME = 'modelname' AND CHARACTER_MAXIMUM_LENGTH < 100) as chp;
        IF tot=1
            THEN
                ALTER TABLE `Ophtalmologie`.`IOLs`
                CHANGE COLUMN `modelname` `modelname` VARCHAR(100) NULL DEFAULT NULL;
        END IF;
    -- cotations.Tip : varchar -> text (libelles NGAP longs + personnalisation multiligne,
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'Tip' AND DATA_TYPE = 'varchar') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`cotations`
                CHANGE COLUMN `Tip` `Tip` TEXT NULL DEFAULT NULL;
        END IF;
    -- ccam.nom : varchar -> text (libelles CCAM longs)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ccam' AND COLUMN_NAME = 'nom' AND DATA_TYPE = 'varchar') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`ccam`
                CHANGE COLUMN `nom` `nom` TEXT NULL DEFAULT NULL;
        END IF;
    -- cotations.Typecotation : nouveau champ « type de cotation » (1 = CCAM, 2 = Association CCAM, 3 = NGAP, 4 = Autre)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'Typecotation') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`cotations`
                ADD COLUMN `Typecotation` INT(1) NULL DEFAULT NULL
                COMMENT '1 = CCAM\n2 = Association CCAM\n3 = NGAP\n4 = Autre' AFTER `CCAM`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'CCAM') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`cotations`
                MODIFY COLUMN `CCAM` INT(1) NULL DEFAULT NULL
                COMMENT 'plus utilise, garde pour compatibilite avec les versions anterieures au 15-07-2026';
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'idUser') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`cotations`
                MODIFY COLUMN `idUser` INT(11) NULL DEFAULT NULL
                COMMENT 'plus utilise, garde pour compatibilite avec les versions anterieures au 15-07-2026';
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'MontantPratique') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`cotations`
                MODIFY COLUMN `MontantPratique` decimal(9,2) NULL DEFAULT NULL
                COMMENT 'plus utilise, garde pour compatibilite avec les versions anterieures au 15-07-2026';
        END IF;

-- tables de personnalisation par utilisateur du montant pratiqué (cotations et ccam)
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresautrescotations` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantConventionnel` decimal(9,2) NULL DEFAULT NULL,
        `MontantPratique` decimal(9,2) NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresccam` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` decimal(9,2) NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresassociations` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` decimal(9,2) NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresNGAP` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    -- jointuresNGAP n'a PAS de MontantPratique
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'jointuresNGAP' AND COLUMN_NAME = 'MontantPratique') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`jointuresNGAP` DROP COLUMN `MontantPratique`;
        END IF;
    -- ParametresSysteme.Pays : code pays ISO 3166-1 alpha-2 (FR, ES, MX, CL...), DISTINCT de
    -- Version qui porte la langue - Servira d'abord aux codes postaux et a la liste des villes.
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'Pays') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `Pays` VARCHAR(2) NULL DEFAULT NULL
                COMMENT 'code pays ISO 3166-1 alpha-2 - null = pas encore choisi' AFTER `Version`;
        END IF;
    -- ParametresSysteme.CotationDesActes : on souhaite ou non utiliser le systeme de cotation des actes
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'CotationDesActes') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `CotationDesActes` INT(1) NULL DEFAULT 1
                COMMENT 'utiliser ou non le systeme de cotation des actes - 1 = oui' AFTER `Pays`;
        END IF;
    -- ParametresSysteme.Compta : gestion de la comptabilité (1 = normale, 2 = réduite, 3 = aucune)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'Compta') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `Compta` INT(1) NULL DEFAULT 1
                COMMENT '1 = comptabilite normale\n2 = comptabilite reduite\n3 = pas de comptabilite' AFTER `CotationDesActes`;
        END IF;
    -- SansCompta et ComptaFrance : remplaces par Compta, conserves pour les postes en version anterieure
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'SansCompta') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                MODIFY COLUMN `SansCompta` INT(1) NULL DEFAULT NULL
                COMMENT 'deprecie, remplace par Compta - garde pour compatibilite avec les versions anterieures au 12-08-2026';
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ComptaFrance') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                MODIFY COLUMN `ComptaFrance` INT(1) NULL DEFAULT NULL
                COMMENT 'deprecie, remplace par Compta - garde pour compatibilite avec les versions anterieures au 12-08-2026';
        END IF;
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 83;
END|
CALL MAJ83();
DROP PROCEDURE IF EXISTS MAJ83;
