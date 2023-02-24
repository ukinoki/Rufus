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

#ifndef MACROS_H
#define MACROS_H

// Les versions de la base et des ressources
#define VERSION_BASE                          72  // version de la base de données
#define VERSION_RESSOURCES                    35  // version des fichiers de ressources

// Les bases
#define DB_CCAM                               "CCAM"
#define DB_CONSULTS                           "rufus"
#define DB_COMPTA                             "ComptaMedicale"
#define DB_OPHTA                              "Ophtalmologie"
#define DB_IMAGES                             "Images"

// Les tables
#define TBL_ARCHIVESBANQUE                    "ComptaMedicale.archivesbanques"
#define TBL_BANQUES                           "ComptaMedicale.banques"
#define TBL_COMPTES                           "ComptaMedicale.Comptes"
#define TBL_DEPENSES                          "ComptaMedicale.Depenses"
#define TBL_FACTURES                          "ComptaMedicale.Factures"
#define TBL_LIGNESCOMPTES                     "ComptaMedicale.lignescomptes"
#define TBL_LIGNESPAIEMENTS                   "ComptaMedicale.lignespaiements"
#define TBL_RECETTES                          "ComptaMedicale.lignesrecettes"
#define TBL_RECETTESSPECIALES                 "ComptaMedicale.autresrecettes"
#define TBL_REMISECHEQUES                     "ComptaMedicale.remisecheques"
#define TBL_RUBRIQUES2035                     "ComptaMedicale.rubriques2035"
#define TBL_TIERS                             "ComptaMedicale.tiers"
#define TBL_TYPEPAIEMENTACTES                 "ComptaMedicale.typepaiementactes"
#define TBL_VERROUCOMPTAACTES                 "ComptaMedicale.verrouscomptaactes"

#define TBL_ECHANGEIMAGES                     "Images.EchangeImages"
#define TBL_DOCSASUPPRIMER                    "Images.DocsASupprimer"
#define TBL_FACTURESASUPPRIMER                "Images.FacturesASupprimer"

#define TBL_BILANORTHO                        "Ophtalmologie.bilanortho"                         /*! pas fait */
#define TBL_BIOMETRIES                        "Ophtalmologie.biometries"
#define TBL_COMMENTAIRESLUNETTES              "Ophtalmologie.commentaireslunettes"
#define TBL_DONNEES_OPHTA_PATIENTS            "Ophtalmologie.DonneesOphtaPatients"
#define TBL_IOLS                              "Ophtalmologie.IOLs"
#define TBL_LIGNESPRGOPERATOIRES              "Ophtalmologie.ProgrammesOperatoires"
#define TBL_SESSIONSOPERATOIRES               "Ophtalmologie.SessionsOperatoires"
#define TBL_TYPESINTERVENTIONS                "Ophtalmologie.TypesInterventions"
#define TBL_REFRACTIONS                       "Ophtalmologie.refractions"
#define TBL_TONOMETRIE                        "Ophtalmologie.tonometries"
#define TBL_PACHYMETRIE                       "Ophtalmologie.pachymetries"

#define TBL_ACTES                             "rufus.Actes"
#define TBL_APPAREILSCONNECTESCENTRE          "rufus.appareilsconnectescentre"
#define TBL_APPAREILSREFRACTION               "rufus.AppareilsRefraction"
#define TBL_CCAM                              "rufus.ccam"
#define TBL_COMMERCIALS                       "rufus.Commercials"
#define TBL_CORRESPONDANTS                    "rufus.correspondants"
#define TBL_COTATIONS                         "rufus.cotations"
#define TBL_DOSSIERSIMPRESSIONS               "rufus.Metadocuments"
#define TBL_DONNEESSOCIALESPATIENTS           "rufus.donneessocialespatients"
#define TBL_FLAGS                             "rufus.flags"
#define TBL_DOCSEXTERNES                      "rufus.Impressions"
#define TBL_IMPRESSIONS                       "rufus.courriers"
#define TBL_JOINTURESIMPRESSIONS              "rufus.jointuresdocuments"
#define TBL_JOINTURESLIEUX                    "rufus.jointuresLieux"
#define TBL_LIEUXEXERCICE                     "rufus.LieuxExercice"
#define TBL_LISTEAPPAREILS                    "rufus.listeappareils"
#define TBL_LISTETIERS                        "rufus.listetiers"
#define TBL_MANUFACTURERS                     "rufus.Manufacturers"
#define TBL_MESSAGES                          "rufus.Messagerie"
#define TBL_MESSAGESJOINTURES                 "rufus.MessagerieJointures"
#define TBL_MOTIFSRDV                         "rufus.MotifsRDV"
#define TBL_MOTSCLES                          "rufus.MotsCles"                                  /*! pas fait */
#define TBL_MOTSCLESJOINTURES                 "rufus.MotsClesJointures"                         /*! pas fait */
#define TBL_PARAMSYSTEME                      "rufus.ParametresSysteme"
#define TBL_PATIENTS                          "rufus.Patients"
#define TBL_RENSEIGNEMENTSMEDICAUXPATIENTS    "rufus.renseignementsmedicauxpatients"
#define TBL_SALLEDATTENTE                     "rufus.salledattente"
#define TBL_SESSIONS                          "rufus.Sessions"
#define TBL_SPECIALITES                       "rufus.specialitesmedicales"
#define TBL_UTILISATEURS                      "rufus.utilisateurs"
#define TBL_USERSCONNECTES                    "rufus.UtilisateursConnectes"
#define TBL_VILLES                            "rufus.Villes"

// Les champs
#define CP_ISALLLOADED                                  "isallloaded"
#define CP_ISMEDICALlOADED                              "ismedicalloaded"
#define CP_ISSOCIALlOADED                               "issocialloaded"
#define CP_ISGENERALISTE                                "isgeneraliste"
#define CP_ISMEDECIN                                    "ismedecin"
#define CP_METIER                                       "metier"

                            //! Table Rufus.Patients
#define CP_IDPAT_PATIENTS                               "idPat"
#define CP_NOM_PATIENTS                                 "PatNom"
#define CP_PRENOM_PATIENTS                              "PatPrenom"
#define CP_DDN_PATIENTS                                 "PatDDN"
#define CP_SEXE_PATIENTS                                "Sexe"
#define CP_DATECREATION_PATIENTS                        "PatCreeLe"
#define CP_IDCREATEUR_PATIENTS                          "PatCreePar"
#define CP_IDLIEU_PATIENTS                              "NumCentre"
#define CP_COMMENTAIRE_PATIENTS                         "Commentaire"

                            //! Table Rufus.donneessocialespatients
#define CP_IDPAT_DSP                                    "idPat"
#define CP_ADRESSE1_DSP                                 "PatAdresse1"
#define CP_ADRESSE2_DSP                                 "PatAdresse2"
#define CP_ADRESSE3_DSP                                 "PatAdresse3"
#define CP_CODEPOSTAL_DSP                               "PatCodepostal"
#define CP_VILLE_DSP                                    "PatVille"
#define CP_TELEPHONE_DSP                                "PatTelephone"
#define CP_PORTABLE_DSP                                 "PatPortable"
#define CP_MAIL_DSP                                     "PatMail"
#define CP_NNI_DSP                                      "PatNNI"
#define CP_ALD_DSP                                      "PatALD"
#define CP_CMU_DSP                                      "PatCMU"
#define CP_PROFESSION_DSP                               "PatProfession"

                            //! Table Rufus.renseignementsmedicauxpatients
#define CP_IDPAT_RMP                                    "idPat"
#define CP_ATCDTSOPH_RMP                                "RMPAtcdtsOphs"
#define CP_TRAITMTOPH_RMP                               "RMPTtOphs"
#define CP_IMPORTANT_RMP                                "Important"
#define CP_RESUME_RMP                                   "Resume"
#define CP_ATCDTSPERSOS_RMP                             "RMPAtcdtsPersos"
#define CP_ATCDTSFAMLXS_RMP                             "RMPAtcdtsFamiliaux"
#define CP_TRAITMTGEN_RMP                               "RMPTtGeneral"
#define CP_AUTRESTOXIQUES_RMP                           "Autrestoxiques"
#define CP_GENCORRESPONDANT_RMP                         "Gencorresp"
#define CP_IDMG_RMP                                     "idCorMedMG"
#define CP_IDSPE1_RMP                                   "idCorMedSpe1"
#define CP_IDSPE2_RMP                                   "idCorMedSpe2"
#define CP_IDSPE3_RMP                                   "idCorMedSpe3"
#define CP_IDCORNONMED_RMP                              "idCorNonMed"
#define CP_TABAC_RMP                                    "Tabac"

                            //! Table Rufus.Actes
