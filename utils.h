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

#ifndef UTILS_H
#define UTILS_H
#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFileDialog>
#include <QJsonDocument>
#include <QRegExp>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QMetaEnum>
#include <QProcess>
#include <QJsonObject>
#include <QTextCodec>
#include <QUrl>
#include <cmath>

#include "uplineedit.h"
#include "uplabel.h"
#include "uptextedit.h"
#include "upmessagebox.h"
#include "dlg_message.h"
#include "poppler-qt5.h"

#include <QInputDialog>
#include <QCoreApplication>
#include <QEventLoop>
#include <QSerialPort>
#include <QTime>

class Utils : public QObject
{
    Q_OBJECT
private:
    static Utils*      instance;
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
    enum Logic {True, False, Null};
        /*! pour certaines données bool, on a 3 valeurs possibles, true, false et null
         * en fait, on pourrait même faire une 4ème valeur correspondant à "ne sait pas" -> ne sait pas si la valeur est true, false ou null
         */
    enum ModeAcces { Poste = 0x1, ReseauLocal = 0x2, Distant = 0x4};     Q_ENUM(ModeAcces)
    enum Cote {Droit, Gauche, Les2, NoLoSo};
    enum Period {Debut, Fin};

    static Utils   *I();

    static QRegExp const rgx_rx;
    static QRegExp const rgx_AlphaNumeric;
    static QRegExp const rgx_AlphaNumeric_3_12;
    static QRegExp const rgx_AlphaNumeric_5_12;
    static QRegExp const rgx_AlphaNumeric_5_15;
    static QRegExp const rgx_MajusculeSeul;

    static QRegExp const rgx_IPV4;
    static QRegExp const rgx_IPV4_mask;

    static QRegExp const rgx_mail;
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
    static bool convertHTML(QString &text);
    static void convertPlainText(QString &text);
    static void nettoieHTML(QString &text, bool supprimeLesLignesVidesDuMilieu = false);
    static bool retirelignevidefinhtml(QString &txthtml);

    //! QString
    static QSize                    CalcSize(QString txt, QFont fm = qApp->font());
    static QString                  retirecaracteresaccentues(QString nom);
    static QString                  trim(QString text, bool end=true, bool removereturnend = false);
    static QString                  capitilize(QString text, bool onlyfirst = false);
    static QString                  trimcapitilize(QString, bool end = true, bool maj = true, bool lower = true);
    static qint32                   ArrayToInt(QByteArray source);
    static int                      MaxInt()    {return std::numeric_limits<int>::max();}
    static QByteArray               IntToArray(int source);
    static QString                  IPAdress();
    static QString                  calcIP(QString IP, bool aveczero = false);
    static QString                  MACAdress();
    static QString                  getMacForIP(QString ipAddress);

    //! Fichiers
    static bool                     CompressFileJPG(QString pathfile, QString Dirprov, QDate datetransfert = QDate::currentDate());
    static QMap<QString, qint64>    dir_size(const QString DirPath);
    static QString                  getExpressionSize(qint64 size);                 //! concertit en Go, To la taille en Mo du qint64 passé en paramètre
    static bool                     mkpath(QString path);
    static void                     cleanfolder(QString path);
    static double                   mmToInches(double mm);
    static QUrl                     getExistingDirectoryUrl(QWidget *parent, QString title = "", QUrl Dirdefaut = QUrl::fromLocalFile(QDir::homePath()), QStringList listnomsaeliminer = QStringList(), bool ExclureNomAvecEspace = true);

    //! refraction
    static QString                  PrefixePlus(double);                           //! convertit en QString signé + ou - les valeurs QDouble de dioptries

