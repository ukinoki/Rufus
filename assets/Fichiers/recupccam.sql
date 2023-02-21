CREATE DEFINER=`serge`@`localhost` PROCEDURE `new_procedure`()
BEGIN
create table ccam.CCAMD as (SELECT distinct b.modificateur, b.montant, b.codeCCAM, c.nom  FROM CCAM.basedepart b inner join ccam.ccam c
on b.codeccam = c.code
 where b.date = '2015-01-01' and b.activite = 1);
END