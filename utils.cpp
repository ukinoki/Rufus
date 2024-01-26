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

#include "utils.h"

Utils* Utils::instance =  Q_NULLPTR;
Utils* Utils::I()
{
    if( !instance )
        instance = new Utils();
    return instance;
}


/*
 * Initialisation des variables static const
*/
QRegExp const Utils::rgx_rx = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùûÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_AlphaNumeric = QRegExp("[A-Za-z0-9]*");
QRegExp const Utils::rgx_AlphaNumeric_3_12 = QRegExp("[A-Za-z0-9]{3,12}$");
QRegExp const Utils::rgx_AlphaNumeric_5_15 = QRegExp("[A-Za-z0-9]{5,15}$");
QRegExp const Utils::rgx_AlphaNumeric_5_12  = QRegExp("[A-Za-z0-9]{5,12}$");
QRegExp const Utils::rgx_Question  = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùûÙçÇ'a-zA-ZŒœ0-9°, -]*[?]*");
QRegExp const Utils::rgx_MajusculeSeul = QRegExp("[A-Z]*");
QRegExp const Utils::rgx_IPV4 = QRegExp("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
QRegExp const Utils::rgx_IPV4_mask = QRegExp("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");

QRegExp const Utils::rgx_mail           = QRegExp("^[A-Za-z0-9_-]+(.[A-Za-z0-9_-]+)+@[A-Za1-z0-9_-]+(.[A-Za1-z0-9_-]+).[A-Za-z0-9_-]{2,6}");
QRegExp const Utils::rgx_NNI = QRegExp("[12][0-9]{14}");

QRegExp const Utils::rgx_adresse = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùûÙçÇ'a-zA-ZŒœ0-9°, -]*");
QRegExp const Utils::rgx_intitulecompta = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùûÙçÇ'a-zA-ZŒœ0-9°, -/%]*");
QRegExp const Utils::rgx_CP = QRegExp(cp());
QRegExp const Utils::rgx_ville = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùûÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_telephone = QRegExp("[0-9 ]*");

QRegExp const Utils::rgx_tabac = QRegExp("[0-9]{2}");
QRegExp const Utils::rgx_cotation = QRegExp("[a-zA-Z0-9.+/ ]*");

QRegExp const Utils::rgx_recherche = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùûÙçÇ'a-zA-Z %-]*");


/*!
 *  \brief Pause
 *
 *  Methode qui permet d'attendre un certain temps (donné en paramètre)
 *  sert surtout à forcer la mise à jour d'un affichage sans attendre la fin d'une fonction
 *  \param msec : le temps à attendre en milliseconde
 *
 */
void Utils::Pause(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

/*!
 * \brief Utils::trim
 * Cette fonction va supprimer :
 * - les " ", "-" et "'" en début et fin du texte
 * - les " ", "-" et "'" en doublon dans le texte
 * - les retour à la ligne en fin du texte
 * \param text le texte à nettoyer
 * \param end (true par défaut) mettre false si on ne souhaite pas nettoyer la fin du texte
 * \param removereturnend (false par défaut) mettre true si on souhaite retirer les retours à la ligne à la fin du texte
 * \return le texte nettoyé
 */
QString Utils::trim(QString text, bool end, bool removereturnend)
{
    if (text == "" || text == QString())
        return "";
    QString textC = text;
    QChar c;
    while( textC.size() )                   // enlève les espaces, les tirets et les apostrophes du début
    {
        c = textC.at(0);
        if( c == " " || c == "-" || c == "'" )
            textC = textC.remove(0,1);
        else
            break;
    }

    if( end )                               // enlève les espaces, les tirets et les apostrophes de la fin
        while( textC.size() )
        {
            int lastIndex = textC.size() - 1;
            c = textC.at(lastIndex);
            if( c == " " || c == "-" || c == "'" )
                textC = textC.remove(lastIndex,1);
            else
                break;
        }

    if( removereturnend )                   // enlève les retours à la ligne de la fin
        while( textC.size() )
        {
            int lastIndex = textC.size() - 1;
            c = textC.at(lastIndex);
            if( c == "\n" )
                textC = textC.remove(lastIndex,1);
            else
                break;
        }

    QString newText = "";
    QChar lastChar;
    for( int i=0; i < textC.size(); ++i )   // enlève les espaces, les tirets et les apostrophes en doublon
    {
        c = textC.at(i);
        if( lastChar == " " || lastChar == "-" || lastChar == "'" )
            if( lastChar == c )
                continue;

        newText += c;
        lastChar = c;
    }

    return newText;
}

/*!
 * \brief Utils::Capitilize
 * Cette fonction va mettre tous les premiers caractères en majuscule
 * \param text le texte à modifier
 * \return le texte modifié
 */
QString Utils::capitilize(QString text, bool onlyfirst)
{
    QString newText="";
    if (onlyfirst)
    {
        QChar c = text.at(0);
        c = c.toUpper();
        newText += c;
        for( int i=1; i < text.size(); ++i )
            newText += text.at(i);
        return newText;
    }
    QChar c;
    //Permet de forcer le premier caractère en majucule
    QChar lastChar = ' ';
    for( int i=0; i < text.size(); ++i )
    {
        c = text.at(i);
        if( lastChar == " " || lastChar == "-" || lastChar == "'" )
            c = c.toUpper();
        newText += c;
        lastChar = c;
    }
    return newText;
}

/*!
 * \brief Utils::trimcapitilize
 * Cette fonction va supprimer :
 * - les " ", "-" et "'" en début et fin du texte
 * - les " ", "-" et "'" en doublon dans le texte
 * et va mettre tous les premiers caractères en majuscule
 * \param text le texte à modifier
 * \param end mettre false si on ne souhaite pas nettoyer la fin du texte
 * \param maj mettre false si on ne souhaite mettre le premier caractère de chaque mot en majuscule
 * \param lower mettre false si on ne souhaite pas mettre en minuscule les majuscules situées au milieu des mots
 * \return le texte modifié
 */
QString Utils::trimcapitilize(QString text, bool end, bool maj, bool lower)
{
    if (lower)
        text = text.toLower();
    text = trim (text, end);
    if (maj)
        text = capitilize(text);
    return text;
}

QString Utils::retirecaracteresaccentues(QString nom)
{
    nom.replace(QRegExp("[éêëè]"),"e");
    nom.replace(QRegExp("[ÉÈÊË]"),"E");
    nom.replace(QRegExp("[àâ]"),"a");
    nom.replace(QRegExp("[ÂÀ]"),"A");
    nom.replace(QRegExp("[îï]"),"i");
    nom.replace(QRegExp("[ÏÎ]"),"I");
    nom.replace(QRegExp("[ôö]"),"o");
    nom.replace(QRegExp("[ÔÖ]"),"O");
    nom.replace("ù","u");
    nom.replace("Ù","U");
    nom.replace("ç","c");
    nom.replace("Ç","C");
    nom.replace("Œ","OE");
    nom.replace("œ","oe");
    return nom;
}

/*!
 *  \brief convertHTML
 *  convertir un QString en html
 *  on écrit le QString dans un QtextEdit et on récupère le html avec QTextEdit::toHtml()
 *  on retire les lignes vides de la fin
 */
bool Utils::convertHTML(QString &text)
{
    QTextEdit textprov;
    textprov.setText( text );
    text = textprov.toHtml();
    //qDebug() << text;
    return retirelignevidefinhtml(text);

}

/*!
 *  \brief convertPlainText
 *  convertir un QString en plaintext
 *  on écrit le QString dans un QtextEdit et on récupère le plaintext avec QTextEdit::toPlainText()
 *  on retire les lignes vides de la fin
 */
void Utils::convertPlainText(QString &text)
{
    QTextEdit textprov;
    textprov.setText( text );
    text = textprov.toPlainText();
    text  = trim(text, true, true);
}

/*!
 *  \brief nettoieHTML
 *  nettoyer tous les trucs inutiles dans un html généré par QT
 * \param supprimeLesLignesVidesDuMilieu - comme son nom l'indique
 *  placer les marqueurs Linux ou Mac
 */
void Utils::nettoieHTML(QString &text, bool supprimeLesLignesVidesDuMilieu)
{
    QRegExp reg1 = QRegExp("<p style=\"-qt-paragraph-type:empty; "
                           "margin-top:[0-9]{1,2}px; margin-bottom:[0-9]{1,2}px; "
                           "margin-left:[0-9]{1,2}px; margin-right:[0-9]{1,2}px; "
                           "-qt-block-indent:0; text-indent:[0-9]{1,2}px;\"><br /></p>");
    //reg1 = QRegExp("<p style=\"-qt-paragraph-type:empty;([\\.]*)<br /></p>");
    QRegExp reg2 = QRegExp("<p style=\" margin-top:0px; margin-bottom:0px; "
                           "margin-left:[0-9]{1,2}px; margin-right:[0-9]{1,2}px; "
                           "-qt-block-indent:0; text-indent:[0-9]{1,2}px;\">");
    //qDebug() << text;
    bool remetunelignealafin = convertHTML(text);
    //qDebug() << text;
   if (supprimeLesLignesVidesDuMilieu)
        text.remove(reg1);
    text.replace(reg2,"<p style=\" margin-top:0px; margin-bottom:0px;\">");
    text.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
    text.remove(HTMLCOMMENT_LINUX);
    text.remove(HTMLCOMMENT_MAC);
    if (remetunelignealafin)
        text.append(HTML_FINPARAGRAPH);
#ifdef Q_OS_LINUX
    text.append(HTMLCOMMENT_LINUX);
#endif
#ifdef Q_OS_MAC
    text.append(HTMLCOMMENT_MAC);
#endif
}

/*!
 * \brief Utils::retirelignevidehtml(QString txthtml)
 * retirer les paragraphes vierges à la fin d'un texte en html
 * \param &txthtml
 */
bool Utils::retirelignevidefinhtml(QString &txthtml)
{
    bool ligneretiree = false;
    bool a = true;
    while (a) {
        int debut = txthtml.lastIndexOf("<p");
        int fin   = txthtml.lastIndexOf("</p>");
        int longARetirer = fin - debut + 4;
        if (txthtml.midRef(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
        {
            txthtml.remove(debut,longARetirer);
            ligneretiree = true;
        }
        else a = false;
    }
    return ligneretiree;
}

bool Utils::epureFontFamily(QString &text)
{
    QString txt= text;
    QRegExp rx("font-family:'([a-zA-Z0-9 ,-]+)");
    int pos = 0;
    while (pos != -1) {
        pos = rx.indexIn(text, pos);
        QStringList list = rx.capturedTexts();
        if (list.size() >0)
        {
            QString replacmt = list.at(0).split(",").at(0);
            text.replace(list.at(0), replacmt);
            pos += replacmt.length();
        }
    }
    return (txt != text);
}

/*!
    * \brief Utils::corrigeErreurHtmlEntete
    * \param text
    * \param ALD
    * \return
    *  L'entête de chaque texte émis est constitué de 2 blocs contigus:
      * un bloc gauche dans lequel sont rassemblés les concernant l'émetteur du document
         * docteur Bidule
         * adresse
         * .etc...
      * un bloc droit rassemblant
         * la date
         * le nom du patient dans les ordonnances
         * ...etc...
    * il y a donc 2 tables dont la largeur est dééterminée par les macros
      * pour les ordonnances ALD
         * HTML_LARGEUR_ENTETE_GAUCHE_ALD
         * HTML_LARGEUR_ENTETE_DROITE_ALD
      * pour les autres documents
         * HTML_LARGEUR_ENTETE_GAUCHE
         * HTML_LARGEUR_ENTETE_GAUCHE
    * Sur d'anciennes versions de Rufus, il y avait des erreurs dans ces largeurs et elles ne s'affichent pas convenablement.
    * Cette fonction sert à corriger ces erreurs
*/
bool Utils::corrigeErreurHtmlEntete(QString &text, bool ALD)
{
    QString txt = text;
    QString largeurALDG = "float: left;\" cellpadding=\"5\"><tr><td width=\"" HTML_LARGEUR_ENTETE_GAUCHE_ALD "\">";
    QString largeurALDD = "float: right;\"><tr><td width=\"" HTML_LARGEUR_ENTETE_DROITE "\">";
    QString largeurG = "float: left;\"><tr><td width=\"" HTML_LARGEUR_ENTETE_GAUCHE "\">";
    QString largeurD = "float: right;\"><tr><td width=\"" HTML_LARGEUR_ENTETE_DROITE "\">";
    QRegExp rx;
    QString patternALDG = "float: left;\" cellpadding=\"5\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    QString patternALDD = "float: right;\" cellpadding=\"6\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    QString patternG    = "float: left;\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    QString patternD    = "float: right;\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    rx.setPattern(ALD? patternALDG : patternG);
    rx.indexIn(text);
    QStringList list = rx.capturedTexts();
    if (list.size() >0)
    {
        if (list.at(0) !="")
        {
            patternALDG = "float: left;\" cellpadding=\"5\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_GAUCHE_ALD "\">";
            patternG    = "float: left;\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_GAUCHE "\">";
            rx.setPattern(ALD? patternALDG : patternG);
            if (rx.indexIn(list.at(0)) == -1)
                text.replace(list.at(0), ALD? largeurALDG : largeurG);
        }
    }
    rx.setPattern(ALD? patternALDD : patternD);
    rx.indexIn(text);
    list = rx.capturedTexts();
    if (list.size() >0)
    {
        if (list.at(0) !="")
        {
            patternALDD = "float: right;\" cellpadding=\"6\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_DROITE_ALD "\">";
            patternD    = "float: right;\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_DROITE "\">";
            rx.setPattern(ALD? patternALDG : patternG);
            if (rx.indexIn(list.at(0)) == -1)
                text.replace(list.at(0), ALD? largeurALDD : largeurD);
        }
    }
    return (txt != text);
}


/*!
 * \brief Utils::CalcSize(QString txt)
 * calcule la taille en pixels du texte passé en paramètres dans la police passée en paramètre
 * \param QString txt
 * \param QFint font
 */
QSize Utils::CalcSize(QString txt, QFont fm)
{
    double correction = (txt.contains("<b>")? 1.2 : 1.1); //le 1.2 est là pour tenir compte des éventuels caractères gras
    convertPlainText(txt);
    QStringList lmsg            = txt.split("\n");
    int         w               = 0;
    double      hauteurligne    = QFontMetrics(fm).height()*correction;
    int         nlignes         = lmsg.size();
    for (int k=0; k<nlignes; k++)
    {
        int x   = int(QFontMetrics(fm).width(lmsg.at(k))*correction);
        w       = (x>w? x : w);
        //qDebug() << lmsg.at(k) + " - ligne = " + QString::number(k+1) + " - largeur = " + QString::number(w);
    }
    return QSize(w,int(hauteurligne*nlignes));
}

/*!
 * \brief Utils::CompressFileJPG(QString pathfile, QString Dirprov, QString nomfileEchec)
 * comprime un fichier jpg à une taille inférieure à celle de la macro TAILLEMAXIIMAGES
 * \param QString pathfile le chemin complet du fichier d'origine utilisé aussi en cas d'échec pour faire le log
 * \param QString dirprov le nom du dossier d'imagerie
 * \param QDate datetransfert date utilisée en cas d'échec pour faire le log
 * \return true si réussi, false si échec de l'enregistrement du fichier
 * en cas d'échec
    * un fichier de log est utilisé ou créé au besoin dans le répertoire DIR_ECHECSTRANSFERTS
    * et une ligne résumant l'échec est ajoutée en fin de ce fichier
    * le fichier d'origine est ajouté dans ce même répertoire
 */
bool Utils::CompressFileJPG(QString pathfile, QString Dirprov, QDate datetransfert)
{
    /* on vérifie si le dossier des echecs de transferts existe et on le crée au besoin*/
    QString CheminEchecTransfrDir   = Dirprov + NOM_DIR_ECHECSTRANSFERTS;
    if (!mkpath(CheminEchecTransfrDir))
    {
        QString msg = QObject::tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminEchecTransfrDir + "</b></font>" + QObject::tr(" invalide");
        ShowMessage::I()->SplashMessage(msg, 3000);
        return false;
    }
    /* on vérifie si le dossier provisoire existe sur le poste et on le crée au besoin*/
    QString DirStockProvPath = Dirprov + NOM_DIR_PROV;
    if (!mkpath(DirStockProvPath))
    {
        QString msg = QObject::tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + DirStockProvPath + "</b></font>" + QObject::tr(" invalide");
        ShowMessage::I()->SplashMessage(msg, 3000);
        return false;
    }

    QFile CC(pathfile);
    double sz = CC.size();
    if (sz < TAILLEMAXIIMAGES)
        return true;
    QImage  img(pathfile);
    QString filename = QFileInfo(pathfile).fileName();
    QString nomfichresize = DirStockProvPath + "/" + filename;
    QFile fileresize(nomfichresize);
    if (fileresize.exists())
        fileresize.remove();
    QFile echectrsfer(CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfert.toString("yyyy-MM-dd") + ".txt");
    QPixmap pixmap;
    double w = img.width();
    double h = img.height();
    int x = img.width();
    if (int(w*h)>(4096*1024)) // si l'image dépasse 4 Mpixels, on la réduit en conservant les proportions
    {
        double proportion = w/h;
        int y = int(sqrt((4096*1024)/proportion));
        x = int (y*proportion);
    }
    pixmap = pixmap.fromImage(img.scaledToWidth(x,Qt::SmoothTransformation));
    /* on enregistre le fichier sur le disque du serveur
     * si on n'y arrive pas,
        * on crée le fichier log des echecstransferts correspondants dans le répertoire des echecs de transfert sur le serveur
        * on complète ce fichier en ajoutant une ligne correspondant à cet échec
        * on enregistre dans ce dossier une copie du fichier d'origine
     */
    if (!pixmap.save(nomfichresize, "jpeg"))
    {
        if (echectrsfer.open(QIODevice::Append))
        {
            QTextStream out(&echectrsfer);
            out << CC.fileName() << "\n" ;
            echectrsfer.close();
            CC.copy(CheminEchecTransfrDir + "/" + filename);
        }
        return false;
    }
    CC.remove();
    /* on comprime*/
    int tauxcompress = 90;
    while (sz > TAILLEMAXIIMAGES && tauxcompress > 1)
    {
        pixmap.save(nomfichresize, "jpeg",tauxcompress);
        sz = fileresize.size();
        tauxcompress -= 10;
    }
    fileresize.copy(pathfile);
    fileresize.close();
    fileresize.remove();
    return true;
}

/*!
 * \brief Utils::dir_size
 * Cette fonction va renvoyer le nombre de fichiers contenu dans un dossier ainsi que le volume du dossier
 * utilisé pour le calcul du volume d'une opération de sauvegarde-restauration p.e.
 * \param text le chemin du dossier
 * \return un QMap avec ces 2 infos
 */
QMap<QString, qint64> Utils::dir_size(const QString DirPath)
{
    QMap<QString, qint64>      DataDir;
    qint64 sizex = 0;
    qint64 nfiles = 0;

    QDir dir(DirPath);
    if(!dir.exists())
    {
        DataDir["Size"]= 0;
        DataDir["Nfiles"]= 0;
        return DataDir;
    }
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();

    for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QMap<QString, qint64> size = QMap<QString, qint64>();
        if (fileInfo.isDir())
        {
            QString path = fileInfo.absoluteFilePath();
            size = dir_size(path);
        }
        sizex += (fileInfo.isDir()) ? size["Size"]: fileInfo.size();
        nfiles += (fileInfo.isDir()) ? size["Nfiles"] : i+1;
    }
    DataDir["Size"]= sizex;
    DataDir["Nfiles"]= nfiles;
    return DataDir;
}

/*!
 * \brief Utils::getExpressionSize
 * Cette fonction va renvoyer la taille d'un double en Mo, Go ou To
 */
QString Utils::getExpressionSize(qint64 size)
{
    QString com = "Mo";
    double dsize = size;
    if (dsize>1024)
    {
        com = "Go";
        dsize /= 1024;
        if (dsize>1024)
        {
            com = "To";
            dsize /= 1024;
        }
    }
    return QString::number(dsize,'f',2) + com;
}

qint32 Utils::ArrayToInt(QByteArray source)
{
    qint32 number;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> number;
    return number;
}

QByteArray Utils::IntToArray(int source)
{
    //permet d'éviter le cast
    QByteArray ba;
    QDataStream data(&ba, QIODevice::ReadWrite);
    data << source;
    return ba;
}

QString Utils::IPAdress()
{
    QString IPadress = "";

    //autre méthode
    /*!
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost)
        {
            IPadress = address.toString();
            break;
        }
    */

    //autre méthode
    foreach (const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces())
    {
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if(flags.testFlag(QNetworkInterface::IsRunning) && !flags.testFlag(QNetworkInterface::IsLoopBack))
            foreach (const QNetworkAddressEntry &address, netInterface.addressEntries())
                if(address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    IPadress = address.ip().toString();
                    break;
                }
        if (IPadress != "")
            break;
    }

    return IPadress;
}

QString Utils::calcIP(QString IP, bool aveczero)
{
    if (!rgx_IPV4.exactMatch(IP))
        return IP;

    QString IPaveczero (""), IPsanszero ("");
    QStringList listIP = IP.split(".");
    for (int i=0;i<listIP.size();i++)
    {
        IPsanszero += QString::number(listIP.at(i).toInt());
        QString AvecZero;
        AvecZero += QString::number(listIP.at(i).toInt());
        if (listIP.at(i).toInt()<100)
            AvecZero = "0" + AvecZero;
        if (listIP.at(i).toInt()<10)
            AvecZero = "0" + AvecZero;
        IPaveczero += AvecZero;
        if (i<listIP.size()-1)
        {
            IPaveczero += ".";
            IPsanszero += ".";
        }
    }
    return (aveczero? IPaveczero : IPsanszero);
}

QString Utils::MACAdress()
{
    QString IPadress = IPAdress();
    QString MACAddress = "";
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &entry, networkInterface.addressEntries()) {
            if (entry.ip().toString() == IPadress) {
                MACAddress = networkInterface.hardwareAddress();
                break;
            }
        }
    }
    return MACAddress;
}

QString Utils::getMacForIP(QString ipAddress)
{
    QString MAC;
    QProcess process;
    process.start(QString("arp -a %1").arg(ipAddress));
    if(process.waitForFinished())
    {
        QString result = process.readAll();
        QStringList list = result.split(QRegularExpression("\\s+"));
        if(list.contains(ipAddress))
            MAC = list.at(list.indexOf(ipAddress) + 1);
    }
    return MAC;
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Faire précéder l'apostrophe d'un caractère d'échappement pour les requêtes SQL --------------------
------------------------------------------------------------------------------------------------------------------------------------*/
QString Utils::correctquoteSQL(QString text)
{
    text.replace("\\","\\\\");
    return text.replace("'","\\'");
}

QString Utils::getBaseFromMode(ModeAcces mode )
{
    switch (mode) {
    case ReseauLocal:
        return "BDD_LOCAL";
    case Distant:
        return "BDD_DISTANT";
    case Poste:
        return "BDD_POSTE";
    }
    return "BDD_LOCAL";
}

QString Utils::calcSHA1(QString mdp)
{
    QByteArray ba = QCryptographicHash::hash(mdp.toUtf8(), QCryptographicHash::Sha1);
    return QString(ba.toHex());
}

/*---------------------------------------------------------------------------------------------------------------------
    -- VÉRIFICATION DE MDP --------------------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
/*!
 * \brief Utils::VerifMDP
 * \param MDP = le mdp à vérifeir
 * \param Msg = message de la boîte de diaolgue
 * \param mdpval = la valeur entrée par l'utilisateur dans la boîte de dialogue
 * \param mdpverified = le mot de passe a déjà été vérifié -> permet de shunter la fonction si le mdp a déjà été vérifié
 * \param parent
 * \return
 */
bool Utils::VerifMDP(QString MDP, QString Msg, QString &mdpval, bool mdpverified, QWidget *parent)
{
    if (mdpverified)
        return true;

    if (parent != Q_NULLPTR)
    {
        UpDialog *dlg_askMDP    = new UpDialog(parent);
        dlg_askMDP      ->setWindowModality(Qt::WindowModal);

        UpLineEdit *ConfirmMDP = new UpLineEdit(dlg_askMDP);
        ConfirmMDP      ->setEchoMode(QLineEdit::Password);
        ConfirmMDP      ->setAlignment(Qt::AlignCenter);
        ConfirmMDP      ->setMaxLength(25);
        dlg_askMDP      ->dlglayout()->insertWidget(0,ConfirmMDP);

        UpLabel *labelConfirmMDP = new UpLabel();
        labelConfirmMDP ->setText(Msg);
        labelConfirmMDP ->setAlignment(Qt::AlignCenter);
        dlg_askMDP      ->dlglayout()->insertWidget(0,labelConfirmMDP);

        dlg_askMDP      ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
        connect(dlg_askMDP->OKButton,    &QPushButton::clicked, dlg_askMDP, [=] {
            if (calcSHA1(ConfirmMDP->text()) == MDP)
                dlg_askMDP->accept();
            else if (ConfirmMDP->text() == MDP)
                dlg_askMDP->accept();
            else
                UpMessageBox::Watch(dlg_askMDP, QObject::tr("Mot de passe invalide!"));
        });
        dlg_askMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
        dlg_askMDP->dlglayout()->setSpacing(8);
        mdpval = ConfirmMDP->text();
        mdpverified = (dlg_askMDP->exec() == QDialog::Accepted);
        delete dlg_askMDP;
        return mdpverified;
    }
    else
    {
        QInputDialog quest(parent);
        quest.setCancelButtonText("Annuler");
        quest.setLabelText(Msg);
        quest.setInputMode(QInputDialog::TextInput);
        quest.setTextEchoMode(QLineEdit::Password);
        QList<QLineEdit*> list = quest.findChildren<QLineEdit*>();
        for (int i=0;i<list.size();i++)
            list.at(0)->setAlignment(Qt::AlignCenter);
        QList<QLabel*> listlab = quest.findChildren<QLabel*>();
        for (int i=0;i<listlab.size();i++)
            listlab.at(0)->setAlignment(Qt::AlignCenter);
        quest.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        int a = quest.exec();
        mdpval = quest.textValue();
        if (a > 0)
        {
            if (calcSHA1(quest.textValue()) == MDP)
                return true;
            else if (quest.textValue() == MDP)
                return true;
            else
                UpMessageBox::Watch(parent, QObject::tr("Mot de passe invalide!"));
        }
        return false;
    }
}

/*---------------------------------------------------------------------------------------------------------------------
    -- Crée le path d'un dossier --------------------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Utils::mkpath(QString path)
{
    QDir Dir;
    return Dir.mkpath(path);
}

/*!
 * \brief Utils::cleanfolder
 * élimine les sous-dossiers vides d'un dossier
 */
void Utils::cleanfolder(const QString DirPath)
{
    QDir dir(DirPath);
    if (!dir.exists())
        return;
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();

    if (list.size()==0)
    {
        dir.rmdir(DirPath);
        //qDebug() << "dossier vide effacé" << DirPath;
    }
    else for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isDir())
            cleanfolder(fileInfo.absoluteFilePath());
    }
}

