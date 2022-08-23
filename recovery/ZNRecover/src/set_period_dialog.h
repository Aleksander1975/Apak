#ifndef SET_PERIOD_DIALOG_H
#define SET_PERIOD_DIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QMessageBox>

namespace Ui {
  class SetPeriodDialog;
  }

class SetPeriodDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SetPeriodDialog(QWidget *parent = 0);
    ~SetPeriodDialog();

    QDateTime begin() { return m_begin; }
    QDateTime end()   { return m_end;   }

  private:
    Ui::SetPeriodDialog *ui;

    QDateTime m_begin = QDateTime();
    QDateTime m_end   = QDateTime();

    void accept() override;
};

#endif // SET_PERIOD_DIALOG_H
