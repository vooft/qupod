#ifndef CMODELITEM_H
#define CMODELITEM_H

#include "cipodmanager.h"

class CModelItem
{
public:

    enum Type
    {
        EArtist,
        EAlbum,
        ETrack,
        EPlaylist,
        EVACathegory,
        EArtistsCathegory,
        EPlaylistsCathegory,
        ENone
    };

    enum ViewType
    {
        ETreeItem,
        EListItem,
        ETableItem
    };

    enum Columns
    {
        COL_Cover,
        COL_Artist,
        COL_Album,
        COL_Title,
        COL_Duration,
        END_OF_COLUMNS
    };

    CModelItem(CModelItem::Type, CModelItem::ViewType, int id, CModelItem *parent);
    ~CModelItem();


    CModelItem::Type itemType() { return m_itemType; }
    CModelItem::ViewType viewType() { return m_viewType; }

    int id() { return m_id; }
    int rowCount();
    int row();

    void appendChild(CModelItem*);
    void removeChild(int row);
    QVariant data(int role, int column);
    CModelItem* child(int row);
    CModelItem* parent() { return m_parent; }

    const QList<CModelItem*>& children();

    static void setIpodManager(CIpodManager *manager) { m_manager = manager; }

    static QString description(int);

private:
    CModelItem::Type m_itemType;
    CModelItem::ViewType m_viewType;
    int m_id; // id where item is linked to
    static CIpodManager *m_manager;
    QList<CModelItem*> m_children;
    CModelItem *m_parent;

    void debug(QString msg) const { qDebug() << "[ModelItem] " + msg; }
};

#endif // CMODELITEM_H
