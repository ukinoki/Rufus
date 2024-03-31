#ifndef RESSOURCES_H
#define RESSOURCES_H

#include <QObject>
#include "macros.h"

class Ressources : public QObject
{
    Q_OBJECT

private:
    Ressources();
    static Ressources *instance;

public:
    static Ressources*   I();

    QString HeaderOrdo();
    QString HeaderOrdoALD();

    QString BodyOrdo();
    QString BodyOrdoALD();

    QString FooterOrdo();
    QString FooterOrdoLunettes();
};

#endif // RESSOURCES_H
