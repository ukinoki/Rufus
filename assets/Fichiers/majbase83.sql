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
    -- ParametresSysteme.ValeurAMYMetropole : valeur de la lettre-cle AMY en metropole
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'ValeurAMYMetropole') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `ValeurAMYMetropole` DOUBLE NULL DEFAULT 2.60 AFTER `VersionNGAP`;
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
    -- Ophtalmologie.IOLs.modelname : 45 -> 100 (noms d'implants IOL plus longs).
    -- IOLs vit dans le schéma Ophtalmologie (pas rufus) : on QUALIFIE le TABLE_SCHEMA dans la sonde
    -- INFORMATION_SCHEMA, sinon le COUNT porte sur tous les schémas et « IF tot=1 » peut échouer.
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
    -- cotations.Typecotation : nouveau champ « type de cotation » (1 = CCAM, 2 = Association CCAM,
    -- 3 = NGAP, 4 = Autre). On NE RENOMME PAS CCAM : des postes en version antérieure au 15-07-2026
    -- lisent encore la colonne CCAM ; la renommer les bloquerait (colonne introuvable) sur le réseau.
    -- On ajoute donc Typecotation à côté, on y recopie CCAM comme valeur de départ (au démarrage,
    -- exporteJointures la recalcule de toute façon à partir du Typeacte), et on documente CCAM et
    -- idUser comme conservés pour compatibilité.
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'cotations' AND COLUMN_NAME = 'Typecotation') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`cotations`
                ADD COLUMN `Typecotation` INT(1) NULL DEFAULT NULL
                COMMENT '1 = CCAM\n2 = Association CCAM\n3 = NGAP\n4 = Autre' AFTER `CCAM`;
                UPDATE `rufus`.`cotations` SET `Typecotation` = `CCAM`;
                ALTER TABLE `rufus`.`cotations`
                MODIFY COLUMN `CCAM` INT(1) NULL DEFAULT NULL
                    COMMENT 'plus utilisé, gardé pour compatibilité avec les versions antérieures au 15-07-2026',
                MODIFY COLUMN `idUser` INT(11) NULL DEFAULT NULL
                    COMMENT 'plus utilisé, gardé pour compatibilité avec les versions antérieures au 15-07-2026';
        END IF;

-- tables de personnalisation par utilisateur du montant pratiqué (cotations et ccam)
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresautrescotations` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantConventionnel` DOUBLE NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresccam` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresassociations` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        `MontantPratique` DOUBLE NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    CREATE TABLE IF NOT EXISTS `rufus`.`jointuresNGAP` (
        `idJointure` INT(11) NOT NULL AUTO_INCREMENT,
        `idCotation` INT(11) NULL DEFAULT NULL,
        `idUser` INT(11) NULL DEFAULT NULL,
        PRIMARY KEY (`idJointure`));
    -- jointuresNGAP n'a PAS de MontantPratique : pour un acte NGAP le pratiqué est toujours égal au
    -- conventionnel, le champ ne servirait à rien. On le supprime s'il a été créé par un passage
    -- antérieur de MAJ83 (le CREATE IF NOT EXISTS ci-dessus n'aurait pas retouché la colonne).
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_NAME
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'jointuresNGAP' AND COLUMN_NAME = 'MontantPratique') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`jointuresNGAP` DROP COLUMN `MontantPratique`;
        END IF;
    -- ParametresSysteme.Pays : code pays ISO 3166-1 alpha-2 (FR, ES, MX, CL...), DISTINCT de
    -- Version qui porte la langue - un cabinet mexicain travaille en ES, un cabinet espagnol
    -- aussi. Servira d'abord aux codes postaux et a la liste des villes.
    -- null = pas encore choisi : le programme demandera.
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
    -- ParametresSysteme.CotationDesActes : on souhaite ou non utiliser le systeme de cotation des
    -- actes. Propriete GENERALE de la base, et non plus de l'utilisateur : la decision est celle du
    -- cabinet, pas de chacun. 1 par defaut = on cote.
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
    -- renommer SansCompta en Compta si elle existe
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'SansCompta') as chp;
        IF tot=1
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                CHANGE COLUMN `SansCompta` `Compta` INT(1) NULL DEFAULT 1
                COMMENT '1 = comptabilité normale\n2 = comptabilité réduite\n3 = pas de comptabilité';
        END IF;
    -- si Compta n'existe pas encore, la créer
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'Compta') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `Compta` INT(1) NULL DEFAULT 1
                COMMENT '1 = comptabilité normale\n2 = comptabilité réduite\n3 = pas de comptabilité' AFTER `CotationDesActes`;
        END IF;
    UPDATE `rufus`.`ParametresSysteme` SET Compta = 1;
    -- ParametresSysteme.Compta : gestion de la comptabilité (1 = normale, 2 = réduite, 3 = aucune)
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'ParametresSysteme' AND COLUMN_NAME = 'Compta') as chp;
        IF tot=0
            THEN
                ALTER TABLE `rufus`.`ParametresSysteme`
                ADD COLUMN `Compta` INT(1) NULL DEFAULT 1
                COMMENT '1 = comptabilité normale\n2 = comptabilité réduite\n3 = pas de comptabilité' AFTER `CotationDesActes`;
        END IF;
    UPDATE `rufus`.`ParametresSysteme` SET Compta = 1;
    UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 83;
END|
CALL MAJ83();
DROP PROCEDURE IF EXISTS MAJ83;
