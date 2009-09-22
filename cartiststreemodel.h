#ifndef CARTISTSTREEMODEL_H
#define CARTISTSTREEMODEL_H

#include "cabstractitemmodel.h"

class CArtistsTreeModel : public CAbstractItemModel
{
    Q_OBJECT
public:
    CArtistsTreeModel(QAbstractItemView*);

    QList<QAction*> actions(const QModelIndex&) const;

public slots:
    void fill();

private slots:
    void addTracks();
    void setArtwork();

signals:
    void addTracks(QList<int>);

private:

    enum Actions
    {
        AAddTracks = CAbstractItemModel::END_OF_ACTIONS,
        ASetArtwork
    };

    CModelItem *VAItems;
    CModelItem *artistsItems;
    CModelItem *playlistItems;

    void showSetArtworkDialog(int);
};

#endif // CARTISTSTREEMODEL_H
