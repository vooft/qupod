#ifndef CITEMMODEL_H
#define CITEMMODEL_H

#include <QAbstractItemModel>
#include <QAbstractItemView>

#include "cmodelitem.h"
#include "cipodmanager.h"

class CAbstractItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    CAbstractItemModel(QAbstractItemView*);
    ~CAbstractItemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex &) const { return 1; }
    QModelIndex parent(const QModelIndex &index) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QList<int> tracks(const QModelIndex&) const;

    static void setIpodManager(CIpodManager *man) { m_manager = man; }

    virtual QList<QAction*> actions(const QModelIndex&) const = 0;

public slots:
    virtual void fill() { }

protected slots:
    void removeFromDatabase();
    void editTag();

private:
    QAbstractItemView *m_itemView;

protected:
    enum Action
    {
        AEditTag,
        ARemoveFromDb,
        END_OF_ACTIONS
    };

    QMap<int, QAction*> m_actions;

    CModelItem* rootItem;
    static CIpodManager *m_manager;

    QList<int> tracks_p(CModelItem*) const;

    QItemSelectionModel* selectionModel_p() { return m_itemView->selectionModel(); }
    QModelIndex currentIndex() { return m_itemView->currentIndex(); }
    void setCurrentIndex(const QModelIndex &index) { m_itemView->setCurrentIndex(index); }
    void addAction(QAction *a) { m_itemView->addAction(a); }
    void setActions(QList<QAction*>);
};

#endif // CITEMMODEL_H
