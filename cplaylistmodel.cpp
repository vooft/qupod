#include "cplaylistmodel.h"

CPlaylistModel::CPlaylistModel(QAbstractItemView *view):
        CAbstractItemModel(view), m_currentTrack(-1), m_randomize(false)
{
    rootItem = new CModelItem(CModelItem::ENone, CModelItem::ETableItem, 0, 0);

    m_actions.insert(ADeleteFromPlaylist, new QAction(QIcon(":/icons/delete"), tr("Delete from playlist"), this));
    m_actions.value(ADeleteFromPlaylist)->setShortcut(Qt::Key_Delete);
    connect(m_actions.value(ADeleteFromPlaylist), SIGNAL(triggered()), this, SLOT(delFromPlaylist()));
    addAction(m_actions.value(ADeleteFromPlaylist));

    m_actions.insert(APlay, new QAction(QIcon(":/icons/play"), tr("Play"), this));
    connect(m_actions.value(APlay), SIGNAL(triggered()), this, SLOT(play()));

    m_actions.insert(AAddToQueue, new QAction(QIcon(":/icons/queue"), tr("Queue"), this));
    connect(m_actions.value(AAddToQueue), SIGNAL(triggered()), this, SLOT(addToQueue()));

    connect(view, SIGNAL(activated(QModelIndex)), m_actions.value(APlay), SIGNAL(triggered()));
}

int CPlaylistModel::columnCount(const QModelIndex &) const
{
    return CModelItem::END_OF_COLUMNS;
}

void CPlaylistModel::addTracks(QList<int> tracks)
{
    if(!tracks.size())
        return;

    m_tracks.append(tracks);
    foreach(int i, tracks)
    {
        CModelItem *item = new CModelItem(CModelItem::ETrack, CModelItem::ETableItem, i, rootItem);
        rootItem->appendChild(item);
    }
    QList<int> tmp = shuffle(tracks.size());
    bool need_play = !m_currentList.size();
    int count = m_currentList.size();
    foreach(int i, tmp)
        m_currentList.append(i+count);

    if(m_currentTrack==-1)
        m_currentTrack=0;
    //debug(QString("addTracks(): m_tracks.size==%1\tm_currentList==%2").arg(m_tracks.size()).arg(m_currentList.size()));
/*    for(int i=0; i<m_currentList.size(); i++)
        debug(QString("m_currentList.at(%1)==%2").arg(i).arg(m_currentList.at(i)));*/

    if(need_play)
        play(m_tracks.at(current()));

    reset();
}

void CPlaylistModel::fill()
{
    beginResetModel();
    if(rootItem)
        delete rootItem;

    rootItem = new CModelItem(CModelItem::ENone, CModelItem::ETableItem, 0, 0);

    foreach(int i, m_tracks)
    {
        CModelItem *item = new CModelItem(CModelItem::ETrack, CModelItem::ETableItem, i, rootItem);
        rootItem->appendChild(item);
    }
    endResetModel();

    //reset();
}

QVariant CPlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role!=Qt::DisplayRole)
        return QVariant();

    if(orientation==Qt::Horizontal)
        return CModelItem::description(section);

    QString result = QString::number(section+1);
    if(m_queue.contains(section))
        result += QString(" [%1]").arg(m_queue.indexOf(section)+1);

    return result;
}

QList<QAction*> CPlaylistModel::actions(const QModelIndex& index) const
{
    QList<QAction*> result;

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    switch(item->itemType())
    {
        case CModelItem::ETrack:
        {
            result << m_actions.value(CPlaylistModel::AAddToQueue);
            result << m_actions.value(CPlaylistModel::APlay);
            result << m_actions.value(CAbstractItemModel::AEditTag);
            result << m_actions.value(CPlaylistModel::ADeleteFromPlaylist);
        }
        default: break;
    }

    return result;
}

