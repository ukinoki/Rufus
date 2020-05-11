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
* Cette fonction vide un QMap de son contenu et delete les item
* \param m_map le QMap que l'on veut vider
*/
template <typename K, typename T>
static void clearAll(QMap<K, T*> *m_map)
{
    qDeleteAll(*m_map);
    m_map->clear();
}

protected:

    bool m_ok;
/*!
* \brief ItemsList::addList
* Cette fonction rajoute une liste d'items à un QMap
* Les items dont l'id est déjà présent ne sont pas remplacés
* \param Item::UPDATE upd - si l'id d'un item est déjà présent, il n'est pas remplacé mais il est mis à jour si upd = Item::Update)
* \param m_map le QMap que l'on veut vider
*/
template <typename K, typename T>
void addList(QMap<K, T*> *m_map, QList<T*> *listitems, Item::UPDATE upd = Item::Update)
{
    for (int i=0; i<listitems->size();)
        if (!add( m_map, listitems->at(i), upd))
            listitems->removeAt(i);                 //! on ne delete pas l'item, cela a été fait par la méthode add
        else
            ++i;
}

/*!
 * \brief ItemsList::epurelist
 * Cette fonction va retirer dans le QMap m_oldmap tous les éléments dont l'id ne figure pas dans la liste d'items m_newlist
 * Utile quand on veut réinitialiser une QMap d'items à partir d'une nouvelle liste
 * tous les éléments du QMap dont l'id ne figure pas dans la nouvelle liste sont supprimés et délétés
 */
template <typename T>
void epurelist(QMap<int, T*> *m_oldmap, const QList<T*> *m_newlist)
{
    if (m_oldmap->size() == 0)
        return;
    if (m_newlist->size() == 0)
        clearAll(m_oldmap);
    else {
    QMap<int, T*> m_newmap;
    foreach (T *item, *m_newlist)
        m_newmap.insert(item->id(), item);
    for (auto it = m_oldmap->begin(); it != m_oldmap->end();)
        if (m_newmap.find(it.key()) == m_newmap.constEnd())
        {
            T* item = const_cast<T*>(it.value());
//            T* item = m_oldmap->take(it.key()); ne marche pas
            if (item != Q_NULLPTR)
                delete item;
            it = m_oldmap->erase(it);
        }
        else
            ++it;
    }
}

