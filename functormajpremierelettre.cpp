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
    QString textC = MajLettre;
    QChar c;
    QString Car1, Car2;
    if (lower)
        textC = MajLettre.toLower();

    while( textC.size() ) //On supprime les espace, tiret et apostrophe du début
    {
        c = textC.at(0);
        if( c == " " || c == "-" || c == "'" )
            textC = textC.remove(0,1);
        else
            break;
    }
    if( fin ) //On supprime les espace et tiret de la fin
        while( textC.size() )
        {
            int lastIndex = textC.size() - 1;
            c = textC.at(lastIndex);
            if( c == " " || c == "-" || c == "'" )
                textC = textC.remove(lastIndex,1);
            else
                break;
        }

    QString newText = ""; //On supprime les espace et tiret en répétition au milieu
    QChar lastChar;
    for( int i=0; i < textC.size(); ++i )
    {
        c = textC.at(i);
        if( lastChar == " " || lastChar == "-" || lastChar == "'" )
            if( lastChar == c )
                continue;

        newText += c;
        lastChar = c;
    }
    textC = newText;

    // On met en majuscule derrière les espace, tiret et apostrophe
    if (Maj){
        for (int i = 1; i < (textC.size() - 1); i++)
        {
            Car1 = textC.mid(i,1);
            if( (Car1 == " " || Car1 == "-" || Car1 == "'") )
            {
                QString Car2 = textC.mid(i+1,1);
                textC = textC.replace(i+1,1, Car2.toUpper());
            }
        }
    }
    QString C = textC.mid(0,1).toUpper();
    MajLettre = textC.replace(0,1, C);
    return(MajLettre);
}
