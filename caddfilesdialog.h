#ifndef CADDFILESDIALOG_H
#define CADDFILESDIALOG_H

#include <QDialog>

#include "cmp3filemodel.h"

namespace Ui {
    class CAddFilesDialog;
}

class CAddFilesDialog : public QDialog {
    Q_OBJECT
public:
    CAddFilesDialog(QWidget *parent = 0);
    ~CAddFilesDialog();

private slots:
    void onAddFile();
    void addDirectory();
    void addFiles();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CAddFilesDialog *ui;
    CMp3FileModel m_model;
};

#endif // CADDFILESDIALOG_H
