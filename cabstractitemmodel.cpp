#include "cabstractitemmodel.h"
#include "ctageditdialog.h"

CIpodManager* CAbstractItemModel::m_manager = 0;

CAbstractItemModel::CAbstractItemModel(QAbstractItemView *view):
        m_itemView(view), rootItem(0)
{
    m_actions.insert(AEditTag, new QAction(QIcon(":/icons/edit"), tr("Edit"), this));
    connect(m_actions.value(AEditTag), SIGNAL(triggered()), this, SLOT(editTag()));

    m_actions.insert(ARemoveFromDb, new QAction(QIcon(":/icons/delete"), tr("Remove from database"), this));
    connect(m_actions.value(ARemoveFromDb), SIGNAL(triggered()), this, SLOT(removeFromDatabase()));

    connect(m_manager, SIGNAL(changed()), this, SLOT(fill()));
}

CAbstractItemModel::~CAbstractItemModel()
{
    delete rootItem;
}

Qt::ItemFlags CAbstractItemModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex CAbstractItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    CModelItem *parentItem;

    if(!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<CModelItem*>(parent.internalPointer());

    CModelItem *childItem = parentItem->child(row);
    if(childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


int CAbstractItemModel::rowCount(const QModelIndex &parent) const
{
    if(!rootItem)
        return 0;

    CModelItem *parentItem;

    if(parent.column()>0)
        return 0;

    if(!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<CModelItem*>(parent.internalPointer());

    return parentItem->rowCount();
}

QList<int> CAbstractItemModel::tracks(const QModelIndex& index) const
{
    QList<int> result;
    if(!index.isValid())
        return result;

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    if(!item)
        return result;

    return tracks_p(item);
}

QVariant CAbstractItemModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    return static_cast<CModelItem*>(index.internalPointer())->data(role, index.column());
}


QModelIndex CAbstractItemModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    CModelItem *child = static_cast<CModelItem*>(index.internalPointer());
    CModelItem *parentItem = child->parent();

    if(parentItem==rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QList<int> CAbstractItemModel::tracks_p(CModelItem *item) const
{
    QList<int> result;
    switch(item->itemType())
    {
        case CModelItem::ETrack:
        {
            result << item->id();
            break;
        }

        case CModelItem::EArtist:
        case CModelItem::EAlbum:
        case CModelItem::EPlaylist:
        {
            int count = item->rowCount();
            for(int i=0; i<count; i++)
                result.append(tracks_p(item->child(i)));
            break;
        }

        default: break;
    }

    return result;
}

void CAbstractItemModel::editTag()
{
    QList<int> tr = tracks(currentIndex());
    if(!tr.size())
        return;

    CTagEditDialog dialog(m_manager);
    dialog.setTracks(tr);
    dialog.exec();
}

void CAbstractItemModel::removeFromDatabase()
{

}

void CAbstractItemModel::setActions(QList<QAction*> list)
{
    QList<QAction*> tmp = m_itemView->actions();
    foreach(QAction *a, tmp)
        m_itemView->removeAction(a);

    m_itemView->addActions(list);
}
