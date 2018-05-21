USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ41;
DELIMITER |
    CREATE PROCEDURE MAJ41()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='autresrecettes' AND COLUMN_NAME = 'DateRecette') as chp;
        IF tot=0
        THEN
			ALTER TABLE `autresrecettes` 
			CHANGE COLUMN `Date` `DateRecette` DATE NULL,
			CHANGE COLUMN `Libelle` `Libelle` VARCHAR(100) NULL,
			CHANGE COLUMN `Typerecette` `Typerecette` VARCHAR(30) NULL,
			CHANGE COLUMN `Montant` `Montant` DOUBLE NULL,
			ADD COLUMN `idRecette` INT(11) NOT NULL AUTO_INCREMENT FIRST,
			ADD COLUMN `idCompte` INT(11) NULL AFTER `Monnaie`,
			ADD COLUMN `idUser` INT(11) NULL AFTER `idRecette`,
			DROP PRIMARY KEY,
			ADD PRIMARY KEY (`idRecette`); 
		END IF;
		update `autresrecettes` set Paiement = 'E' where Paiement = 'Espèces';
		update `autresrecettes` set Paiement = 'V' where Paiement like 'Virement%';
		update `autresrecettes` set Paiement = 'C' where Paiement = 'Chèque';
		update `autresrecettes` set idCompte = 1 where Paiement = 'V' or Paiement = 'C';
		update `autresrecettes` set idUser = 1;
		UPDATE `Rufus`.`ParametresSysteme` SET VersionBase = 41;
	END|
CALL MAJ41();
DROP PROCEDURE MAJ41;

