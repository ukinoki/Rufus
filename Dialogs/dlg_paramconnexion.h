/* (C) 2020 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_PARAMCONNEXION_H
#define DLG_PARAMCONNEXION_H

#include <QMessageBox>
#include <QNetworkInterface>
#include <QSqlDatabase>
#include <QtSql>
#include <QTimer>
#include "upmessagebox.h"
#include "upradiobutton.h"
#include "updialog.h"
#include "database.h"
#include "icons.h"

/* Cette classe sert à paramétrer la connexion au serveur
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/

namespace Ui {
class dlg_paramconnexion;
}

/*! \brief Fenêtre de paramétrage de la connexion au serveur MySQL.
 *
 * Cette fiche est présentée au démarrage quand Rufus ne parvient pas à se
 * connecter avec les paramètres enregistrés dans rufus.ini (premier lancement,
 * fichier ini absent ou serveur injoignable). Elle existe en deux variantes
 * (Rufus et RufusAdmin).
 *
 * L'utilisateur y renseigne :
 *  - un identifiant et un mot de passe APPLICATIFS Rufus (table utilisateurs) ;
 *  - le mode d'accès au serveur : « Sur ce poste » (monoposte / localhost),
 *    « Réseau local » (IP du serveur du cabinet) ou « Distant » (accès par
 *    internet, chiffrement SSL obligatoire) ;
 *  - le port MySQL (3306 / 3307) et, en mode distant, le dossier des clés SSL.
 *
 * Le bouton « Tester » (Test) vérifie les paramètres sans fermer la fiche ;
 * « OK » (Verif) les valide puis ferme la fiche si la connexion réussit. La
 * connexion MySQL elle-même passe TOUJOURS par le compte fixe adminrufus
 * (MySQLInstaller::motDePasseSQL) ; les identifiants saisis sont ensuite
 * confrontés à la table utilisateurs (DataBase::verifExistUser).
 *
 * À la première connexion monoposte réussie, la base est sécurisée à la volée
 * si elle est encore sur le mot de passe public (securiserBaseSiNecessaire).
 */
class dlg_paramconnexion : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_paramconnexion(QWidget *parent = Q_NULLPTR);
    ~dlg_paramconnexion();
    Ui::dlg_paramconnexion *ui;
    //! Récupération du mot de passe aléatoire (saisie ou clé USB). STATIQUE et PUBLIQUE : appelée
    //! aussi AU DÉMARRAGE (Connexion_A_La_Base), hors de toute instance du dialogue.
    static bool     RecupererMotDePasseMySQL(QWidget *parent);

private:
    QString         m_IPaveczero = "";              //!< adresse du serveur normalisée avec des zéros de remplissage (000.000.000.000)
    QString         m_adresseserveur = "";          //!< adresse effective utilisée pour la connexion (localhost ou IP)
    void            DossierClesSSL();               //!< choisit le dossier des clés SSL (accès distant) et le mémorise dans rufus.ini
    void            CalcIP(QString IP);             //!< déduit m_adresseserveur / m_IPaveczero du mode d'accès et de l'IP saisie
    void            MAJIP();                        //!< recalcule et réaffiche l'IP normalisée après saisie
    void            RegleAffichage(QRadioButton *butt); //!< adapte l'affichage (masque IP, messages) au mode d'accès choisi
    void            Test();                         //!< teste les paramètres sans fermer la fiche (bouton « Tester »)
    void            Verif();                        //!< valide les paramètres et ferme la fiche si la connexion réussit (bouton « OK »)
    bool            VerifFiche();                   //!< contrôle que les champs obligatoires sont renseignés
    bool            TestConnexion();                //!< tente la connexion MySQL et la validation du compte utilisateur
    QString         TenterConnexionAvecRecuperation(); //!< cascade complète (MySQLInstaller) : candidats, puis récupération du mdp en dernier ressort, puis nouvel essai
};

#endif // DLG_PARAMCONNEXION_H
