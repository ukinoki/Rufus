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

#include "dlg_identificationcommercial.h"

/*!
 * \brief dlg_identificationcommercial::dlg_identificationcommercial
 * la fiche peut-être appelée de 2 façons
        * Mode Creation totale depuis dlg_listecommercials
            * la fiche est lancée en mode Creation, tous les champs sont vierges et les paramètres COMMERCIAL et Manufacturer sont nullptr
        * Mode Modification
            * la fiche est lancée depuis dlg_listecommercials pour modifier les paramètres dun COMMERCIAL existant
            * le paramètre COMMERCIAL est donc celui de l'implant à modifier
            * la paramètre Manufacturer peut-être laissé à Q_NULLPTR
            * toutes les zones de saisie sont renseignées avec las paramètres déjà connus de l'implant
 * \param mode
 * \param commercial
 * \param man
 * \param parent
 */
dlg_identificationcommercial::dlg_identificationcommercial(enum Mode mode, Item *itm, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionIdentCOMMERCIAL", parent)
{
    setWindowModality(Qt::WindowModal);
    m_mode = mode;
    switch (m_mode) {
    case Modification:
        m_currentcommercial = qobject_cast<Commercial*>(itm);
        if (m_currentcommercial)
            m_currentmanufacturer = Datas::I()->manufacturers->getById(m_currentcommercial->idmanufacturer());
        break;
    case Creation:
        m_currentmanufacturer= qobject_cast<Manufacturer*>(itm);
    }
    if (!m_currentcommercial)
        m_mode = Creation;
    if (m_currentcommercial)
        m_currentmanufacturer = Datas::I()->manufacturers->getById(m_currentcommercial->idmanufacturer());

    //! FABRICANT
    if (m_manufacturersmodel != Q_NULLPTR)
        delete m_manufacturersmodel;
    m_manufacturersmodel = new QStandardItemModel(this);
    foreach (Manufacturer *man, *Datas::I()->manufacturers->manufacturers())
        if (man->isactif()) {
            QList<QStandardItem *> items;
            //qDebug() << man->nom() << man->id();
            UpStandardItem *itemman = new UpStandardItem(man->nom(), man);
            UpStandardItem *itemid = new UpStandardItem(QString::number(man->id()), man);
            items << itemman << itemid;
            m_manufacturersmodel->appendRow(items);
        }
    m_manufacturersmodel->sort(0);

    QHBoxLayout *choixManufacturerLay    = new QHBoxLayout();
    UpLabel* lblManufacturer    = new UpLabel;
    lblManufacturer             ->setText(tr("Fabricant"));
    wdg_manufacturercombo       = new QComboBox();
    wdg_manufacturercombo       ->setEditable(true);
    wdg_manufacturercombo       ->lineEdit()->setAlignment(Qt::AlignCenter);
    wdg_manufacturercombo       ->lineEdit()->setFocusPolicy(Qt::NoFocus);
    for (int i=0; i< m_manufacturersmodel->rowCount(); ++i)
    {
        wdg_manufacturercombo   ->addItem(m_manufacturersmodel->item(i,0)->text());         //! le nom du fabricant
        wdg_manufacturercombo   ->setItemData(i, m_manufacturersmodel->item(i,1)->text());       //! l'id en data
    }
    if (m_currentmanufacturer)
        wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
    else
        m_currentmanufacturer = Datas::I()->manufacturers->getById(wdg_manufacturercombo->itemData(0).toInt());
    choixManufacturerLay        ->addWidget(lblManufacturer);
    choixManufacturerLay        ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixManufacturerLay        ->addWidget(wdg_manufacturercombo);
    choixManufacturerLay        ->setSpacing(5);
    choixManufacturerLay        ->setContentsMargins(0,0,0,0);

    //! NOM
    QHBoxLayout *choixnomLay    = new QHBoxLayout();
    UpLabel* lblnom             = new UpLabel;
    lblnom                      ->setText(tr("Nom"));
    wdg_nomcommercialline       = new UpLineEdit();
    wdg_nomcommercialline       ->setFixedSize(QSize(250,28));
    wdg_nomcommercialline       ->setValidator(new QRegularExpressionValidator(Utils::rgx_rx));
    wdg_nomcommercialline       ->setMaxLength(40);
    choixnomLay                 ->addWidget(lblnom);
    choixnomLay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixnomLay                 ->addWidget(wdg_nomcommercialline);
    choixnomLay                 ->setSpacing(5);
    choixnomLay                 ->setContentsMargins(0,0,0,0);

    //! PRENOM
    QHBoxLayout *choixprenomLay = new QHBoxLayout();
    UpLabel* lblprenom          = new UpLabel;
    lblprenom                   ->setText(tr("Prénom"));
    wdg_prenomcommercialline    = new UpLineEdit();
    wdg_prenomcommercialline    ->setFixedSize(QSize(250,28));
    wdg_prenomcommercialline    ->setValidator(new QRegularExpressionValidator(Utils::rgx_rx));
    wdg_prenomcommercialline    ->setMaxLength(40);
    choixprenomLay              ->addWidget(lblprenom);
    choixprenomLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixprenomLay              ->addWidget(wdg_prenomcommercialline);
    choixprenomLay              ->setSpacing(5);
    choixprenomLay              ->setContentsMargins(0,0,0,0);

    //! STATUT
    QHBoxLayout *choixstatutLay = new QHBoxLayout();
    UpLabel* lblstatut          = new UpLabel;
    lblstatut                   ->setText(tr("Statut"));
    wdg_statutcommercialline    = new UpLineEdit();
    wdg_statutcommercialline    ->setFixedSize(QSize(250,28));
    wdg_statutcommercialline    ->setValidator(new QRegularExpressionValidator(Utils::rgx_rx));
    wdg_statutcommercialline    ->setMaxLength(40);
    choixstatutLay              ->addWidget(lblstatut);
    choixstatutLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixstatutLay              ->addWidget(wdg_statutcommercialline);
    choixstatutLay              ->setSpacing(5);
    choixstatutLay              ->setContentsMargins(0,0,0,0);

    //! TELEPHONE
    QHBoxLayout *choixtelephoneLay = new QHBoxLayout();
    UpLabel* lbltelephone          = new UpLabel;
    lbltelephone                   ->setText(tr("Téléphone"));
    wdg_telephonecommercialline    = new UpLineEdit();
    wdg_telephonecommercialline    ->setFixedSize(QSize(250,28));
    wdg_telephonecommercialline    ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone));
    wdg_telephonecommercialline    ->setMaxLength(17);
    choixtelephoneLay              ->addWidget(lbltelephone);
    choixtelephoneLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixtelephoneLay              ->addWidget(wdg_telephonecommercialline);
    choixtelephoneLay              ->setSpacing(5);
    choixtelephoneLay              ->setContentsMargins(0,0,0,0);

    //! MAIL
    QHBoxLayout *choixmailLay = new QHBoxLayout();
    UpLabel* lblmail          = new UpLabel;
    lblmail                   ->setText(tr("Mail"));
    wdg_mailcommercialline    = new UpLineEdit();
    wdg_mailcommercialline    ->setFixedSize(QSize(250,28));
    wdg_mailcommercialline    ->setValidator(new QRegularExpressionValidator(Utils::rgx_mail));
    wdg_mailcommercialline    ->setMaxLength(40);
    choixmailLay              ->addWidget(lblmail);
    choixmailLay              ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    choixmailLay              ->addWidget(wdg_mailcommercialline);
    choixmailLay              ->setSpacing(5);
    choixmailLay              ->setContentsMargins(0,0,0,0);

    dlglayout()   ->insertLayout(0, choixmailLay);
    dlglayout()   ->insertLayout(0, choixtelephoneLay);
    dlglayout()   ->insertLayout(0, choixstatutLay);
    dlglayout()   ->insertLayout(0, choixprenomLay);
    dlglayout()   ->insertLayout(0, choixnomLay);
    dlglayout()   ->insertLayout(0, choixManufacturerLay);

    dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    dlglayout()   ->setSpacing(5);

    QList <QWidget*> ListTab;
    ListTab << wdg_manufacturercombo << wdg_nomcommercialline << wdg_prenomcommercialline << wdg_statutcommercialline << wdg_telephonecommercialline << wdg_mailcommercialline;
    for (int i = 0; i<ListTab.size()-1 ; i++ )
    {
        setTabOrder(ListTab.at(i), ListTab.at(i+1));
        ListTab.at(i)->installEventFilter(this);
    }

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    wdg_nomcommercialline->setFocus();

    QFont font = qApp->font();
    font.setBold(true);
    //font.setItalic(true);
    //font.setPointSize(font.pointSize()+2);
    foreach (UpLabel *lbl, findChildren<UpLabel*>())
        lbl->setFont(font);
    foreach (UpLineEdit *line, findChildren<UpLineEdit*>())
        line->setAlignment(Qt::AlignRight);

    AfficheDatascommercial(m_currentcommercial);
    OKButton->setEnabled(false);
    OKButton->setText(tr("Enregistrer"));
    CancelButton->setText(tr("Annuler"));
    setStageCount(1);
    installEventFilter(this);
}

