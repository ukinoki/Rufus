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
#include <QDebug>

UpDialog::UpDialog(QString NomSettings, QString NomPosition, QWidget *parent) : QDialog(parent)
{
    m_enregistreposition    = true;
    m_position              = NomPosition;
    m_nomfichierini         = NomSettings;
    m_settings              = new QSettings(m_nomfichierini, QSettings::IniFormat);
    restoreGeometry(m_settings->value(m_position).toByteArray());
    AjouteLay();
    setStageCount(0);
    m_mode           = NullMode;
}

UpDialog::UpDialog(QWidget *parent) : QDialog(parent)
{
    AjouteLay();
    m_enregistreposition   = false;
}

void UpDialog::AjouteLay()
{
    m_stageheight       = 40;
    wdg_buttonswidget   = new QWidget();
    wdg_buttonslayout   = new QHBoxLayout();
    wdg_buttonslayout   ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    wdg_buttonslayout   ->setContentsMargins(0,10,0,10);
    wdg_buttonslayout   ->setSpacing(10);
    wdg_buttonswidget   ->setLayout(wdg_buttonslayout);
    dlglayout()         ->addWidget(wdg_buttonswidget);
}
void UpDialog::AjouteLayButtons(Buttons Button)
{
    // le Button Cancel est toujours le plus à gauche
    // Close le plus à droite et OK juste avant Close
    if (Button.testFlag(ButtonRecord))
    {
        RecordButton    = new UpSmallButton();
        RecordButton    ->setUpButtonStyle(UpSmallButton::RECORDBUTTON);
        wdg_buttonslayout      ->addWidget(RecordButton);
    }
    if (Button.testFlag(ButtonCancel))
    {
        CancelButton    = new UpSmallButton();
        CancelButton    ->setShortcut(QKeySequence("F12"));
        CancelButton    ->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        wdg_buttonslayout      ->addWidget(CancelButton);
        connect(CancelButton,   &QPushButton::clicked, this, &UpDialog::reject);
    }
    if (Button.testFlag(ButtonPrint))
    {
        PrintButton     = new UpSmallButton();
        PrintButton     ->setUpButtonStyle(UpSmallButton::PRINTBUTTON);
        PrintButton     ->setShortcut(QKeySequence("Meta+P"));
        wdg_buttonslayout      ->addWidget(PrintButton);
    }
    if (Button.testFlag(ButtonSuppr))
    {
        SupprButton     = new UpSmallButton();
        SupprButton     ->setUpButtonStyle(UpSmallButton::SUPPRBUTTON);
        wdg_buttonslayout      ->addWidget(SupprButton);
    }
    if (Button.testFlag(ButtonEdit))
    {
        EditButton     = new UpSmallButton();
        EditButton     ->setUpButtonStyle(UpSmallButton::EDITBUTTON);
        wdg_buttonslayout     ->addWidget(EditButton);
    }
    if (Button.testFlag(ButtonOK))
    {
        OKButton        = new UpSmallButton();
        OKButton        ->setUpButtonStyle(UpSmallButton::STARTBUTTON);
        OKButton        ->setShortcut(QKeySequence("Meta+Return"));
        wdg_buttonslayout      ->addWidget(OKButton);
    }
    if (Button.testFlag(ButtonClose))
    {
        CloseButton     = new UpSmallButton();
        CloseButton     ->setUpButtonStyle(UpSmallButton::CLOSEBUTTON);
        CloseButton     ->setShortcut(QKeySequence("Meta+P"));
        wdg_buttonslayout      ->addWidget(CloseButton);
    }
    if (Button.testFlag(ButtonOups))
    {
        CloseButton     = new UpSmallButton();
        CloseButton     ->setUpButtonStyle(UpSmallButton::OUPSBUTTON);
        wdg_buttonslayout      ->addWidget(CloseButton);
    }
    UpdateTabOrder();
    setStageCount(1);
}

QVBoxLayout* UpDialog::dlglayout()
{
    QVBoxLayout *globallay = dynamic_cast<QVBoxLayout*>(this->layout());
    if (globallay == Q_NULLPTR)
    {
        globallay   = new QVBoxLayout(this);
        globallay   ->setContentsMargins(10,10,10,10);
        globallay   ->setSpacing(0);
    }
    return globallay;
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
 * avec des upsmallbuttons et du texte nbstages = 1
 * et plus si des widgets de plus grande taille dans le widget de buttons
 * à déterminer à chaque fois pifométriquement
 * je sais, c'est du bricolage
 */
void UpDialog::setStageCount(double nbstages)
{
    wdg_buttonswidget->setFixedHeight(int(m_stageheight * nbstages) + wdg_buttonslayout->contentsMargins().bottom() + wdg_buttonslayout->contentsMargins().top());
}

void UpDialog::UpdateTabOrder()
{
    if (wdg_buttonslayout->findChildren<UpSmallButton*>().size()>1)
        for (int i=0; i<wdg_buttonslayout->findChildren<UpSmallButton*>().size()-1; i++)
            setTabOrder(wdg_buttonslayout->findChildren<UpSmallButton*>().at(i),wdg_buttonslayout->findChildren<UpSmallButton*>().at(i+1));
}

void UpDialog::AjouteWidgetLayButtons(QWidget *widg, bool ALaFin)
{
    if (ALaFin)
        wdg_buttonslayout->addWidget(widg);
    else
        wdg_buttonslayout->insertWidget(0,widg);
    UpSmallButton *but = dynamic_cast<UpSmallButton*>(widg);
    if (but != Q_NULLPTR)
    {
        if (but->ButtonStyle() == UpSmallButton::CANCELBUTTON)
            but    ->setShortcut(QKeySequence("F12"));
        else if (but->ButtonStyle() == UpSmallButton::STARTBUTTON)
            but    ->setShortcut(QKeySequence("Meta+Return"));
        else if (but->ButtonStyle() == UpSmallButton::CLOSEBUTTON)
            but    ->setShortcut(QKeySequence("Meta+Return"));
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

void UpDialog::EnregistrePosition()
{
    if (m_enregistreposition)
        m_settings->setValue(m_position, saveGeometry());
}

void UpDialog::closeEvent(QCloseEvent *event)
{
    EnregistrePosition();
    event->accept();
}

void UpDialog::setEnregPosition(bool a)
{
    m_enregistreposition = a;
}

void UpDialog::TuneSize(bool fix)
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
            if (dynamic_cast<UpSmallButton*>(listwidg.at(i)) == Q_NULLPTR && dynamic_cast<UpPushButton*>(listwidg.at(i)))
            {
                stages = 0.0;
                break;
            }
            if (dynamic_cast<UpPushButton*>(listwidg.at(i)) != Q_NULLPTR)
                stages = 1;
            if (dynamic_cast<UpSmallButton*>(listwidg.at(i)) != Q_NULLPTR && stages < 1)
            {
                if (dynamic_cast<UpSmallButton*>(listwidg.at(i))->text() =="")
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
    if (fix)
        setFixedSize(larg,haut);
    else
        resize(larg, haut);
}