#define CP_ID_ACTES                                     "idActe"
#define CP_IDPAT_ACTES                                  "idPat"
#define CP_IDUSER_ACTES                                 "idUser"
#define CP_IDSESSION_ACTES                              "idSession"    //!> pas utilisé
#define CP_MOTIF_ACTES                                  "ActeMotif"
#define CP_TEXTE_ACTES                                  "ActeTexte"
#define CP_CONCLUSION_ACTES                             "ActeConclusion"
#define CP_COURRIERAFAIRE_ACTES                         "ActeCourrierafaire"
#define CP_COTATION_ACTES                               "ActeCotation"
#define CP_MONTANT_ACTES                                "ActeMontant"
#define CP_MONNAIE_ACTES                                "ActeMonnaie"
#define CP_DATE_ACTES                                   "ActeDate"
#define CP_HEURE_ACTES                                  "ActeHeure"
#define CP_IDUSERCREATEUR_ACTES                         "CreePar"
#define CP_IDUSERCOMPTABLE_ACTES                        "UserComptable"
#define CP_IDUSERPARENT_ACTES                           "UserParent"
#define CP_SUPERVISEURREMPLACANT_ACTES                  "SuperViseurRemplacant"
#define CP_NUMCENTRE_ACTES                              "NumCentre"
#define CP_IDLIEU_ACTES                                 "idLieu"
#define CP_IMPORTANT_ACTES                              "PointImportant"
#define CP_TEXTIMPORTANT_ACTES                          "CommentPointImportant"
#define CP_NOCOTATION_ACTES                             "SansCotation"

                            //! Table Rufus.appareilsconnectescentre
#define CP_IDAPPAREIL_APPAREILS                         "idAppareil"
#define CP_IDLIEU_APPAREILS                             "idLieu"

                            //! Table Rufus.Courriers
#define CP_ID_IMPRESSIONS                               "idDocument"
#define CP_TEXTE_IMPRESSIONS                            "TextDocument"
#define CP_RESUME_IMPRESSIONS                           "ResumeDocument"
#define CP_CONCLUSION_IMPRESSIONS                       "ConclusionDocument"
#define CP_IDUSER_IMPRESSIONS                           "idUser"
#define CP_DOCPUBLIC_IMPRESSIONS                        "DocPublic"
#define CP_PRESCRIPTION_IMPRESSIONS                     "Prescription"
#define CP_EDITABLE_IMPRESSIONS                         "Editable"
#define CP_MEDICAL_IMPRESSIONS                          "Medical"

                            //! Table Rufus.flags
#define CP_MAJFLAGSALDAT_FLAGS                          "MAJflagSalDat"
#define CP_MAJFLAGCORRESPONDANTS_FLAGS                  "MAJflagMG"
#define CP_MAJFLAGMESSAGES_FLAGS                        "MAJflagMessages"
#define CP_MAJFLAGUSERDISTANT_FLAGS                     "flagUserDistant"
#define CP_MAJFLAGIMPRESSIONS_FLAGS                     "MAJflagImpressions"

                            //! Table Rufus.ccam
#define CP_ID_CCAM                                      "idccam"
#define CP_CODECCAM_CCAM                                "codeccam"
#define CP_NOM_CCAM                                     "nom"
#define CP_MONTANTOPTAM_CCAM                            "optam"
#define CP_MONTANTNONOPTAM_CCAM                        "nonoptam"

                            //! Table Rufus.cotations
#define CP_ID_COTATIONS                                 "idcotation"
#define CP_TYPEACTE_COTATIONS                           "Typeacte"
#define CP_MONTANTOPTAM_COTATIONS                       "MontantOPTAM"
#define CP_MONTANTNONOPTAM_COTATIONS                    "MontantNonOPTAM"
#define CP_MONTANTPRATIQUE_COTATIONS                    "MontantPratique"
#define CP_CODECCAM_COTATIONS                           "CCAM"
#define CP_IDUSER_COTATIONS                             "idUser"
#define CP_FREQUENCE_COTATIONS                          "Frequence"
#define CP_TIP_COTATIONS                                "Tip"

                            //! Table Rufus.LieuxExercice
#define CP_ID_SITE                                      "idLieu"
#define CP_NOM_SITE                                     "NomLieu"
#define CP_ADRESSE1_SITE                                "LieuAdresse1"
#define CP_ADRESSE2_SITE                                "LieuAdresse2"
#define CP_ADRESSE3_SITE                                "LieuAdresse3"
#define CP_CODEPOSTAL_SITE                              "LieuCodePostal"
#define CP_VILLE_SITE                                   "LieuVille"
#define CP_TELEPHONE_SITE                               "LieuTelephone"
#define CP_FAX_SITE                                     "LieuFax"
#define CP_COULEUR_SITE                                 "LieuCouleur"

                            //! Table Rufus.jointuresLieux
#define CP_IDUSER_JOINTSITE                             "idUser"
#define CP_IDLIEU_JOINTSITE                             "idLieu"

                            //! Table Rufus.Manufacturers
#define CP_ID_MANUFACTURER                              "idManufacturer"
#define CP_NOM_MANUFACTURER                             "ManName"
#define CP_ADRESSE1_MANUFACTURER                        "ManAdresse1"
#define CP_ADRESSE2_MANUFACTURER                        "ManAdresse2"
#define CP_ADRESSE3_MANUFACTURER                        "ManAdresse3"
#define CP_CODEPOSTAL_MANUFACTURER                      "ManCodepostal"
#define CP_VILLE_MANUFACTURER                           "ManVille"
#define CP_TELEPHONE_MANUFACTURER                       "ManTelephone"
#define CP_FAX_MANUFACTURER                             "ManFax"
#define CP_PORTABLE_MANUFACTURER                        "ManPortable"
#define CP_WEBSITE_MANUFACTURER                         "ManWebsite"
#define CP_MAIL_MANUFACTURER                            "ManMail"
#define CP_INACTIF_MANUFACTURER                         "ManInactif"
#define CP_DISTRIBUEPAR_MANUFACTURER                    "DistribuePar"
#define CP_IDRUFUS_MANUFACTURER                         "idRufus"

                            //! Table Rufus.Commercials
#define CP_ID_COM                                       "idCommercial"
#define CP_NOM_COM                                      "Nom"
#define CP_PRENOM_COM                                   "Prenom"
#define CP_STATUT_COM                                   "Statut"
#define CP_MAIL_COM                                     "Mail"
#define CP_TELEPHONE_COM                                "Telephone"
#define CP_IDMANUFACTURER_COM                           "idManufacturer"

                            //! Table Rufus.Metadocuments
#define CP_ID_DOSSIERIMPRESSIONS                        "idMetaDocument"
#define CP_RESUME_DOSSIERIMPRESSIONS                    "ResumeMetaDocument"
#define CP_IDUSER_DOSSIERIMPRESSIONS                    "idUser"
#define CP_PUBLIC_DOSSIERIMPRESSIONS                    "Public"

                            //! Table Rufus.jointuresdocuments
#define CP_ID_JOINTURESIMPRESSIONS                      "idJointure"
#define CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS          "idMetaDocument"
#define CP_IDDOCUMENT_JOINTURESIMPRESSIONS              "idDocument"

                            //! Table Rufus.Impressions
#define CP_ID_DOCSEXTERNES                              "idImpression"
#define CP_IDUSER_DOCSEXTERNES                          "idUser"
#define CP_IDPAT_DOCSEXTERNES                           "idPat"
#define CP_TYPEDOC_DOCSEXTERNES                         "TypeDoc"
#define CP_SOUSTYPEDOC_DOCSEXTERNES                     "SousTypeDoc"
#define CP_TITRE_DOCSEXTERNES                           "Titre"
#define CP_TEXTENTETE_DOCSEXTERNES                      "TextEntete"
#define CP_TEXTCORPS_DOCSEXTERNES                       "TextCorps"
#define CP_TEXTORIGINE_DOCSEXTERNES                     "TextOrigine"
#define CP_TEXTPIED_DOCSEXTERNES                        "TextPied"
#define CP_DATE_DOCSEXTERNES                            "DateImpression"
#define CP_PDF_DOCSEXTERNES                             "pdf"
#define CP_COMPRESSION_DOCSEXTERNES                     "Compression"
#define CP_JPG_DOCSEXTERNES                             "jpg"
#define CP_AUTRE_DOCSEXTERNES                           "autre"
#define CP_FORMATAUTRE_DOCSEXTERNES                     "formatautre"
#define CP_LIENFICHIER_DOCSEXTERNES                     "lienversfichier"
#define CP_LIENFICHIERDISTANT_DOCSEXTERNES              "LienFichierDistant"
#define CP_IDREFRACTION_DOCSEXTERNES                    "idRefraction"
#define CP_ALD_DOCSEXTERNES                             "ALD"
#define CP_IDEMETTEUR_DOCSEXTERNES                      "UserEmetteur"
#define CP_CONCLUSION_DOCSEXTERNES                      "Conclusion"
#define CP_EMISORRECU_DOCSEXTERNES                      "EmisRecu"
#define CP_FORMATDOC_DOCSEXTERNES                       "FormatDoc"
#define CP_IDLIEU_DOCSEXTERNES                          "idLieu"
#define CP_IMPORTANCE_DOCSEXTERNES                      "Importance"

                            //! Table rufus.listeappareils
#define CP_ID_APPAREIL                                  "idAppareil"
#define CP_TITREEXAMEN_APPAREIL                         "TitreExamen"
#define CP_NOMAPPAREIL_APPAREIL                         "NomAppareil"
#define CP_FORMAT_APPAREIL                              "Format"

                            //! Table rufus.MotsCles
