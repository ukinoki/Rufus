#ifndef CLS_REFRACTIONDEVICE_H
#define CLS_REFRACTIONDEVICE_H

#include "cls_item.h"

class RefractionDevice : public Item
{
public:
    RefractionDevice();
    enum Type {
        AppNoneDev     = 0x0,
        FrontoDev      = 0x1,
        AutorefDev     = 0x2,
        RefracteurDev  = 0x4,
        TonometreDev   = 0x8,
    };  Q_ENUM(Type)

    explicit RefractionDevice(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString brand() const;
    void setBrand(const QString &newModel);
    QString model() const;
    void setModel(const QString &newModel);
    QString comment() const;
    void setComment(const QString &newComment);
    bool hasCOM() const;
    void setHasCOM(bool newHasCOM);
    bool hasLAN() const;
    void setHasLAN(bool newHasLAN);
    bool isrecognized() const;
    void setIsrecognized(bool newIsrecognized);
    int likedevice() const;
    void setLikedevice(int newLikedevice);
    RefractionDevice::Type type() const;
    void setType(Type newType);
    bool isFronto()     { return m_type == FrontoDev; }
    bool isAutoref()    { return m_type == AutorefDev; }
    bool isRefracteur() { return m_type == RefracteurDev; }
    bool isTono()       { return m_type == TonometreDev; }

private:
    QString m_brand     = QString();
    QString m_model     = QString();
    QString m_comment   = QString();
    bool m_hasCOM       = true;
    bool m_hasLAN       = true;
    bool m_isrecognized = true;
    int m_likedevice    = 0;
    Type m_type         = AppNoneDev;
};

#endif // CLS_REFRACTIONDEVICE_H
