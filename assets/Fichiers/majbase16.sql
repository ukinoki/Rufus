USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ16;
DELIMITER |
        CREATE PROCEDURE MAJ16()
        BEGIN
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'TextOrigine') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions`
                ADD COLUMN `TextOrigine` blob NULL DEFAULT NULL AFTER `TextCorps`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'SousTypeDoc') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions`
                ADD COLUMN `SousTypeDoc` varchar(45) NULL DEFAULT NULL AFTER `TypeDoc`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'EmisRecu') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions`
                ADD COLUMN `EmisRecu` int(1) NULL DEFAULT NULL;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'FormatDoc') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions`
                ADD COLUMN `FormatDoc` VARCHAR(12) NULL DEFAULT NULL;
            END IF;
        END|
CALL MAJ16();
DROP PROCEDURE MAJ16;--

DROP TABLE IF EXISTS `listeappareils`;
CREATE TABLE `listeappareils` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `TitreExamen` varchar(45) DEFAULT NULL,
  `NomAppareil` varchar(150) DEFAULT NULL,
  `Format` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB AUTO_INCREMENT=11;

--
-- Dumping data for table `listeappareils`
--

LOCK TABLES `listeappareils` WRITE;
INSERT INTO `listeappareils` VALUES (1,'Biométrie','TOPCON ALADDIN',NULL),(2,'RNM','CANON CR-2',NULL),(3,'OCT','OTI SLO',NULL),(4,'AngioOCT','OPTOVUE',NULL),(5,'OCT','NIDEK-OCT',NULL),(6,'RNM','NIDEK-RNM',NULL),(7,'OCT','SPECTRALIS',NULL),(8,'LOG','NAVIS-EX',NULL),(9,'RNM-E','EIDON',NULL),(10,'RNM-O','OPTOS Daytona series',NULL),(11,'OCT','CANON-OCT',NULL);
UNLOCK TABLES;


LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 16;
UNLOCK TABLES;
