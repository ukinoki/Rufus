USE `Rufus`;

DROP TABLE IF EXISTS `listeappareils`;
CREATE TABLE `listeappareils` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `TitreExamen` varchar(45) DEFAULT NULL,
  `NomAppareil` varchar(150) DEFAULT NULL,
  `Format` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB;

LOCK TABLES `listeappareils` WRITE;
INSERT INTO `listeappareils` VALUES (1,'Biométrie','TOPCON ALADDIN',NULL),(2,'RNM','CANON CR-2',NULL),(3,'OCT','OTI SLO',NULL),(4,'AngioOCT','OPTOVUE',NULL),(5,'OCT','NIDEK-OCT',NULL),(6,'RNM','NIDEK-RNM',NULL),(7,'OCT','SPECTRALIS',NULL),(8,'LOG','NAVIS-EX',NULL),(9,'RNM-E','EIDON',NULL),(10,'RNM-O','OPTOS Daytona series',NULL),(11,'OCT','CANON-OCT',NULL),(12,'Biométrie','TOPCON ALADDIN II',NULL);
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 18;
UNLOCK TABLES;
