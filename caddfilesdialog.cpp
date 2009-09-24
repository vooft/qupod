#include <QFileDialog>
#include <QMenu>

#include "caddfilesdialog.h"
#include "ui_caddfilesdialog.h"

CAddFilesDialog::CAddFilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddFilesDialog)
{
    ui->setupUi(this);

    ui->tableView->setModel(&m_model);

    connect(ui->addBtn, SIGNAL(clicked()), this, SLOT(onAddFile()));
    connect(ui->actionAdd_directory, SIGNAL(triggered()), this, SLOT(addDirectory()));
    connect(ui->actionAdd_file, SIGNAL(triggered()), this, SLOT(addFiles()));
}

CAddFilesDialog::~CAddFilesDialog()
{
    delete ui;
}

void CAddFilesDialog::changeEvent(QEvent *e)
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

void CAddFilesDialog::onAddFile()
{
    QMenu menu(this);
    menu.addAction(ui->actionAdd_file);
    menu.addAction(ui->actionAdd_directory);

    menu.exec(QCursor::pos());
}

void CAddFilesDialog::addDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose directory containing media files"));
    if(dir.length())
        m_model.addDirectory(dir);
}

void CAddFilesDialog::addFiles()
{
    QStringList list = QFileDialog::getOpenFileNames(this,
                         tr("Choose files you want to transfer to player"), "",
                         tr("mp3 files (*.mp3)"));
    if(list.size())
        m_model.addFiles(list);
}
