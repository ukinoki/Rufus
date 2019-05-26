-- 04/04/2019
USE  `rufus`;


ALTER TABLE `rufus`.`Patients` 
ADD FULLTEXT INDEX `idx_nom` (`Patnom` ASC);
ALTER TABLE `rufus`.`Patients` 
ADD FULLTEXT INDEX `idx_prenom` (`PatPrenom` ASC);
ALTER TABLE `rufus`.`Patients` 
ADD FULLTEXT INDEX `idx_nomprenom` (`PatNom` ASC, `PatPrenom` ASC);

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 54;

