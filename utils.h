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
#include <QMetaEnum>
#include <QProcess>
#include <cmath>

#include "uplineedit.h"
#include "uplabel.h"
#include "uptextedit.h"
#include "upmessagebox.h"
#include "dlg_message.h"

#include <QInputDialog>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTime>

class Utils
{
public:
    enum Day {
                Lundi       = 0x1,
                Mardi       = 0x2,
                Mercredi    = 0x4,
                Jeudi       = 0x8,
                Vendredi    = 0x10,
                Samedi      = 0x20,
                Dimanche    = 0x40,
              };
    Q_DECLARE_FLAGS(Days, Day)
    enum ModeAcces { Poste, ReseauLocal, Distant };

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
    static QRegExp const rgx_NNI;

    static QRegExp const rgx_adresse;
    static QRegExp const rgx_intitulecompta;
    static QRegExp const rgx_CP;
    static QRegExp const rgx_ville;
    static QRegExp const rgx_telephone;

    static QRegExp const rgx_tabac;
    static QRegExp const rgx_cotation;

    static QRegExp const rgx_recherche;


    static void Pause(int msec = 1000);

    //! html
    static void convertHTML(QString &text);
    static void convertPlainText(QString &text);
    static void nettoieHTML(QString &text, bool supprimeLesLignesVidesDuMilieu = false);
    static void retirelignevidehtml(QString &txthtml);
    static void supprimeAncre(QString &text, QString ancredebut, QString ancrefin = "");

    //! QString
    static QSize                    CalcSize(QString txt, QFont fm = qApp->font());
    static QString                  retirecaracteresaccentues(QString nom);
    static QString                  trim(QString text, bool end=true, bool removereturnend = false);
    static QString                  capitilize(QString text);
    static QString                  trimcapitilize(QString, bool end = true, bool maj = true, bool lower = true);
    static qint32                   ArrayToInt(QByteArray source);
    static int                      MaxInt()    {return std::numeric_limits<int>::max();}
    static QByteArray               IntToArray(int source);
    static QString                  getIpAdress();
    static QString                  getMACAdress();
    static QString                  getMacForIP(QString ipAddress);
    static bool                     VerifMDP(QString MDP, QString Msg, bool mdpverified=false);

    //! Fichiers
    static bool                     CompressFileJPG(QString nomfile, QString Dirprov, QDate datetransfert = QDate::currentDate());
    static QMap<QString, qint64>    dir_size(const QString DirPath);
    static QString                  getExpressionSize(qint64 size);                 //! concertit en Go, To la taille en Mo du qint64 passé en paramètre
    static bool                     mkpath(QString path);
    static void                     cleanfolder(QString path);
    static double                   mmToInches(double mm);

    //! refraction
    static QString                  PrefixePlus(double);                           //! convertit en QString signé + ou - les valeurs QDouble de dioptries

    //! SQL
    static QString                  correctquoteSQL(QString text);
    static QStringList              DecomposeScriptSQL(QString nomficscript);
    static QString                  ConvertitModePaiement(QString mode);            // convertit en clair les abréviations utilisées dans la compta pour les modes de paiement (B= carte de crédit, E = Espèces...etc...)
    static void                     CalcStringValueSQL(QVariant &newvalue);         // convertit un Qvariant en valeur string SQL équivalente
    static void                     CalcintValueSQL(QVariant &newvalue);            // convertit un Qvariant en valeur int SQL équivalente
    static void                     CalcdoubleValueSQL(QVariant &newvalue);         // convertit un Qvariant en valeur decimal SQL équivalente
    static void                     CalcDateValueSQL(QVariant &newvalue);           // convertit un Qvariant en valeur date SQL équivalente
    static void                     CalcTimeValueSQL(QVariant &newvalue);           // convertit un Qvariant en valeur time SQL équivalente
    static void                     CalcDateTimeValueSQL(QVariant &newvalue);       // convertit un Qvariant en valeur datetime SQL équivalente
    static QString                  getBaseFromMode(ModeAcces mode);  /*! renvoie le mode d'accès au serveur tel qu'il est inscrit dans le fichier rufus.ini
                                                                    \result monoposte = BDD_POSTE, reseau local = BDD_LOCAL, distant = BDD_DISTANT
                                                                    \param le mode d'accès */

    //! Calcule âge
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour = QDate::currentDate());

    //! renvoie la valeur littérale d'un enum (à condition d'avoir placé la macro Q_ENUM(nomdelenum) dans la définition de l'enum
    static QString EnumDescription(QMetaEnum metaEnum, int val);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Utils::Days)

#endif // UTILS_H
