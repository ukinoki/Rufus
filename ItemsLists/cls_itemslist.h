#ifndef ITEMSLIST_H
#define ITEMSLIST_H

#include "QObject"
#include "cls_tiers.h"

class ItemsList : public QObject
{
    Q_OBJECT
public:
    explicit ItemsList(QObject *parent = Q_NULLPTR);

protected:
    template <typename T>
    void add(QMap<int, T*> *m_list, T* item, int id)
    {
        if( m_list->contains(id) )
            return;
        m_list->insert(id, item);
    }
};

#endif // ITEMSLIST_H