void Utils::countFilesInDirRecursively(const QString dirpath, int &tot)
{
    QDir dir(dirpath);
    if (!dir.exists())
    return;
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.size(); ++i)
    {
    QFileInfo fileInfo = list.at(i);
    if (fileInfo.isDir())
        countFilesInDirRecursively(fileInfo.absoluteFilePath(), tot);
    else
        tot++;
    }
}

void Utils::copyfolderrecursively(const QString origindirpath, const QString destdirpath, int &n, QString firstline, QProgressDialog *progress, QFileDevice::Permissions permissions)
{
    cleanfolder(origindirpath);
    cleanfolder(destdirpath);
    QDir dir(origindirpath);
    if (!dir.exists())
    return;
    QDir dirdest(destdirpath);
    if (!dirdest.exists())
    mkpath(destdirpath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isDir())
            copyfolderrecursively(fileInfo.absoluteFilePath(), destdirpath + "/" + fileInfo.fileName(), n, firstline, progress);
        else
        {
            QFile file(fileInfo.absoluteFilePath());
            if (progress)
            {
                n ++;
                QString text = firstline;
                if (text != QString())
                    text += "\n";
                text += QString::number(n) + "/" + QString::number(progress->maximum()) + " " + QFileInfo(file).fileName();
                progress->setLabelText(text);
                progress->setValue(n);
            }
            if (file.open(QIODevice::ReadOnly))
            {
                QString filedestpath = destdirpath + "/" + QFileInfo(file).fileName();
                file.copy(filedestpath);
                QFile(filedestpath).setPermissions(permissions);
            }
        }
    }
}

