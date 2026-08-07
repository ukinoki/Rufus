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

#include "updialog.h"
#include <QtCore/qdir.h>
#include <QShowEvent>

UpDialog::UpDialog(QWidget *parent) : QDialog(parent)
{
    setFont(qApp->font());
    m_globallay->setContentsMargins(m_marges);
    m_globallay->setSpacing(m_marge);
    setWindowModality(Qt::ApplicationModal);
    setMouseTracking(true);
    wdg_buttonswidget   = new QWidget();
    wdg_buttonslayout   = new QHBoxLayout();
    wdg_buttonslayout   ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    wdg_buttonslayout   ->setContentsMargins(0,10,0,10);
    wdg_buttonslayout   ->setSpacing(10);
    wdg_buttonswidget   ->setLayout(wdg_buttonslayout);
    int n               = m_globallay->count();
    m_globallay         ->insertWidget(n,wdg_buttonswidget);
    setStageCount(0.7);
}

void UpDialog::setSaveGeometry(QString geometryname)
{
    QSettings sets(PATH_FILE_INI, QSettings::IniFormat);
    m_originalgeometry  = sets.value(DialogGeometry + geometryname).toRect();
    if (m_originalgeometry != QRect())
        setGeometry(m_originalgeometry);
    connect(this,   &QDialog::finished, this,   [=, this] {
            if (m_enregistreposition)
            {
                QSettings sets(PATH_FILE_INI, QSettings::IniFormat);
                sets.setValue(DialogGeometry + geometryname, geometry());
            }
                                                    });

}

void UpDialog::mouseMoveEvent(QMouseEvent* event)
{
    /*QToolTip::showText( event->globalPosition().toPoint(),
                        QString::number( QCursor::pos().x() ) + ", " +
                        QString::number( QCursor::pos().y() ),
                        this, rect() );*/
    QWidget::mouseMoveEvent(event);
}

void UpDialog::addSearchLine()
{
    wdg_label               = new UpLabel();
    wdg_label               ->setFixedSize(21,21);
    wdg_label               ->setPixmap(Icons::pxLoupe().scaled(20,20)); //WARNING : icon scaled : pxLoupe 20,20
    wdg_chercheuplineedit   = new UpLineEdit();
    wdg_chercheuplineedit   ->setFixedSize(180,25);
    wdg_chercheuplineedit   ->setStyleSheet(
    "UpLineEdit {background-color:white; border: 1px solid rgb(150,150,150);border-radius: 10px;}"
    "UpLineEdit:focus {border: 2px solid rgb(164, 205, 255);border-radius: 10px;}");
    wdg_buttonslayout->insertWidget(0,wdg_label);
    wdg_buttonslayout->insertWidget(0,wdg_chercheuplineedit);
}

void UpDialog::AjouteLayButtons(Buttons Button)
{
    m_Buttons |= Button;
    // le Button Cancel est toujours juste à gauche du bouton OK
    // OK le plus à droite
    if (Button.testFlag(ButtonOK))
    {
        OKButton            = new UpSmallButton();
        OKButton            ->setUpButtonStyle(UpSmallButton::STARTBUTTON);
        // Ctrl+Entrée = valider et fermer la fiche.
        // Le cas particulier, c'est macOS (et non Windows) : Qt y intervertit Ctrl et Cmd,
        // donc Qt::META désigne la touche Ctrl PHYSIQUE du Mac. Sous Linux, Qt::META vaut la
        // touche Super (⊞), captée par le gestionnaire de fenêtres → le raccourci n'arrive
        // jamais à Rufus : il faut Qt::CTRL, comme sous Windows.
#ifdef Q_OS_MACOS
        OKButton            ->setShortcut(QKeySequence(Qt::META | Qt::Key_Return));
#else
        OKButton            ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
#endif
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count(), OKButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonPdf))
    {
        PdfButton           = new UpSmallButton();
        PdfButton           ->setUpButtonStyle(UpSmallButton::PDFBUTTON);
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count() - m_nbbuttons, PdfButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonRecord))
    {
        RecordButton        = new UpSmallButton();
        RecordButton        ->setUpButtonStyle(UpSmallButton::RECORDBUTTON);
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count() - m_nbbuttons, RecordButton);
        m_nbbuttons ++;
    }
     if (Button.testFlag(ButtonPrint))
    {
        PrintButton         = new UpSmallButton();
        PrintButton         ->setUpButtonStyle(UpSmallButton::PRINTBUTTON);
        // idem OKButton : Qt::META = Super sous Linux (capté par le bureau) → Qt::CTRL ; Mac = Qt::META
#ifdef Q_OS_MACOS
        PrintButton         ->setShortcut(QKeySequence(Qt::META | Qt::Key_P));
#else
        PrintButton         ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
#endif
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count() - m_nbbuttons, PrintButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonSuppr))
    {
        SupprButton         = new UpSmallButton();
        SupprButton         ->setUpButtonStyle(UpSmallButton::SUPPRBUTTON);
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count() - m_nbbuttons, SupprButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonEdit))
    {
        EditButton          = new UpSmallButton();
        EditButton          ->setUpButtonStyle(UpSmallButton::EDITBUTTON);
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count() - m_nbbuttons, EditButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonOups))
    {
        OupsButton         = new UpSmallButton();
        OupsButton         ->setUpButtonStyle(UpSmallButton::OUPSBUTTON);
        wdg_buttonslayout  ->insertWidget(wdg_buttonslayout->count() - m_nbbuttons, OupsButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonClose))
    {
        CloseButton         = new UpSmallButton();
        CloseButton         ->setUpButtonStyle(UpSmallButton::CLOSEBUTTON);
        // idem OKButton : Qt::META = Super sous Linux (capté par le bureau) → Qt::CTRL ; Mac = Qt::META
#ifdef Q_OS_MACOS
        CloseButton         ->setShortcut(QKeySequence(Qt::META | Qt::Key_C));
#else
        CloseButton         ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
#endif
        wdg_buttonslayout   ->insertWidget(wdg_buttonslayout->count(), CloseButton);
        m_nbbuttons ++;
    }
    if (Button.testFlag(ButtonCancel))
    {
        CancelButton        = new UpSmallButton();
        CancelButton        ->setShortcut(QKeySequence(Qt::Key_F12));
        CancelButton        ->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        wdg_buttonslayout   ->insertWidget(1, CancelButton);
        m_nbbuttons ++;
        connect(CancelButton,   &QPushButton::clicked, this, &UpDialog::reject);
    }
    UpdateTabOrder();
}

void UpDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    //! À l'affichage, le texte des boutons (donc leur hauteur) est fixé : c'est le bon moment pour
    //! aligner la rangée. Idempotent → sans effet aux affichages suivants (déjà uniformes).
    uniformiserHauteurBoutons();
}

//! Défaut historique : dans une rangée, un bouton AVEC texte passe en hauteur 45 (UpSmallButton) tandis
//! qu'un bouton à icône seule reste à 35 → ils n'étaient pas à la même hauteur (typiquement OK « Se
//! connecter » à côté d'Annuler). On aligne tout le monde sur le PLUS HAUT : comme le plus grand bouton
//! imposait déjà cette hauteur au conteneur, la rangée ne grandit pas et rien ne peut être tronqué.
void UpDialog::uniformiserHauteurBoutons()
{
    const QList<UpSmallButton*> boutons = wdg_buttonswidget->findChildren<UpSmallButton*>();
    int hmax = 0;
    for (UpSmallButton* b : boutons)
    {
        const int h = b->maximumHeight();               // = hauteur FIXÉE du bouton (35 ou 45)
        if (h > 0 && h < QWIDGETSIZE_MAX)               // garde-fou : ignore un bouton sans hauteur fixée
            hmax = qMax(hmax, h);
    }
    if (hmax <= 0)
        return;
    for (UpSmallButton* b : boutons)
        if (b->maximumHeight() != hmax)
            b->setFixedHeight(hmax);
}

QVBoxLayout* UpDialog::dlglayout() const
{
    return m_globallay;
}

QHBoxLayout* UpDialog::buttonslayout() const
{
    return wdg_buttonslayout;
}

QWidget* UpDialog::widgetbuttons() const
{
    return wdg_buttonswidget;
}

/*!
 * \brief UpDialog::setStageCount(double nbstages)
 * permet de fixer grossièrement le hauteur du widget contenant les buttons
 * de manière à connaître cette valeur à l'avance, avant l'affichage de la fiche
 * si que des upsmallbutton sans texte nbstages = 0.7
 * avec des upsmallbuttons et du texte nbstages = 1.5
 * et plus si des widgets de plus grande taille dans le widget de buttons
 * à déterminer à chaque fois pifométriquement
 * je sais, c'est du bricolage
 */
void UpDialog::setStageCount(double nbstages)
{
    double stages = m_stageheight; //! to cast m_stageheight from int to double
    m_nstages = nbstages;
    wdg_buttonswidget->setFixedHeight(int(stages * nbstages) + wdg_buttonslayout->contentsMargins().bottom() + wdg_buttonslayout->contentsMargins().top());
}

void UpDialog::UpdateTabOrder()
{
    if (wdg_buttonslayout->findChildren<UpSmallButton*>().size()>1)
        for (int i=0; i<wdg_buttonslayout->findChildren<UpSmallButton*>().size()-1; i++)
            setTabOrder(wdg_buttonslayout->findChildren<UpSmallButton*>().at(i),wdg_buttonslayout->findChildren<UpSmallButton*>().at(i+1));
}

