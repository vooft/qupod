#ifndef CTAGEDITDIALOG_H
#define CTAGEDITDIALOG_H

#include <QDialog>

#include "cipodmanager.h"
#include "ctageditormodel.h"

namespace Ui {
    class CTagEditDialog;
}

class CTagEditDialog : public QDialog {
    Q_OBJECT
public:
    CTagEditDialog(CIpodManager *, QWidget *parent = 0);
    ~CTagEditDialog();
    void setTracks(const QList<int>&);

private slots:
    void adjustColumnsWidth();
    void loadTrack();
    void save();

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent*);
    void showEvent(QShowEvent*);

private:
    Ui::CTagEditDialog *ui;
    CIpodManager *m_man;
    CTagEditorModel *m_model;
    int m_currentTrack;
};

#endif // CTAGEDITDIALOG_H
