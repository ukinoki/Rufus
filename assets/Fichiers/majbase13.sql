USE `Ophtalmologie`;

DROP PROCEDURE IF EXISTS MAJ13;
DELIMITER |
        CREATE PROCEDURE MAJ13()
        BEGIN
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='DonneesOphtaPatients' AND COLUMN_NAME = 'DioptrieK1OD') as chp;
            IF tot=0
            THEN
                ALTER TABLE `DonneesOphtaPatients`
                ADD COLUMN `DioptrieK1OD` DECIMAL (4,2) NULL DEFAULT NULL,
                ADD COLUMN `DioptrieK2OD` DECIMAL (4,2) NULL DEFAULT NULL,
                ADD COLUMN `DioptrieKOD` DECIMAL (4,2) NULL DEFAULT NULL,
                ADD COLUMN `DioptrieK1OG` DECIMAL (4,2) NULL DEFAULT NULL,
                ADD COLUMN `DioptrieK2OG` DECIMAL (4,2) NULL DEFAULT NULL,
                ADD COLUMN `DioptrieKOG` DECIMAL (4,2) NULL DEFAULT NULL;
            END IF;
        END|
CALL MAJ13();
DROP PROCEDURE MAJ13;

Use `Rufus`;

DROP TABLE IF EXISTS `champsfusionimpression`;
CREATE TABLE `champsfusionimpression` (
  `idChamp` int(11) NOT NULL AUTO_INCREMENT,
  `NomChamp` varchar(45) DEFAULT NULL,
  `ResumeChamp` varchar(90) DEFAULT NULL,
  `Interrogatif` int(1) DEFAULT NULL,
  PRIMARY KEY (`idChamp`)
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=latin1;

LOCK TABLES `champsfusionimpression` WRITE;
INSERT INTO `champsfusionimpression` VALUES (1,'TITREUSER','Titre, nom et prénom de l\'utilisateur',NULL),(2,'NOM PATIENT','Nom du patient',NULL),(3,'DATE','Date du jour',NULL),(4,'DDN','Date de naissance',NULL),(7,'TITRE PATIENT','Titre du patient',NULL),(8,'AGE PATIENT','Age du patient',NULL),(9,'PRENOM PATIENT','Prénom du patient',NULL),(10,'OEIL','oeil à traiter',1),(11,'DATE INTERVENTION','Date de l\'intervention',1),(12,'TYPE INTERVENTION','Type d\'intervention',1),(13,'DUREE TRAITEMENT','Durée du traitement',1),(14,'MEDECIN PATIENT','Médecin du patient',NULL),(17,'FORMULE POLITESSE MEDECIN','formule de politesse médecin patient',NULL),(18,'PRENOM MEDECIN','Prénom du médecin',NULL),(19,'NOM MEDECIN','Nom du médecin',NULL),(20,'REFRACTION','Refraction du patient',NULL),(21,'KERATOMETRIE','Keratométrie du patient',NULL);
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 13;
UNLOCK TABLES;