UpDialog::Buttons UpDialog::buttons() const
{
    return m_Buttons;
}

int UpDialog::nbbuttons() const
{
    return m_nbbuttons;
}

int UpDialog::stageheight() const
{
    return m_stageheight;
}

void UpDialog::AjouteWidgetLayButtons(QWidget *widg, bool ALaFin)
{
    if (ALaFin)
        wdg_buttonslayout->addWidget(widg);
    else
        wdg_buttonslayout->insertWidget(0,widg);
    UpSmallButton *but = qobject_cast<UpSmallButton*>(widg);
    if (but != Q_NULLPTR)
    {
        if (but->ButtonStyle() == UpSmallButton::CANCELBUTTON)
            but    ->setShortcut(QKeySequence("F12"));
        // "Meta+Return" (forme chaîne) souffrait du même défaut : sous Linux Meta = Super (capté
        // par le bureau), et même sous Windows Meta = touche ⊞ → le raccourci ne marchait que sur Mac.
        // Forme enum portable : Mac = Qt::META (Ctrl physique), Windows + Linux = Qt::CTRL.
        else if (but->ButtonStyle() == UpSmallButton::STARTBUTTON || but->ButtonStyle() == UpSmallButton::CLOSEBUTTON)
#ifdef Q_OS_MACOS
            but    ->setShortcut(QKeySequence(Qt::META | Qt::Key_Return));
#else
            but    ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
#endif
        UpdateTabOrder();
    }
    if (wdg_buttonslayout->findChildren<UpPushButton *>().size()>1)
        setStageCount(1);
}

void UpDialog::setMode(Mode mode)
{
     m_mode = mode;
}

UpDialog::Mode UpDialog::mode() const
{
    return m_mode;
}

void UpDialog::setEnregPosition(bool a)
{
    m_enregistreposition = a;
}

void UpDialog::TuneSize(bool fixh, bool fixw)
{
    int larg    = 0;
    int haut    = 0;
    int r,t,l,b;
    double stages = 0.0;
    QList<QWidget*> listwidg = wdg_buttonswidget->findChildren<QWidget*>();
    if (listwidg.size()>0)
    {
        for (int i=0; i<listwidg.size(); i++)
        {
            if (qobject_cast<UpSmallButton*>(listwidg.at(i)) == Q_NULLPTR && qobject_cast<UpPushButton*>(listwidg.at(i)))
            {
                stages = 0.0;
                break;
            }
            if (qobject_cast<UpPushButton*>(listwidg.at(i)) != Q_NULLPTR)
                stages = 1;
            if (qobject_cast<UpSmallButton*>(listwidg.at(i)) != Q_NULLPTR && stages < 1)
            {
                if (qobject_cast<UpSmallButton*>(listwidg.at(i))->text() =="")
                    stages = 0.7;
                else
                    stages = 1;
            }
        }
    }
    if (stages > 0.0)
        setStageCount(stages);
    layout()    ->getContentsMargins(&r,&t,&l,&b);
    larg        = larg + r + l;
    haut        = haut + t + b;
    for (int i=0; i< layout()->count(); i++)
    {
        if (layout()->itemAt(i)->sizeHint().width() > larg)
            larg += layout()->itemAt(i)->sizeHint().width();
        haut += layout()->itemAt(i)->sizeHint().height();
    }
    haut += (layout()->count()-1) * layout()->spacing();
    if (fixh && fixw)
        setFixedSize(larg, haut);
    else if (fixh && ! fixw)
        setFixedHeight(haut);
    else if (fixw && ! fixh)
        setFixedWidth(larg);
    else
        resize(larg, haut);
}


QSize UpDialog::deltas() const              //! difference between geometry size and all widget's size except mainwidget
{
    QSize sz = sizeForMainWidgetDisplay();
    int wdelta   = geometry().width()  - sz.width();
    int hdelta   = geometry().height() - sz.height();
    return QSize(wdelta, hdelta);
}

QSize UpDialog::sizeForMainWidgetDisplay() const
{
    int             claywidth     = 0;
    QHBoxLayout *hlay = dynamic_cast<QHBoxLayout*>(dlglayout()->itemAt(0));
    if (hlay)
        claywidth = hlay->spacing()*(hlay->count()-1) + hlay->contentsMargins().left() + hlay->contentsMargins().right();

    int width = geometry()          .width()
                - m_globallay       ->contentsMargins().left()
                - m_globallay       ->contentsMargins().right()
                - m_correctionwidth
                - claywidth;
    int height = geometry()         .height()
                 - m_globallay      ->contentsMargins().top()
                 - m_globallay      ->contentsMargins().bottom()
                 - m_globallay      ->spacing()
                 - wdg_buttonslayout->contentsMargins().top()
                 - wdg_buttonslayout->contentsMargins().bottom()
                 - m_stageheight * m_nstages;
    return QSize(width,height);
}


