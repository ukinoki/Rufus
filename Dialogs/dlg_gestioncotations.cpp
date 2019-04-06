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

#include "dlg_gestioncotations.h"

dlg_gestioncotations::dlg_gestioncotations(dlg_gestioncotations::TypeActe type, dlg_gestioncotations::Mode mode, QString CodeActe, QWidget *parent) :
    UpDialog(parent)
{
    gTypeActe   = type;
    gMode       = mode;
    gCodeActe   = CodeActe;

    db                      = DataBase::getInstance();
    gidUser                 = db->getUserConnected()->id();
    gSecteurUser            = db->getUserConnected()->getSecteur();
    QDoubleValidator *val   = new QDoubleValidator(this);

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    codeline            = new UpLineEdit(this);
    tarifoptamline      = new UpLineEdit(this);
    tarifnooptamline    = new UpLineEdit(this);
    tarifpratiqueline   = new UpLineEdit(this);
    tipline             = new UpLineEdit(this);

    codeline            ->setFixedWidth(150);
    tarifoptamline      ->setFixedWidth(70);
    tarifnooptamline    ->setFixedWidth(70);
    tarifpratiqueline   ->setFixedWidth(70);
    tipline             ->setFixedWidth(300);

    codeline            ->setAlignment(Qt::AlignRight);
    tarifoptamline      ->setAlignment(Qt::AlignRight);
    tarifnooptamline    ->setAlignment(Qt::AlignRight);
    tarifpratiqueline   ->setAlignment(Qt::AlignRight);
    tipline             ->setAlignment(Qt::AlignRight);

    codewidg            = new QWidget();
    tarifoptamwidg      = new QWidget();
    tarifnooptamwidg    = new QWidget();
    tarifpratiquewidg   = new QWidget();
    tipwidg             = new QWidget();

    UpLabel *codelabel          = new UpLabel();
    codelabel                   ->setText("Code");
    if (gTypeActe == Association)
        codeline                ->setValidator(new QRegExpValidator(Utils::rgx_cotation,this));
    QHBoxLayout *codelay        = new QHBoxLayout;
    codelay                     ->insertWidget(0, codelabel);
    codelay                     ->insertSpacerItem(1, new QSpacerItem(10,5));
    codelay                     ->insertWidget(2, codeline);
    codeline                    ->setEnabled(gMode==Creation);
    codeline                    ->setMaxLength(15);
    codeline                    ->setText(CodeActe);
    codelay                     ->setContentsMargins(0,0,0,0);
    codewidg                    ->setLayout(codelay);
    dlglayout()                 ->insertWidget(0, codewidg);

    UpLabel *tiplabel           = new UpLabel();
    tiplabel                    ->setText("Tip");
    QHBoxLayout *tiplay         = new QHBoxLayout;
    tiplay                      ->insertWidget(0, tiplabel);
    tiplay                      ->insertSpacerItem(1, new QSpacerItem(10,5));
    tiplay                      ->insertWidget(2, tipline);
    tipline                     ->setMaxLength(75);
    tiplay                      ->setContentsMargins(0,0,0,0);
    tipwidg                     ->setLayout(tiplay);
    dlglayout()                 ->insertWidget(1, tipwidg);

    UpLabel *tarifoptamlabel    = new UpLabel();
    tarifoptamlabel             ->setText("Tarif conventionnel OPTAM");
    tarifoptamline              ->setValidator(val);
    QHBoxLayout *tarifoptamlay  = new QHBoxLayout;
    tarifoptamlay               ->insertWidget(0, tarifoptamlabel);
    tarifoptamlay               ->insertSpacerItem(1, new QSpacerItem(10,5));
    tarifoptamlay               ->insertWidget(2, tarifoptamline);
    tarifoptamlay               ->setContentsMargins(0,0,0,0);
    tarifoptamwidg              ->setLayout(tarifoptamlay);
    dlglayout()                 ->insertWidget(2, tarifoptamwidg);

    UpLabel *tarifnooptamlabel  = new UpLabel();
    tarifnooptamlabel           ->setText("Tarif conventionnel non OPTAM");
    tarifnooptamline            ->setValidator(val);
    QHBoxLayout *tarifnooptamlay= new QHBoxLayout;
    tarifnooptamlay             ->insertWidget(0, tarifnooptamlabel);
    tarifnooptamlay             ->insertSpacerItem(1, new QSpacerItem(10,5));
    tarifnooptamlay             ->insertWidget(2, tarifnooptamline);
    tarifnooptamlay               ->setContentsMargins(0,0,0,0);
    tarifnooptamwidg            ->setLayout(tarifnooptamlay);
    dlglayout()                 ->insertWidget(3, tarifnooptamwidg);

    UpLabel *tarifpratiquelabel  = new UpLabel();
    tarifpratiquelabel           ->setText("Tarif pratiqué");
    tarifpratiqueline            ->setValidator(val);
    QHBoxLayout *tarifpratiquelay= new QHBoxLayout;
    tarifpratiquelay             ->insertWidget(0, tarifpratiquelabel);
    tarifpratiquelay             ->insertSpacerItem(1, new QSpacerItem(10,5));
    tarifpratiquelay             ->insertWidget(2, tarifpratiqueline);
    tarifpratiquelay               ->setContentsMargins(0,0,0,0);
    tarifpratiquewidg            ->setLayout(tarifpratiquelay);
    dlglayout()                  ->insertWidget(4, tarifpratiquewidg);

    tarifoptamwidg              ->setVisible(gTypeActe==Association);
    tarifnooptamwidg            ->setVisible(gTypeActe==Association);
    tarifpratiquewidg           ->setVisible((gTypeActe==Association && gSecteurUser>1) || gTypeActe==HorsNomenclature);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()->setSpacing(5);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    setWindowTitle(gTypeActe == Association? "Association de codes CCAM ou  autres types d'acte" : "Acte hors nomenclature");

    connect(OKButton,       &QPushButton::clicked,  this, [=] {if (VerifFiche()) accept();});
    connect(CancelButton,   &QPushButton::clicked,  this, [=] {reject();});

    QString req = "select distinct typeacte from " NOM_TABLE_COTATIONS " where CCAM = ";
    req += (gTypeActe == Association? "2" : "3");

    if (gMode == Creation)
    {
        bool ok;
        QList<QVariantList> listtypacte = db->StandardSelectSQL(req, ok);
        if (ok && listtypacte.size()>0)
        {
            for (int i=0; i<listtypacte.size(); i++)
                glistActes << listtypacte.at(i).at(0).toString();
            QCompleter *complistactes = new QCompleter(glistActes,this);
            complistactes->setCompletionMode(QCompleter::InlineCompletion);
            codeline->setCompleter(complistactes);
        }
    }
    else
    {
        req = " select montantoptam, montantnonoptam, montantpratique, tip from " NOM_TABLE_COTATIONS
              " where idUser = " + QString::number(gidUser) + " and TypeActe = '" + gCodeActe + "'";
        bool ok;
        QVariantList listcot = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (ok && listcot.size()>0)
        {
            tarifoptamline      ->setText(QLocale().toString(listcot.at(0).toDouble(),'f',2));
            tarifnooptamline    ->setText(QLocale().toString(listcot.at(1).toDouble(),'f',2));
            tarifpratiqueline   ->setText(QLocale().toString(listcot.at(2).toDouble(),'f',2));
            tipline             ->setText(listcot.at(3).toString());
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
    switch (gTypeActe) {
    case Association:
        if (tarifoptamline->text()=="")
        {
           a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif OPTAM"));
           tarifoptamline->setFocus();
           break;
        }
        if (tarifnooptamline->text()=="")
        {
           a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif hors OPTAM"));
           tarifnooptamline->setFocus();
           break;
        }
        if (tarifpratiqueline->isVisible() && tarifpratiqueline->text()=="")
        {
           a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif pratiqué"));
           tarifpratiqueline->setFocus();
           break;
        }
        if (gMode == Creation)
        {
            req = "select idcotation from " NOM_TABLE_COTATIONS " where typeacte = '" + codeline->text() + "' and CCAM = 2 and idUser = " + QString::number(gidUser);
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
        if (gMode == Creation)
        {
            req = "insert into " NOM_TABLE_COTATIONS " (TypeACte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, idUser, tip) VALUES ('"
                    + codeline->text() + "', "
                    + QString::number(QLocale().toDouble(tarifoptamline->text())) + ", "
                    + QString::number(QLocale().toDouble(tarifnooptamline->text())) + ", "
                    + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", 2, "
                    + QString::number(gidUser) + ", '"
                    + Utils::correctquoteSQL(tipline->text()) + "')";
        }
        else
        {
            req = "update " NOM_TABLE_COTATIONS " set "
                  " MontantOPTAM = "    + QString::number(QLocale().toDouble(tarifoptamline->text())) + ", " +
                  " MontantNonOPTAM = " + QString::number(QLocale().toDouble(tarifnooptamline->text())) + ", " +
                  " MontantPratique = " + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", " +
                  " tip = '"            + Utils::correctquoteSQL(tipline->text()) + "' " +
                  " where"
                  " idUser = "          + QString::number(gidUser) +
                  " and TypeActe = '"   + codeline->text() + "'";
        }
        break;
    case HorsNomenclature:
        if (QLocale().toDouble(tarifpratiqueline->text())==0.0)
        {
            a = false;
           UpMessageBox::Watch(this, msg, tr("Vous avez oublié de mentionner le tarif pratiqué"));
           tarifpratiqueline->setFocus();
           break;
        }
        if (gMode == Creation)
        {
            req = "select idcotation from " NOM_TABLE_COTATIONS " where typeacte = '" + codeline->text() + "' and CCAM = 3 and idUser = " + QString::number(gidUser);
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
        if (gMode == Creation)
        {
            req = "insert into " NOM_TABLE_COTATIONS " (TypeACte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, idUser) VALUES ('"
                    + codeline->text() + "', "
                    + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", "
                    + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", "
                    + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", 3, "
                    + QString::number(gidUser) + ", '"
                    + Utils::correctquoteSQL(tipline->text()) + "')";
        }
        else
        {
            req = "update " NOM_TABLE_COTATIONS " set "
                  " MontantOPTAM = "    + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", " +
                  " MontantNonOPTAM = " + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", " +
                  " MontantPratique = " + QString::number(QLocale().toDouble(tarifpratiqueline->text())) + ", " +
                  " tip = '"            + Utils::correctquoteSQL(tipline->text()) + "' " +
                  " where"
                  " idUser = "          + QString::number(gidUser) +
                  " and TypeActe = '"   + codeline->text() + "'";
        }
        break;
    }
    if (!a)
        return false;
    db->StandardSQL(req);
    return true;
}
