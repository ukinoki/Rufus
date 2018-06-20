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
 *
 *  \param msec : le temps à attendre en milliseconde
 *
 */
void Utils::Pause(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

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
QString Utils::trim(QString text, bool end)
{
    QString textC = text;
    QChar c;
    while( textC.size() )
    {
        c = textC.at(0);
        if( c == " " || c == "-" || c == "'" )
            textC = textC.remove(0,1);
        else
            break;
    }

    if( end )
        while( textC.size() )
        {
            int lastIndex = textC.size() - 1;
            c = textC.at(lastIndex);
            if( c == " " || c == "-" || c == "'" )
                textC = textC.remove(lastIndex,1);
            else
                break;
        }

    QString newText = "";
    QChar lastChar;
    for( int i=0; i < textC.size(); ++i )
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
