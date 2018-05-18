USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ10;
DELIMITER |
	CREATE PROCEDURE MAJ10() 
	BEGIN
		DECLARE tot INT DEFAULT 1;
         SELECT COUNT(*) INTO tot FROM        
			(SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
			WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'NUMCENTRE') as chp;
		IF tot=0
			THEN 
				ALTER TABLE `Actes` 
                                ADD COLUMN `NumCentre` int(11) NULL DEFAULT NULL;
				ALTER TABLE `patients` 
                                ADD COLUMN `NumCentre` int(11) NULL DEFAULT NULL;
		END IF;
         SELECT COUNT(*) INTO tot FROM        
			(SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
			WHERE TABLE_NAME='donneessocialespatients' AND COLUMN_NAME = 'PatCMU') as chp;
		IF tot=0
			THEN 
				ALTER TABLE `donneessocialespatients` 
				ADD COLUMN `PatCMU` TINYINT(1) NULL DEFAULT NULL AFTER `PatALD`;
		END IF;
         SELECT COUNT(*) INTO tot FROM        
			(SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
			WHERE TABLE_NAME='utilisateurs' AND COLUMN_NAME = 'Signature') as chp;
		IF tot=0
			THEN 
				ALTER TABLE `utilisateurs` 
				ADD COLUMN `Signature` BLOB NULL DEFAULT NULL AFTER `UserSecteur`,
				ADD COLUMN `OrdoSigne` TINYINT(1) NULL DEFAULT NULL AFTER `Signature`;
		END IF;
	END|
CALL MAJ10();
DROP PROCEDURE MAJ10;

DROP TABLE IF EXISTS `MAJBase`;

DROP TABLE IF EXISTS `ParametresSysteme`;
CREATE TABLE `ParametresSysteme` (
  `MDPAdmin` varchar(8) DEFAULT NULL,
  `NumCentre` int(11) DEFAULT NULL,
  `DocsComprimes` int(1) DEFAULT NULL,
  `VersionBase` int(3) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

LOCK TABLES `ParametresSysteme` WRITE;
INSERT INTO `ParametresSysteme` VALUES ('bob',NULL,1,10);
UNLOCK TABLES;

DROP TABLE IF EXISTS `listeappareils`;
CREATE TABLE `listeappareils` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `TitreExamen` varchar(45) DEFAULT NULL,
  `NomAppareil` varchar(150) DEFAULT NULL,
  `Format` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;

LOCK TABLES `listeappareils` WRITE;
INSERT INTO `listeappareils` VALUES (1,'Biométrie','TOPCON ALADDIN',NULL),(2,'RNM','CANON CR-2',NULL),(3,'OCT','OTI SLO',NULL),(4,'AngioOCT','OPTOVUE',NULL),(5,'OCT','NIDEK-OCT',NULL),(6,'RNM','NIDEK-RNM',NULL),(7,'OCT','SPECTRALIS',NULL),(8,'LOG','NAVIS-EX',NULL),(9,'RNM-E','EIDON',NULL);
UNLOCK TABLES;

DROP TABLE IF EXISTS `MotifsRDV`;
CREATE TABLE `MotifsRDV` (
  `idMotifsRDV` int(11) NOT NULL AUTO_INCREMENT,
  `Motif` varchar(45) NOT NULL,
  `Raccourci` varchar(10) NOT NULL,
  `Couleur` varchar(6) DEFAULT NULL,
  `Duree` int(2) DEFAULT NULL,
  `ParDefaut` int(1) DEFAULT NULL,
  `Utiliser` int(1) DEFAULT NULL,
  `NoOrdre` int(3) DEFAULT NULL,
  PRIMARY KEY (`idMotifsRDV`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;

LOCK TABLES `MotifsRDV` WRITE;
INSERT INTO `MotifsRDV` VALUES (1,'Consultation','CS','FFFFEE',1,1,1,1),(2,'Champ visuel','CV','FFEEFF',NULL,NULL,1,2),(3,'OCT','OCT','DFF2FF',NULL,NULL,1,3),(4,'Biométrie','BM','EEFFFF',NULL,NULL,1,4),(5,'Bilan orthoptique','BO','DDEEDD',NULL,NULL,1,5),(6,'Angiographie','ANG','FFEEEE',NULL,NULL,1,6),(7,'Autre motif','AUT','EFEFEF',NULL,NULL,1,7);
UNLOCK TABLES;

DROP TABLE IF EXISTS `LieuxExercice`;
CREATE TABLE `LieuxExercice` (
  `idLieu` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `LieuAdresse1` varchar(45) CHARACTER SET utf8 DEFAULT NULL,
  `LieuAdresse2` varchar(45) CHARACTER SET utf8 DEFAULT NULL,
  `LieuAdresse3` varchar(45) CHARACTER SET utf8 DEFAULT NULL,
  `LieuCodePostal` mediumint(9) DEFAULT NULL,
  `LieuVille` varchar(45) CHARACTER SET utf8 DEFAULT NULL,
  `LieuTelephone` varchar(17) CHARACTER SET utf8 DEFAULT NULL,
  `LieuFax` varchar(17) CHARACTER SET utf8 DEFAULT NULL,
  PRIMARY KEY (`idLieu`)
) ENGINE=InnoDB AUTO_INCREMENT=100 DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `JointuresLieux`;

CREATE TABLE `JointuresLieux` (
  `idUser` INT NULL,
  `idLieu` INT NULL
) ENGINE=InnoDB AUTO_INCREMENT=100 DEFAULT CHARSET=latin1;
