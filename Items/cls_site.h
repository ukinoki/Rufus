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

#ifndef CLS_SITE_H
#define CLS_SITE_H

#include "cls_item.h"

class Site : public Item
{
    Q_OBJECT
private:
    //Adresse de travail
    QString m_nom;
    QString m_adresse1;
    QString m_adresse2;
    QString m_adresse3;
    int m_codePostal;
    QString m_ville;
    QString m_telephone;
    QString m_fax;
    QString m_couleur;

public:
    explicit Site(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);

    QString nom() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    int codePostal() const;
    QString ville() const;
    QString telephone() const;
    QString fax() const;
    QString couleur() const;
    QString adresseComplete() const;
    void setnom(const QString &nom);
    void setadresse1(const QString &adresse1);
    void setadresse2(const QString &adresse2);
    void setadresse3(const QString &adresse3);
    void setcodepostal(int codePostal);
    void setville(const QString &ville);
    void settelephone(const QString &telephone);
    void setfax(const QString &fax);
    void setcouleur(const QString &couleur);
};

#endif // CLS_SITE_H
