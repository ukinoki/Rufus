#ifndef UPTABLEVIEW_H
#define UPTABLEVIEW_H

#include <QTableView>

class UpTableView : public QTableView
{
    Q_OBJECT
public:
    explicit UpTableView(QWidget *parent = Q_NULLPTR);
    void    FixLargeurTotale();
    int     rowNoHiddenCount();
};

#endif // UPTABLEVIEW_H
