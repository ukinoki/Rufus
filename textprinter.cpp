///////////////////////////////////////////////////////////////////////////////
// textprinter.h
// -------------------
// Copyright (c) 2007 David Johnson <david@usermode.org>
// Please see the header file for copyright and license information.
///////////////////////////////////////////////////////////////////////////////


#include "textprinter.h"
#include <QPdfWriter>

TextPrinter::TextPrinter(QObject *parent) : QObject(parent), parent_(Q_NULLPTR)
{
    if (parent)
        parent_ = qobject_cast<QWidget*>(parent);

    printer_->setFullPage(true);
    printer_->setPageOrientation(QPageLayout::Portrait);

    switch (QLocale::system().territory()) {
      case QLocale::Canada:
      case QLocale::UnitedStates:
      case QLocale::UnitedStatesMinorOutlyingIslands:
          printer_->setPageSize(QPageSize::Letter); break;
      default:
          printer_->setPageSize(QPageSize::A4); break;
    }
    setUnits(textprinterunits_);
}

TextPrinter::~TextPrinter() { delete printer_; }

void TextPrinter::setTopMargin(double margin)
{
    if ((margin > 0) && (margin < printer_->paperRect(units_).height() / 4))
        topmargin_ = margin;
    else
        topmargin_ = 0;
}

void TextPrinter::setHeaderSize(double size)
{
    // Valeur par defaut = 0mm (pas d'en-tête)
    if ((size > 0) && (size <= printer_->paperRect(units_).height() / 4))
        headersize_ = size;
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
    if ((size > 0) && (size <= printer_->paperRect(units_).height() / 4))
        footersize_ = size;
}

void TextPrinter::setFooterText(const QString &text)
{
    footertext_ = text;
}

void TextPrinter::setDuplex(const QPrinter::DuplexMode duplex)
{
    duplex_ = duplex;
}

void TextPrinter::setUnits(const Unit value)
{
    switch (value)
    {
    case TextPrinter::Point:
        units_ = QPrinter::Point;
        toinchfactor_= 1/72;
        break;
    case TextPrinter::Inch:
        units_ = QPrinter::Inch;
        toinchfactor_= 1;
        break;
    case TextPrinter::Millimeter:
        units_ = QPrinter::Millimeter;
        toinchfactor_= 0.039370147;
        break;
    }
}

bool TextPrinter::print(const QTextDocument *document, QString ficpdf, const QString &caption, bool AvecChoixImprimante, bool QueLePdf)
{
    if (!document) return false;
    // Get PDF document;
    QByteArray ba = getPDFByteArray(document);

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

        // Get imagelist from PDF ByteArray
        QList<QImage> pagelist = Utils::calcImagefromPdf(ba);
        PrintPageList(printer_,  pagelist);
    }

    // enregistre le pdf
    if (ficpdf != "")
    {
        Utils::writeBinaryFile(ba, ficpdf);
    }
    return true;
}


void TextPrinter::PrintPageList(QPrinter *Imprimante, QList<QImage> pagelist)
{
    if (Imprimante == Q_NULLPTR)
        return;

    QPainter PrintingPreView(Imprimante);

    for (int i=0; i<pagelist.size();++i)
    {
        if( i > 0 ) {
            Imprimante->newPage();
        }
        //QPixmap pix = QPixmap::fromImage(m_imagelist.at(i)).scaledToWidth(int(m_rect.width()),Qt::SmoothTransformation);
        QPageSize pgSize = Imprimante->pageLayout().pageSize();
        QImage page = pagelist.at(i).scaled(pgSize.sizePixels(Imprimante->resolution()), Qt::KeepAspectRatio);
        PrintingPreView.drawImage(QPoint(0,0),page);
    }
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
    if (QFileInfo(exportname).suffix().toLower().isEmpty())
        exportname.append(".pdf");

    QByteArray ba = getPDFByteArray(document);
    Utils::writeBinaryFile(ba, exportname);
}

bool TextPrinter::preview(const QTextDocument *document, QString ficpdf, const QString &caption)
{
    if (!document) return false;

    QByteArray ba = getPDFByteArray(document);
    // Get imagelist from PDF ByteArray
    QList<QImage> pagelist = Utils::calcImagefromPdf(ba);


    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer_, parent_);
    dialog->setWindowTitle(caption.isEmpty() ? "Print Preview" : caption);
    dialog->setWindowModality(Qt::WindowModal);
    connect(dialog, &QPrintPreviewDialog::paintRequested, this, [=](){ PrintPageList(printer_, pagelist); });


    // preview it
    int b = dialog->exec();
    if (b>0)
    {
        if (ficpdf != "")
        {
            Utils::writeBinaryFile(ba, ficpdf);
        }
    }
    delete dialog;
    return (b>0);
}

