/* (C) 2018 LAINE SERGE
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
#define VERSION_BASE                                58  // version de la base de données
#define VERSION_RESSOURCES                          34  // version des fichiers de ressources

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

#define TBL_BILANORTHO                        "Ophtalmologie.bilanortho"
#define TBL_BIOMETRIES                        "Ophtalmologie.biometries"
#define TBL_COMMENTAIRESLUNETTES              "Ophtalmologie.commentaireslunettes"
#define TBL_DONNEES_OPHTA_PATIENTS            "Ophtalmologie.DonneesOphtaPatients"
#define TBL_REFRACTIONS                       "Ophtalmologie.refractions"
#define TBL_TONOMETRIE                        "Ophtalmologie.tonometries"
#define TBL_PACHYMETRIE                       "Ophtalmologie.pachymetries"

#define TBL_ACTES                             "rufus.Actes"
#define TBL_APPAREILSCONNECTESCENTRE          "rufus.appareilsconnectescentre"
#define TBL_APPAREILSREFRACTION               "rufus.AppareilsRefraction"
#define TBL_CCAM                              "rufus.ccam"
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
#define TBL_MESSAGES                          "rufus.Messagerie"
#define TBL_MESSAGESJOINTURES                 "rufus.MessagerieJointures"
#define TBL_MOTIFSRDV                         "rufus.MotifsRDV"
#define TBL_MOTSCLES                          "rufus.MotsCles"
#define TBL_MOTSCLESJOINTURES                 "rufus.MotsClesJointures"
#define TBL_PARAMSYSTEME                      "rufus.ParametresSysteme"
#define TBL_PATIENTS                          "rufus.Patients"
#define TBL_RENSEIGNEMENTSMEDICAUXPATIENTS    "rufus.renseignementsmedicauxpatients"
#define TBL_SALLEDATTENTE                     "rufus.salledattente"
#define TBL_SPECIALITES                       "rufus.specialitesmedicales"
#define TBL_UTILISATEURS                      "rufus.utilisateurs"
#define TBL_USERSCONNECTES                    "rufus.UtilisateursConnectes"
#define TBL_VILLES                            "rufus.Villes"

// Les champs
#define CP_ISALLLOADED                               "isallloaded"
#define CP_ISMEDICALlOADED                           "ismedicalloaded"
#define CP_ISSOCIALlOADED                            "issocialloaded"
#define CP_ISGENERALISTE                             "isgeneraliste"
#define CP_ISMEDECIN                                 "ismedecin"
#define CP_METIER                                    "metier"

                            //! Table Rufus.Patients
#define CP_IDPAT_PATIENTS                            "idPat"
#define CP_NOM_PATIENTS                              "PatNom"
#define CP_PRENOM_PATIENTS                           "PatPrenom"
#define CP_DDN_PATIENTS                              "PatDDN"
#define CP_SEXE_PATIENTS                             "Sexe"
#define CP_DATECREATION_PATIENTS                     "PatCreeLe"
#define CP_IDCREATEUR_PATIENTS                       "PatCreePar"
#define CP_IDLIEU_PATIENTS                           "NumCentre"
#define CP_COMMENTAIRE_PATIENTS                      "Commentaire"

                            //! Table Rufus.donneessocialespatients
#define CP_IDPAT_DSP                                 "idPat"
#define CP_ADRESSE1_DSP                              "PatAdresse1"
#define CP_ADRESSE2_DSP                              "PatAdresse2"
#define CP_ADRESSE3_DSP                              "PatAdresse3"
#define CP_CODEPOSTAL_DSP                            "PatCodepostal"
#define CP_VILLE_DSP                                 "PatVille"
#define CP_TELEPHONE_DSP                             "PatTelephone"
#define CP_PORTABLE_DSP                              "PatPortable"
#define CP_MAIL_DSP                                  "PatMail"
#define CP_NNI_DSP                                   "PatNNI"
#define CP_ALD_DSP                                   "PatALD"
#define CP_CMU_DSP                                   "PatCMU"
#define CP_PROFESSION_DSP                            "PatProfession"

                            //! Table Rufus.renseignementsmedicauxpatients
#define CP_IDPAT_RMP                                 "idPat"
#define CP_ATCDTSOPH_RMP                             "RMPAtcdtsOphs"
#define CP_TRAITMTOPH_RMP                            "RMPTtOphs"
#define CP_IMPORTANT_RMP                             "Important"
#define CP_RESUME_RMP                                "Resume"
#define CP_ATCDTSPERSOS_RMP                          "RMPAtcdtsPersos"
#define CP_ATCDTSFAMLXS_RMP                          "RMPAtcdtsFamiliaux"
#define CP_TRAITMTGEN_RMP                            "RMPTtGeneral"
#define CP_AUTRESTOXIQUES_RMP                        "Autrestoxiques"
#define CP_GENCORRESPONDANT_RMP                      "Gencorresp"
#define CP_IDMG_RMP                                  "idCorMedMG"
#define CP_IDSPE1_RMP                                "idCorMedSpe1"
#define CP_IDSPE2_RMP                                "idCorMedSpe2"
#define CP_IDSPE3_RMP                                "idCorMedSpe3"
#define CP_IDCORNONMED_RMP                           "idCorNonMed"
#define CP_TABAC_RMP                                 "Tabac"


                            //! Table Rufus.Actes
#define CP_IDACTE_ACTES                              "idActe"
#define CP_IDPAT_ACTES                               "idPat"
#define CP_IDUSER_ACTES                              "idUser"
#define CP_IDSESSION_ACTES                           "idSession"    //!> pas utilisé
#define CP_MOTIF_ACTES                               "ActeMotif"
#define CP_TEXTE_ACTES                               "ActeTexte"
#define CP_CONCLUSION_ACTES                          "ActeConclusion"
#define CP_COURRIERAFAIRE_ACTES                      "ActeCourrierafaire"
#define CP_COTATION_ACTES                            "ActeCotation"
#define CP_MONTANT_ACTES                             "ActeMontant"
#define CP_MONNAIE_ACTES                             "ActeMonnaie"
#define CP_DATE_ACTES                                "ActeDate"
#define CP_HEURE_ACTES                               "ActeHeure"
#define CP_IDUSERCREATEUR_ACTES                      "CreePar"
#define CP_IDUSERCOMPTABLE_ACTES                     "UserComptable"
#define CP_IDUSERPARENT_ACTES                        "UserParent"
#define CP_SUPERVISEURREMPLACANT_ACTES               "SuperViseurRemplacant"
#define CP_NUMCENTRE_ACTES                           "NumCentre"
#define CP_IDLIEU_ACTES                              "idLieu"
#define CP_IMPORTANT_ACTES                           "PointImportant"
#define CP_TEXTIMPORTANT_ACTES                       "CommentPointImportant"
#define CP_NOCOTATION_ACTES                          "SansCotation"

                            //! Table Rufus.Courriers
#define CP_ID_IMPRESSIONS                            "idDocument"
#define CP_TEXTE_IMPRESSIONS                         "TextDocument"
#define CP_RESUME_IMPRESSIONS                        "ResumeDocument"
#define CP_CONCLUSION_IMPRESSIONS                    "ConclusionDocument"
#define CP_IDUSER_IMPRESSIONS                        "idUser"
#define CP_DOCPUBLIC_IMPRESSIONS                     "DocPublic"
#define CP_PRESCRIPTION_IMPRESSIONS                  "Prescription"
#define CP_EDITABLE_IMPRESSIONS                      "Editable"
#define CP_MEDICAL_IMPRESSIONS                       "Medical"

                            //! Table Rufus.flagss
#define CP_MAJFLAGSALDAT_FLAGS                       "MAJflagSalDat"
#define CP_MAJFLAGCORRESPONDANTS_FLAGS               "MAJflagMG"
#define CP_MAJFLAGMESSAGES_FLAGS                     "MAJflagMessages"
#define CP_MAJFLAGUSERDISTANT_FLAGS                  "flagUserDistant"

//! Table Rufus.Metadocuments
#define CP_ID_DOSSIERIMPRESSIONS                     "idMetaDocument"
#define CP_TEXTE_DOSSIERIMPRESSIONS                  "TextMetaDocument"
#define CP_RESUME_DOSSIERIMPRESSIONS                 "ResumeMetaDocument"
#define CP_IDUSER_DOSSIERIMPRESSIONS                 "idUser"
#define CP_PUBLIC_DOSSIERIMPRESSIONS                 "Public"

                            //! Table Rufus.jointuresdocuments
#define CP_ID_JOINTURESIMPRESSIONS                   "idJointure"
#define CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS       "idMetaDocument"
#define CP_IDDOCUMENT_JOINTURESIMPRESSIONS           "idDocument"

                            //! Table Rufus.Impressions
#define CP_ID_DOCSEXTERNES                            "idImpression"
#define CP_IDUSER_DOCSEXTERNES                        "idUser"
#define CP_IDPAT_DOCSEXTERNES                         "idPat"
#define CP_TYPEDOC_DOCSEXTERNES                       "TypeDoc"
#define CP_SOUSTYPEDOC_DOCSEXTERNES                   "SousTypeDoc"
#define CP_TITRE_DOCSEXTERNES                         "Titre"
#define CP_TEXTENTETE_DOCSEXTERNES                    "TextEntete"
#define CP_TEXTCORPS_DOCSEXTERNES                     "TextCorps"
#define CP_TEXTORIGINE_DOCSEXTERNES                   "TextOrigine"
#define CP_TEXTPIED_DOCSEXTERNES                      "TextPied"
#define CP_DATE_DOCSEXTERNES                          "DateImpression"
#define CP_PDF_DOCSEXTERNES                           "pdf"
#define CP_COMPRESSION_DOCSEXTERNES                   "Compression"
#define CP_JPG_DOCSEXTERNES                           "jpg"
#define CP_AUTRE_DOCSEXTERNES                         "autre"
#define CP_FORMATAUTRE_DOCSEXTERNES                   "formatautre"
#define CP_LIENFICHIER_DOCSEXTERNES                   "lienversfichier"
#define CP_LIENFICHIERDISTANT_DOCSEXTERNES            "LienFichierDistant"
#define CP_IDREFRACTION_DOCSEXTERNES                  "idRefraction"
#define CP_ALD_DOCSEXTERNES                           "ALD"
#define CP_IDEMETTEUR_DOCSEXTERNES                    "UserEmetteur"
#define CP_CONCLUSION_DOCSEXTERNES                    "Conclusion"
#define CP_EMISORRECU_DOCSEXTERNES                    "EmisRecu"
#define CP_FORMATDOC_DOCSEXTERNES                     "FormatDoc"
#define CP_IDLIEU_DOCSEXTERNES                        "idLieu"
#define CP_IMPORTANCE_DOCSEXTERNES                    "Importance"

                            //! Table Rufus.ParametresSysteme
#define CP_MDPADMIN_PARAMSYSTEME                      "MDPAdmin"
#define CP_NUMCENTRE_PARAMSYSTEME                     "NumCentre"
#define CP_IDLIEUPARDEFAUT_PARAMSYSTEME               "idLieuParDefaut"
#define CP_DOCSCOMPRIMES_PARAMSYSTEME                 "DocsComprimes"
#define CP_VERSIONBASE_PARAMSYSTEME                   "VersionBase"
#define CP_SANSCOMPTA_PARAMSYSTEME                    "SansCompta"
#define CP_ADRESSELOCALSERVEUR_PARAMSYSTEME           "AdresseServeurLocal"
#define CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME         "AdresseServeurDistant"
#define CP_DIRIMAGERIE_PARAMSYSTEME                   "DirImagerie"
#define CP_LUNDIBKUP_PARAMSYSTEME                     "LundiBkup"
#define CP_MARDIBKUP_PARAMSYSTEME                     "MardiBkup"
#define CP_MERCREDIBKUP_PARAMSYSTEME                  "MercrediBkup"
#define CP_JEUDIBKUP_PARAMSYSTEME                     "JeudiBkup"
#define CP_VENDREDIBKUP_PARAMSYSTEME                  "VendrediBkup"
#define CP_SAMEDIBKUP_PARAMSYSTEME                    "SamediBkup"
#define CP_DIMANCHEBKUP_PARAMSYSTEME                  "DimancheBkup"
#define CP_HEUREBKUP_PARAMSYSTEME                     "HeureBkup"
#define CP_DIRBKUP_PARAMSYSTEME                       "DirBkup"

                            //! Table Rufus.Messagerie
#define CP_TEXTMSG_MESSAGERIE                        "TexteMessage"

                            //! Table Rufus.salledattente
#define CP_IDPAT_SALDAT                              "idPat"
#define CP_IDUSER_SALDAT                             "idUser"
#define CP_STATUT_SALDAT                             "Statut"
#define CP_HEURESTATUT_SALDAT                        "HeureStatut"
#define CP_HEURERDV_SALDAT                           "HeureRDV"
#define CP_HEUREARRIVEE_SALDAT                       "HeureArrivee"
#define CP_MOTIF_SALDAT                              "Motif"
#define CP_MESSAGE_SALDAT                            "Message"
#define CP_IDACTEAPAYER_SALDAT                       "idActeAPayer"
#define CP_POSTEEXAMEN_SALDAT                        "PosteExamen"
#define CP_IDUSERENCOURSEXAM_SALDAT                  "idUserEnCoursExam"
#define CP_IDSALDAT_SALDAT                           "idSalDat"

                            //! Table Rufus.Utilisateurs connectes
#define CP_IDUSER_USRCONNECT                          "idUser"
#define CP_NOMPOSTE_USRCONNECT                        "NomPosteConnecte"
#define CP_MACADRESS_USRCONNECT                       "MACAdressePosteConnecte"
#define CP_IPADRESS_USRCONNECT                        "IPAdress"
#define CP_DISTANT_USRCONNECT                         "AccesDistant"
#define CP_IDUSERSUPERVISEUR_USRCONNECT               "UserSuperviseur"
#define CP_IDUSERCOMPTABLE_USRCONNECT                 "UserComptable"
#define CP_IDUSERPARENT_USRCONNECT                    "UserParent"
#define CP_IDLIEU_USRCONNECT                          "idLieu"
#define CP_HEUREDERNIERECONNECTION_USRCONNECT         "HeureDerniereConnexion"
#define CP_IDPATENCOURS_USRCONNECT                    "idPat"

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

                            //! Table ComptaMedicale.banques
#define CP_IDBANQUE_BANQUES                          "idBanque"
#define CP_NOMABREGE_BANQUES                         "idBanqueAbrege"
#define CP_NOMBANQUE_BANQUES                         "NomBanque"
#define CP_CODE_BANQUES                              "CodeBanque"

                            //! Table ComptaMedicale.Comptes
#define CP_IDCOMPTE_COMPTES                          "idCompte"
#define CP_IDBANQUE_COMPTES                          "idBanque"
#define CP_IDUSER_COMPTES                            "idUser"
#define CP_IBAN_COMPTES                              "IBAN"
#define CP_INTITULE_COMPTES                          "IntituleCompte"
#define CP_NOMABREGE_COMPTES                         "NomCompteAbrege"
#define CP_SOLDE_COMPTES                             "SoldeSurDernierReleve"
#define CP_PARTAGE_COMPTES                           "Partage"
#define CP_DESACTIVE_COMPTES                         "Desactive"

                            //! Table ComptaMedicale.Depenses
#define CP_IDDEPENSE_DEPENSES                       "idDep"
#define CP_IDUSER_DEPENSES                          "idUser"
#define CP_DATE_DEPENSES                            "DateDep"
#define CP_REFFISCALE_DEPENSES                      "RefFiscale"
#define CP_OBJET_DEPENSES                           "Objet"
#define CP_MONTANT_DEPENSES                         "Montant"
#define CP_FAMILLEFISCALE_DEPENSES                  "FamFiscale"
#define CP_MONNAIE_DEPENSES                         "Monnaie"
#define CP_IDRECETTE_DEPENSES                       "idRec"
#define CP_MODEPAIEMENT_DEPENSES                    "ModePaiement"
#define CP_COMPTE_DEPENSES                          "Compte"
#define CP_NUMCHEQUE_DEPENSES                       "Nocheque"
#define CP_IDFACTURE_DEPENSES                       "idFacture"

                            //! Table ComptaMedicale.Factures
#define CP_IDFACTURE_FACTURES                       "idFacture"
#define CP_DATEFACTURE_FACTURES                     "DateFacture"
#define CP_LIENFICHIER_FACTURES                     "LienFichier"
#define CP_INTITULE_FACTURES                        "Intitule"
#define CP_ECHEANCIER_FACTURES                      "Echeancier"
#define CP_IDDEPENSE_FACTURES                       "idDepense"
#define CP_PDF_FACTURES                             "pdf"
#define CP_JPG_FACTURES                             "jpg"

                            //! Table ComptaMedicale.lignesrecettes
#define CP_IDRECETTE_LIGNRECETTES                   "idRecette"
#define CP_IDUSER_LIGNRECETTES                      "idUser"
#define CP_DATE_LIGNRECETTES                        "DatePaiement"
#define CP_DATEENREGISTREMENT_LIGNRECETTES          "DateEnregistrement"
#define CP_MONTANT_LIGNRECETTES                     "Montant"
#define CP_MODEPAIEMENT_LIGNRECETTES                "ModePaiement"
#define CP_TIREURCHEQUE_LIGNRECETTES                "TireurCheque"
#define CP_IDCPTEVIREMENT_LIGNRECETTES              "CompteVirement"
#define CP_BANQUECHEQUE_LIGNRECETTES                "BanqueCheque"
#define CP_TIERSPAYANT_LIGNRECETTES                 "TiersPayant"
#define CP_NOMPAYEUR_LIGNRECETTES                   "NomTiers"
#define CP_COMMISSION_LIGNRECETTES                  "Commission"
#define CP_MONNAIE_LIGNRECETTES                     "Monnaie"
#define CP_IDREMISECHQ_LIGNRECETTES                 "idRemise"
#define CP_CHQENATTENTE_LIGNRECETTES                "EnAttente"
#define CP_IDUSERENREGISTREUR_LIGNRECETTES          "EnregistrePar"
#define CP_TYPERECETTE_LIGNRECETTES                 "TypeRecette"

                            //! Table Ophtalmologie.Refractions
#define CP_ID_REFRACTIONS                   "idRefraction"
#define CP_IDPAT_REFRACTIONS                "idPat"
#define CP_IDACTE_REFRACTIONS               "idActe"
#define CP_DATE_REFRACTIONS                 "DateRefraction"
#define CP_TYPEMESURE_REFRACTIONS           "QuelleMesure"
#define CP_DISTANCEMESURE_REFRACTIONS       "QuelleDistance"
#define CP_CYCLOPLEGIE_REFRACTIONS          "Cycloplegie"
#define CP_ODMESURE_REFRACTIONS             "ODcoche"
#define CP_SPHEREOD_REFRACTIONS             "SphereOD"
#define CP_CYLINDREOD_REFRACTIONS           "CylindreOD"
#define CP_AXECYLOD_REFRACTIONS             "AxeCylindreOD"
#define CP_AVLOD_REFRACTIONS                "AVLOD"
#define CP_ADDVPOD_REFRACTIONS              "AddVPOD"
#define CP_AVPOD_REFRACTIONS                "AVPOD"
#define CP_PRISMEOD_REFRACTIONS             "PrismeOD"
#define CP_BASEPRISMEOD_REFRACTIONS         "BasePrismeOD"
#define CP_BASEPRISMETEXTOD_REFRACTIONS     "BasePrismeTextOD"
#define CP_PRESSONOD_REFRACTIONS            "PressOnOD"
#define CP_DEPOLIOD_REFRACTIONS             "DepoliOD"
#define CP_PLANOD_REFRACTIONS               "PlanOD"
#define CP_RYSEROD_REFRACTIONS              "RyserOD"
#define CP_FORMULEOD_REFRACTIONS            "FormuleOD"
#define CP_OGMESURE_REFRACTIONS             "OGcoche"
#define CP_SPHEREOG_REFRACTIONS             "SphereOG"
#define CP_CYLINDREOG_REFRACTIONS           "CylindreOG"
#define CP_AXECYLOG_REFRACTIONS             "AxeCylindreOG"
#define CP_AVLOG_REFRACTIONS                "AVLOG"
#define CP_ADDVPOG_REFRACTIONS              "AddVPOG"
#define CP_AVPOG_REFRACTIONS                "AVPOG"
#define CP_PRISMEOG_REFRACTIONS             "PrismeOG"
#define CP_BASEPRISMEOG_REFRACTIONS         "BasePrismeOG"
#define CP_BASEPRISMETEXTOG_REFRACTIONS     "BasePrismeTextOG"
#define CP_PRESSONOG_REFRACTIONS            "PressOnOG"
#define CP_DEPOLIOG_REFRACTIONS             "DepoliOG"
#define CP_PLANOG_REFRACTIONS               "PlanOG"
#define CP_RYSEROG_REFRACTIONS              "RyserOG"
#define CP_FORMULEOG_REFRACTIONS            "FormuleOG"
#define CP_COMMENTAIREORDO_REFRACTIONS      "CommentaireOrdoLunettes"
#define CP_TYPEVERRES_REFRACTIONS           "QuelsVerres"
#define CP_OEIL_REFRACTIONS                 "QuelOeil"
#define CP_MONTURE_REFRACTIONS              "Monture"
#define CP_VERRETEINTE_REFRACTIONS          "VerreTeinte"
#define CP_PD_REFRACTIONS                   "PD"

                            //! Table Ophtalmologie.DonneesOphtaPatients
#define CP_ID_DATAOPHTA                     "idMesure"
#define CP_IDPATIENT_DATAOPHTA              "idPat"
#define CP_MESURE_DATAOPHTA                 "QuelleMesure"
#define CP_DISTANCE_DATAOPHTA               "QuelleDistance"
#define CP_K1OD_DATAOPHTA                   "K1OD"
#define CP_K2OD_DATAOPHTA                   "K2OD"
#define CP_AXEKOD_DATAOPHTA                 "AxeKOD"
#define CP_K1OG_DATAOPHTA                   "K1OG"
#define CP_K2OG_DATAOPHTA                   "K2OG"
#define CP_AXEKOG_DATAOPHTA                 "AxeKOG"
#define CP_MODEMESUREKERATO_DATAOPHTA       "OrigineK"
#define CP_DATEKERATO_DATAOPHTA             "DateK"
#define CP_SPHEREOD_DATAOPHTA               "SphereOD"
#define CP_CYLINDREOD_DATAOPHTA             "CylindreOD"
#define CP_AXECYLINDREOD_DATAOPHTA          "AxeCylindreOD"
#define CP_AVLOD_DATAOPHTA                  "AVLOD"
#define CP_ADDVPOD_DATAOPHTA                "AddVPOD"
#define CP_AVPOD_DATAOPHTA                  "AVPOD"
#define CP_DATEREFRACTIONOD_DATAOPHTA       "DateRefOD"
#define CP_SPHEREOG_DATAOPHTA               "SphereOG"
#define CP_CYLINDREOG_DATAOPHTA             "CylindreOG"
#define CP_AXECYLINDREOG_DATAOPHTA          "AxeCylindreOG"
#define CP_AVLOG_DATAOPHTA                  "AVLOG"
#define CP_ADDVPOG_DATAOPHTA                "AddVPOG"
#define CP_AVPOG_DATAOPHTA                  "AVPOG"
#define CP_DATEREFRACTIONOG_DATAOPHTA       "DateRefOG"
#define CP_ECARTIP_DATAOPHTA                "PD"
#define CP_DIOTRIESK1OD_DATAOPHTA           "DioptrieK1OD"
#define CP_DIOTRIESK2OD_DATAOPHTA           "DioptrieK2OD"
#define CP_DIOTRIESK1OG_DATAOPHTA           "DioptrieK1OG"
#define CP_DIOTRIESK2OG_DATAOPHTA           "DioptrieK2OG"

// Divers
#define PDF                                         "pdf"
#define JPG                                         "jpg"

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

#define NOM_APPLI                                   "Rufus"

#define NOM_POSTEIMPORTDOCS                         "posteimportdocs"
#define NOM_ADMINISTRATEURDOCS                      "Admin"
#define NOM_MDPADMINISTRATEUR                       "bob"

//les TCP
#define NOM_PORT_TCPSERVEUR                         "50885"
#define TCPMSG_idUser                               "idUser"                                            /*! nom du message fournissant l'idUser */
#define TCPMSG_MAJCorrespondants                    "MajCorrespondants"                                 /*! fin du message invitant à mettre à jour la liste des correspondants */
#define TCPMSG_MAJDocsExternes                      "MajDocsExternes"                                   /*! fin du message invitant à mettre à jour la liste des documents externes pour un patient */
#define TCPMSG_MAJSalAttente                        "MajSalAttente"                                     /*! fin du message invitant à mettre à jour la salle d'attente */
#define TCPMSG_MAJPatient                           "MajPatient"                                        /*! fin du message indiquant qu'un patient a été créé ou modifié */
#define TCPMSG_MAJListeUsers                        "MajListeUsers"                                     /*! fin du message invitant à mettre à jour la liste des utilisateurs */
#define TCPMSG_MsgBAL                               "MsgRecuDansBAL"                                    /*! fin du message informant de la réception d'un message de messagerie instantanée */
#define TCPMSG_Separator                            "triumphspeedtriple1050"                            /*! separateur au sein d'un message */
#define TCPMSG_DataSocket                           "DataSocket"                                        /*! fin du message fournissant IP et MAC et nom d'un client */
#define TCPMSG_ListeSockets                         "ListeSockets"                                      /*! fin du message informant le client que ce message contient la liste des TcpSockets */
#define TCPMSG_EnvoieListSocket                     "EnvoieListe"                                       /*! fin du message demandant l'envoi de la liste des sockets à un client */
#define TCPMSG_Disconnect                           "Deconnexion"                                       /*! fin du message demandant la deconnexion et la fermeture d'un socket */
#define TCPMSG_TestConnexion                        "TestConnexion"                                     /*! fin du message de test de la connexion */
#define TCPMSG_OKConnexion                          "OKConnexion"                                       /*! fin du message de confirmation de la connexion */

