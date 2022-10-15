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

#include "pyxinterf.h"

#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include <QMainWindow>
#include <QApplication>

// ---------------------------------------------------------------------------------------------------
pyxinterf::pyxinterf(QObject *parent) :
    QObject(parent)
{
}
// ---------------------------------------------------------------------------------------------------
// Lecture d'une CPS
// - ouvre la connexion avec Pyxvital
// - commande la lecture de la CPS à Pyxvital
// - retourne le nom du fichier Praticien.par
// ---------------------------------------------------------------------------------------------------
QString pyxinterf::Lecture_CPS()
{
    InitVariables();
    if (!InitConnexionPyxvital())
        return ("");
    appelPyxvital("P", SW_HIDE, "Lecture de la CPS");	// (P) lire la carte PS et alimenter Praticien.par.
    FermeTout();
    return gnomFicPraticien; // A REVOIR TESTER LECTURE OK
}
// ---------------------------------------------------------------------------------------------------
// Lecture d'une CV
// - ouvre la connexion avec Pyxvital
// - commande la lecture de la CV à Pyxvital
// - retourne le nom du fichier Patient.par
// ---------------------------------------------------------------------------------------------------
QString pyxinterf::Lecture_CV()
{
    InitVariables();
    if (!InitConnexionPyxvital())
        return ("");
    appelPyxvital("V", SW_HIDE, "Lecture de la CV");	// (P) lire la carte Vitale et alimenter Patient.par.
    FermeTout();
    return gnomFicPatient;
}

//-------------------------------- On ferme avant de partir --------------------------------------------
void pyxinterf::FermeTout()
{
#ifdef Q_OS_WIN_PYXVITAL
    FreeLibrary(hinstDLL);
#endif
    if (g_tcpSocket != Q_NULLPTR) g_tcpSocket->close();
}

//--------------------------------- InitVariables ----------------------------------------------------
void pyxinterf::InitVariables()
{
    strcpy(CRLF,"\r\n.\r\n");       // pour fin messages Socket avec PyxvitalX <CRLF>.<CRLF>
    g_tcpSocket     = Q_NULLPTR;
    CombienDeFois   = 0;
    gSaisiePyxvitalEnCours = false;
}

