#include "csortfiltermodel.h"

CSortFilterModel::CSortFilterModel()
{
}

bool CSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceRow);
    Q_UNUSED(sourceParent);
    return true;
}
