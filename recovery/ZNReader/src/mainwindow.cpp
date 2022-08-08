#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  ui->comboReadType->clear();
  ui->comboReadType->addItem("Все", QVariant("all"));
  ui->comboReadType->addItem("Выбранные задачи", QVariant("tasks"));
  ui->comboReadType->setCurrentIndex(0);

  m_config_file_name = "config.json"; //QString("%1%2%3").arg(qApp->applicationDirPath()).arg(QDir::separator()).arg(qApp->applicationName() + ".json");

  QDir::setCurrent(qApp->applicationDirPath());

  QString error;
  if(!loadConfig(error)) {

    QMessageBox::critical(this, "Ошибка", error);
    return;
  }

  ui->lineZNIP->setText(m_config.readerParams.host.toString());
  ui->spinZNPort->setValue(m_config.readerParams.port);
  ui->lineZNZoneName->setText(m_config.readerParams.zone);
  ui->spinZNZoneSize->setValue(m_config.readerParams.zone_size_gb);
  ui->spinZNTimeout->setValue(m_config.readerParams.timeout);
  ui->checkRequestZoneSize->setChecked(m_config.readerParams.ask_zone_size);
  ui->lineSaveFilePath->setText(m_config.zn_data_file);

  sv::log::Options logopt;
  logopt.enable = true;
  logopt.level = m_config.log_level;

  m_logger = new sv::SvWidgetLogger{logopt, ui->textLog};

  setButtonsStateStopped();

  ui->textLog->document()->setMaximumBlockCount(2000);

}

