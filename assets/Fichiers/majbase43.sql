USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ42;

USE `ComptaMedicale`;
DROP PROCEDURE IF EXISTS MAJ43;
DELIMITER |
    CREATE PROCEDURE MAJ43()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='lignescomptes' AND COLUMN_NAME = 'idRecSpec') as chp;
        IF tot=0
        THEN
            ALTER TABLE `lignescomptes` 
            CHANGE COLUMN `idDep` `idDep` INT(11) NULL DEFAULT NULL COMMENT 'id de la dépense correspondante' ,
            CHANGE COLUMN `idRec` `idRec` INT(11) NULL DEFAULT NULL COMMENT 'id de la recette correspondante' ,
            CHANGE COLUMN `LigneDebitCredit` `LigneDebitCredit` INT(1) NOT NULL COMMENT '0 = debit\n1 = credit' ,
            ADD COLUMN `idRecSpec` INT NULL DEFAULT NULL COMMENT 'id de la recette speciale correspondante' AFTER `idRec`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='archivesbanques' AND COLUMN_NAME = 'idRecSpec') as chp;
        IF tot=0
        THEN
            ALTER TABLE `archivesbanques`
            CHANGE COLUMN `idDep` `idDep` INT(11) NULL DEFAULT NULL COMMENT 'id de la dépense correspondante' ,
            CHANGE COLUMN `idRec` `idRec` INT(11) NULL DEFAULT NULL COMMENT 'id de la recette correspondante' ,
            CHANGE COLUMN `LigneDebitCredit` `LigneDebitCredit` INT(1) NOT NULL COMMENT '0 = debit\n1 = credit' ,
            ADD COLUMN `idRecSpec` INT NULL DEFAULT NULL COMMENT 'id de la recette speciale correspondante' AFTER `idRec`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='autresrecettes' AND COLUMN_NAME = 'idCompte') as chp;
        IF tot=1
        THEN
            ALTER TABLE `autresrecettes`
            CHANGE COLUMN `idCompte` `idRemise` INT(11) NULL DEFAULT NULL COMMENT 'id de la remise en cas de cheque',
            ADD COLUMN `CompteVirement` INT(11) NULL DEFAULT NULL COMMENT 'idCompte credite en cas de virement' AFTER `Monnaie`,
            ADD COLUMN `BanqueCheque` VARCHAR(25) NULL DEFAULT NULL COMMENT 'Banque emettrice en cas de cheque' AFTER `CompteVirement`,
            ADD COLUMN `EnAttente` TINYINT(1) NULL DEFAULT NULL COMMENT 'cheque mis en attente encaissement' AFTER `idRemise`;
        END IF;
        update `autresrecettes` set idRemise = null;
    END|
CALL MAJ43();
DROP PROCEDURE IF EXISTS MAJ43;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 43;
