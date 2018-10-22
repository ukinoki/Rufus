USE `rufus`;

DROP PROCEDURE IF EXISTS MAJ42;
DELIMITER |
    CREATE PROCEDURE MAJ42()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'idSession') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Actes`
            ADD COLUMN `idSession` INT(11) NULL AFTER `idUser`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'Importance') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Impressions`
            ADD COLUMN `Importance` INT(11) NULL DEFAULT 1 COMMENT 'O MASQUE PAR DEFAUT\n1 AFFICHE PAR DEFAUT\n3 TOUJOURS AFFICHE';
        END IF;
        update `Impressions` set Importance = 1;
        update `courriers` set Medical = 1;
        ALTER TABLE `Impressions`
        CHANGE COLUMN `FormatDoc` `FormatDoc` VARCHAR(12) NULL DEFAULT NULL COMMENT 'Prescription de lunettes = PrescrLun\nPrescription medicale = Prescription\nCourrier emis = Courrier\nDocument scan = DocRecu\nDocument imagerie = Imagerie\nBilan orthoptique = BO\nDocument administratif = DocAdmin' ;
    END|
CALL MAJ42();

DROP TABLE IF EXISTS `JournalSessions`;
CREATE TABLE `JournalSessions` (
  `idJournalSessions` int(11) NOT NULL AUTO_INCREMENT,
  `idSession` int(11) DEFAULT NULL,
  `HeureDebut` datetime DEFAULT NULL,
  `HeureFin` datetime DEFAULT NULL,
  PRIMARY KEY (`idJournalSessions`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `Sessions`;
CREATE TABLE `Sessions` (
  `idSession` int(11) NOT NULL AUTO_INCREMENT,
  `idUser` int(11) DEFAULT NULL,
  `idSuperviseur` int(11) DEFAULT NULL COMMENT '-1 NON_RENSEIGNE\n-2 VIDE\n-3 INDETERMINE\n',
  `idParent` int(11) DEFAULT NULL COMMENT '-1 NON_RENSEIGNE\n-2 VIDE\n-3 INDETERMINE',
  `idComptable` int(11) DEFAULT NULL COMMENT '-1 NON_RENSEIGNE\n-2 VIDE\n-3 INDETERMINE',
  `idLieu` int(11) DEFAULT NULL,
  `DateSession` date DEFAULT NULL,
  PRIMARY KEY (`idSession`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 42;
