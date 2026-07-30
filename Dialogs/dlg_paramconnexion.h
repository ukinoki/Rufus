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

/*! \brief Fiche de SAISIE des paramètres de connexion : elle ne sert QU'À RECONSTRUIRE un Rufus.ini.
 *
 * Cette fiche ne teste RIEN et ne se connecte À RIEN : elle recueille de quoi écrire un Rufus.ini
 * exploitable, et c'est tout. Savoir si ce fichier permet réellement d'ouvrir la base est l'affaire
 * du démarrage normal (Procedures::Connexion_A_La_Base), qui s'exécute juste après — d'où l'absence
 * de bouton « Tester » et de tout contrôle MySQL ici (cf. « initialisation Rufus.txt » § II.1).
 *
 * L'utilisateur y renseigne :
 *  - un identifiant et un mot de passe APPLICATIFS Rufus (table utilisateurs) ;
 *  - le mode d'accès : « Sur ce poste » (monoposte / localhost), « Réseau local » (IP du serveur
 *    du cabinet) ou « Distant » (accès par internet, chiffrement SSL obligatoire) ;
 *  - le port MySQL (3306 / 3307) et, en mode distant, le dossier des clés SSL.
 *
 * « OK » (Verif) contrôle seulement que la saisie est COMPLÈTE (VerifFiche) puis ferme la fiche ;
 * c'est l'appelant (Procedures::VerifParamConnexion) qui écrit Rufus.ini. « Annuler » revient à la
 * fiche précédente.
 *
 * RecupererMotDePasseMySQL() n'appartient pas à la fiche : c'est un utilitaire STATIQUE de
 * récupération du mot de passe MySQL du cabinet, appelé au démarrage (cf. déclaration ci-dessous).
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
    //! titre/corps optionnels : permettent d'adapter le message d'introduction au CONTEXTE — échec
    //! d'authentification (défaut) OU récupération PROACTIVE alors que le générique fonctionne encore
    //! (cf. MySQLInstaller::proposerRecuperationAleatoire). Vides → le message « échec » par défaut.
    //! reinitialiserDemande (MONOPOSTE) : si non nul, ajoute un 4e bouton « mot de passe inconnu →
    //! réinitialiser le serveur » ; un clic met *reinitialiserDemande=true et renvoie false (on ne
    //! cherche pas à se connecter, on veut remplacer le serveur sans sauvegarde).
    static bool     RecupererMotDePasseMySQL(QWidget *parent, const QString &titre = QString(),
                                             const QString &corps = QString(),
                                             bool *reinitialiserDemande = nullptr);

private:
    void            DossierClesSSL();                   //!< choisit le dossier des clés SSL (accès distant) et le renseigne dans le champ
    void            RegleAffichage(QRadioButton *butt); //!< adapte l'affichage (visibilité des champs, message accès distant) au mode d'accès choisi
    void            Verif();                            //!< bouton « OK » : ferme la fiche si la saisie est complète
    bool            VerifFiche();                       //!< la saisie est-elle complète (de quoi écrire un Rufus.ini valide) ?
};

#endif // DLG_PARAMCONNEXION_H
