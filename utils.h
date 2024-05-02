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
#include <QColorDialog>
#include <QCryptographicHash>
#include <QDir>
#include <QFileDialog>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QMetaEnum>
#include <QProcess>
#include <QJsonObject>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSerialPortInfo>
#include <QUrl>
#include <cmath>
#include <QStandardPaths>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDesktopServices>
#include <QMovie>
#include <QTextEdit>

#include <QPdfDocument>

#include <QInputDialog>
#include <QCoreApplication>
#include <QEventLoop>
#include <QSerialPort>
#include <QTime>

#include "updialog.h"
#include "uptextedit.h"
#include "dlg_message.h"

const unsigned char SOH = 01;  //0x01
const unsigned char STX = 02;  //0x02
const unsigned char EOT = 04;  //0x04
const unsigned char ETB = 23; //0x17
const unsigned char LF  = 10; //0x0A
const unsigned char CR  = 13; //0x0D

class ShowMessage;
class UpDialog;
class UpTextEdit;

class UtilsMessageBox : public UpDialog
{
    Q_OBJECT
public:
    explicit                            UtilsMessageBox(QWidget *parent = Q_NULLPTR);
    ~UtilsMessageBox();
    static void                         Show        (QWidget*, QString Text = "", QString InfoText = "");
    static void                         Information (QWidget*, QString Text = "", QString InfoText = "");
    static UpSmallButton::StyleBouton   Watch(QWidget*, QString Text = "", QString InfoText = "", Buttons Butts = UpDialog::ButtonOK, QString link = "");
    static UpSmallButton::StyleBouton   Question(QWidget*, QString Text = "", QString InfoText = "", Buttons Butts = UpDialog::ButtonCancel | UpDialog::ButtonOK, QStringList titresboutonslist = QStringList());
    enum                                Icon   {Quest, Info, Warning, Critical, Print}; Q_ENUM(Icon)
    enum                                Movie   {QuestionGif, InfoGif, WarningGif}; Q_ENUM(Movie)
    void                                addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton);
    void                                addButton(UpPushButton *button);
    void                                removeButton(UpSmallButton *);
    UpSmallButton*                      clickedButton() const;
    UpPushButton*                       clickedpushbutton() const;
    void                                setIcon(enum Icon icn, bool animatedIcon = true);
    void                                setIconPixmap(QPixmap);
    void                                setText(QString);
    void                                setInformativeText(QString);
    void                                setDefaultButton(QPushButton*);

private:
    static UpSmallButton::StyleBouton   ExecMsgBox(UtilsMessageBox*msgbox);
    UpLabel         *wdg_iconlbl, *wdg_texteditlbl, *wdg_infolbl;
    QHBoxLayout     *wdg_infolayout;
    QVBoxLayout     *wdg_textlayout;
    QMovie          *m_movie;
    UpSmallButton   *wdg_ReponsSmallButton;
    UpPushButton    *wdg_ReponsPushButton;
    void            Repons(QPushButton *butt);
    void            setAnimatedIcon(Movie movie);
};

class Utils : public QObject
{
    Q_OBJECT
private:
    static Utils*      instance;
    static QString cp() {
        QString mcp = "[0-9]{5}" ;
        if (QLocale().territory() == QLocale::Madagascar)
            mcp = "[0-9]{3}";
        return mcp;
    }
public:
    enum Day {
                Aucun       = 0x0,
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
    enum Period {Debut, Fin};

    static Utils   *I();

    static int correctedwidth(int width);

    static QRegularExpression const rgx_rx;
    static QRegularExpression const rgx_AlphaNumeric;
    static QRegularExpression const rgx_AlphaNumeric_3_12;
    static QRegularExpression const rgx_AlphaNumeric_5_12;
    static QRegularExpression const rgx_AlphaNumeric_5_15;
    static QRegularExpression const rgx_MajusculeSeul;
    static QRegularExpression const rgx_Question;

    static QRegularExpression const rgx_IPV4;
    static QRegularExpression const rgx_IPV4_mask;

    static QRegularExpression const rgx_mail;
    static QRegularExpression const rgx_NNI;

