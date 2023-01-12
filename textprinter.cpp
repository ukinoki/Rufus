///////////////////////////////////////////////////////////////////////////////
// textprinter.h
// -------------------
// Copyright (c) 2007 David Johnson <david@usermode.org>
// Please see the header file for copyright and license information.
///////////////////////////////////////////////////////////////////////////////


#include "textprinter.h"
#include <QPdfWriter>



TextPrinter::TextPrinter(QObject *parent)
    : QObject(parent), parent_(Q_NULLPTR),
                        printer_(new QPrinter(QPrinter::HighResolution)),
                        tempdoc_(Q_NULLPTR),
                        leftmargin_(10), rightmargin_(10), topmargin_(10), bottommargin_(10), spacing_(5),
                        headersize_(0.0), headerrule_(1), headertext_(QString()), footersize_(0.0), footerrule_(1), footertext_(QString()),
                        dateformat_(), duplex_(QPrinter::DuplexAuto), units_(QPrinter::Millimeter)
{
    if (parent)
        parent_ = qobject_cast<QWidget*>(parent);

    printer_->setFullPage(true);
    printer_->setPageOrientation(QPageLayout::Portrait);

    // for convenience, default to US_Letter for C/US/Canada
    // NOTE: bug in Qt, this value is not loaded by QPrintDialog
    switch (QLocale::system().country()) {
      //case QLocale::AnyCountry:
      case QLocale::Canada:
      case QLocale::UnitedStates:
      case QLocale::UnitedStatesMinorOutlyingIslands:
          printer_->setPageSize(QPageSize::Letter); break;
      default:
          printer_->setPageSize(QPageSize::A4); break;
    }
    setUnits(units_);
}

TextPrinter::~TextPrinter()
{
    delete printer_;
}

QPageSize TextPrinter::pageSize() const
{
    return printer_->pageLayout().pageSize();
}

void TextPrinter::setPageSize(QPageSize size)
{
    printer_->setPageSize(size);
}

QPageLayout::Orientation TextPrinter::orientation() const
{
    return printer_->pageLayout().orientation();
}

void TextPrinter::setOrientation(QPageLayout::Orientation orientation)
{
    // Valeur par defaut = QPrinter::Portrait
    QPageLayout lay = printer_->pageLayout();
    lay.setOrientation(orientation);
}

void TextPrinter::setLeftMargin(double margin)
{
    // Valeur par defaut = 10mm
    if ((margin > 0) && (margin < printer_->paperRect(units()).width() / 2))
        leftmargin_ = margin;
    else
        leftmargin_ = 0;
}


double TextPrinter::rightMargin() const
{
    return rightmargin_;
}

void TextPrinter::setRightMargin(double margin)
{
    // Valeur par defaut = 10mm
    if ((margin > 0) && (margin < printer_->paperRect(units()).width() / 2))
        rightmargin_ = margin;
    else
        rightmargin_ = 0;
}

double TextPrinter::topMargin() const
{
    return topmargin_;
}

void TextPrinter::setTopMargin(double margin)
{
    // Valeur par defaut = 15mm
    if ((margin > 0) && (margin < printer_->paperRect(units()).height() / 4))
        topmargin_ = margin;
    else
        topmargin_ = 0;
}

double TextPrinter::bottomMargin() const
{
    return bottommargin_;
}

void TextPrinter::setBottomMargin(double margin)
{
    // Valeur par defaut = 15mm
    if ((margin > 0) && (margin < printer_->paperRect(units()).height() / 4))
        bottommargin_ = margin;
    else
        bottommargin_ = 0;
}

void TextPrinter::setMargins(double margin)
{
    if ((margin > 0)
            && (margin < printer_->paperRect(units()).height() / 2)
            && (margin < printer_->paperRect(units()).width() / 2))
        leftmargin_ = rightmargin_ = topmargin_ = bottommargin_ = margin;
    else
        leftmargin_ = rightmargin_ = topmargin_ = bottommargin_ = 0;
}

double TextPrinter::spacing() const
{
    return spacing_;
}

void TextPrinter::setSpacing(double spacing)
{
    // Espace entre le contenu de la page et les blocs d'en-tête et de pied
    // Valeur par defaut = 5mm
    if ((spacing > 0) && (spacing <= printer_->paperRect(units()).height() / 4))
        spacing_ = spacing;
    else
        spacing_ = 0;
}

double TextPrinter::headerSize() const
{
    return headersize_;
}

