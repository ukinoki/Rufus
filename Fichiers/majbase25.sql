USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ25;
DELIMITER |
    CREATE PROCEDURE MAJ25()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'ResponsableActes') as chp;
        IF tot=1
        THEN
        ALTER TABLE `Utilisateurs`
        CHANGE COLUMN `ResponsableActes` `ResponsableActes` INT NULL DEFAULT NULL COMMENT '1 = effectue exlusivement des actes sous sa responsabilite\n2 = effectue des actes sous sa responsabilite et sous celle des autres users\n3 = n effectue aucun acte sous sa responsabilite' AFTER `Soignant`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Utilisateurs' AND COLUMN_NAME = 'UserEnregHonoraires') as chp;
        IF tot=1
        THEN
        ALTER TABLE `Rufus`.`Utilisateurs`
        CHANGE COLUMN `UserEnregHonoraires` `UserEnregHonoraires` TINYINT(1) NULL DEFAULT NULL COMMENT '-1 = Salarie\n1 = enregistre toujours des honoraires\n2 = retrocession honoraires\n3 = enregistre parfois des honoraires\n4 = pas de comptabilite\n5 = societe' ;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'SansCompta') as chp;
        IF tot=1
        THEN
        ALTER TABLE `Rufus`.`ParametresSysteme`
        CHANGE COLUMN `SansCompta` `SansCompta` TINYINT(1) NULL DEFAULT NULL COMMENT '0 = AvecCompta\n1 = SansCompta et sans cotation\n2 = Sans compta mais avec cotation\n3 = Avec compta mais sans cotation';
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'PourLeCompteDe') as chp;
        IF tot=1
        THEN
        ALTER TABLE `Rufus`.`Actes`
        CHANGE COLUMN `PourLeCompteDe` `PourLeCompteDe` INT(11) NULL DEFAULT NULL COMMENT 'id du user qui comptabilise cet acte\n-2 = acte sans compta' ;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='Actes' AND COLUMN_NAME = 'SansCotation') as chp;
        IF tot=0
        THEN
        ALTER TABLE `Rufus`.`Actes`
        ADD COLUMN `SansCotation` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = Sans cotation' AFTER `CommentPointImportant`;
        END IF;
        END|
CALL MAJ25();
DROP PROCEDURE MAJ25;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 25;
UNLOCK TABLES;
