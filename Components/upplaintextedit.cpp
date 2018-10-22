#include "upplaintextedit.h"

UpPlainTextEdit::UpPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    ValeurAvant = "";
    ValeurApres = "";
    Champ       = "";
    Table       = "";
}

UpPlainTextEdit::~UpPlainTextEdit()
{

}

void UpPlainTextEdit::setValeurAvant(QString valprec)
{
    ValeurAvant = valprec;
}

QString UpPlainTextEdit::getValeurAvant() const
{
    return ValeurAvant;
}

void UpPlainTextEdit::setValeurApres(QString valpost)
{
    ValeurApres = valpost;
}

QString UpPlainTextEdit::getValeurApres() const
{
    return ValeurApres;
}

void UpPlainTextEdit::setChampCorrespondant(QString champcorrespondant)
{
    Champ = champcorrespondant;
}

QString UpPlainTextEdit::getChampCorrespondant() const
{
    return Champ;
}

void UpPlainTextEdit::setTableCorrespondant(QString tablecorrespondant)
{
    Table = tablecorrespondant;
}

QString UpPlainTextEdit::getTableCorrespondant() const
{
    return Table;
}
