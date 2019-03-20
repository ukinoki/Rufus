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

#include "utils.h"

/*
 * Initialization des variables static const
*/
QRegExp const Utils::rgx_rx = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_AlphaNumeric = QRegExp("[A-Za-z0-9]*");
QRegExp const Utils::rgx_AlphaNumeric_3_12 = QRegExp("[A-Za-z0-9]{3,12}$");
QRegExp const Utils::rgx_AlphaNumeric_5_15 = QRegExp("[A-Za-z0-9]{5,15}$");
QRegExp const Utils::rgx_AlphaNumeric_5_12  = QRegExp("[A-Za-z0-9]{5,12}$");
QRegExp const Utils::rgx_MajusculeSeul = QRegExp("[A-Z]*");
QRegExp const Utils::rgx_IPV4 = QRegExp("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
QRegExp const Utils::rgx_IPV4_mask = QRegExp("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");

QRegExp const Utils::rgx_mail = QRegExp("^[A-Za-z0-9_-]+(.[A-Za-z0-9_-]+)+@[A-Za1-z0-9_-]+.[A-Za-z0-9_-]{2,6}");
QRegExp const Utils::rgx_mailexactmatch = QRegExp("^[A-Za-z0-9_-]+(.[A-Za-z0-9_-]+)+@[A-Za1-z0-9_-]+.[A-Za-z0-9_-]{2,6}$");

QRegExp const Utils::rgx_adresse = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°, -]*");
QRegExp const Utils::rgx_intitulecompta = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°, -/%]*");
QRegExp const Utils::rgx_CP = QRegExp("[0-9]{5}");
QRegExp const Utils::rgx_ville = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_telephone = QRegExp("[0-9 ]*");

QRegExp const Utils::rgx_tabac = QRegExp("[0-9]{2}");
QRegExp const Utils::rgx_cotation = QRegExp("[xsA-Z0-9.+/]*");  // le x pour BZQKOO1x1.5 et le s pour Cs

QRegExp const Utils::rgx_recherche = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-Z %-]*");


//AUTRE QRegExp trouvé
//QRegExp("^[A-Za-z0-9]{5,15}$")
//QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9 -]+") -> qlinedelegate

//JAMAIS UTILISE
//QRegExp const Utils::rgx_AVP = QRegExp("[1,5|1.5|2|3|4|5|6|8|10|14|28|<28]"); // JE NE COMPRENDS PAS, car 1,5 1.5 10 14 et 28 ne sortiront jamais

/*!
 *  \brief Pause
 *
 *  Methode qui permet d'attendre un certain temps (donné en paramètre)
 *  sert surtout à forcer la mise à jour d'un affichage sans attendre la fin d'une foncion
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
 * \param text le texte à nettoyer
 * \param end mettre false si on ne souhaite pas nettoyer la fin du texte
 * \return le texte nettoyé
 */
