USE `Ophtalmologie`;

DROP TABLE IF EXISTS `pachymetries`;
CREATE TABLE `pachymetries` (
  `idPat` int(10) unsigned NOT NULL,
  `pachyOD` smallint(3) DEFAULT NULL,
  `pachyOG` smallint(3) DEFAULT NULL,
  `pachyDate` date DEFAULT NULL,
  `pachyType` varchar(15) DEFAULT NULL,
  KEY `idPatTO_idx` (`idPat`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `speculaires`;
CREATE TABLE `speculaires` (
  `idPat` int(10) unsigned NOT NULL,
  `specOD` smallint(4) DEFAULT NULL,
  `specOG` smallint(4) DEFAULT NULL,
  `specDate` date DEFAULT NULL,
  `specType` varchar(15) DEFAULT NULL,
  KEY `idPatTO_idx` (`idPat`)
) ENGINE=InnoDB;

USE `Rufus`;
DROP TABLE IF EXISTS `champsfusionimpression`;
CREATE TABLE `champsfusionimpression` (
  `idChamp` int(11) NOT NULL AUTO_INCREMENT,
  `NomChamp` varchar(45) DEFAULT NULL,
  `ResumeChamp` varchar(90) DEFAULT NULL,
  `Interrogatif` int(1) DEFAULT NULL,
  PRIMARY KEY (`idChamp`)
) ENGINE=InnoDB;

LOCK TABLES `champsfusionimpression` WRITE;
INSERT INTO `champsfusionimpression` VALUES (1,'TITREUSER','Titre, nom et prénom de l\'utilisateur',NULL),(2,'NOM PATIENT','Nom du patient',NULL),(3,'DATE','Date du jour',NULL),(4,'DDN','Date de naissance',NULL),(7,'TITRE PATIENT','Titre du patient',NULL),(8,'AGE PATIENT','Age du patient',NULL),(9,'PRENOM PATIENT','Prénom du patient',NULL),(10,'OEIL','oeil à traiter',1),(11,'DATE INTERVENTION','Date de l\'intervention',1),(12,'TYPE INTERVENTION','Type d\'intervention',1),(13,'DUREE TRAITEMENT','Durée du traitement',1),(14,'MEDECIN PATIENT','Médecin du patient',NULL),(17,'FORMULE POLITESSE MEDECIN','formule de politesse médecin patient',NULL),(18,'PRENOM MEDECIN','Prénom du médecin',NULL),(19,'NOM MEDECIN','Nom du médecin',NULL),(20,'REFRACTION','Refraction du patient',NULL),(21,'KERATOMETRIE','Keratométrie du patient',NULL),(22,'CORRESPONDANT PATIENT','Correspondant du patient',NULL),(23,'FORMULE POLITESSE CORRESPONDANT','formule de politesse correspondant',NULL),(24,'NOM CORRESPONDANT','Nom du correspondant',NULL),(25,'PRENOM CORRESPONDANT','Prénom du correspondant',NULL);
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 20;
UNLOCK TABLES;

