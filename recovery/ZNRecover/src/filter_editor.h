#ifndef FILTER_EDITOR_H
#define FILTER_EDITOR_H

#include <QDialog>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDir>

//#include "ui_task_editor_dialog.h"

#include "../../../../../job/svlib/SvException/svexception.h"
#include "../../recovery_defs.h"
//#include "zn_select_system_dialog.h"

namespace Ui {
  class FilterEditorDialog;
}

class FilterEditor : public QDialog
{
    Q_OBJECT

  public:
    enum Result { Accepted = QDialog::Accepted, Rejected = QDialog::Rejected, Error = -1 };
    enum ShowMode:bool { smNew = true, smEdit = false };

    explicit FilterEditor(QWidget *parent, zn1::FineFilter* filter = nullptr);
    ~FilterEditor();

    zn1::FineFilter* task() { return &m_filter; }

    QString lastError() { return m_last_error; }

    int showMode;

  public slots:
    void accept() Q_DECL_OVERRIDE;

private slots:
    void on_checkManualSystemName_toggled(bool checked);

    void on_checkManuaFileName_toggled(bool checked);

    void on_bnSelectPath_clicked();

    void on_bnSelectSystem_clicked();

private:
    Ui::FilterEditorDialog *ui;

    zn1::FineFilter m_filter;

    QString m_last_error = "";

};

#endif // FILTER_EDITOR_H
