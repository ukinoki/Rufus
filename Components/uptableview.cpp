#include "uptableview.h"

UpTableView::UpTableView(QWidget *parent) : QTableView(parent)
{

}

void UpTableView::FixLargeurTotale()
{
    int larg = 0;
    for (int i=0; i < model()->columnCount(); i++)
        if (!isColumnHidden(i))
            larg += columnWidth(i);
    setFixedWidth(larg+2);
}

int UpTableView::rowNoHiddenCount() const
{
    int nrow = 0;
    for (int i=0; i<model()->rowCount(); i++)
        if (!isRowHidden(i))
            nrow += 1;
    return nrow;
}