// Constantes de statut salle d'attente  //+++++ATTENTION ces constantes servent dans des requêtes SQL - NE PAS METTRE DE CARACTERES SPECIAUX
#define ARRIVE                                      "Arrivé"
#define ENCOURS                                     "En cours"
#define ENATTENTENOUVELEXAMEN                       "En attente de nouvel examen par "
#define RETOURACCUEIL                               "Retour accueil"
#define ENCOURSEXAMEN                               "En cours de consultation avec "

// Les fichiers et répertoires divers
#define DIR_LIBS2                                "/Applications";
#define DIR_RESSOURCES                           "/Ressources"
#define DIR_RUFUS                                "/Documents/Rufus"
#define DIR_FACTURES                             "/Factures"
#define DIR_ORIGINAUX                            "/Originaux"
#define DIR_FACTURESSANSLIEN                     "/FacturesSansLien"
#define DIR_DOSSIERECHANGE                       "/DossierEchange"
#define DIR_CRDOSSIERS                           "/CRDossiers"
#define DIR_IMAGERIE                             "/Imagerie"
#define DIR_IMAGES                               "/Images"
#define DIR_VIDEOS                               "/Videos"
#define DIR_PROV                                 "/Prov"
#define DIR_LOGS                                 "/Logs"
#define DIR_ECHECSTRANSFERTS                     "/EchecsTransferts"

