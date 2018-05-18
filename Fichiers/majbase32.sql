USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ32;
DELIMITER |
    CREATE PROCEDURE MAJ32()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='courriers' AND COLUMN_NAME = 'idUser') as chp;
        IF tot=1
        THEN
            ALTER TABLE `courriers`
            CHANGE COLUMN `idUser` `idUser` INT(11) NULL DEFAULT NULL COMMENT 'Id de l’utilisateur qui a cree le document';
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='courriers' AND COLUMN_NAME = 'AvecDupli') as chp;
        IF tot=1
        THEN
            ALTER TABLE `courriers`
            DROP COLUMN `AvecDupli`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME='courriers' AND COLUMN_NAME = 'Medical') as chp;
        IF tot=0
        THEN
            ALTER TABLE `courriers`
            ADD COLUMN `Medical` TINYINT(1) NULL DEFAULT NULL COMMENT 'document uniquement utilisable par un medecin' AFTER `Editable`;
        END IF;
        UPDATE `ParametresSysteme` SET VersionBase = 32;
        END|
CALL MAJ32();
DROP PROCEDURE MAJ32;


