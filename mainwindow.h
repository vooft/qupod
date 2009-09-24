#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QSystemTrayIcon>

#include <QtDebug>

#include "cipodmanager.h"
#include "cartiststreemodel.h"
#include "cplaylistmodel.h"
#include "caudioplayer.h"
#include "caddfilesdialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
/*    void addTracksFromTree(const QModelIndex &);
    void addTracksFromPlaylist(const QModelIndex &);
    void changeTitle(const QString&);
    void delFromPlaylist();
    void setVolumeSlider();
    void setVolumeLevel(int);
    void loadPlaylist(const QModelIndex&);

    void showSetArtworkTree();
    void showSetArtworkTracks();
    void showSetArtworkPlaylis();

    void filterTree();*/

    void changeSliderPos(int pos, const QTime &begin, const QTime &end);
    void playNext();
    void playPrev();

    void showArtistsTreeContextMenu(const QPoint&);
    void showTracksListContextMenu(const QPoint&);

    void adjustColumnsWidth();

    void setVolumeLevel(int);
    void setVolumeSlider();

    void trackStateChanged(int, int);

    void trayActivated(QSystemTrayIcon::ActivationReason);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *m_trayIcon;
    QSettings *m_set;

    QByteArray m_state;

    QString m_mountPoint;

    QList<QAction*> m_tracksListActions;
    QList<QAction*> m_treeListActions;

    CIpodManager *m_manager;
    CArtistsTreeModel *m_artistsTreeModel;
    CPlaylistModel *m_playlistModel;
    CAudioPlayer *m_player;
    CAddFilesDialog *m_addFiles;

    QTimer *m_filterTimer;

    void debug(QString msg) const { qDebug() << "[MainWindow]" + msg; }
    void createActions();
    bool loadIpod_p();
};

#endif // MAINWINDOW_H