//--------------------------------- InitConnexionPyxvital ----------------------------------------------------
bool pyxinterf::InitConnexionPyxvital()
{
    QString     nomFicPyxIni;
    QString     nomFicIni;
    QString     PyxvitalPath;

    // Recherche paramètres Pyxvital.ini.
    nomFicIni   = PATH_FILE_INI;

    //proc->settings()->setIniCodec("ISO 8859-1");

    PyxvitalPath = QDir::homePath() + "/" + proc->settings()->value("PyxInterf/PyxvitalPath").toString();
    if (PyxvitalPath.length() == 0)
        {UpMessageBox::Watch(Q_NULLPTR, "Lecture Paramètres Pyxvital",
                    " Le chemin d'accès à Pyxvital n'est pas paramétré!\nVeuillez vérifier vos paramètres PyxInterf dans Rufus.ini");
        return false;
        }
    // Recherche DLL Pyxvital dand AixVitale.ini.
    /*
    gzRepPyx = settingsPyxinterfIni.value(tr("Pyxvital/DLL_Interface")).toString();
    if (gzRepPyx.length() <= 1)
        {UpMessageBox::Watch(0, NomAppli, "Fichier de configuration AixVitale.ini absent ou paramétrage Interface Pyxvital incorrect.");
        return;
        }
    // Recherche Type de saisie dans  AixVitale.ini.
    gVisuPresta   = settingsAixIni->value(tr("Pyxvital/Visu_Prestations")).toString();
    gCdeSaisieFSE = settingsAixIni->value(tr("Pyxvital/Cde_Saisie_FSE")).toString();
*/

    //#ifdef Q_WS_MAC

        nomFicPyxIni    = PyxvitalPath  + "/PyxvitalOSX.ini";
        QSettings *PyxIni = new QSettings(nomFicPyxIni, QSettings::IniFormat);
        gnomPyxvitalExe = PyxvitalPath  + "/PyxvitalOSX.app";


        //PyxIni->setIniCodec ("ISO 8859-1");

        // Recup des paramètres pour lancer Pyxvital en mode Serveur.
        gServeurPyx = "localhost";
        gPortPyx    = PyxIni->value("Paramètres/Serveur_port").toInt();
        //UpMessageBox::Watch(0,PyxIni->value("Paramètres/Serveur_port").toString());
        if (gPortPyx == 0)
        {UpMessageBox::Watch(Q_NULLPTR, "Lecture Paramètres Pyxvital",
                    "PyxVitalOSX doit être lancé en mode serveur!\nVeuillez vérifier vos paramètres dans PyxvitalOSX.ini");
        return false;
        }

        // récup n° facturation du PS
        gnumPS    = PyxIni->value("PS/Numéro").toString();

        // Recherche nom des répertoires selon paramétrage Pyxvital
        /*
        [RÈpertoires]
        Fond=leer:Users:Serge:Pyxvital
        Tables=leer:Users:Serge:Pyxvital:TABLES
        Interface=leer:Users:Serge:Pyxvital:INTERF
        FSE=leer:Users:Serge:Pyxvital:FSE
        */
        gRepInterf  = PyxIni->value("Répertoires/Interface").toString().replace(":","/");
        gRepFSE     = PyxIni->value("Répertoires/FSE").toString().replace(":","/");
        if (gRepFSE.right(1) == "#")
            gRepFSE = gRepFSE.remove("#") + gnumPS + "/";
        else
            gRepFSE +=  "/";
        // pour les MAC on enleve le nom de la machine devant le chemin
#ifdef Q_OS_MAC
        int pos1 = gRepInterf.indexOf("/");
        gRepInterf = gRepInterf.mid(pos1,gRepInterf.length());
        int pos2 = gRepFSE.indexOf("/");
        gRepFSE = gRepFSE.mid(pos2,gRepFSE.length());
#endif
qDebug() << "INTERF =" + gRepInterf;
qDebug() << "FSE =" + gRepFSE;

        // Ouverture connexion avec le serveur PyxvitalX .
        g_tcpSocket = new QTcpSocket(this);
        //connect(g_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &pyxinterf::ErreurConnexionPyx);
        connect(g_tcpSocket, &QAbstractSocket::errorOccurred, this, &pyxinterf::ErreurConnexionPyx);
        connexionPyx();
        // il faudrait pouvoir attendre que la connexion soit établie ou Aixvitale arrété pour erreur de connexion. ?????
    //#endif

#ifdef Q_OS_WIN_PYXVITAL
    // Chargement de la DLL Pyxinterf.dll pour Windows seulement ----------------------------------------------
    char nomDLL[100];
    char charNomDll[100];
    uint32_t hinstDLL;
    strcpy(charNomDll, gzRepPyx.toAscii().data());
    sprintf(nomDLL,"%s/PYXINTERF.DLL",charNomDll);
    if ((hinstDLL=LoadLibraryA(nomDLL))) {
    //if ((hinstDLL=LoadLibrary(nomDLL)   )) {
        PSSV_Command = (SSV_Command)GetProcAddress(hinstDLL, "_SSV_Command@28");
        if (!PSSV_Command) {
            UpMessageBox::Watch(Q_NULLPTR , "AixVitale", "Problème lors de l'appel à Pyxvital \n Veuillez vérifier vos paramètres.");
            return false;}
        }
#endif

    gnomFicPatient      = gRepInterf + "/Patient.par";
    gnomFicPraticien    = gRepInterf + "/Praticien.par";
    gnomFicFacture      = gRepInterf + "/Facture.par";
    gnomficErreur       = gRepInterf + "/Erreurs.par";
    gnomficStop         = gRepInterf + "/Stop";

    return true;
}
//-------------------------------------connexionPyx------------------------------------------------------------------
//  La première connexion peut échouer si le serveur PyxvitalX n'est pas en attente.
//  (voir le signal erreur connexion pour traitement d'initialisation du serveur.)
//  ensuite on se contente d'ouvrir une connexion .qui sera fermée à la sortie d' Aixvitale
// ---------------------------------------------------------------------------------------------------
void pyxinterf::connexionPyx()
{
    qDebug("Connexion Pyx.");
    g_tcpSocket->abort();
    g_tcpSocket->connectToHost(gServeurPyx, gPortPyx);

    QString Erreur = g_tcpSocket->errorString();
       qDebug() << "Erreur connexion " + Erreur;

    // on attend 5 seconde la validation de la connexion.
    if (g_tcpSocket->waitForConnected(5000))
        qDebug("Connexion effectuée.");
    else
        qDebug("Connexion Impossible.");
}

