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

#ifndef UPDELEGATE_H
#define UPDELEGATE_H

#include <QStyledItemDelegate>
#include "uplabel.h"
#include "uplineedit.h"

class UpLabel;
class UpLineEdit;

class UpLabelDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    UpLabelDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget*    createEditor    (QWidget* parent,   const QStyleOptionViewItem& option, const QModelIndex &index) const  Q_DECL_OVERRIDE;
    void        setEditorData   (QWidget* editor,   const QModelIndex& index) const Q_DECL_OVERRIDE;
    bool        editorEvent     (QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)  Q_DECL_OVERRIDE;
signals:
    void        focusitem(int a);
};

class UpLineDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    UpLineDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget*    createEditor    (QWidget* parent, const QStyleOptionViewItem&, const QModelIndex &index) const  Q_DECL_OVERRIDE;
    void        setEditorData   (QWidget* editor,   const QModelIndex& index) const Q_DECL_OVERRIDE;
    void        setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const override;
    void        updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool        editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)  Q_DECL_OVERRIDE;

signals:
    void        focusitem(int a);
    void        editingFinished();
    void        textEdited();
};

class TreeViewDelegate : public QStyledItemDelegate
{
private:
    int m_height;
public:
    TreeViewDelegate(QObject *parent = nullptr, int height = -1) : QStyledItemDelegate(parent), m_height(height) {}
    void setHeight(int height) { m_height = height; }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class UpchkDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        const QWidget *widget = option.widget;
        initStyleOption(&opt, index);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);
        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            switch (opt.checkState) {
            case Qt::Unchecked:
                opt.state |= QStyle::State_Off;
                break;
            case Qt::PartiallyChecked:
                opt.state |= QStyle::State_NoChange;
                break;
            case Qt::Checked:
                opt.state |= QStyle::State_On;
                break;
            }
            auto rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
            opt.rect = QStyle::alignedRect(opt.direction, index.data(Qt::TextAlignmentRole).value<Qt::Alignment>(), rect.size(), opt.rect);
            opt.state = opt.state & ~QStyle::State_HasFocus;
            style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, painter, widget);
        } else if (!opt.icon.isNull()) {
            // draw the icon
            QRect iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, widget);
            iconRect = QStyle::alignedRect(opt.direction, index.data(Qt::TextAlignmentRole).value<Qt::Alignment>(), iconRect.size(), opt.rect);
            QIcon::Mode mode = QIcon::Normal;
            if (!(opt.state & QStyle::State_Enabled))
                mode = QIcon::Disabled;
            else if (opt.state & QStyle::State_Selected)
                mode = QIcon::Selected;
            QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            opt.icon.paint(painter, iconRect, opt.decorationAlignment, mode, state);
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        Q_ASSERT(event);
        Q_ASSERT(model);
        // make sure that the item is checkable
        Qt::ItemFlags flags = model->flags(index);
        if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled) || !(flags & Qt::ItemIsEnabled))
            return false;
        // make sure that we have a check state
        QVariant value = index.data(Qt::CheckStateRole);
        if (!value.isValid())
            return false;
        const QWidget *widget = option.widget;
        QStyle *style = option.widget ? widget->style() : QApplication::style();
        // make sure that we have the right event type
        if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick) || (event->type() == QEvent::MouseButtonPress)) {
            QStyleOptionViewItem viewOpt(option);
            initStyleOption(&viewOpt, index);
            QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &viewOpt, widget);
            checkRect = QStyle::alignedRect(viewOpt.direction, index.data(Qt::TextAlignmentRole).value<Qt::Alignment>(), checkRect.size(), viewOpt.rect);
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() != Qt::LeftButton || !checkRect.contains(me->pos()))
                return false;
            if ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonDblClick))
                return true;
        } else if (event->type() == QEvent::KeyPress) {
            if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
                return false;
        } else {
            return false;
        }
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        if (flags & Qt::ItemIsUserTristate)
            state = ((Qt::CheckState)((state + 1) % 3));
        else
            state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
        return model->setData(index, state, Qt::CheckStateRole);
    }
};

#endif // UPDELEGATE_H