/*! le même avec des QString en key */
template <typename T>
void epurelist(QMap<QString, T*> *m_oldmap, const QList<T*> *m_newlist)
{
    if (m_oldmap->size() == 0)
        return;
    if (m_newlist->size() == 0)
        clearAll(m_oldmap);
    else {
    QMap<QString, T*> m_newmap;
    foreach (T* item, *m_newlist)
        m_newmap.insert(item->stringid(), item);
    for (auto it = m_oldmap->begin(); it != m_oldmap->end();)
        if (m_newmap.find(it.key()) == m_newmap.constEnd())
        {
            T* item = const_cast<T*>(it.value());
//            T* item = m_oldmap->take(it.key()); ne marche pass
            if (item != Q_NULLPTR)
                delete item;
            it = m_oldmap->erase(it);
        }
        else
            ++it;
    }
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
* \return false dans le cas contraire (l'item est un Q_NULLPTR ou l'item est déjà présent dans le QMap -> delete l'item passé en paramètre dans ce dernier cas)
*/
template <typename T>
bool add(QMap<int, T*> *m_map, T* item, Item::UPDATE upd = Item::NoUpdate)
{
    bool itemadded = false;
    if (item != Q_NULLPTR)
    {
        auto it = m_map->find(item->id());
        itemadded = (it == m_map->cend());
        if (!itemadded)
        {
            if (it.value())
            {
                if (upd == Item::Update)
                    it.value()->setData(item->datas());
                if (it.value() != item)
                    delete item;
            }
            else
                m_map->insert(item->id(), item);
        }
        else
            m_map->insert(item->id(), item);
    }
    return itemadded;
}

/*! le même avec des QString en key */
template <typename T>
bool add(QMap<QString, T*> *m_map, T* item, Item::UPDATE upd = Item::NoUpdate)
{
    bool itemadded = false;
    if (item != Q_NULLPTR)
    {
        auto it = m_map->find(item->stringid());
        itemadded = (it == m_map->cend());
        if (!itemadded)
        {
            if (it.value())
            {
                if (upd == Item::Update)
                    it.value()->setData(item->datas());
                if (it.value() != item)
                    delete item;
            }
            else
                m_map->insert(item->stringid(), item);
        }
        else
            m_map->insert(item->stringid(), item);
    }
    return itemadded;
}


public:
/*!
 * \brief ItemsList::remove
 * Cette fonction va retirer un item d'un QMap
 * \param m_map le QMap dans lequel on veut retirer l'item
 * \param item l'item que l'on veut retirer
*/
template <typename T>
static void remove(QMap<int, T*> *m_map, const T* item)
{
    if (item == Q_NULLPTR)
        return;
    auto it = m_map->find(item->id());
    if (it != m_map->cend())
        delete m_map->take(it.key());
    if (item != Q_NULLPTR)
        delete item;
}
template <typename T>
static void remove(QMap<QString, T*> *m_map, const T* item)
{
    if (item == Q_NULLPTR)
        return;
    auto it = m_map->find(item->stringid());
    if (it != m_map->cend())
        it = m_map->erase(it);
    delete item;
}

protected:
/*!
     * \brief ItemsList::Supprime
     * Cette fonction va supprimer un item passé en paramètre dans un QMap et retirer son enregistrement correpondant dans la base de données
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
            idname = CP_ID_ACTES;
            loop = true;
            break;
        }
        if (dynamic_cast<Banque*>(item)!= Q_NULLPTR)
        {
            table = TBL_BANQUES;
            idname = CP_ID_BANQUES;
            loop = true;
            break;
        }
        if (dynamic_cast<Compte*>(item)!= Q_NULLPTR)
        {
            table = TBL_COMPTES;
            idname = CP_ID_COMPTES;
            loop = true;
            break;
        }
        if (dynamic_cast<Depense*>(item)!= Q_NULLPTR)
        {
            table = TBL_DEPENSES;
            idname = CP_ID_DEPENSES;
            loop = true;
            break;
        }
        if (dynamic_cast<DocExterne*>(item) != Q_NULLPTR)
        {
            table = TBL_DOCSEXTERNES;
            idname = CP_ID_DOCSEXTERNES;
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
        if (dynamic_cast<LigneCompte*>(item) != Q_NULLPTR)
        {
            table = TBL_LIGNESCOMPTES;
            idname = CP_ID_LIGNCOMPTES;
            loop = true;
            break;
        }
        if (dynamic_cast<SessionOperatoire*>(item) != Q_NULLPTR)
        {
            table = TBL_SESSIONSOPERATOIRES;
            idname = CP_ID_SESSIONOPERATOIRE;
            loop = true;
            break;
        }
        if (dynamic_cast<Intervention*>(item) != Q_NULLPTR)
        {
            table = TBL_LIGNESPRGOPERATOIRES;
            idname = CP_ID_LIGNPRGOPERATOIRE;
            loop = true;
            break;
        }
        if (dynamic_cast<TypeIntervention*>(item) != Q_NULLPTR)
        {
            table = TBL_TYPESINTERVENTIONS;
            idname = CP_ID_TYPINTERVENTION;
            loop = true;
            break;
        }
        if (dynamic_cast<Manufacturer*>(item) != Q_NULLPTR)
        {
            table = TBL_MANUFACTURERS;
            idname = CP_ID_MANUFACTURER;
            loop = true;
            break;
        }
        if (dynamic_cast<IOL*>(item) != Q_NULLPTR)
        {
            table = TBL_IOLS;
            idname = CP_ID_IOLS;
            loop = true;
            break;
        }
        if (dynamic_cast<Tiers*>(item) != Q_NULLPTR)
        {
            table = TBL_TIERS;
            idname = CP_ID_TIERS;
            loop = true;
            break;
        }
        if (dynamic_cast<Commercial*>(item) != Q_NULLPTR)
        {
            table = TBL_COMMERCIALS;
            idname = CP_ID_COM;
            loop = true;
            break;
        }
        if (dynamic_cast<CommentLunet*>(item) != Q_NULLPTR)
        {
            table = TBL_COMMENTAIRESLUNETTES;
            idname = CP_ID_COMLUN;
            loop = true;
            break;
        }
        if (dynamic_cast<MotCle*>(item) != Q_NULLPTR)
        {
            table = TBL_MOTSCLES;
            idname = CP_ID_MOTCLE;
            loop = true;
            break;
        }
        if (dynamic_cast<Site*>(item) != Q_NULLPTR)
        {
            table = TBL_LIEUXEXERCICE;
            idname = CP_ID_SITE;
            loop = true;
            break;
        }
        if (dynamic_cast<Impression*>(item) != Q_NULLPTR)
        {
            table = TBL_IMPRESSIONS;
            idname = CP_ID_IMPRESSIONS;
            loop = true;
            break;
        }
        if (dynamic_cast<DossierImpression*>(item) != Q_NULLPTR)
        {
            table = TBL_DOSSIERSIMPRESSIONS;
            idname = CP_ID_DOSSIERIMPRESSIONS;
            loop = true;
            break;
        }
        loop = true;
    }
    if (table == "")
        UpMessageBox::Watch(Q_NULLPTR, tr("Suppression impossible - l'item n'est pas reconnu"));
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
