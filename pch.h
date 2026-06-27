/* (C) 2026 LAINE SERGE
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

/*! En-tête précompilé (PCH).
 *  But : compiler UNE seule fois les gros en-têtes Qt au lieu de les re-parser dans chacun des
 *  ~170 fichiers du projet. Gain net à la compilation, surtout sous MSVC (Windows).
 *
 *  Règle : on ne met ici QUE des en-têtes Qt STABLES — jamais d'en-tête du projet Rufus.
 *  Sinon, modifier un en-tête Rufus invaliderait le PCH et recompilerait TOUT le projet à
 *  chaque fois, ce qui serait l'effet inverse de celui recherché.
 *
 *  Activé par « CONFIG += precompile_header » dans RufusQt6.pro : qmake force-inclut ce fichier
 *  en tête de chaque unité de compilation. */

#ifndef PCH_H
#define PCH_H

#ifdef __cplusplus

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QtNetwork>
#include <QtPrintSupport>
#include <QtXml>
#include <QtMultimedia>
#include <QtMultimediaWidgets>

#endif // __cplusplus

#endif // PCH_H