    static QRegularExpression const rgx_adresse;
    static QRegularExpression const rgx_intitulecompta;
    static QRegularExpression const rgx_CP;
    static QRegularExpression const rgx_ville;
    static QRegularExpression const rgx_telephone;

    static QRegularExpression const rgx_tabac;
    static QRegularExpression const rgx_cotation;

    static QRegularExpression const rgx_recherche;


    static void Pause(int msec = 1000);

    //! html
    static bool convertHTML(QString &text);
    static void convertPlainText(QString &text);
    static void nettoieHTML(QString &text, int fontsize = 0, bool supprimeLesLignesVidesDuMilieu = false);
    static void nettoieHtmlOldQt(QString &text, bool converttohtml = true);
    static bool retirelignevidefinhtml(QString &txthtml);
    static bool epureFontFamily(QString &text);  /*! >il y eut un temps où on entrait dans les html de Qt la font-family avec tous ses attributs
                                                 * ce qui donnait -> font-family:'Comic Sans MS,13,-1,5,50,0,0,0,0,0' dans le html
                                                  * depuis Qt 5.10 cela ne marche plus et il faut enlever tous les attributs sinon Qt s'emmêle les pinceaux dans l'interprétation du html
                                                 * il faut donc p.e. remplacer font-family:'Comic Sans MS,13,-1,5,50,0,0,0,0,0' par font-family:'Comic Sans MS'
                                                 * c'est le rôle de cette fonction */

    static bool corrigeErreurHtmlEntete(QString &text, bool ALD= false);
                                                /*! > idem que la fonction précédente, corrige une erreur sur les anciennes largeurs d'entête */

    //! QString
    static QSize                    CalcSize(QString txt, QFont fm = qApp->font());
    static QString                  retirecaracteresaccentues(QString nom);
    static bool                     IsCharSpecial( QChar c);
    static bool                     IsCharNL( QChar c);
    static bool                     IsCharCR( QChar c);
    static QString                  trim(QString text, bool end=true, bool removereturnend = false);
    static QString                  capitilize(QString text, bool onlyfirst = false);
    static QString                  trimcapitilize(QString, bool end = true, bool maj = true, bool lower = true);
    static qint32                   ArrayToInt(QByteArray source);
    static int                      MaxInt()    {return std::numeric_limits<int>::max();}
    static QByteArray               IntToArray(int source);
    static QString                  IPAdress();
    static bool                     RegularExpressionMatches(QRegularExpression rgx, QString s, bool exact = true);
    static QString                  calcIP(QString IP, bool aveczero = false);
    static QString                  MACAdress();
    static QString                  getMacForIP(QString ipAddress);
    static QByteArray               StringToArray(QString source);

    //! Fichiers
    static bool                     CompressFileJPG(QString pathfile, QString Dirprov, QDate datetransfert = QDate::currentDate());
    static QMap<QString, qint64>    dir_size(const QString DirPath);
    static QString                  getExpressionSize(qint64 size);                 //! concertit en Go, To la taille en Mo du qint64 passé en paramètre
    static bool                     mkpath(QString path);
    static void                     cleanfolder(QString path);
    static void                     countFilesInDirRecursively(const QString dirpath, int &tot); // compte le nombre de fichiers présents dans un dossier et ses sous-dossiers
    static void                     copyfolderrecursively(const QString origindirpath, const QString destdirpath,
                                                                int &n,
                                                                QString firstline = QString(),
                                                                QProgressDialog *progress = Q_NULLPTR,
                                                                QFileDevice::Permissions permissions = QFileDevice::ReadOther
                                                                                                       | QFileDevice::ReadGroup
                                                                                                       | QFileDevice::ReadOwner  | QFileDevice::WriteOwner | QFileDevice::ExeOwner
                                                                                                       | QFileDevice::ReadUser);
    static void                     setDirPermissions(QString dirpath, QFileDevice::Permissions permissions = QFileDevice::ReadOther | QFileDevice::WriteOther
                                                                                                 | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                                                                                                 | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                                                                                 | QFileDevice::ReadUser   | QFileDevice::WriteUser);      // attribue recursivement les permissions énumérées par le flag permissions à tous les fichiers du dossier Dir
    static void                     copyWithPermissions(QFile &file, QString path, QFileDevice::Permissions permissions = QFileDevice::ReadOther | QFileDevice::WriteOther
                                                                                                 | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                                                                                                 | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                                                                                 | QFileDevice::ReadUser   | QFileDevice::WriteUser);      // copie le fichier file vers la destination path avec les permissions énumérées par le flag permissions
    static void                     setPermissions(QFile &file, QFileDevice::Permissions permissions = QFileDevice::ReadOther | QFileDevice::WriteOther
                                                                                                 | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                                                                                                 | QFileDevice::ExeOwner  | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                                                                                 | QFileDevice::ExeUser   | QFileDevice::ReadUser   | QFileDevice::WriteUser);      // définit les permissions énumérées par le flag permissions
    static bool                     removeWithoutPermissions(QFile &file);      // efface le fichier file vers la destination path même s'il est enlecture seule
    static double                   mmToInches(double mm);
    static QUrl                     getExistingDirectoryUrl(QWidget *parent = Q_NULLPTR, QString title = "", QUrl Dirdefaut = QUrl::fromLocalFile(PATH_DIR_RUFUS), QStringList listnomsaeliminer = QStringList(), bool ExclureNomAvecEspace = true);