bool dlg_identificationcommercial::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key()==Qt::Key_Return  && keyEvent->modifiers() == Qt::NoModifier) || keyEvent->key() == Qt::Key_Enter)
            return QWidget::focusNextChild();
    }
    return QWidget::eventFilter(obj, event);
}

void dlg_identificationcommercial::connectSignals()
{
    connect(OKButton, &QPushButton::clicked, this, &dlg_identificationcommercial::OKpushButtonClicked);
    connect (wdg_manufacturercombo, QOverload<int>::of(&QComboBox::currentIndexChanged),   this,   [&](int id) {
                                                                                                                     int idman = wdg_manufacturercombo->itemData(id).toInt();
                                                                                                                     m_currentmanufacturer = Datas::I()->manufacturers->getById(idman);
                                                                                                                     EnableOKpushButton();
                                                                                                                 });
    connect (wdg_nomcommercialline,        &QLineEdit::textEdited,                         this,   &dlg_identificationcommercial::EnableOKpushButton);
    connect (wdg_prenomcommercialline,     &QLineEdit::textEdited,                         this,   &dlg_identificationcommercial::EnableOKpushButton);
    connect (wdg_statutcommercialline,     &QLineEdit::textEdited,                         this,   &dlg_identificationcommercial::EnableOKpushButton);
    connect (wdg_telephonecommercialline,  &QLineEdit::textEdited,                         this,   &dlg_identificationcommercial::EnableOKpushButton);
    connect (wdg_mailcommercialline,       &QLineEdit::textEdited,                         this,   &dlg_identificationcommercial::EnableOKpushButton);
}