void TextPrinter::setPrinterName(QString printerName)
{
    if (printerName != "") printer_->setPrinterName(printerName);
}

QString TextPrinter::getPrinterName()
{
    return printer_->printerName();
}

///////////////////////////////////////////////////////////////////////////////
// private methods
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief TextPrinter::QRectF2device
 * \param rect
 * \param device
 * convert a QRectF in units_ to a QRectF in resolution in DPI of the device
 */
void TextPrinter::QRectF2device(QRectF *rect, QPaintDevice *device)
{
    int factorX = device->logicalDpiX() * toinchfactor_;
    int factorY = device->logicalDpiY() * toinchfactor_;
    qreal x = rect->x();
    qreal y = rect->y();
    qreal w = rect->width();
    qreal h = rect->height();
    rect->setRect(x * factorX, y * factorY, w * factorX, h * factorY);
}

QRectF TextPrinter::paperRectDPI(QPaintDevice *device)
{
    QRectF rect = printer_->paperRect(units_);
    QRectF2device(&rect, device);
    return rect;
}


/*! * \brief TextPrinter::adjustedContentRect
    * \param painter
    * \return the adjusted rect for the content block in DPI of painter
*/
QRectF TextPrinter::adjustedContentRect(QPainter *painter)
{
    // calculate size of content (paper - margins)
    QRectF rect = paperRectDPI(painter->device());

    rect.adjust(leftmargin_ * toinchfactor_ * painter->device()->logicalDpiX(),
                topmargin_* toinchfactor_ * painter->device()->logicalDpiY(),
                -rightmargin_ *toinchfactor_ * painter->device()->logicalDpiX(),
                -bottommargin_ *toinchfactor_ * painter->device()->logicalDpiX());

    // header
    if (headersize_ > 0) {
        rect.adjust(0, adjustedheaderheight(painter), 0, 0);
    }
    // footer
    if (footersize_ > 0) {
        rect.adjust(0, 0, 0, -footersize_ * toinchfactor_ * painter->device()->logicalDpiY());
        rect.adjust(0, 0, 0, -spacing_ * toinchfactor_ * painter->device()->logicalDpiY());
    }

    return rect;
}

qreal  TextPrinter::adjustedheaderheight(QPainter *painter)
{
    if (adjustedheaderheight_ == 0.0)
        adjustedHeaderRect(painter);
    return adjustedheaderheight_;
}

/*!
 * \brief TextPrinter::adjustedHeaderRect
 * \param painter
 * \return the adjusted rect of header block in DPI of painter
 */
QRectF TextPrinter::adjustedHeaderRect(QPainter *painter)
{
    QRectF rect;
    double onepoint = painter->device()->logicalDpiY() / 72.0;
    // header
    if (headersize_ > 0) {
        rect = paperRectDPI(painter->device());
        rect.adjust(leftmargin_ * toinchfactor_ * painter->device()->logicalDpiX(),
                    topmargin_* toinchfactor_ * painter->device()->logicalDpiY(),
                    -rightmargin_ *toinchfactor_ * painter->device()->logicalDpiX(),
                    0);

        painter->save();
        painter->translate(0, 0);
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(headertext_);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());

        adjustedheaderheight_ = doc.size().height();
        adjustedheaderheight_ += spacing_* toinchfactor_ * painter->device()->logicalDpiY();
        adjustedheaderheight_ += headerlinepenwidth_ * onepoint;
        adjustedheaderheight_ += spacing_* toinchfactor_ * painter->device()->logicalDpiY();
        rect.setBottom(adjustedheaderheight_);
        painter->restore();
    }
    return rect;
}

// footerRect() ///////////////////////////////////////////////////////////////
// calculate the rect for the footer block

QRectF TextPrinter::footerRect(QPaintDevice *device)
{
    /*! calculate printable size in units_ (default = mm) */
    /*! size of  paper in units */
    QRectF rect = printer_->paperRect(units_);
    /*!  size of footer (paper - margins) */
    rect.adjust(leftmargin_, 0,-rightmargin_,-bottommargin_);
    rect.setTop(rect.bottom() -footersize_);
    /*! convert to printer resolution */
    QRectF2device(&rect, device);
    return rect;
}

/*!
    * \brief TextPrinter::getPDFByteArray
    * \param document
    * \return
*/

