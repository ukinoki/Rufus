/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MACROS_H
#define MACROS_H

// Les versions de la base et des ressources
#define VERSION_BASE                                41                  // version de la base de données
#define VERSION_RESSOURCES                          34                  // version des fichiers de ressources

// Les bases
#define NOM_BASE_CCAM                               "CCAM"
#define NOM_BASE_CONSULTS                           "rufus"
#define NOM_BASE_COMPTA                             "ComptaMedicale"
#define NOM_BASE_OPHTA                              "Ophtalmologie"
#define NOM_BASE_IMAGES                             "Images"

// Les tables
#define NOM_TABLE_ARCHIVESBANQUE                    "ComptaMedicale.archivesbanques"
#define NOM_TABLE_BANQUES                           "ComptaMedicale.banques"
#define NOM_TABLE_COMPTES                           "ComptaMedicale.Comptes"
#define NOM_TABLE_DEPENSES                          "ComptaMedicale.Depenses"
#define NOM_TABLE_LIGNESCOMPTES                     "ComptaMedicale.lignescomptes"
#define NOM_TABLE_LIGNESPAIEMENTS                   "ComptaMedicale.lignespaiements"
#define NOM_TABLE_RECETTES                          "ComptaMedicale.lignesrecettes"
#define NOM_TABLE_REMISECHEQUES                     "ComptaMedicale.remisecheques"
#define NOM_TABLE_RUBRIQUES2035                     "ComptaMedicale.rubriques2035"
#define NOM_TABLE_TIERS                             "ComptaMedicale.tiers"
#define NOM_TABLE_TYPEPAIEMENTACTES                 "ComptaMedicale.typepaiementactes"
#define NOM_TABLE_VERROUCOMPTAACTES                 "ComptaMedicale.verrouscomptaactes"

#define NOM_TABLE_ECHANGEIMAGES                     "Images.EchangeImages"
#define NOM_TABLE_DOCSASUPPRIMER                    "Images.DocsASupprimer"

#define NOM_TABLE_BILANORTHO                        "Ophtalmologie.bilanortho"
#define NOM_TABLE_BIOMETRIES                        "Ophtalmologie.biometries"
#define NOM_TABLE_COMMENTAIRESLUNETTES              "Ophtalmologie.commentaireslunettes"
#define NOM_TABLE_DONNEES_OPHTA_PATIENTS            "Ophtalmologie.DonneesOphtaPatients"
#define NOM_TABLE_REFRACTION                        "Ophtalmologie.refractions"
#define NOM_TABLE_TONOMETRIE                        "Ophtalmologie.tonometries"

#define NOM_TABLE_ACTES                             "rufus.Actes"
#define NOM_TABLE_APPAREILSCONNECTESCENTRE          "rufus.appareilsconnectescentre"
#define NOM_TABLE_APPAREILSREFRACTION               "rufus.AppareilsRefraction"
#define NOM_TABLE_CORRESPONDANTS                    "rufus.correspondants"
#define NOM_TABLE_CCAM                              "rufus.ccam"
#define NOM_TABLE_COTATIONS                         "rufus.cotations"
#define NOM_TABLE_COURRIERS                         "rufus.courriers"
#define NOM_TABLE_DONNEESSOCIALESPATIENTS           "rufus.donneessocialespatients"
#define NOM_TABLE_FLAGS                             "rufus.flags"
#define NOM_TABLE_GESTIONFICHIERS                   "rufus.gestionfichiers"
#define NOM_TABLE_IMPRESSIONS                       "rufus.Impressions"
#define NOM_TABLE_JOINTURESDOCS                     "rufus.jointuresdocuments"
#define NOM_TABLE_JOINTURESLIEUX                    "rufus.jointuresLieux"
#define NOM_TABLE_LIEUXEXERCICE                     "rufus.LieuxExercice"
#define NOM_TABLE_LISTEAPPAREILS                    "rufus.listeappareils"
#define NOM_TABLE_LISTETIERS                        "rufus.listetiers"
#define NOM_TABLE_MESSAGES                          "rufus.Messagerie"
#define NOM_TABLE_MESSAGESJOINTURES                 "rufus.MessagerieJointures"
#define NOM_TABLE_METADOCUMENTS                     "rufus.Metadocuments"
#define NOM_TABLE_MOTIFSRDV                         "rufus.MotifsRDV"
#define NOM_TABLE_MOTSCLES                          "rufus.MotsCles"
#define NOM_TABLE_MOTSCLESJOINTURES                 "rufus.MotsClesJointures"
#define NOM_TABLE_PARAMSYSTEME                      "rufus.ParametresSysteme"
#define NOM_TABLE_PATIENTS                          "rufus.Patients"
#define NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS    "rufus.renseignementsmedicauxpatients"
#define NOM_TABLE_SALLEDATTENTE                     "rufus.salledattente"
#define NOM_TABLE_SPECIALITES                       "rufus.specialitesmedicales"
#define NOM_TABLE_UTILISATEURS                      "rufus.utilisateurs"
#define NOM_TABLE_USERSCONNECTES                    "rufus.UtilisateursConnectes"
#define NOM_TABLE_VILLES                            "rufus.Villes"

// Divers
#define PDF                                         "pdf"
#define JPG                                         "jpg"

#define BILANORTHOPTIQUE                            "BO"
#define COURRIER                                    "Courrier"
#define DOCUMENTRECU                                "DocRecu"
#define IMAGERIE                                    "Imagerie"
#define PRESCRIPTION                                "Prescription"
#define PRESCRIPTIONLUNETTES                        "PrescrLun"
#define VIDEO                                       "Video"

