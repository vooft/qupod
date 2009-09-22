#ifndef CMOUNTPOINTDIALOG_H
#define CMOUNTPOINTDIALOG_H

#include <QDialog>

namespace Ui {
    class CMountPointDialog;
}

class CMountPointDialog : public QDialog {
    Q_OBJECT
public:
    CMountPointDialog(QWidget *parent = 0);
    ~CMountPointDialog();
    QString mountPoint();

private slots:
    void browse();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CMountPointDialog *ui;
};

#endif // CMOUNTPOINTDIALOG_H
