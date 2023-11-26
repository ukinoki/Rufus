/*///////////////////////////////////////////////////////////////////////////////
// textprinter.h
// -------------------
// Copyright (c) 2007 David Johnson <david@usermode.org>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////
*/

#ifndef TEXTPRINTER_H
#define TEXTPRINTER_H

#include <QPrinter>
#include <QPrinterInfo>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextFrame>
#include <QPrintPreviewDialog>

#include <utils.h>
#include <QStandardPaths>

///////////////////////////////////////////////////////////////////////////////
/// \class TextPrinter
///
///
/// TextPrinter is a printing utility class. It provides methods to print,
/// preview, and export to PDF. The page format, including header and footers,
/// can be defined. The content to be printed is provided as a QTextDocument
/// object.
///////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The TextPrinter class
 * Textprinter êrmet d'imprimer un document à partir d'un QTextDocument passé en paramètre vers une impimante ou vers un fichier pdf. Le contenu du QTextDocument est exprimé en html
 * Le format de la page, les marges peuvent être définis, ainsi que l'entête et le pied de page
 * L'entête et le pied de page sont fournis par des QString en html passés en paramètre
 *
 * Pour imprimer un document
    * on crée un objet TextPrinter
    * on définit
        * les marges avec setLeftMagin, setTopMargin(), setRightMargin() et setBottomMargin() ou avec setMargins qui définit les 4 propriétés en meême temps.
        * la taille de la page, l'orientation - pas utilisés dans Rufus pour le moment
            * setPageSize(QPageSize size)
            * setOrientation(QPageLayout::Orientation orientation = QPageLayout::Portrait);
        * l'entête avec setHeaderText(const QString &text) où text est du texte en html
        * le pied de page avec setFooterText(const QString &text) où text est du texte en html
        * le corps du texte est fourni au moment de lancer l'impression, la conversion en pdf ou la prévisualisation avec
            * print(const QTextDocument *document,  QString ficpdf = QString(), const QString &caption = QString(), bool AvecChoixImprimante = true, bool QueLePdf = false)
            * exportPdf(const QTextDocument *document, const QString &caption=QString(), const QString &filename=QString());
            * preview(const QTextDocument *document, QString ficpdf = QString(), const QString &caption=QString());
        * d'autres fonctions moins importantes
            * setSpacing(double spacing) détermine l'espacement entre l'entête et le ccontenu et entre le contenu et le pied de page
            * setHeaderLinePenWidth(double pointsize) et setFooterLinePenWidth(double pointsize)
                * tirent un trait horizontal entre entête et contenu et contenu et pied de page.
                * on met pointsize à zero pour ne pas avoir de trait
            * setDuplex(const QPrinter::DuplexMode duplex) imprime rectoverso ou non
        * setUnits(const TextPrinter::Unit) fixe les unités de mesure: point, mm ou inch

    * les fonctions privées
        * paperRectDPI(QPaintDevice *device) renvoie la taille du papier exprimé en DPI
 */


class TextPrinter : public QObject
{
    Q_OBJECT

public:
    explicit                TextPrinter(QObject *parent = Q_NULLPTR);
    enum Unit {Point, Inch, Millimeter}; Q_ENUM(Unit)    /*! +++ pour les distraits un point n'est pas un pixel mais une dimension de 1/72 inch et ne varie donc pas avec la résolution de l'imprimante */
    ~TextPrinter();


    //added by Javier
    QByteArray              getPDFByteArray(const QTextDocument *document);

    bool                    print(const QTextDocument *document,  QString ficpdf = QString(), const QString &caption = QString(), bool AvecChoixImprimante = true, bool QueLePdf = false);   // Print the document
    void                    exportPdf(const QTextDocument *document, const QString &caption=QString(), const QString &filename=QString());                              // Export the document to PDF
    bool                    preview(const QTextDocument *document, QString ficpdf = QString(), const QString &caption=QString());                                       // Display the document in a preview dialog

    void                    setTopMargin(double margin);                                                        // Set top margin width

    void                    setHeaderSize(double size);                                                         // Set header size
    void                    setHeaderText(const QString &text);                                                 // Set header text

    double                  footerSize() const;                                                                 // Get footer size
    void                    setFooterSize(double size);                                                         // Set footer size
    void                    setFooterText(const QString &text);                                                 // Set footer text

    void                    setDuplex(const QPrinter::DuplexMode duplex);                                       // rectoverso
    void                    setPrinterName(QString printerName);
    QString                 getPrinterName();

