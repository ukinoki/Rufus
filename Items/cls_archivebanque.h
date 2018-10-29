#ifndef CLS_ARCHIVE_H
#define CLS_ARCHIVE_H

#include <QObject>
#include "cls_item.h"

class Archive : public Item
{
private:
    int m_idligne, m_idcompte;
    int m_iddepense, m_idrecette, m_idrecettespeciale, m_idremisecheque;
    QDate m_lignedate, m_lignedateconsolidation;
    QString m_lignelibelle, m_lignetypeoperation;
    int m_idarchive;
    double m_montant;

public:
    explicit Archive(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int id() const;
    int idcompte() const;
    int iddepense() const;
    int idrecette() const;
    int idrecettespeciale() const;
    int idremisecheque() const;
    QDate lignedate() const;
    QString lignelibelle() const;
    double montant() const;
    QString lignetypeoperation() const;
    QDate lignedateconsolidation() const;
    int idarchive() const;
};


class Archives
{
private:
    QMap<int, Archive*> m_archives; //!< la liste des archives
public:
    explicit Archives();
    QMap<int, Archive *> archives() const;
    void addArchive(Archive *archive);
    void addArchive(QList<Archive*> listarchives);
};

#endif // CLS_ARCHIVE_H