void Utils::setDirPermissions(QString dirpath, QFileDevice::Permissions permissions)
{
    QDir dir(dirpath);
    if (!dir.exists())
    return;
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isDir())
            setDirPermissions(fileInfo.absoluteFilePath(), permissions);
        else
        {
            QFile file(fileInfo.absoluteFilePath());
            file.setPermissions(permissions);
        }
    }
}

void Utils::copyWithPermissions(QFile &file, QString path, QFileDevice::Permissions permissions)
{
    file.copy(path);
    QFile CO(path);
    CO.setPermissions(permissions);
}

bool Utils::removeWithoutPermissions(QFile &file)
{
    file.setPermissions(QFileDevice::ReadOther | QFileDevice::WriteOther
                        | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                        | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                        | QFileDevice::ReadUser   | QFileDevice::WriteUser);
    return file.remove();
}

double Utils::mmToInches(double mm )  { return mm * 0.039370147; }

QUrl   Utils::getExistingDirectoryUrl(QWidget *parent, QString title, QUrl Dirdefaut, QStringList listnomsaeliminer, bool ExclureNomAvecEspace)
{
    /*! il faut utiliser la fonction static QFileDialog::getExistingDirectoryUrl() parce que la QFileDialog implémentée dans Qt ne donne pas accès aux lecteurs réseaux sous linux
     * avec la fonction static, on utilise la boîte de dialog du système
     * bien sûr, il faut paramétrer le fstab sous linux pour que le dossier réseau soit ouvert automatiquement au moment du boot*/
    QUrl url = Dirdefaut;
    url = QFileDialog::getExistingDirectoryUrl(parent, title, url, QFileDialog::ShowDirsOnly);
    if (url.path() == "")
        return QUrl();
    if (ExclureNomAvecEspace)
            if (url.path().contains(" "))
            {
                UpMessageBox::Watch(parent, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
                return QUrl();
            }
    if (listnomsaeliminer.contains(url.path()))
    {
        UpMessageBox::Watch(parent, tr("Nom de dossier non conforme"),tr("Le dossier doit être différent"));
        return QUrl();
    }
    return url;
}


QString Utils::PrefixePlus(double Dioptr)                          // convertit en QString signé + ou - les valeurs de dioptries issues des appareils de mesure
{
//    if  (Dioptr == 0.0)
//        return "0" + QString(QLocale().decimalPoint()) + "00";
    return (Dioptr > 0.0 ? "+" : "") + QLocale().toString(Dioptr,'f',2);
}

/*! ++++ PLUS UTILISE - trop sensible aux choix de jeu de caractère et marche mal avec les blobs
 * \brief Utils::DecomposeScriptSQL(QString nomficscript)
 * Cette fonction va décomposer un script SQL en une suite d'instructions SQL utilisables par Qt
 * \param l'emplacement du fichier à traiter
 * \return une QStringList avec la liste des instructions
 * +++ ne marche pas toujours mais suffisant pour un script de sauvegarde de BDD généré par mysqldump
 *  QStringList listinstruct = DecomposeScriptSQL(QDir::homePath() + "/Documents/Rufus/Ressources/dump.sql");
    bool e = true;
        foreach(const QString &s, listinstruct)
        if (!db->StandardSQL(s))
        {
            e = false;
            break;
        }
        a = (e? 0:99);
        if (a==0)
        {
            ...
        }
 */
QStringList Utils::DecomposeScriptSQL(QString nomficscript)
{
    QStringList listinstruct;
    QFile file(nomficscript);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        listinstruct << "";
        return QStringList();
    }
    QString queryStr(file.readAll());
    file.close();
    QRegularExpression re;
    // On retire tous les commentaires, les tabulations, les espaces ou les retours à la ligne multiples
    //        queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    re.setPattern("(\\/\\*(.|\\n)*?\\*\\/)");
    queryStr = queryStr.replace(re, "");
    re.setPattern("(^;\\n)");
    queryStr = queryStr.replace(re, "");
    re.setPattern("(--.*\\n)");
    queryStr = queryStr.replace(re, "\n");
    re.setPattern("( +)");
    queryStr = queryStr.replace(re, " ");
    re.setPattern("((\\t)+)");
    queryStr = queryStr.replace(re, " ");
    re.setPattern("(^ *)");
    queryStr = queryStr.replace(re, "");
    re.setPattern("((\\n)+)");
    queryStr = queryStr.replace(re, "\n");
    //Retire les espaces en début et fin de string
    queryStr = queryStr.trimmed();

    QString matched, delimiter, Atraiter;
    re.setPattern("^(\\s|\\n)*DELIMITER\\s*(.|\\n)*END\\s*.\\n"); //isole les créations de procédure SQL dans le script

    while (queryStr.size()>0 && queryStr.contains(";"))
    {
        //Edit(queryStr);
        QRegularExpressionMatch match = re.match(queryStr);
        if (match.hasMatch())  // --> c'est une procédure à créer
        {
                matched     = match.capturedTexts().at(0);
                Atraiter    = matched.trimmed();
                //Edit(Atraiter);
                delimiter   = Atraiter.data()[Atraiter.size()-1];
                //Edit(delimiter);
                re.setPatternOptions(QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption);
                re.setPattern("DELIMITER\\s*");
                Atraiter.replace(re,"");
                Atraiter.replace(delimiter,"");
                re.setPattern("^ *)");
                Atraiter.replace(re,"");
                re.setPattern("(^(\\n)+)");
                Atraiter.replace(re,"");
                re.setPattern("((\\n)+)");
                Atraiter.replace(re,"\n");

                //Edit(Atraiter);
                queryStr.replace(0,matched.size(),"");
        }
        else                    // -- c'est une requête SQL
        {
                matched = queryStr.split(";\n").at(0);
                Atraiter = matched.trimmed()+ ";";
                queryStr.replace(0,matched.size()+2,"");
                re.setPattern("((\\n)+)");
                queryStr = queryStr.replace(re, "\n");
        }
        re.setPattern("(^(\\n)*)");
        queryStr = queryStr.replace(re, "");
        listinstruct << Atraiter;
    }
    return listinstruct;

    /* POUR CREER DES PROCEDURES AVEC Qt - cf fichier créer des procédures mysql avec QSt dans /assets/diagrams */
}

