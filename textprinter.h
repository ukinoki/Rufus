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
#include <procedures.h>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextFrame>

#include <QPrintPreviewDialog>

///////////////////////////////////////////////////////////////////////////////
/// \class TextPrinter
/// \version 0.3
///
/// TextPrinter is a printing utility class. It provides methods to print,
/// preview, and export to PDF. The page format, including header and footers,
/// can be defined. The content to be printed is provided as a QTextDocument
/// object.
///////////////////////////////////////////////////////////////////////////////

class TextPrinter : public QObject
{
    Q_OBJECT

public:
    explicit                TextPrinter(QObject *parent = Q_NULLPTR);
    ~TextPrinter();

    bool                    print(const QTextDocument *document,  QString ficpdf = QString(), const QString &caption = QString(), bool AvecChoixImprimante = true, bool QueLePdf = false);   // Print the document
    void                    exportPdf(const QTextDocument *document, const QString &caption=QString(), const QString &filename=QString());                              // Export the document to PDF
    bool                    preview(const QTextDocument *document, QString ficpdf = QString(), const QString &caption=QString());                                       // Display the document in a preview dialog

    QPrinter::PageSize      pageSize() const;                                                                   // Get page size
    void                    setPageSize(QPrinter::PageSize size);                                               // Set page size
    QPrinter::Orientation   orientation() const;                                                                // Get page orientation
    void                    setOrientation(QPrinter::Orientation orientation);                                  // Set page orientation


    double                  leftMargin() const;                                                                 // Get left margin width
    void                    setLeftMargin(double margin);                                                       // Set left margin width
    double                  rightMargin() const;                                                                // Get right margin width
    void                    setRightMargin(double margin);                                                      // Set right margin width
    double                  topMargin() const;                                                                  // Get top margin width
    void                    setTopMargin(double margin);                                                        // Set top margin width
    double                  bottomMargin() const;                                                               // Get bottom margin width
    void                    setBottomMargin(double margin);                                                     // Set bottom margin width
    void                    setMargins(double margin);                                                          // Set all margins

    double                  spacing() const;                                                                    // Get spacing between content and header and footer
    void                    setSpacing(double spacing);                                                         // Set spacing between content and header and footer

    double                  headerSize() const;                                                                 // Get header size
    void                    setHeaderSize(double size);                                                         // Set header size
    double                  headerLinePenWidth() const;                                                         // Set header line pen width
    void                    setHeaderLinePenWidth(double pointsize);                                            // Get header line pen width
    const QString           &headerText() const;                                                                // Get header text
    void                    setHeaderText(const QString &text);                                                 // Set header text

    double                  footerSize() const;                                                                 // Get footer size
    void                    setFooterSize(double size);                                                         // Set footer size
    double                  footerLinePenWidth() const;                                                         // Get footer line pen with
    void                    setFooterLinePenWidth(double pointsize);                                            // Set footer line pen width
    const QString           &footerText() const;                                                                // Get footer text
    void                    setFooterText(const QString &text);                                                 // Set footer text

    void                    setDuplex(const QPrinter::DuplexMode duplex);                                       // rectoverso
    void                    setPrinterName(QString printerName);
    QString                 getPrinterName();

private:
    TextPrinter(const TextPrinter&);
    TextPrinter             &operator=(const TextPrinter&);                                                     // not copyable

    QRectF                  paperRectDPI(QPaintDevice *device);                                                 // return paper rect

    QRectF                  contentRect(QPaintDevice *device);                                                  // return printable rects
    QRectF                  headerRect(QPaintDevice *device);
    QRectF                  footerRect(QPaintDevice *device);

    qreal                   adjustedheaderheight_ = 0.0;                                                        // calculated header height, depending on headertext_
    qreal                   adjustedheaderheight(QPainter *painter);
    QRectF                  adjustedHeaderRect(QPainter *painter);
    QRectF                  adjustedContentRect(QPainter *painter);

    void                    launchprint(QPrinter *printer = Q_NULLPTR);                                                                      // common print routine
    void                    paintPage(QPainter *painter, QTextDocument *document, int pagenum, int nbpages);    // paint specific page

    QWidget                 *parent_;
    QPrinter                *printer_;

    QTextDocument           *tempdoc_;

    double                  leftmargin_;
    double                  rightmargin_;
    double                  topmargin_;
    double                  bottommargin_;
    double                  spacing_;

    double                  headersize_;
    double                  headerlinepenwidth_;
    QString                 headertext_;
    double                  footersize_;
    double                  footerlinepenwidth_;
    QString                 footertext_;

    QPrinter::DuplexMode    duplex_;
};

#endif // TEXTPRINTER_H
