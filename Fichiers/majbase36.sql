USE `Rufus`;

DROP PROCEDURE IF EXISTS MAJ36;
DELIMITER |
    CREATE PROCEDURE MAJ36()
    BEGIN
    DECLARE tot INT DEFAULT 1;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'DirImagerie') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ParametresSysteme`
            ADD COLUMN `DirImagerie` VARCHAR(250) NULL DEFAULT NULL;
        END IF;
        SELECT COUNT(*) INTO tot FROM
        (SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME='ParametresSysteme' AND COLUMN_NAME = 'LundiBkup') as chp;
        IF tot=0
        THEN
            ALTER TABLE `ParametresSysteme`
            ADD COLUMN `LundiBkup` INT(1) NULL,
            ADD COLUMN `MardiBkup` INT(1) NULL,
            ADD COLUMN `MercrediBkup` INT(1) NULL,
            ADD COLUMN `JeudiBkup` INT(1) NULL,
            ADD COLUMN `VendrediBkup` INT(1) NULL,
            ADD COLUMN `SamediBkup` INT(1) NULL,
            ADD COLUMN `DimancheBkup` INT(1) NULL,
            ADD COLUMN `HeureBkup` TIME(0) NULL,
            ADD COLUMN `DirBkup` VARCHAR(250) NULL;
        END IF;
        UPDATE `ParametresSysteme` SET VersionBase = 36;
    END|
CALL MAJ36();
DROP PROCEDURE MAJ36;

DROP EVENT IF EXISTS VideImagesEchange;
CREATE EVENT VideImagesEchange
ON SCHEDULE EVERY 1 DAY STARTS '2018-03-23 02:00:00'
DO DELETE FROM Images.EchangeImages;

USE `Images`;
DROP TABLE IF EXISTS `EchangeImages`;
CREATE TABLE `EchangeImages` (
  `idImpression` INT NOT NULL,
  `pdf` LONGBLOB NULL,
  `jpg` LONGBLOB NULL,
  `Compression` INT(1) NULL,
  PRIMARY KEY (`idImpression`))
ENGINE=InnoDB;
  
