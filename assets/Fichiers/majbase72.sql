-- 22/04/2022

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ72;
DELIMITER |
CREATE PROCEDURE MAJ72()
BEGIN
DECLARE tot INT DEFAULT 1;

DROP TABLE IF EXISTS `AppareilsRefraction`;
CREATE TABLE `AppareilsRefraction` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `Marque` varchar(45) CHARACTER SET latin1 NOT NULL,
  `Modele` varchar(45) CHARACTER SET latin1 NOT NULL,
  `Type` varchar(45) CHARACTER SET latin1 NOT NULL,
  `SimilaireA` int(11) DEFAULT NULL,
  `Serie` int(1) DEFAULT NULL,
  `LAN` int(1) DEFAULT NULL,
  `Implemente` varchar(45) CHARACTER SET latin1 DEFAULT NULL,
  `Commentaire` blob,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB AUTO_INCREMENT=65 DEFAULT CHARSET=utf8;

INSERT INTO `AppareilsRefraction` VALUES (1,'TOPCON','CV-5000','Refracteur',NULL,NULL,NULL,NULL,NULL),(2,'TOPCON','CV-3000','Refracteur',NULL,NULL,NULL,NULL,NULL),(3,'TOPCON','CV-2500','Refracteur',NULL,NULL,NULL,NULL,NULL),(4,'NIKON','Remote Vision','Refracteur',NULL,NULL,NULL,NULL,NULL),(5,'TOMEY','TAP-2000','Refracteur',NULL,NULL,NULL,NULL,NULL),(6,'POTEC','PDR-7000','Refracteur',5,NULL,NULL,NULL,NULL),(7,'VISIONIX','VX-60','Refracteur',NULL,NULL,NULL,NULL,NULL),(8,'TOMEY','TL-2000C','Fronto',NULL,NULL,NULL,'1',NULL),(9,'TOMEY','AL-200','Fronto',NULL,NULL,NULL,NULL,NULL),(10,'TOMEY','AL-300','Fronto',9,NULL,NULL,NULL,NULL),(11,'TOMEY','TL-3000C','Fronto',8,1,NULL,'1',NULL),(12,'TOMEY','RC-5000','Autoref',NULL,NULL,NULL,NULL,NULL),(13,'TOMEY','RT-7000','Autoref',NULL,NULL,NULL,NULL,NULL),(14,'TOMEY','PRK-7000','Autoref',NULL,NULL,NULL,NULL,NULL),(15,'TOMEY','RC-800','Autoref',NULL,NULL,NULL,NULL,NULL),(16,'ADAPTICA','2WIN','Autoref',NULL,NULL,NULL,NULL,NULL),(17,'TOPCON','CL-300','Fronto',NULL,NULL,NULL,NULL,NULL),(18,'TOPCON','KR-800S','Autoref',NULL,NULL,NULL,NULL,NULL),(19,'VISIONIX','RK F-1','Autoref',NULL,NULL,NULL,NULL,NULL),(20,'NIKON','Retinomax 3','Autoref',NULL,NULL,NULL,NULL,NULL),(21,'NIKON','Retinomax K+3','Autoref',NULL,NULL,NULL,NULL,NULL),(22,'VISIONIX','VX35','Fronto',NULL,NULL,NULL,NULL,NULL),(23,'VISIONIX','VX40','Fronto',NULL,NULL,NULL,NULL,NULL),(24,'NIKON','Speedy-K','Autoref',NULL,NULL,NULL,NULL,NULL),(25,'KEELER','Pulsair','Tonometre',NULL,NULL,NULL,NULL,NULL),(26,'KEELER','Intellipuff','Tonometre',NULL,NULL,NULL,NULL,NULL),(27,'CANON','TX-20','Tonometre',NULL,NULL,NULL,NULL,NULL),(28,'TOMEY','FT-1000','Tonometre',NULL,NULL,NULL,NULL,NULL),(29,'TOPCON','CT-800','Tonometre',NULL,NULL,NULL,NULL,NULL),(30,'REICHERT','PT-100','Tonometre',NULL,NULL,NULL,NULL,NULL),(31,'VISIONIX','VL1000','Fronto',NULL,1,NULL,'1',NULL),(32,'HUVITZ','CLM7000','Fronto',31,1,NULL,'1',NULL),(33,'NIDEK','RT-5100','Refracteur',NULL,1,NULL,'1',NULL),(34,'NIDEK','ARK-1A','Autoref',NULL,1,1,'1',NULL),(35,'NIDEK','ARK-1','Autoref',34,1,1,'1',NULL),(36,'NIDEK','ARK-1S','Autoref',34,1,1,'1',NULL),(37,'NIDEK','AR-1','Autoref',34,1,1,'1',NULL),(38,'NIDEK','AR-1A','Autoref',34,1,1,'1',NULL),(39,'NIDEK','AR-1S','Autoref',34,1,1,'1',NULL),(40,'NIDEK','ARK-530A','Autoref',NULL,1,1,'1',NULL),(41,'NIDEK','ARK-510A','Autoref',40,1,1,'1',NULL),(42,'NIDEK','LM-1800P','Fronto',NULL,1,NULL,'1',NULL),(43,'NIDEK','LM-1800PD','Fronto',42,1,NULL,'1',NULL),(44,'NIDEK','HandyRef-K','Autoref',40,1,1,'1',NULL),(45,'NIDEK','RT-2100','Refracteur',33,1,NULL,'1',NULL),(46,'NIDEK','TONOREF III','Autoref',NULL,1,1,'1',NULL),(47,'NIDEK','ARK-30','Autoref',NULL,1,NULL,'1',NULL),(48,'NIDEK','AR-20','Autoref',NULL,1,NULL,'1',NULL),(49,'NIDEK','LM-500','Fronto',NULL,1,1,'1',NULL),(50,'NIDEK','RT-6100','Refracteur',NULL,NULL,1,'1',NULL),(51,'NIDEK','Glasspop','Refracteur',50,NULL,1,'1',NULL),(52,'NIDEK','LM-7','Fronto',NULL,1,1,'1',NULL),(53,'TOMEY','TL-6100','Fronto ',NULL,1,NULL,NULL,NULL),(54,'RODENSTOCK','AL 6500','Fronto',53,1,NULL,NULL,NULL),(55,'RODENSTOCK','AL 6700','Fronto',53,1,NULL,NULL,NULL),(56,'TOMEY','TL-6000','Fronto ',NULL,1,1,'1',NULL),(57,'RODENSTOCK','AL 6600','Fronto',56,1,1,'1',NULL),(58,'TOMEY','RC-5000','Autoref',NULL,1,1,'1',NULL),(59,'RODENSTOCK','CX 2000','Autoref',58,1,1,'1',NULL),(60,'TOMEY','TOP-1000','Tonometre',NULL,1,1,'1',NULL),(61,'RODENSTOCK','Topascope','Tonometre',60,1,1,'1',NULL),(62,'TOMEY','TAP-2000','Refracteur',NULL,1,NULL,'1',NULL),(63,'RODENSTOCK','Phoromat 2000','Refracteur',62,1,NULL,'1',NULL),(64,'TOMEY','TL-7000','Fronto ',56,1,1,'1',NULL);

SELECT COUNT(*) INTO tot FROM
    (SELECT NomAppareil
    FROM listeappareils
    WHERE NomAppareil = 'Spectralis SHIFT') as chp;
    IF tot=0
    THEN
        INSERT INTO `rufus`.`listeappareils` (`TitreExamen`, `NomAppareil`) VALUES ('0CT', 'Spectralis SHIFT');
    END IF;
SELECT COUNT(*) INTO tot FROM
    (SELECT * FROM INFORMATION_SCHEMA.COLUMNS
		WHERE column_key = 'PRI'
		AND table_name = 'salledattente'
		AND column_name = 'idPat') as chp;
    IF tot=1
    THEN
        ALTER TABLE `rufus`.`salledattente`
        DROP PRIMARY KEY;
    END IF;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 72;

END|

CALL MAJ72();
DROP PROCEDURE IF EXISTS MAJ72;
