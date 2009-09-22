#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "csetartworkdialog.h"
#include "ui_csetartworkdialog.h"

CSetArtworkDialog::CSetArtworkDialog(int album_id, CIpodManager *manager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSetArtworkDialog), m_frameIndex(0),
    m_albumId(album_id), m_ipodManager(manager),
    m_manager(new QNetworkAccessManager(this)), m_fetchingProgress(false)
{
    ui->setupUi(this);
    connect(ui->browseBtn, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui->fetchBtn, SIGNAL(clicked()), this, SLOT(fetchUrl()));

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fetchFinished(QNetworkReply*)));

    m_imageTimer.setInterval(100);
    connect(&m_imageTimer, SIGNAL(timeout()), this, SLOT(setNextFrame()));

    if(m_ipodManager->albums().at(m_albumId).artwork!=-1)
        ui->artworkLbl->setPixmap(m_ipodManager->artwork(m_albumId));

    m_fetchCaption = ui->fetchBtn->text();
    connect(this, SIGNAL(rejected()), this, SLOT(cancel()));
}

CSetArtworkDialog::~CSetArtworkDialog()
{
    delete m_manager;
    delete ui;
}

void CSetArtworkDialog::changeEvent(QEvent *e)
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

void CSetArtworkDialog::openFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "",
                                                    tr("Images (*.png *.jpg *.bmp);;Any file (*.*)"));

    if(!filename.length())
        return;

    QFile file(filename);

    bool result = file.open(QIODevice::ReadOnly);
    if(result)
        result = setPixmap(file.readAll());

    if(result)
        ui->pathEdit->setText(filename);
}

void CSetArtworkDialog::fetchUrl()
{
    if(m_fetchingProgress)
    {
        m_reply->abort();
        return;
    }

    debug("Downloading url: " + ui->urlEdit->text());
    m_reply = m_manager->get(QNetworkRequest(QUrl(ui->urlEdit->text())));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

    if(ui->artworkLbl->pixmap())
        m_savedPixmap = *(ui->artworkLbl->pixmap());

    m_imageTimer.start();
    m_fetchingProgress = true;
}

void CSetArtworkDialog::setNextFrame()
{
    QString filename = QString(":/busy/frame%1").arg(m_frameIndex, 2, 10, QChar('0'));
    if(!QFile::exists(filename))
        m_frameIndex = 0;

    filename = QString(":/busy/frame%1").arg(m_frameIndex, 2, 10, QChar('0'));

    ui->artworkLbl->setPixmap(QPixmap(filename));
    m_frameIndex++;
}

bool CSetArtworkDialog::setPixmap(const QByteArray &data)
{
    QPixmap pixmap;
    pixmap.loadFromData(data);
    if(pixmap.isNull())
    {
        QMessageBox::critical(this, tr("Can't set image."), tr("Wrong image data."));
        return false;
    }

    if(pixmap.height()>320 || pixmap.width()>320)
    {
        if(pixmap.height()>pixmap.width())
            ui->artworkLbl->setPixmap(pixmap.scaledToHeight(320, Qt::SmoothTransformation));
        else
            ui->artworkLbl->setPixmap(pixmap.scaledToWidth(320, Qt::SmoothTransformation));
    }
    else
        ui->artworkLbl->setPixmap(pixmap);

    return true;
}

void CSetArtworkDialog::fetchFinished(QNetworkReply *reply)
{
    if(reply->error())
    {
        QMessageBox::critical(this, tr("Can't fetch image."), tr("Error: %1").arg(reply->errorString()));
        reply->deleteLater();
    }
    else
    {
        QByteArray data = reply->readAll();
        setPixmap(data);
    }
    resetView(reply->error());
}

void CSetArtworkDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    ui->fetchBtn->setText(tr("Stop [%2k/%3k]").arg(bytesReceived/1024).arg(bytesTotal/1024));
}

void CSetArtworkDialog::resetView(bool reset_image)
{
    m_imageTimer.stop();
    if(reset_image)
        ui->artworkLbl->setPixmap(m_savedPixmap);
    ui->fetchBtn->setText(m_fetchCaption);

    m_fetchingProgress = false;
}

void CSetArtworkDialog::cancel()
{
    if(m_fetchingProgress)
    {
        m_reply->disconnect();
        m_reply->abort();
    }
}

QPixmap CSetArtworkDialog::pixmap() const
{
    return *ui->artworkLbl->pixmap();
}