#define CP_ID_MOTCLE                                    "idMotCle"
#define CP_TEXT_MOTCLE                                  "MotCle"

                            //! Table rufus.MotsClesJointures
#define CP_ID_JOINTURESMOTSCLES                         "idJointure"
#define CP_IDPATIENT_JOINTURESMOTSCLES                  "idPat"
#define CP_IDMOTCLE_JOINTURESMOTSCLES                   "idMotCle"

                            //! Table Rufus.ParametresSysteme
#define CP_MDPADMIN_PARAMSYSTEME                        "MDPAdmin"
#define CP_NUMCENTRE_PARAMSYSTEME                       "NumCentre"
#define CP_IDLIEUPARDEFAUT_PARAMSYSTEME                 "idLieuParDefaut"
#define CP_DOCSCOMPRIMES_PARAMSYSTEME                   "DocsComprimes"
#define CP_VERSIONBASE_PARAMSYSTEME                     "VersionBase"
#define CP_SANSCOMPTA_PARAMSYSTEME                      "SansCompta"
#define CP_ADRESSELOCALSERVEUR_PARAMSYSTEME             "AdresseServeurLocal"
#define CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME           "AdresseServeurDistant"
#define CP_DIRIMAGERIE_PARAMSYSTEME                     "DirImagerie"
#define CP_LUNDIBKUP_PARAMSYSTEME                       "LundiBkup"
#define CP_MARDIBKUP_PARAMSYSTEME                       "MardiBkup"
#define CP_MERCREDIBKUP_PARAMSYSTEME                    "MercrediBkup"
#define CP_JEUDIBKUP_PARAMSYSTEME                       "JeudiBkup"
#define CP_VENDREDIBKUP_PARAMSYSTEME                    "VendrediBkup"
#define CP_SAMEDIBKUP_PARAMSYSTEME                      "SamediBkup"
#define CP_DIMANCHEBKUP_PARAMSYSTEME                    "DimancheBkup"
#define CP_HEUREBKUP_PARAMSYSTEME                       "HeureBkup"
#define CP_DIRBKUP_PARAMSYSTEME                         "DirBkup"

                            //! Table Rufus.salledattente
#define CP_IDPAT_SALDAT                                 "idPat"
#define CP_IDUSER_SALDAT                                "idUser"
#define CP_STATUT_SALDAT                                "Statut"
#define CP_HEURESTATUT_SALDAT                           "HeureStatut"
#define CP_HEURERDV_SALDAT                              "HeureRDV"
#define CP_HEUREARRIVEE_SALDAT                          "HeureArrivee"
#define CP_MOTIF_SALDAT                                 "Motif"
#define CP_MESSAGE_SALDAT                               "Message"
#define CP_IDACTEAPAYER_SALDAT                          "idActeAPayer"
#define CP_POSTEEXAMEN_SALDAT                           "PosteExamen"
#define CP_IDUSERENCOURSEXAM_SALDAT                     "idUserEnCoursExam"
#define CP_IDSALDAT_SALDAT                              "idSalDat"

                            //! Table Rufus.Sessions
#define CP_ID_SESSIONS                                  "idSession"
#define CP_IDUSER_SESSIONS                              "idUser"
#define CP_IDSUPERVISEUR_SESSIONS                       "idSuperviseur"
#define CP_IDPARENT_SESSIONS                            "idParent"
#define CP_IDCOMPTABLE_SESSIONS                         "idComptable"
#define CP_IDLIEU_SESSIONS                              "idLieu"
#define CP_DATEDEBUT_SESSIONS                           "DebutSession"
#define CP_DATEFIN_SESSIONS                             "FinSession"

                            //! Table Rufus.Utilisateurs connectes
#define CP_IDUSER_USRCONNECT                            "idUser"
#define CP_NOMPOSTE_USRCONNECT                          "NomPosteConnecte"
#define CP_MACADRESS_USRCONNECT                         "MACAdressePosteConnecte"
#define CP_IPADRESS_USRCONNECT                          "IPAdress"
#define CP_DISTANT_USRCONNECT                           "AccesDistant"
#define CP_IDUSERSUPERVISEUR_USRCONNECT                 "UserSuperviseur"
#define CP_IDUSERCOMPTABLE_USRCONNECT                   "UserComptable"
#define CP_IDUSERPARENT_USRCONNECT                      "UserParent"
#define CP_IDLIEU_USRCONNECT                            "idLieu"
#define CP_HEUREDERNIERECONNECTION_USRCONNECT           "HeureDerniereConnexion"
#define CP_IDPATENCOURS_USRCONNECT                      "idPat"

                            //! Table Rufus.utilisateurs
#define CP_ID_USR                                       "idUser"
#define CP_NOM_USR                                      "UserNom"
#define CP_PRENOM_USR                                   "UserPrenom"
#define CP_FONCTION_USR                                 "UserFonction"
#define CP_SPECIALITE_USR                               "UserSpecialite"
#define CP_IDSPECIALITE_USR                             "UserNoSpecialite"
#define CP_TITRE_USR                                    "UserTitre"
#define CP_DROITS_USR                                   "UserDroits"
#define CP_LOGIN_USR                                    "UserLogin"
#define CP_MDP_USR                                      "UserMDP"
#define CP_ADRESSE1_USR                                 "UserAdresse1"    //!> pas utilisé
#define CP_ADRESSE2_USR                                 "UserAdresse2"    //!> pas utilisé
#define CP_ADRESSE3_USR                                 "UserAdresse3"    //!> pas utilisé
#define CP_CP_USR                                       "UserCodePostal"  //!> pas utilisé
#define CP_VILLE_USR                                    "UserVille"       //!> pas utilisé
#define CP_TEL_USR                                      "UserTelephone"   //!> pas utilisé
#define CP_POSTE_USR                                    "UserPoste"
#define CP_PORTABLE_USR                                 "UserPortable"
#define CP_FAX_USR                                      "UserFax"         //!> pas utilisé
#define CP_WEBSITE_USR                                  "UserWeb"
#define CP_MAIL_USR                                     "UserMail"
#define CP_MEMO_USR                                     "UserMemo"        //!> pas utilisé
#define CP_IDCOMPTEPARDEFAUT_USR                        "IdCompteParDefaut"
#define CP_NUMPS_USR                                    "UserNumPS"
#define CP_NUMCO_USR                                    "UserNumCO"
#define CP_SOIGNANTSTATUS_USR                           "Soignant"
#define CP_ISMEDECIN_USR                                "Medecin"
#define CP_RESPONSABLEACTES_USR                         "ResponsableActes"
#define CP_ENREGHONORAIRES_USR                          "UserEnregHonoraires"
#define CP_IDEMPLOYEUR_USR                              "UserEmployeur"
#define CP_IDCOMPTEENCAISSEMENTHONORAIRES_USR           "idCompteEncaissHonoraires"
#define CP_ISDESACTIVE_USR                              "UserDesactive"
#define CP_POLICEECRAN_USR                              "UserPoliceEcran"
#define CP_POLICEATTRIBUT_USR                           "UserPoliceAttribut"
#define CP_ISAGA_USR                                    "UserAGA"
#define CP_SECTEUR_USR                                  "UserSecteur"
#define CP_ISOPTAM_USR                                  "OPTAM"
#define CP_CCAM_USR                                     "UserCCAM"
#define CP_SIGNATURE_USR                                "Signature"   //!> pas utilisé
#define CP_ISORDOSIGNE_USR                              "OrdoSigne"   //!> pas utilisé
#define CP_DATEDERNIERECONNEXION_USR                    "DateDerniereConnexion"
#define CP_DATECREATIONMDP_USR                          "DateCreationMDP"
#define CP_USERBARCODE_USR                              "UserBarCode"
#define CP_AFFICHEDOCSPUBLICS_USR                       "AfficheDocsPublics"
#define CP_AFFICHECOMMENTSPUBLICS_USR                   "AfficheCommentsPublics"

                            //! Table Rufus.correspondants
#define CP_ID_CORRESP                                   "idCor"
#define CP_NOM_CORRESP                                  "CorNom"
#define CP_PRENOM_CORRESP                               "CorPreNom"
#define CP_SEXE_CORRESP                                 "CorSexe"
#define CP_ADRESSE1_CORRESP                             "CorAdresse1"
#define CP_ADRESSE2_CORRESP                             "CorAdresse2"
#define CP_ADRESSE3_CORRESP                             "CorAdresse3"
#define CP_CODEPOSTAL_CORRESP                           "CorCodepostal"
#define CP_VILLE_CORRESP                                "CorVille"
#define CP_TELEPHONE_CORRESP                            "CorTelephone"
#define CP_PORTABLE_CORRESP                             "CorPortable"
#define CP_FAX_CORRESP                                  "CorFax"
#define CP_MAIL_CORRESP                                 "CorMail"
#define CP_ISMEDECIN_CORRESP                            "CorMedecin"
#define CP_SPECIALITE_CORRESP                           "CorSpecialite"
#define CP_AUTREPROFESSION_CORRESP                      "CorAutreProfession"

                            //! Table  Rufus.Messagerie
