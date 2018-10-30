#ifndef UPHEADERVIEW_H
#define UPHEADERVIEW_H

#include <QObject>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QTableWidget>
#include <QScrollBar>
#include <QEvent>

class UpHeaderView : public QHeaderView
{
    /*
     * Cette classe permet de faire des span sur les entêtes horizontaux de qtableview et qtablewidget
     * exemple dans dlg_gestionusers.cpp fonction ReconstruitListeLieuxExercice()
     */
    Q_OBJECT
public:
    explicit    UpHeaderView(QHeaderView *header);
    void        reDim(int section, int debut, int fin); //permet à la secion section de recouvrir les colonnes depuis debut à fin

protected:
    bool        eventFilter(QObject *o, QEvent *e);

private:
    void        updateOffset();
    int         getSectionSizes(int first, int second);
    QHeaderView *mainHeader;
};


#endif // UPHEADERVIEW_H