    //! SQL
    static QString                  correctquoteSQL(QString text);
    static QStringList              DecomposeScriptSQL(QString nomficscript);       //! plus utilisé - imparfait - on passe par les QProcess pour éxécuter un script SQL - voir Procedures:: DefinitScriptRestore(QStringList ListNomFiles);
    static QString                  ConvertitModePaiement(QString mode);            // convertit en clair les abréviations utilisées dans la compta pour les modes de paiement (B= carte de crédit, E = Espèces...etc...)
    static void                     CalcBlobValueSQL(QVariant &newvalue);           // convertit un Qvariant en valeur blob SQL équivalente
    static void                     CalcStringValueSQL(QVariant &newvalue);         // convertit un Qvariant en valeur string SQL équivalente
    static void                     CalcintValueSQL(QVariant &newvalue);            // convertit un Qvariant en valeur int SQL équivalente
    static void                     CalcdoubleValueSQL(QVariant &newvalue);         // convertit un Qvariant en valeur decimal SQL équivalente
    static void                     CalcDateValueSQL(QVariant &newvalue);           // convertit un Qvariant en valeur date SQL équivalente
    static void                     CalcTimeValueSQL(QVariant &newvalue);           // convertit un Qvariant en valeur time SQL équivalente
    static void                     CalcDateTimeValueSQL(QVariant &newvalue);       // convertit un Qvariant en valeur datetime SQL équivalente
    static QString                  getBaseFromMode(ModeAcces mode);  /*! renvoie le mode d'accès au serveur tel qu'il est inscrit dans le fichier rufus.ini
                                                                    \result monoposte = BDD_POSTE, reseau local = BDD_LOCAL, distant = BDD_DISTANT
                                                                    \param le mode d'accès */
    static QString                  calcSHA1(QString mdp);              /*! renvoie la valeur de mdp codée en SHA */
    static bool                     VerifMDP(QString MDP, QString Msg, bool mdpverified = false);

    //! Calcule âge
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour = QDate::currentDate());

    //! renvoie la valeur littérale d'un enum (à condition d'avoir placé la macro Q_ENUM(nomdelenum) dans la définition de l'enum
    static QString EnumDescription(QMetaEnum metaEnum, int val);
    // p.e.         qDebug() << Utils::EnumDescription(QMetaEnum::fromType<Refraction::Mesure>(), ref->typemesure());

    //! calcule la taille idéale d'une police
    static void CalcFontSize(QFont &font);

    //! convertit un côté en QString : droit = "D", Gauche = "G", Les 2 = "2"
    static Cote     ConvertCote(QString cote);
    static QString  ConvertCote(Cote mode);
    static QString  TraduitCote(Cote mode);
    static QString  TraduitCote(QString cote);

    //! gestion des QJsonObject des items et des mesures
    static void setDataString(QJsonObject data, QString key, QString &prop, bool useTrim=false);
    static void setDataInt(QJsonObject data, QString key, int &prop);
    static void setDataLongLongInt(QJsonObject data, QString key, qlonglong &prop);
    static void setDataDouble(QJsonObject data, QString key, double &prop);
    static void setDataBool(QJsonObject data, QString key, bool &prop);
    static void setDataDateTime(QJsonObject data, QString key, QDateTime &prop);
    static void setDataDate(QJsonObject data, QString key, QDate &prop);
    static void setDataTime(QJsonObject data, QString key, QTime &prop);
    static void setDataByteArray(QJsonObject data, QString key, QByteArray &prop);
    static void setDataLogic(QJsonObject data, QString key, Logic &prop);


    //! arrondit un int au multiple de 5 le plus proche
    static int roundToNearestFive(int number)                   { return static_cast<int>(number / 5. + .5) * 5; }

    //! arrondit un double au multiple de .25 le plus proche
    static double roundToNearestPointTwentyFive(double number)  { return static_cast<double>(std::round(number*4)) / 4; }

    //! arrondit un double au multiple de .50 le plus proche
    static double roundToNearestPointFifty(double number)  { return static_cast<double>(std::round(number*2)) / 2; }

    //! affiche la fiche enchantier
    static void EnChantier(bool avecMsg = false);

    //! renvoie chaque page d'un pdf comme une image
    static QList<QImage> calcImagefromPdf(QByteArray pdf);

    //! gestion des images en QJsonValue
    static QJsonValue jsonValFromImage(const QImage &p);
    static QImage imagemapFrom(const QJsonValue &val);

    //! écriture sur un port série d'un qByteArray
    static void writeDatasSerialPort (QSerialPort *port, QByteArray datas, QString msgdebug, int timetowaitms = 0);

};
Q_DECLARE_OPERATORS_FOR_FLAGS(Utils::Days)

#endif // UTILS_H
