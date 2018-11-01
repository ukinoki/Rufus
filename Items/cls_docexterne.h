#ifndef CLS_DOCEXTERNE_H
#define CLS_DOCEXTERNE_H

#include "cls_item.h"

class DocExterne : public Item
{
private:
    int m_idimpression, m_iduser, m_importance, m_idpatient, m_compression, m_useremetteur;
    QString m_typedoc, m_titre, m_soustypedoc, m_textentete, m_textcorps, m_textorigine, m_textpied, m_formatdoc, m_lienversfichier;
    QDateTime m_dateimpression;
    bool m_ald;
    bool m_isAllLoaded = false;

public:
    explicit DocExterne(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int id() const;
    int idpatient() const;
    int iduser() const;
    QString typedoc() const;
    QString soustypedoc() const;
    QString titre() const;
    QString textentete() const;
    QString textcorps() const;
    QString textorigine() const;
    QString textpied() const;
    QDateTime date() const;
    QString lienversfichier() const;
    bool isALD() const;
    int useremetteur() const;
    QString format() const;
    int importance() const;
    int compression() const;
    bool isAllLoaded() const;

    void setimportance(int imptce);
    void setAllLoaded(bool allloaded);
};
#endif // CLS_DOCEXTERNE_H
