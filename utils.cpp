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

#include "uptextedit.h"
#include "utils.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTime>


/*
 * Initialization des varaibles static const
*/
QRegExp const Utils::rgx_rx = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_AlphaNumeric = QRegExp("[A-Za-z0-9]*");
QRegExp const Utils::rgx_AlphaNumeric_3_15 = QRegExp("[A-Za-z0-9]{3,15}$");
QRegExp const Utils::rgx_AlphaNumeric_5_15 = QRegExp("[A-Za-z0-9]{5,15}$");
QRegExp const Utils::rgx_IPV4 = QRegExp("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
QRegExp const Utils::rgx_IPV4_mask = QRegExp("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");

QRegExp const Utils::rgx_mail = QRegExp("[A-Za-z0-9_-]+.[A-Za-z0-9_-]+@[A-Za-z0-9_-]+.[A-Za-z0-9_-]+");
                              //QRegExp("^[a-zA-Z][\\w\\.-]*[a-zA-Z0-9]@[a-zA-Z0-9][\\w\\.-]*[a-zA-Z0-9]\\.[a-zA-Z][a-zA-Z\\.]*[a-zA-Z]$");
                              //QRegExp("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$");

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
 *  \brief convertHTML
 *
 *  Methode qui permet de convertir un QString en html
 *  on écrit le QString dans un QtextEdit et on récupère le html avec QTextEdit::toHtml()
 *
 */
QString Utils::convertHTML(QString text)
{
    UpTextEdit textprov;
    textprov.setText( text );
    // on retire la dernière ligne si elle est vide
    QString texte = textprov.toHtml();
    bool a = true;
    while (a)
    {
        // il faut retirer la dernière ligne du html qui contient le retour à la ligne
        int debut = texte.lastIndexOf("<p");
        int fin   = texte.lastIndexOf("</p>");
        int longARetirer = fin - debut + 4;
        if( (a = (texte.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))) )
            texte.remove(debut,longARetirer);
    }

    return texte;
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

    if( removereturnend )                   // enlève les retours à la ligne de la fin de la fin
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
            UpMessageBox::Watch(Q_NULLPTR, "Mot de passe invalide!");
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

double Utils::mmToInches(double mm )  { return mm * 0.039370147; }
