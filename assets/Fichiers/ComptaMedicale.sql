-- MySQL dump 10.13  Distrib 5.7.19, for macos10.12 (x86_64)
--
-- Host: localhost    Database: ComptaMedicale
-- ------------------------------------------------------
-- Server version	5.7.19

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `ComptaMedicale`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `ComptaMedicale` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `ComptaMedicale`;

--
-- Table structure for table `Comptes`
--

DROP TABLE IF EXISTS `Comptes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Comptes` (
  `idCompte` int(11) NOT NULL AUTO_INCREMENT,
  `idBanque` int(11) NOT NULL,
  `idUser` int(11) NOT NULL COMMENT 'User createur du compte',
  `IBAN` varchar(45) DEFAULT NULL,
  `IntituleCompte` varchar(80) NOT NULL,
  `NomCompteAbrege` varchar(10) DEFAULT NULL,
  `SoldeSurDernierReleve` decimal(10,2) DEFAULT NULL,
  `Partage` tinyint(1) DEFAULT NULL COMMENT '1= compte commun',
  `Desactive` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`idCompte`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Depenses`
--

DROP TABLE IF EXISTS `Depenses`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Depenses` (
  `idDep` int(11) NOT NULL AUTO_INCREMENT,
  `idUser` int(11) DEFAULT NULL,
  `DateDep` date NOT NULL,
  `RefFiscale` varchar(100) NOT NULL,
  `Objet` varchar(100) NOT NULL,
  `Montant` decimal(8,2) NOT NULL,
  `FamFiscale` varchar(47) DEFAULT NULL,
  `Nooperation` int(11) DEFAULT NULL,
  `Monnaie` varchar(1) DEFAULT NULL,
  `idRec` int(11) DEFAULT NULL COMMENT 'Ce champ sert à stocker l’idRec de LignesRecettes d’une commission sur la recette IdRec.\nSi on veut supprimer la recette idRec, on retrouve ainsi sans pb la commission correspondante.',
  `ModePaiement` varchar(1) DEFAULT NULL,
  `Compte` int(11) DEFAULT NULL,
  `Nocheque` int(11) DEFAULT NULL,
  `idFacture` int(11) DEFAULT NULL,
  PRIMARY KEY (`idDep`),
  UNIQUE KEY `idCom_UNIQUE` (`idRec`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Depenses`
--

LOCK TABLES `Depenses` WRITE;
/*!40000 ALTER TABLE `Depenses` DISABLE KEYS */;
/*!40000 ALTER TABLE `Depenses` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Factures`
--

DROP TABLE IF EXISTS `Factures`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Factures` (
  `idFacture` int(11) NOT NULL AUTO_INCREMENT,
  `DateFacture` date DEFAULT NULL,
  `LienFichier` varchar(250) DEFAULT NULL,
  `Intitule` varchar(250) DEFAULT NULL,
  `Echeancier` int(1) DEFAULT NULL,
  `idDepense` int(11) DEFAULT NULL,
  `pdf` longblob,
  `jpg` longblob,
  PRIMARY KEY (`idFacture`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Factures`
--

LOCK TABLES `Factures` WRITE;
/*!40000 ALTER TABLE `Factures` DISABLE KEYS */;
/*!40000 ALTER TABLE `Factures` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `archivesbanques`
--

DROP TABLE IF EXISTS `archivesbanques`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `archivesbanques` (
  `idLigne` int(11) NOT NULL,
  `idCompte` int(11) NOT NULL,
  `idDep` int(11) DEFAULT NULL COMMENT 'id de la dépense correspondante',
  `idRec` int(11) DEFAULT NULL COMMENT 'id de la recette correspondante',
  `idRecSpec` int(11) DEFAULT NULL COMMENT 'id de la recette speciale correspondante',
  `idRemCheq` int(11) DEFAULT NULL COMMENT 'id de la remise de cheques correspondante',
  `LigneDate` date NOT NULL,
  `LigneLibelle` varchar(50) NOT NULL,
  `LigneMontant` decimal(12,2) DEFAULT NULL,
  `LigneDebitCredit` int(1) NOT NULL COMMENT '0 = debit\n1 = credit',
  `LigneTypeoperation` varchar(30) DEFAULT NULL,
  `LigneDateconsolidation` date NOT NULL,
  `idArchive` int(11) NOT NULL,
  PRIMARY KEY (`idLigne`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `archivesbanques`
--

LOCK TABLES `archivesbanques` WRITE;
/*!40000 ALTER TABLE `archivesbanques` DISABLE KEYS */;
/*!40000 ALTER TABLE `archivesbanques` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `autresrecettes`
--

DROP TABLE IF EXISTS `autresrecettes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `autresrecettes` (
  `idRecette` int(11) NOT NULL AUTO_INCREMENT,
  `idUser` int(11) DEFAULT NULL,
  `DateRecette` date DEFAULT NULL,
  `Libelle` varchar(100) DEFAULT NULL,
  `Typerecette` varchar(30) DEFAULT NULL,
  `Montant` double DEFAULT NULL,
  `Paiement` varchar(19) CHARACTER SET latin1 DEFAULT NULL,
  `Nooperation` int(11) DEFAULT NULL,
  `Monnaie` varchar(1) CHARACTER SET latin1 DEFAULT NULL,
  `CompteVirement` int(11) DEFAULT NULL COMMENT 'idCompte credite en cas de virement',
  `BanqueCheque` varchar(25) DEFAULT NULL COMMENT 'Banque emettrice en cas de cheque',
  `TireurCheque` varchar(45) DEFAULT NULL,
  `DateEnregistrement` date DEFAULT NULL COMMENT 'date de la remise en cas de cheque',
  `idRemise` int(11) DEFAULT NULL COMMENT 'id de la remise en cas de cheque',
  `EnAttente` tinyint(1) DEFAULT NULL COMMENT 'cheque mis en attente encaissement',
  PRIMARY KEY (`idRecette`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `autresrecettes`
--

LOCK TABLES `autresrecettes` WRITE;
/*!40000 ALTER TABLE `autresrecettes` DISABLE KEYS */;
/*!40000 ALTER TABLE `autresrecettes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `banques`
--

DROP TABLE IF EXISTS `banques`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `banques` (
  `IdBanque` int(11) NOT NULL AUTO_INCREMENT,
  `idBanqueAbrege` varchar(12) NOT NULL,
  `NomBanque` varchar(45) DEFAULT NULL,
  `CodeBanque` int(11) DEFAULT NULL,
  PRIMARY KEY (`IdBanque`)
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `banques`
--

LOCK TABLES `banques` WRITE;
/*!40000 ALTER TABLE `banques` DISABLE KEYS */;
INSERT INTO `banques` VALUES (1,'SG','Société Générale',NULL),(2,'BF','Banque de France',NULL),(3,'BNP','Banque nationale de Paris',NULL),(4,'BOURSORAMA','Boursorama',NULL),(5,'BPPC','Banque populaire provençale et corse',NULL),(6,'CA','Crédit agricole',NULL),(7,'CCP','La Poste',NULL),(8,'CE','Caisse d\'épargne',NULL),(9,'GROUPAMA','Groupama',NULL),(10,'LCL','Crédit Lyonnais',NULL),(11,'SMC','Société marseillaise de crédit',NULL),(12,'BMM','Banque Martin Maurel',NULL),(13,'CIC','Crédit industriel et commercial',NULL),(14,'CM','Crédit mutuel',NULL),(15,'ING','Ing Direct',NULL),(16,'HSBC','Hsbc',NULL),(17,'BPI','Banco Bpi',NULL);
/*!40000 ALTER TABLE `banques` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `lignescomptes`
--

DROP TABLE IF EXISTS `lignescomptes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lignescomptes` (
  `idLigne` int(11) NOT NULL AUTO_INCREMENT,
  `idCompte` int(11) NOT NULL,
  `idDep` int(11) DEFAULT NULL COMMENT 'id de la dépense correspondante',
  `idRec` int(11) DEFAULT NULL COMMENT 'id de la recette correspondante',
  `idRecSpec` int(11) DEFAULT NULL COMMENT 'id de la recette speciale correspondante',
  `idRemCheq` int(11) DEFAULT NULL COMMENT 'id de la remise de cheques correspondante',
  `LigneDate` date NOT NULL,
  `LigneLibelle` varchar(50) NOT NULL,
  `LigneMontant` decimal(10,2) NOT NULL,
  `LigneDebitCredit` int(1) NOT NULL COMMENT '0 = debit\n1 = credit',
  `LigneTypeoperation` varchar(45) NOT NULL,
  `LigneConsolide` int(1) DEFAULT NULL,
  PRIMARY KEY (`idLigne`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `lignescomptes`
--

LOCK TABLES `lignescomptes` WRITE;
/*!40000 ALTER TABLE `lignescomptes` DISABLE KEYS */;
/*!40000 ALTER TABLE `lignescomptes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `lignespaiements`
--

DROP TABLE IF EXISTS `lignespaiements`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lignespaiements` (
  `idActe` int(11) NOT NULL,
  `idRecette` int(11) NOT NULL,
  `Paye` decimal(8,2) DEFAULT NULL,
  PRIMARY KEY (`idActe`,`idRecette`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `lignespaiements`
--

LOCK TABLES `lignespaiements` WRITE;
/*!40000 ALTER TABLE `lignespaiements` DISABLE KEYS */;
/*!40000 ALTER TABLE `lignespaiements` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `lignesrecettes`
--

DROP TABLE IF EXISTS `lignesrecettes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lignesrecettes` (
  `idRecette` int(11) NOT NULL AUTO_INCREMENT,
  `idUser` int(11) DEFAULT NULL,
  `DatePaiement` date DEFAULT NULL,
  `DateEnregistrement` date DEFAULT NULL,
  `Montant` decimal(8,2) DEFAULT NULL,
  `ModePaiement` varchar(1) DEFAULT NULL,
  `TireurCheque` varchar(45) DEFAULT NULL,
  `CompteVirement` int(11) DEFAULT NULL,
  `BanqueCheque` varchar(25) DEFAULT NULL,
  `TiersPayant` varchar(1) DEFAULT NULL,
  `NomTiers` varchar(45) DEFAULT NULL,
  `Commission` decimal(8,2) DEFAULT NULL,
  `Monnaie` varchar(1) DEFAULT NULL,
  `idRemise` int(11) DEFAULT NULL,
  `EnAttente` tinyint(1) DEFAULT NULL COMMENT 'Ce champ concerne les chèques reçus dont le tireur a demandé que l’on attende pour l’encaissement.',
  `EnregistrePar` int(11) DEFAULT NULL,
  `TypeRecette` int(11) DEFAULT NULL,
  PRIMARY KEY (`idRecette`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `lignesrecettes`
--

LOCK TABLES `lignesrecettes` WRITE;
/*!40000 ALTER TABLE `lignesrecettes` DISABLE KEYS */;
/*!40000 ALTER TABLE `lignesrecettes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `remisecheques`
--

DROP TABLE IF EXISTS `remisecheques`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `remisecheques` (
  `idRemCheq` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Montant` decimal(10,2) unsigned NOT NULL,
  `RCDate` date NOT NULL,
  `idCompte` int(11) NOT NULL,
  PRIMARY KEY (`idRemCheq`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `remisecheques`
--

LOCK TABLES `remisecheques` WRITE;
/*!40000 ALTER TABLE `remisecheques` DISABLE KEYS */;
/*!40000 ALTER TABLE `remisecheques` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `rubriques2035`
--

DROP TABLE IF EXISTS `rubriques2035`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rubriques2035` (
  `idRubrique` smallint(6) NOT NULL,
  `Reffiscale` varchar(50) DEFAULT NULL,
  `Famfiscale` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`idRubrique`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rubriques2035`
--

LOCK TABLES `rubriques2035` WRITE;
/*!40000 ALTER TABLE `rubriques2035` DISABLE KEYS */;
INSERT INTO `rubriques2035` VALUES (2,'Débours payés',''),(3,'Honoraires rétrocédés',''),(5,'Produits financiers',''),(6,'Gains divers',''),(8,'Achats','Achats'),(9,'Frais de personnel - Salaires nets','Frais de personnel'),(10,'Frais de personnel - Charges sociales sur salaires','Frais de personnel'),(11,'Taxe sur la valeur ajoutée','Impôts et taxes'),(12,'CFE/CVAE (ancienne taxe professionnelle)','Impôts et taxes'),(13,'Autres impôts','Impôts et taxes'),(14,'Contribution sociale généralisée déductible','Impôts et taxes'),(15,'Loyer et charges locatives','Loyers et charges locatives'),(16,'Location de matériel et de mobilier','Location de matériel et de mobilier'),(17,'Entretien et réparations','Travaux, fournitures et services extérieurs'),(18,'Personnel intérimaire','Travaux, fournitures et services extérieurs'),(19,'Petit outillage','Travaux, fournitures et services extérieurs'),(20,'Chauffage, eau, gaz, électricité','Travaux, fournitures et services extérieurs'),(21,'Honoraires ne constituant pas des rétrocessions','Travaux, fournitures et services extérieurs'),(22,'Primes d\'assurances','Travaux, fournitures et services extérieurs'),(23,'Frais de voiture automobile','Transports et déplacements'),(24,'Autres frais de déplacement','Transports et déplacements'),(25,'Charges sociales personnelles','Charges sociales personnelles'),(26,'Frais de réception, représentation et congrès','Frais de réception, représentation et congrès'),(27,'Fournitures de bureau, documentation, PTT','Frais divers de gestion'),(28,'Frais d\'actes et de contentieux','Frais divers de gestion'),(29,'Cotisations syndicales et professionnelles','Frais divers de gestion'),(30,'Autres frais divers de gestion','Frais divers de gestion'),(31,'Frais financiers','Frais financiers'),(32,'Pertes diverses','Pertes diverses'),(41,'Amortissements','Amortissements'),(99,'Compte d\'attente SCM','Compte d\'attente SCM'),(100,'Prélèvement personnel','Prélèvement personnel');
/*!40000 ALTER TABLE `rubriques2035` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tiers`
--

DROP TABLE IF EXISTS `tiers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tiers` (
  `idTiers` int(11) NOT NULL AUTO_INCREMENT,
  `Nomtiers` varchar(25) CHARACTER SET latin1 NOT NULL,
  `Adresse1Tiers` varchar(80) DEFAULT NULL,
  `Adresse2Tiers` varchar(80) DEFAULT NULL,
  `Adresse3Tiers` varchar(80) DEFAULT NULL,
  `Codepostaltiers` varchar(5) DEFAULT NULL,
  `Villetiers` varchar(25) DEFAULT NULL,
  `Telephonetiers` varchar(17) DEFAULT NULL,
  `FaxTiers` varchar(17) DEFAULT NULL,
  `MailTiers` varchar(45) DEFAULT NULL,
  `WebsiteTiers` varchar(120) DEFAULT NULL,
  `InterlocTiers` varchar(20) DEFAULT NULL,
  `SexeInterlocTiers` varchar(1) DEFAULT NULL,
  `PosteInterlocTiers` varchar(8) DEFAULT NULL,
  `TiersParDefaut` varchar(1) DEFAULT NULL,
  PRIMARY KEY (`idTiers`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tiers`
--

LOCK TABLES `tiers` WRITE;
/*!40000 ALTER TABLE `tiers` DISABLE KEYS */;
INSERT INTO `tiers` VALUES (3,'Art 115',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(4,'CAMAC',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(5,'CB',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(7,'CNMSS',NULL,NULL,NULL,'83090','TOULON CEDEX 9','94 16 36 00','',NULL,NULL,'','','',''),(13,'Divers',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(19,'LMDE',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(22,'MG PTT','18, rue Dieudé',NULL,NULL,'13292','Marseille CEDEX 06','','',NULL,NULL,'','','',''),(23,'MGEN',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(24,'MGS',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(25,'MPCA','5/7 rue d\'Italie BP 83',NULL,NULL,'13253','Marseille CEDEX 06','','',NULL,NULL,'','','',''),(26,'MSA',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(28,'RAM',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(29,'RSI',NULL,NULL,NULL,'','','','',NULL,NULL,'','','',''),(33,'SNCF','Caisse de prévoyance et de retraite',NULL,NULL,'13347','Marseille CEDEX 20','91 95 06 45','',NULL,NULL,'','','',''),(35,'Tutelle',NULL,NULL,NULL,'','','','',NULL,NULL,'','','','');
/*!40000 ALTER TABLE `tiers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `typepaiementactes`
--

DROP TABLE IF EXISTS `typepaiementactes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `typepaiementactes` (
  `idActe` int(11) NOT NULL,
  `TypePaiement` varchar(1) DEFAULT NULL,
  `Tiers` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idActe`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `typepaiementactes`
--

LOCK TABLES `typepaiementactes` WRITE;
/*!40000 ALTER TABLE `typepaiementactes` DISABLE KEYS */;
/*!40000 ALTER TABLE `typepaiementactes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `verrouscomptaactes`
--

DROP TABLE IF EXISTS `verrouscomptaactes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `verrouscomptaactes` (
  `idActe` int(11) NOT NULL,
  `DateTimeVerrou` datetime DEFAULT NULL,
  `PosePar` int(11) DEFAULT NULL,
  PRIMARY KEY (`idActe`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `verrouscomptaactes`
--

LOCK TABLES `verrouscomptaactes` WRITE;
/*!40000 ALTER TABLE `verrouscomptaactes` DISABLE KEYS */;
/*!40000 ALTER TABLE `verrouscomptaactes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping events for database 'ComptaMedicale'
--
/*!50106 SET @save_time_zone= @@TIME_ZONE */ ;
/*!50106 DROP EVENT IF EXISTS `VideFactures` */;
DELIMITER ;;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;;
/*!50003 SET character_set_client  = utf8mb4 */ ;;
/*!50003 SET character_set_results = utf8mb4 */ ;;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;;
/*!50003 SET @saved_time_zone      = @@time_zone */ ;;
/*!50003 SET time_zone             = 'SYSTEM' */ ;;
/*!50106 CREATE*/ /*!50117 DEFINER=`adminrufus`@`%`*/ /*!50106 EVENT `VideFactures` ON SCHEDULE EVERY 1 DAY STARTS '2018-03-23 00:00:00' ON COMPLETION NOT PRESERVE ENABLE DO UPDATE ComptaMedicale.Factures SET jpg = null, pdf = null */ ;;
/*!50003 SET time_zone             = @saved_time_zone */ ;;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;;
/*!50003 SET character_set_client  = @saved_cs_client */ ;;
/*!50003 SET character_set_results = @saved_cs_results */ ;;
/*!50003 SET collation_connection  = @saved_col_connection */ ;;
DELIMITER ;
/*!50106 SET TIME_ZONE= @save_time_zone */ ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2020-04-13 15:31:55
