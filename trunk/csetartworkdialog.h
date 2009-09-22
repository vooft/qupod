#ifndef CSETARTWORKDIALOG_H
#define CSETARTWORKDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include <QtDebug>

#include "cipodmanager.h"

namespace Ui {
    class CSetArtworkDialog;
}

class CSetArtworkDialog : public QDialog {
    Q_OBJECT
public:
    CSetArtworkDialog(int album_id, CIpodManager *manager, QWidget *parent = 0);
    ~CSetArtworkDialog();

    QPixmap pixmap() const;

private:
    bool setPixmap(const QByteArray&);

private slots:
    void openFile();
    void fetchUrl();
    void setNextFrame();
    void fetchFinished(QNetworkReply*);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void cancel();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CSetArtworkDialog *ui;
    int m_frameIndex;
    QTimer m_imageTimer;
    int m_albumId;
    CIpodManager *m_ipodManager;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    QString m_fetchCaption;
    bool m_fetchingProgress;
    QPixmap m_savedPixmap;

    void debug(QString msg) const { qDebug() << "[CSetArtworkDialog]" + msg; }
    void resetView(bool);
};

#endif // CSETARTWORKDIALOG_H
