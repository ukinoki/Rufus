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

#ifndef DLG_CHOIXCOTATION_H
#define DLG_CHOIXCOTATION_H

#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>

#include "cls_cotations.h"
#include "upcheckbox.h"
#include "updialog.h"
#include "widgetbuttonframe.h"

enum ColonneCotation { ColCotation, ColDescriptif, ColNonOptam, ColOptam, ColPratique };
enum { RoleOphtalmo = Qt::UserRole };   /*!< la ligne relève de l'ophtalmologie */

/*! Filtre de la ligne de recherche : le code de la cotation ou son descriptif, jamais les montants. */
class FiltreCotations : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void setOphtalmoSeule(bool ophtalmo)    {m_ophtalmoseule = ophtalmo; invalidateFilter();}
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
private:
    bool m_ophtalmoseule = true;
};

/*!
 Recherche d'une cotation dans la nomenclature, en remplacement du lien vers le site de la CCAM.
 La liste est celle du QCompleter des cotations : les cotations de référence puis la CCAM entière.
    * 5 colonnes : la cotation, son descriptif, le conventionnel non OPTAM, le conventionnel OPTAM
      et le montant pratiqué ; les deux conventionnels ne s'affichent qu'en France.
    * la ligne de recherche filtre sur le descriptif ; la hauteur des lignes suit ce texte, souvent long.
    * cotation() rend la cotation retenue (double-clic ou OK), nullptr si la fiche a été annulée.
*/

class dlg_choixcotation : public UpDialog
{
    Q_OBJECT
public:
    explicit    dlg_choixcotation(QWidget *parent = nullptr);
    Cotation*   cotation() const    { return m_cotation; }

protected:
    void        showEvent(QShowEvent *event) override;   //!< la hauteur des lignes se calcule une fois la largeur du descriptif connue

private:
    Cotation                *m_cotation = nullptr;      //!< la cotation retenue, nullptr si aucune
    QTimer                  *m_timerhauteurs = nullptr; //!< recalcule les hauteurs une fois le redimensionnement fini
    QStandardItemModel      *m_model    = nullptr;
    FiltreCotations         *m_proxy    = nullptr;      //!< filtre de la ligne de recherche
    QTableView              *wdg_table  = nullptr;
    WidgetButtonFrame       *wdg_buttonframe = nullptr;
    UpCheckBox              *wdg_ophtalmo    = nullptr;   //!< n'afficher que les cotations d'ophtalmologie

    void        RemplitTable();                     //!< une ligne par cotation de référence, puis par code CCAM
    void        RetientCotation(QModelIndex idx);   //!< retient la cotation de la ligne et ferme la fiche
};

#endif // DLG_CHOIXCOTATION_H
