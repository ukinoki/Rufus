#ifndef CLS_FLAGS_H
#define CLS_FLAGS_H

#include "database.h"

            /*! les flags sont utilisés pour contrôler la mise à jour ils pallient l'absence du TCP en cas de on utilisation de RufusAdmin
             *  - de la liste des messages
             *  - de la liste des correspondants
             *  - de la salle d'attente
             * Ils fonctionnent tous sur le même principe.

             * A - Pour les correspondants
                * au démarrage, si le TCP n'est pas utilisé (pas de RufusAdmin ou accès distant), la classe rufus.cpp lit la valeur de MAJFlagMG dans la table rufus.flags (fonction Flags::flagCorrespondants()) et la stocke dans la variale m_flagcorrespondants
                * si le TCP n'est pas utilisé, un timer est lancé pour scruter la valeur du champ MAJFlagMG
                    * 1 - l'utilisateur modifie, supprime ou crée un correspondant par la classe dlg_identificationpatient ou dlg_listecorrespondants
                        * la classe concernée reinitialise la liste des correspondants -> DataBase::I()->loadCorrespondants()
                        * et éxécute Flags::MAJflagCorrespondants()
                        * Flags::MAJflagCorrespondants()
                            * incrémente le champ MAJFlagMG de la table rufus.flags en cas de non utilisation de TCP
                            * émet le signal UpdCorrespondants()
                        * rufus.cpp récupère le signal et
                            . en cas d'utilisation de TCP, envoie le message TCPMSG_MAJCorrespondants vers RufusAdmin qui va le rediffuser vers les autres postes
                            . hors TCP, incrémente la variable m_flagcorrespondants
                            . met à jour les combobox de correspondants
                        * en cas d'utilisation de RufusAdmin, RufusAdmin
                            * récupère et traite le message TCPMSG_MAJCorrespondants
                            * incrémente le champ MAJFlagMG de la table rufus.flags
                    * 2 - Un autre utilisateur a modifié, supprimé ou créé un correspondant
                        * en cas d'utilisation de TCP, rufus.cpp reçoit le message TCPMSG_MAJCorrespondants invitant à la mise à jour de la liste des correspondants
                            * fait cette mise à jour
                            * reconstruit les combobox de correspondants
                        * hors TCP, le timer compare la valeur du champ MAJFlagMG avec celle stockée en mémoire dans la variable m_flagcorrespondants
                            * si la valeur a augmenté, rufus.cpp
                                * fait cette mise à jour
                                * reconstruit les combobox de correspondants

             * B - Pour les messages
                * au démarrage, si le TCP n'est pas utilisé (pas de RufusAdmin ou accès distant), la classe rufus.cpp lit la valeur de MAJFlagMessages dans la table rufus.flags (fonction Flags::flagMessagess()) et la stocke dans la variale m_flagmessages
                * si le TCP n'est pas utilisé, un timer est lancé pour scruter la valeur du champ MAJFlagMessages
                    * 1 - l'utilisateur crée un message par la classe rufus.cpp
                        * rufus.cpp éxécute Flags::MAJflagMessages()
                        * Flags::MAJflagMessages()
                            * incrémente le champ MAJFlagMessages de la table rufus.flags en cas de non utilistation de TCP
                            * émet le signal UpdMessages()
                        * rufus.cpp récupère le signal et
                            . hors TCP, incrémente la variable m_flagcorrespondants
                            . met à jour les messages
                        * en cas d'utilisation de RufusAdmin, RufusAdmin
                            * incrémente le champ MAJFlagMessages de la table rufus.flags
                    * 2 - Un autre utilisateur a créé un message
                        * en cas d'utilisation de TCP, rufus.cpp reçoit le message TCP
                            * met à jour les messages
                        * hors TCP, le timer compare la valeur du champ MAJFlagMessages avec celle stockée en mémoire dans la variable m_flagmessages
                            * si la valeur a augmenté, rufus.cpp met à jour les messages

             * C - Pour la salle d'attente
                * au démarrage, si le TCP n'est pas utilisé (pas de RufusAdmin ou accès distant), la classe rufus.cpp lit la valeur de MAJFlagSalDat dans la table rufus.flags (fonction Flags::flagSalleDAttente()) et la stocke dans la variale m_flagsalledattente
                * si le TCP n'est pas utilisé, un timer est lancé pour scruter la valeur du champ MAJFlagSalDat
                    * 1 - l'utilisateur modifie la salle d'attente
                        * rufus.cpp reconstruit la salle d'attente
                        * et éxécute Flags::MAJflagSalleDAttente()
                        * Flags::MAJflagSalleDAttente()
                            * incrémente le champ MAJFlagSalDat de la table rufus.flags en cas de non utilisation de TCP
                            * émet le signal UpdSalleDAttente()
                        * rufus.cpp récupère le signal et
                            . en cas d'utilisation de TCP, envoie le message TCPMSG_MAJSalAttente vers RufusAdmin qui va le rediffuser vers les autres postes
                            . hors TCP, incrémente la variable m_flagcorrespondants
                            . reconstruit la salle d'attente
                        * en cas d'utilisation de RufusAdmin, RufusAdmin
                            * incrémente le champ MAJFlagSalAttente de la table rufus.flags
                    * 2 - Un autre utilisateur a modifié, supprimé ou créé un correspondant
                        * en cas d'utilisation de TCP, rufus.cpp reçoit le message TCPMSG_MAJSalAttente
                            * reconstruit la salle d'attente
                        * hors TCP, le timer compare la valeur du champ MAJFlagMG avec celle stockée en mémoire dans la variable m_flagcorrespondants
                            * si la valeur a augmenté, rufus.cpp reconstruit la salle d'attente

             * D - Pour les utilisateurs distants
                * si on est en accès accès distant,
                    * au lancement du programme, rufus.cpp éxécute Flags::MAJflagUserDistant() qui incrémente la valeur de MAJFlagUserDistant dans la table rufus.flags
                    * Flags::MAJflagUserDistant()
                        * incrémente le champ MAJFlagUserDistant de la table rufus.flags en cas d'accès distant
                * RufusAdmin scrute le champ MAJFlagUserDistant par le biais d'un timer et stocke sa valeur dans la variale m_flaguserdistant
                    * si la valeur a augmenté, rufusAdmin reconstruit la liste des utilisateurs connectés

            * Quand RufusAdmin est utilisé,
                * seuls les postes distants n'utilisent pas le TCP et mettent donc à jour les champs MAJFlag de la table rufus.flags
                * RufusAdmin scrute ces champs (RufusAdmin::VerifModifsFlags() déclenché par un timer) et déclenche les messages TCP correspondants
                * A l'inverse, quand un poste du réseau local modifie un des 3 items (salle d'attente, correpondants et messages), il en informe RufusAdmin par un message TCP qui va
                    * traiter le message
                    * mettre à jour les champs MJFlag correspondants pour que les postes distants soient informés
            */