    //added by Javier
    QPrinter::Unit          units() const;
    //added by Javier
    void                    setUnits(const Unit);

    /*! ----------- PAS UTILISÉS --------------------------- */
    QPageSize                pageSize() const;                                                                  // Get page size
    void                     setPageSize(QPageSize size);                                                       // Set page size
    QPageLayout::Orientation orientation() const;                                                               // Get page orientation
    void                     setOrientation(QPageLayout::Orientation orientation = QPageLayout::Portrait);      // Set page orientation
    double                  leftMargin() const;                                                                 // Get left margin width
    void                    setLeftMargin(double margin);                                                       // Set left margin width
    double                  rightMargin() const;                                                                // Get right margin width
    void                    setRightMargin(double margin);                                                      // Set right margin width
    double                  topMargin() const;                                                                  // Get top margin width
    double                  bottomMargin() const;                                                               // Get bottom margin width
    void                    setBottomMargin(double margin);                                                     // Set bottom margin width
    void                    setMargins(double margin);                                                          // Set all margins
    double                  spacing() const;                                                                    // Get spacing between content and header and footer
    void                    setSpacing(double spacing);                                                         // Set spacing between content and header and footer
    double                  headerSize() const;                                                                 // Get header size
    double                  headerLinePenWidth() const;                                                         // Set header line pen width
    void                    setHeaderLinePenWidth(double pointsize);                                            // Get header line pen width
    const QString           &headerText() const;                                                                // Get header text
    double                  footerLinePenWidth() const;                                                         // Get footer line pen width
    void                    setFooterLinePenWidth(double pointsize);                                            // Set footer line pen width
    const QString           &footerText() const;                                                                // Get footer text

private:
    TextPrinter(const TextPrinter&);
    TextPrinter             &operator=(const TextPrinter&);                                                     // not copyable

    QRectF                  paperRectDPI(QPaintDevice *device);                                                 // return paper rect in DPI resolution of the device


    void                    launchprint(QPrinter *printer = Q_NULLPTR);                                         // common print routine
    void                    paintPage(QPainter *painter, int pagenum, int nbpages);                             // paint specific page
    //added by Javier
    void                    QRectF2device(QRectF *rect, QPaintDevice *device);                                  // adjusts rect to device resolution ???
     //added by Javier
    qreal                   x2device(qreal x, QPaintDevice *device, QPrinter::Unit unit = QPrinter::Millimeter);// returns number of dots of device for dimension x given in unit
    //added by Javier
    qreal                   y2device(qreal y, QPaintDevice *device, QPrinter::Unit unit = QPrinter::Millimeter);// returns number of dots of device for dimension y given in unit
    //added by Javier
    void                    printToDevice(QPagedPaintDevice *device);

    qreal                   adjustedheaderheight_ = 0.0;                                                        // calculated header height in DPI, depending on headertext_ and QpaintDevice used
    qreal                   adjustedheaderheight(QPainter *painter);
    QRectF                  adjustedHeaderRect(QPainter *painter);
    QRectF                  footerRect(QPaintDevice *device);
    QRectF                  adjustedContentRect(QPainter *painter);

    QWidget                 *parent_        = Q_NULLPTR;
    QPrinter                *printer_       = new QPrinter(QPrinter::HighResolution);
    QTextDocument           *tempdoc_       = Q_NULLPTR;

    // all margins in units_ (default = millimeter)
    double                  leftmargin_     = 10;
    double                  rightmargin_    = 10;
    double                  topmargin_      = 10;
    double                  bottommargin_   = 10;
    double                  spacing_        = 3;

    // footer and header size in units_ (default = millimeter)
    double                  headersize_     = 0.0;
    double                  footersize_     = 0.0;

    // header and footer line width in points (= 1/72 inch)
    double                  headerlinepenwidth_ = 0.5;                                                                // pen width in points of the line separating header from content
    double                  footerlinepenwidth_ = 0.5;                                                                // pen width in points of the line separating footer from content

    QString                 headertext_     = QString();
    QString                 footertext_     = QString();

    QPrinter::DuplexMode    duplex_         = QPrinter::DuplexAuto;

    // enum unit allows to limit printing units to millimeter, inch or points
    enum Unit               textprinterunits_ = Millimeter;

    //added by Javier
    QPrinter::Unit          units_;
    //added by Javier
    double                  toinchfactor_;
    void                    PrintPageList(QPrinter *Imprimante, QList<QImage> pagelist);
};

#endif // TEXTPRINTER_H
