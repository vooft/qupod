#include "ctageditormodel.h"

CTagEditorModel::CTagEditorModel(QAbstractItemView *view):
        CAbstractItemModel(view)
{
    rootItem = new CModelItem(CModelItem::ENone, CModelItem::ETableItem, 0, 0);
}

int CTagEditorModel::columnCount(const QModelIndex &) const
{
    return CModelItem::END_OF_COLUMNS;
}

void CTagEditorModel::addTracks(const QList<int> &tracks)
{
    if(!tracks.size())
        return;

    m_tracks.append(tracks);
    foreach(int i, tracks)
    {
        CModelItem *item = new CModelItem(CModelItem::ETrack, CModelItem::ETableItem, i, rootItem);
        rootItem->appendChild(item);
    }

    reset();
}

int CTagEditorModel::currentTrack()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return -1;

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    return item->id();
}

QVariant CTagEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role!=Qt::DisplayRole)
        return QVariant();

    if(orientation==Qt::Horizontal)
        return CModelItem::description(section);

    return QString::number(section+1);
}
