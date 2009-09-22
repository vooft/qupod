#include <QFileDialog>
#include <QPushButton>

#include "cmountpointdialog.h"
#include "ui_cmountpointdialog.h"

CMountPointDialog::CMountPointDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CMountPointDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(ui->browseBtn, SIGNAL(clicked()), this, SLOT(browse()));
}

CMountPointDialog::~CMountPointDialog()
{
    delete ui;
}

void CMountPointDialog::changeEvent(QEvent *e)
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

void CMountPointDialog::browse()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose iPod mount point"));
    if(path.length())
        ui->mpEdit->setText(path);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

QString CMountPointDialog::mountPoint()
{
    return ui->mpEdit->text();
}
