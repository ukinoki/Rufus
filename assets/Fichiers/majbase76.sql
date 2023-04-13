-- 13/04/2023
ALTER TABLE `rufus`.`cotations` 
CHANGE COLUMN `MontantOPTAM` `MontantOPTAM` DECIMAL(9,2) NULL DEFAULT NULL ,
CHANGE COLUMN `MontantNonOPTAM` `MontantNonOPTAM` DECIMAL(9,2) NULL DEFAULT NULL ,
CHANGE COLUMN `MontantPratique` `MontantPratique` DECIMAL(9,2) NULL DEFAULT NULL ;

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ76;
DELIMITER |
CREATE PROCEDURE MAJ76()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'idCompteEncaissHonoraires') as chp;
        IF tot=1
        THEN
                ALTER TABLE `rufus`.`utilisateurs`
                DROP COLUMN `idCompteEncaissHonoraires`;
        END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 76;
END|

CALL MAJ76();
DROP PROCEDURE IF EXISTS MAJ76;
