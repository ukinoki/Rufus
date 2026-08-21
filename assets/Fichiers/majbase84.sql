-- 19/08/2026

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ84;
DELIMITER |
CREATE PROCEDURE MAJ84()
BEGIN
    DECLARE tot INT DEFAULT 1;
    -- LieuxExercice : coordonnees d'envoi des mails, par lieu d'exercice
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'LieuMail') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`LieuxExercice`
                ADD COLUMN `LieuMail` VARCHAR(100) NULL DEFAULT NULL
                COMMENT 'adresse mail du lieu - expediteur et copie des envois' AFTER `LieuFax`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'LieuSMTPServeur') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`LieuxExercice`
                ADD COLUMN `LieuSMTPServeur` VARCHAR(100) NULL DEFAULT NULL
                COMMENT 'serveur d envoi des mails' AFTER `LieuMail`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'LieuSMTPPort') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`LieuxExercice`
                ADD COLUMN `LieuSMTPPort` INT(11) NULL DEFAULT NULL
                COMMENT '465 = connexion chiffree d emblee, sinon chiffrement apres connexion' AFTER `LieuSMTPServeur`;
        END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'LieuxExercice' AND COLUMN_NAME = 'LieuSMTPLogin') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`LieuxExercice`
                ADD COLUMN `LieuSMTPLogin` VARCHAR(100) NULL DEFAULT NULL
                COMMENT 'le mot de passe n est pas en base, il est local au poste (~/.rufus/.mlkey)' AFTER `LieuSMTPPort`;
        END IF;

-- Il y a peut-être eu des version publiées sans ces corrections dans majbase83 donc on les reprend
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

-- tracabilite des documents envoyes par mail
-- un envoi = une ligne dans mailsenvoyes + une ligne de jointure par document joint
    CREATE TABLE IF NOT EXISTS `rufus`.`mailsenvoyes` (
        `idMail` INT(11) NOT NULL AUTO_INCREMENT,
        `Destinataire` VARCHAR(150) NULL DEFAULT NULL
            COMMENT 'adresse telle qu elle a servi ce jour la, pas un lien vers une fiche',
        `DateEnvoi` DATETIME NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        PRIMARY KEY (`idMail`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresmails` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idMail` INT(11) NULL DEFAULT NULL,
        `idImpression` INT(11) NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`),
        KEY `idMailidx` (`idMail`),
        KEY `idImpressionidx` (`idImpression`));
-- commentaires des documents imprimes, sur le modele de Ophtalmologie.commentaireslunettes
    CREATE TABLE IF NOT EXISTS `rufus`.`commentairesimpressions` (
        `idCommentImpr` INT(11) NOT NULL AUTO_INCREMENT,
        `TextComment` TEXT NOT NULL,
        `ResumeComment` VARCHAR(50) NOT NULL,
        `ParDefautComment` TINYINT(1) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `ComPublic` INT(1) NULL DEFAULT NULL,
        PRIMARY KEY (`idCommentImpr`));
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 84;
END|
CALL MAJ84();
DROP PROCEDURE IF EXISTS MAJ84;