void TextPrinter::setHeaderSize(double size)
{
    // Valeur par defaut = 0mm (pas d'en-tête
    if ((size > 0) && (size <= printer_->paperRect(units()).height() / 4))
        headersize_ = size;
    else
        headersize_ = 0;
}

double TextPrinter::headerRule() const
{
    return headerrule_;
}

void TextPrinter::setHeaderRule(double pointsize)
{
    // Valeur par defaut = 0.5 pt (1/144 inch)
    headerrule_ = qMax(0.0, pointsize);
}

const QString &TextPrinter::headerText() const
{
    return headertext_;
}

void TextPrinter::setHeaderText(const QString &text)
{
    headertext_ = text;
}

double TextPrinter::footerSize() const
{
    return footersize_;
}

void TextPrinter::setFooterSize(double size)
{
    // Valeur par defaut = 0 mm (pas de pied de page)
    if ((size > 0) && (size <= printer_->paperRect(units()).height() / 4))
        footersize_ = size;
    else
        footersize_ = 0;
}

double TextPrinter::footerRule() const
{
    return footerrule_;
}

void TextPrinter::setFooterRule(double pointsize)
{
    // Valeur par defaut = 0.5 pt (1/144 inch)
    footerrule_ = qMax(0.0, pointsize);
}

const QString &TextPrinter::footerText() const
{
    return footertext_;
}

void TextPrinter::setFooterText(const QString &text)
{
    footertext_ = text;
}

const QString &TextPrinter::dateFormat() const
{
    return dateformat_;
}

void TextPrinter::setDateFormat(const QString &format)
{
    dateformat_ = format;
}

void TextPrinter::setDuplex(const QPrinter::DuplexMode duplex)
{
    duplex_ = duplex;
}


QPrinter::Unit TextPrinter::units() const
{
    return units_;
}

void TextPrinter::setUnits(const QPrinter::Unit value)
{
    units_=value;
    switch (value)
    {
        case QPrinter::Point:
            toinchfactor_= 1/72;
            break;
        case QPrinter::Inch:
            toinchfactor_= 1;
            break;
        default:
            units_=QPrinter::Millimeter;
            toinchfactor_= 0.039370147;
            break;
    }
}

bool TextPrinter::print(const QTextDocument *document, QString ficpdf, const QString &caption, bool AvecChoixImprimante, bool QueLePdf)
{
    if (!document) return false;
    if (!QueLePdf)
    {
        if (QPrinterInfo::availablePrinterNames().size() == 0)
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Aucune imprimante n'est\nconfigurée sur cet ordinateur!"));
            return false;
        }

        // imprime le document
        // show print dialog
        if (AvecChoixImprimante || printer_->printerName() == "")
        {
            if (QPrinterInfo::availablePrinterNames().size() > 1)
            {
                QPrintDialog dialog(printer_, parent_);
                dialog.setWindowTitle(caption.isEmpty() ? tr("Impression") : caption);
                if (dialog.exec() == QDialog::Rejected)
                    return false;
                else printer_= dialog.printer();
            }
            else if (!QPrinterInfo::defaultPrinter().isNull())
                printer_ = new QPrinter(QPrinterInfo::defaultPrinter());
            else
                printer_ = new QPrinter(QPrinterInfo::availablePrinters().at(0));
        }
        printer_->setOutputFormat(QPrinter::NativeFormat);
        printer_->setOutputFileName(QString());
        // print it
        tempdoc_ = document->clone();
        launchprint();
    }
    // enregistre le pdf
    if (ficpdf != "")
    {
        QString printerName = printer_->printerName();              // nécessaire parce que l'impression du pdf réinitialise le nom de l'imprimante
        printer_->setOutputFormat(QPrinter::PdfFormat);
        printer_->setOutputFileName(ficpdf);
        tempdoc_ = document->clone();
        launchprint();
        printer_->setOutputFormat(QPrinter::NativeFormat);
        printer_->setPrinterName(printerName);                      // nécessaire parce que l'impression du pdf réinitialise le nom de l'imprimante
    }
    delete tempdoc_;
    tempdoc_ = Q_NULLPTR;
    return true;
}

