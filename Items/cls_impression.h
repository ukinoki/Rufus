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

#ifndef CLS_IMPRESSION_H
#define CLS_IMPRESSION_H

#include "cls_item.h"

class Impression : public Item
{

private:
    int m_iduser;
    QString m_texte, m_resume, m_conclusion;
    bool m_public, m_prescription, m_editable, m_medical;

public:
    explicit Impression(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int iduser() const;
    QString texte() const;
    QString resume() const;
    QString conclusion() const;
    bool ispublic() const;                      //! le document est accessible à tous les utilisateurs habilités
    bool isprescription() const;                /*! le document est une prescription -> un dupli est émis
                                                 *                                      l'entête est différent
                                                 *                                      il n'est pas accessible au personnel non médical à moins d'avoir été déclaré administratif
                                                 */
    bool ismedical() const;                     //! le document est médical (-> son accès réservé au personnel médical) ou administratif
    bool iseditable() const;                    //! le document est éditable avant impression

    void settext(QString txt);
    void setresume(QString resume);
    void setconclusion(QString conclusion);
    void setiduser(int id);
    void setpublic(bool pblic);
    void setprescription(bool prescription);
    void setmedical(bool medical);
    void seteditable(bool editable);
};

class DossierImpression : public Item
{
private:
    int m_dossierimpression, m_iduser;
    QString m_textedossier, m_resumedossier;
    bool m_public;

public:
    explicit DossierImpression(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int id() const;
    int iduser() const;
    QString texte() const;
    QString resume() const;
    bool ispublic() const;
    void settexte(const QString &textedossier);
    void setresume(const QString &resumedossier);
    void setiduser(int id);
    void setpublic(bool pblic);
};

#endif // CLS_IMPRESSION_H