QByteArray TextPrinter::getPDFByteArray(const QTextDocument *document)
{
    QByteArray bapdf;
    if (document)
    {
        QBuffer buf;
        buf.open(QIODeviceBase::WriteOnly);
        QPdfWriter writer(&buf);

        tempdoc_ = document->clone();
        QMarginsF pgMargins(0,0,0,0);
        QPageSize pgSize = printer_->pageLayout().pageSize();
        writer.setPageSize(pgSize);
        writer.setPageMargins(pgMargins);

        printToDevice(&writer);

        bapdf=buf.data();
        delete tempdoc_;
        tempdoc_ = Q_NULLPTR;
    }
    return bapdf;
}

void TextPrinter::printToDevice(QPagedPaintDevice *device)
{
    if (!device || !tempdoc_) return;

    QPainter painter(device);
    tempdoc_        ->setUseDesignMetrics(true);
    tempdoc_        ->documentLayout()->setPaintDevice(device);
    QSizeF sizeCr   = adjustedContentRect(&painter).size();
    tempdoc_        ->setPageSize(sizeCr);
    // dump existing margin (if any)
    QTextFrameFormat fmt =  tempdoc_->rootFrame()->frameFormat();
    fmt                     .setMargin(0);
    tempdoc_->rootFrame()   ->setFrameFormat(fmt);

    int    firstpage = 1;
    int    lastpage = tempdoc_->pageCount();

    // loop through and print pages
    painter.setRenderHints(QPainter::Antialiasing |
                           QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform, true);
    int pagenum = firstpage;
    while(true)
    {
        // print page
        paintPage(&painter, pagenum, lastpage);
        if (pagenum == lastpage) break;
        pagenum+=1;
        device->newPage();
    }
}

/*!
 * \brief TextPrinter::paintPage
 * paint an individual page of the document to the painter (QPdfWriter or QPrinter)
 * dessine la page avec le QPainter passé en paramètre (QPdfWriter ou QPrinter)
 * \param QPainter painter -> QPdfWriter or QPrinter
 * \param int pagenum -> current page number
 * \param int nbpages -> total number of pages
 */
void TextPrinter::paintPage(QPainter *painter, int pagenum, int nbpages)
{
    /*!
     *  Note the difference between Point and DevicePixel.
     *  The Point unit is defined to be 1/72th of an inch,
     *  while the DevicePixel unit is resolution dependent and is based on the actual pixels, or dots, on the printer.
     */

    QRectF rect;
    double onepoint = painter->device()->logicalDpiY() / 72.0;

/*! 1 - header ----------------------------------------------------------------------------------------------------------------------------------- */
    if (headersize_ > 0) {
        rect = adjustedHeaderRect(painter);

        // replace page variables
        QString header = headertext_;
        header.replace("&page;", QString::number(pagenum) + "/" + QString::number(nbpages));

        painter->save();
        painter->translate(rect.left(), rect.top());

        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(header);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());

        QRectF clip(0, 0, rect.width(), rect.height());
        doc.drawContents(painter, clip);
        painter->restore();

        if (headerlinepenwidth_ > 0.0) {
            /*! draw line between header and content */
            painter->save();
            painter->translate(0, rect.top() - spacing_ * toinchfactor_ * painter->device()->logicalDpiY());
            painter->setPen(QPen(Qt::blue, headerlinepenwidth_ * onepoint));               // choisit le format du pinceau
            painter->drawLine(rect.bottomLeft(),rect.bottomRight());                       // trace une ligne depuis le bas à gauche au bas à droite de l'en-tête
            painter->restore();
        }

    }

/*! 2 - footer ----------------------------------------------------------------------------------------------------------------------------------- */
    if (footersize_ > 0) {
        rect = footerRect(painter->device());
        if (footerlinepenwidth_ > 0.0) {
            /*! draw line between content and footer */
            painter->save();
            painter->setPen(QPen(Qt::black, footerlinepenwidth_ * onepoint));
            painter->drawLine(rect.topLeft(), rect.topRight());
            painter->restore();
        }

        // replace page variables
        QString footer = footertext_;
        footer.replace("&page;", QString::number(pagenum) + "/" + QString::number(nbpages));

        painter->save();
        painter->translate(rect.left(), rect.top() + spacing_ * toinchfactor_ * painter->device()->logicalDpiY());

        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(footer);
        doc.documentLayout()->setPaintDevice(painter->device());
        doc.setPageSize(rect.size());

        QRectF clip(0, 0, rect.width(), rect.height() - spacing_ * toinchfactor_ * painter->device()->logicalDpiY());
        doc.drawContents(painter, clip);
        painter->restore();
    }

 /*! 3 - content ----------------------------------------------------------------------------------------------------------------------------------- */
    painter->save();
    rect = adjustedContentRect(painter);
    painter->translate(rect.left(), rect.top() - (pagenum-1) * rect.height());
    QRectF clip(0, (pagenum-1) * rect.height(), rect.width(), rect.height());
    tempdoc_->drawContents(painter, clip);
    painter->restore();

}