#define CP_ID_MSG                                       "idMessage"
#define CP_IDEMETTEUR_MSG                               "idEmetteur"
#define CP_TEXT_MSG                                     "TexteMessage"
#define CP_IDPATIENT_MSG                                "idPatient"
#define CP_TACHE_MSG                                    "Tache"
#define CP_DATELIMITE_MSG                               "DateLimite"
#define CP_DATECREATION_MSG                             "CreeLe"
#define CP_URGENT_MSG                                   "Urge"
#define CP_ENREPONSEA_MSG                               "ReponseA"
#define CP_ASUPPRIMER_MSG                               "ASupprimer"

                            //! Table  Rufus.MessagerieJointures
#define CP_ID_JOINTURESMSG                              "idJointure"
#define CP_IDMSG_JOINTURESMSG                           "idMessage"
#define CP_IDDESTINATAIRE_JOINTURESMSG                  "idDestinataire"
#define CP_LU_JOINTURESMSG                              "Lu"
#define CP_FAIT_JOINTURESMSG                            "Fait"

                            //! Table Images.EchangeImages
#define CP_ID_ECHGIMAGES                                "idImpression"
#define CP_PDF_ECHGIMAGES                               "pdf"
#define CP_JPG_ECHGIMAGES                               "jpg"
#define CP_COMPRESSION_ECHGIMAGES                       "Compression"
#define CP_FACTURE_ECHGIMAGES                           "Facture"

                            //! Table Images.DocsASupprimer
#define CP_FILEPATH_DOCSASUPPR                          "FilePath"

                            //! Table Images.FacturesASupprimer
#define CP_ID_FACTASUPPR                                "idFacture"
#define CP_ECHEANCIER_FACTASUPPR                        "Echeancier"
#define CP_LIENFICHIER_FACTASUPPR                       "LienFichier"

                            //! Table ComptaMedicale.banques
#define CP_ID_BANQUES                                   "idBanque"
#define CP_NOMABREGE_BANQUES                            "idBanqueAbrege"
#define CP_NOMBANQUE_BANQUES                            "NomBanque"
#define CP_CODE_BANQUES                                 "CodeBanque"


                            //! Table ComptaMedicale.Comptes
#define CP_ID_COMPTES                                   "idCompte"
#define CP_IDBANQUE_COMPTES                             "idBanque"
#define CP_IDUSER_COMPTES                               "idUser"
#define CP_IBAN_COMPTES                                 "IBAN"
#define CP_INTITULE_COMPTES                             "IntituleCompte"
#define CP_NOMABREGE_COMPTES                            "NomCompteAbrege"
#define CP_SOLDE_COMPTES                                "SoldeSurDernierReleve"
#define CP_PARTAGE_COMPTES                              "Partage"
#define CP_DESACTIVE_COMPTES                            "Desactive"

                            //! Table ComptaMedicale.Depenses
#define CP_ID_DEPENSES                                 "idDep"
#define CP_IDUSER_DEPENSES                             "idUser"
#define CP_DATE_DEPENSES                               "DateDep"
#define CP_REFFISCALE_DEPENSES                         "RefFiscale"
#define CP_OBJET_DEPENSES                              "Objet"
#define CP_MONTANT_DEPENSES                            "Montant"
#define CP_FAMILLEFISCALE_DEPENSES                     "FamFiscale"
#define CP_IDOPERATION_DEPENSES                        "Nooperation"
#define CP_MONNAIE_DEPENSES                            "Monnaie"
#define CP_IDRECETTE_DEPENSES                          "idRec"
#define CP_MODEPAIEMENT_DEPENSES                       "ModePaiement"
#define CP_COMPTE_DEPENSES                             "Compte"
#define CP_NUMCHEQUE_DEPENSES                          "Nocheque"
#define CP_IDFACTURE_DEPENSES                          "idFacture"

                            //! Table ComptaMedicale.Factures
#define CP_ID_FACTURES                                 "idFacture"
#define CP_DATEFACTURE_FACTURES                        "DateFacture"
#define CP_LIENFICHIER_FACTURES                        "LienFichier"
#define CP_INTITULE_FACTURES                           "Intitule"
#define CP_ECHEANCIER_FACTURES                         "Echeancier"
#define CP_IDDEPENSE_FACTURES                          "idDepense"
#define CP_PDF_FACTURES                                "pdf"
#define CP_JPG_FACTURES                                "jpg"

                            //! Table ComptaMedicale.lignescomptes
#define CP_ID_LIGNCOMPTES                              "idLigne"
#define CP_IDCOMPTE_LIGNCOMPTES                        "idCompte"
#define CP_IDDEP_LIGNCOMPTES                           "idDep"
#define CP_IDREC_LIGNCOMPTES                           "idRec"
#define CP_IDRECSPEC_LIGNCOMPTES                       "idRecSpec"
#define CP_IDREMCHEQ_LIGNCOMPTES                       "idRemCheq"
#define CP_DATE_LIGNCOMPTES                            "LigneDate"
#define CP_LIBELLE_LIGNCOMPTES                         "LigneLibelle"
#define CP_MONTANT_LIGNCOMPTES                         "LigneMontant"
#define CP_DEBITCREDIT_LIGNCOMPTES                     "LigneDebitCredit"
#define CP_TYPEOPERATION_LIGNCOMPTES                   "LigneTypeoperation"
#define CP_CONSOLIDE_LIGNCOMPTES                       "LigneConsolide"

                            //! Table ComptaMedicale.archivesbanques
#define CP_ID_ARCHIVESCPT                              "idLigne"
#define CP_IDCOMPTE_ARCHIVESCPT                        "idCompte"
#define CP_IDDEP_ARCHIVESCPT                           "idDep"
#define CP_IDREC_ARCHIVESCPT                           "idRec"
#define CP_IDRECSPEC_ARCHIVESCPT                       "idRecSpec"
#define CP_IDREMCHEQ_ARCHIVESCPT                       "idRemCheq"
#define CP_DATE_ARCHIVESCPT                            "LigneDate"
#define CP_LIBELLE_ARCHIVESCPT                         "LigneLibelle"
#define CP_MONTANT_ARCHIVESCPT                         "LigneMontant"
#define CP_DEBITCREDIT_ARCHIVESCPT                     "LigneDebitCredit"
#define CP_TYPEOPERATION_ARCHIVESCPT                   "LigneTypeoperation"
#define CP_DATECONSOLIDE_ARCHIVESCPT                   "LigneDateconsolidation"
#define CP_IDARCHIVE_ARCHIVESCPT                       "idArchive"

                            //! Table ComptaMedicale.lignespaiements
#define CP_IDACTE_LIGNEPAIEMENT                         "idActe"
#define CP_IDRECETTE_LIGNEPAIEMENT                      "idRecette"
#define CP_PAYE_LIGNEPAIEMENT                           "Paye"

                            //! Table ComptaMedicale.lignesrecettes
#define CP_ID_LIGNRECETTES                             "idRecette"
#define CP_IDUSER_LIGNRECETTES                         "idUser"
#define CP_DATE_LIGNRECETTES                           "DatePaiement"
#define CP_DATEENREGISTREMENT_LIGNRECETTES             "DateEnregistrement"
#define CP_MONTANT_LIGNRECETTES                        "Montant"
#define CP_MODEPAIEMENT_LIGNRECETTES                   "ModePaiement"
#define CP_TIREURCHEQUE_LIGNRECETTES                   "TireurCheque"
#define CP_IDCPTEVIREMENT_LIGNRECETTES                 "CompteVirement"
#define CP_BANQUECHEQUE_LIGNRECETTES                   "BanqueCheque"
#define CP_TIERSPAYANT_LIGNRECETTES                    "TiersPayant"
#define CP_NOMPAYEUR_LIGNRECETTES                      "NomTiers"
#define CP_COMMISSION_LIGNRECETTES                     "Commission"
#define CP_MONNAIE_LIGNRECETTES                        "Monnaie"
#define CP_IDREMISECHQ_LIGNRECETTES                    "idRemise"
#define CP_CHQENATTENTE_LIGNRECETTES                   "EnAttente"
#define CP_IDUSERENREGISTREUR_LIGNRECETTES             "EnregistrePar"
#define CP_TYPERECETTE_LIGNRECETTES                    "TypeRecette"

                            //! Table ComptaMedicale.autresrecettes
#define CP_ID_AUTRESRECETTES                            "idRecette"
#define CP_IDUSER_AUTRESRECETTES                        "idUser"
#define CP_DATE_AUTRESRECETTES                          "DateRecette"
#define CP_LIBELLE_AUTRESRECETTES                       "Libelle"
#define CP_TYPERECETTE_AUTRESRECETTES                   "Typerecette"
#define CP_MONTANT_AUTRESRECETTES                       "Montant"
#define CP_PAIEMENT_AUTRESRECETTES                      "Paiement"
#define CP_NOOPERATION_AUTRESRECETTES                   "Nooperation"
#define CP_MONNAIE_AUTRESRECETTES                       "Monnaie"
#define CP_IDCPTEVIREMENT_AUTRESRECETTES                "CompteVirement"
#define CP_BANQUECHEQUE_AUTRESRECETTES                  "BanqueCheque"
#define CP_TIREURCHEQUE_AUTRESRECETTES                  "TireurCheque"
#define CP_DATEENREGISTREMENT_AUTRESRECETTES            "DateEnregistrement"
#define CP_IDREMISECHQ_AUTRESRECETTES                   "idRemise"
#define CP_CHQENATTENTE_AUTRESRECETTES                  "EnAttente"

                            //! Table ComptaMedicale.remisecheques
