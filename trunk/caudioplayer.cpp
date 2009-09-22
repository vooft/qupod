#include <QTime>

#include "caudioplayer.h"

CAudioPlayer::CAudioPlayer(CIpodManager* man):
        m_manager(man), m_current(-1)
{
    m_player = new Phonon::MediaObject(this);
    m_audOut = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    m_path = Phonon::createPath(m_player, m_audOut);

    m_player->setTickInterval(1000);
    connect(m_player, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(stateChanged(Phonon::State, Phonon::State)));

    connect(m_player, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(m_player, SIGNAL(aboutToFinish()), this, SIGNAL(aboutToFinish()));
    connect(m_player, SIGNAL(totalTimeChanged(qint64)), this, SLOT(totalTimeChanged(qint64)));

    connect(m_audOut, SIGNAL(volumeChanged(qreal)), this, SIGNAL(volumeLevel(qreal)));
    connect(this, SIGNAL(currentTrackChanged(int)), this, SLOT(changeTrack_p(int)));
}

void CAudioPlayer::stateChanged(Phonon::State, Phonon::State oldState)
{
    if(oldState==Phonon::LoadingState)
        m_player->play();
}

void CAudioPlayer::tick(qint64 time)
{
    //debug(QString("tick(%1)").arg(time));
    int pos = time*1000 / m_trackLength;
    QTime before = QTime().addMSecs(time);
    QTime after = QTime().addMSecs(m_trackLength-time);

    emit currentPosChanged(pos, before, after);
}

void CAudioPlayer::totalTimeChanged(qint64 newTime)
{
    //debug(QString("totalTimeChanged(%1)").arg(newTime));
    m_trackLength = newTime;
    QTime time = QTime().addMSecs(m_trackLength);

    emit currentPosChanged(0, QTime(0, 0, 0, 0), time);
}

void CAudioPlayer::play(int id)
{
    debug(QString("play(%1)").arg(id));

    m_player->clearQueue();
    m_player->setCurrentSource(filename(id));
    m_current = id;
    emit trackStateChanged(m_current, CAudioPlayer::SPlaying);
    emit currentTrackChanged(id);
}

void CAudioPlayer::play()
{
    if(m_player->currentSource().type()!=Phonon::MediaSource::LocalFile)
        debug("play(): Can't play `current' track");
    else
    {
        if(m_player->state()==Phonon::PlayingState)
            m_player->stop();
        m_player->play();
        emit trackStateChanged(m_current, CAudioPlayer::SPlaying);
    }
}

QString CAudioPlayer::filename(int id)
{
    if(id>m_manager->tracks().size() || id<0)
    {
        debug("filename(): incorrect id==" + QString::number(id));
        return QString();
    }

    return m_manager->tracks().at(id).ipod_path;
}

void CAudioPlayer::enqueue(int id)
{
    if(id>m_manager->tracks().size() || id<0)
    {
        debug("enqueue(): Can't play track with id: " + QString::number(id));
        return;
    }

    m_player->clearQueue();
    m_player->enqueue(filename(id));
    emit currentTrackChanged(id);
}

void CAudioPlayer::seek(int pos)
{
    qint64 time = m_trackLength * ((double)pos/1000);
    m_player->seek(time);

    QTime before = QTime().addMSecs(time);
    QTime after = QTime().addMSecs(m_trackLength-time);

    emit currentPosChanged(-1, before, after);
}

void CAudioPlayer::stop()
{
    m_player->stop();
    emit trackStateChanged(m_current, CAudioPlayer::SStopped);
}

void CAudioPlayer::pause()
{
    m_player->pause();
    emit trackStateChanged(m_current, CAudioPlayer::SPaused);
}

qreal CAudioPlayer::volume()
{
    return m_audOut->volume();
}

void CAudioPlayer::setVolume(qreal level)
{
    m_audOut->setVolume(level);
}

void CAudioPlayer::changeTrack_p(int id)
{
    m_current = id;
}
