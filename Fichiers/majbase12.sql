USE `rufus`;

DROP PROCEDURE IF EXISTS MAJ12;
DELIMITER |
        CREATE PROCEDURE MAJ12()
        BEGIN
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'Conclusion') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions`
                ADD COLUMN `Conclusion` VARCHAR(45) NULL DEFAULT NULL;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Messagerie' AND COLUMN_NAME = 'ReponseA') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Messagerie`
                ADD COLUMN `ReponseA` INT(11) NULL DEFAULT NULL;
            END IF;
        END|
CALL MAJ12();
DROP PROCEDURE MAJ12;--

DROP TABLE IF EXISTS `champsfusionimpression`;
CREATE TABLE `champsfusionimpression` (
  `idChamp` int(11) NOT NULL AUTO_INCREMENT,
  `NomChamp` varchar(45) DEFAULT NULL,
  `ResumeChamp` varchar(90) DEFAULT NULL,
  `Interrogatif` int(1) DEFAULT NULL,
  PRIMARY KEY (`idChamp`)
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=latin1;

LOCK TABLES `champsfusionimpression` WRITE;
INSERT INTO `champsfusionimpression` VALUES (1,'TITREUSER','Titre, nom et prénom de l\'utilisateur',NULL),(2,'NOM PATIENT','Nom du patient',NULL),(3,'DATE','Date du jour',NULL),(4,'DDN','Date de naissance',NULL),(7,'TITRE PATIENT','Titre du patient',NULL),(8,'AGE PATIENT','Age du patient',NULL),(9,'PRENOM PATIENT','Prénom du patient',NULL),(10,'OEIL','oeil à traiter',1),(11,'DATE INTERVENTION','Date de l\'intervention',1),(12,'TYPE INTERVENTION','Type d\'intervention',1),(13,'DUREE TRAITEMENT','Durée du traitement',1),(14,'MEDECIN PATIENT','Médecin du patient',NULL),(17,'FORMULE POLITESSE MEDECIN','formule de politesse médecin patient',NULL),(18,'PRENOM MEDECIN','Prénom du médecin',NULL),(19,'NOM MEDECIN','Nom du médecin',NULL),(20,'REFRACTION','Refraction du patient',NULL),(21,'KERATOMETRIE','Keratomeétrie du patient',NULL);
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 12;
UNLOCK TABLES;