#define CP_ID_REMCHEQ                                   "idRemCheq"
#define CP_MONTANT_REMCHEQ                              "Montant"
#define CP_DATE_REMCHEQ                                 "RCDate"
#define CP_IDCOMPTE_REMCHEQ                             "idCompte"

                            //! Table ComptaMedicale.tiers
#define CP_ID_TIERS                                     "idTiers"
#define CP_NOM_TIERS                                    "Nomtiers"
#define CP_ADRESSE1_TIERS                               "Adresse1Tiers"
#define CP_ADRESSE2_TIERS                               "Adresse2Tiers"
#define CP_ADRESSE3_TIERS                               "Adresse3Tiers"
#define CP_CODEPOSTAL_TIERS                             "Codepostaltiers"
#define CP_VILLE_TIERS                                  "Villetiers"
#define CP_TELEPHONE_TIERS                              "Telephonetiers"
#define CP_FAX_TIERS                                    "FaxTiers"
#define CP_MAIL_TIERS                                   "MailTiers"
#define CP_WEB_TIERS                                    "WebsiteTiers"

                            //! Table ComptaMedicale.verrouscomptaactes
#define CP_IDACTE_VERROUCOMPTA                          "idActe"
#define CP_DATEVERROU_VERROUCOMPTA                       "DateTimeVerrou"
#define CP_POSEPAR_VERROUCOMPTA                          "PosePar"

                            //! Table ComptaMedicale.rubriques2035
#define CP_ID_2035                                      "idRubrique"
#define CP_REFFISCALE_2035                              "Reffiscale"
#define CP_FAMFISCALE_2035                              "Famfiscale"

                            //! Table ComptaMedicale.typepaiementactes
#define CP_IDACTE_TYPEPAIEMENTACTES                    "idActe"
#define CP_TYPEPAIEMENT_TYPEPAIEMENTACTES              "TypePaiement"
#define CP_TIERS_TYPEPAIEMENTACTES                     "Tiers"

                            //! Table Ophtalmologie.ProgrammesOperatoires
#define CP_ID_LIGNPRGOPERATOIRE                         "idLigneProgramme"
#define CP_HEURE_LIGNPRGOPERATOIRE                      "ProgHeure"
#define CP_IDSESSION_LIGNPRGOPERATOIRE                  "idSession"
#define CP_IDPATIENT_LIGNPRGOPERATOIRE                  "idPat"
#define CP_TYPEANESTH_LIGNPRGOPERATOIRE                 "TypeAnesthesie"
#define CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE         "TypeIntervention"
#define CP_COTE_LIGNPRGOPERATOIRE                       "Cote"
#define CP_IDIOL_LIGNPRGOPERATOIRE                      "idIOL"
#define CP_PWRIOL_LIGNPRGOPERATOIRE                     "PwIOL"
#define CP_CYLIOL_LIGNPRGOPERATOIRE                     "CylIOL"
#define CP_OBSERV_LIGNPRGOPERATOIRE                     "Observation"
#define CP_IDACTE_LIGNPRGOPERATOIRE                     "idActe"
#define CP_INCIDENT_LIGNPRGOPERATOIRE                   "Incident"

                            //! Table Ophtalmologie.SessionsOperatoires
#define CP_ID_SESSIONOPERATOIRE                         "idSession"
#define CP_DATE_SESSIONOPERATOIRE                       "DateSession"
#define CP_IDUSER_SESSIONOPERATOIRE                     "idUser"
#define CP_IDAIDE_SESSIONOPERATOIRE                     "idAide"
#define CP_IDLIEU_SESSIONOPERATOIRE                     "idLieu"
#define CP_INCIDENT_SESSIONOPERATOIRE                   "Incident"

                            //! Table Ophtalmologie.IOLs
#define CP_ID_IOLS                                      "idIOL"
#define CP_IDMANUFACTURER_IOLS                          "idmanufacturer"
#define CP_MODELNAME_IOLS                               "modelname"
#define CP_DIAOPT_IOLS                                  "DiametreOptique"
#define CP_DIAALL_IOLS                                  "DiametreAll"
#define CP_ACD_IOLS                                     "ACD"
#define CP_MINPWR_IOLS                                  "PuissanceMin"
#define CP_MAXPWR_IOLS                                  "PuissanceMax"
#define CP_PWRSTEP_IOLS                                 "PuissancePas"
#define CP_MINCYL_IOLS                                  "CylindreMin"
#define CP_MAXCYL_IOLS                                  "CylindreMax"
#define CP_CYLSTEP_IOLS                                 "CylindrePas"
#define CP_CSTEAOPT_IOLS                                "CsteAOptique"
#define CP_CSTEAECHO_IOLS                               "CsteAEcho"
#define CP_HAIGISA0_IOLS                                "Haigisa0"
#define CP_HAIGISA1_IOLS                                "Haigisa1"
#define CP_HAIGISA2_IOLS                                "Haigisa2"
#define CP_HOLL1_IOLS                                   "Holladay1sf"
#define CP_DIAINJECTEUR_IOLS                            "DiametreInjecteur"
#define CP_ARRAYIMG_IOLS                                "Image"
#define CP_TYPIMG_IOLS                                  "TypeImage"
#define CP_MATERIAU_IOLS                                "Materiau"
#define CP_REMARQUE_IOLS                                "Remarque"
#define CP_PRECHARGE_IOLS                               "Precharge"
#define CP_JAUNE_IOLS                                   "Jaune"
#define CP_TORIC_IOLS                                   "Toric"
#define CP_EDOF_IOLS                                    "EDOF"
#define CP_MULTIFOCAL_IOLS                              "Multifocal"
#define CP_INACTIF_IOLS                                 "IOLInactif"
#define CP_TYP_IOLS                                     "TypeImplant"

                            //! Table Ophtalmologie.TypeInterventions
#define CP_ID_TYPINTERVENTION                           "idTypeIntervention"
#define CP_TYPEINTERVENTION_TYPINTERVENTION             "TypeIntervention"
#define CP_CODECCAM_TYPINTERVENTION                     "CodeIntervention"
#define CP_DUREE_TYPINTERVENTION                        "DureeIntervention"

                            //! Table Ophtalmologie.Refractions
#define CP_ID_REFRACTIONS                              "idRefraction"
#define CP_IDPAT_REFRACTIONS                           "idPat"
#define CP_IDACTE_REFRACTIONS                          "idActe"
#define CP_DATE_REFRACTIONS                            "DateRefraction"
#define CP_TYPEMESURE_REFRACTIONS                      "QuelleMesure"
#define CP_DISTANCEMESURE_REFRACTIONS                  "QuelleDistance"
#define CP_CYCLOPLEGIE_REFRACTIONS                     "Cycloplegie"
#define CP_ODMESURE_REFRACTIONS                        "ODcoche"
#define CP_SPHEREOD_REFRACTIONS                        "SphereOD"
#define CP_CYLINDREOD_REFRACTIONS                      "CylindreOD"
#define CP_AXECYLOD_REFRACTIONS                        "AxeCylindreOD"
#define CP_AVLOD_REFRACTIONS                           "AVLOD"
#define CP_ADDVPOD_REFRACTIONS                         "AddVPOD"
#define CP_AVPOD_REFRACTIONS                           "AVPOD"
#define CP_PRISMEOD_REFRACTIONS                        "PrismeOD"
#define CP_BASEPRISMEOD_REFRACTIONS                    "BasePrismeOD"
#define CP_BASEPRISMETEXTOD_REFRACTIONS                "BasePrismeTextOD"
#define CP_PRESSONOD_REFRACTIONS                       "PressOnOD"
#define CP_DEPOLIOD_REFRACTIONS                        "DepoliOD"
#define CP_PLANOD_REFRACTIONS                          "PlanOD"
#define CP_RYSEROD_REFRACTIONS                         "RyserOD"
#define CP_FORMULEOD_REFRACTIONS                       "FormuleOD"
#define CP_OGMESURE_REFRACTIONS                        "OGcoche"
#define CP_SPHEREOG_REFRACTIONS                        "SphereOG"
#define CP_CYLINDREOG_REFRACTIONS                      "CylindreOG"
#define CP_AXECYLOG_REFRACTIONS                        "AxeCylindreOG"
#define CP_AVLOG_REFRACTIONS                           "AVLOG"
#define CP_ADDVPOG_REFRACTIONS                         "AddVPOG"
#define CP_AVPOG_REFRACTIONS                           "AVPOG"
#define CP_PRISMEOG_REFRACTIONS                        "PrismeOG"
#define CP_BASEPRISMEOG_REFRACTIONS                    "BasePrismeOG"
#define CP_BASEPRISMETEXTOG_REFRACTIONS                "BasePrismeTextOG"
#define CP_PRESSONOG_REFRACTIONS                       "PressOnOG"
#define CP_DEPOLIOG_REFRACTIONS                        "DepoliOG"
#define CP_PLANOG_REFRACTIONS                          "PlanOG"
#define CP_RYSEROG_REFRACTIONS                         "RyserOG"
#define CP_FORMULEOG_REFRACTIONS                       "FormuleOG"
#define CP_COMMENTAIREORDO_REFRACTIONS                 "CommentaireOrdoLunettes"
#define CP_TYPEVERRES_REFRACTIONS                      "QuelsVerres"
#define CP_OEIL_REFRACTIONS                            "QuelOeil"
#define CP_MONTURE_REFRACTIONS                         "Monture"
#define CP_VERRETEINTE_REFRACTIONS                     "VerreTeinte"
#define CP_PD_REFRACTIONS                              "PD"

                            //! Table Ophtalmologie.commentaireslunettes
