#ifndef CLS_ITEMSLIST_H
#define CLS_ITEMSLIST_H

#include "QObject"
#include "cls_item.h"

class ItemsList : public QObject
{
    Q_OBJECT
public:
    explicit ItemsList(QObject *parent = Q_NULLPTR);
    enum ADDTOLIST {AddToList, NoAddToList};
    enum POSITION {Debut, Prec, Suiv, Fin};
    /*!
     * \brief ItemsList::clearAll
     * Cette fonction vide un QMap de son contenu et delete les items
     * \param m_map le QMap que l'on veut vider
     */
    template <typename T>
    void clearAll(QMap<int, T*> *m_map)
    {
        for(typename QMap<int, T*>::const_iterator it = m_map->constBegin(); it != m_map->constEnd(); ++it)
            delete it.value();
        m_map->clear();
    }

protected:

    /*!
     * \brief ItemsList::add
     * Cette fonction va ajouter un item et sa key spassé en paramètre dans un QMap
     * \param m_map le QMap dnas lequel on veut ajouter l'item
     * \param id l'id de l'item que l'on veut ajouter en key du QMap
     * \param item l'item que l'on veut ajouter
     * \return true si l'item est ajouté
     * \return false si l'item est un Q_NULLPTR
     * \return false si l'item est déjà présent dans le QMap et delete l'item dans ce cas
     */
    template <typename T>
    bool add(QMap<int, T*> *m_map, int id, T* item)
    {
        if (item == Q_NULLPTR)
            return false;
        if( m_map->contains(id) )
        {
            delete item;
            return false;
        }
        m_map->insert(id, item);
        return true;
    }
    template <typename T>
    bool add(QMap<QString, T*> *m_map, QString stringid, T* item)
    {
        if (item == Q_NULLPTR)
            return false;
        if( m_map->contains(stringid) )
        {
            delete item;
            return false;
        }
        m_map->insert(stringid, item);
        return true;
    }


    /*!
     * \brief ItemsList::remove
     * Cette fonction va retirer un item d'un QMap
     * \param m_map le QMap dans lequel on veut retirer l'item
     * \param item l'e patient'item que l'on veut ajouter
     */
    template <typename T>
    void remove(QMap<int, T*> *m_map, T* item)
    {
        if (item == Q_NULLPTR)
            return;
        m_map->remove(item->id());
        delete item;
    }
    template <typename T>
    void remove(QMap<QString, T*> *m_map, T* item)
    {
        if (item == Q_NULLPTR)
            return;
        m_map->remove(item->stringid());
        delete item;
    }
};

#endif // ITEMSLIST_H