// ----------------------------- appelPyxvital -----------------------------------------------------------------------------------
// Lance la DLL de Pyxvital avec le paramètre reçu.(P= lecture CPS, V=lecteure CV et sélection, W=lecture tte la CV en batch, ...)
// ---------------------------------------------------------------------------------------------------
bool pyxinterf::appelPyxvital(const char* modeAppel,int visuel, const char *libAction)
{

//char            szResult[1024] = "";          // retour DLL Pyxvital

//#ifdef Q_WS_MAC
//------------------------------------- Mode Socket TCPIP --------------------------------------------------------------
char    Cde[100];


QByteArray RecuPyx;

    if (visuel == SW_HIDE)          strcpy(Cde,"EXECHIDE://");
    else                            strcpy(Cde,"EXECSHOW://");
    strcat (Cde,modeAppel);
    strcat (Cde,CRLF);
    // envoi de la commande à PyxvitalX : Lire la CPS par exemple, mais aussi Télétransmette..
    // le temps d'attente est donc variable.
    g_tcpSocket->write(Cde) ;

    g_tcpSocket->waitForReadyRead(300000);     // attend le signal ReadyRead (ou 5mn) pour lire le retour.

    // on arrive ici qu'après avoir exécuté l'action dans Pyxvital...
    // ex : saisie du code PS ou lecture de la CPS .
    // voir que faire dans ErreurConnexionPyx rien reçu et timeout atteint ????????

    RecuPyx = g_tcpSocket->readAll() ;

    strcpy (gResultPyx, RecuPyx.constData() + 3);

    if (RecuPyx.mid(0,1) != "E")
        return true;
    traiteErreur(libAction, gResultPyx + 1);
    return false;

//#endif  //-------------------------------fin mode socket -----------------------------------------------------



#ifdef Q_WS_X11
//------------------------------------- Mode Socket TCPIP --------------------------------------------------------------
char    Cde[50];

QByteArray RecuPyx;

    if (visuel == SW_HIDE)          strcpy(Cde,"EXECHIDE://");
    else                            strcpy(Cde,"EXECSHOW://");
    strcat (Cde,modeAppel);
    strcat (Cde,CRLF);
    // envoi de la commande à PyxvitalX : Lire la CPS par exemple, mais aussi Télétransmette..
    // le temps d'attente est donc variable.
    tcpSocket->write(Cde) ;

    tcpSocket->waitForReadyRead(300000);     // attend le signal ReadyRead (ou 5mn) pour lire le retour.

    // on arrive ici qu'après avoir exécuté l'action dans Pyxvital...
    // ex : saisie du code PS ou lecture de la CPS .
    // voir que faire dans ErreurConnexionPyx rien reçu et timeout atteint ????????

    RecuPyx = tcpSocket->readAll() ;

    strcpy (szResult, RecuPyx.constData() + 3); // a revoir pour virer szResult.

    if (RecuPyx.mid(0,1) != "E")
        return true;
    traiteErreur(libAction, szResult + 1);
    return false;

#endif  //-------------------------------fin mode socket -----------------------------------------------------

#ifdef Q_OS_WIN_PYXVITAL
//------------------------------ Mode DLL spécifique Windows -------------------------------------------------
int iRet;
    iRet = PSSV_Command(hWndPyx, WM_USER, TRUE, visuel,(char*) modeAppel, szResult, sizeof(szResult));
    if (iRet)	{			// retour normal dll : Résultat dans szResult : Bon ou E+message
        if (szResult[0] != 'E')	{	// il n'y a pas eu d' erreur lors de la cde Pyxvital.
            return true;
            }
    }
    traiteErreur(libAction, szResult + 1);
    return false;
#endif  //--------------------------- fin mode DLL----------------------------------------------------------------
}
// ---------------------------------------------------------------------------------------------------
// Traitement des erreurs de comm avec le serveur Pyxvital (connexion, timeout, ...)
// ---------------------------------------------------------------------------------------------------
void pyxinterf::ErreurConnexionPyx(QAbstractSocket::SocketError socketError)
{
QString zbid;

qDebug() << "ErreurConnexion - "+g_tcpSocket->errorString() + " - Combien=" + QString::number(CombienDeFois);

    zbid =zbid.setNum(gPortPyx);
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        if (CombienDeFois < 15)
            { LancerLeServeur();
            return;
            }
        UpMessageBox::Watch(Q_NULLPTR, NomAppli, "Pyxvital (serveur) n'a pas été trouvé.<br><br>"
                                            "Veuillez vérifier les paramètres dans Pyxvital.ini.<br>"
                                            "- Nom serveur = " + gServeurPyx + "<br>- Port = " + zbid + "<br>- Progamme serveur = " + gnomPyxvitalExe);
        break;
    case QAbstractSocket::ConnectionRefusedError:
        if (CombienDeFois < 15)
            { LancerLeServeur();
            return;
            }
        QMessageBox::information(Q_NULLPTR, tr(NomAppli),
                                 tr("La connexion à Pxyvital (serveur) a échoué.<br><br>"
                                    "Veuillez vérifier les paramètres dans Pyxvital.ini.<br>"
                                 "- Nom serveur = ") + gServeurPyx + tr("<br>- Port = ") + zbid + tr("<br>- Progamme serveur = ") + gnomPyxvitalExe);
        break;
    default:
        // time out de saisie fse
        //if (gSaisiePyxvitalEnCours)
        //    return; // c'est juste pour avoir une tempo.
        QMessageBox::information(Q_NULLPTR, tr(NomAppli),
                   tr("Erreur : %1 .<br><br>Veuillez vérifier les paramètres dans Pyxvital.ini.<br>(Nom serveur = ").arg(g_tcpSocket->errorString())
                    + gServeurPyx + tr("<br>Port = ") + zbid + tr("<br>Progamme serveur = ") + gnomPyxvitalExe + ")");
    }
}
//---------------------------------------LancerLeServeur---------------------------------------------------------------
// Uniquement en mode Socket (Linux et MAC)
// la première erreur est peut-être normale, si le programme (serveur) PyxvitalX ne tourne pas.
// on le lance donc et on masque le formulaire ...
// on retente la connexion 15 fois après le lancement du prog serveur, puis on arrête.
// ---------------------------------------------------------------------------------------------------
void pyxinterf::LancerLeServeur()
{
#ifdef Q_WS_X11
    QStringList argList;
    argList << "";

    if (CombienDeFois == 1)     // on ne lance le prg serveur qu'une fois.
        {
         QProcess::startDetached(gnomPyxvitalExe, argList);
         }

    CombienDeFois ++;
   // Attendre longtemps . car sur certains systèmes c'est très long ...
    sleep(1);
    connexionPyx();
    modeSecurise();  // Pour MASQUER PYXVITAL
#endif
#ifdef Q_OS_MAC

    if (CombienDeFois == 0)     // on ne lance le prg serveur qu'une fois.
        {
        QProcess::startDetached("open " + gnomPyxvitalExe);
        }
    CombienDeFois ++;
   // Attendre longtemps . car sur certains systèmes c'est très long ...
    //sleep(1);
    connexionPyx();
    modeSecurise();  // Pour MASQUER PYXVITAL
#endif
}
// ---------------------------------------------------------------------------------------------------
// ------------------------------- traiteErreur() -----------------------------------------------------------------------
// Traite les erreurs : affiche un message écran et alimente le fichier d'échange du message d'erreur.
void pyxinterf::traiteErreur(const char *messer1, const char *messer2)
{
char messerr[2000];

    QMessageBox::critical (Q_NULLPTR, QObject::tr(NomAppli),
                              QObject::tr(messer1) + "\n" + QObject::tr(messer2),
                              QMessageBox::Ok, Qt::NoButton, Qt::NoButton );

    sprintf(messerr,"Erreur : %s : %s",messer1,messer2);
    //if (MEDINTUX)
    //    majEchange(messerr);
}