void dlg_identificationcommercial::disconnectSignals()
{
     OKButton->disconnect();
     wdg_manufacturercombo->disconnect();
     wdg_nomcommercialline->disconnect();
     wdg_prenomcommercialline->disconnect();
     wdg_statutcommercialline->disconnect();
     wdg_telephonecommercialline->disconnect();
     wdg_mailcommercialline->disconnect();
}


/*--------------------------------------------------------------------------------------------
-- Afficher la fiche de l'implant
--------------------------------------------------------------------------------------------*/
void dlg_identificationcommercial::AfficheDatascommercial(Commercial *commercial)
{
    if (m_mode == Modification)
    {
        if (commercial == Q_NULLPTR)
            return;
        disconnectSignals();
        m_currentcommercial = commercial;
        if (m_currentmanufacturer)
            wdg_manufacturercombo   ->setCurrentIndex(wdg_manufacturercombo->findData(m_currentmanufacturer->id()));
        if (m_mode == Modification)
        {
            wdg_nomcommercialline       ->setText(m_currentcommercial->nom());
            wdg_prenomcommercialline    ->setText(m_currentcommercial->prenom());
            wdg_statutcommercialline    ->setText(m_currentcommercial->statut());
            wdg_telephonecommercialline ->setText(m_currentcommercial->telephone());
            wdg_mailcommercialline      ->setText(m_currentcommercial->mail());
        }
    }
    connectSignals();
}

void dlg_identificationcommercial:: EnableOKpushButton()
{
    bool a  = wdg_nomcommercialline->text() != ""
           && wdg_manufacturercombo->currentData().toInt()>0;
    OKButton            ->setEnabled(a);
    OKButton->setShortcut(a? QKeySequence("Meta+Return") : QKeySequence());
}

void dlg_identificationcommercial::OKpushButtonClicked()
{
    if (wdg_nomcommercialline->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas indiqué le nom!"));
        wdg_nomcommercialline->setFocus();
        return;
    }

    m_listbinds[CP_NOM_COM]             = wdg_nomcommercialline->text();
    m_listbinds[CP_PRENOM_COM]          = wdg_prenomcommercialline->text();
    m_listbinds[CP_STATUT_COM]          = wdg_statutcommercialline->text();
    m_listbinds[CP_TELEPHONE_COM]       = wdg_telephonecommercialline->text();
    m_listbinds[CP_MAIL_COM]            = wdg_mailcommercialline->text();
    m_listbinds[CP_IDMANUFACTURER_COM]  = m_currentmanufacturer->id();
    if (m_mode == Creation)
        m_currentcommercial = Datas::I()->commercials->CreationCommercial(m_listbinds);
    else if (m_mode == Modification)
        DataBase::I()->UpdateTable(TBL_COMMERCIALS, m_listbinds, " where " CP_ID_COM " = " + QString::number(m_currentcommercial->id()),tr("Impossible de modifier le dossier"));
    accept();
}

