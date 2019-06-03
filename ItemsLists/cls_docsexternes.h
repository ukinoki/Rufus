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

#ifndef CLS_DOCSEXTERNES_H
#define CLS_DOCSEXTERNES_H

#include "cls_docexterne.h"
#include "database.h"
#include "cls_itemslist.h"

class DocsExternes : public ItemsList
{
private:
    QMap<int, DocExterne*> *m_docsexternes = Q_NULLPTR; //!< la liste des DocExternes pour un patient donné
    bool m_nouveaudocument;

public:
    explicit                    DocsExternes(QObject *parent = Q_NULLPTR);
    QMap<int, DocExterne *>*    docsexternes();
    DocExterne*                 getById(int id, Item::LOADDETAILS loadDetails = Item::LoadDetails, ItemsList::ADDTOLIST addToList = ItemsList::AddToList);
    void                        addList(QList<DocExterne*> listdocs);
    void                        initListeByPatient(Patient *pat);

    bool                        NouveauDocument();
    void                        setNouveauDocumentFalse();
    void                        setsoustype(DocExterne* docmt, QString soustype = "");                  //!> actualise le soustype d'un document

    //!> actions sur les enregistrements
    void                SupprimeDocument(DocExterne *doc);
    DocExterne*         CreationDocument(int idImpression, int idUser, int idPat, QString TypeDoc, QString SousTypeDoc,
                                         QString Titre, QString TextEntete, QString TextCorps, QString TextOrigine, QString  TextPied,
                                         QDateTime DateImpression, QByteArray pdf, bool Compression, QByteArray jpg, QByteArray autre,
                                         QString formatautre, QString lienversfichier, QString LienFichierDistant, int idRefraction, bool ALD,
                                         int UserEmetteur, QString Conclusion, int EmisRecu, QString FormatDoc, int idLieu,
                                         int Importance);
};
#endif // CLS_DOCSEXTERNES_H