void CPlaylistModel::delFromPlaylist()
{
    QModelIndexList indexes = selectionModel_p()->selectedRows(0);
    QList<int> list;
    foreach(QModelIndex index, indexes)
        if(index.isValid())
            list.append(index.row());

    if(!list.size())
        return;

    debug("remove(): list.size()==" + QString::number(list.size()));

    qSort(list);

    bool need_play;
    int min_row = list.at(0);
    for(int i=list.size()-1; i>=0; i--)
    {
        int row = list.at(i);
        debug("remove(): row==" + QString::number(row));
        beginRemoveRows(QModelIndex(), row, row);
        m_tracks.removeAt(row);
        rootItem->removeChild(row);
        endRemoveRows();

        int curRow = m_currentList.indexOf(row);
        m_currentList.removeOne(row);
        int count = m_currentList.count();
        for(int i=0; i<count; i++)
            if(m_currentList.at(i)>row)
                m_currentList[i]--;

        count = m_queue.count();
        for(int i=0; i<count; i++)
            if(m_queue.at(i)>row)
                m_queue[i]--;

        if(curRow==m_currentTrack)
            need_play = true;

        if(m_currentTrack>curRow)
            m_currentTrack--;

        if(row<min_row)
            min_row = row;
    }

    if(m_tracks.size())
    {
        setCurrentIndex(index(min_row, 0));
        if(need_play)
            play();
    }
    else
    {
        m_currentTrack = -1;
        emit stop();
    }

    //reset();
}

void CPlaylistModel::play()
{
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    debug("play(): item->id()==" + QString::number(item->id()));
    m_currentTrack = index.row();
    debug("play(): m_currentTrack==" + QString::number(m_currentTrack));
    emit play(item->id());
}

int CPlaylistModel::next()
{
    if(!m_tracks.size())
        return -1;

    if(m_queue.size())
    {
        int i = m_queue.at(0);
        m_queue.removeAt(0);
        reset();
        m_currentTrack = m_currentList.indexOf(i);
        return m_tracks.at(i);
    }

    if(++m_currentTrack>=m_currentList.size())
    {
        m_currentTrack = 0;
        if(m_randomize)
            m_currentList = shuffle(m_tracks.size());
    }
    return m_tracks.at(current());
}

int CPlaylistModel::prev()
{
    if(!m_tracks.size())
        return -1;
    if(--m_currentTrack<0)
    {
        m_currentTrack = m_currentList.size()-1;
        if(m_randomize)
            m_currentList = shuffle(m_tracks.size());
    }
    return m_tracks.at(current());
}

void CPlaylistModel::setCurrentTrack(int id)
{
    m_currentTrack = m_currentList.indexOf(m_tracks.indexOf(id));
    if(m_currentTrack==-1)
    {
        debug("ACHTUNG! m_currentTrack==-1");
        m_currentTrack = 0;
    }
    reset();
}

QVariant CPlaylistModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role!=Qt::BackgroundRole && role!=Qt::ForegroundRole && role!=Qt::FontRole)
        return CAbstractItemModel::data(index, role);

    if(m_currentTrack==-1)
        return QVariant();

    CModelItem *item = static_cast<CModelItem*>(index.internalPointer());
    if(item->id()!=m_tracks.at(current()))
        return QVariant();

    if(role==Qt::FontRole)
    {
        QFont font = QApplication::font();
        font.setItalic(true);
        return font;
    }

    QPalette pal = QApplication::palette();
    QColor color;
    if(role==Qt::BackgroundRole)
        color = pal.color(QPalette::Link);
    else
        color = pal.color(QPalette::HighlightedText);

    return QBrush(color);
}

void CPlaylistModel::setShuffle(bool b)
{
    m_randomize = b;
    shuffle();
}

void CPlaylistModel::shuffle()
{
    debug("shuffle()");
    if(!m_tracks.size())
        return;

    int index = m_currentList.at(m_currentTrack);
    m_currentList = shuffle(m_tracks.size());
    m_currentTrack = m_currentList.indexOf(index);
}

QList<int> CPlaylistModel::shuffle(/*const QList<int> &list*/ int count) const
{
    QList<int> result;
    //int count = list.count();
    for(int i=0; i<count; i++)
        result.append(i);

    if(!m_randomize)
        return result;

    QTime midnight(0, 0, 0);
    qsrand(midnight.msecsTo(QTime::currentTime()));

    while(count>1)
    {
        count--;
        int k = ((double)qrand()/RAND_MAX)*count;
        int tmp = result.at(k);
        result[k] = result[count];
        result[count] = tmp;
    }

    return result;
}

bool CPlaylistModel::isShuffle()
{
    return m_randomize;
}

void CPlaylistModel::addToQueue()
{
    QModelIndex idx = currentIndex();
    if(!idx.isValid())
        return;

    m_queue.append(idx.row());
    reset();
}
