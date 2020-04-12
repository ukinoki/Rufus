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

#include "dlg_listeiols.h"

dlg_listeiols::dlg_listeiols(QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setWindowTitle(tr("Liste des IOLs"));
    wdg_manufacturerscombo = new UpComboBox();

    wdg_iolstree = new QTreeView(this);
    wdg_iolstree ->setFixedWidth(320);
    wdg_iolstree ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_iolstree ->setFocusPolicy(Qt::StrongFocus);
    wdg_iolstree ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_iolstree ->setAnimated(true);
    wdg_iolstree ->setIndentation(10);
    wdg_iolstree ->setMouseTracking(true);
    wdg_iolstree ->header()->setVisible(false);
    bool reconstruirelaliste = true;
    ReconstruitTreeViewIOLs(reconstruirelaliste);

    wdg_manufacturerscombo = new UpComboBox();
    wdg_manufacturerscombo->setEditable(false);
    wdg_manufacturerscombo->addItem(tr("Tous"), 0);
    wdg_manufacturerscombo->setFixedWidth(250);
    QHBoxLayout *manufacturerlay = new QHBoxLayout();
    manufacturerlay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    manufacturerlay     ->addWidget(wdg_manufacturerscombo);
    manufacturerlay     ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    manufacturerlay     ->setContentsMargins(0,0,0,5);

    for (int i=0; i<m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm)
        {
            Manufacturer *man = dynamic_cast<Manufacturer*>(itm->item());
            if (man)
                wdg_manufacturerscombo->addItem(man->nom(), man->id());
        }
    }

    wdg_buttonframe         = new WidgetButtonFrame(wdg_iolstree);
    wdg_buttonframe         ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);

    wdg_label               = new UpLabel();
    wdg_label               ->setFixedSize(21,21);
    wdg_label               ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    wdg_chercheuplineedit   = new UpLineEdit();
    wdg_chercheuplineedit   ->setFixedSize(140,25);
    wdg_chercheuplineedit   ->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}");
    wdg_buttonframe->layButtons()->insertWidget(0,wdg_label);
    wdg_buttonframe->layButtons()->insertWidget(0,wdg_chercheuplineedit);
    AjouteLayButtons(UpDialog::ButtonOK);
    UpPushButton *importbutt =   new UpPushButton(tr("Mettre à jour"));
    importbutt->setIcon(Icons::icImport());
    AjouteWidgetLayButtons(importbutt, false);
    connect(importbutt, &QPushButton::clicked, this, &dlg_listeiols::ImportListeIOLS);

    dlglayout()->insertWidget(0,wdg_buttonframe->widgButtonParent());
    dlglayout()->insertLayout(0,manufacturerlay);

    connect(OKButton,               &QPushButton::clicked,      this,   &QDialog::reject);
    connect(wdg_chercheuplineedit,  &QLineEdit::textEdited,     this,   [=] (QString txt) {
                                                                                            wdg_chercheuplineedit->setText(txt);
                                                                                            ReconstruitTreeViewIOLs(false, txt);
                                                                                           });
    connect(wdg_buttonframe,        &WidgetButtonFrame::choix,  this,   &dlg_listeiols::ChoixButtonFrame);
    connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(false); } );

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(false);
    wdg_buttonframe->wdg_moinsBouton    ->setEnabled(false);
    wdg_chercheuplineedit               ->setFocus();
    QString req = "select " CP_ID_IOLS " from " TBL_IOLS " where " CP_ID_IOLS " in (select " CP_IDIOL_LIGNPRGOPERATOIRE " from " TBL_LIGNESPRGOPERATOIRES ")";
    bool ok;
    QList<QVariantList> listidiols = DataBase::I()->StandardSelectSQL(req,ok);
    if (ok)
        for (int i=0; i<listidiols.size(); ++i)
            m_listidiolsutilises << listidiols.at(i).at(0).toInt();
}

dlg_listeiols::~dlg_listeiols()
{
}