#define CP_ID_COMLUN                                    "idCommentLunet"
#define CP_TEXT_COMLUN                                  "TextComment"
#define CP_RESUME_COMLUN                                "ResumeComment"
#define CP_PARDEFAUT_COMLUN                             "ParDefautComment"
#define CP_IDUSER_COMLUN                                "idUser"
#define CP_PUBLIC_COMLUN                                "ComPublic"

                        //! Table Ophtalmologie.tonometries
#define CP_ID_TONO                                      "idTono"
#define CP_IDPAT_TONO                                   "idPat"
#define CP_TOD_TONO                                     "TOOD"
#define CP_TOG_TONO                                     "TOOG"
#define CP_TODATE_TONO                                  "TODate"
#define CP_TOTYPE_TONO                                  "TOType"

                        //! Table Ophtalmologie.pachymetries
#define CP_ID_PACHY                                     "idPachy"
#define CP_IDPAT_PACHY                                  "idPat"
#define CP_PACHYOD_PACHY                                "pachyOD"
#define CP_PACHYOG_PACHY                                "pachyOG"
#define CP_PACHYDATE_PACHY                              "pachyDate"
#define CP_PACHYTYPE_PACHY                              "pachyType"

                            //! Table Ophtalmologie.DonneesOphtaPatients
#define CP_ID_DATAOPHTA                                "idMesure"
#define CP_IDPATIENT_DATAOPHTA                         "idPat"
#define CP_MESURE_DATAOPHTA                            "QuelleMesure"
#define CP_DISTANCE_DATAOPHTA                          "QuelleDistance"
#define CP_K1OD_DATAOPHTA                              "K1OD"
#define CP_K2OD_DATAOPHTA                              "K2OD"
#define CP_AXEKOD_DATAOPHTA                            "AxeKOD"
#define CP_K1OG_DATAOPHTA                              "K1OG"
#define CP_K2OG_DATAOPHTA                              "K2OG"
#define CP_AXEKOG_DATAOPHTA                            "AxeKOG"
#define CP_MODEMESUREKERATO_DATAOPHTA                  "OrigineK"
#define CP_DATEKERATO_DATAOPHTA                        "DateK"
#define CP_SPHEREOD_DATAOPHTA                          "SphereOD"
#define CP_CYLINDREOD_DATAOPHTA                        "CylindreOD"
#define CP_AXECYLINDREOD_DATAOPHTA                     "AxeCylindreOD"
#define CP_AVLOD_DATAOPHTA                             "AVLOD"
#define CP_ADDVPOD_DATAOPHTA                           "AddVPOD"
#define CP_AVPOD_DATAOPHTA                             "AVPOD"
#define CP_DATEREFRACTIONOD_DATAOPHTA                  "DateRefOD"
#define CP_SPHEREOG_DATAOPHTA                          "SphereOG"
#define CP_CYLINDREOG_DATAOPHTA                        "CylindreOG"
#define CP_AXECYLINDREOG_DATAOPHTA                     "AxeCylindreOG"
#define CP_AVLOG_DATAOPHTA                             "AVLOG"
#define CP_ADDVPOG_DATAOPHTA                           "AddVPOG"
#define CP_AVPOG_DATAOPHTA                             "AVPOG"
#define CP_DATEREFRACTIONOG_DATAOPHTA                  "DateRefOG"
#define CP_ECARTIP_DATAOPHTA                           "PD"
#define CP_DIOTRIESK1OD_DATAOPHTA                      "DioptrieK1OD"
#define CP_DIOTRIESK2OD_DATAOPHTA                      "DioptrieK2OD"
#define CP_DIOTRIESK1OG_DATAOPHTA                      "DioptrieK1OG"
#define CP_DIOTRIESK2OG_DATAOPHTA                      "DioptrieK2OG"

// Divers
#define PDF                                         "pdf"
#define JPG                                         "jpg"
#define PNG                                         "png"

#define BILANORTHOPTIQUE                            "BO"
#define CORRECTION                                  "Correction"
#define COURRIER                                    "Courrier"
#define DOCUMENTRECU                                "DocRecu"
#define IMAGERIE                                    "Imagerie"
#define ORHTOPTIE                                   "Orthoptie"
#define PRESCRIPTION                                "Prescription"
#define PRESCRIPTIONLUNETTES                        "PrescrLun"
#define COURRIERADMINISTRATIF                       "DocAdmin"
#define VIDEO                                       "Video"
#define FACTURE                                     "Facture"
#define ECHEANCIER                                  "Echéancier"

                        //! les types d'IOLS
#define IOL_CP                                      "Chambre postérieure"
#define IOL_CA                                      "Chambre antérieure"
#define IOL_ADDON                                   "Add-on"
#define IOL_IRIEN                                   "Support irien"
#define IOL_CAREFRACTIF                             "Refractif CA"
#define IOL_AUTRE                                   "Autre"

#define NOM_APPLI                                   "Rufus"

#define LOGIN_SQL                                   "adminrufus"
#define MDP_SQL                                     "gaxt78iy"

#define NOM_POSTEIMPORTDOCS                         "posteimportdocs"

#define NOM_ADMINISTRATEUR                          "Admin"
#define MDP_ADMINISTRATEUR                          "bob"


//les TCP
#define NOM_PORT_TCPSERVEUR                         "50885"
#define TCPMSG_StringidPoste                        "StringidPoste"                                     /*! nom du message fournissant le Stringid du poste qui vient de se connecter */
#define TCPMSG_DeconnexionPoste                     "DeconnexionPoste"                                  /*! nom du message fournissant le Stringid du poste qui vient de se déconnecter */
#define TCPMSG_MAJCorrespondants                    "MajCorrespondants"                                 /*! fin du message invitant à mettre à jour la liste des correspondants */
#define TCPMSG_MAJDocsExternes                      "MajDocsExternes"                                   /*! fin du message invitant à mettre à jour la liste des documents externes pour un patient */
#define TCPMSG_MAJSalAttente                        "MajSalAttente"                                     /*! fin du message invitant à mettre à jour la salle d'attente */
#define TCPMSG_MAJPatient                           "MajPatient"                                        /*! fin du message indiquant qu'un patient a été créé ou modifié */
#define TCPMSG_MAJListeUsers                        "MajListeUsers"                                     /*! fin du message invitant à mettre à jour la liste des utilisateurs */
#define TCPMSG_MsgBAL                               "MsgRecuDansBAL"                                    /*! fin du message informant de la réception d'un message de messagerie instantanée */
#define TCPMSG_Separator                            "triumphspeedtriple1050"                            /*! separateur au sein d'un message */
#define TCPMSG_AskListeStringId                     "AskLisPostes"                                      /*! fin du message demandant la liste des stringid des postes connectes */
#define TCPMSG_ListeStringIdPostesConnectes         "ListeSockets"                                      /*! fin du message informant le client que ce message contient la liste des stringid des postes connectes */
#define TCPMSG_EnvoieListSocket                     "EnvoieListe"                                       /*! fin du message demandant l'envoi de la liste des sockets à un client */
#define TCPMSG_TestConnexion                        "TestConnexion"                                     /*! fin du message de test de la connexion */
#define TCPMSG_OKConnexion                          "OKConnexion"                                       /*! fin du message de confirmation de la connexion */

// Constantes de statut salle d'attente  //+++++ATTENTION ces constantes servent dans des requêtes SQL - NE PAS METTRE DE CARACTERES SPECIAUX
#define ARRIVE                                  tr("Arrivé")
#define ENCOURS                                 tr("En cours")
#define ENATTENTENOUVELEXAMEN                   tr("En attente de nouvel examen par ")
#define RETOURACCUEIL                           tr("Retour accueil")
#define ENCOURSEXAMEN                           tr("En cours de consultation avec ")

// Les fichiers et répertoires divers
#define NOM_DIR_RUFUS                           "/Documents/Rufus"
#define NOM_DIR_RUFUSADMIN                      "/Documents/RufusAdmin"
#define NOM_DIR_IMAGERIE                        "/Imagerie"
#define NOM_DIR_RESSOURCES                      "/Ressources"
#define NOM_DIR_FACTURES                        "/Factures"
#define NOM_DIR_ORIGINAUX                       "/Originaux"
#define NOM_DIR_FACTURESSANSLIEN                "/FacturesSansLien"
#define NOM_DIR_DOSSIERECHANGE                  "/DossierEchange"
#define NOM_DIR_IMAGES                          "/Images"
#define NOM_DIR_VIDEOS                          "/Videos"
#define NOM_DIR_PROV                            "/Prov"
#define NOM_DIR_REFRACTION                      "/Refraction"
#define NOM_DIR_FRONTO                          "/Fronto"
#define NOM_DIR_AUTOREF                         "/Autoref"
#define NOM_DIR_REFRACTEUR                      "/Refracteur"
#define NOM_DIR_REFRACTEURIN                    "/In"
#define NOM_DIR_REFRACTEUROUT                   "/Out"
#define NOM_DIR_TONO                            "/Tono"
#define NOM_DIR_LOGS                            "/Logs"
#define NOM_DIR_ECHECSTRANSFERTS                "/EchecsTransferts"

