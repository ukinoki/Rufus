USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ15;
DELIMITER |
        CREATE PROCEDURE MAJ15()
        BEGIN
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Patients' AND COLUMN_NAME = 'Commentaire') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Patients`
                ADD COLUMN `Commentaire` blob NULL DEFAULT NULL;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'PointImportant') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Actes`
                ADD COLUMN `PointImportant` int(1) NULL DEFAULT NULL,
                ADD COLUMN `CommentPointImportant` blob NULL DEFAULT NULL;
            END IF;
        END|
CALL MAJ15();
DROP PROCEDURE MAJ15;--


--
-- Table structure for table `MotsCles`
--

DROP TABLE IF EXISTS `MotsCles`;
CREATE TABLE `MotsCles` (
  `idMotCle` int(11) NOT NULL AUTO_INCREMENT,
  `MotCle` varchar(80) NOT NULL,
  PRIMARY KEY (`idMotCle`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `MotsClesJointures`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MotsClesJointures` (
  `idJointure` int(11) NOT NULL AUTO_INCREMENT,
  `idPat` int(11) NOT NULL,
  `idMotCle` int(11) NOT NULL,
  PRIMARY KEY (`idJointure`)
) ENGINE=InnoDB;
/*!40101 SET character_set_client = @saved_cs_client */;


LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 15;
UNLOCK TABLES;
