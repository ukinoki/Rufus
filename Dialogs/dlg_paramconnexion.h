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

#include <QComboBox>
#include <QFrame>
#include <QRadioButton>
#include "upmessagebox.h"
#include "updialog.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "uppushbutton.h"
#include "icons.h"
#include "utils.h"

/*! Fiche de saisie des paramètres de connexion : elle ne sert qu'à reconstruire un Rufus.ini.

Elle ne teste rien et ne se connecte à rien — savoir si le fichier ouvre la base est l'affaire de
Connexion_A_La_Base, juste après (§ II.1). D'où l'absence de bouton « Tester ».

	* trois modes exclusifs : ce poste, réseau local, accès distant -> modeacces()
	* le cadre IP/clés SSL n'apparaît qu'en réseau, les clés qu'en distant (RegleAffichage)
	* hors monoposte, ImporterDonneesConnexion remplit les champs depuis le dossier exporté par le serveur
	* la fiche n'écrit rien : l'appelant lit la saisie par les accesseurs après exec()
*/

class dlg_paramconnexion : public UpDialog
{
    Q_OBJECT
public:
    explicit            dlg_paramconnexion(QWidget *parent = nullptr);
    Utils::ModeAcces    modeacces() const       { return wdg_localradio->isChecked()?   Utils::ReseauLocal
                                                       : wdg_distantradio->isChecked()? Utils::Distant
                                                                                      : Utils::Poste; }
    QString             ip() const              { return wdg_iplineedit->text(); }
    QString             dossierclesSSL() const  { return wdg_clesSSLlineedit->text(); }
    QString             port() const            { return wdg_portcombo->currentText(); }

private:
    /*! les widgets */
    QRadioButton    *wdg_posteradio, *wdg_localradio, *wdg_distantradio;
    QFrame          *wdg_ipframe;                   /*!< adresse du serveur et clés SSL, masqué en monoposte */
    UpLineEdit      *wdg_iplineedit, *wdg_clesSSLlineedit;
    UpLabel         *wdg_clesSSLlabel;
    QPushButton     *wdg_clesSSLbouton;
    UpPushButton    *wdg_importbouton;              /*!< reprend le dossier exporté par le serveur, masqué en monoposte */
    QComboBox       *wdg_portcombo;

    void            DossierClesSSL();                   /*!< choisit le dossier des clés SSL (accès distant) et le renseigne dans le champ */
    void            ImporterDonneesConnexion();         /*!< remplit les champs avec le dossier RufusConnexion exporté par le serveur */
    void            RegleAffichage(QRadioButton *butt); /*!< adapte l'affichage (visibilité des champs, message accès distant) au mode d'accès choisi */
    void            Verif();                            /*!< bouton « OK » : ferme la fiche si la saisie est complète */
    bool            VerifFiche();                       /*!< la saisie est-elle complète (de quoi écrire un Rufus.ini valide) ? */
};

#endif // DLG_PARAMCONNEXION_H
