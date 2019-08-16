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

#ifndef UPTABLEWIDGET_H
#define UPTABLEWIDGET_H

#include <QTableWidget>
#include <QHeaderView>
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>
#include "upmessagebox.h"
#include "poppler-qt5.h"


class UpTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit UpTableWidget(QWidget *parent = Q_NULLPTR);

    QList<QImage>   AfficheDoc(QMap<QString,QVariant> doc, bool aveczoom = false);
    void            clearSelection();
    void            clearAllRowsExceptHeader();                 //! supprime toutes les rangées d'une table (clearContents() vide le contenu des rangées mais ne supprime pas les rangées)
    int             FixLargeurTotale();
    int             rowNoHiddenCount() const;
    int             FirstRowNoHidden() const;
    int             LastRowNoHidden() const;
    void            setAllRowHeight(int h);
    void            selectRow(int row);
    QByteArray      dropData();

private:
    QByteArray      m_encodedData;

protected:
    void            dropEvent(QDropEvent *) Q_DECL_OVERRIDE;

signals:
    void            dropsignal(QByteArray);
    void            zoom();             // ce signal génère l'affichage de le table dans une fenêtre spécifique en taille maxi
                                        // utilisé pour afficher une image, un document pdf ou une video en grande taille
};

#endif // UPTABLEWIDGET_H
