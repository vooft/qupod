#ifndef CTAGEDITORMODEL_H
#define CTAGEDITORMODEL_H

#include "cabstractitemmodel.h"

#include "cipodmanager.h"

class CTagEditorModel : public CAbstractItemModel
{
    Q_OBJECT
public:
    CTagEditorModel(QAbstractItemView*);

    void addTracks(const QList<int> &tracks);
    int currentTrack();
    void fill() { }
    QList<QAction*> actions(const QModelIndex&) const { return QList<QAction*>(); }
    int columnCount(const QModelIndex&) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<int> m_tracks;
};

#endif // CTAGEDITORMODEL_H
