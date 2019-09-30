#ifndef CLS_MESUREDIVERS_H
#define CLS_MESUREDIVERS_H

#include <QObject>
#include <QTime>

class Pachy : public QObject
{
    Q_OBJECT
public:
    explicit Pachy() {}
    enum Mode {Echo, Optique, OCT, NoMesure};      Q_ENUM(Mode)
private:
    int m_pachyOD       = 0;
    int m_pachyOG       = 0;
    Mode m_modemesure   = NoMesure;
    bool m_cleandatas   = true;

public:
    int pachyOD() const         { return m_pachyOD; }
    int pachyOG() const         { return m_pachyOG; }
    Mode modemesure() const     { return m_modemesure; }

    void setpachyOD(int pa)         { m_pachyOD = pa; m_cleandatas = false; }
    void setpachyOG(int pa)         { m_pachyOG = pa; m_cleandatas = false; }
    void setmodemesure(Mode mode)   { m_modemesure = mode; m_cleandatas = false; }

    bool isdataclean()              { return m_cleandatas; }
    void cleandatas()
    {
        m_pachyOD       = 0;
        m_pachyOG       = 0;
        m_modemesure    = NoMesure;
        m_cleandatas    = true;
    }

    bool isEqual(Pachy *other) const
    {
        return  (m_pachyOD         == other->pachyOD()
                && m_pachyOG       == other->pachyOG()
                && m_modemesure    == other->modemesure());
    }

    bool isDifferent(Pachy *other) const
    {
        return !(isEqual(other));
    }

    static Mode     ConvertMesure(QString Mesure)
    {
        if (Mesure == "O")  return Optique;
        if (Mesure == "T")  return OCT;
        if (Mesure == "E")  return Echo;
        return  NoMesure;
    }

    static QString  ConvertMesure(Pachy::Mode mode)
    {
        switch (mode) {
        case Optique:   return "O";
        case OCT:       return "T";
        case Echo:      return "E";
        default:        return "";
        }
    }

};

class Tono : public QObject
{
    Q_OBJECT
public:
    explicit Tono() {}
    enum Mode {Air, Aplanation, Autre, NoMesure};      Q_ENUM(Mode)
private:
    int m_TOD           = 0;
    int m_TOG           = 0;
    QTime m_heuremesure = QTime();
    Mode m_modemesure   = NoMesure;
    bool m_cleandatas   = true;

public:
    int TOD() const                 { return m_TOD; }
    int TOG() const                 { return m_TOG; }
    QTime heuremesure() const       { return m_heuremesure; }
    Mode modemesure() const         { return m_modemesure; }

    void setTOD(int to)             { m_TOD = to; m_cleandatas = false; }
    void setTOG(int to)             { m_TOG = to; m_cleandatas = false; }
    void setheuremesure(QTime time) { m_heuremesure = time; m_cleandatas = false; }
    void setmodemesure(Mode mode)   { m_modemesure = mode; m_cleandatas = false; }

    bool isdataclean()              { return m_cleandatas; }
    void cleandatas()
    {
        m_TOD           = 0;
        m_TOG           = 0;
        m_heuremesure   = QTime();
        m_modemesure    = NoMesure;
        m_cleandatas    = true;
    }

    bool isEqual(Tono *other) const
    {
        return  (m_TOD              == other->TOD()
                && m_TOG            == other->TOG()
                && m_heuremesure    == other->heuremesure()
                && m_modemesure     == other->modemesure());
    }

    bool isDifferent(Tono *other) const
    {
        return !(isEqual(other));
    }

    static Mode     ConvertMesure(QString Mesure)
    {
        if (Mesure == "Air")        return Air;
        if (Mesure == "Aplanation") return Aplanation;
        if (Mesure == "Autre")      return Autre;
        return  NoMesure;
    }

    static QString  ConvertMesure(Tono::Mode mode)
    {
        switch (mode) {
        case Air:           return "Air";
        case Aplanation:    return "Aplanation";
        case Autre:         return "Autre";
        default: return "";
        }
    }
};

#endif // CLS_MESUREDIVERS_H
