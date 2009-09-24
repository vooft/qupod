#include <QTemporaryFile>

#include "cipodmanager.h"
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

CIpodManager::CIpodManager():
        m_itdb(0), m_wasChanged(false)
{
    g_type_init();
}

CIpodManager::~CIpodManager()
{
    if(m_itdb)
    {
        if(m_wasChanged)
            writeDatabase();
        itdb_free(m_itdb);
    }
}

void CIpodManager::writeDatabase()
{
    debug("Writing database");
    bool ok = true;
    GError *error = 0;
    if(!itdb_write(m_itdb, &error))
    {   /* an error occurred */
        if(error)
        {
            if (error->message)
                debug("itdb_write error: " + QString::fromUtf8(error->message));
            else
                debug("itdb_write error: error->message == 0!");
            g_error_free (error);
        }
        error = 0;
        ok = false;
    }
}

bool CIpodManager::load(QString path)
{
    if(path.isEmpty())
    {
        debug("Path is empty");
        m_errorString = tr("Path was not defined. Can't mount empty path.");
        return false;
    }

    if(path.endsWith("/"))
        path.remove(path.length()-1, 1);
    m_mountPoint = path;

    GError *err = 0;

    debug("Calling the db parser");

    m_itdb = itdb_parse( QFile::encodeName( m_mountPoint ),  &err );
    if(err)
    {
        debug("Error while parsing db. Error: " + QString(err->message));
        m_errorString = err->message;

        g_error_free(err);

        return false;
    }

    for(GList *list=g_list_first(m_itdb->playlists); list!=NULL; list=g_list_next(list))
    {
        Itdb_Playlist *pls = static_cast<Itdb_Playlist*>(list->data);

        CPlaylist playlist(QString::fromUtf8(pls->name));
        playlist.is_mpl = itdb_playlist_is_mpl(pls);

        for(GList *tracks=g_list_first(pls->members); tracks!=NULL; tracks=g_list_next(tracks))
        {
            Itdb_Track *itrack = static_cast<Itdb_Track*>(tracks->data);
            playlist.tracks_id.append(itrack->id);
            playlist.tracks.append(0);
        }
        m_playlists.append(playlist);

        if(playlist.is_mpl)
            debug("Main playlist: " + playlist.name);
    }

    QMap<QString, int> compilations;
    for(GList *list=g_list_first(m_itdb->tracks); list!=NULL; list=g_list_next(list))
    {
        Itdb_Track *itrack = static_cast<Itdb_Track*>(list->data);
        if(itrack)
        {
            QString artistName = QString::fromUtf8(itrack->artist);
            QString albumName = QString::fromUtf8(itrack->album);
            QString title = QString::fromUtf8(itrack->title);
            QString genre = QString::fromUtf8(itrack->genre);

            CAudioTrack track(itrack->id);
            track.track_ref = itrack;
            track.artist = artistName;
            track.artist_id = findArtist(artistName);
            
            //debug("artistName==" + artistName);
            //debug("m_artists[]==" + m_artists.at(track.artist_id).name);
            
            track.album = albumName;
            track.isCompilation = itrack->compilation==0x1;

            if(itrack->compilation==0x1) // True if set to 0x1, false if set to 0x0.
                track.album_id = findCompilation(albumName);
            else
                track.album_id = findAlbum(albumName, m_artists[track.artist_id]);


            CAlbum &album = m_albums[track.album_id];

            track.title = title;
            track.genre = genre;
            track.ipod_path = m_mountPoint + QString::fromUtf8(itrack->ipod_path).replace(":", "/");
            track.tracklen = QTime().addMSecs(itrack->tracklen);
            if(itrack->track_nr)
                track.track_nr = itrack->track_nr;

            if(itrack->year)
            {
                if(album.year==-1)
                    album.year = itrack->year;
                track.year = album.year;
            }

            if(album.artwork==-1)
            {
                if(itrack->has_artwork == 0x01 ) // has_artwork: True if set to 0x01, false if set to 0x02
                {
                    QTemporaryFile temp_file;
                    if(temp_file.open())
                    {
                        temp_file.close();
                        GdkPixbuf *pixbuf = (GdkPixbuf*) itdb_artwork_get_pixbuf(itrack->itdb->device,
                                                                                 itrack->artwork, -1, -1 );
                        if(pixbuf)
                        {
                            gdk_pixbuf_save(pixbuf, temp_file.encodeName(temp_file.fileName()),
                                            "png", NULL, (const char*)(NULL));
                            gdk_pixbuf_unref(pixbuf);

                            QPixmap pixmap(temp_file.fileName());
                            if(!pixmap.isNull())
                            {
                                m_albumArt.append(pixmap);
                                album.artwork = m_albumArt.count()-1;
                            }
                        }
                    }
                }
            }

            m_tracks.append(track);
            album.tracks.append(m_tracks.count()-1);

            for(int j=0; j<m_playlists.count(); j++)
            {
                CPlaylist &pls = m_playlists[j];
                int index = pls.tracks_id.indexOf(itrack->id);
                if(index==-1)
                    continue;

                pls.tracks[index] = m_tracks.count()-1;
            }
        }
        else
            debug("Cast is not valid");
    }

    qSort(m_artists);
    int count = m_artists.count();
    for(int i=0; i<count; i++)
        sortAlbums(m_artists[i].albums);

    count = m_albums.count();
    for(int i=0; i<count; i++)
        sortTracks(m_albums[i].tracks);

    count = m_tracks.count(); // too bad
    for(int i=0; i<count; i++)
        m_tracks[i].artist_id = findArtist(m_tracks.at(i).artist);

    sortAlbums(m_compilations);

    debug("Artists count: " + QString::number(m_artists.count()));
    debug("Tracks count:" + QString::number(m_tracks.size()));

    return true;
}

