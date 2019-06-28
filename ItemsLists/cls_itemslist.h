#ifndef CLS_ITEMSLIST_H
#define CLS_ITEMSLIST_H

#include <QMapIterator>
#include "QObject"
#include "cls_acte.h"
#include "cls_docexterne.h"
#include "cls_patientencours.h"
#include "cls_posteconnecte.h"
#include "macros.h"
#include "database.h"
#include "upmessagebox.h"

class ItemsList : public QObject
{
    Q_OBJECT
public:
    explicit ItemsList(QObject *parent = Q_NULLPTR);
    enum ADDTOLIST {AddToList, NoAddToList};
    enum POSITION {Debut, Prec, Suiv, Fin};

    static bool update(Item*item, QString field, QVariant newvalue = QVariant());

    /*!
     * \brief ItemsList::clearAll
     * Cette fonction vide un QMap de son contenu et delete les items
     * \param m_map le QMap que l'on veut vider
     */
    template <typename T>
    void clearAll(QMap<int, T*> *m_map)
    {
        QMapIterator<int, T*> it(*m_map);
        while (it.hasNext())
            delete it.next().value();
        m_map->clear();
    }

    /*! le même avec des QString en key */
    template <typename T>
    void clearAll(QMap<QString, T*> *m_map)
    {
        QMapIterator<QString, T*> it(*m_map);
        while (it.hasNext())
            delete it.next().value();
        m_map->clear();
    }

protected:

    /*!
     * \brief ItemsList::add
     * Cette fonction va ajouter un item et sa key spassé en paramètre dans un QMap
     * \param m_map le QMap dnas lequel on veut ajouter l'item
     * \param id l'id de l'item que l'on veut ajouter en key du QMap
     * \param item l'item que l'on veut ajouter
     * \param Item::UPDATE - si ForceUpdate, force l'update de l'item s'il est déjà présent dans le QMap
     * \return true si l'item est ajouté
     * \return false si l'item est un Q_NULLPTR
     * \return false si l'item est déjà présent dans le QMap et delete l'item dans ce cas
     */
    template <typename T>
    bool add(QMap<int, T*> *m_map, int id, T* item, Item::UPDATE upd = Item::NoUpdate)
    {
        if (item == Q_NULLPTR)
            return false;
        if( m_map->contains(id) )
        {
            if (upd == Item::ForceUpdate)
            {
                typename QMap<int, T*>::const_iterator it = m_map->find(id);
                if (it != m_map->constEnd())
                    it.value()->setData(item->datas());
            }
            delete item;
            return false;
        }
        m_map->insert(id, item);
        return true;
    }

    /*! le même avec des QString en key */
    template <typename T>
    bool add(QMap<QString, T*> *m_map, QString stringid, T* item, Item::UPDATE upd = Item::NoUpdate)
    {
        if (item == Q_NULLPTR)
            return false;
        if( m_map->contains(stringid) )
        {
            if (upd == Item::ForceUpdate)
            {
                typename QMap<QString, T*>::const_iterator it = m_map->find(stringid);
                if (it != m_map->constEnd())
                    it.value()->setData(item->datas());
            }
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
     * \param item l'item que l'on veut retirer
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

    /*! le même avec des QString en key */
    void remove(QMap<QString, T*> *m_map, T* item)
    {
        if (item == Q_NULLPTR)
            return;
        m_map->remove(item->stringid());
        delete item;
    }
};

#endif // ITEMSLIST_H
