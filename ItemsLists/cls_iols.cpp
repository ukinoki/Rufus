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

#include "cls_iols.h"
#include "gbl_datas.h"
#include "utils.h"
#include <QtXml>
#include <algorithm>

IOLs::IOLs(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, IOL*>();
}

QMap<int, IOL*>* IOLs::iols() const
{
    return map_all;
}

IOL* IOLs::getById(int id, bool reload)
{
    QMap<int, IOL*>::const_iterator itref = map_all->constFind(id);
    if( itref == map_all->constEnd() )
    {
        QJsonObject data = DataBase::I()->loadIOLdataById(id);
        if (data.isEmpty())
            return nullptr;
        IOL* iol = new IOL(data);
        if (iol != Q_NULLPTR)
            add( map_all, iol, Item::Update );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<IOL*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        IOL *iol = itref.value();
        QJsonObject data = DataBase::I()->loadIOLdataById(id);
        if (data.isEmpty())
            return nullptr;
        else
        {
            iol->resetdatas();
            iol->setData(data);
        }
    }
    return itref.value();
}

/*!
 * \brief IOLs::initListe
 * Charge l'ensemble des IOLs
 * et les ajoute à la classe IOLS
 */
void IOLs::initListe()
{
    QList<IOL*> listIOLs = DataBase::I()->loadIOLs();
    clearAll(map_all);
    addList(map_all, &listIOLs);
    m_isfull     = true;
}

void IOLs::SupprimeIOL(IOL *iol)
{
    Supprime(map_all, iol);
}

IOL* IOLs::CreationIOL(QHash<QString, QVariant> sets, QWidget *parent)
{
    IOL *iol = Q_NULLPTR;
    int idiol = 0;
    DataBase::I()->locktables(QStringList() << TBL_IOLS);
    idiol = DataBase::I()->selectMaxFromTable(CP_ID_IOLS, TBL_IOLS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idiol;
        sets[CP_ID_IOLS] = idiol;
        result = DataBase::I()->InsertSQLByBinds(TBL_IOLS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(parent,tr("Impossible d'enregistrer cet implant dans la base!"), sets[CP_MODELNAME_IOLS].toString());
        return iol;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_IOLS] = idiol;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDMANUFACTURER_IOLS)                        data[champ] = itset.value().toInt();
        else if (champ == CP_IDIOLCON_IOLS)                         data[champ] = itset.value().toInt();
        else if (champ == CP_MODELNAME_IOLS)                        data[champ] = itset.value().toString();
        else if (champ == CP_INACTIF_IOLS)                          data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_ACD_IOLS)                              data[champ] = itset.value().toDouble();
        else if (champ == CP_MINPWR_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_MAXPWR_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_PWRSTEP_IOLS)                          data[champ] = itset.value().toDouble();
        else if (champ == CP_MINCYL_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_MAXCYL_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_CYLSTEP_IOLS)                          data[champ] = itset.value().toDouble();
        else if (champ == CP_ADDINTERMEDIATE_IOLS)                  data[champ] = itset.value().toDouble();
        else if (champ == CP_ADDNEAR_IOLS)                          data[champ] = itset.value().toDouble();
        else if (champ == CP_CSTEAOPT_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_CSTEAECHO_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA0_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA1_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA2_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_HOLL1_IOLS)                            data[champ] = itset.value().toDouble();
        else if (champ == CP_HOFFERQ_IOLS)                          data[champ] = itset.value().toDouble();
        else if (champ == CP_BARRETTDF_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_BARRETTLF_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_OLSEN_IOLS)                            data[champ] = itset.value().toDouble();
        else if (champ == CP_DIAOPT_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_DIAALL_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_DIAINJECTEUR_IOLS)                     data[champ] = itset.value().toDouble();
        else if (champ == CP_TYPIMG_IOLS)                           data[champ] = itset.value().toString();
        else if (champ == CP_SINGLEPIECE_IOLS)                      data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_REMARQUE_IOLS)                         data[champ] = itset.value().toString();
        else if (champ == CP_OPTICMATERIAU_IOLS)                    data[champ] = itset.value().toString();
        else if (champ == CP_HYDROFILY_IOLS)                        data[champ] = itset.value().toString();
        else if (champ == CP_TYP_IOLS)                              data[champ] = itset.value().toInt();
        else if (champ == CP_PRECHARGE_IOLS)                        data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_JAUNE_IOLS)                            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_MULTIFOCAL_IOLS)                       data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_EDOF_IOLS)                             data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_TORIC_IOLS)                            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_JAUNE_IOLS)                            data[champ] = (itset.value().toInt() == 1);
        else if (champ == CP_ARRAYIMG_IOLS)                         data[champ] = QLatin1String(itset.value().toByteArray().toBase64());
        else if (champ == CP_RESULTSU_IOLS)                         data[champ] = itset.value().toInt();
        else if (champ == CP_CSTEAECHOU_IOLS)                       data[champ] = itset.value().toDouble();
        else if (champ == CP_CSTEAOPTU_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA0U_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA1U_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA2U_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HOLL1U_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_HOFFERQU_IOLS)                         data[champ] = itset.value().toDouble();
        else if (champ == CP_BARRETTDFU_IOLS)                       data[champ] = itset.value().toDouble();
        else if (champ == CP_BARRETTLFU_IOLS)                       data[champ] = itset.value().toDouble();
        else if (champ == CP_OLSENU_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_RESULTSO_IOLS)                         data[champ] = itset.value().toInt();
        else if (champ == CP_CSTEAOPTO_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA0O_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA1O_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HAIGISA2O_IOLS)                        data[champ] = itset.value().toDouble();
        else if (champ == CP_HOLL1O_IOLS)                           data[champ] = itset.value().toDouble();
        else if (champ == CP_HOFFERQO_IOLS)                         data[champ] = itset.value().toDouble();
     }
    iol = new IOL(data);
    if (iol != Q_NULLPTR)
        map_all->insert(iol->id(), iol);
    return iol;
}