void TextPrinter::exportPdf(const QTextDocument *document, const QString &caption, const QString &filename)
{
    if (!document) return;

    // file save dialog
    QString dialogcaption = caption.isEmpty() ? "Export PDF" : caption;
    QString exportname;
    if (QFile::exists(filename))
        exportname = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at((0)) + "/" + filename;
    else
        exportname = QFileDialog::getSaveFileName(parent_, dialogcaption, filename, "*.pdf");
    if (exportname.isEmpty()) return;
    if (QFileInfo(exportname).suffix().isEmpty())
        exportname.append(".pdf");

    // setup printer
    printer_->setOutputFormat(QPrinter::PdfFormat);
    printer_->setOutputFileName(exportname);

    // print it
    tempdoc_ = document->clone();
    launchprint();

    delete tempdoc_;
    tempdoc_ = Q_NULLPTR;
}

bool TextPrinter::preview(const QTextDocument *document, QString ficpdf, const QString &caption)
{
    if (!document) return false;

    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer_, parent_);
    dialog->setWindowTitle(caption.isEmpty() ? "Print Preview" : caption);
    dialog->setWindowModality(Qt::WindowModal);
    connect(dialog, &QPrintPreviewDialog::paintRequested, this, &TextPrinter::launchprint);

    // preview it
    tempdoc_ = document->clone();
    QString printerName;
    int b = dialog->exec();
    if (b>0)
    {
        printer_= dialog->printer();
        printerName = printer_->printerName();
        // enregistre le pdf
        if (ficpdf != "")
        {
            printer_->setOutputFormat(QPrinter::PdfFormat);
            printer_->setOutputFileName(ficpdf);
            tempdoc_ = document->clone();
            launchprint();
            printer_->setOutputFormat(QPrinter::NativeFormat);
            printer_->setPrinterName(printerName); // nécessaire parce que l'impression du pdf réinitialise le nom de l'imprimante
        }
    }
    delete tempdoc_;
    tempdoc_ = Q_NULLPTR;
    delete dialog;
    return (b>0);
}

///////////////////////////////////////////////////////////////////////////////
// private methods
///////////////////////////////////////////////////////////////////////////////

void TextPrinter::setPrinterName(QString printerName)
{
    if (printerName != "") printer_->setPrinterName(printerName);
}

QString TextPrinter::getPrinterName()
{
    return printer_->printerName();
}


void TextPrinter::QRectF2device(QRectF *rect, QPaintDevice *device)
{
    int factorX = device->logicalDpiX() * toinchfactor_;
    int factorY = device->logicalDpiY() * toinchfactor_;

    rect->setRect(rect->x() * factorX,
                  rect->y() * factorY,
                  rect->width() * factorX,
                  rect->height() * factorY );
}

qreal TextPrinter::x2device(qreal x, QPaintDevice *device)
{
    /*! device horizontal resolution by units_ */
    int factorX = device->logicalDpiX() * toinchfactor_;
    /*! number of dots in device resolution of x */
    return x * factorX;
}

qreal TextPrinter::y2device(qreal y, QPaintDevice *device)
{
    /*! device vertical resolution by units_ */
    int factorY = device->logicalDpiY() * toinchfactor_;
    /*! number of dots in device resolution of y */
    return y * factorY;
}


/*!
 * \brief TextPrinter::paperRect
 * \param device
 * \return the printer paperRect() adjusted for units(mm)
 */
QRectF TextPrinter::paperRect()
{
    QRectF rect = printer_->paperRect(units());
    return rect;
}


// contentRect() //////////////////////////////////////////////////////////////
// calculate the rect for the content block

QRectF TextPrinter::contentRect(QPaintDevice *device)
{
    /*!  calculate size of content (paper - margins - header - footer) */
    QRectF rect = paperRect();

    /*! camculate size in units_ (default = mm) */

    /*! adjust without margins */
    rect.adjust(leftmargin_,
                topmargin_,
                -rightmargin_,
                -bottommargin_);

    /*! adjust without headersize and spacing to the top*/
    if (headersize_ > 0) {
        rect.adjust(0, headersize_, 0, 0);
        rect.adjust(0, spacing_, 0, 0);
    }

    /*! adjust without footersize and spacing to the bottom */
    if (footersize_ > 0) {
        rect.adjust(0, 0, 0, -footersize_);
        rect.adjust(0, 0, 0, -spacing_);
    }

    /*! convert to printer resolution */
    QRectF2device(&rect, device);

    return rect;
}


// headerRect() //////////////////////////////////////////////////////////////
// calculate the rect for the header block

QRectF TextPrinter::headerRect(QPaintDevice *device)
{
    /*!  calculate size of header (paper - margins) */
    QRectF rect = paperRect();
    rect.adjust(leftmargin_,
                topmargin_,
                -rightmargin_, 0);

    rect.setBottom(rect.top() + headersize_);

    /*! convert to printer resolution */
    QRectF2device(&rect, device);

    return rect;
}

