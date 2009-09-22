#include "cmodelitem.h"

CIpodManager* CModelItem::m_manager = 0;

CModelItem::CModelItem(CModelItem::Type itemType, CModelItem::ViewType viewType, int id, CModelItem *parent):
        m_itemType(itemType), m_viewType(viewType), m_id(id), m_parent(parent)
{
}

CModelItem::~CModelItem()
{
    qDeleteAll(m_children);
}

void CModelItem::appendChild(CModelItem* item)
{
    m_children.append(item);
}

void CModelItem::removeChild(int row)
{
    if(row>=m_children.size())
        return;

    CModelItem *item = m_children[row];
    m_children.removeAt(row);
    delete item;
}

QVariant CModelItem::data(int role, int column)
{
    if(viewType()==CModelItem::ETableItem)
    {
        const CAudioTrack &track = m_manager->tracks().at(id());
        if(role==Qt::DecorationRole)
        {
            if(column!=CModelItem::COL_Cover)
                return QVariant();

            return m_manager->artwork(track.album_id).scaledToWidth(32, Qt::SmoothTransformation);
        }

        if(role!=Qt::DisplayRole)
            return QVariant();

        switch(column)
        {
            case CModelItem::COL_Album:
                return track.album;

            case CModelItem::COL_Artist:
                return track.artist;

            case CModelItem::COL_Title:
                return track.title;

            case CModelItem::COL_Duration:
                return track.tracklen.toString("m:ss");

            default:
                return QVariant();
        }
    }


    if(role==Qt::DecorationRole)
    {
        if(m_itemType!=EAlbum)
            return QVariant();
        
        QPixmap pix = m_manager->artwork(m_id);
        if(!pix.isNull())
            return pix.scaledToHeight(32);

        return pix;
    }

    if(role!=Qt::DisplayRole)
        return QVariant();

    switch(m_itemType)
    {
        case EArtist:
            return m_manager->artists().at(m_id).name;

        case EAlbum:
        {
            QString result = m_manager->albums().at(m_id).name;
            if(m_manager->albums().at(m_id).year!=-1)
                result = QString::number(m_manager->albums().at(m_id).year) + " - " + result;
            return result;
        }

        case ETrack:
        {
            QString result = m_manager->tracks().at(m_id).title;
            if(m_manager->tracks().at(m_id).track_nr!=-1)
                result = QString::number(m_manager->tracks().at(m_id).track_nr) + " - " + result;
            return result;
        }

        case EPlaylist:
        {
            return m_manager->playlists().at(m_id).name;
        }

        case EVACathegory: return QObject::tr("Various artists");

        case EArtistsCathegory: return QObject::tr("Artists");

        case EPlaylistsCathegory: return QObject::tr("Playlists");

        case ENone:
            return QVariant();
    }

    return QVariant();
}

CModelItem* CModelItem::child(int row)
{
    return m_children.at(row);
}

int CModelItem::rowCount()
{
    return m_children.size();
}

int CModelItem::row()
{
    if(m_parent)
    {
        int result = m_parent->children().indexOf(const_cast<CModelItem*>(this));
        return result;
    }

    return 0;
}

const QList<CModelItem*>& CModelItem::children()
{
    return m_children;
}

QString CModelItem::description(int id)
{
    switch(id)
    {
        case CModelItem::COL_Album: return QObject::tr("Album");
        case CModelItem::COL_Artist: return QObject::tr("Artist");
        case CModelItem::COL_Cover: return QString(); //QObject::tr("Cover");
        case CModelItem::COL_Duration: return QObject::tr("Duration");
        case CModelItem::COL_Title: return QObject::tr("Title");
    }

    return QString();
}