#define NOM_FILE_INI                            "/Rufus.ini"
#define NOM_FILE_SCRIPTBACKUP                   "/RufusScriptBackup.sh"
#define NOM_FILE_SCRIPT_MACOS_PLIST             "/rufus.bup.plist"
#define NOM_FILE_ENTETEORDO                     "/Entete_Ordonnance.txt"
#define NOM_FILE_CORPSORDO                      "/Corps_Ordonnance.txt"
#define NOM_FILE_ENTETEORDOALD                  "/Entete_OrdoALD.txt"
#define NOM_FILE_CORPSORDOALD                   "/Corps_OrdoALD.txt"
#define NOM_FILE_PIEDPAGE                       "/Pied_Ordonnance.txt"
#define NOM_FILE_PIEDPAGEORDOLUNETTES           "/Pied_Ordonnance_Lunettes.txt"
#define NOM_FILE_PDF                            "/pdf.pdf"
#define NOM_FILE_XMLAUTOREF                     "/AR.xml"
#define NOM_FILE_XMLFRONTO                      "/LM.xml"
#define NOM_FILE_TONO                           "/TO.xml"

#define PATH_DIR_RUFUS                          QDir::homePath() + NOM_DIR_RUFUS
#define PATH_DIR_RUFUSADMIN                     QDir::homePath() + NOM_DIR_RUFUSADMIN
#define PATH_DIR_IMAGERIE                       PATH_DIR_RUFUS NOM_DIR_IMAGERIE
#define PATH_DIR_RESSOURCES                     PATH_DIR_RUFUS NOM_DIR_RESSOURCES

#define PATH_DIR_IMAGES                         PATH_DIR_IMAGERIE NOM_DIR_IMAGES
#define PATH_DIR_FACTURES                       PATH_DIR_IMAGERIE NOM_DIR_FACTURES
#define PATH_DIR_ORIGINAUX                      PATH_DIR_IMAGERIE NOM_DIR_ORIGINAUX
#define PATH_DIR_FACTURESSANSLIEN               PATH_DIR_IMAGERIE NOM_DIR_FACTURESSANSLIEN
#define PATH_DIR_DOSSIERECHANGE                 PATH_DIR_IMAGERIE NOM_DIR_DOSSIERECHANGE
#define PATH_DIR_REFRACTION                     PATH_DIR_DOSSIERECHANGE NOM_DIR_REFRACTION
#define PATH_DIR_VIDEOS                         PATH_DIR_IMAGERIE NOM_DIR_VIDEOS
#define PATH_DIR_PROV                           PATH_DIR_IMAGERIE NOM_DIR_PROV
#define PATH_DIR_ECHECSTRANSFERTS               PATH_DIR_IMAGERIE NOM_DIR_ECHECSTRANSFERTS

#define PATH_DIR_FRONTO                         PATH_DIR_REFRACTION NOM_DIR_FRONTO
#define PATH_DIR_AUTOREF                        PATH_DIR_REFRACTION NOM_DIR_AUTOREF
#define PATH_DIR_TONO                           PATH_DIR_REFRACTION NOM_DIR_TONO
#define PATH_DIR_REFRACTEUR                     PATH_DIR_REFRACTION NOM_DIR_REFRACTEUR
#define PATH_DIR_REFRACTEUR_IN                  PATH_DIR_REFRACTEUR NOM_DIR_REFRACTEURIN
#define PATH_DIR_REFRACTEUR_AUTOREFIN           PATH_DIR_REFRACTEUR_IN NOM_DIR_AUTOREF
#define PATH_DIR_REFRACTEUR_FRONTOIN            PATH_DIR_REFRACTEUR_IN NOM_DIR_FRONTO
#define PATH_DIR_REFRACTEUR_TONOIN              PATH_DIR_REFRACTEUR_IN NOM_DIR_TONO
#define PATH_DIR_REFRACTEUR_OUT                 PATH_DIR_REFRACTEUR NOM_DIR_REFRACTEUROUT

#define PATH_FILE_ENTETEORDO                    PATH_DIR_RESSOURCES NOM_FILE_ENTETEORDO
#define PATH_FILE_CORPSORDO                     PATH_DIR_RESSOURCES NOM_FILE_CORPSORDO
#define PATH_FILE_ENTETEORDOALD                 PATH_DIR_RESSOURCES NOM_FILE_ENTETEORDOALD
#define PATH_FILE_CORPSORDOALD                  PATH_DIR_RESSOURCES NOM_FILE_CORPSORDOALD
#define PATH_FILE_PIEDPAGE                      PATH_DIR_RESSOURCES NOM_FILE_PIEDPAGE
#define PATH_FILE_PIEDPAGEORDOLUNETTES          PATH_DIR_RESSOURCES NOM_FILE_PIEDPAGEORDOLUNETTES
#define PATH_FILE_PDF                           PATH_DIR_RESSOURCES NOM_FILE_PDF
#define PATH_FILE_INI                           PATH_DIR_RUFUS NOM_FILE_INI
#define PATH_FILE_SCRIPTBACKUP                  PATH_DIR_RUFUS NOM_FILE_SCRIPTBACKUP
#define PATH_FILE_SCRIPT_MACOS_PLIST            "/Library/LaunchAgents" NOM_FILE_SCRIPT_MACOS_PLIST
#define PATH_FILE_XMLAUTOREF                    PATH_DIR_AUTOREF NOM_FILE_XMLAUTOREF
#define PATH_FILE_XMLFRONTO                     PATH_DIR_FRONTO NOM_FILE_XMLFRONTO

// Variables mysql
#define MAX_ALLOWED_PACKET                      "16"

// le son d'alarme
#define NOM_ALARME                              "://goutte.wav"

// la largeur de la formule de réfraction dans le champ ActeTexte
#define LARGEUR_FORMULE                         "310"

// la couleur des titres
#define COULEUR_TITRES                          "blue"

// le lien vers la CCAM
#define LIEN_CCAM                               "https://www.ameli.fr/accueil-de-la-ccam/trouver-un-acte/consultation-par-chapitre.php?chap=a%3A0%3A%7B%7D&add=2#chapitre_2"

// la taille maximale des images
#define TAILLEMAXIIMAGES                        262144

// la police d'écran par défaut et le facteur de correction des td width html en impression qui varie entre Linux et OSX (???)
#define POLICEPARDEFAUT         "Comic Sans MS"
#define POLICEATTRIBUTPARDEFAUT "Regular"
#ifdef Q_OS_MACX
#define CORRECTION_td_width     1
#endif
#ifdef Q_OS_LINUX
#define CORRECTION_td_width     1.3
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
#define NEUTRE                  "neutre"

#define HAUTEUR_SANS_ORDONNANCE_MINI                    191                 // Hauteur du formulaire Refraction sans la partie ordonnance sans detail sans observ
#define HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL             335                 // Hauteur du formulaire Refraction sans la partie ordonnance avec detail sans observ
#define HAUTEUR_AVEC_ORDONNANCE_SANS_DETAIL             450                 // Hauteur du formulaire Refraction avec partie ordonnance sans detail
#define HAUTEUR_AVEC_ORDONNANCE_AVEC_DETAIL             590                 // Hauteur du formulaire Refraction avec partie ordonnance avec detail
#define LARGEURLISTE                                    1300
#define LARGEURNORMALE                                  1260
#define HAUTEUR_MINI                                    280
#define HAUTEUR_NORMALE                                 970

//Opérations bancaires
#define VIREMENT            "Virement"
#define CHEQUE              "Chèque"
#define PRELEVEMENT         "Prélèvement"
#define TIP                 "TIP"
#define ESPECES             "Espèces"
#define CARTECREDIT         "Carte de crédit"
#define GRATUIT             "Acte gratuit"
#define IMPAYE              "Impayé"

//Modes Paiements comptables
#define TRS                 "T"       // plutôt un type de paiement d'acte qu'un mode de paiement comptable
#define TP                  "T"       // Abrégé pour TIP
#define ESP                 "E"
#define GRAT                "G"
#define CHQ                 "C"
#define IMP                 "I"
#define VRMT                "V"
#define PLVMT               "P"
#define CB                  "B"

// ancres et comentaires html
#define HTMLCOMMENT_LINUX           "<!LINUX>"
#define HTMLCOMMENT_MAC             "<!MAC>"
#define HTMLANCHOR_BODEBUT          "<BOdebut>"
#define HTMLANCHOR_BOFIN            "<BOfin>"
#define HTMLANCHOR_PACHYDEBUT       "pachydebut"
#define HTMLANCHOR_PACHYFIN         "pachyfin"
#define HTMLANCHOR_TODEBUT          "TOdebut"
#define HTMLANCHOR_TOFIN            "TOfin"
#define HTML_FINPARAGRAPH           "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px;\"></p>"
#define HTML_RETOURLIGNE            "<p style = \"margin-top:0px; margin-bottom:0px;\">"


