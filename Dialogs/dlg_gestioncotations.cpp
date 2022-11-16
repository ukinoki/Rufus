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

#include "dlg_gestioncotations.h"

dlg_gestioncotations::dlg_gestioncotations(TypeActe type, Mode mode, QString CodeActe, QWidget *parent) :
    UpDialog(parent)
{
    m_typeacte   = type;
    m_mode       = mode;
    m_ciodeacte   = CodeActe;

    QDoubleValidator *val   = new QDoubleValidator(this);

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    wdg_codeline            = new UpLineEdit(this);
    wdg_tarifoptamline      = new UpLineEdit(this);
    wdg_tarifnooptamline    = new UpLineEdit(this);
    wdg_tarifpratiqueline   = new UpLineEdit(this);
    wdg_tipline             = new UpLineEdit(this);

    wdg_codeline            ->setFixedWidth(150);
    wdg_tarifoptamline      ->setFixedWidth(70);
    wdg_tarifnooptamline    ->setFixedWidth(70);
    wdg_tarifpratiqueline   ->setFixedWidth(70);
    wdg_tipline             ->setFixedWidth(300);

    wdg_codeline            ->setAlignment(Qt::AlignRight);
    wdg_tarifoptamline      ->setAlignment(Qt::AlignRight);
    wdg_tarifnooptamline    ->setAlignment(Qt::AlignRight);
    wdg_tarifpratiqueline   ->setAlignment(Qt::AlignRight);
    wdg_tipline             ->setAlignment(Qt::AlignRight);

    wdg_codewidg            = new QWidget();
    wdg_tarifoptamwidg      = new QWidget();
    wdg_tarifnooptamwidg    = new QWidget();
    wdg_tarifpratiquewidg   = new QWidget();
    wdg_tipwidg             = new QWidget();

    UpLabel *codelabel          = new UpLabel();
    codelabel                   ->setText("Code");
    if (m_typeacte == Association)
        wdg_codeline            ->setValidator(new QRegExpValidator(Utils::rgx_cotation,this));
    QHBoxLayout *codelay        = new QHBoxLayout;
    codelay                     ->insertWidget(0, codelabel);
    codelay                     ->insertSpacerItem(1, new QSpacerItem(10,5));
    codelay                     ->insertWidget(2, wdg_codeline);
    wdg_codeline                ->setEnabled(m_mode==Creation);
    wdg_codeline                ->setMaxLength(15);
    wdg_codeline                ->setText(CodeActe);
    codelay                     ->setContentsMargins(0,0,0,0);
    wdg_codewidg                ->setLayout(codelay);
    dlglayout()                 ->insertWidget(0, wdg_codewidg);

    UpLabel *tiplabel           = new UpLabel();
    tiplabel                    ->setText(TIP);
    QHBoxLayout *tiplay         = new QHBoxLayout;
    tiplay                      ->insertWidget(0, tiplabel);
    tiplay                      ->insertSpacerItem(1, new QSpacerItem(10,5));
    tiplay                      ->insertWidget(2, wdg_tipline);
    wdg_tipline                 ->setMaxLength(75);
    tiplay                      ->setContentsMargins(0,0,0,0);
    wdg_tipwidg                 ->setLayout(tiplay);
    dlglayout()                 ->insertWidget(1, wdg_tipwidg);

    UpLabel *tarifoptamlabel    = new UpLabel();
    tarifoptamlabel             ->setText("Tarif conventionnel OPTAM");
    wdg_tarifoptamline          ->setValidator(val);
    QHBoxLayout *tarifoptamlay  = new QHBoxLayout;
    tarifoptamlay               ->insertWidget(0, tarifoptamlabel);
    tarifoptamlay               ->insertSpacerItem(1, new QSpacerItem(10,5));
    tarifoptamlay               ->insertWidget(2, wdg_tarifoptamline);
    tarifoptamlay               ->setContentsMargins(0,0,0,0);
    wdg_tarifoptamwidg           ->setLayout(tarifoptamlay);
    dlglayout()                 ->insertWidget(2, wdg_tarifoptamwidg);

    UpLabel *tarifnooptamlabel  = new UpLabel();
    tarifnooptamlabel           ->setText("Tarif conventionnel non OPTAM");
    wdg_tarifnooptamline        ->setValidator(val);
    QHBoxLayout *tarifnooptamlay= new QHBoxLayout;
    tarifnooptamlay             ->insertWidget(0, tarifnooptamlabel);
    tarifnooptamlay             ->insertSpacerItem(1, new QSpacerItem(10,5));
    tarifnooptamlay             ->insertWidget(2, wdg_tarifnooptamline);
    tarifnooptamlay             ->setContentsMargins(0,0,0,0);
    wdg_tarifnooptamwidg        ->setLayout(tarifnooptamlay);
    dlglayout()                 ->insertWidget(3, wdg_tarifnooptamwidg);

    UpLabel *tarifpratiquelabel  = new UpLabel();
    tarifpratiquelabel           ->setText("Tarif pratiqué");
    wdg_tarifpratiqueline        ->setValidator(val);
    QHBoxLayout *tarifpratiquelay= new QHBoxLayout;
    tarifpratiquelay             ->insertWidget(0, tarifpratiquelabel);
    tarifpratiquelay             ->insertSpacerItem(1, new QSpacerItem(10,5));
    tarifpratiquelay             ->insertWidget(2, wdg_tarifpratiqueline);
    tarifpratiquelay             ->setContentsMargins(0,0,0,0);
    wdg_tarifpratiquewidg        ->setLayout(tarifpratiquelay);
    dlglayout()                  ->insertWidget(4, wdg_tarifpratiquewidg);

    wdg_tarifoptamwidg              ->setVisible(m_typeacte==Association);
    wdg_tarifnooptamwidg            ->setVisible(m_typeacte==Association);
    wdg_tarifpratiquewidg           ->setVisible((m_typeacte==Association && Datas::I()->users->userconnected()->secteurconventionnel()>1) || m_typeacte==HorsNomenclature);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    OKButton                    ->setEnabled(false);
    dlglayout()->setSpacing(5);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    setWindowTitle(m_typeacte == Association? "Association de codes CCAM ou  autres types d'acte" : "Acte hors nomenclature");

    connect(OKButton,           &QPushButton::clicked,  this,   [=] {if (VerifFiche()) accept();});
    connect(CancelButton,       &QPushButton::clicked,  this,   [=] {reject();});

    connect(wdg_codeline,           &QLineEdit::textEdited, this,   [=] {OKButton->setEnabled(true);});
    connect(wdg_tarifoptamline,     &QLineEdit::textEdited, this,   [=] {OKButton->setEnabled(true);});
    connect(wdg_tarifnooptamline,   &QLineEdit::textEdited, this,   [=] {OKButton->setEnabled(true);});
    connect(wdg_tarifpratiqueline,  &QLineEdit::textEdited, this,   [=] {OKButton->setEnabled(true);});
    connect(wdg_tipline,            &QLineEdit::textEdited, this,   [=] {OKButton->setEnabled(true);});

    QString req = "select distinct " CP_TYPEACTE_COTATIONS " from " TBL_COTATIONS " where " CP_CODECCAM_COTATIONS " = ";
    req += (m_typeacte == Association? "2" : "3");
    if (m_mode == Creation)
    {
        bool ok;
        QList<QVariantList> listtypacte = db->StandardSelectSQL(req, ok);
        if (ok && listtypacte.size()>0)
        {
            for (int i=0; i<listtypacte.size(); i++)
                m_listeactes << listtypacte.at(i).at(0).toString();
            QCompleter *complistactes = new QCompleter(m_listeactes,this);
            complistactes->setCompletionMode(QCompleter::InlineCompletion);
            wdg_codeline->setCompleter(complistactes);
        }
    }
    else
    {
        req = " select " CP_MONTANTOPTAM_COTATIONS ", " CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", " CP_TIP_COTATIONS " from " TBL_COTATIONS
              " where " CP_IDUSER_COTATIONS " = " + QString::number(Datas::I()->users->userconnected()->id()) + " and " CP_TYPEACTE_COTATIONS " = '" + m_ciodeacte + "'";
        bool ok;
        QVariantList listcot = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (ok && listcot.size()>0)
        {
            wdg_tarifoptamline      ->setText(QLocale().toString(listcot.at(0).toDouble(),'f',2));
            wdg_tarifnooptamline    ->setText(QLocale().toString(listcot.at(1).toDouble(),'f',2));
            wdg_tarifpratiqueline   ->setText(QLocale().toString(listcot.at(2).toDouble(),'f',2));
            wdg_tipline             ->setText(listcot.at(3).toString());
        }
    }

}

