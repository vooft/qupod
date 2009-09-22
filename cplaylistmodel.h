#ifndef CPLAYLISTMODEL_H
#define CPLAYLISTMODEL_H

#include "cabstractitemmodel.h"

#include <QtDebug>

class CPlaylistModel : public CAbstractItemModel
{
    Q_OBJECT
public:
    CPlaylistModel(QAbstractItemView*);
    int columnCount(const QModelIndex&) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QList<QAction*> actions(const QModelIndex&) const;

    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

    int next();
    int prev();

    bool isShuffle();

    enum
    {
        ADeleteFromPlaylist = CAbstractItemModel::END_OF_ACTIONS,
        APlay,
        AAddToQueue
    };

public slots:
    void addTracks(QList<int>);
    void setCurrentTrack(int);
    void setShuffle(bool b);
    void shuffle();
    void fill();

private slots:
    void delFromPlaylist();
    void play();
    void addToQueue();

signals:
    void play(int);
    void stop();

private:
    QList<int> m_tracks;
    QList<int> m_currentList;
    int m_currentTrack;
    bool m_randomize;
    QList<int> m_queue;

    void debug(QString msg) const { qDebug() << "[CPlaylistModel]" + msg; }
    //QList<int> shuffle(const QList<int>&) const;
    QList<int> shuffle(int) const;
    int current() const { return m_currentList.at(m_currentTrack); }
};

#endif // CPLAYLISTMODEL_H
