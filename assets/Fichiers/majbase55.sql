-- 22/05/2019
-- ajoute 2 nouvelles cotations de chirurgie de cataracte à la CCAM
USE  `rufus`;

INSERT INTO `rufus`.`ccam` (`idccam`, `codeccam`, `nom`, `optam`, `nonoptam`) VALUES (8702,'BFGA368', 'Extraction extracapsulaire du cristallin par phakoémulsification, avec implantation de cristallin artificiel dans la chambre postérieure de l\'oeil, avec implantation de dispositif de drainage trabéculaire ab interno', 252.96, 252.96);
INSERT INTO `rufus`.`ccam` (`idccam`, `codeccam`, `nom`, `optam`, `nonoptam`) VALUES (8703,'BFGA427', 'Extraction extracapsulaire du cristallin par phakoémulsification, avec implantation de cristallin artificiel dans la chambre postérieure de l\'oeil, sans implantation de dispositif de drainage trabéculaire', 271.70, 271.70);

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 55;

