#ifndef CMP3FILEMODEL_H
#define CMP3FILEMODEL_H

#include <QAbstractTableModel>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

#include <QtDebug>

struct SMp3File
{
    SMp3File() { isCompilation = false; }
    TagLib::MPEG::File* ptr;
    QString path;
    QString artist;
    QString album;
    QString title;
    int year;
    bool isCompilation;
};

class CMp3FileModel : public QAbstractTableModel
{
public:
    CMp3FileModel();
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    int columnCount(const QModelIndex &) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

    void addFiles(const QStringList&, bool need_reset=true);
    void addDirectory(const QString&);

    QString getHeaderDescr(int) const;

    void resetTrack(int);

private:
    QList<SMp3File> m_files;
    int m_deep;

    enum
    {
        Artist,
        Album,
        Title,
        Filename,
        Year,
        Path,
        //Length,

        END_OF_COLUMNS
    };

    QString data(int, int) const;
    void debug(QString msg) const { qDebug() << "[Mp3FileModel] " + msg; }
};

#endif // CMP3FILEMODEL_H
