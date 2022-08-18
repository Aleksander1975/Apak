#include "filter_editor.h"

FilterEditor::FilterEditor(QWidget *parent, zn1::Filter* filter) :
  QDialog(parent),
  ui(new Ui::FilterEditorDialog)
{
  ui->setupUi(this);

  if(filter) {

    ui->lineSystemName->setText(filter->marker());
    ui->dateTimeStart->setDateTime(filter->begin());
    ui->dateTimeEnd->setDateTime(filter->end());
    ui->lineSavePath->setText(filter->path());
    ui->lineFileName->setText(filter->file_name());

    setWindowTitle(QString("%1_%2_%3")
                   .arg(filter->marker())
                   .arg(filter->begin().toString(DEFAULT_DATETIME_FORMAT))
                   .arg(filter->end().toString(DEFAULT_DATETIME_FORMAT)));

    showMode = smEdit;

//    ui->checkManualSystemName->setEnabled(false);
//    ui->bnSelectSystem->setEnabled(false);

  }
  else {

    ui->lineSystemName->setText("");
    ui->dateTimeStart->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEnd->setDateTime(QDateTime::currentDateTime());
    ui->lineSavePath->setText("./");
    ui->lineFileName->setText(DEFAULT_FILE_NAME_TEMPLATE);

    setWindowTitle(QString("Новая задача"));

    showMode = smNew;
  }

  connect(ui->bnApply,  &QPushButton::clicked, this, &QDialog::accept);
  connect(ui->bnCancel, &QPushButton::clicked, this, &QDialog::reject);

  this->setModal(true);
  this->show();
}

FilterEditor::~FilterEditor()
{
  close();
  delete ui;
}

void FilterEditor::accept()
{
  try {

    /* делаем всякие проверки вводимых данных */
    if(ui->lineSystemName->text().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", "Имя системы не указано");
      return;
    }

    if(ui->lineFileName->text().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", "Имя файла не указано");
      return;
    }

    if(!QDir(ui->lineSavePath->text()).exists()) {
      QMessageBox::critical(this, "Ошибка", "Путь для сохранения указан неверно. Необходимо указать существующий каталог");
      return;
    }

    if(ui->dateTimeEnd->dateTime() < ui->dateTimeStart->dateTime()) {
      QMessageBox::critical(this, "Ошибка", "Период задан неверно");
      return;
    }

    m_filter.setId(QDateTime::currentMSecsSinceEpoch());
    m_filter.setMarker(ui->lineSystemName->text());

    //! отбрасываем миллисекунды, а то оператор == не работает для класса Interval
    QDateTime dt;

    dt.setDate(ui->dateTimeStart->date());
    dt.setTime(QTime(ui->dateTimeStart->time().hour(), ui->dateTimeStart->time().minute(), ui->dateTimeStart->time().second()));
    m_filter.setBegin(dt);

    dt.setDate(ui->dateTimeEnd->date());
    dt.setTime(QTime(ui->dateTimeEnd->time().hour(), ui->dateTimeEnd->time().minute(), ui->dateTimeEnd->time().second()));
    m_filter.setEnd(dt);

    m_filter.setPath(ui->lineSavePath->text());
    m_filter.setFileName(ui->lineFileName->text());

    QDialog::done(Accepted);

  }

  catch(SvException& e) {

    m_last_error = e.error;
    QDialog::done(Error);
  }

}


void FilterEditor::on_checkManualSystemName_toggled(bool checked)
{
  ui->lineSystemName->setEnabled(checked);
}

void FilterEditor::on_checkManuaFileName_toggled(bool checked)
{
  ui->lineFileName->setEnabled(checked);
}

void FilterEditor::on_bnSelectPath_clicked()
{
  QString p = QFileDialog::getExistingDirectory(this, "Выбор каталога");
  if(p.isEmpty())
    return;

  ui->lineSavePath->setText(p);
}

void FilterEditor::on_bnSelectSystem_clicked()
{
  ZNSystemSelectDialog* ssd = new ZNSystemSelectDialog(this, "config.json");

  switch (ssd->exec()) {

    case ZNSystemSelectDialog::Accepted:
    {
      ui->lineSystemName->setText(ssd->selectedRecord());

    }
    break;

    case ZNSystemSelectDialog::Error:

      QMessageBox::critical(this, "Ошибка", ssd->lastError());
      break;

    default:
      break;
  }

  delete ssd;
}
