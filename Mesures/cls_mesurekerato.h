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

#ifndef CLS_MESUREKERATO_H
#define CLS_MESUREKERATO_H

#include <QObject>

class MesureKerato : public QObject
{
    Q_OBJECT
public:
    explicit MesureKerato() {}

private:
    double m_K1OD           = 0;
    double m_K2OD           = 0;
    double m_dioptriesK1OD  = 0;
    double m_dioptriesK2OD  = 0;
    int m_AxeKOD            = 0;
    double m_K1OG           = 0;
    double m_K2OG           = 0;
    double m_dioptriesK1OG  = 0;
    double m_dioptriesK2OG  = 0;
    int m_AxeKOG            = 0;
    bool m_cleandatas       = true;

public:
    double K1OD() const                   { return m_K1OD; }
    double K2OD() const                   { return m_K2OD; }
    double dioptriesK1OD() const          { return m_dioptriesK1OD; }
    double dioptriesK2OD() const          { return m_dioptriesK2OD; }
    int axeKOD() const                    { return m_AxeKOD; }
    double K1OG() const                   { return m_K1OG; }
    double K2OG() const                   { return m_K2OG; }
    double dioptriesK1OG() const          { return m_dioptriesK1OG; }
    double dioptriesK2OG() const          { return m_dioptriesK2OG; }
    int axeKOG() const                    { return m_AxeKOG; }
    double dioptriesKOD() const           { return m_dioptriesK1OD - m_dioptriesK2OD; }
    double dioptriesKOG() const           { return m_dioptriesK1OG - m_dioptriesK2OG; }

    void setK1OD(double val)            { m_K1OD = val; m_cleandatas = false; }
    void setK2OD(double val)            { m_K2OD = val; m_cleandatas = false; }
    void setdioptriesK1OD(double val)   { m_dioptriesK1OD = val; m_cleandatas = false; }
    void setdioptriesK2OD(double val)   { m_dioptriesK2OD = val; m_cleandatas = false; }
    void setaxeKOD(int axe)             { m_AxeKOD = axe; m_cleandatas = false; }
    void setK1OG(double val)            { m_K1OG = val; m_cleandatas = false; }
    void setK2OG(double val)            { m_K2OG = val; m_cleandatas = false; }
    void setdioptriesK1OG(double val)   { m_dioptriesK1OG = val; m_cleandatas = false; }
    void setdioptriesK2OG(double val)   { m_dioptriesK2OG = val; m_cleandatas = false; }
    void setaxeKOG(int axe)             { m_AxeKOD = axe; m_cleandatas = false; }

    bool isdataclean()                  { return m_cleandatas; }
    void cleandatas()
    {
        m_K1OD           = 0;
        m_K2OD           = 0;
        m_dioptriesK1OD  = 0;
        m_dioptriesK2OD  = 0;
        m_AxeKOD         = 0;
        m_K1OG           = 0;
        m_K2OG           = 0;
        m_dioptriesK1OG  = 0;
        m_dioptriesK2OG  = 0;
        m_AxeKOG         = 0;
        m_cleandatas     = true;
    }

    void setdatas(MesureKerato *ker)
    {
        if (ker->isdataclean())
        {
            cleandatas();
            return;
        }
        m_K1OD           = ker->K1OD();
        m_K2OD           = ker->K2OD();
        m_dioptriesK1OD  = ker->dioptriesK1OD();
        m_dioptriesK2OD  = ker->dioptriesK2OD();
        m_AxeKOD         = ker->axeKOD();
        m_K1OG           = ker->K1OG();
        m_K2OG           = ker->K2OG();
        m_dioptriesK1OG  = ker->dioptriesK1OG();
        m_dioptriesK2OG  = ker->dioptriesK2OG();
        m_AxeKOG         = ker->axeKOG();
        m_cleandatas     = ker->isdataclean();
    }

    bool isEqual(MesureKerato *other) const
    {
        bool a = false;
        a = (  int(m_K1OD*100) == int(other->K1OD()*100)
            && int(m_K2OD*100) == int(other->K2OD()*100)
            && m_AxeKOD        == other->axeKOD()
            && int(m_K1OG*100) == int(other->K1OG()*100)
            && int(m_K2OG*100) == int(other->K2OG()*100)
            && m_AxeKOG        == other->axeKOG());
        return  a;
    }

    bool isDifferent(MesureKerato *other) const
    {
        return !(isEqual(other));
    }
};


#endif // CLS_MESUREKERATO_H
