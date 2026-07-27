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

#include "upmessagebox.h"
#include <QScreen>

// ─────────────────────────────────────────────────────────────────────────────
//  Plafond de largeur COMMUN à toutes les boîtes de message. La boîte est en
//  SetFixedSize (elle prend la taille de son contenu) : sans plafond, une phrase
//  longue donne un label plus large que l'écran, la boîte déborde et son bouton
//  de fermeture sort du cadre. On plafonne donc ICI, une fois pour toutes, la
//  largeur de CHAQUE label (titre et texte) — plus besoin d'y penser à chaque
//  message. Plafond fixe (les écrans cliniques sont larges), borné à 75 % de
//  l'écran par sécurité sur un petit écran.
static int largeurMaxMessage()
{
    int lmax = 700;
    if (const QScreen *ecran = QApplication::primaryScreen())
        lmax = qMin(lmax, int(ecran->availableGeometry().width() * 0.75));
    return lmax;
}

//  Fige la taille d'un label de boîte de message : si le texte tient sous le
//  plafond, taille exacte (comportement historique) ; sinon largeur plafonnée +
//  retour à la ligne (wordWrap), la hauteur étant recalculée par heightForWidth.
//  Le texte du label doit déjà être posé (heightForWidth s'appuie dessus).
static void figeTailleLabel(UpLabel *lbl, const QString &texteMesure)
{
    const QSize taille = Utils::CalcSize(texteMesure);
    const int   lmax   = largeurMaxMessage();
    if (taille.width() > lmax)
    {
        lbl->setWordWrap(true);
        lbl->setFixedWidth(lmax);
        lbl->setFixedHeight(lbl->heightForWidth(lmax));
    }
    else
        lbl->setFixedSize(taille);
}

// ═════════════════════════════════════════════════════════════════════════════
//  UpMessageBox — boîte de message « maison » (en remplacement de QMessageBox),
//  construite sur UpDialog. Disposition :
//      [ icône ]  |  [ titre en gras (wdg_texteditlbl) ]
//                    [ texte informatif (wdg_infolbl)  ]
//  En pratique on l'utilise surtout via les méthodes STATIQUES de fin de fichier
//  (Watch / Show / Question / Information) : elles créent la boîte, la configurent,
//  l'affichent (exec) puis la détruisent, et renvoient le bouton cliqué.
// ═════════════════════════════════════════════════════════════════════════════
UpMessageBox::UpMessageBox(QWidget *parent) : UpDialog(parent)
{
    // wdg_textlayout (titre + texte) est imbriqué dans wdg_infolayout ([icône] + textes),
    // lui-même inséré dans le layout d'UpDialog AU-DESSUS de la barre de boutons.
    // SetFixedSize : la boîte prend exactement la taille de son contenu.
    wdg_iconlbl             = new UpLabel();
    wdg_texteditlbl         = new UpLabel();
    wdg_infolbl             = new UpLabel();
    wdg_textlayout          = new QVBoxLayout();
    wdg_infolayout          = new QHBoxLayout();
    wdg_ReponsSmallButton   = Q_NULLPTR;
    wdg_ReponsPushButton    = Q_NULLPTR;
    // Texte sélectionnable (copier/coller) à la souris comme au clavier.
    wdg_texteditlbl         ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    wdg_infolbl             ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

    wdg_textlayout      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    wdg_textlayout      ->addSpacerItem(new QSpacerItem(350,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    wdg_infolayout      ->addLayout(wdg_textlayout);
    wdg_infolayout      ->setSpacing(30);
    wdg_textlayout      ->setSpacing(5);
    wdg_textlayout      ->setContentsMargins(0,0,0,0);
    dlglayout()     ->insertLayout(0,wdg_infolayout);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    setStageCount(1);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    setFont(qApp->font());
}

UpMessageBox::~UpMessageBox()
{
}

void UpMessageBox::addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton Style)
{
    button->setUpButtonStyle(Style);
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, this, [=, this] {Repons(button);});
}

void UpMessageBox::addButton(UpPushButton *button)
{
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, this, [=, this] {Repons(button);});
}

void UpMessageBox::removeButton(UpSmallButton *button)
{
    for (int i=0; i<buttonslayout()->count();i++)
    {
        UpSmallButton *buttonARetirer =  qobject_cast<UpSmallButton*>(buttonslayout()->itemAt(i)->widget());
        if (buttonARetirer!=Q_NULLPTR)
            if (buttonARetirer == button)
            {
                delete buttonARetirer;
                return;
            }
    }
}