QString Utils::ConvertitModePaiement(QString mode)
{
    if (mode == ESP)        mode = QObject::tr(ESPECES);
    else if (mode == CB)    mode = QObject::tr(CARTECREDIT);
    else if (mode == TP)    mode = QObject::tr(TIP);
    else if (mode == VRMT)  mode = QObject::tr(VIREMENT);
    else if (mode == PLVMT) mode = QObject::tr(PRELEVEMENT);
    else if (mode == CHQ)   mode = QObject::tr(CHEQUE);
    else if (mode == GRAT)  mode = QObject::tr(GRATUIT);
    else if (mode == IMP)   mode = QObject::tr(IMPAYE);
    return mode;
}

void Utils::CalcBlobValueSQL(QVariant &newvalue)
{
    newvalue = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : QVariant(newvalue));
}

void Utils::CalcStringValueSQL(QVariant &newvalue)
{
    newvalue = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + correctquoteSQL(newvalue.toString()) + "'");
}

void Utils::CalcintValueSQL(QVariant &newvalue)
{
    newvalue = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
}

void Utils::CalcdoubleValueSQL(QVariant &newvalue)
{
    newvalue = (newvalue == QVariant()? "null" : QString::number(newvalue.toDouble()));
}

void Utils::CalcDateValueSQL(QVariant &newvalue)
{
    newvalue = ((newvalue == QVariant() || !newvalue.toDate().isValid())? "null" : "'" + newvalue.toDate().toString("yyyy-MM-dd") + "'");
}