IOLs::ImportResult IOLs::ImportListeIOLS(QDomDocument docxml, QWidget *parent)
{
    //! Liste des IOLs utilisés dans un programme opératoire : on ne les supprime pas.
    QList<int> listidiolsutilises;
    {
        bool ok;
        QString req = "select " CP_ID_IOLS " from " TBL_IOLS " where " CP_ID_IOLS " in (select " CP_IDIOL_LIGNPRGOPERATOIRE " from " TBL_LIGNESPRGOPERATOIRES ")";
        QList<QVariantList> l = DataBase::I()->StandardSelectSQL(req, ok);
        if (ok)
            for (int i=0; i<l.size(); ++i)
                listidiolsutilises << l.at(i).at(0).toInt();
    }
    QDomElement xml = docxml.documentElement();
    /*! Mise à jour de la liste des fabricants */
    QStringList listmanufacturers;
    for (int i=0; i<xml.childNodes().size(); i++)
    {
        QDomElement lensnode = xml.childNodes().at(i).toElement();
        for (int i=0; i<lensnode.childNodes().size(); i++)
        {
            QDomElement node = lensnode.childNodes().at(i).toElement();
            if (node.tagName() == "Manufacturer")
                if (!listmanufacturers.contains(node.text()))
                    listmanufacturers << node.text();
        }
    }
    listmanufacturers.sort();
    QStringList listmanofficiel;
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *manf = const_cast<Manufacturer*>(it.value());
        if (manf->nom().toUpper() == "JOHNSON & JOHNSON VISION")
            ItemsList::update(manf, CP_NOM_MANUFACTURER, "JOHNSON AND JOHNSON VISION");
        listmanofficiel << manf->nom().toUpper();
    }
    foreach (QString nommanufacturer, listmanufacturers)
    {
        if (!listmanofficiel.contains(nommanufacturer.toUpper()))
        {
            QHash<QString,QVariant> listbinds;
            listbinds[CP_NOM_MANUFACTURER] = nommanufacturer.toUpper();
            Datas::I()->manufacturers->CreationManufacturer(listbinds);
        }
    }
    /*! fin mise à jour de la liste des fabricants */

    /*! Mise à jour des IOLS - correction des modifs de la base 1 iolcon des IOLS */
    int id1stq = 0;
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *manf = const_cast<Manufacturer*>(it.value());
        if (manf->nom().toUpper() == "1STQ")
        {
            id1stq = manf->id();
            break;
        }
    }
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        QString optmaterial = Utils::trim(iol->opticalmaterial()).toLower();
        QString Hydrof = Utils::trim(iol->hydrofily()).toLower();
        if (optmaterial.split(" ").size()>1)
        {
            QString split = optmaterial;
            Hydrof = split.split(" ").at(1).toLower();
            optmaterial = split.split(" ").at(0).toLower();
        }
        if (optmaterial == "acrylic" || optmaterial == "hydrophobic")
            optmaterial ="acrylique";
        else if (optmaterial == "copolymer")
            optmaterial = "copolymère";
        ItemsList::update(iol, CP_OPTICMATERIAU_IOLS,optmaterial);
        ItemsList::update(iol, CP_HYDROFILY_IOLS, Hydrof);
        if (!iol->isedof() && !iol->ismultifocal())
        {
            if(iol->addnear() > 0)
                ItemsList::update(iol, CP_ADDNEAR_IOLS, 0.0);
            if(iol->addintermediate() > 0)
                ItemsList::update(iol, CP_ADDINTERMEDIATE_IOLS, 0.0);
        }
        if (iol->idmanufacturer() == id1stq)
            ItemsList::update(iol, CP_SINGLEPIECE_IOLS,true);
    }
    //! copy data Physiol IOL to BVI IOL
    QMap<int, QString> physioliolmap, BVIiolmap;
    int idbvi = -1;
    for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
    {
        Manufacturer *manf = const_cast<Manufacturer*>(it.value());
        if (manf->nom().toUpper() == "BVI")
            idbvi = manf->id();
    }
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
    {
        IOL *iol = const_cast<IOL*>(it.value());
        if( iol)
        {
            Manufacturer* manf = Datas::I()->manufacturers->getById(iol->idmanufacturer());
            if (manf)
            {
                if (manf->nom().toUpper() == "PHYSIOL")
                    physioliolmap.insert(iol->id(), iol->modele());
                else if (manf->nom().toUpper() == "BVI")
                    BVIiolmap.insert(iol->id(), iol->modele());
            }
        }
    }
    for (auto it = physioliolmap.cbegin(); it!= physioliolmap.cend(); ++it)
    {
        IOL *physioliol = Datas::I()->iols->getById(it.key());
        QImage img = physioliol->image();
        QString physioliolname = physioliol->modele();
        QStringList bvinameslist = BVIiolmap.values();
        if (bvinameslist.contains(physioliolname))
        {
            for (auto itbv = BVIiolmap.cbegin(); itbv!= BVIiolmap.cend(); ++itbv)
            {
                IOL *bviol = Datas::I()->iols->getById(itbv.key());
                if (bviol)
                {
                    if (bviol->modele() == Datas::I()->iols->getById(it.key())->modele())
                    {
                        bviol->setData(Datas::I()->iols->getById(itbv.key())->datas());
                        bviol->setidmanufacturer(idbvi);
                        bviol->setimage(img);
                        QHash <QString, QVariant> sets = bviol->datas().toVariantHash();
                        sets[CP_ARRAYIMG_IOLS] = physioliol->arrayimgiol();
                        DataBase::I()->UpDateImgIOL(bviol->id(), sets);
                        break;   //! le BVI de même modèle est trouvé et mis à jour : inutile de scruter les autres (était « itbv->cend() », sans effet)
                    }
                }
            }
        }
        else
        {
            QHash <QString, QVariant> sets = physioliol->datas().toVariantHash();
            sets[CP_IDMANUFACTURER_IOLS] = idbvi;
            sets[CP_ARRAYIMG_IOLS] = physioliol->arrayimgiol();
            Datas::I()->iols->CreationIOL(sets, parent);
        }
        if (!listidiolsutilises.contains(physioliol->id()))
            Datas::I()->iols->SupprimeIOL(physioliol);
    }
    /*! fin mise à jour de la liste des IOLs */

    /*! Mise à jour de la liste des IOLs */
    int newiols = 0;
    int updateiols = 0;
    QStringList iolbrandmodellist= QStringList();
    QList<int> iollistid = QList<int>();
    foreach (IOL *iolfromlist, *Datas::I()->iols->iols())
    {
        if (iolfromlist != Q_NULLPTR)
        {
            Manufacturer* man = Datas::I()->manufacturers->getById(iolfromlist->idmanufacturer());
            if (man != Q_NULLPTR)
                if (!iolbrandmodellist.contains(man->nom()) && !iolbrandmodellist.contains(man->nom().toUpper()) && !iolbrandmodellist.contains(man->nom().toLower()))
                {
                    iolbrandmodellist << man->nom().toUpper() + " " + iolfromlist->modele().toUpper();
                    iollistid << iolfromlist->id();
                }
        }
    }
    for (int i=0; i<xml.childNodes().size(); i++)
    {
        QDomElement childnode = xml.childNodes().at(i).toElement();
        if (childnode.tagName() == "Lens")
        {
            IOL iol;
            iol.setidiolcon(childnode.attribute("id").toInt());
            for (int j=0; j<childnode.childNodes().size(); j++)
            {
                QDomElement Lensnode = childnode.childNodes().at(j).toElement();
                if (Lensnode.tagName() == "Manufacturer")
                    iol.setstringid(Lensnode.text());
                else if (Lensnode.tagName() == "Name")
                {
                    //! La colonne modelname est un varchar(45) et MySQL tourne en STRICT_TRANS_TABLES :
                    //! un nom trop long ferait échouer l'INSERT (au lieu d'être tronqué en silence).
                    //! On tronque donc à 45 caractères dès la lecture (ex. IOLexport 2.3 : le Clareon
                    //! Vivity Toric AutonoMe d'Alcon fait 49 caractères).
                    iol.setmodele(Lensnode.text().left(45));
                }
                else if (Lensnode.tagName() == "Specifications")
                {
                    for (int k=0; k<Lensnode.childNodes().size(); k++)
                    {
                        QDomElement Specsnode = Lensnode.childNodes().at(k).toElement();
                        if (Specsnode.tagName() == "SinglePiece")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yes")
                                b = true;
                            iol.setsinglepiece(b);
                       }
                        else if (Specsnode.tagName() == "OpticMaterial")
                        {
                            QString optmaterial = Specsnode.text();
                            if (optmaterial == "acrylic")
                                iol.setOpticalMaterial("acrylique");
                            else if (optmaterial == "copolymer")
                                iol.setOpticalMaterial("copolymère");
                            else if (optmaterial == "silicone")
                                iol.setOpticalMaterial("silicone");
                            else
                                iol.setOpticalMaterial(optmaterial);
                            //if (!listitem.contains(Specsnode.text()))
                            //    listitem << Specsnode.text();
                        }
                        else if (Specsnode.tagName() == "HapticMaterial" && !iol.issinglepiece())
                        {
                            iol.setHapticalMaterial(Specsnode.text());
                        }
                        else if (Specsnode.tagName() == "Hydro")
                        {
                            QString Hydrofily = Specsnode.text();
                            if (Hydrofily == "hydrophilic")
                                iol.setHydrofily("hydrophile");
                            else if (Hydrofily == "hydrophobic")
                                iol.setHydrofily("hydrophobe");
                            else
                                iol.setHydrofily(Hydrofily);
                        }
                        else if (Specsnode.tagName() == "Preloaded")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yes")
                                b = true;
                            iol.setpreloaded(b);
                        }
                        else if (Specsnode.tagName() == "Toric")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yes")
                                b = true;
                            iol.setToric(b);
                        }
                        else if (Specsnode.tagName() == "OpticConcept")
                        {
                            bool b = false;
                            if (Specsnode.text()=="EDoF")
                            {
                                b = true;
                                iol.setEdof(b);
                            }
                            else if (Specsnode.text()=="multifocal" || Specsnode.text()=="bifocal")
                            {
                                b = true;
                                iol.setMultifocal(b);
                            }
                        }
                        else if (Specsnode.tagName() == "Filter")
                        {
                            bool b = false;
                            if (Specsnode.text()=="yellow")
                                b = true;
                            iol.setyellow(b);
                        }
                        else if (Specsnode.tagName() == "IncisionWidth")
                        {
                            if (Specsnode.text().toDouble() >0)
                                iol.setDiainjecteur(Specsnode.text().toDouble());
                        }
                        else if (Specsnode.tagName() == "InjectorSize")
                        {
                            if (Specsnode.text().toDouble() >0)
                                iol.setDiainjecteur(Specsnode.text().toDouble());
                        }
                        else if (Specsnode.tagName() == "OpticDiameter")
                            iol.setOpticalDiameter(Specsnode.text().toDouble());
                        else if (Specsnode.tagName() == "HapticDiameter")
                            iol.setDiaall(Specsnode.text().toDouble());
                        else if (Specsnode.tagName() == "HapticDesign")
                        {
                            if (Specsnode.text() == "Iris claw")
                                iol.setType(IOL_IRIEN);
                        }
                        else if (Specsnode.tagName() == "IntendedLocation" && iol.typeString() == "")
                        {
                            if (Specsnode.text()=="anterior chamber" || Specsnode.text()=="pre iridal")
                                iol.setType(IOL_CA);
                            else if (Specsnode.text()=="capsular bag" || Specsnode.text()=="retro iridal" || Specsnode.text()=="")
                                iol.setType(IOL_CP);
                            else if (Specsnode.text()=="sulcus ciliaris")
                                iol.setType(IOL_ADDON);
                        }
                    }
                }
                else if (Lensnode.tagName() == "Availability")
                {
                    QList<double> listpwr;
                    for (int k=0; k<Lensnode.childNodes().size(); k++)
                    {
                        QDomElement Availabilitynode = Lensnode.childNodes().at(k).toElement();
                        if (Availabilitynode.tagName() == "Sphere")
                        {
                            for (int l=0; l<Availabilitynode.childNodes().size(); l++)
                            {
                                QDomElement Spherenode = Availabilitynode.childNodes().at(l).toElement();
                                if (Spherenode.tagName() == "From")
                                    listpwr << Spherenode.text().toDouble();
                                else if (Spherenode.tagName() == "To")
                                    listpwr << Spherenode.text().toDouble();
                            }
                        }
                        if (Availabilitynode.tagName() == "Addition" && Availabilitynode.attribute("distance") == "intermediate")
                        {
                            if (Availabilitynode.text().toDouble() > 0)
                                iol.setAddintermediate(Availabilitynode.text().toDouble());
                        }
                        if (Availabilitynode.tagName() == "Addition" && Availabilitynode.attribute("distance") == "near")
                        {
                            if (Availabilitynode.text().toDouble() > 0)
                                iol.setAddnear(Availabilitynode.text().toDouble());
                        }
                    }
                    if (listpwr.size() >0)
                    {
                        auto mm = std::minmax_element(listpwr.begin(), listpwr.end());
                        iol.setPwrmin(*mm.first);
                        iol.setPwrmax(*mm.second);
                    }
                    if (iol.istoric())
                    {
                        listpwr.clear();
                        for (int k=0; k<Lensnode.childNodes().size(); k++)
                        {
                            QDomElement Availabilitynode = Lensnode.childNodes().at(k).toElement();
                            for (int l=0; l<Availabilitynode.childNodes().size(); l++)
                            {
                                if (Availabilitynode.tagName() == "Cylinder")
                                {
                                    QDomElement Cylindrenode = Availabilitynode.childNodes().at(l).toElement();
                                    if (Cylindrenode.tagName() == "From")
                                        listpwr << Cylindrenode.text().toDouble();
                                    else if (Cylindrenode.tagName() == "To")
                                        listpwr << Cylindrenode.text().toDouble();
                                    else if (Cylindrenode.tagName() == "Power")
                                        listpwr << Cylindrenode.text().toDouble();
                                }
                            }
                        }
                        if (listpwr.size() >0)
                        {
                            auto mm = std::minmax_element(listpwr.begin(), listpwr.end());
                            iol.setCylmin(*mm.first);
                            iol.setCylmax(*mm.second);
                        }
                    }
                }
                else if (Lensnode.tagName() == "Constants" && Lensnode.attribute("type")=="nominal")
                {
                    for (int i=0; i<Lensnode.childNodes().size(); i++)
                    {
                        QDomElement Constantnode = Lensnode.childNodes().at(i).toElement();
                        if (Constantnode.tagName() == "Ultrasound")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAEcho_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "SRKt")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAopt_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Haigis")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Haigisnode = Constantnode.childNodes().at(i).toElement();
                                if (Haigisnode.tagName() == "a0")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa0_nominal(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a1")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa1_nominal(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a2")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa2_nominal(Haigisnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "HofferQ")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHofferQ_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Holladay1")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHolladay1_nominal(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Barrett")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Barrettnode = Constantnode.childNodes().at(i).toElement();
                                if (Barrettnode.tagName() == "DF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettDF_nominal(Barrettnode.text().toDouble());
                                }
                                else if (Barrettnode.tagName() == "LF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettLF_nominal(Barrettnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "Olsen")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setOlsen_nominal(Constantnode.text().toDouble());
                        }
                    }
                }
                else if (Lensnode.tagName() == "Constants" && Lensnode.attribute("type")=="ULIB")
                {
                    iol.setresults_ulib(Lensnode.attribute("results").toInt());
                    for (int i=0; i<Lensnode.childNodes().size(); i++)
                    {
                        QDomElement Constantnode = Lensnode.childNodes().at(i).toElement();
                        if (Constantnode.tagName() == "Ultrasound")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAEcho_ulib(Constantnode.text().toDouble());
                        }
                        if (Constantnode.tagName() == "SRKt")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAopt_ulib(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Haigis")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Haigisnode = Constantnode.childNodes().at(i).toElement();
                                if (Haigisnode.tagName() == "a0")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa0_ulib(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a1")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa1_ulib(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a2")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa2_ulib(Haigisnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "HofferQ")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHofferQ_ulib(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Holladay1")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHolladay1_ulib(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Barrett")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Barrettnode = Constantnode.childNodes().at(i).toElement();
                                if (Barrettnode.tagName() == "DF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettDF_ulib(Barrettnode.text().toDouble());
                                }
                                else if (Barrettnode.tagName() == "LF")
                                {
                                    if (Barrettnode.text().toDouble() !=0)
                                        iol.setBarrettLF_ulib(Barrettnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "Olsen")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setOlsen_ulib(Constantnode.text().toDouble());
                        }
                    }
                }
                else if (Lensnode.tagName() == "Constants" && Lensnode.attribute("type")=="optimized" && Lensnode.attribute("results").toInt() >0)
                {
                    iol.setresults_optimized(Lensnode.attribute("results").toInt());
                    for (int i=0; i<Lensnode.childNodes().size(); i++)
                    {
                        QDomElement Constantnode = Lensnode.childNodes().at(i).toElement();
                        if (Constantnode.tagName() == "SRKt")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setCsteAopt_optimized(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Haigis")
                            for (int i=0; i<Constantnode.childNodes().size(); i++)
                            {
                                QDomElement Haigisnode = Constantnode.childNodes().at(i).toElement();
                                if (Haigisnode.tagName() == "a0")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa0_optimized(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a1")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa1_optimized(Haigisnode.text().toDouble());
                                }
                                else if (Haigisnode.tagName() == "a2")
                                {
                                    if (Haigisnode.text().toDouble() !=0)
                                        iol.setHaigisa2_optimized(Haigisnode.text().toDouble());
                                }
                            }
                        else if (Constantnode.tagName() == "HofferQ")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHofferQ_optimized(Constantnode.text().toDouble());
                        }
                        else if (Constantnode.tagName() == "Holladay1")
                        {
                            if (Constantnode.text().toDouble() !=0)
                                iol.setHolladay1_optimized(Constantnode.text().toDouble());
                        }
                    }
                }
            }
            Manufacturer *man = Q_NULLPTR;
            for (auto it = Datas::I()->manufacturers->manufacturers()->constBegin(); it != Datas::I()->manufacturers->manufacturers()->constEnd(); ++it)
            {
                Manufacturer *manf = const_cast<Manufacturer*>(it.value());
                if (manf)
                    if (manf->nom().toUpper() == iol.stringid().toUpper())
                    {
                        man = manf;
                        break;
                    }
            }
            if (!man)
                continue;
            int idx = iolbrandmodellist.indexOf(iol.stringid().toUpper() + " " + iol.modele().toUpper());
            if (idx > -1)
            {
                //! update existant iol
                IOL* ioloflist = Datas::I()->iols->getById(iollistid.at(idx));
                if (ioloflist != Q_NULLPTR)
                {
                    //! update existant iol
                    ItemsList::update(ioloflist, CP_MODELNAME_IOLS,       iol.modele());
                    ItemsList::update(ioloflist, CP_IDMANUFACTURER_IOLS,  man->id());
                    ItemsList::update(ioloflist, CP_CSTEAECHO_IOLS,       iol.csteAEcho_nominal());

                    ItemsList::update(ioloflist, CP_CSTEAOPT_IOLS,        iol.csteAopt_nominal());
                    ItemsList::update(ioloflist, CP_HAIGISA0_IOLS,        iol.haigisa0_nominal());
                    ItemsList::update(ioloflist, CP_HAIGISA1_IOLS,        iol.haigisa1_nominal());
                    ItemsList::update(ioloflist, CP_HAIGISA2_IOLS,        iol.haigisa2_nominal());
                    ItemsList::update(ioloflist, CP_HOLL1_IOLS,           iol.holladay1_nominal());
                    ItemsList::update(ioloflist, CP_HOFFERQ_IOLS,         iol.hofferQ_nominal());
                    ItemsList::update(ioloflist, CP_BARRETTDF_IOLS,       iol.barrettDF_nominal());
                    ItemsList::update(ioloflist, CP_BARRETTLF_IOLS,       iol.barrettLF_nominal());
                    ItemsList::update(ioloflist, CP_OLSEN_IOLS,           iol.olsen_nominal());

                    ItemsList::update(ioloflist, CP_ACD_IOLS,             iol.acd());
                    ItemsList::update(ioloflist, CP_OPTICMATERIAU_IOLS,   iol.opticalmaterial());
                    ItemsList::update(ioloflist, CP_HYDROFILY_IOLS,       iol.hydrofily());
                    ItemsList::update(ioloflist, CP_HAPTICMATERIAU_IOLS,  iol.hapticalmaterial());
                    ItemsList::update(ioloflist, CP_DIAALL_IOLS,          iol.diaall());
                    ItemsList::update(ioloflist, CP_DIAOPT_IOLS,          iol.opticdiameter());
                    ItemsList::update(ioloflist, CP_DIAINJECTEUR_IOLS,    iol.diainjecteur());
                    ItemsList::update(ioloflist, CP_PRECHARGE_IOLS,       iol.ispreloaded());
                    ItemsList::update(ioloflist, CP_MAXPWR_IOLS,          iol.pwrmax());
                    ItemsList::update(ioloflist, CP_MINPWR_IOLS,          iol.pwrmin());
                    ItemsList::update(ioloflist, CP_MAXCYL_IOLS,          iol.cylmax());
                    ItemsList::update(ioloflist, CP_MINCYL_IOLS,          iol.cylmin());
                    ItemsList::update(ioloflist, CP_ADDINTERMEDIATE_IOLS, iol.addintermediate());
                    ItemsList::update(ioloflist, CP_ADDNEAR_IOLS,         iol.addnear());
                    ItemsList::update(ioloflist, CP_JAUNE_IOLS,           iol.isyellow());
                    ItemsList::update(ioloflist, CP_MULTIFOCAL_IOLS,      iol.ismultifocal());
                    ItemsList::update(ioloflist, CP_EDOF_IOLS,            iol.isedof());
                    ItemsList::update(ioloflist, CP_TORIC_IOLS,           iol.istoric());
                    ItemsList::update(ioloflist, CP_SINGLEPIECE_IOLS,     iol.issinglepiece());

                    ItemsList::update(ioloflist, CP_RESULTSU_IOLS,        iol.results_ulib());
                    ItemsList::update(ioloflist, CP_CSTEAECHOU_IOLS,      iol.csteAEcho_ulib());
                    ItemsList::update(ioloflist, CP_CSTEAOPTU_IOLS,       iol.csteAopt_ulib());
                    ItemsList::update(ioloflist, CP_HAIGISA0U_IOLS,       iol.haigisa0_ulib());
                    ItemsList::update(ioloflist, CP_HAIGISA1U_IOLS,       iol.haigisa1_ulib());
                    ItemsList::update(ioloflist, CP_HAIGISA2U_IOLS,       iol.haigisa2_ulib());
                    ItemsList::update(ioloflist, CP_HOLL1U_IOLS,          iol.holladay1_ulib());
                    ItemsList::update(ioloflist, CP_HOFFERQU_IOLS,        iol.hofferQ_ulib());
                    ItemsList::update(ioloflist, CP_BARRETTDFU_IOLS,      iol.barrettDF_ulib());
                    ItemsList::update(ioloflist, CP_BARRETTLFU_IOLS,      iol.barrettLF_ulib());
                    ItemsList::update(ioloflist, CP_OLSENU_IOLS,          iol.olsen_ulib());

                    ItemsList::update(ioloflist, CP_RESULTSO_IOLS,        iol.results_optimized());
                    ItemsList::update(ioloflist, CP_CSTEAOPTO_IOLS,       iol.csteAopt_optimized());
                    ItemsList::update(ioloflist, CP_HAIGISA0O_IOLS,       iol.haigisa0_optimized());
                    ItemsList::update(ioloflist, CP_HAIGISA1O_IOLS,       iol.haigisa1_optimized());
                    ItemsList::update(ioloflist, CP_HAIGISA2O_IOLS,       iol.haigisa2_optimized());
                    ItemsList::update(ioloflist, CP_HOLL1O_IOLS,          iol.holladay1_optimized());
                    ItemsList::update(ioloflist, CP_HOFFERQO_IOLS,        iol.hofferQ_optimized());

                    ItemsList::update(ioloflist, CP_TYP_IOLS,             iol.type());
                    ++ updateiols;
                }
            }
            else //! newiol
            {
                QHash<QString, QVariant> m_listbinds;
                m_listbinds[CP_MODELNAME_IOLS]      = iol.modele();
                m_listbinds[CP_IDMANUFACTURER_IOLS] = man->id();
                m_listbinds[CP_CSTEAECHO_IOLS]      = (iol.csteAEcho_nominal() >0.0?    iol.csteAEcho_nominal()     : QVariant());

                m_listbinds[CP_CSTEAOPT_IOLS]       = (iol.csteAopt_nominal() >0.0?     iol.csteAopt_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA0_IOLS]       = (iol.haigisa0_nominal() >0.0?     iol.haigisa0_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA1_IOLS]       = (iol.haigisa1_nominal() >0.0?     iol.haigisa1_nominal()      : QVariant());
                m_listbinds[CP_HAIGISA2_IOLS]       = (iol.haigisa2_nominal() >0.0?     iol.haigisa2_nominal()      : QVariant());
                m_listbinds[CP_HOLL1_IOLS]          = (iol.holladay1_nominal()>0.0?     iol.holladay1_nominal()     : QVariant());
                m_listbinds[CP_HOFFERQ_IOLS]        = (iol.hofferQ_nominal()>0.0?       iol.hofferQ_nominal()       : QVariant());
                m_listbinds[CP_BARRETTDF_IOLS]      = (iol.barrettDF_nominal()>0.0?     iol.barrettDF_nominal()     : QVariant());
                m_listbinds[CP_BARRETTLF_IOLS]      = (iol.barrettLF_nominal()>0.0?     iol.barrettLF_nominal()     : QVariant());
                m_listbinds[CP_OLSEN_IOLS]          = (iol.olsen_nominal()>0.0?         iol.olsen_nominal()         : QVariant());

                m_listbinds[CP_ACD_IOLS]            = (iol.acd()>0.0?                   iol.acd()                   : QVariant());
                m_listbinds[CP_OPTICMATERIAU_IOLS]  = iol.opticalmaterial();
                m_listbinds[CP_HYDROFILY_IOLS]      = iol.hydrofily();
                m_listbinds[CP_HAPTICMATERIAU_IOLS] = iol.hapticalmaterial();
                m_listbinds[CP_DIAALL_IOLS]         = (iol.diaall() >0.0?               iol.diaall()                : QVariant());
                m_listbinds[CP_DIAOPT_IOLS]         = (iol.opticdiameter() >0.0?        iol.opticdiameter()         : QVariant());
                m_listbinds[CP_DIAINJECTEUR_IOLS]   = (iol.diainjecteur() >0.0?         iol.diainjecteur()          : QVariant());
                m_listbinds[CP_PRECHARGE_IOLS]      = (iol.ispreloaded()?               "1"                         : QVariant());
                m_listbinds[CP_MAXPWR_IOLS]         = iol.pwrmax();
                m_listbinds[CP_MINPWR_IOLS]         = iol.pwrmin();
                m_listbinds[CP_MAXCYL_IOLS]         = (iol.istoric()?                   iol.cylmax()                : QVariant());
                m_listbinds[CP_MINCYL_IOLS]         = (iol.istoric()?                   iol.cylmin()                : QVariant());
                m_listbinds[CP_ADDINTERMEDIATE_IOLS]= (iol.addintermediate() >0.0?      iol.addintermediate()       : QVariant());
                m_listbinds[CP_ADDNEAR_IOLS]        = (iol.addnear() >0.0?              iol.addnear()               : QVariant());
                m_listbinds[CP_JAUNE_IOLS]          = (iol.isyellow()?                  "1"                         : QVariant());
                m_listbinds[CP_MULTIFOCAL_IOLS]     = (iol.ismultifocal()?              "1"                         : QVariant());
                m_listbinds[CP_EDOF_IOLS]           = (iol.isedof()?                    "1"                         : QVariant());
                m_listbinds[CP_TORIC_IOLS]          = (iol.istoric()?                   "1"                         : QVariant());
                m_listbinds[CP_SINGLEPIECE_IOLS]    = (iol.issinglepiece()?             "1"                         : QVariant());

                m_listbinds[CP_RESULTSU_IOLS]       = (iol.results_ulib() >0?           iol.results_ulib()          : QVariant());
                m_listbinds[CP_CSTEAECHOU_IOLS]     = (iol.csteAEcho_ulib() >0.0?       iol.csteAEcho_ulib()        : QVariant());
                m_listbinds[CP_CSTEAOPTU_IOLS]      = (iol.csteAopt_ulib() >0.0?        iol.csteAopt_ulib()         : QVariant());
                m_listbinds[CP_HAIGISA0U_IOLS]      = (iol.haigisa0_ulib() >0.0?        iol.haigisa0_ulib()         : QVariant());
                m_listbinds[CP_HAIGISA1U_IOLS]      = (iol.haigisa1_ulib() >0.0?        iol.haigisa1_ulib()         : QVariant());
                m_listbinds[CP_HAIGISA2U_IOLS]      = (iol.haigisa2_ulib() >0.0?        iol.haigisa2_ulib()         : QVariant());
                m_listbinds[CP_HOLL1U_IOLS]         = (iol.holladay1_ulib()>0.0?        iol.holladay1_ulib()        : QVariant());
                m_listbinds[CP_HOFFERQU_IOLS]       = (iol.hofferQ_ulib()>0.0?          iol.hofferQ_ulib()          : QVariant());
                m_listbinds[CP_BARRETTDFU_IOLS]     = (iol.barrettDF_ulib()>0.0?        iol.barrettDF_ulib()        : QVariant());
                m_listbinds[CP_BARRETTLFU_IOLS]     = (iol.barrettLF_ulib()>0.0?        iol.barrettLF_ulib()        : QVariant());
                m_listbinds[CP_OLSENU_IOLS]         = (iol.olsen_ulib()>0.0?            iol.olsen_ulib()            : QVariant());

                m_listbinds[CP_RESULTSO_IOLS]       = (iol.results_optimized() >0?      iol.results_optimized()     : QVariant());
                m_listbinds[CP_CSTEAOPTO_IOLS]      = (iol.csteAopt_optimized() >0.0?   iol.csteAopt_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA0O_IOLS]      = (iol.haigisa0_optimized() >0.0?   iol.haigisa0_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA1O_IOLS]      = (iol.haigisa1_optimized() >0.0?   iol.haigisa1_optimized()    : QVariant());
                m_listbinds[CP_HAIGISA2O_IOLS]      = (iol.haigisa2_optimized() >0.0?   iol.haigisa2_optimized()    : QVariant());
                m_listbinds[CP_HOLL1O_IOLS]         = (iol.holladay1_optimized()>0.0?   iol.holladay1_optimized()   : QVariant());
                m_listbinds[CP_HOFFERQO_IOLS]       = (iol.hofferQ_optimized()>0.0?     iol.hofferQ_optimized()     : QVariant());
                m_listbinds[CP_TYP_IOLS]            = (iol.type()>0?                    iol.type()                  : QVariant());
                Datas::I()->iols->CreationIOL(m_listbinds, parent);
                ++newiols;
            }
        }
    }
    //for (int i = 0; i<listitem.size();++i)
    //    qDebug()<<listitem.at(i);
    /*! fin mise à jour de la liste des IOLs */

    //! Redimensionnement des images surdimensionnées : UNE SEULE FOIS, ici après l'import, et non au
    //! survol de la souris comme auparavant (ce qui écrivait en base à chaque passage de souris sur une
    //! ligne). resizeiolimage ne touche que les images bitmap dépassant le seuil (SIZEMAXIMGIOL).
    for (auto it = Datas::I()->iols->iols()->constBegin(); it != Datas::I()->iols->iols()->constEnd(); ++it)
        resizeiolimage(const_cast<IOL*>(it.value()));
    return { newiols, updateiols, int(Datas::I()->iols->iols()->size()) };
}

