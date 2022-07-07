-- 26/06/2022

USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ71;
DELIMITER |
CREATE PROCEDURE MAJ71()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Metadocuments' AND COLUMN_NAME = 'TextMetaDocument') as chp;)
        IF tot=1
        THEN
            ALTER TABLE `Metadocuments` 
            DROP COLUMN `TextMetaDocument`;
        END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT COLUMN_KEY
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME = 'flags' AND COLUMN_NAME = 'MAJflagImpressions') as chp;
            IF tot=0
            THEN
                ALTER TABLE `rufus`.`flags`
                ADD COLUMN `MAJflagImpressions` INT(11) NULL DEFAULT NULL AFTER `flagUserDistant`;
            END IF;
        SELECT COUNT(*) INTO tot FROM
            (SELECT COLUMN_KEY
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_NAME = 'utilisateurs' AND COLUMN_NAME = 'Soignant') as chp;
            IF tot=1
            THEN
                ALTER TABLE `rufus`.`utilisateurs`
                CHANGE COLUMN `Soignant` `Soignant` TINYINT(1) NULL DEFAULT NULL COMMENT '1 = ophtalmo\n2 = orthoptiste\n3 = autre\n4 = Non soignant\n5 = societe comptable\n6 = neutre (poste qui n’interviendra pas sur les données)' ;
            END IF;
UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 71;
END|

CALL MAJ71();
DROP PROCEDURE IF EXISTS MAJ71;