void Utils::CalcTimeValueSQL(QVariant &newvalue)
{
    newvalue = ((newvalue == QVariant() || !newvalue.toTime().isValid())? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");
}
void Utils::CalcDateTimeValueSQL(QVariant &newvalue)
{
    newvalue = ((newvalue == QVariant() || !newvalue.toDateTime().isValid())? "null" : "'" + newvalue.toDateTime().toString("yyyy-MM-dd HH:mm:ss") + "'");
}

/*!
 *  \brief Calcul de l'âge
 *
 *  Methode qui permet ????
 *
 *  \param datedenaissance : la date de naissance
 *  \param Sexe : le sexe de la personne [""]
 *  \param datedujour : la date du jour [Date du jour]
 *
 *  \return un object contenant :
 * toString : une chaine de caractères ( ex: 2 ans 3 mois )
 * annee : l'age brut de la personne
 * mois :
 * icone : l'icone à utiliser [man women, girl, boy, kid, baby]
 * formule : une valeur parmi [l'enfant, la jeune, le jeune, madame, monsieur]
 *
 */
QMap<QString,QVariant> Utils::CalculAge(QDate datedenaissance)
{
    return Utils::CalculAge(datedenaissance, "", QDate::currentDate());
}
QMap<QString,QVariant> Utils::CalculAge(QDate datedenaissance, QDate datedujour)
{
    return Utils::CalculAge(datedenaissance, "", datedujour);
}
QMap<QString,QVariant> Utils::CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour)
{
    QMap<QString,QVariant>  Age;
    int         AnneeNaiss, MoisNaiss, JourNaiss;
    int         AnneeCurrent, MoisCurrent, JourCurrent;
    int         AgeAnnee, AgeMois;
    int         FormuleMoisJourNaissance, FormuleMoisJourAujourdhui;

    AnneeNaiss                  = datedenaissance.toString("yyyy").toInt();
    MoisNaiss                   = datedenaissance.toString("MM").toInt();
    JourNaiss                   = datedenaissance.toString("dd").toInt();
    AnneeCurrent                = datedujour.toString("yyyy").toInt();
    MoisCurrent                 = datedujour.toString("MM").toInt();
    JourCurrent                 = datedujour.toString("dd").toInt();
    FormuleMoisJourNaissance    = (MoisNaiss*100) + JourNaiss;
    FormuleMoisJourAujourdhui   = (MoisCurrent*100) + JourCurrent;
    AgeAnnee                    = AnneeCurrent - AnneeNaiss;
    AgeMois                     = MoisCurrent - MoisNaiss;
    if (FormuleMoisJourAujourdhui < FormuleMoisJourNaissance)   AgeAnnee --;
    if (JourNaiss > JourCurrent)                                AgeMois --;
    if (AgeMois < 0)                                            AgeMois = AgeMois + 12;

    Age["annee"] = AgeAnnee;
    Age["mois"]  = AgeMois;

    // On formate l'âge pour l'affichage
    switch (AgeAnnee) {
    case 0:
        if (datedenaissance.daysTo(datedujour) > 31)
            Age["toString"]               = QString::number(AgeMois) + " mois";
        else
            Age["toString"]               = QString::number(datedenaissance.daysTo(datedujour)) + " jours";
        break;
    case 1: case 2: case 3: case 4:
        Age["toString"]                    = QString::number(AgeAnnee) + " an";
        if (AgeAnnee > 1) Age["toString"]  = Age["toString"].toString() + "s";
        if (AgeMois > 0)  Age["toString"]  = Age["toString"].toString() + " " + QString::number(AgeMois) + " mois";
        break;
    default:
        Age["toString"]                    = QString::number(AgeAnnee) + " ans";
        break;
    }

    // On cherche l'icone correspondant au mieux à la personne
    QString img = "silhouette";
    if (AgeAnnee < 2)                       img = "baby";
    else if (AgeAnnee < 8)                  img = "kid";
    else if (AgeAnnee < 16 && Sexe == "M")  img = "boy";
    else if (AgeAnnee < 16 && Sexe == "F")  img = "girl";
    else if (Sexe =="M")                    img = "man";
    else if (Sexe =="F")                    img = "women";
    Age["icone"] = img;

    // On cherche la formule de polistesse correspondant au mieux à la personne
    QString formule = "";
    if (AgeAnnee < 11)                  formule = "l'enfant";
    else if (AgeAnnee < 18) {
        if (Sexe == "F")                formule = "la jeune";
        if (Sexe == "M")                formule = "le jeune";
    }
    else {
        if (Sexe == "F")                formule = "madame";
        if (Sexe == "M")                formule = "monsieur";
    }
    Age["formule"] = formule;

    return Age;
}

