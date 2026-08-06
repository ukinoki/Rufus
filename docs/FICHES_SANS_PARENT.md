# Recensement des fenêtres ouvertes sans parent

Une fiche ou une boîte sans parent devient une fenêtre de premier niveau : le gestionnaire
de fenêtres ne garantit plus qu'elle passe devant celle qui l'a ouverte. Le défaut ne se voit
que lorsqu'une fiche est déjà affichée — d'où son apparition au démarrage, où les fiches
s'empilent (VerifierIni, InitialisationBaseEtDossiers, installeur MySQL).

Le parent n'est légitimement absent que si aucune fenêtre n'est ouverte à ce moment.

État au jour du recensement, hors `build/` et `archive/`.

**106 occurrences dans 27 fichiers.**

| Type | Nombre |
|---|---|
| Watch | 70 |
| new UpDialog | 10 |
| UpMessageBox locale | 8 |
| fiche dlg_* | 7 |
| Information | 5 |
| Question | 5 |
| new UpMessageBox | 1 |

## procedures.cpp — 50

| Ligne | Type | Fonction |
|---|---|---|
| 241 | Watch | `Procedures::ab()` |
| 939 | Information | `Procedures::setDirSQLExecutable()` |
| 951 | Question | `Procedures::setDirSQLExecutable()` |
| 994 | Information | `Procedures::setDirSSLKeys()` |
| 1005 | Question | `Procedures::setDirSSLKeys()` |
| 1402 | new UpDialog | `Procedures::ResolutionRendu()` |
| 1489 | Watch | `Procedures::ResolutionRendu()` |
| 1580 | Watch | `Procedures::Cree_pdffile()` |
| 1827 | new UpDialog | `Procedures::EditHtml()` |
| 2388 | Information | `Procedures::ReinitBase()` |
| 3093 | UpMessageBox locale | `Procedures::MettreAJourSocleMySQL()` |
| 3114 | Watch | `Procedures::MettreAJourSocleMySQL()` |
| 3123 | Watch | `Procedures::MettreAJourSocleMySQL()` |
| 3343 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3371 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3374 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3410 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3428 | UpMessageBox locale | `Procedures::Connexion_A_La_Base()` |
| 3462 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3467 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3476 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3489 | UpMessageBox locale | `Procedures::Connexion_A_La_Base()` |
| 3502 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3545 | Question | `Procedures::Connexion_A_La_Base()` |
| 3565 | Question | `Procedures::Connexion_A_La_Base()` |
| 3581 | Question | `Procedures::Connexion_A_La_Base()` |
| 3607 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3610 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3635 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3663 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3690 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3708 | Watch | `Procedures::Connexion_A_La_Base()` |
| 3749 | Watch | `Procedures::ControleSocleMySQLApresAffichage()` |
| 3784 | new UpDialog | `Procedures::CalcLieuExercice()` |
| 3895 | Watch | `Procedures::VerifnumAM()` |
| 3899 | new UpDialog | `Procedures::VerifnumAM()` |
| 4179 | fiche dlg_* | `Procedures::IdentificationUser()` |
| 4265 | Watch | `Procedures::IdentificationUser()` |
| 4368 | Watch | `Procedures::IdentificationUser()` |
| 4378 | Watch | `Procedures::IdentificationUser()` |
| 4458 | new UpDialog | `Procedures::DefinitRoleUser()` |
| 4545 | Watch | `Procedures::DefinitRoleUser()` |
| 4607 | Watch | `Procedures::DefinitRoleUser()` |
| 4648 | new UpDialog | `Procedures::DefinitRoleUser()` |
| 4692 | Watch | `Procedures::DefinitRoleUser()` |
| 4720 | Watch | `Procedures::DefinitRoleUser()` |
| 5359 | UpMessageBox locale | `Procedures::VerifierIni()` |
| 5406 | new UpMessageBox | `Procedures::CreerOuRestaurerBase()` |
| 5422 | Watch | `Procedures::CreerOuRestaurerBase()` |
| 5540 | Watch | `Procedures::Ouverture_Appareils_Refraction()` |

## MySQLInstaller/mysqlinstaller.cpp — 10

| Ligne | Type | Fonction |
|---|---|---|
| 1255 | Watch | `inviterANoterMotDePasse()` |
| 1291 | Watch | `avertirSecurisationMiseEnPlace()` |
| 1308 | UpMessageBox locale | `confirmerSuppressionGaxt78iy()` |
| 1329 | Watch | `avertirSuppressionGaxt78iyEffectuee()` |
| 1861 | UpMessageBox locale | `MySQLInstaller::controlerClesSSLMonoposte()` |
| 2275 | UpMessageBox locale | `MySQLInstaller::verifierEtReparerConfigMonoposte()` |
| 2753 | UpMessageBox locale | `MySQLInstaller::proposerRecuperationAleatoire()` |
| 2786 | Information | `MySQLInstaller::proposerRecuperationAleatoire()` |
| 2940 | Information | `MySQLInstaller::suggererSecurisationDepuisLocal()` |
| 3035 | Watch | `MySQLInstaller::supprimerGaxt78iySiEchue()` |

