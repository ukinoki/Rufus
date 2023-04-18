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

#include "uptextedit.h"
#include <QTextBlock>

UpTextEdit::UpTextEdit(QWidget *parent) : QTextEdit(parent)
{
    m_valeuravant = "";
    m_valeurapres = "";
    m_champ       = "";
    m_table       = "";
    m_id          = -1;
    m_iduser      = -1;
    installEventFilter(this);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,  &UpTextEdit::customContextMenuRequested, this, &UpTextEdit::MenuContextuel);
    setMouseTracking(true);
}

UpTextEdit::UpTextEdit(QString txt, QWidget *parent) : QTextEdit(txt, parent)
{
    m_valeuravant = "";
    m_valeurapres = "";
    m_champ       = "";
    m_table       = "";
    m_id          = -1;
    m_iduser      = -1;
    installEventFilter(this);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,  &UpTextEdit::customContextMenuRequested, this, &UpTextEdit::MenuContextuel);
    setMouseTracking(true);
}

void UpTextEdit::MenuContextuel()
{
    QMenu *gmenuContextuel          = new QMenu();

    if (textCursor().selectedText().size() > 0)   {
        QAction *pAction_ModifPolice    = gmenuContextuel->addAction(Icons::icFont(),           tr("Modifier la police"));
        QAction *pAction_Fontbold       = gmenuContextuel->addAction(Icons::icFontbold(),       tr("Gras"));
        QAction *pAction_Fontitalic     = gmenuContextuel->addAction(Icons::icFontitalic(),     tr("Italique"));
        QAction *pAction_Fontunderline  = gmenuContextuel->addAction(Icons::icFontunderline(),  tr("Souligné"));
        QAction *pAction_Fontnormal     = gmenuContextuel->addAction(Icons::icFontnormal(),     tr("Normal"));
        gmenuContextuel->addSeparator();
        connect (pAction_ModifPolice,   &QAction::triggered,    this, [=] {ChoixMenuContextuel("Police");});
        connect (pAction_Fontbold,      &QAction::triggered,    this, [=] {ChoixMenuContextuel("Gras");});
        connect (pAction_Fontitalic,    &QAction::triggered,    this, [=] {ChoixMenuContextuel("Italique");});
        connect (pAction_Fontunderline, &QAction::triggered,    this, [=] {ChoixMenuContextuel("Souligne");});
        connect (pAction_Fontnormal,    &QAction::triggered,    this, [=] {ChoixMenuContextuel("Normal");});
    }
    QAction *pAction_Blockleft          = gmenuContextuel->addAction(Icons::icBlockLeft(),      tr("Aligné à gauche"));
    QAction *pAction_Blockright         = gmenuContextuel->addAction(Icons::icBlockRight(),     tr("Aligné à droite"));
    QAction *pAction_Blockcentr         = gmenuContextuel->addAction(Icons::icBlockCenter(),    tr("Centré"));
    QAction *pAction_Blockjust          = gmenuContextuel->addAction(Icons::icBlockJustify(),   tr("Justifié"));
    gmenuContextuel->addSeparator();
    if (textCursor().selectedText().size() > 0)   {
        QAction *pAction_Copier         = gmenuContextuel->addAction(Icons::icCopy(),            tr("Copier"));
        QAction *pAction_Cut            = gmenuContextuel->addAction(Icons::icCut(),             tr("Couper"));
        connect (pAction_Copier,        &QAction::triggered,    this, [=] {ChoixMenuContextuel("Copier");});
        connect (pAction_Cut,           &QAction::triggered,    this, [=] {ChoixMenuContextuel("Couper");});
    }
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasText() || mimeData->hasUrls() || mimeData->hasImage() || mimeData->hasHtml())
    {
        QAction *pAction_Coller         = gmenuContextuel->addAction(Icons::icPaste(),  tr("Coller"));
        connect (pAction_Coller,        &QAction::triggered,    this, [=] {ChoixMenuContextuel("Coller");});
    }

    connect (pAction_Blockcentr,    &QAction::triggered,    this, [=] {ChoixMenuContextuel("Centre");});
    connect (pAction_Blockright,    &QAction::triggered,    this, [=] {ChoixMenuContextuel("Droite");});
    connect (pAction_Blockleft,     &QAction::triggered,    this, [=] {ChoixMenuContextuel("Gauche");});
    connect (pAction_Blockjust,     &QAction::triggered,    this, [=] {ChoixMenuContextuel("Justifie");});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void UpTextEdit::ChoixMenuContextuel(QString choix)
{
    if (choix       == "Coller")    paste();
    else if (choix  == "Copier")    copy();
    else if (choix  == "Couper")    cut();
    else if (choix  == "Police")
    {
        bool ok = false;
        QFont police = QFontDialog::getFont(&ok, qApp->font(), Q_NULLPTR, tr("Choisissez une police"));
        if (ok)
        {
            QTextCharFormat format;
            format.setFont(police);
            textCursor().setCharFormat(format);
        }
    }
    else if (choix  == "Gras")
    {
        QTextCharFormat format  = textCursor().charFormat();
        format.setFontWeight(QFont::Bold);
        format.setFontUnderline(format.fontUnderline());
        format.setFontItalic(format.fontItalic());
        textCursor().setCharFormat(format);
    }
    else if (choix  == "Italique")
    {
        QTextCharFormat format  = textCursor().charFormat();
        format.setFontItalic(true);
        format.setFontUnderline(format.fontUnderline());
        format.setFontWeight(format.fontWeight());
        textCursor().setCharFormat(format);
    }
    else if (choix  == "Souligne")
    {
        QTextCharFormat format  = textCursor().charFormat();
        format.setFontUnderline(true);
        format.setFontItalic(format.fontItalic());
        format.setFontWeight(format.fontWeight());
        textCursor().setCharFormat(format);
    }
    else if (choix  == "Normal")
    {
        QTextCharFormat format  = textCursor().charFormat();
        format.setFont(qApp->font());
        textCursor().setCharFormat(format);
    }
    else if (choix  == "Gauche")
    {
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignLeft);
        textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Justifie")
    {
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignJustify);
        textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Droite")    {
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignRight);
        textCursor().setBlockFormat(blockformat);
    }
    else if (choix  == "Centre")    {
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignCenter);
        textCursor().setBlockFormat(blockformat);
    }
}


