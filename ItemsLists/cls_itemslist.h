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
        {
            it.next();
            if (it.value() != Q_NULLPTR)
                delete it.value();
        }
        m_map->clear();
    }

    /*! le même avec des QString en key */
    template <typename T>
    void clearAll(QMap<QString, T*> *m_map)
    {
        QMapIterator<QString, T*> it(*m_map);
        while (it.hasNext())
        {
            it.next();
            if (it.value() != Q_NULLPTR)
                delete it.value();
        }
        m_map->clear();
    }

    template <typename T>
    void addList(QMap<int, T*> *m_map, QList<T*> listitems, Item::UPDATE upd = Item::NoUpdate)
    {
        for(auto it = listitems.begin(); it != listitems.end(); )
        {
            T* item = const_cast<T*>(*it);
            if (!add( m_map, item, upd))
            {
                it = listitems.erase(it);
                if (item != Q_NULLPTR)
                    delete item;
            }
            else
                 ++it;
        }
    }

    template <typename T>
    void addList(QMap<QString, T*> *m_map, QList<T*> listitems, Item::UPDATE upd = Item::NoUpdate)
    {
        for(auto it = listitems.begin(); it != listitems.end(); )
        {
            T* item = const_cast<T*>(*it);
            if (!add( m_map, item, upd))
            {
                it = listitems.erase(it);
                if (item != Q_NULLPTR)
                    delete item;
            }
            else
                 ++it;
        }
    }


protected:

/*!
     * \brief ItemsList::add
     * Cette fonction va ajouter un item dans un QMap
     * \param m_map le QMap dans lequel on veut ajouter l'item
     * \param item l'item que l'on veut ajouter
     * \param Item::UPDATE - si ForceUpdate, force l'update de l'item s'il est déjà présent dans le QMap
     * \return true si l'item est ajouté
     * \return false si l'item est un Q_NULLPTR
     * \return false si l'item est déjà présent dans le QMap et delete l'item passé en paramètre dans ce cas
     */
template <typename T>
bool add(QMap<int, T*> *m_map, T* item, Item::UPDATE upd = Item::NoUpdate)
{
    if (item == Q_NULLPTR)
        return false;
    if( m_map->contains(item->id()) )
    {
        if (upd == Item::ForceUpdate)
        {
            typename QMap<int, T*>::const_iterator it = m_map->find(item->id());
            if (it.value() == Q_NULLPTR)
            {
                m_map->insert(it.key(), item);
                return true;
            }
            else
                it.value()->setData(item->datas());
        }
        return false;
    }
    m_map->insert(item->id(), item);
    return true;
}

/*! le même avec des QString en key */
template <typename T>
bool add(QMap<QString, T*> *m_map, T* item, Item::UPDATE upd = Item::NoUpdate)
{
    if (item == Q_NULLPTR)
        return false;
    if( m_map->contains(item->stringid()) )
    {
        if (upd == Item::ForceUpdate)
        {
            typename QMap<QString, T*>::const_iterator it = m_map->find(item->stringid());
            if (it.value() == Q_NULLPTR)
            {
                m_map->insert(it.key(), item);
                return true;
            }
            else
                it.value()->setData(item->datas());
        }
        return false;
    }
    m_map->insert(item->stringid(), item);
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

/*! le même avec des QString en key */
template <typename T>
void remove(QMap<QString, T*> *m_map, T* item)
{
    if (item == Q_NULLPTR)
        return;
    m_map->remove(item->stringid());
    delete item;
}

/*!
     * \brief ItemsList::Supprime
     * Cette fonction va supprimer un item passé en paramètre dans un QMap
     * \param m_map le QMap dans lequel on veut supprimer l'item
     * \param item l'item que l'on veut supprimer
     * \return true si l'item est supprimé
     * \return false si l'item est un Q_NULLPTR
*/
template <typename T>
static bool Supprime(QMap<int, T*> *m_map, T* item)
{
    if (item == Q_NULLPTR)
        return false;
    QString table (""), idname ("");
    bool loop = false;
    while (!loop)
    {
        if (dynamic_cast<Acte*>(item) != Q_NULLPTR)
        {
            table = TBL_ACTES;
            idname = CP_IDACTE_ACTES;
            loop = true;
            break;
        }
        if (dynamic_cast<Banque*>(item)!= Q_NULLPTR)
        {
            table = TBL_BANQUES;
            idname = CP_IDBANQUE_BANQUES;
            loop = true;
            break;
        }
        if (dynamic_cast<Compte*>(item)!= Q_NULLPTR)
        {
            table = TBL_COMPTES;
            idname = CP_IDCOMPTE_COMPTES;
            loop = true;
            break;
        }
        if (dynamic_cast<Depense*>(item)!= Q_NULLPTR)
        {
            table = TBL_DEPENSES;
            idname = CP_IDDEPENSE_DEPENSES;
            loop = true;
            break;
        }
        if (dynamic_cast<DocExterne*>(item) != Q_NULLPTR)
        {
            table = TBL_IMPRESSIONS;
            idname = CP_IDIMPRESSION_IMPRESSIONS;
            loop = true;
            break;
        }
        if (dynamic_cast<PatientEnCours*>(item) != Q_NULLPTR)
        {
            table = TBL_SALLEDATTENTE;
            idname = CP_IDPAT_SALDAT;
            loop = true;
            break;
        }
        if (dynamic_cast<Refraction*>(item) != Q_NULLPTR)
        {
            table = TBL_REFRACTIONS;
            idname = CP_ID_REFRACTIONS;
            loop = true;
            break;
        }
        loop = true;
    }
    if (table != "" && idname != "")
    {
        DataBase::I()->SupprRecordFromTable(item->id(), idname, table);
        m_map->remove(item->id());
        return true;
    }
    return false;
}

};

#endif // ITEMSLIST_H
