#include "cls_item.h"

#include <QDateTime>

Item::Item(QObject *parent) : QObject(parent)
{

}

void Item::setStringProperty(QJsonObject data, QList<QString> props)
{
    QList<QString>::iterator i;
    for(i = props.begin(); i != props.end(); ++i)
        setDataString(data, *i);
}
void Item::setDataString(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toString() );
}

void Item::setIntProperty(QJsonObject data, QList<QString> props)
{
    QList<QString>::iterator i;
    for(i = props.begin(); i != props.end(); ++i)
        setDataInt(data, *i);
}
void Item::setDataInt(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toInt() );
}

void Item::setDataDouble(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toDouble() );
}

void Item::setBoolProperty(QJsonObject data, QList<QString> props)
{
    QList<QString>::iterator i;
    for(i = props.begin(); i != props.end(); ++i)
        setDataBool(data, *i);
}
void Item::setDataBool(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
        this->property(thisKey).setValue( data[key].toBool() );
}

void Item::setDataDateTime(QJsonObject data, QString key)
{
    const char* thisKey = ("m_"+key).toStdString().c_str();
    if( data.contains(key) )
    {
        int64_t time = data[key].toInt();
        if( time <= 0 )
            return;
        QDateTime dt;
        dt.setMSecsSinceEpoch( time );
        this->property(thisKey).setValue( dt );
    }
}