//! renvoie la valeur littérale d'un enum (à condition d'avoir placé la macro Q_ENUM(nomdelenum) dans la définition de l'enum
//! utilisé comme ça
//!    qDebug() << Utils::EnumDescription(QMetaEnum::fromType<nomdelenum>(), valeurdelenum);
/*! ex
 *     enum METIER {Ophtalmo, Orthoptiste, AutreSoignant, NonSoignant, SocieteComptable, NoMetier};    Q_ENUM(METIER) *
 *     qDebug() << User::METIER;
 * ->  0
 *     qDebug() << Utils::EnumDescription(QMetaEnum::fromType<User::METIER>(), 0);
 * ou  qDebug() << Utils::EnumDescription(QMetaEnum::fromType<User::METIER>(), User::Ophtalmo);
 * ->  "User::Ophtalmo"
 */
QString Utils::EnumDescription(QMetaEnum metaEnum, int val)
{
    return metaEnum.valueToKey(val);
}


//! calcule la taille idéale en points d'une police pour l'adapter aux fenêtres Rufus
//! \param la QFont
//! \return int -> la taille en points
void Utils::CalcFontSize(QFont &font)
{
    for (int i = 5; i < 30; i++)
    {
        font.setPointSize(i);
        QFontMetrics fm(font);
        int Htaille = fm.width("date de naissance");
        if (Htaille > 108 || fm.height()*1.1 > 20)
        {
            font.setPointSize(i-1);
            i=30;
        }
    }
}

