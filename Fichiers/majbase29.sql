USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ29;
DELIMITER |
    CREATE PROCEDURE MAJ29()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'UserEmployeur') as chp;
        IF tot=0
        THEN
            ALTER TABLE `Utilisateurs`
            ADD COLUMN `UserEmployeur` INT NULL DEFAULT NULL COMMENT 'iduser employeur si user salarie' AFTER `UserEnregHonoraires`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'Soignant') as chp;
        IF tot=1
        THEN
            ALTER TABLE `utilisateurs`
            CHANGE COLUMN `Soignant` `Soignant` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = ophtalmo\n2 = orthoptiste\n3 = autre\n4 = Non soignant\n5 = societe comptable' ;
        END IF;
        END|
CALL MAJ29();
DROP PROCEDURE MAJ29;

UPDATE Actes set PourLeCompteDe = idUser;
 
LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 29;
UNLOCK TABLES;
