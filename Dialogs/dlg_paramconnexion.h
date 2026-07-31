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

#include "upmessagebox.h"
#include "upradiobutton.h"
#include "updialog.h"
#include "icons.h"

/* Cette classe sert à paramétrer la connexion au serveur
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/

namespace Ui {
class dlg_paramconnexion;
}

/*! \brief Fiche de saisie des paramètres de connexion : elle ne sert qu'à reconstruire un Rufus.ini.
 *
 * Elle ne teste rien et ne se connecte à rien — savoir si le fichier ouvre la base est l'affaire de
 * Connexion_A_La_Base, juste après (§ II.1). D'où l'absence de bouton « Tester ».
 *
 * L'utilisateur y renseigne :
 *  - un identifiant et un mot de passe applicatifs Rufus (table utilisateurs) ;
 *  - le mode d'accès : ce poste, réseau local (IP du serveur) ou distant (SSL obligatoire) ;
 *  - le port MySQL et, en distant, le dossier des clés SSL.
 *
 * OK vérifie seulement que la saisie est complète ; c'est VerifParamConnexion qui écrit Rufus.ini.
 */
class dlg_paramconnexion : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_paramconnexion(QWidget *parent = Q_NULLPTR);
    ~dlg_paramconnexion();
    Ui::dlg_paramconnexion *ui;
private:
    void            DossierClesSSL();                   //!< choisit le dossier des clés SSL (accès distant) et le renseigne dans le champ
    void            RegleAffichage(QRadioButton *butt); //!< adapte l'affichage (visibilité des champs, message accès distant) au mode d'accès choisi
    void            Verif();                            //!< bouton « OK » : ferme la fiche si la saisie est complète
    bool            VerifFiche();                       //!< la saisie est-elle complète (de quoi écrire un Rufus.ini valide) ?
};

#endif // DLG_PARAMCONNEXION_H
