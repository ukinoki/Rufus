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

#ifndef CLS_SITE_H
#define CLS_SITE_H

#include "cls_item.h"

class Site : public Item
{
    Q_OBJECT
private:
    //Adresse de travail
    QString m_nom = "";
    QString m_adresse1 = "";
    QString m_adresse2 = "";
    QString m_adresse3 = "";
    QString m_codepostal = "";
    QString m_ville = "";
    QString m_telephone = "";
    QString m_fax = "";
    QString m_couleur = "";
    QByteArray m_logo = QByteArray();   //! le logo du site
    //Envoi des mails
    QString m_mail = "";
    QString m_smtpserveur = "";
    int     m_smtpport = 0;
    QString m_smtplogin = "";

public:
    explicit Site(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data) override;
    void resetdatas();

    QString nom() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    QString codepostal() const;
    QString ville() const;
    QString telephone() const;
    QString fax() const;
    QString couleur() const;
    QString coordonnees() const;
    void setnom(const QString &nom);
    void setadresse1(const QString &adresse1);
    void setadresse2(const QString &adresse2);
    void setadresse3(const QString &adresse3);
    void setcodepostal(const QString codepostal);
    void setville(const QString &ville);
    void settelephone(const QString &telephone);
    void setfax(const QString &fax);
    void setcouleur(const QString &couleur);

    /*! envoi des mails - le mot de passe n'est pas en base, il est local au poste */
    QString mail() const                            { return m_mail; }
    QString smtpserveur() const                     { return m_smtpserveur; }
    int     smtpport() const                        { return m_smtpport; }
    QString smtplogin() const                       { return m_smtplogin; }
    void    setmail(const QString &mail)            { m_mail = mail;                m_data[CP_MAIL_SITE] = mail; }
    void    setsmtpserveur(const QString &serveur)  { m_smtpserveur = serveur;      m_data[CP_SMTPSERVEUR_SITE] = serveur; }
    void    setsmtpport(int port)                   { m_smtpport = port;            m_data[CP_SMTPPORT_SITE] = port; }
    void    setsmtplogin(const QString &login)      { m_smtplogin = login;          m_data[CP_SMTPLOGIN_SITE] = login; }
};

#endif // CLS_SITE_H
