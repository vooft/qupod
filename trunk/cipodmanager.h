#ifndef CIPODMANAGER_H
#define CIPODMANAGER_H

#include <QtCore>
#include <QtGui>

#include <gpod-1.0/gpod/itdb.h>
#include <glib-2.0/glib.h>

#include <QtDebug>

struct CAudioTrack
{
    CAudioTrack(quint32 i): id(i) { track_nr = 0; year=0; }
    bool operator==(const CAudioTrack &s) const { return id==s.id; }

    quint32 id;
    QString artist;
    int artist_id;
    QString album;
    int album_id;
    QString title;
    QString ipod_path;
    QTime tracklen;
    int track_nr;
    QString genre;
    int year;
    bool isCompilation;

    Itdb_Track *track_ref;
};

struct CAlbum
{
    CAlbum(QString n): name(n) { artwork = -1; isCompilation=false; year=-1;}
    bool operator==(const CAlbum &s) const { return name.compare(s.name, Qt::CaseInsensitive)==0; }
    bool operator<(const CAlbum &s) const { return name<s.name; }

    QString name;
    bool isCompilation;
    QList<int> tracks;
    int artwork;
    int year;
};

struct CArtist
{
    CArtist(QString n): name(n) { }
    bool operator==(const CArtist &s) const { return name.compare(s.name, Qt::CaseInsensitive)==0; }
    bool operator<(const CArtist &s) const { return name.compare(s.name, Qt::CaseInsensitive)<0; }

    QString name;
    QList<int> albums;
};

struct CPlaylist
{
    CPlaylist(QString n): name(n) { }
    QString name;
    QList<int> tracks; // internal id
    QList<int> tracks_id; // Itdb's id
    bool is_mpl; // is master playlist
};

class CIpodManager : public QObject
{
    Q_OBJECT

    void debug(QString msg) const { qDebug() << "[CIpodManager]" + msg; }

    int findArtist(const QString&);
    int findAlbum(const QString&, CArtist&);
    int findCompilation(const QString&);
    void sortAlbums(QList<int>&);
    void sortTracks(QList<int>&);
    void writeDatabase();

    void setYearForAlbum(int album_id, int year);
    void setGChar(gchar**, QString);

public:
    CIpodManager();
    ~CIpodManager();

    bool load(QString path);
    bool initialize(QString path);

    QString mountPoint() { return m_mountPoint; }

    QString errorString() { return m_errorString; }

    QList<CArtist> &artists() { return m_artists; }
    QList<CAlbum> &albums() { return m_albums; }
    QList<int> &compilations() { return m_compilations; }
    QList<CAudioTrack> &tracks() { return m_tracks; }
    QList<QPixmap> &artworks() { return m_albumArt; }
    QList<CPlaylist> &playlists() { return m_playlists; }

    QPixmap artwork(int album_id);

    void saveArtwork(int album_id, const QPixmap&);
    void saveTrack(int);

signals:
    void changed();

private:
    QString m_mountPoint;
    QString m_errorString;

    Itdb_iTunesDB* m_itdb;
    QList<CArtist> m_artists;

    QList<CAlbum> m_albums;
    QList<int> m_compilations;
    QList<CAudioTrack> m_tracks;
    QList<QPixmap> m_albumArt;
    QList<CPlaylist> m_playlists;

    bool m_wasChanged;
};

#endif // CIPODMANAGER_H
