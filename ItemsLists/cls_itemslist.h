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
    enum ADDTOLIST {AddToList, NoAddToList};    Q_ENUM(ADDTOLIST)
    enum POSITION {Debut, Prec, Suiv, Fin};     Q_ENUM(POSITION)

    static bool update(Item*item, QString field, QVariant newvalue = QVariant());

    /*!
     * \brief ItemsList::clearAll
     * Cette fonction vide un QMap de son contenu et delete les items
     * \param m_map le QMap que l'on veut vider
     */
    template <typename T>
    void clearAll(QMap<int, T*> *m_map)
    {
        for (auto it = m_map->begin(); it != m_map->end();)
        {
            if (it.value() != Q_NULLPTR)
            {
                T* item = const_cast<T*>(it.value());
                delete item;
            }
            it = m_map->erase(it);
        }
        m_map->clear();
    }

    /*! le même avec des QString en key */
    template <typename T>
    void clearAll(QMap<QString, T*> *m_map)
    {
        for (auto it = m_map->begin(); it != m_map->end();)
        {
            if (it.value() != Q_NULLPTR)
            {
                T* item = const_cast<T*>(it.value());
                delete item;
            }
            it = m_map->erase(it);
        }
        m_map->clear();
    }

protected:

    /*!
     * \brief ItemsList::addList
     * Cette fonction rajoute une liste d'items à un QMap
     * Les items dont l'id est déjà présent ne sont pas remplacés
     * \param Item::UPDATE upd - si un item est déjà présent, il n'est pas remplacé mais il est mis à jour si upd = Item::ForceUpdate)
     * \param m_map le QMap que l'on veut vider
     */
    template <typename T>
    void addList(QMap<int, T*> *m_map, QList<T*> *listitems, Item::UPDATE upd = Item::NoUpdate)
    {
        for(auto it = listitems->begin(); it != listitems->end(); )
        {
            T* item = const_cast<T*>(*it);
            if (!add( m_map, item, upd))
            {
                it = listitems->erase(it);
                if (item != Q_NULLPTR)
                    delete item;
            }
            else
                 ++it;
        }
    }

    template <typename T>
    void addList(QMap<QString, T*> *m_map, QList<T*> *listitems, Item::UPDATE upd = Item::NoUpdate)
    {
        for(auto it = listitems->begin(); it != listitems->end(); )
        {
            T* item = const_cast<T*>(*it);
            if (!add( m_map, item, upd))
            {
                it = listitems->erase(it);
                if (item != Q_NULLPTR)
                    delete item;
            }
            else
                 ++it;
        }
    }

/*!
 * \brief ItemsList::epurelist
 * Cette fonction va retirer dans le QMap m_oldMap tous les éléments qui ne sont pas dans la list d'items m_newlist
 * Utile quand on veut réinitialiser une QMAP d'items àpartir d'une nouvelle liste
 * tous les éléments du QMap qui ne sont pas dans la nouvelle liste sont supprimés et délétés
 */
template <typename T>
void epurelist(QMap<int, T*> *m_oldmap, QList<T*> *m_newlist)
{
    if (m_oldmap->size() == 0)
        return;
    QMap<int, T*> m_newmap;
    foreach (T *item, *m_newlist)
        m_newmap.insert(item->id(), item);
    for (auto it = m_oldmap->begin(); it != m_oldmap->end();)
        if (m_newmap.find(it.key()) == m_newmap.constEnd())
        {
            delete  it.value();
            it = m_oldmap->erase(it);
        }
        else
            ++it;
}

/*! le même avec des QString en key */
template <typename T>
void epurelist(QMap<QString, T*> *m_oldmap, QList<T*> *m_newlist)
{
    if (m_oldmap->size() == 0)
        return;
    QMap<QString, T*> m_newmap;
    foreach (T* item, *m_newlist)
        m_newmap.insert(item->stringid(), item);
    for (auto it = m_oldmap->begin(); it != m_oldmap->end();)
        if (m_newmap.find(it.key()) == m_newmap.constEnd())
        {
            delete it.value();
            it = m_oldmap->erase(it);
        }
        else
            ++it;
}



/*!
     * \brief ItemsList::add
     * Cette fonction va ajouter un item dans un QMap.
     * Si un item avec le même id est présent dans le QMap,
        * il n'est pas remplacé mais ses datas sont remplacées par celle de l'item à ajouter
        * l'item à ajouter est deleté
        * la fonction renvoie false
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
            auto it = m_map->find(item->id());
            if (it == m_map->constEnd())
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
            auto it = m_map->find(item->stringid());
            if (it == m_map->constEnd())
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
            table = TBL_DOCSEXTERNES;
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
        if (dynamic_cast<Correspondant*>(item) != Q_NULLPTR)
        {
            table = TBL_CORRESPONDANTS;
            idname = CP_ID_CORRESP;
            loop = true;
            break;
        }
        loop = true;
    }
    if (table != "" && idname != "")
    {
        DataBase::I()->SupprRecordFromTable(item->id(), idname, table);
        m_map->remove(item->id());
        delete item;
        return true;
    }
    return false;
}

};

#endif // ITEMSLIST_H