## rufus.cpp — 7

| Ligne | Type | Fonction |
|---|---|---|
| 1522 | fiche dlg_* | `Rufus::BilanRecettes()` |
| 2225 | Watch | `Rufus::ExporteDocs()` |
| 2522 | Watch | `Rufus::ExporteDocs()` |
| 2684 | Watch | `Rufus::ExporteDocs()` |
| 3281 | fiche dlg_* | `Rufus::ListeCorrespondants()` |
| 3304 | fiche dlg_* | `Rufus::ListeManufacturers()` |
| 3396 | new UpDialog | `Rufus::RechercheParID()` |

## Database/database.cpp — 4

| Ligne | Type | Fonction |
|---|---|---|
| 91 | Watch | `DataBase::InfosConnexionSQL()` |
| 126 | Watch | `DataBase::erreurRequete()` |
| 129 | Watch | `DataBase::erreurRequete()` |
| 3325 | Watch | `DataBase::EnregistreAutreVille()` |

## ItemsLists/cls_messages.cpp — 4

| Ligne | Type | Fonction |
|---|---|---|
| 127 | Watch | `Messages::CreationMessage()` |
| 133 | Watch | `Messages::CreationMessage()` |
| 152 | Watch | `Messages::UpdateMessage()` |
| 171 | Watch | `Messages::EnregistreDestinataires()` |

## conversionbase.cpp — 4

| Ligne | Type | Fonction |
|---|---|---|
| 45 | Watch | `conversionbase::conversionbaseophtalogic()` |
| 594 | Watch | `conversionbase::conversionbaseoplus()` |
| 906 | Watch | `conversionbase::conversionbaseoplus()` |
| 954 | Watch | `conversionbase::conversionbaseoplus()` |

## ItemsLists/cls_interventions.cpp — 3

| Ligne | Type | Fonction |
|---|---|---|
| 101 | Watch | `Interventions::CreationIntervention()` |
| 200 | Watch | `SessionsOperatoires::CreationSessionOperatoire()` |
| 309 | Watch | `TypesInterventions::CreationTypeIntervention()` |

## Dialogs/dlg_docsexternes.cpp — 2

| Ligne | Type | Fonction |
|---|---|---|
| 724 | new UpDialog | `dlg_docsexternes::ModifierDate()` |
| 763 | new UpDialog | `dlg_docsexternes::ModifierItem()` |

## Dialogs/dlg_identificationuser.cpp — 2

| Ligne | Type | Fonction |
|---|---|---|
| 127 | Watch | `dlg_identificationuser::ControleDonnees()` |
| 143 | Watch | `dlg_identificationuser::ControleDonnees()` |

## ItemsLists/cls_impressions.cpp — 2

| Ligne | Type | Fonction |
|---|---|---|
| 91 | Watch | `Impressions::CreationImpression()` |
| 203 | Watch | `DossiersImpressions::CreationDossierImpression()` |

## ItemsLists/cls_itemslist.cpp — 2

| Ligne | Type | Fonction |
|---|---|---|
| 1579 | Watch | `ItemsList::update()` |
| 1628 | Watch | `ItemsList::updateBlob()` |

## Dialogs/dlg_depenses.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 1101 | fiche dlg_* | `dlg_depenses::ZoomDoc()` |

## Dialogs/dlg_docsvideo.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 34 | fiche dlg_* | `lg_docsvideo::dlg_docsvideo()` |

## Dialogs/dlg_gestionusers.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 399 | Watch | `dlg_gestionusers::changeLogo()` |

## ImageViewer/dlg_multiimageviewer.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 941 | fiche dlg_* | `dlg_multiimageviewer::ZoomDoc()` |

## ItemsLists/cls_commentslunets.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 86 | Watch | `CommentsLunets::CreationCommentLunet()` |

## ItemsLists/cls_commercials.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 91 | Watch | `Commercials::CreationCommercial()` |

## ItemsLists/cls_docsexternes.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 151 | Watch | `DocsExternes::CreationDocumentExterne()` |

## ItemsLists/cls_manufacturers.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 88 | Watch | `Manufacturers::CreationManufacturer()` |

## ItemsLists/cls_motscles.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 101 | Watch | `MotsCles::CreationMotCle()` |

## ItemsLists/cls_patients.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 255 | Watch | `Patients::CreationPatient()` |

## ItemsLists/cls_refractions.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 78 | Watch | `Refractions::CreationRefraction()` |

## ItemsLists/cls_sessions.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 72 | Watch | `Sessions::CreationSession()` |

## ItemsLists/cls_sites.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 119 | Watch | `Sites::CreationSite()` |

## ItemsLists/cls_tierspayants.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 80 | Watch | `TiersPayants::CreationTiers()` |

## TextPrinter/textprinter.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 103 | Watch | `TextPrinter::print()` |

## utils.cpp — 1

| Ligne | Type | Fonction |
|---|---|---|
| 1666 | new UpDialog | `Utils::AfficheImage()` |