void dlg_listeiols::Enablebuttons(QModelIndex idx)
{
    IOL *iol = getIOLFromIndex(idx);
    if (iol != Q_NULLPTR)
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(true);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(m_listidiolsutilises.indexOf(iol->id()) == -1);
    }
    else
    {
        wdg_buttonframe->wdg_modifBouton->setEnabled(false);
        wdg_buttonframe->wdg_moinsBouton->setEnabled(false);
    }
}


void dlg_listeiols::ChoixButtonFrame()
{
    IOL *iol = Q_NULLPTR;
    if (wdg_iolstree->selectionModel()->selectedIndexes().size())
        iol = getIOLFromIndex(wdg_iolstree->selectionModel()->selectedIndexes().at(0));
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        EnregistreNouveauIOL();
        break;
    case WidgetButtonFrame::Modifier:
        if (iol)
            ModifIOL(iol);
        break;
    case WidgetButtonFrame::Moins:
        if (iol)
            SupprIOL(iol);
        break;
    }
}

bool dlg_listeiols::listeIOLsmodifiee() const
{
    return m_listemodifiee;
}

// ------------------------------------------------------------------------------------------
// Enregistre un nouveau correpondant
// ------------------------------------------------------------------------------------------
void dlg_listeiols::EnregistreNouveauIOL()
{
    dlg_identificationIOL *Dlg_IdentIOL    = new dlg_identificationIOL(dlg_identificationIOL::Creation);
    if (!Dlg_IdentIOL->initok())
        return;
    if (Dlg_IdentIOL->exec()>0)
    {
        IOL *iol = Dlg_IdentIOL->currentIOL();
        m_listemodifiee = true;
        wdg_manufacturerscombo->disconnect();
        wdg_manufacturerscombo->setCurrentIndex(0);
        connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(false); } );
        ReconstruitTreeViewIOLs(true);
        if (iol)
            scrollToIOL(iol);
    }
    delete Dlg_IdentIOL;
}
// ------------------------------------------------------------------------------------------
// renvoie l'IOL correspondant à l'index
// ------------------------------------------------------------------------------------------
IOL* dlg_listeiols::getIOLFromIndex(QModelIndex idx )
{
    UpStandardItem *it = dynamic_cast<UpStandardItem*>(m_IOLsmodel->itemFromIndex(idx));
    if (it != Q_NULLPTR)
        return dynamic_cast<IOL *>(it->item());
    else
        return Q_NULLPTR;
}

