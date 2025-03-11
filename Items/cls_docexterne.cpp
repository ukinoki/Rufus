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


#include "cls_docexterne.h"
#include <QDebug>

int DocExterne::cote() const
{
    return m_cote;
}

void DocExterne::setCote(int newCote)
{
    m_cote = newCote;
}

DocExterne::DocExterne(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
    m_data = data;
}

DocExterne::DocExterne(QString filepath, bool &initOK, QWidget *parent)
{
    QFile       qFile(filepath);
    if (!qFile.open( QIODevice::ReadOnly ))
    {
        UtilsMessageBox::Watch(parent,  tr("Erreur d'accès au fichier"), qFile.fileName());
        initOK = false;
        return;
    }
    QByteArray bapdf = qFile.readAll();
    QString suffixe = QFileInfo(qFile).suffix().toLower();
    qFile.close ();
    setimageformat(suffixe);
    setimageblob(bapdf);
    setsoustype(QFileInfo(filepath).fileName());
    initOK = true;
}

bool DocExterne::isAllLoaded() const                { return m_isAllLoaded;}

int DocExterne::iduser() const                      { return m_iduser;}
int DocExterne::idpatient() const                   { return m_idpatient;}
QString DocExterne::typedoc() const                 { return m_typedoc;}
QString DocExterne::soustypedoc() const             { return m_soustypedoc;}
QString DocExterne::titre() const                   { return m_titre;}

QString DocExterne::textentete() const              { return m_textentete;}
QString DocExterne::textcorps() const               { return m_textcorps;}
QString DocExterne::textorigine() const             { return m_textorigine;}
QString DocExterne::textpied() const                { return m_textpied;}
QDateTime DocExterne::datetimeimpression() const    { return m_dateimpression;}

int DocExterne::compression() const                 { return m_compression;}
QString DocExterne::lienversfichier() const         { return m_lienversfichier;}
bool DocExterne::isALD() const                      { return m_ald == 1;}
int DocExterne::useremetteur() const                { return m_useremetteur;}
QString DocExterne::format() const                  { return m_formatdoc;}
QByteArray DocExterne::imageblob() const            { return m_blob;}
QString DocExterne::imageformat() const             { return m_formatimage;}
QList<QImage> DocExterne::pagelist() const          { return m_imagelist;}
int DocExterne::importance() const                  { return m_importance;}

int DocExterne::idrefraction() const                { return m_idrefraction; }

void DocExterne::setdate(QDateTime date)            { m_dateimpression = date;
                                                      m_data[CP_DATE_DEPENSES] = date.toString("yyyy-MM-dd"); }
void DocExterne::setimportance(int imptce)          { m_importance = imptce;
                                                      m_data[CP_IMPORTANCE_DOCSEXTERNES] = imptce; }
void DocExterne::setAllLoaded(bool AllLoaded)       { m_isAllLoaded = AllLoaded;
                                                      m_data[CP_ISALLLOADED] = AllLoaded; }
void DocExterne::setimageformat(QString format)     { m_formatimage = format.toLower(); }

void DocExterne::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataBool(data, CP_ISALLLOADED, m_isAllLoaded);

    setDataInt(data, CP_ID_DOCSEXTERNES, m_id);
    setDataInt(data, CP_IDUSER_DOCSEXTERNES, m_iduser);
    setDataInt(data, CP_IDPAT_DOCSEXTERNES, m_idpatient);
    setDataString(data, CP_TYPEDOC_DOCSEXTERNES, m_typedoc);
    setDataString(data, CP_SOUSTYPEDOC_DOCSEXTERNES, m_soustypedoc);

    setDataString(data, CP_TITRE_DOCSEXTERNES, m_titre);
    setDataString(data, CP_TEXTENTETE_DOCSEXTERNES, m_textentete);
    setDataString(data, CP_TEXTCORPS_DOCSEXTERNES, m_textcorps);
    setDataString(data, CP_TEXTORIGINE_DOCSEXTERNES, m_textorigine);
    setDataByteArray(data, CP_PDFORIGIN_DOCSEXTERNES, m_pdforigin);
    setDataString(data, CP_TEXTPIED_DOCSEXTERNES, m_textpied);

    setDataDateTime(data, CP_DATE_DOCSEXTERNES, m_dateimpression);
    setDataInt(data, CP_COMPRESSION_DOCSEXTERNES, m_compression);
    setDataString(data, CP_LIENFICHIER_DOCSEXTERNES, m_lienversfichier);
    setDataInt(data, CP_ALD_DOCSEXTERNES, m_ald);
    setDataInt(data, CP_IDEMETTEUR_DOCSEXTERNES, m_useremetteur);

    setDataString(data, CP_FORMATDOC_DOCSEXTERNES, m_formatdoc);
    setDataInt(data, CP_IMPORTANCE_DOCSEXTERNES, m_importance);
    setDataInt(data, CP_EMISORRECU_DOCSEXTERNES, m_emisrecu);
    setDataInt(data, CP_IDLIEU_DOCSEXTERNES, m_idsite);
    setDataInt(data, CP_IDREFRACTION_DOCSEXTERNES, m_idrefraction);
    setDataInt(data, CP_COTE_DOCSEXTERNES, m_cote);
    m_data = data;
}


// Called from dlg_docsexternes::CalcImageDocument
void DocExterne::setimageblob(QByteArray blob)
{
    m_blob = blob;

    // Render pages to image list
    m_imagelist.clear();

    if( isPDF() )
    {
        m_imagelist = Utils::calcImagefromPdf(m_blob);
    }
    else
    {
        QImage image = QImage::fromData(m_blob);
        m_imagelist << image;
    }
}