class Flags : public QObject

{
    Q_OBJECT
public:
    static Flags            *I();
    explicit                Flags(QObject *parent = Q_NULLPTR);
    ~Flags();

    void                    setTCP(bool tcp);

    int                     flagCorrespondants() const; //!> flag de mise à jour de la liste des corresondants
    int                     flagMessages() const;       //!> flag de vérification des messages
    int                     flagSalleDAttente() const;  //!> flag de mise à jour de la salle d'attente
    int                     flagUserDistant() const;    //!> flag de mise à jour de la liste des utilisateurs distants

    void                    MAJflagCorrespondants();    //!< MAJ du flag de la liste des correspondants
    void                    MAJflagMessages();          //!< MAJ du flag de la messagerie
    void                    MAJFlagSalleDAttente();     //!< MAJ du flag de la salle d'attente
    void                    MAJflagUserDistant();       //!> MAJ du flag de mise à jour de la liste des utilisateurs distants

signals:
    void                    UpdCorrespondants(int);     //!< signal de mise à jour de la liste des correspondants           - l'argument donne la nouvelle valeur du flag
    void                    UpdMessages(int);           //!< signal de mise à jour de la messagerie                         - l'argument donne la nouvelle valeur du flag
    void                    UpdSalleDAttente(int);      //!< signal de mise à jour de la salle d'attente                    - l'argument donne la nouvelle valeur du flag

private:
    static Flags            *instance;
    bool                    m_ok;
    bool                    m_useTCP;
};


#endif // CLS_FLAGS_H