/*-----------------------------------------------------------------------------------------------------------------
-- // mise à jour de la liste des implants  --------------------------
-----------------------------------------------------------------------------------------------------------------*/
void dlg_listeiols::ImportListeIOLS()
{
    UpMessageBox::Watch(this,tr("Mise à jour de la liste des implants"), tr("Pour mettre à jour la liste des implants") + "\n"
                        + tr("Téléchargez la liste des implants - fichier IOLexport.xml - sur le site https://iolcon.org/") + "\n"
                        + tr("Sélectionnez ce fichier une fois téléchargé dans la boîte de dialogue qui suit") + "\n"
                        + tr("Aucun implant de votre base actuelle ne sera modifié"));

    /*! Choix du fichier xml contenant la liste des implants */
    QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), desktop,  "*.xml");
    if (fileName == "")
        return;
    QFile xmldoc(fileName);
    int nbimplants = 0;
    if (xmldoc.open(QIODevice::ReadOnly))
    {
        QDomDocument *docxml = new QDomDocument();
        docxml->setContent(&xmldoc);
        QDomElement xml = docxml->documentElement();
        /*! Mise à jour de la liste des fabricants */
        Datas::I()->manufacturers->initListe();
        Datas::I()->iols->initListe();
        QStringList listmanufacturers;
        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement lensnode = xml.childNodes().at(i).toElement();
            QString lens = lensnode.tagName();
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
        foreach(Manufacturer *man, Datas::I()->manufacturers->manufacturers()->values())
            listmanofficiel << man->nom().toUpper();
        foreach (QString nommanufacturer, listmanufacturers)
            if (!listmanofficiel.contains(nommanufacturer.toUpper()))
            {
                QHash<QString,QVariant> listbinds;
                listbinds[CP_NOM_MANUFACTURER] = nommanufacturer.toUpper();
                Datas::I()->manufacturers->CreationManufacturer(listbinds);
            }
        /*! fin mise à jour de la liste des fabricants */

        /*! Mise à jour de la liste des IOLs */
        for (int i=0; i<xml.childNodes().size(); i++)     // on reprend chaque IOL un par un
        {
            int idiol;
            QString fabricant  = "";
            QString modele = "";
            QString materiau = "";
            bool precharge = false;
            double incision= 0.0;
            double diaoptique = 0.0;
            double diaall = 0.0;
            bool multifocal = false;
            bool toric = false;
            bool jaune = false;
            bool edof = false;
            double pwrmin = 100.0;
            double pwrmax = -100.0;
            double cylmin = 100.0;
            double cylmax = -100.0;
            double add = 0.0;
            double srkt = 0.0;
            double haigis0 = 0.0;
            double haigis1 = 0.0;
            double haigis2 = 0.0;
            double hofferQ = 0.0;
            double holladay = 0.0;
            double barettdf = 0.0;
            double barettlf = 0.0;
            double olsen = 0.0;
            QDomElement lensnode = xml.childNodes().at(i).toElement();
            idiol = lensnode.attributeNode("id").value().toInt();
            QString lens = lensnode.tagName();
            for (int i=0; i<lensnode.childNodes().size(); i++)
            {
                QDomElement node = lensnode.childNodes().at(i).toElement();
                if (node.tagName() == "Manufacturer")
                    fabricant = node.text();
                else if (node.tagName() == "Name")
                    modele = node.text();
                else if (node.tagName() == "Specifications")
                    for (int i=0; i<node.childNodes().size(); i++)
                    {
                        QDomElement nodespec = node.childNodes().at(i).toElement();
                        if (nodespec.tagName() == "OpticMaterial")
                            materiau = nodespec.text();
                        else if (nodespec.tagName() == "Hydro")
                        {
                            if (nodespec.text() != "")
                                materiau += " " + nodespec.text();
                        }
                        else if (nodespec.tagName() == "Preloaded")
                            precharge = (nodespec.text() != "no");
                        else if (nodespec.tagName() == "Filter")
                            jaune = (nodespec.text() == "yellow");
                        else if (nodespec.tagName() == "IncisionWidth")
                            incision = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "OpticDiameter")
                            diaoptique = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "HapticDiameter")
                            diaall = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "OpticConcept")
                        {
                            multifocal = (nodespec.text() == "multifocal");
                            edof = (nodespec.text() == "EDoF");
                        }
                        else if (nodespec.tagName() == "Toric")
                            toric = (nodespec.text() != "no");
                    }
                else if (node.tagName() == "Availability")
                    for (int i=0; i<node.childNodes().size(); i++)
                    {
                        QDomElement nodespec = node.childNodes().at(i).toElement();
                        if (nodespec.tagName() == "Sphere")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodepwr = nodespec.childNodes().at(i).toElement();
                                if (nodepwr.tagName() == "From")
                                    if (pwrmin > 99.0 || pwrmin > nodepwr.text().toDouble())
                                        pwrmin = nodepwr.text().toDouble();
                                if (nodepwr.tagName() == "To")
                                    if (pwrmax < -99.0 || pwrmax < nodepwr.text().toDouble())
                                        pwrmax = nodepwr.text().toDouble();
                            }
                        else if (nodespec.tagName() == "Cylinder")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodecyl = nodespec.childNodes().at(i).toElement();
                                if (nodecyl.tagName() == "From")
                                    if (cylmin > 99.0 || cylmin > nodecyl.text().toDouble())
                                    cylmin = nodecyl.text().toDouble();
                                if (nodecyl.tagName() == "To")
                                    if (cylmax < -99.0 || cylmax < nodecyl.text().toDouble())
                                        cylmax = nodecyl.text().toDouble();
                            }
                        else if (nodespec.tagName() == "Addition")
                            add = nodespec.text().toDouble();
                    }
                else if (node.tagName() == "Constants" && node.attributeNode("type").value() == "nominal")
                    for (int i=0; i<node.childNodes().size(); i++)
                    {
                        QDomElement nodespec = node.childNodes().at(i).toElement();
                        if (nodespec.tagName() == "SRKt")
                            srkt = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "Haigis")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodehaigis = nodespec.childNodes().at(i).toElement();
                                if (nodehaigis.tagName() == "a0")
                                    haigis0 = nodehaigis.text().toDouble();
                                else if (nodehaigis.tagName() == "a1")
                                    haigis1 = nodehaigis.text().toDouble();
                                else if (nodehaigis.tagName() == "a2")
                                    haigis2 = nodehaigis.text().toDouble();
                            }
                        else if (nodespec.tagName() == "HofferQ")
                            hofferQ = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "Holladay1")
                            holladay = nodespec.text().toDouble();
                        else if (nodespec.tagName() == "Barrett")
                            for (int i=0; i<nodespec.childNodes().size(); i++)
                            {
                                QDomElement nodebarrett = nodespec.childNodes().at(i).toElement();
                                if (nodebarrett.tagName() == "DF")
                                    barettdf = nodebarrett.text().toDouble();
                                if (nodebarrett.tagName() == "LF")
                                    barettlf = nodebarrett.text().toDouble();
                            }
                        else if (nodespec.tagName() == "Olsen")
                            olsen = nodespec.text().toDouble();
                    }
            }
            bool foundiol = false;
            if (pwrmin > 99.0) pwrmin= 0.0;
            if (pwrmax < -99.0) pwrmax= 0.0;
            if (cylmin > 99.0) cylmin= 0.0;
            if (cylmax < -99.0) cylmax= 0.0;
            Manufacturer *man = Q_NULLPTR;
            foreach (Manufacturer *manf, Datas::I()->manufacturers->manufacturers()->values())
            {
                if (manf)
                    if (manf->nom().toUpper() == fabricant.toUpper())
                    {
                        man = manf;
                        break;
                    }
            }
            if (!man)
                continue;
            foreach (IOL *iol, *Datas::I()->iols->iols())
                if (iol->modele().toUpper() == modele.toUpper() && man->nom().toUpper() == fabricant.toUpper())
                {
                    foundiol = true;
                    break;
                }
            if (!foundiol && man != Q_NULLPTR)
            {
                QHash<QString, QVariant> m_listbinds;
                m_listbinds[CP_MODELNAME_IOLS]      = modele;
                m_listbinds[CP_IDMANUFACTURER_IOLS] = man->id();
                m_listbinds[CP_CSTEAECHO_IOLS]      = (srkt >0.0?    srkt   : QVariant());
                m_listbinds[CP_HAIGISA0_IOLS]       = (haigis0 >0.0?  haigis0 : QVariant());
                m_listbinds[CP_HAIGISA1_IOLS]       = (haigis1 >0.0?  haigis1 : QVariant());
                m_listbinds[CP_HAIGISA2_IOLS]       = (haigis2 >0.0?  haigis2 : QVariant());
                m_listbinds[CP_MATERIAU_IOLS]       = materiau;
                m_listbinds[CP_DIAALL_IOLS]         = (diaall >0.0?  diaall : QVariant());
                m_listbinds[CP_DIAOPT_IOLS]         = (diaoptique >0.0? diaoptique  : QVariant());
                m_listbinds[CP_DIAINJECTEUR_IOLS]   = (incision >0.0? incision : QVariant());
                m_listbinds[CP_PRECHARGE_IOLS]      = (precharge?   "1" : QVariant());
                m_listbinds[CP_MAXPWR_IOLS]         = pwrmax;
                m_listbinds[CP_MINPWR_IOLS]         = pwrmin;
                m_listbinds[CP_MAXCYL_IOLS]         = (toric? cylmax : QVariant());
                m_listbinds[CP_MINCYL_IOLS]         = (toric? cylmin : QVariant());
                m_listbinds[CP_JAUNE_IOLS]          = (jaune?       "1" : QVariant());
                m_listbinds[CP_MULTIFOCAL_IOLS]     = (multifocal?  "1" : QVariant());
                m_listbinds[CP_EDOF_IOLS]           = (edof?        "1" : QVariant());
                m_listbinds[CP_TORIC_IOLS]          = (toric?       "1" : QVariant());
                Datas::I()->iols->CreationIOL(m_listbinds);
                ++nbimplants;
            }
        }
    }
    /*! fin mise à jour de la liste des IOLs */
    QString msg = "Aucun implant n'a été rajouté à la base";
    switch (nbimplants) {
    case 0:
        break;
    case 1:
        msg = "Un implant a été rajouté à la base";
        break;
    default:
        msg = QString::number(nbimplants) + " implants ont été rajoutés à la base";
    }
    UpMessageBox::Watch(this, msg);
    ReconstruitListeManufacturers();
    ReconstruitTreeViewIOLs();
}

