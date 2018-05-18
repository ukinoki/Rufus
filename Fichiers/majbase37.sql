USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ37;
DELIMITER |
    CREATE PROCEDURE MAJ37()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='UtilisateursConnectes' AND COLUMN_NAME = 'idLieu') as chp;
        IF tot=0
        THEN
            ALTER TABLE `UtilisateursConnectes`
            ADD COLUMN `idLieu` INT NULL DEFAULT NULL AFTER `UserParent`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'idLieu') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Impressions`
            ADD COLUMN `idLieu` INT NULL DEFAULT NULL COMMENT 'Lieu de réalisation de l examen' AFTER `FormatDoc`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='appareilsconnectescentre' AND COLUMN_NAME = 'idLieu') as chp;
        IF tot=0
        THEN
            ALTER TABLE `appareilsconnectescentre`
            ADD COLUMN `idLieu` INT NULL DEFAULT NULL COMMENT 'Lieu ou se trouve l appareil' AFTER `idAppareil`,
            CHANGE COLUMN `idAppareil` `idAppareil` INT(11) NOT NULL,
            DROP PRIMARY KEY;
        END IF;
        SELECT idlieupardefaut FROM ParametresSysteme INTO tot;
        IF tot>0
        THEN
            UPDATE `Impressions` SET `idLieu` = tot;
            UPDATE `appareilsconnectescentre` SET `idLieu` = tot;
        END IF;
    END|
CALL MAJ37();
DROP PROCEDURE MAJ37;

create user if not exists 'AdminSSL'@'%' identified by 'bob' REQUIRE SSL;
grant all on *.* to 'AdminSSL'@'%' identified by 'bob' with grant option;

USE `Images`;
DROP TABLE IF EXISTS `DistantImages`;
DROP TABLE IF EXISTS `DocsASupprimer`;
CREATE TABLE `DocsASupprimer` (
  `FilePath` VARCHAR(250) DEFAULT NULL
) ENGINE=InnoDB;

USE `Rufus`;
LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 37;
UNLOCK TABLES;

