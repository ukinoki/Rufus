/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CLS_MESUREDIVERS_H
#define CLS_MESUREDIVERS_H

#include "cls_mesure.h"
#include "cls_refraction.h"

class Pachy : public Mesure
{
    Q_OBJECT
public:
    explicit Pachy() {}
    enum Mode {Echo, Optique, OCT, NoMesure};      Q_ENUM(Mode)
private:
    int m_pachyOD       = 0;
    int m_pachyOG       = 0;
    Mode m_modemesure   = NoMesure;

public:
    int pachyOD() const         { return m_pachyOD; }
    int pachyOG() const         { return m_pachyOG; }
    Mode modemesure() const     { return m_modemesure; }

    void setpachyOD(int pa)         { m_pachyOD = pa; m_cleandatas = false; m_isnullOD = false; }
    void setpachyOG(int pa)         { m_pachyOG = pa; m_cleandatas = false; m_isnullOD = false; }
    void setmodemesure(Mode mode)   { m_modemesure = mode; m_cleandatas = false; }

    void cleandatas(Refraction::Oeil cote = Refraction::Les2)
    {
        switch (cote) {
        case Refraction::Les2:
            m_pachyOD       = 0;
            m_pachyOG       = 0;
            m_modemesure    = NoMesure;
            m_cleandatas    = true;
            m_isnullOD      = true;
            m_isnullOG      = true;
            break;
        case Refraction::Droit:
            m_pachyOD       = 0;
            m_isnullOD      = true;
            if (m_isnullOG)
                m_cleandatas = true;
            break;
        case Refraction::Gauche:
            m_pachyOG       = 0;
            m_isnullOG      = true;
            if (m_isnullOD)
                m_cleandatas = true;
            break;
        }
    }

    bool isEqual(Pachy *other) const
    {
        if (m_isnullOD & !m_isnullOG)
            return (other               ->isnullLOD()
                    && !other           ->isnullLOG()
                    && m_pachyOG        == other->pachyOG()
                    && m_modemesure     == other->modemesure());
        else if (m_isnullOG && !m_isnullOD)
            return ( other              ->isnullLOG()
                     && !other          ->isnullLOD()
                     && m_pachyOD       == other->pachyOD()
                     && m_modemesure    == other->modemesure());
        return  (m_pachyOD              == other->pachyOD()
                && m_pachyOG            == other->pachyOG()
                && m_modemesure         == other->modemesure());
    }

    bool isDifferent(Pachy *other) const
    {
        return !(isEqual(other));
    }

    static Mode     ConvertMesure(QString Mesure)
    {
        if (Mesure == OPTIQUE_PACHY)    return Optique;
        if (Mesure == OCT_PACHY)        return OCT;
        if (Mesure == ECHO_PACHY)       return Echo;
        return  NoMesure;
    }

    static QString  ConvertMesure(Pachy::Mode mode)
    {
        switch (mode) {
        case Optique:   return OPTIQUE_PACHY;
        case OCT:       return OCT_PACHY;
        case Echo:      return ECHO_PACHY;
        default:        return "";
        }
    }

};

class Tono : public Mesure
{
    Q_OBJECT
public:
    explicit Tono() {}
    enum Mode {Air, Aplanation, Autre, NoMesure};      Q_ENUM(Mode)
private:
    int m_TOD           = 0;
    int m_TOG           = 0;
    QDateTime m_timemesure = QDateTime();
    Mode m_modemesure   = NoMesure;

public:
    int TOD() const                 { return m_TOD; }
    int TOG() const                 { return m_TOG; }
    QDateTime timemesure() const    { return m_timemesure; }
    Mode modemesure() const         { return m_modemesure; }

    void setTOD(int to)                 { m_TOD = to; m_cleandatas = false; m_isnullOD = false; }
    void setTOG(int to)                 { m_TOG = to; m_cleandatas = false; m_isnullOD = false; }
    void settimeemesure(QDateTime time) { m_timemesure = time; m_cleandatas = false; }
    void setmodemesure(Mode mode)       { m_modemesure = mode; m_cleandatas = false; }

    void cleandatas(Refraction::Oeil cote = Refraction::Les2)
    {
        switch (cote) {
        case Refraction::Les2:
            m_TOD           = 0;
            m_TOG           = 0;
            m_timemesure    = QDateTime();
            m_modemesure    = NoMesure;
            m_cleandatas    = true;
            m_isnullOD      = true;
            m_isnullOG      = true;
            break;
        case Refraction::Droit:
            m_TOD           = 0;
            m_isnullOD      = true;
            if (m_isnullOG)
                m_cleandatas = true;
            break;
        case Refraction::Gauche:
            m_TOG           = 0;
            m_isnullOG      = true;
            if (m_isnullOD)
                m_cleandatas = true;
            break;
        }
    }

    bool isEqual(Tono *other) const
    {
        if (m_isnullOD & !m_isnullOG)
            return (other           ->isnullLOD()
                    && !other       ->isnullLOG()
                    && m_TOG        == other->TOG()
                    && m_modemesure == other->modemesure());
        else if (m_isnullOG && !m_isnullOD)
            return ( other          ->isnullLOG()
                    && !other       ->isnullLOD()
                    && m_TOD        == other->TOD()
                    && m_modemesure == other->modemesure());
        return  (m_TOD              == other->TOD()
                && m_TOG            == other->TOG()
                && m_modemesure     == other->modemesure());
    }

    bool isDifferent(Tono *other) const
    {
        return !(isEqual(other));
    }

    static Mode     ConvertMesure(QString Mesure)
    {
        if (Mesure == AIR_TO)        return Air;
        if (Mesure == APLANATION_TO) return Aplanation;
        if (Mesure == AUTRE_TO)      return Autre;
        return  NoMesure;
    }

    static QString  ConvertMesure(Tono::Mode mode)
    {
        switch (mode) {
        case Air:           return AIR_TO;
        case Aplanation:    return APLANATION_TO;
        case Autre:         return AUTRE_TO;
        default: return "";
        }
    }
    void setdatas(Tono *tono)
    {
        if (tono->isdataclean()|| (tono->isnullLOD() && tono->isnullLOG()))
        {
            cleandatas();
            return;
        }
        m_timemesure    = tono->timemesure();
        m_modemesure    = tono->modemesure();
        if  (tono->isnullLOD())
            cleandatas(Refraction::Droit);
        else
            setTOD(tono->TOD());
        if  (tono->isnullLOG())
            cleandatas(Refraction::Gauche);
        else
            setTOG(tono->TOG());
        m_cleandatas    = false;
    }

};

#endif // CLS_MESUREDIVERS_H