// ------------------------------------------------------------------------------------------
// Modifie un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::ModifIOL(IOL *iol)
{
    if (iol == Q_NULLPTR)
        return;
    dlg_identificationIOL *Dlg_IdentIOL = new dlg_identificationIOL(dlg_identificationIOL::Modification, iol, Q_NULLPTR, this);
    if (!Dlg_IdentIOL->initok())
        return;
    if (Dlg_IdentIOL->exec()>0)
    {
        if (iol)
        {
            int idiol = iol->id();
            m_listemodifiee = true;
            wdg_manufacturerscombo->disconnect();
            wdg_manufacturerscombo->setCurrentIndex(0);
            connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(false); } );
            ReconstruitTreeViewIOLs(true);
            iol = Datas::I()->iols->getById(idiol);
            if (iol)
                scrollToIOL(iol);
        }
    }
    delete Dlg_IdentIOL;
}

void dlg_listeiols::resizeiolimage(int size)
{
    QString szorigin, szfinal;
    QString m_pathdirstockageprovisoire = Procedures::I()->DefinitDossierImagerie() + NOM_DIR_PROV ;
    QStringList listfichresize = QDir(m_pathdirstockageprovisoire).entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (int t=0; t<listfichresize.size(); t++)
    {
        QString nomdocrz  = listfichresize.at(t);
        QString CheminFichierResize = m_pathdirstockageprovisoire + "/" + nomdocrz;
        QFile(CheminFichierResize).remove();
    }
    bool ok;
    QString req = "select " CP_ID_IOLS ", " CP_IMG_IOLS " from " TBL_IOLS
                " where " CP_IMG_IOLS " is not null and " CP_TYPIMG_IOLS " <> '" PDF "'";
    qDebug() << req;
    QList<QVariantList> listimg  = DataBase::I()->StandardSelectSQL(req, ok);
    for (int i=0; i<listimg.size(); ++i)
    {
        QString nomfichresize = m_pathdirstockageprovisoire + "/resize" + listimg.at(i).at(0).toString() + "." JPG;
        QByteArray ba = QByteArray();
        int id = listimg.at(i).at(0).toInt();
        ba = listimg.at(i).at(1).toByteArray();
        QFile file_image;
        QImage img;
        img.loadFromData(ba);
        double sz = ba.size();
        if (sz/(1024*1024) > 1)
            szorigin = QString::number(sz/(1024*1024),'f',1) + "Mo";
        else
            szorigin = QString::number(sz/1024,'f',1) + "Ko";
        szfinal = szorigin;
        file_image.setFileName(nomfichresize);
        int tauxcompress = 90;
        bool resized = false;
        if (sz > size)
        {
            resized = true;
            file_image.remove();
            QPixmap pixmap;
            pixmap = pixmap.fromImage(img.scaledToWidth(256,Qt::SmoothTransformation));
            tauxcompress = 90;
            while (sz > size && tauxcompress > 1)
            {
                pixmap.save(nomfichresize, "jpeg",tauxcompress);
                sz = file_image.size();
                if (tauxcompress > 19)
                    tauxcompress -= 10;
                else tauxcompress -= 1;
            }
            if (sz/(1024*1024) > 1)
                szfinal = QString::number(sz/(1024*1024),'f',0) + "Mo";
            else
                szfinal = QString::number(sz/1024,'f',0) + "Ko";
        }
        if (resized)
        {
            file_image.open(QIODevice::ReadOnly);
            ba = file_image.readAll();
            QHash<QString, QVariant> m_listbinds;
            m_listbinds[CP_IMG_IOLS] = ba;
            m_listbinds[CP_TYPIMG_IOLS] = JPG;
            DataBase::I()->UpDateIOL(id, m_listbinds);
            file_image.remove();
        }
    }
}


