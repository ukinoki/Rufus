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
int Utils::correctedwidth(int width)
{
#ifdef Q_OS_WINDOWS
    double m_larg = 0.8;
# else
    double m_larg = 1;
# endif
    return int(width * m_larg);
}



/*
 * Initialisation des variables static const
*/
QRegularExpression const Utils::rgx_rx                  = QRegularExpression("[\\w' \\-]*", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression const Utils::rgx_AlphaNumeric        = QRegularExpression("[A-Za-z0-9]*");
QRegularExpression const Utils::rgx_AlphaNumeric_3_12   = QRegularExpression("[A-Za-z0-9]{3,12}$");
QRegularExpression const Utils::rgx_AlphaNumeric_5_15   = QRegularExpression("[A-Za-z0-9]{5,15}$");
QRegularExpression const Utils::rgx_AlphaNumeric_5_12   = QRegularExpression("[A-Za-z0-9]{5,12}$");
QRegularExpression const Utils::rgx_MajusculeSeul       = QRegularExpression("[A-Z]*");
QRegularExpression const Utils::rgx_Question            = QRegularExpression("[\\w'°, \\-]*[?]*", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression const Utils::rgx_IPV4                = QRegularExpression("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
QRegularExpression const Utils::rgx_IPV4_mask           = QRegularExpression("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                                                                            "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                                                                            "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                                                                            "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");

QRegularExpression const Utils::rgx_mail                = QRegularExpression("^[A-Za-z0-9_-]+(.[A-Za-z0-9_-]+)+@[A-Za1-z0-9_-]+(.[A-Za1-z0-9_-]+).[A-Za-z0-9_-]{2,6}");
QRegularExpression const Utils::rgx_NNI                 = QRegularExpression("[12][0-9]{14}");

QRegularExpression const Utils::rgx_adresse             = QRegularExpression("[\\w'°, \\-]*", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression const Utils::rgx_intitulecompta      = QRegularExpression("[\\w'°, \\-/%]*", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression const Utils::rgx_CP                  = QRegularExpression(cp());
QRegularExpression const Utils::rgx_ville               = QRegularExpression("[\\w' \\-]*", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression const Utils::rgx_telephone           = QRegularExpression("[0-9 ]*");

QRegularExpression const Utils::rgx_tabac               = QRegularExpression("[0-9]{2}");
QRegularExpression const Utils::rgx_cotation            = QRegularExpression("[a-zA-Z0-9.+/ ]*");

QRegularExpression const Utils::rgx_recherche           = QRegularExpression("[\\w' %\\-]*");


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

bool Utils::IsCharSpecial( QChar c)
{
  if( c == ' ' || c == '-' || c == '\'' ) return true;
  return false;
}

bool Utils::IsCharNL( QChar c)
{
  if( c == '\n' ) return true;
  return false;
}

bool Utils::IsCharCR( QChar c)
{
  if( c == '\r' ) return true;
  return false;
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

        if( IsCharSpecial(c) )
            textC = textC.remove(0,1);
        else
            break;
    }

    if( end )                               // enlève les espaces, les tirets et les apostrophes de la fin
        while( textC.size() )
        {
            int lastIndex = textC.size() - 1;
            c = textC.at(lastIndex);
            if( IsCharSpecial(c) )
                textC = textC.remove(lastIndex,1);
            else
                break;
        }

    if( removereturnend )                   // enlève les retours à la ligne de la fin
        while( textC.size() )
        {
            int lastIndex = textC.size() - 1;
            c = textC.at(lastIndex);
            if( c == '\n' )
                textC = textC.remove(lastIndex,1);
            else
                break;
        }

    QString newText = "";
    QChar lastChar;
    for( int i=0; i < textC.size(); ++i )   // enlève les espaces, les tirets et les apostrophes en doublon
    {
        c = textC.at(i);
        if( IsCharSpecial(lastChar) )
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
        if( IsCharSpecial(lastChar) )
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
    QRegularExpression re;
    re.setPattern("[éêëè]");
    nom.replace(re,"e");
    re.setPattern("[ÉÈÊË]");
    nom.replace(re,"E");
    re.setPattern("[àâ]");
    nom.replace(re,"a");
    re.setPattern("[ÂÀ]");
    nom.replace(re,"A");
    re.setPattern("[îï]");
    nom.replace(re,"i");
    re.setPattern("[ÏÎ]");
    nom.replace(re,"I");
    re.setPattern("[ôö]");
    nom.replace(re,"o");
    re.setPattern("[ÔÖ]");
    nom.replace(re,"O");
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
    nettoieHtmlOldQt(text);
    return retirelignevidefinhtml(text);
}

void Utils::nettoieHtmlOldQt(QString &text, bool converttohtml)
{
    if (text.contains("<!DOCTYPE HTML PUBLIC"))
    {
        //! parce que de vielles versions de QT enregistraient la police avec tout un lot d'attributs et Qt6 ne comprend pas
        epureFontFamily(text);
        if (!text.contains(HTMLCOMMENT))
        {
            QString newsize = "font-size:" + QString::number(qApp->font().pointSize()) + "pt";
            QRegularExpression rs;
            rs.setPattern("font-size( *: *[\\d]{1,2} *)pt");
            QRegularExpressionMatch const match = rs.match(text);
            if (match.hasMatch()) {
                QString matcheds = match.captured(0);
                text.replace(matcheds, newsize);
            }
        }
    }
    QTextEdit textprov;
    textprov.setText(text);
    if (converttohtml)
        text = textprov.toHtml();
    else
        text = textprov.toPlainText();
}


/*!
 *  \brief convertPlainText
 *  convertir un QString en plaintext
 *  on écrit le QString dans un QtextEdit et on récupère le plaintext avec QTextEdit::toPlainText()
 *  on retire les lignes vides de la fin
 */
void Utils::convertPlainText(QString &text)
{
    nettoieHtmlOldQt(text, false);
    text = trim(text, true, true);
}

/*!
 * \brief nettoieHTML
 *      nettoyer tous les trucs inutiles dans un html généré par QT
 *      placer les marqueurs Linux ou Mac
 * \param supprimeLesLignesVidesDuMilieu - comme son nom l'indique
 */
void Utils::nettoieHTML(QString &text, int fontsize, bool supprimeLesLignesVidesDuMilieu)
{
    QRegularExpression reg1;
    reg1.setPattern("<p style=\"-qt-paragraph-type:empty; "
                              "margin-top:[0-9]{1,2}px; margin-bottom:[0-9]{1,2}px; "
                              "margin-left:[0-9]{1,2}px; margin-right:[0-9]{1,2}px; "
                              "-qt-block-indent:0; text-indent:[0-9]{1,2}px;\"><br /></p>");
    QRegularExpression reg2;
    reg2.setPattern("<p style=\" margin-top:0px; margin-bottom:0px; "
                              "margin-left:[0-9]{1,2}px; margin-right:[0-9]{1,2}px; "
                              "-qt-block-indent:0; text-indent:[0-9]{1,2}px;\">");
    bool remetunelignealafin = convertHTML(text);
    if (supprimeLesLignesVidesDuMilieu)
        text.remove(reg1);
    text.replace(reg2,"<p style=\" margin-top:0px; margin-bottom:0px;\">");
    text.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
    text.remove(HTMLCOMMENT);
    if (remetunelignealafin)
        text.append(HTML_FINPARAGRAPH);
    text.append(HTMLCOMMENT);
    if (fontsize>0)
    {
        QRegularExpression rx;
        rx.setPattern("font-size( *: *[\\d]{1,2} *)pt");
        text.replace(rx,"font-size:" + QString::number(fontsize) + "pt");
    }
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

        if ( txthtml.mid(debut,longARetirer).contains( QString("-qt-paragraph-type:empty;")) )
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
    QRegularExpression rx;
    rx.setPattern("font-family:'([a-zA-Z0-9 ,-]+)");
    auto it = rx.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString txtaremplacer = match.captured(0);
        if (txtaremplacer != "")
        {
            QString replacmt = txtaremplacer.split(",").at(0);
            text.replace(txtaremplacer, replacmt);
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
 * il y a donc 2 tables dont la largeur est déterminée par les macros
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
    QRegularExpression rx;
    QString patternALDG = "float: left;\" cellpadding=\"5\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    QString patternALDD = "float: right;\" cellpadding=\"6\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    QString patternG    = "float: left;\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    QString patternD    = "float: right;\">([\\n ]*)<tr><td width=\"([\\d]{3})\">";
    rx.setPattern(ALD? patternALDG : patternG);
    auto it = rx.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString txtaremplacer = match.captured(0);
        if (txtaremplacer != "")
        {
            patternALDG = "float: left;\" cellpadding=\"5\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_GAUCHE_ALD "\">";
            patternG    = "float: left;\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_GAUCHE "\">";
            rx.setPattern(ALD? patternALDG : patternG);
            auto it2 = rx.globalMatch(txtaremplacer);
            if (!it2.hasNext())
                 text.replace(txtaremplacer, ALD? largeurALDG : largeurG);
        }
    }
    rx.setPattern(ALD? patternALDD : patternD);
    it = rx.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString txtaremplacer = match.captured(0);
        if (txtaremplacer != "")
        {
            patternALDD = "float: right;\" cellpadding=\"6\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_DROITE_ALD "\">";
            patternD    = "float: right;\">([\\n ]*)<tr><td width=\"" HTML_LARGEUR_ENTETE_DROITE "\">";
            rx.setPattern(ALD? patternALDD : patternD);
            auto it2 = rx.globalMatch(txtaremplacer);
            if (!it2.hasNext())
                 text.replace(txtaremplacer, ALD? largeurALDD : largeurD);
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
        int x   = int(QFontMetrics(fm).horizontalAdvance(lmsg.at(k))*correction);
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
        removeWithoutPermissions(fileresize);
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
            copyWithPermissions(CC, CheminEchecTransfrDir + "/" + filename);
        }
        return false;
    }
    removeWithoutPermissions(CC);
    /* on comprime*/
    int tauxcompress = 90;
    while (sz > TAILLEMAXIIMAGES && tauxcompress > 1)
    {
        pixmap.save(nomfichresize, "jpeg",tauxcompress);
        sz = fileresize.size();
        tauxcompress -= 10;
    }
    copyWithPermissions(fileresize, pathfile);
    fileresize.close();
    removeWithoutPermissions(fileresize);
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

QByteArray Utils::StringToArray(QString source)
{
    QByteArray ba;
    ba = source.toLocal8Bit();
    return ba;
}

QByteArray Utils::IntToArray(int source)
{
    //permet d'éviter le cast
 #ifdef Q_OS_WIN
    QByteArray ba((const char *) &source, sizeof(int));
 #else
    QByteArray ba;
    //QDataStream data(&ba, QIODevice::ReadWrite);
    //data << source;
    ba = QByteArray::number(source);
#endif
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
        {
            foreach (const QNetworkAddressEntry &address, netInterface.addressEntries())
            {
                if(address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    IPadress = address.ip().toString();
                    break;
                }
            }
        }
        if (IPadress != "")
            break;
    }

    return IPadress;
}

// https://doc-snapshots.qt.io/qt6-dev/qtcore-changes-qt6.html
bool Utils::RegularExpressionMatches(QRegularExpression rgx, QString s, bool exact)
{
    QRegularExpression re = rgx;
    if( exact )
      re = QRegularExpression(QRegularExpression::anchoredPattern(rgx.pattern()));

    QRegularExpressionMatch m =re.match(s);
    return m.hasMatch();
}

QString Utils::calcIP(QString IP, bool aveczero)
{
    //QRegularExpressionMatch::hasMatch():

    if (!RegularExpressionMatches(rgx_IPV4,IP))
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
        QRegularExpression re;
        re.setPattern("\\s+");
        QStringList list = result.split(re);
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
                UtilsMessageBox::Watch(dlg_askMDP, QObject::tr("Mot de passe invalide!"));
        });
        connect(ConfirmMDP, &UpLineEdit::returnPressed, dlg_askMDP->OKButton, &QPushButton::click);
        dlg_askMDP->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
        dlg_askMDP->dlglayout()->setSpacing(8);
        mdpval = ConfirmMDP->text();
        mdpverified = (dlg_askMDP->exec() == QDialog::Accepted);
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
        if (quest.exec() == QDialog::Accepted)
        {
            if (calcSHA1(quest.textValue()) == MDP)
                return true;
            else if (quest.textValue() == MDP)
                return true;
            else
                UtilsMessageBox::Watch(Q_NULLPTR, QObject::tr("Mot de passe invalide!"));
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

void Utils::setPermissions(QFile &file, QFileDevice::Permissions permissions)
{
    file.setPermissions(permissions);
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
#ifdef Q_OS_WIN
    if( url.path().startsWith("/") )
    {
        url.setPath(url.path().last(url.path().length()-1));
    }
#endif
    if (ExclureNomAvecEspace)
            if (url.path().contains(" "))
            {
                UtilsMessageBox::Watch(parent, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
                return QUrl();
            }
    if (listnomsaeliminer.contains(url.path()))
    {
        UtilsMessageBox::Watch(parent, tr("Nom de dossier non conforme"),tr("Le dossier doit être différent"));
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
 * mois : le nombre de mois dans l'année
 * icone : l'icone à utiliser [man women, girl, boy, kid, baby]
 * formule_politesse : une valeur parmi [l'enfant, la jeune, le jeune, madame, monsieur]
 *
 */
QMap<QString,QVariant> Utils::CalculAge(QDate datedenaissance, QDate datedujour, QString Sexe)
{
    QMap<QString,QVariant>  Age = {{"annee", ""}, {"mois", ""}, {"toString", ""}, {"icone",""}, {"formule_politesse",""}};

    if (!datedenaissance.isValid())
        return Age;

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
    Age["formule_politesse"] = formule;

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
        int Htaille = fm.horizontalAdvance("date de naissance");
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

void Utils::EnChantier(bool avecMsg)
{
    UtilsMessageBox msgbox;
    msgbox.setIconPixmap(Icons::pxWorkInProgress());
    UpSmallButton OKBouton;
    if (avecMsg)
    msgbox.setInformativeText(tr("Le code qui suit n'est pas achevé et entraînera\nassez rapidement un plantage du programme\navec un risque élevé de corruption des données"));
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
}


QList<QImage> Utils::calcImagefromPdf(QString filename)
{
    QList<QImage> listimg = QList<QImage>();
    QPdfDocument pdf;
    pdf.load(filename);
    for (int i=0; i<pdf.pageCount(); i++)
    {
        QPdfDocumentRenderOptions renderpdf;
        renderpdf.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::None);
        QSize pageSize = pdf.pagePointSize(i).toSize();
        pageSize.rheight() *= 5;
        pageSize.rwidth() *= 5;
        QImage image = pdf.render(i, pageSize,renderpdf);
        listimg << image;
    }
    return listimg;
}

QList<QImage> Utils::calcImagefromPdf(QByteArray ba)
{
    QList<QImage> listimg = QList<QImage>();
    QPdfDocument pdf;
    QBuffer *buff = new QBuffer(&ba);
    buff->open(QIODevice::ReadWrite);
    pdf.load(buff);
    for (int i=0; i<pdf.pageCount(); i++)
    {
        QPdfDocumentRenderOptions renderpdf;
        renderpdf.setRenderFlags(QPdfDocumentRenderOptions::RenderFlag::None);
        QSize pageSize = pdf.pagePointSize(i).toSize();
        pageSize.rheight() *= 5;
        pageSize.rwidth() *= 5;
        QImage image = pdf.render(i, pageSize,renderpdf);
        listimg << image;

        /*! Pour vérifier que ça marche */
        //AfficheImage(image);
    }
    delete buff;
    return listimg;
}

void Utils::AfficheImage(QImage img)
{
    UpDialog *dlg = new UpDialog();
    QPixmap pix = QPixmap();
    pix = QPixmap::fromImage(img).scaled(QSize(210-2,297-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    UpLabel *lab = new UpLabel();
    lab->resize(pix.width(),pix.height());
    lab->setPixmap(pix);
    dlg->layout()->addWidget(lab);
    dlg->AjouteLayButtons();
    dlg->exec();
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
   reconstruit la liste des ports COM disponibles sur le système (COM1,COM2,COM3, COM4...etc...) à partir de la liste des noms physiques des ports disponibles
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


void Utils::writeDataToFileDateTime (QByteArray data, QString name, QString path)
{
    if( !QDir(path).exists())
    {
        QDir().mkdir(path);
    }

    QDateTime now = QDateTime::currentDateTime();
    writeBinaryFile(data, path+"/"+now.toString("yyyyMMdd_HHmmss")+name);
}

void Utils::writeBinaryFile (QByteArray data, QString fileName)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        file.write(data);
        file.close();
    }
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


bool Utils::isSerialPort( QString name )
{
  if (name.contains("usbserial"))
  {
      return true;
  }
  if (name.contains("ttyUSB"))
  {
      return true;
  }
  if (name.contains("COM"))
  {
      return true;
  }
  return false;
}

void Utils::playAlarm(QString sound)
{
    QSoundEffect se = QSoundEffect();
    se.setSource(QUrl(sound));
    se.play();
}


//! récupérer l'index d'une valeur dans un QMetaEnum
int Utils::getindexFromValue(const QMetaEnum & e, int value)
{
    for(int i=0; i< e.keyCount(); i++){
        if(e.key(i) == e.valueToKey(value))
            return i;
    }
    return -1;
}

QByteArray Utils::cleanByteArray( QByteArray byteArray )
{
    QByteArray reponseDataClean;
    for( int i=0;i < byteArray.length(); i++ )
    {
        unsigned char c = byteArray.at(i);
        // avoid control chars except CR and LF
        if( c > 31 || c == 10 || c == 13 ) {
            reponseDataClean += c;
        }
    }
    return reponseDataClean;
}

UtilsMessageBox::UtilsMessageBox(QWidget *parent) : UpDialog(parent)
{
    wdg_iconlbl             = new UpLabel();
    wdg_texteditlbl         = new UpLabel();
    wdg_infolbl             = new UpLabel();
    wdg_textlayout          = new QVBoxLayout();
    wdg_infolayout          = new QHBoxLayout();
    wdg_ReponsSmallButton   = Q_NULLPTR;
    wdg_ReponsPushButton    = Q_NULLPTR;
    wdg_texteditlbl         ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    wdg_infolbl             ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

    wdg_textlayout      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    wdg_textlayout      ->addSpacerItem(new QSpacerItem(350,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    wdg_infolayout      ->addLayout(wdg_textlayout);
    wdg_infolayout      ->setSpacing(30);
    wdg_textlayout      ->setSpacing(5);
    wdg_textlayout      ->setContentsMargins(0,0,0,0);
    dlglayout()     ->insertLayout(0,wdg_infolayout);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
}

UtilsMessageBox::~UtilsMessageBox()
{
}

void UtilsMessageBox::addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton Style)
{
    button->setUpButtonStyle(Style);
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, this, [=] {Repons(button);});
}

void UtilsMessageBox::addButton(UpPushButton *button)
{
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, this, [=] {Repons(button);});
}

void UtilsMessageBox::removeButton(UpSmallButton *button)
{
    for (int i=0; i<buttonslayout()->count();i++)
    {
        UpSmallButton *buttonARetirer =  qobject_cast<UpSmallButton*>(buttonslayout()->itemAt(i)->widget());
        if (buttonARetirer!=Q_NULLPTR)
            if (buttonARetirer == button)
            {
                delete buttonARetirer;
                return;
            }
    }
}

void UtilsMessageBox::Repons(QPushButton *button)
{
    UpSmallButton *but = qobject_cast<UpSmallButton*>(button);
    if (but != Q_NULLPTR)
        wdg_ReponsSmallButton = but;
    else
        wdg_ReponsPushButton = qobject_cast<UpPushButton*>(button);
    accept();
}

UpSmallButton* UtilsMessageBox::clickedButton() const
{
    return wdg_ReponsSmallButton;
}

UpPushButton* UtilsMessageBox::clickedpushbutton() const
{
    return wdg_ReponsPushButton;
}

/*!
 * \brief UtilsMessageBox::setIcon
 * \param icn
 * \param animatedIcon if true = uses animated gif else uses png or ico
*/

void UtilsMessageBox::setIcon(enum Icon icn, bool animatedIcon)
{
    bool resize = true;
    switch (icn) {
    case Warning:
        if (animatedIcon)
        {
            setAnimatedIcon(WarningGif);
            resize = false;
        }
        else
            wdg_iconlbl     ->setPixmap(QPixmap("://damn-icon.png").scaled(80,80));
        break;
    case Quest:
        if (animatedIcon)
        {
            setAnimatedIcon(QuestionGif);
            resize = false;
        }
        else
            wdg_iconlbl ->setPixmap(QPixmap("://question.png").scaled(80,80));
        break;
    case Info:
        if (animatedIcon)
        {
            setAnimatedIcon(InfoGif);
            resize = false;
        }
        else
            wdg_iconlbl     ->setPixmap(QPixmap("://information.png").scaled(80,80));
        break;
    case Critical:
        wdg_iconlbl     ->setPixmap(QPixmap("://cancel.png").scaled(80,80));
        break;
    case Print:
        wdg_iconlbl     ->setPixmap(QPixmap("://11865.png").scaled(80,80));
        break;
    }
    if (!resize)
        return;
    wdg_iconlbl     ->setFixedSize(80,80);
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
}

void UtilsMessageBox::setAnimatedIcon(enum Movie movie)
{
    switch (movie) {
    case WarningGif:
        m_movie         = new QMovie(":/warning.gif");
        m_movie         ->setScaledSize(QSize(80,80));
        break;
    case InfoGif:
        m_movie         = new QMovie(":/info.gif");
        m_movie         ->setScaledSize(QSize(80,80));
        break;
    case QuestionGif:
        m_movie         = new QMovie(":/question.gif");
        m_movie         ->setScaledSize(QSize(100,100));
        m_movie         ->setSpeed(400);
        break;
    }
    wdg_iconlbl     ->setMovie(m_movie);
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
    m_movie         ->start();
}

void UtilsMessageBox::setIconPixmap(QPixmap pix)
{
    wdg_iconlbl     ->setPixmap(pix);
    wdg_iconlbl     ->setFixedSize(pix.width(),pix.height());
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
}

void UtilsMessageBox::UtilsMessageBox::setText(QString Text)
{
    if (Text == "")
        return;
    wdg_texteditlbl         ->setStyleSheet("border: 0px solid; background-color: rgba(200,200,200,0)");
    wdg_texteditlbl         ->setText("<b>" + Text + "</b>");
    wdg_texteditlbl         ->setWordWrap(true);
    wdg_texteditlbl         ->setFixedSize(Utils::CalcSize(Text));
    wdg_textlayout          ->insertWidget(1,wdg_texteditlbl);
}

void UtilsMessageBox::setInformativeText(QString Text)
{
    if (Text == "")
        return;
    wdg_infolbl     ->setStyleSheet("border: 0px solid; background-color: rgba(200,200,200,0)");
    wdg_infolbl     ->setText(Text);
    wdg_infolbl     ->setWordWrap(true);
    int position = 1;
    if (qobject_cast<QLabel*>(wdg_textlayout->itemAt(1)->widget()) != Q_NULLPTR)
        position += 1;
    wdg_infolbl     ->setFixedSize(Utils::CalcSize(Text));
    wdg_textlayout      ->insertWidget(position,wdg_infolbl);
    wdg_textlayout      ->setSizeConstraint(QLayout::SetFixedSize);
}

void UtilsMessageBox::setDefaultButton(QPushButton *butt)
{
    butt->setDefault(true);
    butt->setFocus();
}

void UtilsMessageBox::Show(QWidget *parent, QString Text, QString InfoText)
{
    UtilsMessageBox*msgbox     = new UtilsMessageBox(parent);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText.toHtmlEscaped());
    msgbox  ->setIcon(UtilsMessageBox::Quest);
    msgbox  ->AjouteLayButtons(UpDialog::ButtonOK);
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    msgbox  ->wdg_texteditlbl   ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl       ->setFixedSize(Utils::CalcSize(InfoText));

    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  qobject_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
            connect(butt, &QPushButton::clicked, msgbox, &UtilsMessageBox::accept);
    }
    msgbox  ->exec();
    delete msgbox;
}

UpSmallButton::StyleBouton UtilsMessageBox::Watch(QWidget *parent, QString Text, QString InfoText, Buttons Butts, QString link)
{
    UtilsMessageBox*msgbox     = new UtilsMessageBox(parent);

    msgbox->setIcon(Warning);

    msgbox  ->setText(Text);
    UpTextEdit text(InfoText.replace("\n","<br>"));
    msgbox  ->setInformativeText(text.toHtml());
    msgbox  ->AjouteLayButtons(Butts);

    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  qobject_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
        {
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, msgbox, [=] {msgbox->Repons(butt);});
            if (butt->ButtonStyle() == UpSmallButton::STARTBUTTON)
                butt->setText("OK");
        }
    }
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    msgbox  ->wdg_texteditlbl   ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl       ->setFixedSize(Utils::CalcSize(InfoText));
    if (link != "")
    {
        msgbox  ->wdg_infolbl   ->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        msgbox  ->wdg_infolbl   ->setOpenExternalLinks(true);
        connect (msgbox->wdg_infolbl,
                &QLabel::linkActivated,                 /*! bug Qt - Impossible to open URL with linkActivated in a QMessageBox - However linkHovered works */
                msgbox,
                [=] { QDesktopServices::openUrl(QUrl(link)); });
    }

    return ExecMsgBox(msgbox);
}

UpSmallButton::StyleBouton UtilsMessageBox::ExecMsgBox(UtilsMessageBox*msgbox)
{
    UpSmallButton::StyleBouton repons = UpSmallButton::CANCELBUTTON;
    if (msgbox  ->exec() == QDialog::Accepted)
        repons = msgbox->clickedButton()->ButtonStyle();
    //qDebug() << Utils::EnumDescription(QMetaEnum::fromType<UpSmallButton::StyleBouton>(), repons);
    delete msgbox;
    return repons;
}


UpSmallButton::StyleBouton UtilsMessageBox::Question(QWidget *parent, QString Text, QString InfoText, Buttons Butts, QStringList titresboutonslist)
{
    UtilsMessageBox*msgbox     = new UtilsMessageBox(parent);
    msgbox->setIcon(Quest);

    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText);
    msgbox  ->AjouteLayButtons(Butts);
    int k = 0;
    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  qobject_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
        {
            if (titresboutonslist.size()>k)
                butt->setText(titresboutonslist.at(k));
            k++;
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, msgbox, [=] {msgbox->Repons(butt);});
        }
    }
    msgbox  ->wdg_texteditlbl   ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl       ->setFixedSize(Utils::CalcSize(InfoText));
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);

    return ExecMsgBox(msgbox);
}

void UtilsMessageBox::Information(QWidget *parent, QString Text, QString InfoText)
{
    UtilsMessageBox*msgbox     = new UtilsMessageBox(parent);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText);
    msgbox  ->setIcon(UtilsMessageBox::Info);

    msgbox  ->AjouteLayButtons(UpDialog::ButtonOK);
    connect (msgbox->OKButton, &QPushButton::clicked, msgbox, [=] {msgbox->accept();});
    msgbox  ->wdg_texteditlbl       ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl   ->setFixedSize(Utils::CalcSize(InfoText));
    msgbox  ->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->exec();
    delete msgbox;
}