bool MainWindow::loadConfig(QString& error)
{
  QFile json_file(m_config_file_name);

  try {

    if(!json_file.open(QIODevice::ReadWrite))
      throw SvException(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QByteArray json = json_file.readAll();
    QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    m_config = zn1::RecoveryConfig::fromJsonObject(jdoc.object());

    json_file.close();
    return true;

  } catch (SvException e) {

    if(json_file.isOpen())
      json_file.close();

    error = e.error;

    return false;

  }
}

bool MainWindow::saveConfig(QString& error)
{
  m_config.readerParams.host = QHostAddress(ui->lineZNIP->text());
  m_config.readerParams.port = ui->spinZNPort->value();
  m_config.readerParams.zone = ui->lineZNZoneName->text();
  m_config.readerParams.zone_size_gb = ui->spinZNZoneSize->value();
  m_config.readerParams.timeout = ui->spinZNTimeout->value();
  m_config.readerParams.ask_zone_size = ui->checkRequestZoneSize->isChecked();
  m_config.zn_data_file = ui->lineSaveFilePath->text();

  QFile config_file(m_config_file_name);

  try {

    if(!config_file.open(QIODevice::WriteOnly))
      throw SvException(config_file.errorString());

    config_file.write(m_config.toByteArray());

    if(config_file.error() != QFile::NoError)
      throw SvException(config_file.errorString());

  } catch (SvException e) {

    if(config_file.isOpen())
      config_file.close();

    error = e.error;

    return false;

  }

  config_file.close();
  return true;
}

MainWindow::~MainWindow()
{
  QString error;
  if(!saveConfig(error))
    QMessageBox::critical(this, "Ошибка", error);

  delete m_logger;

  delete ui;
}

void MainWindow::on_bnSelectZN_clicked()
{
  QString fn = QFileDialog::getOpenFileName(this, "Выбор конфигурации", QString(), "Файлы конфигурации (*.json);;Все файлы (*.*)");

  if(fn.isEmpty())
    return;

  ZNSelector* znr = new ZNSelector(this, fn);

  switch (znr->exec()) {

    case ZNSelector::Accepted:
    {
      ui->lineZNIP->setText(znr->selectedRecord()->host);
      ui->spinZNPort->setValue(znr->selectedRecord()->port);
      ui->lineZNZoneName->setText(znr->selectedRecord()->zone);

      break;
    }

    case ZNSelector::Error:

      QMessageBox::critical(this, "Ошибка", znr->lastError());
      break;

    default:
      break;
  }

  delete znr;
}

void MainWindow::on_checkManualZNEdit_toggled(bool checked)
{
    ui->lineZNIP->setEnabled(checked);
    ui->lineZNZoneName->setEnabled(checked);
    ui->spinZNPort->setEnabled(checked);
    ui->spinZNZoneSize->setEnabled(checked && !ui->checkRequestZoneSize->isChecked());
}

void MainWindow::on_bnStart_clicked()
{
  ui->progressBar->setValue(0);

  if(ui->lineSaveFilePath->text().isEmpty()) {

    QMessageBox::critical(this, "Ошибка", "Укажите файл, в который будут сохраняться данные");
    return;
  }

  // формируем и сохраняем json файл конфигурации
  QString error;
  if(!saveConfig(error)) {

    QMessageBox::critical(this, "Ошибка", error);
    return;
  }

  bool ok;
  QString pass = QInputDialog::getText(this, tr("Авторизация для чтения данных"),
                                       tr("Введите пароль:"), QLineEdit::Normal,
                                       "QWERTY", &ok);
  if (!ok || pass.isEmpty()) {

    QMessageBox::critical(this, "Ошибка", "Не указан пароль. Продолжение операции невозможно.");
    return;
  }

  //! создаем объект-читатель
  m_reader = new zn1::ZNReader();

  if(!m_reader->configure(m_config, pass)) {

    QMessageBox::critical(this, "Ошибка", m_reader->lastError());
    return;
  }

  connect(m_reader, &zn1::ZNReader::started,  this,             &MainWindow::setButtonsStateStarted);
  connect(m_reader, &zn1::ZNReader::finished, this,             &MainWindow::setButtonsStateStopped);
  connect(m_reader, &zn1::ZNReader::message,  this,             &MainWindow::message);

  connect(this,     &MainWindow::stop,        m_reader,         &zn1::ZNReader::stop);
//  connect(m_reader, &zn1::ZNReader::finished, m_reader,         &zn1::ZNReader::quit);

  connect(m_reader, &zn1::ZNReader::finished, m_reader,         &zn1::ZNReader::deleteLater);

  connect(m_reader, &zn1::ZNReader::zonesize, ui->progressBar,     &QProgressBar::setMaximum);
  connect(m_reader, &zn1::ZNReader::total,   ui->progressBar,     &QProgressBar::setValue);
  connect(m_reader, &zn1::ZNReader::partsize, ui->progressBarPart, &QProgressBar::setMaximum);
  connect(m_reader, &zn1::ZNReader::parted,   ui->progressBarPart, &QProgressBar::setValue);

  connect(m_reader, &zn1::ZNReader::total, this, &MainWindow::setProgressZoneSize);
  connect(m_reader, &zn1::ZNReader::parted, this, &MainWindow::setCurrentProgress);

  m_reader->start();

}

void MainWindow::setButtonsStateStarted()
{
  ui->bnStart->setEnabled(false);
  ui->bnStop->setEnabled(true);
  ui->gbZNParams->setEnabled(false);
  ui->gbCommonParams->setEnabled(false);
  qApp->processEvents();
}

void MainWindow::setButtonsStateStopped()
{
  ui->bnStart->setEnabled(true);
  ui->bnStop->setEnabled(false);
  ui->gbZNParams->setEnabled(true);
  ui->gbCommonParams->setEnabled(true);
  qApp->processEvents();
}

void MainWindow::setProgressZoneSize(int size)
{
  ui->lblTotalSize->setText(QString::number(size));
  qApp->processEvents();
//  ui->progressBar->setRange(0, size); //Maximum(size);
//  qDebug() << "set size" << size;
}

void MainWindow::setCurrentProgress(int size)
{
  ui->lblPartZize->setText(QString::number(size));
  qApp->processEvents();
//  ui->progressBar->setValue(size);
//  qDebug() << "set value" << size;
}

void MainWindow::on_bnSelectFile_clicked()
{
  QString full_file_name = QFileDialog::getSaveFileName(this, "Выберите файл для сохранения данных", QString(), "ZN Data (*.zndata);;Все файлы (*.*)");

  if(!full_file_name.isEmpty())
    ui->lineSaveFilePath->setText(full_file_name.endsWith(".zndata") ? full_file_name : QString("%1.zndata").arg(full_file_name));

}

void MainWindow::on_bnStop_clicked()
{
  emit stop();
}

void MainWindow::message(const QString msg, int level, int type)
{
  if(m_logger && level <= m_logger->options().level)
    *m_logger << sv::log::TimeZZZ
              << sv::log::Level(level)
              << sv::log::MessageTypes(type)
              << msg
              << sv::log::endl;
}

void MainWindow::on_checkRequestZoneSize_toggled(bool checked)
{
  ui->spinZNZoneSize->setEnabled(!checked && ui->checkManualZNEdit->isChecked());
}