// footerRect() ///////////////////////////////////////////////////////////////
// calculate the rect for the footer block

QRectF TextPrinter::footerRect(QPaintDevice *device)
{
    /*!  calculate size of footer (paper - margins) */
    QRectF rect = paperRect();
    rect.adjust(leftmargin_, 0,-rightmargin_,-bottommargin_);

    rect.setTop(rect.bottom() -footersize_);

    /*! convert to printer resolution */
    QRectF2device(&rect, device);

    return rect;
}



QByteArray TextPrinter::getPDFByteArray(const QTextDocument *document)
{
    QByteArray bapdf;

    if (document)
    {
        QBuffer buf;
        //QPdfWriter writer(&buf);
        buf.open(QIODeviceBase::WriteOnly);
        QPdfWriter writer(&buf);
        tempdoc_ = document->clone();

        QMarginsF pgMargins = printer_->pageLayout().margins();
        QPageSize pgSize = printer_->pageLayout().pageSize();

        writer.setPageSize(pgSize);
        writer.setPageMargins(pgMargins);
        printToDevice(&writer,1,1,0,0,QPrinter::FirstPageFirst);

        bapdf=buf.data();

    }
    return bapdf;
}

void TextPrinter::printToDevice(QPagedPaintDevice *device,int doccopies,int pagecopies,int firstpage,int lastpage,QPrinter::PageOrder pageorder)
{
    if (!device || !tempdoc_) return;

    tempdoc_->setUseDesignMetrics(true);
    tempdoc_->documentLayout()->setPaintDevice(device);
    QSizeF sizeCr=contentRect(device).size();
    tempdoc_->setPageSize(sizeCr);
    // dump existing margin (if any)
    QTextFrameFormat fmt = tempdoc_->rootFrame()->frameFormat();
    fmt.setMargin(0);
    tempdoc_->rootFrame()->setFrameFormat(fmt);

    // to iterate through pages we have to worry about
    // copies, collation, page range, and print order


    // get page range
    if (firstpage == 0 && lastpage == 0) { // all pages
        firstpage = 1;
        lastpage = tempdoc_->pageCount();
    }

    // print order
    int delta = 1;
    if (pageorder == QPrinter::LastPageFirst) {
        int tmp = firstpage;
        firstpage = lastpage;
        lastpage = tmp;
        delta=-1;
    }

    // loop through and print pages
    QPainter painter(device);
    painter.setRenderHints(QPainter::Antialiasing |
                           QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform, true);
    for (int dc=0; dc<doccopies; dc++) {
        int pagenum = firstpage;
        while(true)
        {
            for (int pc=0; pc<pagecopies; pc++)
            {
                // print page
                paintPage(&painter, pagenum, lastpage);
                if (pc < pagecopies-1) device->newPage();
            }
            if (pagenum == lastpage) break;
            pagenum+=delta;
            device->newPage();
        }
        if (dc < doccopies-1) device->newPage();
    }
}

/*! * \brief TextPrinter::launchprint
 *  Common printing routine. Print tempdoc_ to printer_
 *  Imprime la page à partir du QtextDocument tempdoc_
*/
void TextPrinter::launchprint()
{
    if (!printer_ || !tempdoc_) return;
    if (duplex_ == QPrinter::DuplexLongSide)
        printer_->setDuplex(duplex_);

    tempdoc_->setUseDesignMetrics(true);
    tempdoc_->documentLayout()->setPaintDevice(printer_);
    QSizeF sizeCr=contentRect(printer_).size();
    tempdoc_->setPageSize(sizeCr);
    // dump existing margin (if any)
    QTextFrameFormat fmt = tempdoc_->rootFrame()->frameFormat();
    fmt.setMargin(0);
    tempdoc_->rootFrame()->setFrameFormat(fmt);

    // to iterate through pages we have to worry about
    // copies, collation, page range, and print order

    // get num copies
    int doccopies;
    int pagecopies;
    if (printer_->collateCopies()) {
        doccopies = 1;
        pagecopies = printer_->copyCount();
    } else {
        doccopies = printer_->copyCount();
        pagecopies = 1;
    }

    // get page range
    int firstpage = printer_->fromPage();
    int lastpage = printer_->toPage();
    if (firstpage == 0 && lastpage == 0) { // all pages
        firstpage = 1;
        lastpage = tempdoc_->pageCount();
    }

    // print order
    int delta = 1;
    if (printer_->pageOrder() == QPrinter::LastPageFirst) {
        int tmp = firstpage;
        firstpage = lastpage;
        lastpage = tmp;
        delta=-1;
    }

    // loop through and print pages
    QPainter painter(printer_);
    painter.setRenderHints(QPainter::Antialiasing |
                           QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform, true);
    for (int dc=0; dc<doccopies; dc++) {
        int pagenum = firstpage;
        while(true)
        {
            for (int pc=0; pc<pagecopies; pc++)
            {
                if (printer_->printerState() == QPrinter::Aborted || printer_->printerState() == QPrinter::Error)
                    return;
                // print page
                paintPage(&painter, pagenum, lastpage);
                if (pc < pagecopies-1) printer_->newPage();
            }
            if (pagenum == lastpage) break;
            pagenum+=delta;
            printer_->newPage();
        }
        if (dc < doccopies-1) printer_->newPage();
    }
}


