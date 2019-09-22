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


#ifndef IMPORTDOCSEXTERNESTHREAD_H
#define IMPORTDOCSEXTERNESTHREAD_H

#include <QThread>
#include "procedures.h"
#include "utils.h"


/* Cette classe tourne en tache de fond et importe les documents d'imagerie dans la base de données
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/
/*!
 * \brief The ImportDocsExternesThread class
 * A. Les différents types de documents
        * Tous les documents sont stockés ou indexés dans la table Impressions
        * Il existe 2 types de documents.
            1. Les documents émis par le poste utilisateur:
                • ordonnances,
                • certificats
                • courriers destinés aux correspondants correspondants..etc...
                Ces documents sont élaborés par les fiches
                    * dlg_refraction pour les prescriptions de verres correcteurs
                    * dlg_bilanortho pour les bilans orthoptiques
                    * dlg_impressions pour tous les autres documents
                Ces documents sont stockés au format html
                Les documents émis par le poste utilisateur sont stockés intégralement dans la table Impressions Ils sont stockés sur 4 champs:
                    * TextEntete    -> l’entête du document tel qu’il a été émis, au format html
                    * TextCorps     -> le corps du document tel qu’il a été émis, au format html
                    * TextPied      -> le pied du document tel qu’il a été émis, au format html
                    * textOrigine   -> le corps du document, disponible pour pouvoir être repris et modifié, au format html
                Les 3 premiers champs permettent d’avoir une photo du document, moins lourde à stocker qu’un pdf, pour pouvoir remettre une copie conforme du document.
                le 4ème champ permet de ne reprendre que le contenu du document pour pouvoir en réimprimer une autre exemplaire en en modifiant le contenu au besoin, date, rajout de ligne à l’ordonnance...etc...
            2. Les documents d’imagerie
                • tous les documents émis par les appareils d’imagerie, y compris les videos
                • les documents scannés pour être incorporés dans la base
                Ne sont pas stockés dans la table impressions mais sur le disque dur du serveur.
                le champ lienversfichier de la table Impressions indique le lien vers le fichier d’imagerie sur le disque dur du serveur

 * B. La structure de la table Impressions
    * idImpression  -> l’id de l’impression
    * idUser        -> l’id du user connecté sur le poste qui a intégré le document dans la base (100 pour RufusAdmin par exemple)
    * idPat         -> l’id du patient concerné
    * TypeDoc       -> le type de document
        * Prescription
        * Courrier (les certificats sont considérés comme des courriers)
        * Type d’Imagerie : RNM, CV, OCT, Biométrie...etc...
    * SousTypeDoc   -> plus de précisions sur le document
    * Titre
        * dans le cas d’un OCT, précise le type d’OCT, l’oeil concerné...etc...
        * pour un courrier ou une prescription, correspond
            * Titre de la prescription ou du courrier tel qu’il est inscrit dans la fiche dlg_impressions si pe document est émis par la diche dlg_impressions
            * Correction s’il s’agit de verres correcteurs
            * sinon, la fusion des 2 champs précédents
    * TextEntete        -> ne sert pas pour les documents d’imagerie
    * TextCorps         -> ne sert pas pour les documents d’imagerie
    * TextPied          -> ne sert pas pour les documents d’imagerie
    * textOrigine       -> ne sert pas pour les documents d’imagerie
    * DateImpression    ->  la date d'émission du document
    * pdf               -> s'il s'agit d'un document d'imagerie au format pdf, le blob de ce document - il s'agit d'un stockage provisoire des documents émis par les postes distants cf + bas
    * jpg               -> s'il s'agit d'un docmuent d'imagerie au format jpg, le blob de ce document - il s'agit d'un stockage provisoire des documents émis par les postes distants cf + bas
    * Compression       -> utilisé pour signifier qu'un pdf est compressé - pas utilisé en fait
    * formatautre       -> format du document stocké dans le champ autre
    * autre             -> utilisé en blob pour les formats autres que pdf ou jpg
    * lienversfichier   -> lien vers l’adresse du fichier, relatif à l’adresse du dossier de stockage sur le serveur référencée dans le champ DirImagerie de la table ParametresSysteme
                            * si l’adresse DirImagerie est /home/rodolphe/Documents/Rufus/Imagerie
                            * et que lienversfichier est /2018-06-01/32998_RNM_Eidon OG_20180601-153127-44328.jpg
                            * le lien résultant pour trouver le fichier sera la concaténation des 2
    * LienFichierDistant-> pas utilisé
    * idRefraction      -> id de la réfraction dans la table réfractions pour les prescriptions de lunettes
    * ALD               -> le document est une prescription en ALD
    * UserEmetteur      -> l'id du user qui a émis le document pour les documents émis par le poste
    * Conclusion        -> pas utilisé
    * EmisRecu          -> 0 si document émis, 1 si document reçu (scanné)
    * FormatDoc
        * Prescription de lunettes = PrescrLun
        * Prescription medicale = Prescription
        * Courrier emis = Courrier
        * Document scan = DocRecu
        * Document imagerie = Imagerie
        * Bilan orthoptique = BO
    * idLieu            -> Endroit où le document a été élaboré ou reçu

 * C. Les documents émis par le poste utilisateur
    Ils sont émis
        * soit à partir de la fiche dlg_documents
        * soit à partir de la fiche dlg_refraction pour les prescriptions de lunettes.
    Les 2 fiches élaborent
        * un string html qui correspond au corps du documents
        * un string html entête
        * un string html pied de page
        à partir des modèles de documents situés dans le sous-dossier Ressources du dossier Rufus.
    Ces trois Qstring en html sont envoyés à la classe textprinter.h qui génère un pdf qu’elle imprime.
    Les 3 QString sont ensuite stockés dans la table impressions avec les autres renseignements: idpatient, iduser, idlieu, pres- cription ou pas...etc...

 * D. Les documents d’imagerie
    Le fonctionnement est plus complexe.
    * a - les documents d’imagerie émis sur le réseau local
        Les documents d’imagerie sont émis par les appareils d’imagerie sur un dossier du réseau local qui leur est propre.
        Pour éviter les conflits, une seule instance de Rufus sur le réseau local s’occupe d’importer ces documents dans la base.
        Quand RufusAdmin est utilisé, il prend le pas sur les autres postes.
        Sinon, c’est un poste Rufus qui s’en occupe suivant les choix faits dans la partie CePoste de la fiche Edition/Paramètres.
            * Un poste peut être paramètré comme
                * non importateur de document
                * importateur
                * importateur prioritaire.
            La procédure SQL PosteImports détermine quel est le poste importateur.
            La fonction setPostImports de procédures.h s’occupe de rédiger la procédure SQL qui détermine le poste importateur.
            Un système de timer vérifie que le poste importateur ne s’est pas déconnecté auquel cas un autre poste prend sa place.
        Sur le poste responsable de l’import des documents, la classe importdocsexternes parcourt à intervalles réguliers tous les dossiers où sont émis les documents d’imagerie pour les incorporer dans la base.
        Quand un document est trouvé dans un dossier, il est intégré dans la base comme les prescriptions mais le fichier est recopié dans un sous dossier du dossier référencé par DirImagerie.
        Ce sous dossier est nommé à la date du jour et le fichier est renommé avec la convention
            /Images/DateduJour/idPatient_TypeDoc_SousTypeDoc_Date et heure création du doc_IdImpression. (jpg ou pdf)
            /Videos/DateduJour/idPatient_TypeDoc_SousTypeDoc_Date et heure création du doc_IdImpression. (pour les videos)
        Si le poste responsable de l’import n’est pas le serveur mais est sur le réseau local,
            * il utilise l’adresse relative stockée dans son rufus.ini pour retrouver le chemin de DirImagerie
            * cette adresse est affichée dans Editions/Paramètres/Ce Poste/Réseau Local - ligne dossier de stockage des documents.
        S'il est le serveur, il utilise DirImagerie bien sûr.
    Pour la lecture des documents sur un poste Rufus, c’est la classe dlg_docsexternes.h qui s’en occupe.
        * Quand il s’agit d’un document d’imagerie, la fonction CalcImage va réimporter dans la table EchangeImages de la base Imagerie le fichier voulu
        * en stockant le fichier avec son idImpression dans le champ blocb pdf ou jpg de  cette table
        * Ceci permet de faire voyager l’imagerie dans le tuyau SQL sans avoir à utiliser un serveur Apache ou d’autres clés de cryptage.
        * Puis la classe dlg_docsexternes va lire le champ directement dans la table pour l’afficher.
        * 1 minute avant chaque sauvegarde de la base, la table EchangesImages est purgée.

    * b - les documents d’imagerie émis à distance
        Le fonctionnement est légèrement différent
        Quand un document est émis par un appareil d'imagerie sur un site distant du serveur, sur un dossier d'échange
            * le poste importateur des documents sur ce site distant va incorporer le document dans la table impressions
            * sous forme de blob dans les champs blobs jpg ou pdf suivant la nature de ce document.
            * Sur le réseau local du serveur, le poste responsable de l’import des documents scrute les champs jpg et pdf de la table Impressions.
            * Quand un de ces 2 champs n’est pas vide, le poste va extraire le pdf ou le jpg et recréer le fichier pour le stocker sur le dsiquen dur du serveur avec les conventions de nom vues plus haut
            * puis purger le champ pdf ou jpg de la table Impressions et enregistrer le lien vers le fichier dans le champ lienversfichier.

 * E. La suppression des documents
        Se fait à partir de l’icône poubelle de la fiche dlg_docexternes.
        L’enregistrement est retiré de la base et le lien vers le fichier est enregistré dans la table Images.DocsASupptimer
        Un timer sur le poste importateur scrute cette table et élimine le fichier.


Comment ça marche

I - ENREGISTREMENT D'UN FICHIER

 A. L'enregistrement d'un fichier pdf ou jpg
    * a - Sur le réseau local du serveur
        les documents d'imagerie sont émis par chaque appareil d'imagerie sur un dossier qui leur est spécifique, sur le réseau local.
        une instance de Rufus et une seule va incorporer ce fichier dans la base
        les jpg sont comprimés jusqu'à une taille maximale (256Ko) définie dans macros.h avant d'être incorporés dans la base
            * le fichier est renommé et copié dans le sous-dossier Images du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie
            * un enregistrement correspondant à ce fichier est créé dans la table Impressions et le lien vers le fichier vu depuis le serveur est enregistré dans cet enregistrement
            * une copie du fichier d'origine est effectuée dans le sous-dossier Originaux du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie
        le poste responsable de l'import est déterminé par un ordre de priorité. Si RufusAdmin est utilisé, ce sera lui qui importera.
        Sur tous les postes, un timer vérifie chaque minute que le poste désigné par la variable SQL PosteImportsDocs est toujours connecté.
        En cas de connexion, un autre poste prend la place en fonction de l'ordre de priorité RufusAdmin->RufusPrioritaire->Rufusnonprioritaire
        Pour une instance rufus, on choisit si elle est prioritaire dans la fiche dlg_param, onglet Ce Poste - le choix dépend du psote et pas de l'utilisateur et est enregistré dans rufus.ini
        Un poste distant ne peut pas importer les données.
    * b - à distance
        les documents d'imagerie sont émis par chaque appareil d'imagerie sur un dossier qui leur est spécifique, sur le réseau local distant.
        les jpg sont comprimés jusqu'à une taille maximale (256Ko) définie dans macros.h avant d'être incorporés dans la base
        un poste va importer dans la base le fichier d'imagerie en créant l'enregistrement dans la table Impresions et en enregistrant le contenu du fichier dans le champ jpg ou pdf de cet enregistrement.
        une copie du fichier d'origine est effectuée dans le sous-dossier Originaux du dossier d'imagerie du poste, tel qu'il est défini dans le rufus.ini de ce poste
        le poste importateur sur le réseau local du serveur va extraire ce fichier et le recréer dans le sous-dossier Images du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie

 B. L'enregistrement d'une video
        ne peut se faire que sur le réseau local
        les instances de Rufus qui enregistrent le fichier video le font directement
            * le poste importateur  n'intervient pas
            * le fichier est renommé et copié dans le sous-dossier Videos du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie
            * un enregistrement correspondant à ce fichier est créé dans la table Impressions et le lien vers le fichier vu depuis le serveur est enregistré dans cet enregistrement
        Un poste distant ne peut pas importer de video.

 B. L'enregistrement d'un document scanné
    * a - Sur le réseau local du serveur
        les instances de Rufus qui enregistrent le fichier scanné le font directement
            * le poste importateur  n'intervient pas
            * le fichier est renommé et copié dans le sous-dossier Images du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie
            * un enregistrement correspondant à ce fichier est créé dans la table Impressions et le lien vers le fichier vu depuis le serveur est enregistré dans cet enregistrement
    * b - à distance
        le fonctionnement est le même que pour les fichiers d'imagerie
        les documents scannés sont importés dans la base par le poste qui les a enregistrés en créant l'enregistrement dans la table Impresions et en enregistrant le contenu du fichier dans le champ jpg ou pdf de cet enregistrement.
        le poste importateur sur le réseau local du serveur va extraire ce fichier et le recréer dans le sous-dossier Images du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie

 B. L'enregistrement d'une facture
    fonctionne comme pour les documents scannés mais c'est le sous-dossier Factures qui est utilisé et la table Factures
    * a - Sur le réseau local du serveur
        les instances de Rufus qui enregistrent le fichier scanné le font directement
            * le poste importateur  n'intervient pas
            * le fichier est renommé et copié dans le sous-dossier Factures du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie
            * pour chaque utilisateur, un sous-dossier du nom de l'utilisateur est créé dans le sous-dosseir Factures et c'est dans ce sous-dossier que sont enregistrées les factures
            * un enregistrement correspondant à ce fichier est créé dans la table Factures et le lien vers le fichier vu depuis le serveur est enregistré dans cet enregistrement
    * b - à distance
        le fonctionnement est le même que pour les fichiers d'imagerie
        les documents scannés sont importés dans la base par le poste qui les a enregistrés en créant l'enregistrement dans la table Factures et en enregistrant le contenu du fichier dans le champ jpg ou pdf de cet enregistrement.
        le poste importateur sur le réseau local du serveur va extraire ce fichier et le recréer dans le sous-dossier Facturess du dossier d'imagerie tel qu'il est défini dans la table parametressysteme champ Dirimagerie



II - LECTURE D'UN FICHIER

 A. La lecture d'un fichier pdf ou jpg (image, document scanné ou facture)
    peut se faire de n'importe quel emplacement
    pour ne pas être transmis en clair sur le réseau, les fichiers sont réincorporés dans la base de données avant d'être lus par le poste qui veut les visionner
    Quand un poste veut lire un fichier image, il crée une instruction SQL pour recopier le contenu du fichier dans la table EchangeImages de la base Images et il lit ensuite cette image
    Avant chaque sauvegarde, cette table est vidée de son contenu
 B. La lecture d'une video
    ne se fait que sur le réseau local
    le poste qui veut lire une video le fait directement sur le disque dur du serveur à partir de son emplacement enregistré dans la table Impressions

II - SUPPRESSION D'UN FICHIER

 A. Suppression d'un fichier pdf ou jpg (image, document scanné ou facture)
 B. Suppression d'une video

 */



class ImportDocsExternesThread : public QObject
{
    Q_OBJECT
public:
    explicit ImportDocsExternesThread();
    void                        RapatrieDocumentsThread(QList<QVariantList> listdocs);
    enum Acces                  {Local, Distant};   Q_ENUM(Acces)

signals:
    void                        emitmsg(QStringList m_listemessages, int pause, bool bottom);

private:
    Procedures                  *proc               = Procedures::I();
    DataBase                    *db                 = DataBase::I();
    int                         m_idlieuexercice    = Datas::I()->users->userconnected()->idsitedetravail();
    bool                        m_encours           = false;

    bool                        m_compressiondocs;
    bool                        m_ok;
    QString                     m_pathdirstockageprovisoire;
    QString                     m_pathdirstockageimagerie;
    QString                     m_pathdirOKtransfer;
    QString                     m_pathdiroriginOKtransfer;
    QString                     m_pathdirechectransfer;
    QString                     m_datetransfer;
    QStringList                 m_listemessages;
    QFile                       file_image, file_origine;

    QThread                     m_thread;
    Acces                       m_acces;

    bool                        DefinitDossiers();
    void                        EchecImport(QString txt);
};

#endif // IMPORTDOCSEXTERNESTHREAD_H