/*! ----------- PAS UTILISÉS --------------------------- */
QPageSize TextPrinter::pageSize() const
{
    return printer_->pageLayout().pageSize();
}

void TextPrinter::setPageSize(QPageSize size)
{
    printer_->setPageSize(size);
}
QPageLayout::Orientation TextPrinter::orientation() const { return printer_->pageLayout().orientation();}
void TextPrinter::setOrientation(QPageLayout::Orientation orientation)
{
    // Valeur par defaut = QPageLayout::Portrait
    QPageLayout lay = printer_->pageLayout();
    lay.setOrientation(orientation);
}
void TextPrinter::setLeftMargin(double margin)
{
    // Valeur par defaut = 10mm
    if ((margin > 0) && (margin < printer_->paperRect(units_).width() / 2))
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
    if ((margin > 0) && (margin < printer_->paperRect(units_).width() / 2))
        rightmargin_ = margin;
    else
        rightmargin_ = 0;
}
double TextPrinter::topMargin() const
{
    return topmargin_;
}
double TextPrinter::bottomMargin() const
{
    return bottommargin_;
}
void TextPrinter::setBottomMargin(double margin)
{
    // Valeur par defaut = 15mm
    if ((margin > 0) && (margin < printer_->paperRect(units_).height() / 4))
        bottommargin_ = margin;
    else
        bottommargin_ = 0;
}
void TextPrinter::setMargins(double margin)
{
    if ((margin > 0)
            && (margin < printer_->paperRect(units_).height() / 2)
            && (margin < printer_->paperRect(units_).width() / 2))
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
    if ((spacing > 0) && (spacing <= printer_->paperRect(units_).height() / 4))
        spacing_ = spacing;
    else
        spacing_ = 0;
}
double TextPrinter::headerSize() const
{
    return headersize_;
}
double TextPrinter::headerLinePenWidth() const
{
    return headerlinepenwidth_;
}
void TextPrinter::setHeaderLinePenWidth(double pointsize)
{
    // Valeur par defaut = 0.5 pt (1/144 inch)
    headerlinepenwidth_ = qMax(0.0, pointsize);
}
const QString &TextPrinter::headerText() const
{
    return headertext_;
}

double TextPrinter::footerLinePenWidth() const
{
    return footerlinepenwidth_;
}
void TextPrinter::setFooterLinePenWidth(double pointsize)
{
    // Valeur par defaut = 0.5 pt (1/144 inch)
    footerlinepenwidth_ = qMax(0.0, pointsize);
}
const QString &TextPrinter::footerText() const
{
    return footertext_;
}
QPrinter::Unit TextPrinter::units() const
{
    return units_;
}


/*!       PRIVATES      */
/*!
 * \brief TextPrinter::x2device
 * \param x
 * \param device
 * \param unit
 * \return
 * return an x ccordinate expressed in unit to his value in resolution in DPI of the device
 */
qreal TextPrinter::x2device(qreal x, QPaintDevice *device, QPrinter::Unit unit)
{
    qreal correctX;
    switch (unit) {
    case QPrinter::Inch:
        correctX = 1;
        break;
    case QPrinter::Point:
        correctX = 1/72;
        break;
    case QPrinter::Millimeter:
       correctX = 0.039370147;
       break;
    default:
       correctX = 0.039370147;
       break;
    }
    /*! device horizontal resolution by unit */
    int factorX = device->logicalDpiX() * correctX;
    /*! number of dots in device resolution for x */
    return x * factorX;
}

/*!
 * \brief TextPrinter::y2device
 * \param x
 * \param device
 * \param unit
 * \return
 * return an y ccordinate expressed in unit to his value in resolution in DPI of the device
 */
qreal TextPrinter::y2device(qreal y, QPaintDevice *device, QPrinter::Unit unit)
{
    qreal correctY;
    switch (unit) {
    case QPrinter::Inch:
        correctY = 1;
        break;
    case QPrinter::Point:
        correctY = 1/72;
        break;
    case QPrinter::Millimeter:
       correctY = 0.039370147;
       break;
    default:
        correctY = 0.039370147;
        break;
    }
    /*! device vertical resolution by unit */
    int factorY = device->logicalDpiY() * correctY;
    /*! number of dots in device resolution for y */
    return y * factorY;
}