// ------------------------------- modeSecurise-----------------------------------------------------------------------
// Pour MASQUER PYXVITAL
// ---------------------------------------------------------------------------------------------------
void pyxinterf::modeSecurise()
{
     appelPyxvital("GS",SW_HIDE, "Mode sécurisé");
}
//---------------------------------- lireCPS ------------------------------------------------------------------------------------
//	Lecture de la CPS et retour infos du médecin
// ---------------------------------------------------------------------------------------------------
/*
bool pyxinterf::lireCPS()
{
QString zNom, zPrn;

     //TraceLog("Lecture Carte PS");

    // testLecteur();

    if (appelPyxvital(COMD_LIRE_PS, SW_HIDE, "Lecture de la CPS"))	// (P) lire la carte PS et alimenter Praticien.par.
        {
        if (MEDINTUX)   //	retour des infos Praticien.par vers Manager
            {return (retficEchange(gnomFicPraticien) );
            }
        else            // Affichage des infos Medecin sur l'écran Aixvital >>> à revoir.
            {
            QSettings *settings    = new QSettings(gnomFicPraticien, QSettings::IniFormat);
            zNom  = settings->value(tr("PS/Nom")).toString();
            zPrn  = settings->value(tr("PS/Prénom")).toString();
            nomToubib->setText(zNom + " " + zPrn);
            return true;
            }
        }
else
    return false;
}
*/

