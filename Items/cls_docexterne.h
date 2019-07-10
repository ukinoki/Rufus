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

#ifndef CLS_DOCEXTERNE_H
#define CLS_DOCEXTERNE_H

#include "cls_item.h"

/*!
 * \brief DocExterne class
 * l'ensemble des informations concernant les odcuments externes, gérés par la table Rufus.Impressions
 * les documents externs englobent
    * les documents imprimés: ordonnances, certificats, tous les documents administratifs,
    * les documents d'imagerie
 */

class DocExterne : public Item
{

private:
    int m_iduser;                       //!> l'utilisateur qui est le supervisuer du user crétateur au moment de la création du document
    int m_importance;                   //!> l'importance du document - 0 sans importance , 1 = normal, 2 = important
    int m_idpatient;                    //!< l'id du patient concerné
    int m_useremetteur;                 //!> l'utilisateur qui a créé ou importé le document
    QString m_typedoc;                  //!> le type de document (Video, RNM, OCT, Echo...
    QString m_titre;                    //!> le titre du document
    QDateTime m_dateimpression;         //!< la date d'émission du document
    int  m_ald;                         //!< l'ordonnance est une ordo ALD
    bool m_isAllLoaded = false;         /*! toutes les datas sont chargées
                                        * pour des raisons de temps et d'encombrement du réseau
                                        * le contenu du document (entête, pied, corps pour les documents imprimés - fichier pour les images)
                                        * n'est a priori pas chargé et ne l'est que quand on demande de l'afficher) */

    //! les documents texte
    QString m_soustypedoc;              //!< le soustype du document - en général, des indications sur l'appareil qui a émis le document
    QString m_textentete;               //!< l'entête du document quand il s'agit d'un document texte
    QString m_textcorps;                //!< le corps du document
    QString m_textorigine;              //!< le texte d'origine du document (utilisé pour les rééditions et on peut donc modifier certains éléments comme la date)
    QString m_textpied;                 //!< le pied du document imprimé

    //! les documents d'imagerie
    int m_compression;                  /*! indique si on comprime les pdf - 0 si non, 1, si oui
                                        * pas utilisé, toujours à 0
                                        * parce que la compression des pdf ne donne quasiment rien
                                        * il faudrait pouvoir déconstruire le pdf, en sortir les images jpg, les comprimer et les remettre dans le pdf
                                        * mais je ne sais pas faire
                                        */
    int m_emisrecu;                     //!< 0 si document emis par le cabinet, 1 si document reçu
    int m_idlieu;                       //!< l'id du site où a été créé le document
    QString m_formatdoc;                //!< le format du document quand il s'agit d'un document d'imagerie: jpg, pdf, video...etc...
    QString m_lienversfichier;          //!< le lien vers le fichier d'imagerie
    QByteArray m_blob = QByteArray();   //!< le contenu du fichier image
    QString m_formatimage;              //!< le format du fichier image

    int m_idrefraction;                 //!> l'id de la refraction concernée quand il s'agit d'une prescription de verres correcteurs

public:
    explicit DocExterne(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idpatient() const;
    int iduser() const;
    QString typedoc() const;
    QString soustypedoc() const;
    QString titre() const;
    QString textentete() const;
    QString textcorps() const;
    QString textorigine() const;
    QString textpied() const;
    QDateTime date() const;
    QString lienversfichier() const;
    bool isALD() const;
    int useremetteur() const;
    QString format() const;
    int importance() const;
    int compression() const;
    QByteArray imageblob() const;
    QString imageformat() const;
    QString imagelien() const;
    bool isAllLoaded() const;
    int idrefraction() const;

    void setDate(QDateTime date);
    void setimportance(int imptce);
    void setAllLoaded(bool allloaded);
    void setimageblob(QByteArray blob);
    void setimageformat(QString format);

    void setsoustype(QString soustype)  { m_soustypedoc = soustype; }
};
#endif // CLS_DOCEXTERNE_H
