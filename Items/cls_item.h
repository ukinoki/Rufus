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

#ifndef CLS_ITEM_H
#define CLS_ITEM_H

#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QDate>
#include "macros.h"
#include "QDebug"

class Item : public QObject
{
    Q_OBJECT

public:
    enum Logic {True, False, Null};
    /*! pour certaines données bool, on a 3 valeurs possibles, true, false et null
         * en fait, on pourrait même faire une 4ème valeur correspondant à "ne sait pas" -> ne sait pas si la valeur est true, false ou null
         */
    enum Cote {Droit, Gauche, Les2, NoLoSo};
    enum LOADDETAILS    {LoadDetails, NoLoadDetails};       Q_ENUM(LOADDETAILS)
    enum UPDATE         {NoUpdate, Update};                 Q_ENUM(UPDATE)
    explicit Item(QObject *parent = Q_NULLPTR) : QObject(parent)
    {
        m_id = 0;
        m_stringid = "";
    }
    int id() const                      { return m_id; }
    QString stringid() const            { return m_stringid; }
    QJsonObject datas() const           { return m_data; }

    //! convertit un côté en QString : droit = "D", Gauche = "G", Les 2 = "2"
    static Cote ConvertCote(QString cote)
    {
        if (cote == "D") return Droit;
        if (cote == "G") return Gauche;
        if (cote == "2") return Les2;
        return  NoLoSo;
    }

    static QString ConvertCote(Cote cote)
    {
        switch (cote) {
        case Gauche:     return "G";
        case Droit:      return "D";
        case Les2:       return "2";
        default: return "";
        }
    }

    static QString TraduitCote(QString cote)
    {
        if (cote == "D") return tr("Droit");
        if (cote == "G") return tr("Gauche");
        if (cote == "2") return tr("Les 2");
        return  "";
    }

    static QString TraduitCote(Cote cote)
    {
        switch (cote) {
        case Gauche:     return tr("Gauche");
        case Droit:      return tr("Droit");
        case Les2:       return tr("Les 2");
        default: return "";
        }
    }


protected:
    int m_id = 0;
    QString  m_stringid = "";
    QJsonObject m_data = QJsonObject{};         //!> les datas d'un item

    bool IsCharSpecial( QChar c)
    {
        if( c == ' ' || c == '-' || c == '\'' ) return true;
        return false;
    }


 /*!
 * \brief copy of Utils::trim to avoid circular dependencies
 * Cette fonction va supprimer :
 * - les " ", "-" et "'" en début et fin du texte
 * - les " ", "-" et "'" en doublon dans le texte
 * - les retour à la ligne en fin du texte
 * \param text le texte à nettoyer
 * \param end (true par défaut) mettre false si on ne souhaite pas nettoyer la fin du texte
 * \param removereturnend (false par défaut) mettre true si on souhaite retirer les retours à la ligne à la fin du texte
 * \return le texte nettoyé
 */
QString trim(QString text, bool end=true, bool removereturnend = false)
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

    void setDataString(QJsonObject data, QString key, QString &prop, bool useTrim = false)
    {
        if( data.contains(key) )
        {
            QString str = data[key].toString();
            if( useTrim )
                str = trim(str);
            prop = str;
        }
    }
    void setDataInt(QJsonObject data, QString key, int &prop)
    {
        if( data.contains(key) )
            prop = data[key].toInt();
    }
    void setDataLongLongInt(QJsonObject data, QString key, qlonglong &prop)
    {
        if( data.contains(key) )
            prop = data[key].toVariant().toLongLong();
    }
    void setDataDouble(QJsonObject data, QString key, double &prop)
    {
        if( data.contains(key) )
            prop = data[key].toDouble();
    }
    void setDataBool(QJsonObject data, QString key, bool &prop)
    {
        if( data.contains(key) )
            prop = data[key].toBool();
    }
    void setDataDateTime(QJsonObject data, QString key, QDateTime &prop)
    {
        if( data.contains(key) )
        {
            double time = data[key].toDouble();
            QDateTime dt;
            dt.setMSecsSinceEpoch( qint64(time) );
            prop = dt;
        }
    }
    void setDataTime(QJsonObject data, QString key, QTime &prop)
    {
        if( data.contains(key) )
            prop = QTime::fromString(data[key].toString(),"HH:mm:ss");
    }
    void setDataDate(QJsonObject data, QString key, QDate &prop)
    {
        if( data.contains(key) )
            prop = QDate::fromString(data[key].toString(),"yyyy-MM-dd");
    }
    void setDataByteArray(QJsonObject data, QString key, QByteArray &prop)
    {
        if( data.contains(key) )
            prop = QByteArray::fromBase64(data[key].toString().toLatin1());
    }
    void setDataLogic(QJsonObject data, QString key, Logic &prop)
    {
        if( data.contains(key) )
            prop = (data[key].toBool()? True : False);
        else
            prop = Null;
    }
};

#endif // CLS_ITEM_H
