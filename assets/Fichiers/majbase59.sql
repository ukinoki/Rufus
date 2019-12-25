-- 23/12/2019
USE `rufus`;
DROP PROCEDURE IF EXISTS MAJ59;
DELIMITER |
    CREATE PROCEDURE MAJ59()
BEGIN
    DECLARE tot INT DEFAULT 1;
    SELECT COUNT(*) INTO tot FROM
        (SELECT COLUMN_KEY
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = 'Manufacturers' AND COLUMN_NAME = 'Manwebsite') as chp;
    IF tot=0
        THEN
            ALTER TABLE `Manufacturers`
            ADD COLUMN `ManWebsite` varchar(120) AFTER `ManPortable`,
            CHANGE COLUMN `idManufacturer` `idManufacturer` INT(11) NOT NULL AUTO_INCREMENT ,
            CHANGE COLUMN `ManPortable` `ManPortable` VARCHAR(17) NULL DEFAULT NULL ,
            CHANGE COLUMN `ManMail` `ManMail` VARCHAR(45) NULL DEFAULT NULL ;
    END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT idUser FROM rufus.utilisateurs where UserLogin = 'Admin') as chp;
    IF tot=0
        THEN
            INSERT INTO rufus.utilisateurs (idUser, UserNom, UserLogin) values ('Admin','Admin','bob');
    END IF;
END|

CALL MAJ59();
DROP PROCEDURE IF EXISTS MAJ59;

CREATE USER IF NOT EXISTS 'adminrufus'@'%' IDENTIFIED BY 'gaxt78iy';
GRANT ALL ON *.* TO 'adminrufus'@'%' IDENTIFIED BY 'gaxt78iy' WITH GRANT OPTION;

CREATE USER IF NOT EXISTS 'adminrufusSSL'@'%' IDENTIFIED BY 'gaxt78iy' REQUIRE SSL;
GRANT ALL ON *.* TO 'adminrufusSSL'@'%' IDENTIFIED BY 'gaxt78iy' WITH GRANT OPTION;

UPDATE `rufus`.`ParametresSysteme` SET VersionBase = 59;
