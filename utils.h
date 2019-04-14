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

#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QRegExp>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>

#include "uplineedit.h"
#include "uplabel.h"
#include "uptextedit.h"
#include "upmessagebox.h"

#include <QInputDialog>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTime>

class Utils
{
public:
    static QRegExp const rgx_rx;
    static QRegExp const rgx_AlphaNumeric;
    static QRegExp const rgx_AlphaNumeric_3_12;
    static QRegExp const rgx_AlphaNumeric_5_12;
    static QRegExp const rgx_AlphaNumeric_5_15;
    static QRegExp const rgx_MajusculeSeul;

    static QRegExp const rgx_IPV4;
    static QRegExp const rgx_IPV4_mask;

    static QRegExp const rgx_mail;
    static QRegExp const rgx_mailexactmatch;

    static QRegExp const rgx_adresse;
    static QRegExp const rgx_intitulecompta;
    static QRegExp const rgx_CP;
    static QRegExp const rgx_ville;
    static QRegExp const rgx_telephone;

    static QRegExp const rgx_tabac;
    static QRegExp const rgx_cotation;

    static QRegExp const rgx_recherche;


    static void Pause(int msec = 1000);
    static void convertHTML(QString &text);
    static void convertPlainText(QString &text);
    static void nettoieHTML(QString &text, bool supprimeLesLignesVidesDuMilieu = false);
    static void retirelignevidehtml(QString &txthtml);
    static void supprimeAncre(QString &text, QString ancredebut, QString ancrefin = "");

    static QSize                    CalcSize(QString txt, QFont fm = qApp->font());
    static QString                  retirecaracteresaccentues(QString nom);
    static QString                  trim(QString text, bool end=true, bool removereturnend = false);
    static QString                  capitilize(QString text);
    static QString                  trimcapitilize(QString, bool end = true, bool maj = true, bool lower = true);
    static QMap<QString, double>    dir_size(const QString DirPath);
    static QString                  getExpressionSize(double size);
    static qint32                   ArrayToInt(QByteArray source);
    static int                      MaxInt()    {return std::numeric_limits<int>::max();}
    static QByteArray               IntToArray(int source);
    static QString                  getIpAdress();
    static QString                  getMACAdress();
    static QString                  correctquoteSQL(QString text);
    static bool                     VerifMDP(QString MDP, QString Msg, bool mdpverified=false);
    static bool                     mkpath(QString path);
    static void                     cleanfolder(QString path);
    static double                   mmToInches(double mm);

    static QString                  CalculeFormule(QMap<QString,QVariant>  Donnees,
                                                  QString Cote);                      // comme son nom l'indique
    static QString                  PrefixePlus(QString);                          // convertit en QString signé + ou - les valeurs QDouble de dioptries
    static QStringList              DecomposeScriptSQL(QString nomficscript);

    static QString                  ConvertitModePaiement(QString mode);            // convertit en clair les abréviations utilisées dans la compta pour les modes de paiement (B= carte de crédit, E = Espèces...etc...)

};

#endif // UTILS_H
