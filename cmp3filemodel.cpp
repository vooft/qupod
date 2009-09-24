#include <QDir>
#include <QFileInfo>
#include "cmp3filemodel.h"

CMp3FileModel::CMp3FileModel()
{
    m_deep = 0;
}

int CMp3FileModel::rowCount(const QModelIndex &) const
{
    return m_files.size();
}

QVariant CMp3FileModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role!=Qt::DisplayRole)
        return QVariant();

    if(index.row()>m_files.count() || index.column()>=END_OF_COLUMNS)
        return QVariant();

    return data(index.row(), index.column());
}

int CMp3FileModel::columnCount(const QModelIndex &) const
{
    return END_OF_COLUMNS;
}

void CMp3FileModel::addDirectory(const QString &str)
{
    debug("addDirectory(): str==" + str);
    QDir dir(str);
    QStringList list;
    foreach(QString name, dir.entryList())
    {
        QFileInfo fi(dir.absoluteFilePath(name));
        if(fi.isDir())
        {
            if(!(name=="." || name==".."))
            {
                m_deep++;
                addDirectory(dir.absoluteFilePath(name));
                m_deep--;
            }
        }
        else if(fi.suffix()=="mp3")
            list << dir.absoluteFilePath(name);
    }
    addFiles(list, false);
    if(!m_deep)
        reset();
}

void CMp3FileModel::addFiles(const QStringList &list, bool need_reset)
{
    foreach(QString name, list)
    {
        QFileInfo fi(name);
        if(fi.isDir())
            addDirectory(name);
        else
        {
            debug("addFiles: current file " + name);
            SMp3File file;
            file.ptr = new TagLib::MPEG::File(QFile::encodeName(name));
            file.path = name;
            m_files.append(file);
            resetTrack(m_files.size()-1);
        }
    }
    if(need_reset)
        reset();
}

void CMp3FileModel::resetTrack(int id)
{
    if(id>=m_files.size())
        return;
    SMp3File &file = m_files[id];
    TagLib::Tag *tag = file.ptr->tag();
    file.album = TStringToQString(tag->album());
    file.artist = TStringToQString(tag->artist());
    file.title = TStringToQString(tag->title());
    file.year = tag->year();
}

QString CMp3FileModel::data(int id, int col) const
{
    const SMp3File &file = m_files.at(id);
    switch(col)
    {
        case Path: return file.path;
        case Artist: return file.artist;
        case Album: return file.album;
        case Title: return file.title;
        case Year: return QString::number(file.year);
    }
    return QString();
}

QVariant CMp3FileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation==Qt::Vertical)
        return QString::number(section+1);

    if(role!=Qt::DisplayRole)
        return QVariant();

    return getHeaderDescr(section);
}

QString CMp3FileModel::getHeaderDescr(int section) const
{
    switch(section)
    {
        case Filename: return tr("Filename");
        case Path: return tr("Path");
        case Year: return tr("Year");
        case Artist: return tr("Artist");
        case Album: return tr("Album");
        case Title: return tr("Title");
    }
    return QString();
}
