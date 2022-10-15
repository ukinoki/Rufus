-- 31/12/2019
USE `Ophtalmologie`;
DROP PROCEDURE IF EXISTS MAJ60;
DELIMITER |
    CREATE PROCEDURE MAJ60()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'pachymetries' AND COLUMN_NAME = 'idPachy') as chp;
        IF tot=0
        THEN
            ALTER TABLE `pachymetries`
            ADD COLUMN `idPachy` INT NOT NULL AUTO_INCREMENT FIRST,
            ADD PRIMARY KEY (`idPachy`),
            CHANGE COLUMN `idPat` `idPat` INT(10) NULL DEFAULT NULL;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT INDEX_NAME
        FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_NAME = 'pachymetries' AND INDEX_NAME = 'idPatTO_idx') as chp;
        IF tot=1
        THEN
            ALTER TABLE `pachymetries`
            DROP INDEX `idPatTO_idx` ;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'speculaires' AND COLUMN_NAME = 'idSpec') as chp;
        IF tot=0
        THEN
            ALTER TABLE `speculaires`
            ADD COLUMN `idSpec` INT NOT NULL AUTO_INCREMENT FIRST,
            ADD PRIMARY KEY (`idSpec`),
            CHANGE COLUMN `idPat` `idPat` INT(10) NULL DEFAULT NULL;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT INDEX_NAME
        FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_NAME = 'speculaires' AND INDEX_NAME = 'idPatTO_idx') as chp;
        IF tot=1
        THEN
            ALTER TABLE `speculaires`
            DROP INDEX `idPatTO_idx` ;
        END IF;
     SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'biometries' AND COLUMN_NAME = 'idBio') as chp;
        IF tot=0
        THEN
            ALTER TABLE `biometries`
            ADD COLUMN `idBio` INT NOT NULL AUTO_INCREMENT FIRST,
            ADD PRIMARY KEY (`idBio`),
            CHANGE COLUMN `idPat` `idPat` INT(10) NULL DEFAULT NULL;
         END IF;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'tonometries' AND COLUMN_NAME = 'idTono') as chp;
        IF tot=0
        THEN
            ALTER TABLE `tonometries`
            ADD COLUMN `idTono` INT NOT NULL AUTO_INCREMENT FIRST,
            ADD PRIMARY KEY (`idTono`),
            CHANGE COLUMN `idPat` `idPat` INT(10) NULL DEFAULT NULL;
        END IF;
    SELECT COUNT(*) INTO tot FROM
       (SELECT INDEX_NAME
       FROM INFORMATION_SCHEMA.STATISTICS
       WHERE TABLE_NAME = 'tonometries' AND INDEX_NAME = 'idPatTO_idx') as chp;
       IF tot=1
       THEN
           ALTER TABLE `tonometries`
           DROP INDEX `idPatTO_idx` ;
       END IF;
END|

CALL MAJ60();
DROP PROCEDURE IF EXISTS MAJ60;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 60;
