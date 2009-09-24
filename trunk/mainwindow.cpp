#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <phonon/mediasource.h>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>

#include "csetartworkdialog.h"
#include "cmountpointdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_manager(0),
    m_artistsTreeModel(0),
    m_playlistModel(0)
{
    ui->setupUi(this);

    m_set = new QSettings("qupod.ini", QSettings::IniFormat, this);

    m_addFiles = new CAddFilesDialog(ui->tabWidget);
    ui->tabWidget->addTab(m_addFiles, QIcon(":/icons/add"), tr("Add files to iPod"));

    m_trayIcon = new QSystemTrayIcon(QIcon(":/icons/ipod"), this);
    m_trayIcon->setContextMenu(new QMenu(tr("Qupod"), this));
    m_trayIcon->show();

    if(!loadIpod_p())
    {
        QTimer::singleShot(0, this, SLOT(close()));
        return;
    }

    m_player = new CAudioPlayer(m_manager);

    m_artistsTreeModel = new CArtistsTreeModel(ui->treeView);
    m_artistsTreeModel->fill();
    ui->treeView->setModel(m_artistsTreeModel);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showArtistsTreeContextMenu(QPoint)));

    m_playlistModel = new CPlaylistModel(ui->tracksList);
    ui->tracksList->setModel(m_playlistModel);
    connect(m_playlistModel, SIGNAL(modelReset()), this, SLOT(adjustColumnsWidth()));
    connect(ui->tracksList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTracksListContextMenu(QPoint)));

    connect(m_artistsTreeModel, SIGNAL(addTracks(QList<int>)), m_playlistModel, SLOT(addTracks(QList<int>)));

    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(250);
    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(filterTree()));

    createActions();

    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(adjustColumnsWidth()));

    connect(ui->slider, SIGNAL(sliderMoved(int)), m_player, SLOT(seek(int)));

    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

    connect(m_playlistModel, SIGNAL(play(int)), m_player, SLOT(play(int)));
    connect(m_playlistModel, SIGNAL(stop()), ui->actionStop, SLOT(trigger()));

    connect(m_player, SIGNAL(currentPosChanged(int,QTime,QTime)),
            this, SLOT(changeSliderPos(int,QTime,QTime)));
    connect(m_player, SIGNAL(aboutToFinish()), this, SLOT(playNext()));
    connect(m_player, SIGNAL(currentTrackChanged(int)), m_playlistModel, SLOT(setCurrentTrack(int)));
    connect(m_player, SIGNAL(volumeLevel(qreal)), this, SLOT(setVolumeSlider()));
    connect(m_player, SIGNAL(trackStateChanged(int,int)), this, SLOT(trackStateChanged(int,int)));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolumeLevel(int)));

    QTimer::singleShot(0, this, SLOT(setVolumeSlider()));

    if(m_set->value("player/shuffle").isValid())
        ui->actionShuffle->setChecked(m_set->value("player/shuffle").toBool());
    if(m_set->value("MainWindow/geometry").isValid())
        restoreGeometry(m_set->value("MainWindow/geometry").toByteArray());
    if(m_set->value("MainWindow/splitter").isValid())
        ui->splitter->restoreState(m_set->value("MainWindow/splitter").toByteArray());
}

MainWindow::~MainWindow()
{
    if(isHidden())
        m_set->setValue("MainWindow/geometry", m_state);
    else
        m_set->setValue("MainWindow/geometry", saveGeometry());
    m_set->setValue("MainWindow/splitter", ui->splitter->saveState());
    m_set->setValue("ipod/path", m_mountPoint);

    if(m_playlistModel)
        m_set->setValue("player/shuffle", m_playlistModel->isShuffle());

    m_set->sync();

    if(m_manager)
        delete m_manager;
    if(m_artistsTreeModel)
        delete m_artistsTreeModel;
    if(m_playlistModel)
        delete m_playlistModel;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    adjustColumnsWidth();
}

bool MainWindow::loadIpod_p()
{
    bool flag = false;
    if(m_set->value("ipod/path").isValid())
    {
        m_mountPoint = m_set->value("ipod/path").toString();
        QDir dir(m_mountPoint);
        if(!dir.exists())
            m_mountPoint.clear();
    }

    while(!flag)
    {
        if(!m_mountPoint.length())
        {
            CMountPointDialog dialog(this);
            if(dialog.exec()==QDialog::Rejected)
            {
                break;
            }
            m_mountPoint = dialog.mountPoint();
        }

        if(!m_manager)
            m_manager = new CIpodManager;
        flag = m_manager->load(m_mountPoint);
        if(!flag)
        {
            QMessageBox::critical(this, tr("Error while loading database"),
                                  tr("Can't load database. Reason: %1. Please, choose another directory")
                                  .arg(m_manager->errorString()));
            m_mountPoint.clear();
        }
    }

    if(!flag)
        return false;

    CModelItem::setIpodManager(m_manager);
    CAbstractItemModel::setIpodManager(m_manager);

    return flag;
}

