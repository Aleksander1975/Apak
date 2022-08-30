#include "set_period_dialog.h"
#include "ui_set_period_dialog.h"

SetPeriodDialog::SetPeriodDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SetPeriodDialog)
{
  ui->setupUi(this);

//  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SetPeriodDialog::accept);
//  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

SetPeriodDialog::SetPeriodDialog(QDateTime begin, QDateTime end) :
  ui(new Ui::SetPeriodDialog),
  m_begin(begin),
  m_end(end)
{
  ui->setupUi(this);

  if(m_begin.isValid()) {

    ui->dateBegin->setDate(m_begin.date());
    ui->timeBegin->setTime(m_begin.time());
  }

  if(m_end.isValid()) {

    ui->dateEnd->setDate(m_end.date());
    ui->timeEnd->setTime(m_end.time());
  }
}

SetPeriodDialog::~SetPeriodDialog()
{
  delete ui;
}

void SetPeriodDialog::accept()
{
  if(ui->dateBegin->date() < ui->dateEnd->date() ||
     ui->timeBegin->time() >= ui->timeEnd->time()) {

    QMessageBox::critical(this, "Error", "Неверно задан период");
    return;
  }

  m_begin.setDate(ui->dateBegin->date());
  m_begin.setTime(ui->timeBegin->time());

  m_end.setDate(ui->dateEnd->date());
  m_end.setTime(ui->timeEnd->time());

  QDialog::accept();

}