//! renvoie une couleur
QColor Utils::SelectCouleur(QColor colordep, QWidget *parent)
{
    QColorDialog dlg(colordep, parent);
    dlg.exec();
    QColor colorfin = dlg.selectedColor();
    return  colorfin;
}


void Utils::setDataString(QJsonObject data, QString key, QString &prop, bool useTrim)
{
    if( data.contains(key) )
    {
        QString str = data[key].toString();
        if( useTrim )
            str = Utils::trim(str);
        prop = str;
    }
}
void Utils::setDataInt(QJsonObject data, QString key, int &prop)
{
    if( data.contains(key) )
        prop = data[key].toInt();
}
void Utils::setDataLongLongInt(QJsonObject data, QString key, qlonglong &prop)
{
    if( data.contains(key) )
        prop = data[key].toVariant().toLongLong();
}
void Utils::setDataDouble(QJsonObject data, QString key, double &prop)
{
    if( data.contains(key) )
        prop = data[key].toDouble();
}
void Utils::setDataBool(QJsonObject data, QString key, bool &prop)
{
    if( data.contains(key) )
        prop = data[key].toBool();
}
void Utils::setDataDateTime(QJsonObject data, QString key, QDateTime &prop)
{
    if( data.contains(key) )
    {
        double time = data[key].toDouble();
        QDateTime dt;
        dt.setMSecsSinceEpoch( qint64(time) );
        prop = dt;
    }
}
void Utils::setDataTime(QJsonObject data, QString key, QTime &prop)
{
    if( data.contains(key) )
        prop = QTime::fromString(data[key].toString(),"HH:mm:ss");
}
void Utils::setDataDate(QJsonObject data, QString key, QDate &prop)
{
    if( data.contains(key) )
        prop = QDate::fromString(data[key].toString(),"yyyy-MM-dd");
}
void Utils::setDataByteArray(QJsonObject data, QString key, QByteArray &prop)
{
  if( data.contains(key) )
        prop = QByteArray::fromBase64(data[key].toString().toLatin1());
}
void Utils::setDataLogic(QJsonObject data, QString key, Logic &prop)
{
    if( data.contains(key) )
        prop = (data[key].toBool()? True : False);
    else
        prop = Null;
}

void Utils::EnChantier(bool avecMsg)
{
    UpMessageBox msgbox;
    msgbox.setIconPixmap(Icons::pxWorkInProgress());
    UpSmallButton OKBouton;
    if (avecMsg)
    msgbox.setInformativeText(tr("Le code qui suit n'est pas achevé et entraînera\nassez rapidement un plantage du programme\navec un risque élevé de corruption des données"));
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
}


//! convertit un côté en QString : droit = "D", Gauche = "G", Les 2 = "2"
Utils::Cote Utils::ConvertCote(QString cote)
{
    if (cote == "D") return Droit;
    if (cote == "G") return Gauche;
    if (cote == "2") return Les2;
    return  NoLoSo;
}

QString Utils::ConvertCote(Cote cote)
{
    switch (cote) {
    case Gauche:        return "G";
    case Droit:         return "D";
    case Les2:          return "2";
    default: return "";
    }
}

