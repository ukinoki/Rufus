#ifndef CLS_APPAREILIMAGERIE_H
#define CLS_APPAREILIMAGERIE_H

#include <QObject>

class AppareilImagerie
{
private:
    QString m_titreexamen;
    QString m_nomappareil;
    QString m_nomdossierechange;

public:
    explicit AppareilImagerie(QString titreexamen = "", QString nomappareil = "", QString nomdossierechange ="")
    {
        m_titreexamen = titreexamen;
        m_nomappareil = nomappareil;
        m_nomdossierechange = nomdossierechange;
    }

    QString titreexamen() const             { return m_titreexamen; }
    QString nomappareil() const             { return m_nomappareil; }
    QString nomdossierechange() const       { return m_nomdossierechange; }
    void settitreexamen(QString txt)        { m_titreexamen = txt; }
    void setnomappareil(QString txt)        { m_nomappareil = txt; }
    void setnomdossierechange(QString txt)  { m_nomdossierechange = txt; }
};

#endif // CLS_APPAREILIMAGERIE_H