// ---------------------------------------------------------------------------------------------------
// Saisie d'une FSE
// - ouvre la connexion avec Pyxvital
// - commande la saisie de la FSE à Pyxvital
// - retourne le nom du fichier Facture.par
//   ou rien si erreur
// ---------------------------------------------------------------------------------------------------
QString pyxinterf::Saisie_FSE()
{
    InitVariables();
    if (!InitConnexionPyxvital())
        return ("");

    // Tester si la CPS et la CV ont été lues.
    strcpy(gResultPyx,"");
    appelPyxvital("?", SW_HIDE, "Status de la CPS");	// (?) Interroge Pyxvital sur lecture  des cartes CPS et CV.
    if (gResultPyx[1] != 'O')
        { QMessageBox::information(Q_NULLPTR, NomAppli,tr("La carte PS n'a pas été lue !"));
        return "";
        }
    if (gResultPyx[2] != 'O')
        { QMessageBox::information(Q_NULLPTR, NomAppli,tr("La carte Vitale n'a pas été lue !"));
        return "";
        }

    // Contrôle que la CV appartient bien au patient en cours.
    // A faire

    // on supprime le fichier Facture précédent et le fichier erreurs.
    QFile::remove(gnomFicFacture);
    QFile::remove(gnomficErreur);
    QFile::remove(gnomficStop);


//#ifdef Q_WS_MAC
    /*
char scde[100];

    if (gCdeSaisieFSE != "")    // Une cde particuliËre est paramÈtrÈe dans le .ini
        {strcpy(scde,gCdeSaisieFSE.toAscii());
        }
    else                        // par defaut on execute EXECHIDE;//CF
        {                       // Remarque : CFH plante ??? bloque la saisie.
        strcpy(scde,"EXECHIDE://CF");           // A REVOIR CAR le mode HIDE  plante PYXVITAL....
         strcpy(scde,"EXECSHOW://CF");

        }

    strcat (scde,CRLF);
    //TraceLog("PaSSE02 : cde tcp lance la saisie=" + QString::fromLocal8Bit(scde) ); // A VIRER
    tcpSocket->write(scde) ;
    QByteArray RecuPyx;

    gSaisiePyxvitalEnCours = true;
    CombienDeFois = 0;
    while (gSaisiePyxvitalEnCours)
        {
        tcpSocket->waitForReadyRead(1000);      // time out 1 seconde pour lancer SaisieFSEencOurs
        RecuPyx = tcpSocket->readAll() ;
        Slot_saisieFSEenCours();
        }

//#endif
*/


    // appelPyxvital("CF", SW_SHOW, "Saisie de facture");	// (CF) Saisie d'une facture avec choix facture type .
     appelPyxvital("", SW_SHOW, "Saisie de facture");	//  Saisie d'une facture avec choix facture type .

    // Attendre la fin de la saisie de FSE
    Attendre_fin_FSE();

    // On masque le formulaire Pyxvital
    modeSecurise();

    // on ferme et on s'en va.
    FermeTout();

    if (gResultPyx[0] == 'E')   // il y a eu une erreur pendant la saisie de FSE - Pas de facture saisie .
        return "";

    return gnomFicFacture;      // on retourne le nom du fichier contenant la facture.
}
// -----------------------------------------------------------------------------------------------------------------
// On boucle dans cette fonction tant que le fichier FSE n'est pas créé ou qu'une erreur est générée par Pyxvital
//------------------------------------------------------------------------------------------------------------------
void pyxinterf::Attendre_fin_FSE()
{
    bool SaisieEnCours  = true;
//    bool YaUneErreur    = false;
    QString NomFicFSE;

    while (SaisieEnCours)
        {
        // on controle s'il y a eu des erreurs. Fichier "Erreur.par" présent.
        if (QFile::exists(gnomficErreur) )
            { // on vérifie que le mot Erreur est bien dans le message
            QFile *fileErreur = new QFile(gnomficErreur);
            if (fileErreur->open(QIODevice::ReadWrite))
                {QByteArray qbmesserr = fileErreur->readAll();
                QString messerr = qbmesserr;
                if (messerr.indexOf("[Erreur]") >= 0)
                    {// vérifier si ce message est bloquant ou pas ????
                    if  (   (messerr.indexOf("Titre=Diagnostic") < 0)  &&               // ce n'est pas une vraie erreur
                        (   (messerr.indexOf("Titre=Mutuelle")  >= 0)  ||
                            (messerr.indexOf("Titre=Recherche") >= 0)  ||
                            (messerr.indexOf("Titre=Facture non inscrite") >= 0)))      // voir d'autre erreurs éventuellement ???
                        {
//                        YaUneErreur = true;
                        SaisieEnCours = false;
                        break;
                        } // fin c'est une vraie erreur
                    } // fin if ya le mot Erreur
                } // fin open
            } // fin if ya le fichier erreur

        // on controle s'il y a eu  une interruption. (fichier stop pas toujours généré ...)
        // Pyxvital génère un fichier stop à cahque ligne enregistrée si pas de modèle
        // if (QFile::exists(gnomficStop) )                    // Pyxvital a rendu la main .. mais on se sait pas si ya erreur ?
        //    {break;                                         // sortie de boucle sans savoir pourquoi.
        //    }

        // y'a pas d'erreur, on continue ...
        // on teste si la facture a été saisie : présence du fichier "facture.par"
        if (QFile::exists(gnomFicFacture) )                     // le fichier facture.par existe , donc une au moins a été saisie
            {QString    zfact     = proc->settings()->value(tr("Tarification/Taux")).toString();

            // Si la facture est terminée, on controle la fin de la création du fichier FSExxxxxxxx.fse pour interrompre la boucle.
            if (zfact.length() > 0)
                {zfact = proc->settings()->value(tr("Facture/Numero")).toString();
                NomFicFSE = gRepFSE + "File" + zfact + ".fse";      // Pyxvital/FSE/numeroOrdre(8chiffres)/File000000XXX.fse
                if (QFile::exists(NomFicFSE))       // la saisie est terminée. la FSE a été générée
                    {break;                         // sortie normale de la boucle
                    }
                } // fin if facture terminée (mais FSE pas encore générée)
            } // fin if facture.par existe
        //sleep(1);
        }// fin while SaisieEnCours
    qApp->processEvents(); // pour voir si l'Ècrans se rafraichi correctement aprËs effacement Pyx ?? sous linux ?
}