#define FILE_ENTETEORDO                           "/Documents/Rufus/Ressources/Entete_Ordonnance.txt"
#define FILE_CORPSORDO                            "/Documents/Rufus/Ressources/Corps_Ordonnance.txt"
#define FILE_ENTETEORDOALD                        "/Documents/Rufus/Ressources/Entete_OrdoALD.txt"
#define FILE_CORPSORDOALD                         "/Documents/Rufus/Ressources/Corps_OrdoALD.txt"
#define FILE_PIEDPAGE                             "/Documents/Rufus/Ressources/Pied_Ordonnance.txt"
#define FILE_PIEDORDOLUNETTES                     "/Documents/Rufus/Ressources/Pied_Ordonnance_Lunettes.txt"
#define FILE_PDF                                  "/Documents/Rufus/Ressources/pdf.pdf"
#define FILE_TEST                                 "/Documents/Rufus/Ressources/test.txt"
#define FILE_INI                                  "/Documents/Rufus/Rufus.ini"
#define SCRIPTBACKUPFILE                          "/Documents/Rufus/RufusScriptBackup.sh"
#define SCRIPTRESTOREFILE                         "/Documents/Rufus/RufusScriptRestore.sh"
#define SCRIPT_MACOS_PLIST_FILE                   "/Library/LaunchAgents/rufus.bup.plist"

// Variables mysql
#define MAX_ALLOWED_PACKET                          "16"

// le son d'alarme
#define NOM_ALARME                                  "://goutte.wav"

// la largeur de la formule de réfraction dans le champ ActeTexte
#define LARGEUR_FORMULE                             "310"

// la couleur des titres
#define COULEUR_TITRES                              "blue"

// le lien vers la CCAM
#define LIEN_CCAM                                   "https://www.ameli.fr/accueil-de-la-ccam/trouver-un-acte/consultation-par-chapitre.php?chap=a%3A0%3A%7B%7D&add=2#chapitre_2"

// la taille maximale des images
#define TAILLEMAXIIMAGES                            262144

// la police d'écran par défaut et le facteur de correction des td width html en impression qui varie entre Linux et OSX (???)
#define POLICEPARDEFAUT         "Comic Sans MS"
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

// ancres et comentaires html
#define HTMLCOMMENT_LINUX           "<!LINUX>"
#define HTMLCOMMENT_MAC             "<!MAC>"
#define HTMLANCHOR_BODEBUT          "<BOdebut>"
#define HTMLANCHOR_BOFIN            "<BOfin>"

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


#endif // MACROS_H
