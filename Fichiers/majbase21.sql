USE `Rufus`;

set @MDP = (select MDPAdmin from Rufus.ParametresSysteme LIMIT 1);
SET @query1 = CONCAT('CREATE USER IF NOT EXISTS "dumprufus"@"localhost" IDENTIFIED BY "',@MDP,'"');
PREPARE stmt FROM @query1; EXECUTE stmt; DEALLOCATE PREPARE stmt;
SET @query1 = CONCAT('GRANT SELECT , SHOW DATABASES , LOCK TABLES , EVENT, SHOW VIEW ON * . * TO "dumprufus"@"localhost" IDENTIFIED BY "',@MDP,'" WITH MAX_QUERIES_PER_HOUR 0 MAX_CONNECTIONS_PER_HOUR 0 MAX_UPDATES_PER_HOUR 0 MAX_USER_CONNECTIONS 0');
PREPARE stmt FROM @query1; EXECUTE stmt; DEALLOCATE PREPARE stmt;

DROP TABLE IF EXISTS `champsfusionImpression`;

LOCK TABLES `ParametresSysteme` WRITE;
UPDATE `ParametresSysteme` SET VersionBase = 21;
UNLOCK TABLES;

