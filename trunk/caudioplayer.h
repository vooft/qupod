#ifndef CAUDIOPLAYER_H
#define CAUDIOPLAYER_H

#include <QObject>

#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/audiooutput.h>
#include <phonon/path.h>

#include <QtDebug>

#include "cipodmanager.h"

class CAudioPlayer : public QObject
{
    Q_OBJECT

    void debug(QString msg) const { qDebug() << "[CAudioPlayer]" + msg; }
public:
    CAudioPlayer(CIpodManager*);
    qreal volume();
    void setVolume(qreal);

    enum
    {
        SPlaying,
        SPaused,
        SStopped
    };

public slots:
    void play(int);
    void play(); // try to play current track (if exists)
    void pause();
    void stop();
    void enqueue(int);
    void seek(int);

private slots:
    void stateChanged(Phonon::State, Phonon::State);
    void tick(qint64);
    void totalTimeChanged(qint64);
    void changeTrack_p(int);

signals:
    void currentPosChanged(int pos, const QTime &begin, const QTime &end);
    void currentTrackChanged(const QString&);
    void volumeLevel(qreal);
    void aboutToFinish();
    void currentTrackChanged(int);
    void trackStateChanged(int, int);

private:
    CIpodManager *m_manager;
    Phonon::MediaObject *m_player;
    Phonon::AudioOutput *m_audOut;
    Phonon::Path m_path;    

    int m_current;
    qint64 m_trackLength;

    QString filename(int id);
};

#endif // CAUDIOPLAYER_H
