#ifndef UPWAITOVERLAY_H
#define UPWAITOVERLAY_H

#include <QLabel>
#include <QMovie>
#include <QEvent>

/* Sablier : GIF animé (spinner.gif) superposé au CENTRE de son parent, sans rien changer à son
 * layout (donc aucun redimensionnement de la fiche). À créer sur la pile juste avant une attente
 * bloquante et à laisser mourir en fin de portée :
 *     { UpWaitOverlay sablier(this); ...opération bloquante... }
 * Le GIF s'anime tant que la boucle d'événements tourne (waitProcessResponsive s'en charge pendant
 * les runCmd). Header-only, pas de Q_OBJECT. */
class UpWaitOverlay : public QLabel
{
public:
    explicit UpWaitOverlay(QWidget* parent, int taille = 72) : QLabel(parent)
    {
        m_movie = new QMovie(QStringLiteral(":/spinner.gif"), QByteArray(), this);
        m_movie->setScaledSize(QSize(taille, taille));
        setMovie(m_movie);
        setFixedSize(taille, taille);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        m_movie->start();
        if (parent) parent->installEventFilter(this);   // rester centré si la fiche bouge/redimensionne
        recentrer();
        show();
        raise();
    }

protected:
    bool eventFilter(QObject* o, QEvent* e) override
    {
        if (o == parent() && (e->type() == QEvent::Resize || e->type() == QEvent::Move))
            recentrer();
        return QLabel::eventFilter(o, e);
    }

private:
    void recentrer()
    {
        if (QWidget* p = parentWidget())
            move((p->width() - width()) / 2, (p->height() - height()) / 2);
    }
    QMovie* m_movie;
};

#endif // UPWAITOVERLAY_H