void IOLs::resizeiolimage(IOL *iol)
{
    const int maxsizeimg = SIZEMAXIMGIOL;
    //! On ne traite que les images bitmap. Il faut des && : « si le format n'est AUCUN des trois, on
    //! sort ». L'ancien code utilisait ||, condition toujours vraie (un format ne peut différer à la
    //! fois de JPG, PNG et JPEG) → la fonction sortait systématiquement et ne redimensionnait jamais rien.
    if (iol->imageformat() != JPG && iol->imageformat() != PNG && iol->imageformat() != JPEG)
        return;
    if (iol->arrayimgiol().size() < maxsizeimg)
        return;

    //! Compression entièrement EN MÉMOIRE, déléguée à Utils::CompressImageToJPG (rapide, sans
    //! fichier temporaire). L'ancienne version passait par CompressFileToJPG avec un fichier posé
    //! dans ProvDir, ce qui échouait ici (collision fichier d'origine / fichier compressé).
    QByteArray ba = Utils::CompressImageToJPG(iol->arrayimgiol(), maxsizeimg);
    if (ba.isEmpty())
        return;

    QHash<QString, QVariant> m_listbinds;
    m_listbinds[CP_ARRAYIMG_IOLS] = ba;
    m_listbinds[CP_TYPIMG_IOLS] = JPG;
    DataBase::I()->UpDateImgIOL(iol->id(), m_listbinds);
}