QString Utils::trim(QString text, bool end, bool removereturnend)
{
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
QString Utils::capitilize(QString text)
{
    QString newText="";
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
void Utils::convertHTML(QString &text)
{
    UpTextEdit textprov;
    textprov.setText( text );
    text = textprov.toHtml();
    retirelignevidehtml(text);
}

/*!
 *  \brief convertPlainText
 *  convertir un QString en plaintext
 *  on écrit le QString dans un QtextEdit et on récupère le plaintext avec QTextEdit::toPlainText()
 *  on retire les lignes vides de la fin
 */
void Utils::convertPlainText(QString &text)
{
    UpTextEdit textprov;
    textprov.setText( text );
    text = textprov.toPlainText();
    while (text.at(text.size()-1).unicode() == 10)
        text.remove(text.size()-1,1);
//    while (text.endsWith("\n"))
//        text = text.left(text.size()-1);
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
    convertHTML(text);
    if (supprimeLesLignesVidesDuMilieu)
        text.remove(reg1);
    text.replace(reg2,"<p style=\" margin-top:0px; margin-bottom:0px;\">");
    text.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");
    text.remove(HTMLCOMMENT_LINUX);
    text.remove(HTMLCOMMENT_MAC);
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
void Utils::retirelignevidehtml(QString &txthtml)
{
    bool a = true;
    while (a) {
        int debut = txthtml.lastIndexOf("<p");
        int fin   = txthtml.lastIndexOf("</p>");
        int longARetirer = fin - debut + 4;
        if (txthtml.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
            txthtml.remove(debut,longARetirer);
        else a = false;
    }
}

void Utils::supprimeAncre(QString &text, QString ancredebut, QString ancrefin)
{
    while (text.contains(ancredebut))
    {
        int idx = text.lastIndexOf(ancredebut);
        QString ftext= text.left(idx);
        idx = ftext.lastIndexOf("<table");
        ftext= text.left(idx);
        if (ancrefin != "")
            idx = text.lastIndexOf(ancrefin);
        QString rtext= text.mid(idx);
        idx = rtext.indexOf("</table>");
        rtext = rtext.mid(idx+8);
        text = ftext + rtext;
    }
}

/*!
 * \brief Utils::dir_size
 * Cette fonction va renvoyer le nombre de fichiers contenu dans un dossier ainsi que le volume du dossier
 * utilisé pour le calcul du volume d'une opération de sauvegarde-restauration p.e.
 * \param text le chemin du dossier
 * \return un QMap avec ces 2 infos
 */
QMap<QString, double> Utils::dir_size(const QString DirPath)
{
    QMap<QString, double>      DataDir;
    double sizex = 0;
    double nfiles = 0;

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
        sizex += (fileInfo.isDir()) ? dir_size(fileInfo.absoluteFilePath())["Size"]: fileInfo.size();
        nfiles += (fileInfo.isDir()) ? dir_size(fileInfo.absoluteFilePath())["Nfiles"] : i+1;
    }
    DataDir["Size"]= sizex;
    DataDir["Nfiles"]= nfiles;
    return DataDir;
}

/*!
 * \brief Utils::getExpressionSize
 * Cette fonction va renvoyer la taille d'un double en Mo, Go ou To
 */
QString Utils::getExpressionSize(double size)
{
    QString com = "Mo";
    if (size>1024)
    {
        com = "Go";
        size /= 1024;
        if (size>1024)
        {
            com = "To";
            size /= 1024;
        }
    }
    return QString::number(size,'f',2) + com;
}

qint32 Utils::ArrayToInt(QByteArray source)
{
    qint32 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
}

QByteArray Utils::IntToArray(int source)
{
    //permet d'éviter le cast
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

QString Utils::getIpAdress()
{
    QString IPadress = "";
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            IPadress = address.toString();
    return IPadress;
}

QString Utils::getMACAdress()
{
    QString localhostname =  QHostInfo::localHostName();
    QString IPadress = "";
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            IPadress = address.toString();
    QString MACAddress = "";
       QString localNetmask;
       foreach (const QNetworkInterface& networkInterface, QNetworkInterface::allInterfaces()) {
           foreach (const QNetworkAddressEntry& entry, networkInterface.addressEntries()) {
               if (entry.ip().toString() == IPadress) {
                   MACAddress = networkInterface.hardwareAddress();
                   break;
               }
           }
       }
    //qDebug() << "MacAdress = " + MACAddress;
    return MACAddress;
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Faire précéder l'apostrophe d'un caractère d'échappement pour les requêtes SQL --------------------
------------------------------------------------------------------------------------------------------------------------------------*/
QString Utils::correctquoteSQL(QString text)
{
    text.replace("\\","\\\\");
    return text.replace("'","\\'");
}

/*---------------------------------------------------------------------------------------------------------------------
    -- VÉRIFICATION DE MDP --------------------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Utils::VerifMDP(QString MDP, QString Msg, bool mdpverified)
{
    if (mdpverified)
        return true;
    QInputDialog quest;
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
    quest.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if (quest.exec() > 0)
    {
        if (quest.textValue() == MDP)
            return true;
        else
            UpMessageBox::Watch(Q_NULLPTR, QObject::tr("Mot de passe invalide!"));
    }
    return false;
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
 * \brief Utils::cleanfilder
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
        qDebug() << "dossier vide effacé" << DirPath;
    }
    else for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isDir())
            cleanfolder(fileInfo.absoluteFilePath());
    }
}

double Utils::mmToInches(double mm )  { return mm * 0.039370147; }


//---------------------------------------------------------------------------------
// Calcul de la formule de refraction
//---------------------------------------------------------------------------------
QString Utils::CalculeFormule(QMap<QString,QVariant> Mesure,  QString Cote)
{
        QString mSphere;
        QString mCyl;
        QString mAxe;
        QString mAdd;
        if (Cote == "D")
        {
            mSphere   = PrefixePlus(Mesure["SphereOD"].toString());
            mCyl      = PrefixePlus(Mesure["CylOD"].toString());
            mAxe      = QString::number(Mesure["AxeOD"].toInt());
            mAdd      = PrefixePlus(Mesure["AddOD"].toString());
        }
        else if (Cote == "G")
        {
            mSphere   = PrefixePlus(Mesure["SphereOG"].toString());
            mCyl      = PrefixePlus(Mesure["CylOG"].toString());
            mAxe      = QString::number(Mesure["AxeOG"].toInt());
            mAdd      = PrefixePlus(Mesure["AddOG"].toString());
        }
        else return "";
        QString Resultat;
        if (QLocale().toDouble(mCyl) != 0.00 && QLocale().toDouble(mSphere) != 0.00)
            Resultat = mSphere + " (" + mCyl + QObject::tr(" à ") + mAxe + "°)" ;
        if (QLocale().toDouble(mCyl) == 0.00 && QLocale().toDouble(mSphere) != 0.00)
            Resultat = mSphere ;
        if (QLocale().toDouble(mCyl) != 0.00 && QLocale().toDouble(mSphere) == 0.00)
            Resultat = mCyl + QObject::tr(" à ") + mAxe + "°" ;
        if (QLocale().toDouble(mCyl) == 0.00 && QLocale().toDouble(mSphere) == 0.00)
            Resultat = QObject::tr("plan");
        if (QLocale().toDouble(mAdd) > 0.00)
            Resultat += " add." + mAdd + " VP" ;
        return Resultat;
}

QString Utils::PrefixePlus(QString Dioptr)                          // convertit en QString signé + ou - les valeurs de dioptries issues des appareils de mesure
{
    double i = Dioptr.toDouble();
    if (Dioptr != "")
        return (i>0 ?
                    "+" + QLocale().toString(Dioptr.toDouble(),'f',2)
                    :
                    QLocale().toString(Dioptr.toDouble(),'f',2)
                    );
    else
        return "";
}

/*!
 * \brief Procedures::DecomposeScriptSQL(QString nomficscript)
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
    // On retire tous les commentaires, les tabulations, les espaces ou les retours à la ligne multiples
    //        queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/)",   QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("(^;\\n)",                   QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("(--.*\\n)",                 QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
    queryStr = queryStr.replace(QRegularExpression("( +)",                      QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
    queryStr = queryStr.replace(QRegularExpression("((\\t)+)",                  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
    queryStr = queryStr.replace(QRegularExpression("(^ *)",                     QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("((\\n)+)",                  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
    //Retire les espaces en début et fin de string
    queryStr = queryStr.trimmed();

    QString matched, delimiter, Atraiter;
    QRegularExpression re("^(\\s|\\n)*DELIMITER\\s*(.|\\n)*END\\s*.\\n"); //isole les créations de procédure SQL dans le script

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
            Atraiter.replace(QRegularExpression("DELIMITER\\s*"),"");
            Atraiter.replace(delimiter,"");
            Atraiter = Atraiter.replace(QRegularExpression("(^ *)",     QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
            Atraiter = Atraiter.replace(QRegularExpression("(^(\\n)+)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
            Atraiter = Atraiter.replace(QRegularExpression("((\\n)+)",  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
            //Edit(Atraiter);
            queryStr.replace(0,matched.size(),"");
        }
        else                    // -- c'est une requête SQL
        {
            matched = queryStr.split(";\n", QString::SkipEmptyParts).at(0);
            Atraiter = matched.trimmed()+ ";";
            queryStr.replace(0,matched.size()+2,"");
            queryStr = queryStr.replace(QRegularExpression("((\\n)+)",  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
        }
        queryStr = queryStr.replace(QRegularExpression("(^(\\n)*)",     QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
        listinstruct << Atraiter;
    }
    return listinstruct;

    /* POUR CREER DES PROCEDURES AVEC Qt - cf fichier créer des procédures mysql avec QSt dans /assets/diagrams */
}

