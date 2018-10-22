USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ27;
DELIMITER |
    CREATE PROCEDURE MAJ27()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'idUser') as chp;
        IF tot=1
        THEN
        ALTER TABLE `salledattente`
        CHANGE COLUMN `IdUser` `IdUser` INT(11) NULL DEFAULT NULL COMMENT 'id soignant responsable du patient';
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='salledattente' AND COLUMN_NAME = 'idSuperViseur') as chp;
        IF tot=1
        THEN
        ALTER TABLE `salledattente`
        CHANGE COLUMN `idSuperViseur` `idUserEnCoursExam` INT(11) NULL DEFAULT NULL COMMENT 'id soignant qui examine le patient';
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'Soignant') as chp;
        IF tot=1
        THEN
            ALTER TABLE `utilisateurs`
            CHANGE COLUMN `Soignant` `Soignant` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = ophtalmo\n2 = orthoptiste\n3 = autre\n4 = Non soignant' ;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'UserEnregHonoraires') as chp;
        IF tot=1
        THEN
            ALTER TABLE `Utilisateurs`
            CHANGE COLUMN `UserEnregHonoraires` `UserEnregHonoraires` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = liberal\n2 = salarie\n3 = retrocession\n4 = pas de comptabilite' ;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'idCompteEncaissHonoraires') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Utilisateurs`
            ADD COLUMN `idCompteEncaissHonoraires` INT NULL DEFAULT NULL COMMENT 'Compte bancaire encaissement honoraires' AFTER `UserEnregHonoraires`;
        END IF;
        END|
CALL MAJ27();
DROP PROCEDURE MAJ27;

DROP TABLE IF EXISTS `listeappareils`;
CREATE TABLE `listeappareils` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `TitreExamen` varchar(45) DEFAULT NULL,
  `NomAppareil` varchar(150) DEFAULT NULL,
  `Format` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB;

LOCK TABLES `listeappareils` WRITE;
INSERT INTO `listeappareils` VALUES (1,'Biométrie','TOPCON ALADDIN',NULL),(2,'RNM','CANON CR-2',NULL),(3,'OCT','OTI SLO',NULL),(4,'AngioOCT','OPTOVUE',NULL),(5,'OCT','NIDEK-OCT',NULL),(6,'RNM','NIDEK-RNM',NULL),(7,'OCT','SPECTRALIS',NULL),(8,'LOG','NAVIS-EX',NULL),(9,'RNM-E','EIDON',NULL),(10,'RNM-O','OPTOS Daytona series',NULL),(11,'OCT','CANON-OCT',NULL),(12,'Biométrie','TOPCON ALADDIN II',NULL),(13,'OCT','ZEISS CIRRUS 5000',NULL),(14,'OCT','TOPCON TRITON',NULL);
UNLOCK TABLES;

DROP TABLE IF EXISTS `AppareilsRefraction`;
CREATE TABLE `AppareilsRefraction` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `Marque` varchar(45) NOT NULL,
  `Modele` varchar(45) NOT NULL,
  `Type` varchar(45) NOT NULL,
  `SimilaireA` int(11) DEFAULT NULL,
  `Serie` int(1) DEFAULT NULL,
  `LAN` int(1) DEFAULT NULL,
  `Implemente` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB;

