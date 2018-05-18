USE `rufus`;

DROP PROCEDURE IF EXISTS MAJ40;
DELIMITER |
    CREATE PROCEDURE MAJ40()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='utilisateurs' AND COLUMN_NAME = 'UserSecteur') as chp;
        IF tot=1
        THEN
            ALTER TABLE `utilisateurs`
            CHANGE COLUMN `UserSecteur` `UserSecteur` TINYINT(1) NULL DEFAULT NULL COMMENT '1= Secteur 1\n2= Secteur2\n3= Secteur3' ;
        END IF;
     END|
CALL MAJ40();
DROP PROCEDURE MAJ40;

update `correspondants` set corspecialite = 0 where cormedecin = 1 and corspecialite = 40;

DROP TABLE IF EXISTS `specialitesmedicales`;
CREATE TABLE `specialitesmedicales` (
  `idspecialite` int(11) NOT NULL,
  `nomspecialite` varchar(100) NOT NULL,
  PRIMARY KEY (`idspecialite`)
) ENGINE=InnoDB;

LOCK TABLES `specialitesmedicales` WRITE;
INSERT INTO `specialitesmedicales` VALUES (0, 'médecine générale'),(1,'anatomie et cytologie pathologiques'),(2,'anesthésie-réanimation'),(3,'cardiologie et maladies vasculaires'),(4,'chirurgie générale'),(5,'chirurgie orale'),(6,'dermatologie et vénérologie'),(7,'endocrinologie et métabolisme'),(8,'gastro-entérologie et hépatologie'),(9,'génétique médicale'),(10,'gynécologie médicale'),(11,'gynécologie obstétrique'),(12,'hématologie'),(13,'médecine générale'),(14,'médecine interne'),(15,'médecine nucléaire'),(16,'médecine physique et de réadaptation'),(17,'néphrologie'),(18,'neurochirurgie'),(19,'neurologie'),(20,'oncologie'),(21,'ophtalmologie'),(22,'oto-rhino-laryngologie et chirurgie cervico-faciale'),(23,'pédiatrie'),(24,'pneumologie'),(25,'psychiatrie'),(26,'radiodiagnostic et imagerie médicale'),(27,'rhumatologie'),(28,'santé publique et médecine sociale'),(29,'chirurgie infantile'),(30,'chirurgie maxillo-faciale et stomatologie'),(31,'chirurgie de la face et du cou'),(32,'chirurgie orthopédique et traumatologie'),(33,'chirurgie plastique reconstructrice et esthétique'),(34,'chirurgie thoracique et cardio-vasculaire'),(35,'chirurgie urologique'),(36,'chirurgie vasculaire'),(37,'chirurgie viscérale et digestive'),(38,'gériatrie'),(39,'réanimation médicale');
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 40;
UNLOCK TABLES;