bool UpTextEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        setvaleuravant(toHtml());
    }

    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key()==Qt::Key_Tab)
        {
            // Ctrl-Return ou Ctrl-Enter ou Ctrl-Tab sur un TextEdit- On va sur la tabulation suivante -------------
            if (keyEvent->modifiers() == Qt::MetaModifier)
            {
                UpTextEdit *textnext = dynamic_cast<UpTextEdit*>(nextInFocusChain());
                if (textnext){
                    textnext->setFocus();
                    textnext->moveCursor(QTextCursor::End);
                }
                else
                    focusNextChild();
                return true;
            }
            // Shif-Return ou Shift-Enter ou Shift-Tab sur un TextEdit- On va sur la tabulation suivante --------------
            if (keyEvent->modifiers() == Qt::ShiftModifier)
            {
                {
                    UpTextEdit *textprev = dynamic_cast<UpTextEdit*>(previousInFocusChain());
                    if (textprev)
                    {
                        textprev->setFocus();
                        textprev->moveCursor(QTextCursor::End);
                    }
                    else
                        focusNextChild();
                    return true;
                }
            }
        }
        if (toHtml() != valeuravant())
            emit textEdited(toHtml());
    }
    return QWidget::eventFilter(obj, event);
}
void UpTextEdit::mouseDoubleClickEvent(QMouseEvent * event )
{
    emit dblclick(iD());
    event->ignore();
}

bool UpTextEdit::canInsertFromMimeData( const QMimeData *source ) const
{
    if (source->hasImage())
        return m_acceptimagemimedatas;
    else
        return QTextEdit::canInsertFromMimeData(source);
}

