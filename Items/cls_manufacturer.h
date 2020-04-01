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

#ifndef MANUFACTURER_H
#define MANUFACTURER_H

#include "cls_item.h"
#include "macros.h"

/*!
 * \brief Manufacturer class
 * l'ensemble des informations concernant un fabricant
 */

class Manufacturer : public Item
{
public:
    explicit Manufacturer(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString nom() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    QString adresseComplete() const;
    QString codepostal() const;
    QString ville() const;
    QString telephone() const;
    QString fax() const;
    QString portable() const;
    QString website() const;
    QString mail() const;
    QString cornom() const;
    QString corprenom() const;
    QString corstatut() const;
    QString cormail() const;
    QString cortelephone() const;
    bool isactif() const;
    QString tooltip() const;

    void setnom(const QString &nom);
    void setadresse1(const QString &adresse1);
    void setadresse2(const QString &adresse2);
    void setadresse3(const QString &adresse3);
    void setcodepostal(const QString codepostal);
    void setville(const QString &ville);
    void settelephone(const QString &telephone);
    void setfax(const QString &fax);
    void setportable(const QString &portable);
    void setwebsite(const QString &website);
    void setmail(const QString &mail);
    void setcornom(const QString &cornom);
    void setcorprenom(const QString &corprenom);
    void setcorstatut(const QString &corstatut);
    void setcormail(const QString &corstatut);
    void setcortelephone(const QString &cortelephone);
    void setactif(const bool &actif);
    void resetdatas();

    bool isnull() const                     { return m_id == 0; }

private:
    QString m_nom           = "";
    QString m_adresse1      = "";
    QString m_adresse2      = "";
    QString m_adresse3      = "";
    QString m_codepostal    = "";
    QString m_ville         = "";
    QString m_telephone     = "";
    QString m_fax           = "";
    QString m_portable      = "";
    QString m_website       = "";
    QString m_mail          = "";
    QString m_cornom        = "";
    QString m_corprenom     = "";
    QString m_corstatut     = "";
    QString m_cormail       = "";
    QString m_cortelephone  = "";
    bool m_inactif          = false;
};

#endif // MANUFACTURER_H