int CIpodManager::findArtist(const QString& name)
{
/*    int result = m_artists.indexOf(name);
    if(result==-1)
    {
        result = m_artists.size();
        m_artists.append(name);
    }*/
    int result = -1;
    for(int i=0; i<m_artists.size(); i++)
        if(m_artists.at(i).name==name)
        {
            result = i;
            break;
        }

    if(result==-1)
    {
        result = m_artists.size();
        m_artists.append(name);
    }

    return result;
}

int CIpodManager::findAlbum(const QString& name, CArtist& artist)
{
    foreach(int id, artist.albums)
        if(m_albums.at(id).name==name)
            return id;

    m_albums.append(name);
    artist.albums.append(m_albums.count()-1);
    return m_albums.count()-1;
}

int CIpodManager::findCompilation(const QString& name)
{
    foreach(int i, m_compilations)
        if(m_albums.at(i).name==name)
            return i;

    m_albums.append(name);
    m_compilations.append(m_albums.count()-1);
    m_albums.last().isCompilation = true;

    return m_compilations.last();
}

bool CIpodManager::initialize(QString)
{
    return true;
}

QPixmap CIpodManager::artwork(int album_id)
{
    if(m_albums.count()<album_id)
        return QPixmap();

    const CAlbum &album = m_albums.at(album_id);
    if(album.artwork==-1)
        return QPixmap(":/icons/nocover");

    return m_albumArt.at(album.artwork);
}

void CIpodManager::sortAlbums(QList<int> &list)
{
    int count = list.count();
    for(int i=0; i<count; i++)
    {
        for(int j=0; j<count-1; j++)
            if(m_albums.at(list.at(j)).name>m_albums.at(list.at(j+1)).name)
                list.swap(j, j+1);
    }

    for(int i=0; i<count; i++)
    {
        for(int j=0; j<count-1; j++)
            if(m_albums.at(list.at(j)).year<m_albums.at(list.at(j+1)).year)
                list.swap(j, j+1);
    }
}

