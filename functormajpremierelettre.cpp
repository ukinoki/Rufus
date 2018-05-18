/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "functormajpremierelettre.h"

QString FunctorMAJPremiereLettre::operator()(QString MajLettre, bool fin, bool Maj, bool lower)
{
    int     Comptage;
    QString Car1, Car2;
    if (lower)
        MajLettre = MajLettre.toLower();

    Comptage = MajLettre.size(); //On supprime les espace, tiret et apostrophe du début
    for (int i=0; i <  Comptage; i++)
    {
        Car1 = MajLettre.left(1);
        if ((Car1 == " " || Car1 == "-" || Car1 == "'") && i == 0)
        {
            MajLettre = MajLettre.mid(1,1).toUpper() + MajLettre.right(MajLettre.size()-2);
            i = i - 1;
            Comptage = Comptage - 1;
        }
    }

    Comptage = MajLettre.size(); //On supprime les espace et tiret de la fin
    if (fin)
    {
        for (int i=0; i <  Comptage; i++)
        {
            Car1 = MajLettre.right(1);
            if ((Car1 == " " || Car1 == "-" || Car1 == "\n") && i == (Comptage -1))
            {
                MajLettre = MajLettre.left(MajLettre.size()-1);
                i = i - 2;
                Comptage = Comptage - 1;
            }
        }
    }

    Comptage = MajLettre.size(); //On supprime les espace et tiret en répétition au milieu
    for (int i = 1; i <  Comptage; i++)
    {
        Car1 = MajLettre.mid(i,1);
        if ((Car1 == " " || Car1 == "-" || Car1 == "'") && i < (Comptage -1))
        {
            Car2 = MajLettre.mid(i+1,1);
            if (Car2 =="'")
            {
                MajLettre = MajLettre.left(i) + MajLettre.right(MajLettre.size() - (i+1));
                i = i -1;
                Comptage = Comptage - 1;
            }
            if (Car2 == " " || Car2 == "-")
            {
                MajLettre = MajLettre.left(i+1) + MajLettre.right(MajLettre.size() - (i+2));
                i = i -1;
                Comptage = Comptage - 1;
            }
        }
    }

    Comptage = MajLettre.size();  // On met en majuscule derrière les espace, tiret et apostrophe
    if (Maj){
        for (int i = 1; i <  Comptage; i++)
        {
            Car1 = MajLettre.mid(i,1);
            if ((Car1 == " " || Car1 == "-" || Car1 == "'") && i < (Comptage -1))
            {
                Car2 = MajLettre.mid(i+1,1);
                MajLettre = MajLettre.left(i+1) + Car2.toUpper()+ MajLettre.right(MajLettre.size() - (i+2));
            }
        }
    }
    MajLettre = MajLettre.mid(0,1).toUpper() + MajLettre.right(MajLettre.size()-1);
    return(MajLettre);
}
