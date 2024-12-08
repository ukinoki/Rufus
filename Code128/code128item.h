#ifndef CODE128ITEM_H
#define CODE128ITEM_H

#include <QGraphicsItem>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include "code128.h"


class Code128Item : public QGraphicsItem
{
    bool                        m_TextVisible;
    float                       m_Width;
    float                       m_Height;
    QString                     m_Text;
    Code128::BarCode            m_Code;
    int                         m_CodeLength;
    bool                        m_HighDPI;

public:
    Code128Item();

    /*! SETTERS & GETTERS */
    void setHeight( float height )          { m_Height = height; }
    void setHighDPI(bool highDPI)           { m_HighDPI = highDPI; }
    void setTextVisible(bool visible)       { m_TextVisible = visible; }
    void setWidth(float width)              { m_Width = width; }
    QRectF boundingRect() const             { return QRectF(0,0, m_Width, m_Height); }

    void setText ( const QString & text );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget);
};
#endif // CODE128ITEM_H