void CIpodManager::sortTracks(QList<int> &list)
{
    int count = list.count();
    for(int i=0; i<count; i++)
    {
        for(int j=0; j<count-1; j++)
            if(m_tracks.at(list.at(j)).title>m_tracks.at(list.at(j+1)).title)
                list.swap(j, j+1);
    }

    for(int i=0; i<count; i++)
    {
        for(int j=0; j<count-1; j++)
            if(m_tracks.at(list.at(j)).track_nr>m_tracks.at(list.at(j+1)).track_nr)
                list.swap(j, j+1);
    }
}

void CIpodManager::saveArtwork(int album_id, const QPixmap &pix)
{
    QTemporaryFile temp_file;
    if(temp_file.open())
    {
        temp_file.close();
        bool saved = pix.save(temp_file.fileName()+".bmp", "bmp");
        if(!saved)
        {
            debug("Can't save pixmap for transferring.");
            return;
        }

        foreach(int i, m_albums.at(album_id).tracks)
        {
            const CAudioTrack &track = m_tracks.at(i);
            Itdb_Track *itrack = track.track_ref;
            itdb_artwork_remove_thumbnails(itrack->artwork);

            //GError *error;
            if(!itdb_track_set_thumbnails(itrack, temp_file.encodeName(temp_file.fileName()+".bmp")))
            {
                debug("Can't set thumbnail");
                //g_error_free(error);
            }
            else
                itrack->has_artwork = 0x01;
        }

        m_albumArt.append(pix);
        m_albums[album_id].artwork = m_albumArt.count()-1;
    }

    m_wasChanged = true;
}

void CIpodManager::setGChar(gchar **c, QString str)
{
    if(*c)
        g_free(*c);
    
    *c = g_strdup((gchar*)str.toUtf8().data());
}

void CIpodManager::saveTrack(int id)
{
    if(id>tracks().size())
        return;

    debug("saveTrack(): id==" + QString::number(id));

    CAudioTrack &track = tracks()[id];

    int newArtist = findArtist(track.artist);
    int oldArtist = track.artist_id;

    debug("saveTrack(): newArtist==" + QString::number(newArtist));
    debug("saveTrack(): oldArtist==" + QString::number(oldArtist));

    int newAlbum;
    if(track.isCompilation)
        newAlbum = findCompilation(track.album);
    else
        newAlbum = findAlbum(track.album, m_artists[newArtist]);
    int oldAlbum = track.album_id;
    debug("saveTrack(): newAlbum==" + QString::number(newAlbum));
    debug("saveTrack(): oldAlbum==" + QString::number(oldAlbum));

    if(newAlbum!=oldAlbum)
    {
        m_albums[oldAlbum].tracks.removeOne(id);
        m_albums[newAlbum].tracks.append(id);
        sortTracks(m_albums[newAlbum].tracks);
        track.year = m_albums.at(newAlbum).year;

        debug("oldAlbum:" + m_albums.at(oldAlbum).name);
        debug("m_albums.at(oldAlbum).tracks.size()==" + QString::number(m_albums.at(oldAlbum).tracks.size()));
        if(!m_albums.at(oldAlbum).tracks.size())
        {
            debug("artist: " + m_artists[oldArtist].name);
            debug("before: size()==" + QString::number(m_artists[oldArtist].albums.size()));
            m_artists[oldArtist].albums.removeOne(oldAlbum);
            debug("after: size()==" + QString::number(m_artists[oldArtist].albums.size()));
        }
    }
    else
    {
        if(m_albums.at(oldAlbum).year!=track.year)
             setYearForAlbum(oldAlbum, track.year);
    }

    Itdb_Track *itrack = track.track_ref;

    setGChar(&(itrack->artist), track.artist);
    setGChar(&(itrack->album), track.album);
    setGChar(&(itrack->title), track.title);
    setGChar(&(itrack->genre), track.genre);
    itrack->track_nr = track.track_nr;
    itrack->year = track.year;

    itrack->compilation = track.isCompilation ? 0x1 : 0x0;

    m_wasChanged = true;
    emit changed();
}

void CIpodManager::setYearForAlbum(int album_id, int year)
{
    foreach(int i, m_albums.at(album_id).tracks)
        m_tracks[i].year = year;

    m_albums[album_id].year = year;
}