// Mémorise le bouton cliqué (Small ou Push) puis ferme la boîte (accept()).
void UpMessageBox::Repons(QPushButton *button)
{
    UpSmallButton *but = qobject_cast<UpSmallButton*>(button);
    if (but != Q_NULLPTR)
        wdg_ReponsSmallButton = but;
    else
        wdg_ReponsPushButton = qobject_cast<UpPushButton*>(button);
    accept();
}

UpSmallButton* UpMessageBox::clickedButton() const
{
    return wdg_ReponsSmallButton;
}

UpPushButton* UpMessageBox::clickedpushbutton() const
{
    return wdg_ReponsPushButton;
}

/*!
 * \brief UpMessageBox::setIcon
 * \param icn
 * \param animatedIcon if true = uses animated gif else uses png or ico
*/

void UpMessageBox::setIcon(enum Icon icn, bool animatedIcon)
{
    bool resize = true;
    switch (icn) {
    case Warning:
        if (animatedIcon)
        {
            setAnimatedIcon(WarningGif);
            resize = false;
        }
        else
            wdg_iconlbl     ->setPixmap(QPixmap("://damn-icon.png").scaled(80,80));
        break;
    case Quest:
        if (animatedIcon)
        {
            setAnimatedIcon(QuestionGif);
            resize = false;
        }
        else
            wdg_iconlbl ->setPixmap(QPixmap("://question.png").scaled(80,80));
        break;
    case Info:
        if (animatedIcon)
        {
            setAnimatedIcon(InfoGif);
            resize = false;
        }
        else
            wdg_iconlbl     ->setPixmap(QPixmap("://information.png").scaled(80,80));
        break;
    case Critical:
        wdg_iconlbl     ->setPixmap(QPixmap("://cancel.png").scaled(80,80));
        break;
    case Print:
        wdg_iconlbl     ->setPixmap(QPixmap("://11865.png").scaled(80,80));
        break;
    }
    if (!resize)
        return;
    wdg_iconlbl     ->setFixedSize(80,80);
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
}

void UpMessageBox::setAnimatedIcon(enum Movie movie)
{
    switch (movie) {
    case WarningGif:
        m_movie         = new QMovie(":/warning.gif");
        m_movie         ->setScaledSize(QSize(80,80));
        break;
    case InfoGif:
        m_movie         = new QMovie(":/info.gif");
        m_movie         ->setScaledSize(QSize(80,80));
        break;
    case QuestionGif:
        m_movie         = new QMovie(":/question.gif");
        m_movie         ->setScaledSize(QSize(100,100));
        m_movie         ->setSpeed(400);
        break;
    }
    wdg_iconlbl     ->setMovie(m_movie);
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
    m_movie         ->start();
}

void UpMessageBox::setIconPixmap(QPixmap pix)
{
    wdg_iconlbl     ->setPixmap(pix);
    wdg_iconlbl     ->setFixedSize(pix.width(),pix.height());
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
}

// Titre (ligne en gras du haut). La taille du label est figée sur le texte (CalcSize).
void UpMessageBox::UpMessageBox::setText(QString Text)
{
    if (Text == "")
        return;
    wdg_texteditlbl         ->setStyleSheet("border: 0px solid; background-color: rgba(200,200,200,0)");
    wdg_texteditlbl         ->setText("<b>" + Text + "</b>");
    wdg_texteditlbl         ->setWordWrap(true);
    figeTailleLabel(wdg_texteditlbl, Text);
    wdg_textlayout          ->insertWidget(1,wdg_texteditlbl);
}

// Texte informatif (sous le titre). Text peut être du HTML (liens, gras…) : c'est un
// QLabel, qui détecte le texte riche tout seul. CalcSize retire les balises pour la mesure.
void UpMessageBox::setInformativeText(QString Text)
{
    if (Text == "")
        return;
    wdg_infolbl     ->setStyleSheet("border: 0px solid; background-color: rgba(200,200,200,0)");
    wdg_infolbl     ->setText(Text);
    wdg_infolbl     ->setWordWrap(true);
    int position = 1;
    if (qobject_cast<QLabel*>(wdg_textlayout->itemAt(1)->widget()) != Q_NULLPTR)
        position += 1;
    figeTailleLabel(wdg_infolbl, Text);
    wdg_textlayout      ->insertWidget(position,wdg_infolbl);
    wdg_textlayout      ->setSizeConstraint(QLayout::SetFixedSize);
}

void UpMessageBox::setDefaultButton(QPushButton *butt)
{
    butt->setDefault(true);
    butt->setFocus();
}

