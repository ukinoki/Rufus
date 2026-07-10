-- ============================================================================
--  reset-adminrufus-test.sql — remet les comptes MySQL de Rufus dans l'état
--  ANTÉRIEUR (base NON sécurisée), pour rejouer les tests de sécurisation.
--
--  Ce que fait le script :
--    1. supprime TOUTES les occurrences des comptes 'adminrufus%' (adminrufus,
--       adminrufusSSL, sur TOUS les hosts : localhost, 192.168.%, %, etc.) ;
--    2. recrée UNIQUEMENT :
--         • adminrufus@'%'      (sans SSL)
--         • adminrufusSSL@'%'   (REQUIRE SSL)
--       tous deux avec le SEUL mot de passe générique gaxt78iy (MDP_SQL),
--       plugin mysql_native_password, ALL PRIVILEGES … WITH GRANT OPTION,
--       et SANS mot de passe alternatif (pas de RETAIN CURRENT PASSWORD).
--
--  Résultat = base « non sécurisée » : adminrufusEstSecurise() renverra false,
--  et le prochain poste local relancera la sécurisation depuis zéro.
--
--  À exécuter avec le client mysql, connecté en ROOT (ou un superuser), PAS en
--  adminrufus :   mysql -u root -p < reset-adminrufus-test.sql
--  (le mot de passe gaxt78iy ci-dessous n'est pas un secret : il est déjà en
--   clair dans le code, macros.h → MDP_SQL.)
-- ============================================================================

-- --- 1. Suppression de toutes les occurrences adminrufus% -------------------
-- On construit dynamiquement UNE SEULE instruction DROP USER (MySQL n'accepte
-- pas de joker sur le host, mais accepte plusieurs comptes séparés par des
-- virgules). QUOTE() échappe proprement user et host.
-- ⚠️ PREPARE ne prépare qu'UNE instruction : surtout pas de « ; » entre les
--    comptes, sinon erreur 1064.
SET SESSION group_concat_max_len = 100000;

SELECT CONCAT('DROP USER IF EXISTS ',
              GROUP_CONCAT(CONCAT(QUOTE(User), '@', QUOTE(Host)) SEPARATOR ', '))
  INTO @drops
  FROM mysql.user
 WHERE User LIKE 'adminrufus%';

-- Aucun compte trouvé → @drops vaut NULL : on met une instruction neutre pour
-- que PREPARE ne plante pas.
SET @drops = IFNULL(@drops, 'DO 0');

PREPARE st FROM @drops;
EXECUTE st;
DEALLOCATE PREPARE st;

-- --- 2. Recréation de l'état antérieur (générique seul, non sécurisé) -------
CREATE USER 'adminrufus'@'%'
       IDENTIFIED WITH mysql_native_password BY 'gaxt78iy';
GRANT ALL PRIVILEGES ON *.* TO 'adminrufus'@'%' WITH GRANT OPTION;

CREATE USER 'adminrufusSSL'@'%'
       IDENTIFIED WITH mysql_native_password BY 'gaxt78iy' REQUIRE SSL;
GRANT ALL PRIVILEGES ON *.* TO 'adminrufusSSL'@'%' WITH GRANT OPTION;

FLUSH PRIVILEGES;

-- --- 3. Contrôle (facultatif) -----------------------------------------------
-- Doit lister exactement adminrufus@'%' et adminrufusSSL@'%', chacun avec
-- User_attributes = NULL (donc ni additional_password, ni securepar).
SELECT User, Host,
       User_attributes AS attributs,
       password_last_changed
  FROM mysql.user
 WHERE User LIKE 'adminrufus%'
 ORDER BY User, Host;
