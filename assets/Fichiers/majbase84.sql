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
                ADD COLUMN `LieuSMTPPort` INT(11) NULL DEFAULT 587
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