// Boîte d'information simple (icône Quest, un bouton OK). InfoText échappé (affiché tel quel).
void UpMessageBox::Show(QWidget *parent, QString Text, QString InfoText)
{
    UpMessageBox *msgbox     = new UpMessageBox(parent);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText.toHtmlEscaped());
    msgbox  ->setIcon(UpMessageBox::Quest);
    msgbox  ->AjouteLayButtons(UpDialog::ButtonOK);
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    //! Le dimensionnement (plafonné) est fait par setText()/setInformativeText().

    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  qobject_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
            connect(butt, &QPushButton::clicked, msgbox, &UpMessageBox::accept);
    }
    msgbox  ->exec();
    delete msgbox;
}

// Boîte d'AVERTISSEMENT (icône Warning) la plus utilisée. Renvoie le style du bouton cliqué.
//  - InfoText : les sauts de ligne « \n » sont convertis en « <br> » puis le tout est passé
//    dans un UpTextEdit → toHtml() (texte riche homogène).
//  - link (optionnel) : si renseigné, le texte peut contenir un lien <a href="…"> ; le label
//    devient cliquable et ouvre l'URL dans le navigateur (cf. plus bas).
UpSmallButton::StyleBouton UpMessageBox::Watch(QWidget *parent, QString Text, QString InfoText, Buttons Butts, QString link)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);

    msgbox->setIcon(Warning);

    msgbox  ->setText(Text);
    UpTextEdit text(InfoText.replace("\n","<br>"));
    msgbox  ->setInformativeText(text.toHtml());
    msgbox  ->AjouteLayButtons(Butts);

    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  qobject_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
        {
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, msgbox, [=] {msgbox->Repons(butt);});
            if (butt->ButtonStyle() == UpSmallButton::STARTBUTTON)
                butt->setText("OK");
        }
    }
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    //! Le dimensionnement (plafonné, avec retour à la ligne si besoin) est fait par
    //! setText()/setInformativeText() → figeTailleLabel(). Rien à refaire ici.
    // Lien cliquable : on rend les liens du label accessibles à la souris et on autorise
    // l'ouverture externe (setOpenExternalLinks → QLabel ouvre l'URL au clic). Le connect
    // sur linkActivated est un filet de sécurité explicite (ouvre l'URL via QDesktopServices)
    // au cas où openExternalLinks ne suffirait pas dans certains contextes.
    if (link != "")
    {
        msgbox  ->wdg_infolbl   ->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        msgbox  ->wdg_infolbl   ->setOpenExternalLinks(true);
        connect (msgbox->wdg_infolbl, &QLabel::linkActivated, msgbox,
                [=] { QDesktopServices::openUrl(QUrl(link)); });
    }

    return ExecMsgBox(msgbox);
}

// exec() la boîte, renvoie le style du bouton cliqué (CANCELBUTTON par défaut) puis la détruit.
UpSmallButton::StyleBouton UpMessageBox::ExecMsgBox(UpMessageBox*msgbox)
{
    UpSmallButton::StyleBouton repons = UpSmallButton::CANCELBUTTON;
    if (msgbox  ->exec() == QDialog::Accepted)
        repons = msgbox->clickedButton()->ButtonStyle();
    //qDebug() << Utils::EnumDescription(QMetaEnum::fromType<UpSmallButton::StyleBouton>(), repons);
    delete msgbox;
    return repons;
}


// Boîte de QUESTION (icône Quest) : libellés des boutons fournis via titresboutonslist
// (dans l'ordre). Renvoie le style du bouton cliqué.
UpSmallButton::StyleBouton UpMessageBox::Question(QWidget *parent, QString Text, QString InfoText, Buttons Butts, QStringList titresboutonslist)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox->setIcon(Quest);

    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText);
    msgbox  ->AjouteLayButtons(Butts);
    int k = 0;
    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  qobject_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
        {
            if (titresboutonslist.size()>k)
                butt->setText(titresboutonslist.at(k));
            k++;
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, msgbox, [=] {msgbox->Repons(butt);});
        }
    }
    //! Dimensionnement (plafonné) fait par setText()/setInformativeText().
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);

    return ExecMsgBox(msgbox);
}

// Boîte d'information (icône Info, un bouton OK). Ne renvoie rien (simple accusé de lecture).
void UpMessageBox::Information(QWidget *parent, QString Text, QString InfoText)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText);
    msgbox  ->setIcon(UpMessageBox::Info);

    msgbox  ->AjouteLayButtons(UpDialog::ButtonOK);
    connect (msgbox->OKButton, &QPushButton::clicked, msgbox, [=] {msgbox->accept();});
    //! Dimensionnement (plafonné) fait par setText()/setInformativeText().
    msgbox  ->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->exec();
    delete msgbox;
}