void dlg_listeiols::scrollToIOL(IOL *iol)
{
    if (iol != Q_NULLPTR)
    {
        for (int i=0; i < m_IOLsmodel->rowCount(); ++i)
        {
            UpStandardItem *itm = dynamic_cast<UpStandardItem *>(m_IOLsmodel->item(i));
            if (itm)
            {
                if (itm->hasChildren())
                {
                    for (int j=0; j < itm->rowCount(); ++j)
                    {
                        UpStandardItem *childitm = dynamic_cast<UpStandardItem *>(itm->child(j));
                        if (childitm)
                            if (childitm->item())
                            {
                                IOL *siol = dynamic_cast<IOL*>(childitm->item());
                                if (siol)
                                {
                                    if (siol->id() == iol->id())
                                    {
                                        wdg_iolstree->scrollTo(childitm->index(), QAbstractItemView::PositionAtCenter);
                                        wdg_iolstree->selectionModel()->select(childitm->index(),QItemSelectionModel::Rows | QItemSelectionModel::Select);
                                        j = itm->rowCount();
                                        i = m_IOLsmodel->rowCount();
                                    }
                                }
                            }
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------
// Supprime un IOL
// ------------------------------------------------------------------------------------------
void dlg_listeiols::SupprIOL(IOL *iol)
{
    if (!iol) return;
    QString Msg;
    Msg = tr("Etes vous sûr de vouloir supprimer l'implant") + "\n " +
            iol->modele() + "?" +
            "\n" + tr("La suppression est IRRÉVERSIBLE.");
    UpMessageBox msgbox;
    msgbox.setText("Euuhh... " + Datas::I()->users->userconnected()->login() + "?");
    msgbox.setInformativeText(Msg);
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton NoBouton(tr("Annuler"));
    UpSmallButton OKBouton(tr("Supprimer la fiche"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        Datas::I()->iols->SupprimeIOL(iol);
        m_listemodifiee = true;
        wdg_manufacturerscombo->disconnect();
        wdg_manufacturerscombo->setCurrentIndex(0);
        connect(wdg_manufacturerscombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ReconstruitTreeViewIOLs(false); } );
        ReconstruitTreeViewIOLs(true);
    }
}

void dlg_listeiols::ReconstruitListeManufacturers()
{
    if (m_manufacturersmodel == Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);

    QStringList list;
    UpStandardItem *manufactureritem;
    foreach(IOL *iol, Datas::I()->iols->iols()->values())
    {
        Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
        if (man != Q_NULLPTR)
        {
            QString fabricant  = man->nom();
            if (!list.contains(fabricant))
            {
                list << fabricant;
                manufactureritem  = new UpStandardItem(fabricant, man);
                manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                manufactureritem  ->setEditable(false);
                manufactureritem  ->setEnabled(false);
                m_manufacturersmodel->appendRow(manufactureritem);
            }
        }
    }
    m_manufacturersmodel->sort(0);
}

void dlg_listeiols::ReconstruitTreeViewIOLs(bool reconstruirelaliste, QString filtre)
{
    if (reconstruirelaliste)
    {
        Datas::I()->iols->initListe();
        ReconstruitListeManufacturers();
    }
    wdg_iolstree->disconnect();
    wdg_iolstree->selectionModel()->disconnect();
    if (m_IOLsmodel == Q_NULLPTR)
        delete m_IOLsmodel;
    m_IOLsmodel = new QStandardItemModel(this);
    int idman = wdg_manufacturerscombo->currentData().toInt();
    UpStandardItem *pitem;
    for (int i=0; i<  m_manufacturersmodel->rowCount(); ++i)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(m_manufacturersmodel->item(i));
        if (itm != Q_NULLPTR)
        {
            Manufacturer *man = dynamic_cast<Manufacturer*>(itm->item());
            if (man != Q_NULLPTR)
            {
                if (idman == 0)
                {
                    UpStandardItem *manufactureritem = new UpStandardItem(man->nom(), man);
                    manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                    manufactureritem  ->setEditable(false);
                    manufactureritem  ->setEnabled(false);
                    m_IOLsmodel->appendRow(manufactureritem);
                }
                else if (man->id() == idman)
                {
                    UpStandardItem *manufactureritem = new UpStandardItem(man->nom(), man);
                    manufactureritem  ->setForeground(QBrush(QColor(Qt::red)));
                    manufactureritem  ->setEditable(false);
                    manufactureritem  ->setEnabled(false);
                    m_IOLsmodel->appendRow(manufactureritem);
                }
            }
        }
        m_IOLsmodel->sort(0);
    }
    foreach(IOL *iol, Datas::I()->iols->iols()->values())
    {
        if (iol->modele().startsWith(filtre, Qt::CaseInsensitive))
        {
            pitem   = new UpStandardItem(iol->modele(), iol);
            if (!iol->isactif())
                pitem ->setForeground(QBrush(QColor(Qt::darkGray)));
            pitem   ->setEditable(false);
            Manufacturer *man = Datas::I()->manufacturers->getById(iol->idmanufacturer());
            if (man != Q_NULLPTR)
            {
                QString fabricant  = man->nom();
                QList<QStandardItem *> listitems = m_IOLsmodel->findItems(fabricant);
                if (listitems.size()>0)
                    listitems.at(0)->appendRow(pitem);
            }
        }
    }
    for (int i=0; i<m_IOLsmodel->rowCount();i++)
    {
        if (m_IOLsmodel->item(i) != Q_NULLPTR)
            if (!m_IOLsmodel->item(i)->hasChildren())
            {
                m_IOLsmodel->removeRow(i);
                i--;
            }
    }
    wdg_iolstree     ->setModel(m_IOLsmodel);
    wdg_iolstree     ->expandAll();
    if (m_IOLsmodel->rowCount()>0)
    {
        m_IOLsmodel->sort(0);
        m_IOLsmodel->sort(1);
        connect(wdg_iolstree,    &QAbstractItemView::entered,       this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                        {
                                                                                                            IOL*iol = getIOLFromIndex(idx);
                                                                                                            if (iol)
                                                                                                                QToolTip::showText(cursor().pos(), iol->tooltip());
                                                                                                        }
                                                                                                    } );
        connect(wdg_iolstree->selectionModel(),    &QItemSelectionModel::currentChanged,       this,   &dlg_listeiols::Enablebuttons);
        connect(wdg_iolstree,    &QAbstractItemView::doubleClicked, this,   [=] (QModelIndex idx) { if (!m_IOLsmodel->itemFromIndex(idx)->hasChildren())
                                                                                                            ModifIOL(getIOLFromIndex(idx)); });
    }
}
