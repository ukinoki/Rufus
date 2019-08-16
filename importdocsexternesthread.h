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


#ifndef IMPORTDOCSEXTERNESTHREAD_H
#define IMPORTDOCSEXTERNESTHREAD_H

#include <QThread>
#include <QMessageBox>
#include "procedures.h"
#include "utils.h"


/* Cette classe tourne en tache de fond et importe les documents d'imagerie dans la base de données
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/

class ImportDocsExternesThread : public QObject
{
    Q_OBJECT
public:
    explicit ImportDocsExternesThread(Procedures *proced);
    void                        RapatrieDocumentsThread(QList<QVariantList> listdocs);
    enum Acces                  {Local, Distant};   Q_ENUM(Acces)

signals:
    void                        emitmsg(QStringList m_listemessages, int pause, bool bottom);

private:
    Procedures                  *proc;
    DataBase                    *db;
    int                         m_idlieuexercice;
    bool                        m_compressiondocs;
    bool                        m_encours;
    bool                        m_ok;
    QString                     m_pathdirstockageprovisoire;
    QString                     m_pathdirstockageimagerie;
    QString                     m_pathdirOKtransfer;
    QString                     m_pathdiroriginOKtransfer;
    QString                     m_pathdirechectransfer;
    QString                     m_datetransfer;
    QStringList                 m_listemessages;
    QFile                       file_image, file_origine;

    QThread                     m_thread;
    Acces                       m_acces;

    bool                        DefinitDossiers();
    void                        EchecImport(QString txt);
};

#endif // IMPORTDOCSEXTERNESTHREAD_H