/*!
 * \brief TextPrinter::paintPage
 * paint an individual page of the document to the painter (QPdfWriter or QPrinter)
 * dessine la page avec le Qpainter passé en paramètre (QPdfWriter ou QPrinter)
 * \param QPainter painter -> QPdfWriter or QPrinter
 * \param int pagenum -> current page number
 * \param int nbpages -> total number of pages
 */
void TextPrinter::paintPage(QPainter *painter, int pagenum, int nbpages)
{
    /*!
     *  Note the difference between Point and DevicePixel.
     *  The Point unit is defined to be 1/72th of an inch, while the DevicePixel unit is resolution dependent and is based on the actual pixels, or dots, on the printer.
     */

    QRectF rect;
    double onepoint = painter->device()->logicalDpiY() / 72.0;

/*! 1 - header ----------------------------------------------------------------------------------------------------------------------------------- */
    if (headersize_ > 0) {
        rect = headerRect(painter->device());
        if (headerrule_ > 0.0) {
            painter->save();
            // allow space between rule and header
            qreal hr = y2device(headerrule_/72, painter->device());
            painter->setPen(QPen(Qt::black, hr * onepoint));                // choisit le format du pinceau
            painter->drawLine(rect.bottomLeft(), rect.bottomRight());       // trace une ligne depuis le bas à gauche au bas à droite de l'en-tête
            painter->restore();
        }

        // replace page variables
        QString header = headertext_;
        header.replace("&page;", QString::number(pagenum) + "/" + QString::number(nbpages));
        if (dateformat_.isEmpty())
            header.replace("&date;", QDate::currentDate().toString());
        else
            header.replace("&date;", QDate::currentDate().toString(dateformat_));

        painter->save();
        painter->translate(rect.left(), rect.top());
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(header);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());

        QRectF clip(0, 0, rect.width(), rect.height());
        clip.setHeight(doc.size().height());
        doc.drawContents(painter, clip);
        painter->restore();
    }

/*! 2 - footer ----------------------------------------------------------------------------------------------------------------------------------- */
    if (footersize_ > 0) {
        rect = footerRect(painter->device());
        if (footerrule_ > 0.0) {
            painter->save();
            // allow space between rule and footer
            qreal fr = x2device( footerrule_/72, painter->device());
            //painter->translate(0, -onepoint + (-fr * onepoint / 2.0));
            painter->setPen(QPen(Qt::black, fr * onepoint));
            painter->drawLine(rect.topLeft(), rect.topRight());
            painter->restore();
        }

        // replace page variables
        QString footer = footertext_;
        footer.replace("&page;", QString::number(pagenum) + "/" + QString::number(nbpages));
        if (dateformat_.isEmpty())
            footer.replace("&date;", QDate::currentDate().toString());
        else
            footer.replace("&date;", QDate::currentDate().toString(dateformat_));

        painter->save();
        qreal spaceheight(35);
        painter->translate(rect.left(), rect.top()+spaceheight);
        QRectF clip(0, 0, rect.width(), rect.height()-spaceheight);
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(footer);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());
        doc.drawContents(painter, clip);
        painter->restore();
    }

 /*! 3 - content ----------------------------------------------------------------------------------------------------------------------------------- */
    painter->save();
    rect = contentRect(painter->device());
    painter->translate(rect.left(), rect.top() - (pagenum-1) * rect.height());
    QRectF clip(0, (pagenum-1) * rect.height(), rect.width(), rect.height());
    tempdoc_->drawContents(painter, clip);
    painter->restore();
}
