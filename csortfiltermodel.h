#ifndef CSORTFILTERMODEL_H
#define CSORTFILTERMODEL_H

#include <QSortFilterProxyModel>

class CSortFilterModel : public QSortFilterProxyModel
{
public:
    CSortFilterModel();
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // CSORTFILTERMODEL_H
