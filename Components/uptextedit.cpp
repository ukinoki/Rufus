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

#include "uptextedit.h"

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
}

void UpTextEdit::MenuContextuel()
{
    QMenu *gmenuContextuel          = new QMenu();
    QAction *pAction_ModifPolice    = new QAction();
    QAction *pAction_Fontbold       = new QAction();
    QAction *pAction_Fontitalic     = new QAction();
    QAction *pAction_Fontunderline  = new QAction();
    QAction *pAction_Fontnormal     = new QAction();
    QAction *pAction_Copier         = new QAction();
    QAction *pAction_Cut            = new QAction();
    QAction *pAction_Coller         = new QAction();
    QAction *pAction_Blockcentr     = new QAction();
    QAction *pAction_Blockjust      = new QAction();
    QAction *pAction_Blockright     = new QAction();
    QAction *pAction_Blockleft      = new QAction();

    if (textCursor().selectedText().size() > 0)   {
        pAction_ModifPolice    = gmenuContextuel->addAction(Icons::icFont(),           tr("Modifier la police"));
        pAction_Fontbold       = gmenuContextuel->addAction(Icons::icFontbold(),       tr("Gras"));
        pAction_Fontitalic     = gmenuContextuel->addAction(Icons::icFontitalic(),     tr("Italique"));
        pAction_Fontunderline  = gmenuContextuel->addAction(Icons::icFontunderline(),  tr("Souligné"));
        pAction_Fontnormal     = gmenuContextuel->addAction(Icons::icFontnormal(),     tr("Normal"));
        gmenuContextuel->addSeparator();
    }
    pAction_Blockleft           = gmenuContextuel->addAction(Icons::icBlockLeft(),      tr("Aligné à gauche"));
    pAction_Blockright          = gmenuContextuel->addAction(Icons::icBlockRight(),     tr("Aligné à droite"));
    pAction_Blockcentr          = gmenuContextuel->addAction(Icons::icBlockCenter(),    tr("Centré"));
    pAction_Blockjust           = gmenuContextuel->addAction(Icons::icBlockJustify(),   tr("Justifié"));
    gmenuContextuel->addSeparator();
    if (textCursor().selectedText().size() > 0)   {
        pAction_Copier         = gmenuContextuel->addAction(Icons::icCopy(),            tr("Copier"));
        pAction_Cut            = gmenuContextuel->addAction(Icons::icCut(),             tr("Couper"));
    }
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasText() || mimeData->hasUrls() || mimeData->hasImage() || mimeData->hasHtml())
        pAction_Coller         = gmenuContextuel->addAction(Icons::icPaste(),  tr("Coller"));

    connect (pAction_Fontbold,      &QAction::triggered,    [=] {ChoixMenuContextuel("Gras");});
    connect (pAction_Fontitalic,    &QAction::triggered,    [=] {ChoixMenuContextuel("Italique");});
    connect (pAction_Fontunderline, &QAction::triggered,    [=] {ChoixMenuContextuel("Souligne");});
    connect (pAction_Fontnormal,    &QAction::triggered,    [=] {ChoixMenuContextuel("Normal");});
    connect (pAction_ModifPolice,   &QAction::triggered,    [=] {ChoixMenuContextuel("Police");});
    connect (pAction_Blockcentr,    &QAction::triggered,    [=] {ChoixMenuContextuel("Centre");});
    connect (pAction_Blockright,    &QAction::triggered,    [=] {ChoixMenuContextuel("Droite");});
    connect (pAction_Blockleft,     &QAction::triggered,    [=] {ChoixMenuContextuel("Gauche");});
    connect (pAction_Blockjust,     &QAction::triggered,    [=] {ChoixMenuContextuel("Justifie");});
    connect (pAction_Copier,        &QAction::triggered,    [=] {ChoixMenuContextuel("Copier");});
    connect (pAction_Coller,        &QAction::triggered,    [=] {ChoixMenuContextuel("Coller");});
    connect (pAction_Cut,           &QAction::triggered,    [=] {ChoixMenuContextuel("Couper");});

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
        QTextCursor curs = textCursor();
        moveCursor(QTextCursor::StartOfBlock);
        moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignLeft);
        textCursor().setBlockFormat(blockformat);
        setTextCursor(curs);
    }
    else if (choix  == "Justifie")
    {
        QTextCursor curs = textCursor();
        moveCursor(QTextCursor::StartOfBlock);
        moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignJustify);
        textCursor().setBlockFormat(blockformat);
        setTextCursor(curs);
    }
    else if (choix  == "Droite")    {
        QTextCursor curs = textCursor();
        moveCursor(QTextCursor::StartOfBlock);
        moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignRight);
        textCursor().setBlockFormat(blockformat);
        setTextCursor(curs);
    }
    else if (choix  == "Centre")    {
        QTextCursor curs = textCursor();
        moveCursor(QTextCursor::StartOfBlock);
        moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignCenter);
        textCursor().setBlockFormat(blockformat);
        setTextCursor(curs);
    }
}


bool UpTextEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        UpTextEdit* objUpText = static_cast<UpTextEdit*>(obj);
        objUpText->setvaleuravant(objUpText->toHtml());
    }

    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key()==Qt::Key_Tab)
        {
            // Ctrl-Return ou Ctrl-Enter ou Ctrl-Tab sur un TextEdit- On va sur la tabulation suivante -------------
            if (keyEvent->modifiers() == Qt::MetaModifier)
            {
                UpTextEdit *textw = static_cast<UpTextEdit*>(obj);
                UpTextEdit *textnext = dynamic_cast<UpTextEdit*>(textw->nextInFocusChain());
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
                    UpTextEdit *textw = static_cast<UpTextEdit*>(obj);
                    UpTextEdit *textprev = dynamic_cast<UpTextEdit*>(textw->previousInFocusChain());
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
    }
    return QWidget::eventFilter(obj, event);
}
void UpTextEdit::mouseDoubleClickEvent(QMouseEvent * event )
{
    emit dblclick(iD());
    event->ignore();
}

void UpTextEdit::setiD(int idadef)
{
    m_id = idadef;
}

int UpTextEdit::iD() const
{
    return m_id;
}

void UpTextEdit::setIdUser(int idadef)
{
    m_iduser = idadef;
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
    if (text.contains("<!DOCTYPE HTML PUBLIC"))
    {
        QString txt = text;
#ifdef Q_OS_LINUX
        if (!text.contains(HTMLCOMMENT_LINUX))
            txt.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:" + QString::number(qApp->font().pointSize()) + "pt");
#endif
#ifdef Q_OS_MACOS
        if (text.contains(HTMLCOMMENT_LINUX))
            txt.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:" + QString::number(qApp->font().pointSize()) + "pt");
#endif
        txt.replace(QRegExp("font-family:'([a-zA-Z -]*)'"),"font-family:'" + qApp->font().family() + "'");
        QTextEdit::setText(txt);
    }
    else
        QTextEdit::setText(text);
}

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