QString Utils::TraduitCote(QString cote)
{
    if (cote == "D") return tr("Droit");
    if (cote == "G") return tr("Gauche");
    if (cote == "2") return tr("Les 2");
    return  "";
}

QString Utils::TraduitCote(Cote cote)
{
    switch (cote) {
    case Gauche:        return tr("Gauche");
    case Droit:         return tr("Droit");
    case Les2:          return tr("Les 2");
    default: return "";
    }
}

QList<QImage> Utils::calcImagefromPdf(QByteArray ba)
{
    QList<QImage> listimg = QList<QImage>();
    Poppler::Document* document = Poppler::Document::loadFromData(ba);
    if (!document || document->isLocked()) {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de charger le document"));
        delete document;
        return listimg;
    }
    if (document == Q_NULLPTR) {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de charger le document"));
        delete document;
        return listimg;
    }
    document->setRenderHint(Poppler::Document::TextAntialiasing);
    for (int i=0; i< document->numPages(); ++i)
    {
        Poppler::Page* pdfPage = document->page(i);
        if (pdfPage == Q_NULLPTR) {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
            delete document;
            return listimg;
        }
        QImage image = pdfPage->renderToImage(300,300);
        if (image.isNull()) {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
            delete document;
            return listimg;
        }
        listimg << image;

        /*! pour vérifier que ça marche */
        //AfficheImage(image);
    }
    delete document;
    return listimg;
}

void Utils::AfficheImage(QImage img)
{
    UpDialog *dlg   = new UpDialog();
    QPixmap pix     = QPixmap();
    pix             = QPixmap::fromImage(img.scaled(QSize(1050-2, 1485-2), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    UpLabel *lab    = new UpLabel();
    lab             ->resize(pix.width(), pix.height());
    lab             ->setPixmap(pix);
    dlg->dlglayout()->insertWidget(0,lab);
    dlg             ->AjouteLayButtons();
    dlg             ->exec();
    delete dlg;
}

QJsonValue Utils::jsonValFromImage(const QImage &img)
{
  QBuffer buffer;
  buffer.open(QIODevice::WriteOnly);
  img.save(&buffer, "JPG");
  QByteArray const encoded = buffer.data().toBase64();
  return {QLatin1String(encoded)};
}

QImage Utils::imagemapFrom(const QJsonValue &val)
{
    QByteArray const encoded = val.toString().toLatin1();
    return QImage::fromData(QByteArray::fromBase64(encoded), "JPG");
}

/*!
  reconstruit la liste des ports COM disponibles sur le système  sous la forme (COMxx,nomgeneriqueduport)
*/
QMap<QString, QString> Utils::ReconstruitMapPortsCOM()
{
    QMap<QString,QString> mapports=  QMap<QString,QString> ();
    QString portappareil ("");
    QList<QSerialPortInfo> availableports = QSerialPortInfo::availablePorts();
    if (availableports.size() == 0)
        return mapports;
    for (int i=0; i<availableports.size(); i++)
    {
        QString nomgeneriqueduport = availableports.at(i).portName();
        if (nomgeneriqueduport.contains("usbserial"))
        {
            QString lastchar = nomgeneriqueduport.right(1);
            QString firstchar = nomgeneriqueduport.split("-").at(1).left(1);
            /*!
         * nom des ports sous BigSur  = "usbserial-F******" + no 0,1,2 ou 3
         * on peut aussi avoir un truc du genre "usbserial-A906IXA8" avec certaines clés
         * nom des ports sous driver FTDI (Startech) = "usbserial-FT0G2WCR" + lettre A,B,C ou D
        */
            if (lastchar == "0" ||  lastchar == "A" || firstchar == "A")
                mapports.insert(COM1, nomgeneriqueduport);
            else if (lastchar == "1" ||  lastchar == "B" || firstchar == "B")
                mapports.insert(COM2, nomgeneriqueduport);
            else if (lastchar == "2" ||  lastchar == "C" || firstchar == "C")
                mapports.insert(COM3, nomgeneriqueduport);
            else if (lastchar == "3" ||  lastchar == "D" || firstchar == "D")
                mapports.insert(COM4, nomgeneriqueduport);
            else if (lastchar == "4" ||  lastchar == "E" || firstchar == "E")
                mapports.insert(COM5, nomgeneriqueduport);
            else if (lastchar == "5" ||  lastchar == "F")
                mapports.insert(COM6, nomgeneriqueduport);
            else if (lastchar == "6" ||  lastchar == "G")
                mapports.insert(COM7, nomgeneriqueduport);
            else if (lastchar == "7" ||  lastchar == "H")
                mapports.insert(COM8, nomgeneriqueduport);
        }
        else if (nomgeneriqueduport.contains("ttyUSB"))
        {
            QString lastchar = nomgeneriqueduport.at(nomgeneriqueduport.size() - 1);
            if (lastchar == "0")
                mapports.insert(COM1, nomgeneriqueduport);
            else if (lastchar == "1")
                mapports.insert(COM2, nomgeneriqueduport);
            else if (lastchar == "2")
                mapports.insert(COM3, nomgeneriqueduport);
            else if (lastchar == "3")
                mapports.insert(COM4, nomgeneriqueduport);
            else if (lastchar == "4")
                mapports.insert(COM5, nomgeneriqueduport);
            else if (lastchar == "5")
                mapports.insert(COM6, nomgeneriqueduport);
            else if (lastchar == "6")
                mapports.insert(COM7, nomgeneriqueduport);
            else if (lastchar == "7")
                mapports.insert(COM8, nomgeneriqueduport);
        }
#ifdef Q_OS_WIN
        else if (nomgeneriqueduport.left(3) == "COM")
            mapports.insert(nomgeneriqueduport, nomgeneriqueduport);
#endif
    }
    return mapports;
}


void Utils::writeDatasSerialPort (QSerialPort *port, QByteArray datas, QString msgdebug, int timetowaitms)
{
    qint32 baud = port->baudRate();
    if (timetowaitms == 0)
    {
        timetowaitms= int (datas.size()*8*1000 / baud);
        timetowaitms += 10;
    }
    //qDebug() << msgdebug << "timetowaitms" << timetowaitms;
    port->write(datas);
    port->flush();
    port->waitForBytesWritten(timetowaitms);
}

//! récupérer l'index d'une valeur dans un QMetaEnum
int Utils::getindexFromValue(const QMetaEnum & e, int value)
{
    for(int i=0; i< e.keyCount(); i++){
        if(e.key(i) == e.valueToKey(value))
            return i;
    }
    return -1;
};
