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

#ifndef PYXINTERF_H
#define PYXINTERF_H

#include <QTcpSocket>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include "procedures.h"

#define NomAppli    "PyxInterf"
#define SW_HIDE 0
#define SW_SHOW 5

class pyxinterf : public QObject
{
     Q_OBJECT

public:
    explicit pyxinterf(Procedures *procAPasser, QObject *parent = Q_NULLPTR);

    QString     Lecture_CPS();
    QString     Lecture_CV();
    QString     Saisie_FSE();

private slots:

    void        ErreurConnexionPyx(QAbstractSocket::SocketError socketError);

private:

    Procedures  *proc;
    void        FermeTout();
    bool        InitConnexionPyxvital();
    void        InitVariables();
    void        connexionPyx();
    bool        appelPyxvital(const char* modeAppel,int visuel, const char *libAction);
    void        LancerLeServeur();
    void        traiteErreur(const char *messer1, const char *messer2);
    void        modeSecurise();
    void        Attendre_fin_FSE();

    QTcpSocket  *g_tcpSocket;
    char        CRLF[6];
    char        gResultPyx[1024] ;          // retour appel Pyxvital

    QString     gnomFicPatient;
    QString     gnomFicPraticien;
    QString     gnomFicFacture;
    QString     gnomficErreur;
    QString     gnomficStop;
    QString     gnomPyxvitalExe;
    QString     gRepFSE;
    QString     gnumPS;
    QString     gRepInterf;
    //QString     gzRepfic;
    QString     gServeurPyx;
    int         gPortPyx;
    int         CombienDeFois;
    bool        gSaisiePyxvitalEnCours;
};

#endif // PYXINTERF_H