dlg_gestioncotations::~dlg_gestioncotations()
{
}

bool dlg_gestioncotations::VerifFiche()
{
    bool a = true;
    QString req = "";
    QString msg = tr(("Rensegnements incomplets"));
    switch (m_typeacte) {
    case Association:
        if (wdg_tarifoptamline->text()=="")
        {
           a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif OPTAM"));
           wdg_tarifoptamline->setFocus();
           break;
        }
        if (wdg_tarifnooptamline->text()=="")
        {
           a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif hors OPTAM"));
           wdg_tarifnooptamline->setFocus();
           break;
        }
        if (wdg_tarifpratiqueline->isVisible() && wdg_tarifpratiqueline->text()=="")
        {
           a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif pratiqué"));
           wdg_tarifpratiqueline->setFocus();
           break;
        }
        if (m_mode == Creation)
        {
            req = "select " CP_ID_COTATIONS " from " TBL_COTATIONS " where " CP_TYPEACTE_COTATIONS " = '" + wdg_codeline->text() + "' and " CP_CODECCAM_COTATIONS " = 2 and "
                    CP_IDUSER_COTATIONS " = " + QString::number(Datas::I()->users->userconnected()->id());
            bool ok;
            QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
            if (ok && actdata.size()>0)
            {
                a = false;
                UpMessageBox::Watch(this, msg, tr("Cet acte est déjà enregistré"));
            }
        }
        if (!a)
           break;
        if (m_mode == Creation)
        {
            req = "insert into " TBL_COTATIONS " (" CP_TYPEACTE_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", " CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", "
                   CP_CODECCAM_COTATIONS ", " CP_IDUSER_COTATIONS ", " CP_TIP_COTATIONS ") VALUES ('"
                    + wdg_codeline->text() + "', "
                    + QString::number(QLocale().toDouble(wdg_tarifoptamline->text())) + ", "
                    + QString::number(QLocale().toDouble(wdg_tarifnooptamline->text())) + ", "
                    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", 2, "
                    + QString::number(Datas::I()->users->userconnected()->id()) + ", '"
                    + Utils::correctquoteSQL(wdg_tipline->text()) + "')";
        }
        else
        {
            req = "update " TBL_COTATIONS " set "
                  CP_MONTANTOPTAM_COTATIONS " = "       + QString::number(QLocale().toDouble(wdg_tarifoptamline->text())) + ", " +
                  CP_MONTANTNONOPTAM_COTATIONS " = "    + QString::number(QLocale().toDouble(wdg_tarifnooptamline->text())) + ", " +
                  CP_MONTANTPRATIQUE_COTATIONS " = "    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", " +
                  CP_TIP_COTATIONS " = '"               + Utils::correctquoteSQL(wdg_tipline->text()) + "' " +
                  " where"
                  CP_IDUSER_COTATIONS " = "             + QString::number(Datas::I()->users->userconnected()->id()) +
                  " and " CP_TYPEACTE_COTATIONS " = '"  + wdg_codeline->text() + "'";
        }
        break;
    case HorsNomenclature:
        if (QLocale().toDouble(wdg_tarifpratiqueline->text())==0.0)
        {
            a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif pratiqué"));
           wdg_tarifpratiqueline->setFocus();
           break;
        }
        if (m_mode == Creation)
        {
            req = "select " CP_ID_COTATIONS " from " TBL_COTATIONS " where " CP_TYPEACTE_COTATIONS " = '" + wdg_codeline->text() + "' and "
                    CP_CODECCAM_COTATIONS " = 3 and " CP_IDUSER_COTATIONS " = " + QString::number(Datas::I()->users->userconnected()->id());
            bool ok;
            QVariantList cotdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
            if (ok && cotdata.size()>0)
            {
                a = false;
                UpMessageBox::Watch(this, msg, tr("Cet acte est déjà enregistré"));
            }
        }
        if (!a)
        {
           break;
        }
        if (m_mode == Creation)
        {
            req = "insert into " TBL_COTATIONS " (" CP_TYPEACTE_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", " CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", "
                  CP_CODECCAM_COTATIONS ", " CP_IDUSER_COTATIONS ", " CP_TIP_COTATIONS ") VALUES ('"
                    + wdg_codeline->text() + "', "
                    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", "
                    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", "
                    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", 3, "
                    + QString::number(Datas::I()->users->userconnected()->id()) + ", '"
                    + Utils::correctquoteSQL(wdg_tipline->text()) + "')";
        }
        else
        {
            req = "update " TBL_COTATIONS " set "
                  CP_MONTANTOPTAM_COTATIONS " = "       + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", " +
                  CP_MONTANTNONOPTAM_COTATIONS " = "    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", " +
                  CP_MONTANTPRATIQUE_COTATIONS " = "    + QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text())) + ", " +
                  CP_TIP_COTATIONS " = '"               + Utils::correctquoteSQL(wdg_tipline->text()) + "' " +
                  " where"
                  CP_IDUSER_COTATIONS " = "             + QString::number(Datas::I()->users->userconnected()->id()) +
                  " and " CP_TYPEACTE_COTATIONS " = '"  + wdg_codeline->text() + "'";
        }
        break;
    }
    if (!a)
        return false;
    db->StandardSQL(req);
    return true;
}
