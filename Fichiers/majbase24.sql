USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ24;
DELIMITER |
        CREATE PROCEDURE MAJ24()
        BEGIN 
            DECLARE tot INT DEFAULT 1;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'UserEnregHonoraires') as chp;
            IF tot=1
            THEN
                ALTER TABLE `utilisateurs`
                CHANGE COLUMN `UserEnregHonoraires` `UserEnregHonoraires` TINYINT(1) NULL DEFAULT NULL COMMENT '-1 = Salarie\n1 = enregistre toujours des honoraires\n2 = retrocession honoraires\n3 = enregistre parfois des honoraires\n4 = pas de comptabilite' ;
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
                WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'ResponsableActes') as chp;
            IF tot=1
            THEN
                ALTER TABLE `Utilisateurs`
                CHANGE COLUMN `ResponsableActes` `ResponsableActes` INT NULL DEFAULT NULL COMMENT '0 = effectue des actes sous sa responsabilite et sous celle des autres users\n1 = effectue exlusivement des actes sous sa responsabilite\n2 = n effectue aucun acte sous sa responsabilite' AFTER `Soignant`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'UserCCAM') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Rufus`.`Utilisateurs`
                ADD COLUMN `UserCCAM` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = utilise la CCAM\n2 = utilise un autre système de cotation' AFTER `UserSecteur`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'lienversfichier') as chp;
            IF tot=0
            THEN
                ALTER TABLE `Impressions` 
                ADD COLUMN `lienversfichier` VARCHAR(250) NULL DEFAULT NULL COMMENT 'Pointe un lien vers fichier image ou video externe' AFTER `formatautre`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'idUser') as chp;
            IF tot=1
            THEN
                ALTER TABLE `Actes`
                CHANGE COLUMN `idUser` `idUser` INT NULL DEFAULT NULL COMMENT 'id du Soignant qui supervise cet acte';
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'CreePar') as chp;
            IF tot=1
            THEN
                ALTER TABLE `Actes`
                CHANGE COLUMN `CreePar` `CreePar` INT NULL DEFAULT NULL COMMENT 'id du Soignant qui a cree cet acte';
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'PourLeCompteDe') as chp;
            IF tot=1
            THEN
                ALTER TABLE `Actes`
                CHANGE COLUMN `PourLeCompteDe` `PourLeCompteDe` INT NULL DEFAULT NULL COMMENT 'id du user qui comptabilise cet acte';
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES 
                                 WHERE TABLE_SCHEMA = 'ComptaMedicale' AND TABLE_NAME = 'ComptesJointures') as chp;
            IF tot=0
            THEN
                CREATE TABLE `ComptaMedicale`.`ComptesJointures` (
                `idJointure` INT NOT NULL AUTO_INCREMENT,
                `idCompte` INT NOT NULL,
                `idUser` INT NOT NULL,
                PRIMARY KEY (`idJointure`));
                INSERT INTO  `ComptaMedicale`.`ComptesJointures` (idCompte, idUser)
                  SELECT  idCompte, idUser from  `ComptaMedicale`.`Comptes`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_SCHEMA = 'ComptaMedicale' AND TABLE_NAME='Comptes' AND COLUMN_NAME = 'Partage') as chp;
            IF tot=0
            THEN
                ALTER TABLE `ComptaMedicale`.`comptes`
                ADD COLUMN `Partage` TINYINT(1) NULL DEFAULT NULL COMMENT '1= compte commun' AFTER `SoldeSurDernierReleve`;
            END IF;
            SELECT COUNT(*) INTO tot FROM
                (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
                WHERE TABLE_SCHEMA = 'ComptaMedicale' AND TABLE_NAME='Comptes' AND COLUMN_NAME = 'idUser') as chp;
            IF tot=1
            THEN
                ALTER TABLE `ComptaMedicale`.`comptes`
                CHANGE COLUMN `idUser` `idUser` INT(11) NOT NULL COMMENT 'User createur du compte';
            END IF;
       END|
CALL MAJ24();
DROP PROCEDURE MAJ24;

DROP TABLE IF EXISTS `listeappareils`;
CREATE TABLE `listeappareils` (
  `idAppareil` int(11) NOT NULL AUTO_INCREMENT,
  `TitreExamen` varchar(45) DEFAULT NULL,
  `NomAppareil` varchar(150) DEFAULT NULL,
  `Format` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idAppareil`)
) ENGINE=InnoDB;

LOCK TABLES `listeappareils` WRITE;
INSERT INTO `listeappareils` VALUES (1,'Biométrie','TOPCON ALADDIN',NULL),(2,'RNM','CANON CR-2',NULL),(3,'OCT','OTI SLO',NULL),(4,'AngioOCT','OPTOVUE',NULL),(5,'OCT','NIDEK-OCT',NULL),(6,'RNM','NIDEK-RNM',NULL),(7,'OCT','SPECTRALIS',NULL),(8,'LOG','NAVIS-EX',NULL),(9,'RNM-E','EIDON',NULL),(10,'RNM-O','OPTOS Daytona series',NULL),(11,'OCT','CANON-OCT',NULL),(12,'Biométrie','TOPCON ALADDIN II',NULL),(13,'OCT','Zeiss Cirrus 5000',NULL);
UNLOCK TABLES;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 24;
UNLOCK TABLES;
