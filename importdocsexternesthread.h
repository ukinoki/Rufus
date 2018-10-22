/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IMPORTDOCSEXTERNESTHREAD_H
#define IMPORTDOCSEXTERNESTHREAD_H

#include <QThread>
#include <QMessageBox>
#include "procedures.h"
#include "functormessage.h"
#include "functormajpremierelettre.h"


/* Cette classe tourne en tache de fond et importe les documents d'imagerie dans la base de données
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/

class ImportDocsExternesThread : public QObject
{
    Q_OBJECT
public:
    explicit ImportDocsExternesThread(Procedures *proced);
private:
    int                         idLieuExercice;
    bool                        docscompress;
    void                        RapatrieDocumentsThread();
    Procedures                  *proc;
    bool                        DefinitDossiers();
    void                        EchecImport(QString txt);
    bool                        EnCours;
    FunctorMessage              fmessage;
    FunctorMAJPremiereLettre    fMAJPremLettre;
    QSqlDatabase                db;
    QThread                     *thread;

    int                         Acces;
    enum Acces                  {Local, Distant};

    QString                     NomDirStockageProv, NomDirStockageImagerie, CheminOKTransfrDir, CheminOKTransfrDirOrigin;
    QString                     datetransfer, CheminEchecTransfrDir;
    QStringList                 listmsg;
    QFile                       FichierImage;
};

#endif // IMPORTDOCSEXTERNESTHREAD_H
