#include "ctageditdialog.h"
#include "ui_ctageditdialog.h"

CTagEditDialog::CTagEditDialog(CIpodManager *manager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CTagEditDialog),
    m_man(manager),
    m_currentTrack(-1)
{
    ui->setupUi(this);

    m_model = new CTagEditorModel(ui->tableView);
    ui->tableView->setModel(m_model);

    connect(m_model, SIGNAL(modelReset()), this, SLOT(adjustColumnsWidth()));
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(adjustColumnsWidth()));
    //connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(loadTrack()));
    connect(ui->tableView, SIGNAL(activated(QModelIndex)), this, SLOT(loadTrack()));
    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(save()));
}

CTagEditDialog::~CTagEditDialog()
{
    delete m_model;
    delete ui;
}

void CTagEditDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void CTagEditDialog::setTracks(const QList<int>& list)
{
    m_model->addTracks(list);
}

void CTagEditDialog::adjustColumnsWidth()
{
    if(!m_model->rowCount(QModelIndex()))
        return;

    ui->tableView->setColumnWidth(CModelItem::COL_Cover, 32); // авотхуй, там отступ есть

    int width = (ui->tableView->viewport()->width()-36)/(CModelItem::END_OF_COLUMNS-1);
    for(int i=0; i<CModelItem::END_OF_COLUMNS-1; i++)
        if(i!=CModelItem::COL_Cover)
            ui->tableView->setColumnWidth(i, width);
    ui->tableView->setColumnWidth(CModelItem::END_OF_COLUMNS-1,
                                   ui->tableView->viewport()->width()-36-width*(CModelItem::END_OF_COLUMNS-2));
}

void CTagEditDialog::resizeEvent(QResizeEvent *e)
{
    e->accept();
    adjustColumnsWidth();
}

void CTagEditDialog::showEvent(QShowEvent *e)
{
    e->accept();
    adjustColumnsWidth();
}

void CTagEditDialog::loadTrack()
{
    int id = m_model->currentTrack();
    if(id==-1)
        return;

    m_currentTrack = id;
    const CAudioTrack &track = m_man->tracks().at(id);

    ui->albumEdit->setEditText(track.album);
    ui->artistEdit->setEditText(track.artist);
    ui->genreEdit->setEditText(track.genre);
    ui->pathEdit->setText(track.ipod_path);
    ui->titleEdit->setText(track.title);
    ui->trackEdit->setValue(track.track_nr);

    const CAlbum &album = m_man->albums().at(track.album_id);
    ui->yearEdit->setValue(album.year);

    if(album.artwork!=-1)
        ui->artworkLbl->setPixmap(m_man->artwork(track.album_id));
    else
        ui->artworkLbl->setPixmap(QPixmap());
}

void CTagEditDialog::save()
{
    if(m_currentTrack==-1)
        return;

    CAudioTrack &track = m_man->tracks()[m_currentTrack];

    track.title = ui->titleEdit->text();
    track.artist = ui->artistEdit->currentText();
    track.album = ui->albumEdit->currentText();
    track.track_nr = ui->trackEdit->text().toInt();
    track.genre = ui->genreEdit->currentText();
    track.year = ui->yearEdit->text().toInt();
    track.isCompilation = ui->compilationBox->isChecked();

    m_man->saveTrack(m_currentTrack);
}