int UpTextEdit::row() const
{
    return m_row;
}

void UpTextEdit::setRow(int row)
{
    m_row = row;
}

int UpTextEdit::column() const
{
    return m_column;
}

void UpTextEdit::setColumn(int column)
{
    m_column = column;
}

bool UpTextEdit::acceptimagemimedatas() const
{
    return m_acceptimagemimedatas;
}

void UpTextEdit::setAcceptImageMimeDatas(bool acceptimagemimedatas)
{
    m_acceptimagemimedatas = acceptimagemimedatas;
}

void UpTextEdit::insertFromMimeData( const QMimeData *source )
{
    if (source->hasImage())
    {
        if(m_acceptimagemimedatas)
        {
            QImage image = qvariant_cast<QImage>(source->imageData());
            image = image.scaledToWidth(int(width()*2/3), Qt::SmoothTransformation);
            QString path = document()->baseUrl().path() + "/" + QString::number(iD())+ "_img_" + QTime::currentTime().toString("HHmmss") + ".jpg";
            image.save(path);
            document()->addResource(QTextDocument::ImageResource, QUrl(path), image);
            textCursor().insertImage(path);
            QTextBlockFormat blockformat  = textCursor().blockFormat();
            blockformat.setAlignment(Qt::AlignCenter);
            textCursor().setBlockFormat(blockformat);
            textCursor().insertHtml(HTML_RETOURLIGNE);
        }
    }
    else QTextEdit::insertFromMimeData(source);
}

void UpTextEdit::setiD(int id)
{
    m_id = id;
}

int UpTextEdit::iD() const
{
    return m_id;
}

void UpTextEdit::setIdUser(int id)
{
    m_iduser = id;
}

int UpTextEdit::idUser() const
{
    return m_iduser;
}

void UpTextEdit::setvaleuravant(QString valprec)
{
    m_valeuravant = valprec;
}

QString UpTextEdit::valeuravant() const
{
    return m_valeuravant;
}

void UpTextEdit::setvaleurapres(QString valpost)
{
    m_valeurapres = valpost;
}

QString UpTextEdit::valeurapres() const
{
    return m_valeurapres;
}

void UpTextEdit::setchamp(QString champcorrespondant)
{
    m_champ = champcorrespondant;
}

QString UpTextEdit::champ() const
{
    return m_champ;
}

void UpTextEdit::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
}

void UpTextEdit::settable(QString tablecorrespondant)
{
    m_table = tablecorrespondant;
}

QString UpTextEdit::table() const
{
    return m_table;
}

void UpTextEdit::setText(const QString &text)
{
    QString txt = text;
    if (text.contains("<!DOCTYPE HTML PUBLIC"))
    {
#ifdef Q_OS_LINUX
        if (!text.contains(HTMLCOMMENT_LINUX))
            txt.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:" + QString::number(qApp->font().pointSize()) + "pt");
#endif
#ifdef Q_OS_MACOS
        if (text.contains(HTMLCOMMENT_LINUX))
            txt.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:" + QString::number(qApp->font().pointSize()) + "pt");
#endif
        Utils::epureFontFamily(txt);
        QTextEdit::setText(txt);
    }
    else
        QTextEdit::setText(text);
    m_modified = (txt != text);
}

/*!
 * \brief UpTextEdit::appendHtml
 * \param appendtext
 * \param supprimeLesLignesVidesDuMilieu - true par défaut
 * \param rajouteunelignealafin - true par défaut
 * \return
 */
QString UpTextEdit::appendHtml(QString appendtext, bool supprimeLesLignesVidesDuMilieu, bool rajouteunelignealafin)
{
    QString texte = toHtml();
    Utils::retirelignevidefinhtml(texte);
    texte += appendtext;
    if (rajouteunelignealafin)
        texte += HTML_FINPARAGRAPH;
    Utils::nettoieHTML(texte, supprimeLesLignesVidesDuMilieu);
    setHtml(texte);
    return texte;
}