// valeurs extrêmes de kératométrie
#define MinK                        5
#define MaxK                        9.5

//modes de mesure tonométrie
#define AIR_TO                      "Air"
#define APLANATION_TO               "Aplanation"
#define AUTRE_TO                    "Autre"

//modes de mesure pachymétrie
#define OPTIQUE_PACHY               "O"
#define OCT_PACHY                   "T"
#define ECHO_PACHY                  "E"

//modes de mesure biométrie
#define OPTIQUE_BIOMETRIE           "OBM"
#define ECHO_BIOMETRIE              "EBM"
#define AUTRE_BIOMETRIE             "ABM"

//modes de mesure appareils refraction
#define COM1                        "COM1"
#define COM2                        "COM2"
#define COM3                        "COM3"
#define COM4                        "COM4"
#define COM5                        "COM5"
#define COM6                        "COM6"
#define COM7                        "COM7"
#define COM8                        "COM8"
#define BOX                         "Box"
#define DOSSIER_ECHANGE             "Dossier Echange"
#define N_NULL                      "-"

//datas des ports séries
#define PORT                        "Port"
#define BAUDRATE                    "BaudRate"
#define DATABITS                    "DataBits"
#define PARITY                      "Parity"
#define STOPBITS                    "StopBits"
#define FLOWCONTROL                 "FlowControl"

//mots utilisés en macros dans les documents à imprimer
#define STR_TITRUSER            "TITREUSER"
#define STR_NOMPAT              "NOM PATIENT"
#define STR_DATEDOC             "DATE"
#define STR_DDNPAT              "DDN"
#define STR_TITREPAT            "TITRE PATIENT"
#define STR_AGEPAT              "AGE PATIENT"
#define STR_PRENOMPAT           "PRENOM PATIENT"
#define STR_SEXEPAT             "SEXE PATIENT"
#define STR_MGPAT               "MEDECIN PATIENT"
#define STR_MGPATTITRE          "TITRE MEDECIN PATIENT"
#define STR_POLITESSEMG         "FORMULE POLITESSE MEDECIN"
#define STR_PRENOMMG            "PRENOM MEDECIN"
#define STR_NOMMG               "NOM MEDECIN"
#define STR_REFRACT             "REFRACTION"
#define STR_KERATO              "KERATOMETRIE"
#define STR_POLITESSECOR        "FORMULE POLITESSE CORRESPONDANT"
#define STR_CORPAT              "CORRESPONDANT PATIENT"
#define STR_PRENOMCOR           "PRENOM CORRESPONDANT"
#define STR_NOMCOR              "NOM CORRESPONDANT"
#define STR_PRENOMUSER          "PRENOM RESPONSABLE"
#define STR_NOMUSER             "NOM RESPONSABLE"
#define STR_TELEPHONE           "TELEPHONE PATIENT"

#define STR_PROVENANCE          "PROVENANCE"
#define STR_TYPESEJOUR          "SEJOUR"
#define STR_COTE                "COTE"
#define STR_SITE                "SITE"
#define STR_ANESTHINTERVENTION  "ANESTHINTERVENTION"
#define STR_DATEINTERVENTION    "DATEINTERVENTION"
#define STR_HEUREINTERVENTION   "HEUREINTERVENTION"
#define STR_COTEINTERVENTION    "COTEINTERVENTION"
#define STR_SITEINTERVENTION    "SITEINTERVENTION"
#define STR_TYPEINTERVENTION    "TYPEINTERVENTION"

#define STR_NOCOR               "PAS DE CORRESPONDANT RÉFÉRENCÉ POUR CE PATIENT"

//champs de la map d'un rendez-vous
#define RDV_MOTIF               "MOTIF"
#define RDV_MESSAGE             "MESSAGE"
#define RDV_HEURE               "HEURE"
#define RDV_IDSUPERVISEUR       "IDSUPERVISEUR"

//QSettings
#define Param_Poste_PortFronto                          "Param_Poste/PortFronto"
#define Param_Poste_PortAutoref                         "Param_Poste/PortAutoref"
#define Param_Poste_PortTono                            "Param_Poste/PortTonometre"
#define Param_Poste_PortRefracteur                      "Param_Poste/PortRefracteur"
#define Param_Poste_PortFronto_DossierEchange           Param_Poste_PortFronto "/Reseau"
#define Param_Poste_PortAutoref_DossierEchange          Param_Poste_PortAutoref "/Reseau"
#define Param_Poste_PortTono_DossierEchange             Param_Poste_PortTono "/Reseau"
#define Param_Poste_PortRefracteur_DossierEchange       Param_Poste_PortRefracteur "/Reseau"
#define Param_Poste_PortRefracteur_DossierEchange_Autoref Param_Poste_PortRefracteur_DossierEchange "AdressAutoref"
#define Param_Poste_PortRefracteur_DossierEchange_Fronto  Param_Poste_PortRefracteur_DossierEchange "/AdressFronto"
#define Param_Poste_PortFronto_COM_baudrate             Param_Poste_PortFronto "/baudRate"
#define Param_Poste_PortFronto_COM_databits             Param_Poste_PortFronto "/dataBits"
#define Param_Poste_PortFronto_COM_parity               Param_Poste_PortFronto "/parity"
#define Param_Poste_PortFronto_COM_stopBits             Param_Poste_PortFronto "/stopBits"
#define Param_Poste_PortFronto_COM_flowControl          Param_Poste_PortFronto "/flowControl"
#define Param_Poste_PortAutoref_COM_baudrate            Param_Poste_PortAutoref "/baudRate"
#define Param_Poste_PortAutoref_COM_databits            Param_Poste_PortAutoref "/dataBits"
#define Param_Poste_PortAutoref_COM_parity              Param_Poste_PortAutoref "/parity"
#define Param_Poste_PortAutoref_COM_stopBits            Param_Poste_PortAutoref "/stopBits"
#define Param_Poste_PortAutoref_COM_flowControl         Param_Poste_PortAutoref "/flowControl"
#define Param_Poste_PortRefracteur_COM_baudrate         Param_Poste_PortRefracteur "/baudRate"
#define Param_Poste_PortRefracteur_COM_databits         Param_Poste_PortRefracteur "/dataBits"
#define Param_Poste_PortRefracteur_COM_parity           Param_Poste_PortRefracteur "/parity"
#define Param_Poste_PortRefracteur_COM_stopBits         Param_Poste_PortRefracteur "/stopBits"
#define Param_Poste_PortRefracteur_COM_flowControl      Param_Poste_PortRefracteur "/flowControl"
#define Param_Poste_PortTono_COM_baudrate               Param_Poste_PortTono "/baudRate"
#define Param_Poste_PortTono_COM_databits               Param_Poste_PortTono "/dataBits"
#define Param_Poste_PortTono_COM_parity                 Param_Poste_PortTono "/parity"
#define Param_Poste_PortTono_COM_stopBits               Param_Poste_PortTono "/stopBits"
#define Param_Poste_PortTono_COM_flowControl            Param_Poste_PortTono "/flowControl"

#define Param_Poste_Fronto                              "Param_Poste/Fronto"
#define Param_Poste_Autoref                             "Param_Poste/Autoref"
#define Param_Poste_Tono                                "Param_Poste/Tonometre"
#define Param_Poste_Refracteur                          "Param_Poste/Refracteur"

#define Dossier_ClesSSL                                 "/DossierClesSSL"
#define Dossier_Imagerie                                "/DossierImagerie"
#define Dossier_DocsScannes                             "/DossiersDocsScannes"
#define Dossier_Videos                                  "/DossiersVideos"
#define Dossier_Documents                               "/DossiersDocuments/"
#define Param_SQLExecutable                             "Param_Poste/DirSQLExecutable"

#define Param_SSL                                       "/SSL"
#define Param_Serveur                                   "/Serveur"
#define Param_Port                                      "/Port"
#define Param_Active                                     "/Active"

#define Imprimante_TailleEnTete                         "Param_Imprimante/TailleEnTete"
#define Imprimante_TailleEnTeteALD                      "Param_Imprimante/TailleEnTeteALD"
#define Imprimante_TaillePieddePage                     "Param_Imprimante/TaillePieddePage"
#define Imprimante_TaillePieddePageOrdoLunettes         "Param_Imprimante/TaillePieddePageOrdoLunettes"
#define Imprimante_TailleTopMarge                       "Param_Imprimante/TailleTopMarge"
#define Imprimante_ApercuAvantImpression                "Param_Imprimante/ApercuAvantImpression"
#define Imprimante_OrdoAvecDupli                        "Param_Imprimante/OrdoAvecDupli"

#define PrioritaireGestionDocs                          "/PrioritaireGestionDocs"

#define Ville_Defaut                                    "Param_Poste/VilleParDefaut"
#define CodePostal_Defaut                               "Param_Poste/CodePostalParDefaut"

#define Poste_VersionRessources                         "Param_Poste/VersionRessources"
#define Position_Fiche                                  "PositionsFiches/Position"

#endif // MACROS_H
