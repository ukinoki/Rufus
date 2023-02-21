CREATE SCHEMA `Oplus`;
Use `Oplus`;
CREATE TABLE `Oplus`.`identites_patients` (
  `nopat` INT NOT NULL AUTO_INCREMENT,
  `sexe` VARCHAR(10) NULL,
  `titre` VARCHAR(4) NULL,
  `nom_prenom` VARCHAR(45) NULL,
  `adr1` VARCHAR(100) NULL,
  `adr2` VARCHAR(100) NULL,
  `codepostal` VARCHAR(5) NULL,
  `ville` VARCHAR(100) NULL,
  `date_naiss` VARCHAR(10) NULL,
  `noSS` VARCHAR(45) NULL,
  `nom_naiss` VARCHAR(45) NULL,
  `tel` VARCHAR(45) NULL,
  `tel_bur` VARCHAR(45) NULL,
  `tel_port` VARCHAR(45) NULL,
  `email` VARCHAR(45) NULL,
  `prat_usuel` VARCHAR(45) NULL,
  `med_trait` VARCHAR(45) NULL,
  `org` VARCHAR(45) NULL,
  `profession` VARCHAR(45) NULL,
  `divers` VARCHAR(45) NULL,
  PRIMARY KEY (`nopat`));
LOAD DATA INFILE '/Users/serge/Documents/Rufus/laine/identites_patients.csv'
INTO TABLE `Oplus`.`identites_patients`
FIELDS
    TERMINATED BY '|'
LINES
    TERMINATED BY '\n'
    IGNORE 1 LINES;
CREATE TABLE `Oplus`.`actes` (
  `nopat` INT NOT NULL,
  `dateheure` VARCHAR(15) NULL,
  `rubrique` VARCHAR(15) NULL,
  `valeur` VARCHAR(1000) NULL);
LOAD DATA INFILE '/Users/serge/Documents/Rufus/laine/actes.csv'
INTO TABLE `Oplus`.`actes`
FIELDS
    TERMINATED BY '|'
LINES
    TERMINATED BY '\n'
    IGNORE 1 LINES;