#define NOM_APPLI                                   "Rufus"

#define NOM_POSTEIMPORTDOCS                         "posteimportdocs"
#define NOM_ADMINISTRATEURDOCS                      "Admin"
#define NOM_MDPADMINISTRATEUR                       "bob"

// Constantes de statut salle d'attente  //+++++ATTENTION ces constantes servent dans des requêtes SQL - NE PAS METTRE DE CARACTERES SPECIAUX
#define ARRIVE                                      "Arrivé"
#define ENCOURS                                     "En cours"
#define ENATTENTENOUVELEXAMEN                       "En attente de nouvel examen par "
#define RETOURACCUEIL                               "Retour accueil"
#define ENCOURSEXAMEN                               "En cours de consultation avec "

// Les fichiers et répertoires divers
#define NOMDIR_LIBS2                                "/Applications";
#define NOMDIR_RESSOURCES                           "/Ressources"
#define NOMDIR_RUFUS                                "/Documents/Rufus"
#define NOMDIR_IMAGES                               "/Images"
#define NOMDIR_VIDEOS                               "/Videos"
#define NOMDIR_PROV                                 "/Prov"
#define NOMDIR_ECHECSTRANSFERTS                     "/EchecsTransferts"
#define NOMFIC_ENTETEORDO                           "/Documents/Rufus/Ressources/Entete_Ordonnance.txt"
#define NOMFIC_CORPSORDO                            "/Documents/Rufus/Ressources/Corps_Ordonnance.txt"
#define NOMFIC_ENTETEORDOALD                        "/Documents/Rufus/Ressources/Entete_OrdoALD.txt"
#define NOMFIC_CORPSORDOALD                         "/Documents/Rufus/Ressources/Corps_OrdoALD.txt"
#define NOMFIC_PIEDPAGE                             "/Documents/Rufus/Ressources/Pied_Ordonnance.txt"
#define NOMFIC_PIEDORDOLUNETTES                     "/Documents/Rufus/Ressources/Pied_Ordonnance_Lunettes.txt"
#define NOMFIC_PDF                                  "/Documents/Rufus/Ressources/pdf.pdf"
#define NOMFIC_TEST                                 "/Documents/Rufus/Ressources/test.txt"
#define NOMFIC_INI                                  "/Documents/Rufus/Rufus.ini"
#define SCRIPTBACKUPFILE    "/Documents/Rufus/RufusScriptBackup.sh"
#define SCRIPTPLISTFILE     "/Library/LaunchAgents/rufus.bup.plist"

// Variables mysql
#define MAX_ALLOWED_PACKET                          "16"

// le son d'alarme
#define NOM_ALARME                                  "://goutte.wav"

// la largeur de la formule de réfraction dans le champ ActeTexte
#define LARGEUR_FORMULE                             "310"

// le lien vers la CCAM
#define LIEN_CCAM                                   "https://www.ameli.fr/accueil-de-la-ccam/trouver-un-acte/consultation-par-chapitre.php?chap=a%3A0%3A%7B%7D&add=2#chapitre_2"

// la taille maximale des images
#define TAILLEMAXIIMAGES                            262144

#ifdef Q_OS_MACX
#define POLICEPARDEFAUT         "Comic Sans MS"
#define POINTPARDEFAUT          13
#define PATH_SPEC_MAC "/../../.."
#endif
#ifdef Q_OS_LINUX
#define POLICEPARDEFAUT         "Noto Sans MS"
#define POINTPARDEFAUT          9
#define LINUX "/../../.."
#endif
#ifdef Q_OS_WIN
#define POLICEPARDEFAUT         "Comic Sans MS"
#define POINTPARDEFAUT          13
#endif


#define OPHTALIBERAL            "ophlib"
#define OPHTAREMPLACANT         "ophrmp"
#define OPHTASALARIE            "ophsal"
#define OPHTANOCOMPTA           "ophnocompta"
#define OPHTAASSISTANT          "ophassistant"
#define ORTHOLIBERAL            "ortholib"
#define ORTHOREMPLACANT         "orthormp"
#define ORTHOSALARIE            "orthosal"
#define ORTHONOCOMPTA           "orthonocompta"
#define ORTHOASSISTANT          "orthoassistant"
#define AUTRESOIGNANTLIBERAL    "autresoignantlib"
#define AUTRESOIGNANTREMPLACANT "autresoignantrmp"
#define AUTRESOIGNANTSALARIE    "autresoignantsal"
#define AUTRESOIGNANTNOCOMPTA   "autrenocompta"
#define AUTRESOIGNANTASSISTANT  "autreassistant"
#define AUTREFONCTION           "a"
#define SECRETAIRE              "s"
#define SOCIETECOMPTABLE        "SEL"

#define HAUTEUR_SANS_ORDONNANCE_MINI                    191                 // Hauteur du formulaire Refraction sans la partie ordonnance sans detail sans observ
#define HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL             335                 // Hauteur du formulaire Refraction sans la partie ordonnance avec detail sans observ
#define HAUTEUR_AVEC_ORDONNANCE_SANS_DETAIL             450                 // Hauteur du formulaire Refraction avec partie ordonnance sans detail
#define HAUTEUR_AVEC_ORDONNANCE_AVEC_DETAIL             590                 // Hauteur du formulaire Refraction avec partie ordonnance avec detail
#define LARGEURLISTE   1300
#define LARGEURNORMALE 1260
#define HAUTEUR_MINI        280
#define HAUTEUR_NORMALE     970


#endif // MACROS_H