LOCK TABLES `AppareilsRefraction` WRITE;
INSERT INTO `AppareilsRefraction` VALUES (1,'TOPCON','CV-5000','Refracteur',NULL,NULL,NULL,NULL),(2,'TOPCON','CV-3000','Refracteur',NULL,NULL,NULL,NULL),(3,'TOPCON','CV-2500','Refracteur',NULL,NULL,NULL,NULL),(4,'NIKON','Remote Vision','Refracteur',NULL,NULL,NULL,NULL),(5,'TOMEY','TAP-2000','Refracteur',NULL,NULL,NULL,NULL),(6,'POTEC','PDR-7000','Refracteur',5,NULL,NULL,NULL),(7,'VISIONIX','VX-60','Refracteur',NULL,NULL,NULL,NULL),(8,'TOMEY','TL-2000C','Fronto',NULL,NULL,NULL,'1'),(9,'TOMEY','AL-200','Fronto',NULL,NULL,NULL,NULL),(10,'TOMEY','AL-300','Fronto',9,NULL,NULL,NULL),(11,'TOMEY','TL-3000C','Fronto',8,1,NULL,'1'),(12,'TOMEY','RC-5000','Autoref',NULL,NULL,NULL,NULL),(13,'TOMEY','RT-7000','Autoref',NULL,NULL,NULL,NULL),(14,'TOMEY','PRK-7000','Autoref',NULL,NULL,NULL,NULL),(15,'TOMEY','RC-800','Autoref',NULL,NULL,NULL,NULL),(16,'ADAPTICA','2WIN','Autoref',NULL,NULL,NULL,NULL),(17,'TOPCON','CL-300','Fronto',NULL,NULL,NULL,NULL),(18,'TOPCON','KR-800S','Autoref',NULL,NULL,NULL,NULL),(19,'VISIONIX','RK F-1','Autoref',NULL,NULL,NULL,NULL),(20,'NIKON','Retinomax 3','Autoref',NULL,NULL,NULL,NULL),(21,'NIKON','Retinomax K+3','Autoref',NULL,NULL,NULL,NULL),(22,'VISIONIX','VX35','Fronto',NULL,NULL,NULL,NULL),(23,'VISIONIX','VX40','Fronto',NULL,NULL,NULL,NULL),(24,'NIKON','Speedy-K','Autoref',NULL,NULL,NULL,NULL),(25,'KEELER','Pulsair','Tonometre',NULL,NULL,NULL,NULL),(26,'KEELER','Intellipuff','Tonometre',NULL,NULL,NULL,NULL),(27,'CANON','TX-20','Tonometre',NULL,NULL,NULL,NULL),(28,'TOMEY','FT-1000','Tonometre',NULL,NULL,NULL,NULL),(29,'TOPCON','CT-800','Tonometre',NULL,NULL,NULL,NULL),(30,'REICHERT','PT-100','Tonometre',NULL,NULL,NULL,NULL),(31,'VISIONIX','VL1000','Fronto',NULL,1,NULL,'1'),(32,'HUVITZ','CLM7000','Fronto',31,1,NULL,'1'),(33,'NIDEK','RT-5100','Refracteur',NULL,1,NULL,'1'),(34,'NIDEK','ARK-1A','Autoref',NULL,1,1,'1'),(35,'NIDEK','ARK-1','Autoref',34,1,1,'1'),(36,'NIDEK','ARK-1S','Autoref',34,1,1,'1'),(37,'NIDEK','AR-1','Autoref',34,1,1,'1'),(38,'NIDEK','AR-1A','Autoref',34,1,1,'1'),(39,'NIDEK','AR-1S','Autoref',34,1,1,'1'),(40,'NIDEK','ARK-530A','Autoref',NULL,1,1,'1'),(41,'NIDEK','ARK-510A','Autoref',40,1,1,'1'),(42,'NIDEK','LM-1800P','Fronto',NULL,1,NULL,'1'),(43,'NIDEK','LM-1800PD','Fronto',42,1,NULL,'1'),(44,'NIDEK','HandyRef-K','Autoref',40,1,1,'1'),(45,'NIDEK','RT-2100','Refracteur',33,1,NULL,'1'),(46,'NIDEK','TONOREF III','Autoref',NULL,1,1,'1'),(47,'NIDEK','ARK-30','Autoref',NULL,1,NULL,'1'),(48,'NIDEK','AR-20','Autoref',NULL,1,NULL,'1'),(49,'NIDEK','LM-500','Fronto',NULL,1,1,'1');
UNLOCK TABLES;


LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 27;
UNLOCK TABLES;
