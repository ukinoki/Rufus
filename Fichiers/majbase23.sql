USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ23;
DELIMITER |
        CREATE PROCEDURE MAJ23()
        BEGIN 
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'SupervisePar') as chp;
            IF tot=1
            THEN
                ALTER TABLE `Actes`
                CHANGE COLUMN `SupervisePar` `PourLeCompteDe` int(11) NULL DEFAULT NULL;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'Soignant') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Utilisateurs`
                ADD COLUMN `Soignant` INT NULL DEFAULT NULL COMMENT '1 = realise des actes medicaux' AFTER `UserNumCO`,
                ADD COLUMN `ResponsableActes` INT NULL DEFAULT NULL COMMENT '0 = actes sous sa responsabilite et sous celle des autres users\n1 = actes sous sa responsabilite seulement\n2 = aucun acte sous sa responsabilite' AFTER `Soignant`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'SousLaResponsabiliteDe') as chp;
            IF tot=0
            THEN
                ALTER TABLE `UtilisateursConnectes`
                ADD COLUMN `SousLaResponsabiliteDe` INT NULL DEFAULT NULL AFTER `NomPosteConnecte`;
            END IF;
        END|
CALL MAJ23();
DROP PROCEDURE MAJ23;--

LOCK TABLES `Actes` WRITE;
UPDATE `Actes` SET PourLeCompteDe = idUser;
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 23;
UNLOCK TABLES;