void MainWindow::createActions()
{
    QMenu *menu = new QMenu(tr("Qupod"), this);

    menu->addAction(ui->actionPrevious);
    menu->addAction(ui->actionPlay);
    menu->addAction(ui->actionPause);
    menu->addAction(ui->actionStop);
    menu->addAction(ui->actionNext);
    menu->addSeparator();
    menu->addAction(ui->actionExit);

    m_trayIcon->setContextMenu(menu);

    ui->shuffleBtn->setDefaultAction(ui->actionShuffle);
    ui->nextBtn->setDefaultAction(ui->actionNext);
    ui->prevBtn->setDefaultAction(ui->actionPrevious);
    ui->playBtn->setDefaultAction(ui->actionPlay);
    ui->pauseBtn->setDefaultAction(ui->actionPause);
    ui->stopBtn->setDefaultAction(ui->actionStop);

    connect(ui->actionShuffle, SIGNAL(toggled(bool)), m_playlistModel, SLOT(setShuffle(bool)));
    connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(playNext()));
    connect(ui->actionPause, SIGNAL(triggered()), m_player, SLOT(pause()));
    connect(ui->actionStop, SIGNAL(triggered()), m_player, SLOT(stop()));
    connect(ui->actionPrevious, SIGNAL(triggered()), this, SLOT(playPrev()));
    connect(ui->actionPlay, SIGNAL(triggered()), m_player, SLOT(play()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    /* Tray icon */
/*    m_trayIcon->contextMenu()->addAction(ui->playbackPrev);
    m_trayIcon->contextMenu()->addAction(ui->playbackPlay);
    m_trayIcon->contextMenu()->addAction(ui->playbackPause);
    m_trayIcon->contextMenu()->addAction(ui->playbackStop);
    m_trayIcon->contextMenu()->addAction(ui->playbackNext);
    m_trayIcon->contextMenu()->addSeparator();
    m_trayIcon->contextMenu()->addAction(ui->actionExit);*/

    /* Tracks list */
/*    m_tracksListActions.append(ui->actionPlaylistPlay);
    m_tracksListActions.append(ui->actionDelete);
    ui->tracksList->addActions(m_tracksListActions);*/

    /* Tree view */
/*    m_treeListActions.append(ui->actionSetArtworkTree);
    ui->treeView->addActions(m_treeListActions);*/
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger:
        {
            if(isVisible())
            {
                m_state = saveGeometry();
                hide();
            }
            else
            {
                restoreGeometry(m_state);
                show();
            }
            break;
        }
        default: break;
    }
}

/*void MainWindow::showSetArtworkTree()
{
    debug("showSetArtworkTree()");
    QModelIndex index = m_treeProxyModel->mapToSource(ui->treeView->currentIndex());
    int album_id = m_treeModel->albumId(index);
    if(album_id==-1)
        return;

    CSetArtworkDialog dialog(album_id, m_treeModel->manager(), this);
    if(dialog.exec()==QDialog::Accepted)
    {
        QPixmap pix = dialog.pixmap();
        m_treeModel->manager()->saveArtwork(album_id, pix);
    }
}

void MainWindow::showSetArtworkTracks()
{
    QModelIndex index = ui->tracksList->currentIndex();
}

void MainWindow::showSetArtworkPlaylis()
{
    QModelIndex index = ui->playlistTracksView->currentIndex();
}*/

void MainWindow::showArtistsTreeContextMenu(const QPoint& pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    QList<QAction*> actions = m_artistsTreeModel->actions(index);

    if(actions.size())
    {
        QMenu::exec(actions, QCursor::pos(), 0, this);
    }
}

void MainWindow::showTracksListContextMenu(const QPoint& pos)
{
    QModelIndex index = ui->tracksList->indexAt(pos);
    QList<QAction*> actions = m_playlistModel->actions(index);

    if(actions.size())
    {
        QMenu::exec(actions, QCursor::pos(), 0, this);
    }
}

void MainWindow::adjustColumnsWidth()
{
    if(!m_playlistModel->rowCount(QModelIndex()))
        return;

    ui->tracksList->setColumnWidth(CModelItem::COL_Cover, 32); // авотхуй, там отступ есть

    int width = (ui->tracksList->viewport()->width()-36)/(CModelItem::END_OF_COLUMNS-1);
    for(int i=0; i<CModelItem::END_OF_COLUMNS-1; i++)
        if(i!=CModelItem::COL_Cover)
            ui->tracksList->setColumnWidth(i, width);
    ui->tracksList->setColumnWidth(CModelItem::END_OF_COLUMNS-1,
                                   ui->tracksList->viewport()->width()-36-width*(CModelItem::END_OF_COLUMNS-2));
}

void MainWindow::changeSliderPos(int pos, const QTime &begin, const QTime &end)
{
    if(pos!=-1)
        ui->slider->setValue(pos);
    ui->beforeLbl->setText(begin.toString("mm:ss"));
    ui->afterLbl->setText("-" + end.toString("mm:ss"));
}

void MainWindow::playNext()
{
    int next = m_playlistModel->next();
    if(next==-1)
        return;
    m_player->play(next);
}

void MainWindow::playPrev()
{
    int prev = m_playlistModel->prev();
    if(prev==-1)
        return;
    m_player->play(prev);
}

void MainWindow::setVolumeLevel(int val)
{
    qreal level = (qreal)val/100;
    if(level!=m_player->volume())
        m_player->setVolume(level);
}

void MainWindow::setVolumeSlider()
{
    qreal val = m_player->volume();
    ui->volumeSlider->setValue(val*100);
    ui->volumeLbl->setText(tr("%1%").arg(val*100));
}

void MainWindow::trackStateChanged(int id, int s)
{
    QString title = tr("Qupod");
    QString state;
    switch(s)
    {
        case CAudioPlayer::SPaused: state = tr("paused"); break;
        case CAudioPlayer::SPlaying: state = tr("playing"); break;
        case CAudioPlayer::SStopped: state = tr("stopped"); break;
    }
    const CAudioTrack &track = m_manager->tracks().at(id);
    QString msg = QString("%1 - %2 [%3] [%4]")
                  .arg(track.artist)
                  .arg(track.title)
                  .arg(track.tracklen.toString("mm:ss"))
                  .arg(state);

    m_trayIcon->showMessage(title, msg, QSystemTrayIcon::Information, 5000);
    m_trayIcon->setToolTip(QString("%1: %2").arg(title, msg));
    setWindowTitle(QString("%1: %2").arg(title, msg));
}