    //! refraction
    static QString                  PrefixePlus(double);                           //! convertit en QString signé + ou - les valeurs QDouble de dioptries

    //! SQL
    static QString                  correctquoteSQL(QString text);
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
    static bool                     VerifMDP(QString MDP, QString Msg, QString &mdp, bool mdpverified = false, QWidget *parent = Q_NULLPTR);

    //! Calcule âge
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour = QDate::currentDate(), QString Sexe = "");

    //! renvoie la valeur littérale d'un enum (à condition d'avoir placé la macro Q_ENUM(nomdelenum) dans la définition de l'enum
    static QString EnumDescription(QMetaEnum metaEnum, int val);
    // p.e.         qDebug() << Utils::EnumDescription(QMetaEnum::fromType<Refraction::Mesure>(), ref->typemesure());

    //! calcule la taille idéale d'une police
    static void CalcFontSize(QFont &font);

    //! renvoie une couleur
    static QColor   SelectCouleur(QColor colordep, QWidget *parent= Q_NULLPTR);


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
    static QList<QImage> calcImagefromPdf(QString filename);
    static void AfficheImage(QImage img);


    //! reconstruit la map des ports COM disponibles sur le système sous la forme (COMxx,nomgeneriqueduport)
    static QMap<QString, QString> ReconstruitMapPortsCOM();

    //! gestion des images en QJsonValue
    static QJsonValue jsonValFromImage(const QImage &p);
    static QImage imagemapFrom(const QJsonValue &val);

    //! écriture sur un port série d'un qByteArray
    static void writeDatasSerialPort (QSerialPort *port, QByteArray datas, QString msgdebug, int timetowaitms = 0);
    static void writeDataToFileDateTime (QByteArray datas, QString file, QString path);
    static void writeBinaryFile (QByteArray data, QString fileName);

    //! Savoir si un port es serial
    static bool isSerialPort( QString name );

    //! Sound Alarme
    static void playAlarm(QString sound = NOM_ALARME);

    //! récupérer l'index d'une valeur dans un QMetaEnum
    static int getindexFromValue(const QMetaEnum & e, int value);

    //! Removes control characters from byteArray
    static QByteArray cleanByteArray( QByteArray byteArray );

    //! return min width & size of QTimeEdit and QDatEdit - depends on graphics card and OS, vary W11 and MacOS/Linux
    static int qtimeeditwidth()     { return QTIMEEDITWIDTH; }
    static int qdateeditwidth()     { return QDATEEDITWIDTH; }
    static QSize qtimeeditsize()    { return QSize(QTIMEEDITWIDTH,QDATETIMEEDITHEIGHT); }
    static QSize qdateeditsize()    { return QSize(QDATEEDITWIDTH,QDATETIMEEDITHEIGHT); }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Utils::Days)

#endif // UTILS_H
