USE `rufus`;

DROP PROCEDURE IF EXISTS MAJ11;
DELIMITER |
        CREATE PROCEDURE MAJ11()
        BEGIN
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'DateDerniereConnexion') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Utilisateurs`
                ADD COLUMN `DateDerniereConnexion` DateTime NULL DEFAULT NULL;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'Compression') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions`
                ADD COLUMN `Compression` INT(1) NULL DEFAULT NULL AFTER `pdf`;
            END IF;
        END|
CALL MAJ11();
DROP PROCEDURE MAJ11;--


LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 11;
UNLOCK TABLES;

DROP TABLE IF EXISTS `MessagerieJointures`;
CREATE TABLE `MessagerieJointures` (
  `idJointure` int(11) NOT NULL AUTO_INCREMENT,
  `idMessage` int(11) NOT NULL,
  `idDestinataire` int(11) NOT NULL,
  `Lu` tinyint(1) DEFAULT NULL,
  `Fait` INT(1) DEFAULT NULL,
  PRIMARY KEY (`idJointure`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

LOCK TABLES `MessagerieJointures` WRITE;
UNLOCK TABLES;

DROP TABLE IF EXISTS `Messagerie`;
CREATE TABLE `Messagerie` (
  `idMessage` int(11) NOT NULL AUTO_INCREMENT,
  `idEmetteur` int(11) NOT NULL,
  `TexteMessage` text NOT NULL,
  `idPatient` int(11) DEFAULT NULL,
  `Tache` tinyint(1) DEFAULT NULL,
  `DateLimite` date DEFAULT NULL,
  `CreeLe` datetime DEFAULT NULL,
  `Urge` INT(1) DEFAULT NULL,
  PRIMARY KEY (`idMessage`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;
