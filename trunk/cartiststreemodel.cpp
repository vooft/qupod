#include "cartiststreemodel.h"
#include "csetartworkdialog.h"

CArtistsTreeModel::CArtistsTreeModel(QAbstractItemView *view):
        CAbstractItemModel(view)
{
    QAction *addTracks = new QAction(QIcon(":/icons/add"), tr("Add"), this);
    connect(addTracks, SIGNAL(triggered()), this, SLOT(addTracks()));
    m_actions.insert(AAddTracks, addTracks);

    m_actions.insert(ASetArtwork, new QAction(QIcon(":/icons/image"), tr("Set artwork"), this));
    connect(m_actions.value(ASetArtwork), SIGNAL(triggered()), this, SLOT(setArtwork()));

    connect(view, SIGNAL(activated(QModelIndex)), addTracks, SIGNAL(triggered()));
}

void CArtistsTreeModel::fill()
{
    beginResetModel();
    if(rootItem)
        delete rootItem;

    rootItem = new CModelItem(CModelItem::ENone, CModelItem::ETreeItem, 0, 0);
    VAItems = new CModelItem(CModelItem::EVACathegory, CModelItem::ETreeItem, 0, rootItem);
    artistsItems = new CModelItem(CModelItem::EArtistsCathegory, CModelItem::ETreeItem, 0, rootItem);
    playlistItems = new CModelItem(CModelItem::EPlaylistsCathegory, CModelItem::ETreeItem, 0, rootItem);

    rootItem->appendChild(VAItems);
    rootItem->appendChild(artistsItems);
    rootItem->appendChild(playlistItems);

    for(int i=0; i<m_manager->artists().count(); i++)
    {
        CModelItem *artist = new CModelItem(CModelItem::EArtist, CModelItem::ETreeItem, i, artistsItems);
        for(int j=0; j<m_manager->artists().at(i).albums.count(); j++)
        {
            int album_id = m_manager->artists().at(i).albums.at(j);
            if(!m_manager->albums().at(album_id).isCompilation)
            {
                CModelItem *album = new CModelItem(CModelItem::EAlbum, CModelItem::ETreeItem, album_id, artist);
                for(int k=0; k<m_manager->albums().at(album_id).tracks.count(); k++)
                {
                    int track_id = m_manager->albums().at(album_id).tracks.at(k);
                    CModelItem *track = new CModelItem(CModelItem::ETrack, CModelItem::ETreeItem, track_id, album);
                    album->appendChild(track);
                }
                artist->appendChild(album);
            }
        }
        if(!artist->rowCount())
            delete artist;
        else
            artistsItems->appendChild(artist);
    }

    foreach(int i, m_manager->compilations())
    {
         const CAlbum &a = m_manager->albums().at(i);
         CModelItem *album = new CModelItem(CModelItem::EAlbum, CModelItem::ETreeItem, i, VAItems);
         for(int k=0; k<a.tracks.count(); k++)
         {
            int track_id = a.tracks.at(k);
            CModelItem *track = new CModelItem(CModelItem::ETrack, CModelItem::ETreeItem, track_id, album);
            album->appendChild(track);
         }
         VAItems->appendChild(album);
    }

    for(int i=0; i<m_manager->playlists().count(); i++)
    {
        const CPlaylist &p = m_manager->playlists().at(i);
        CModelItem *playlist = new CModelItem(CModelItem::EPlaylist, CModelItem::ETreeItem, i, playlistItems);
        foreach(int j, p.tracks)
        {
            CModelItem *track = new CModelItem(CModelItem::ETrack, CModelItem::ETreeItem, j, playlist);
            playlist->appendChild(track);
        }
        playlistItems->appendChild(playlist);
    }
    endResetModel();
}

void CArtistsTreeModel::addTracks()
{
    QList<int> result = tracks(currentIndex());
    if(result.size())
        emit addTracks(result);
}

QList<QAction*> CArtistsTreeModel::actions(const QModelIndex& index) const
{
    QList<QAction*> result;

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    switch(item->itemType())
    {
        case CModelItem::EAlbum:
        {
            result << m_actions.value(CArtistsTreeModel::AAddTracks);
            result << m_actions.value(CAbstractItemModel::AEditTag);
            result << m_actions.value(CArtistsTreeModel::ASetArtwork);
            result << m_actions.value(CAbstractItemModel::ARemoveFromDb);
            break;
        }
        case CModelItem::EArtist:
        {
            result << m_actions.value(CArtistsTreeModel::AAddTracks);
            result << m_actions.value(CAbstractItemModel::AEditTag);
            result << m_actions.value(CAbstractItemModel::ARemoveFromDb);
            break;
        }
        case CModelItem::ETrack:
        {
            result << m_actions.value(CArtistsTreeModel::AAddTracks);
            result << m_actions.value(CAbstractItemModel::AEditTag);
            result << m_actions.value(CArtistsTreeModel::ASetArtwork);
            result << m_actions.value(CAbstractItemModel::ARemoveFromDb);
            break;
        }
        case CModelItem::EPlaylist:
        {
            result << m_actions.value(CArtistsTreeModel::AAddTracks);
            result << m_actions.value(CAbstractItemModel::ARemoveFromDb);
            break;
        }
        default: break;
    }

    return result;
}

void CArtistsTreeModel::setArtwork()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    switch(item->itemType())
    {
        case CModelItem::ETrack:
            showSetArtworkDialog(m_manager->tracks().at(item->id()).album_id);
            break;

        case CModelItem::EAlbum:
            showSetArtworkDialog(item->id());

        default:
            break;
    }
}

void CArtistsTreeModel::showSetArtworkDialog(int album_id)
{
    CSetArtworkDialog dialog(album_id, m_manager);
    if(dialog.exec()==QDialog::Accepted)
    {
        QPixmap pix = dialog.pixmap();
        m_manager->saveArtwork(album_id, pix);
    }
}
