#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connect(ui->bnBackToBegin1, &QPushButton::clicked, this, &MainWindow::backToBegin);
  connect(ui->bnBackToBegin2, &QPushButton::clicked, this, &MainWindow::backToBegin);

  connect(ui->bnStart, &QPushButton::clicked, this, &MainWindow::start);

//  ui->comboReadType->clear();
//  ui->comboReadType->addItem("Все", QVariant("all"));
//  ui->comboReadType->addItem("Выбранные задачи", QVariant("tasks"));
//  ui->comboReadType->setCurrentIndex(0);

  m_config_file_name = "config.json";

  QDir::setCurrent(qApp->applicationDirPath());

  QString error;
  if(!loadConfig(error)) {

    QMessageBox::critical(this, "Ошибка", error);
    return;
  }

  ui->lineSaveFilePath->setText(m_config.zn_data_file);
  ui->lineZNIP->setText(m_config.readerParams.host.toString());
  ui->spinZNPort->setValue(m_config.readerParams.port);
  ui->lineZNZoneName->setText(m_config.readerParams.zone);
  ui->cbZoneSizeMode->setCurrentIndex(0);

//  ui->spinZNZoneSize->setValue(m_config.readerParams.zone_size_gb);
//  ui->spinZNTimeout->setValue(m_config.readerParams.timeout);
//  ui->checkRequestZoneSize->setChecked(m_config.readerParams.ask_zone_size);

  sv::log::Options logopt;
  logopt.enable = true;
  logopt.level = m_config.log_level;

  m_logger = new sv::SvWidgetLogger{logopt, ui->textLog};

  setButtonsStateStopped();

  ui->textLog->document()->setMaximumBlockCount(2000);

  QLabel *lblStatus1 = new QLabel(statusBar());
  QLabel *lblStatus2 = new QLabel(this);

  this->statusBar()->addPermanentWidget(lblStatus1);
  qDebug() << lblStatus1->parent()->objectName();

  this->statusBar()->addWidget(lblStatus2);

//  ui->progressBar->setVisible(false);

//  lblStatus1->setText(QString("Сегмент: %1").arg(11));
//  lblStatus2->setText(QString("Общий: %1").arg(222));

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
//  m_config.readerParams.zone_size_gb = ui->spinZNZoneSize->value();
//  m_config.readerParams.timeout = ui->spinZNTimeout->value();
//  m_config.readerParams.ask_zone_size = ui->checkRequestZoneSize->isChecked();
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

/*void MainWindow::on_bnSelectZN_clicked()
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
}*/

/*void MainWindow::on_checkManualZNEdit_toggled(bool checked)
{
    ui->lineZNIP->setEnabled(checked);
    ui->lineZNZoneName->setEnabled(checked);
    ui->spinZNPort->setEnabled(checked);
    ui->spinZNZoneSize->setEnabled(checked && !ui->checkRequestZoneSize->isChecked());
}*/

void MainWindow::start()
{
  if(l_state == Started) {

    emit stop();

  }
  else if (l_state == Stopped) {

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

    connect(m_reader, &zn1::ZNReader::zonesize, ui->progressBar,  &QProgressBar::setMaximum);
    connect(m_reader, &zn1::ZNReader::total,   ui->progressBar,   &QProgressBar::setValue);

  //  connect(m_reader, &zn1::ZNReader::partsize, ui->progressBarPart, &QProgressBar::setMaximum);
  //  connect(m_reader, &zn1::ZNReader::parted,   ui->progressBarPart, &QProgressBar::setValue);

//    connect(m_reader, &zn1::ZNReader::total, this, &MainWindow::setProgressZoneSize);
//    connect(m_reader, &zn1::ZNReader::parted, this, &MainWindow::setCurrentProgress);
    connect(m_reader, &zn1::ZNReader::loaded, this, &MainWindow::setStatusBarText);

    m_reader->start();

  }
}

void MainWindow::setButtonsStateStarted()
{
  ui->gbZNParams->setEnabled(false);
  ui->bnBackToBegin1->setEnabled(false);
  ui->bnToStep3->setEnabled(false);

  ui->progressBar->setVisible(true);

  ui->bnStart->setText("Отмена");

  l_state = Started;

//  qApp->processEvents();
}

void MainWindow::setButtonsStateStopped()
{
  ui->gbZNParams->setEnabled(true);
  ui->bnBackToBegin1->setEnabled(true);
  ui->bnToStep3->setEnabled(true);

  ui->progressBar->setVisible(false);

  ui->bnStart->setText("Загрузить");

  l_state = Stopped;

//  qApp->processEvents();
}

/*void MainWindow::setProgressZoneSize(int size)
{
//  qDebug() << 10 << size;
//  statusTip().
//  reinterpret_cast<QLabel*>(statusBar()->children().at(0))->setText(QString("Общий: %1 Мб").arg(size));
//  lblStatus1->setText("dsdsd"); // QString("Общий: %1 Мб").arg(size));
  qDebug() << 11;
//  qApp->processEvents();
}*/

void MainWindow::setStatusBarText(const QString& text)
{
  statusBar()->showMessage(text);
}

/*void MainWindow::setCurrentProgress(int size)
{
  ui->label_5->setText(QString("Сегмент: %1 Мб").arg(size));
  qDebug() << 12;
//  qApp->processEvents();
}*/

void MainWindow::on_bnSelectFile_clicked()
{
  QString full_file_name = QFileDialog::getSaveFileName(this, "Выберите файл для сохранения данных", QString(), "ZN Data (*.zndata);;Все файлы (*.*)");

  if(!full_file_name.isEmpty())
    ui->lineSaveFilePath->setText(full_file_name.endsWith(".zndata") ? full_file_name : QString("%1.zndata").arg(full_file_name));

}

/*void MainWindow::on_bnStop_clicked()
{
  emit stop();
}*/

void MainWindow::message(const QString msg, int level, int type)
{
  if(m_logger && level <= m_logger->options().level)
    *m_logger << sv::log::TimeZZZ
              << sv::log::Level(level)
              << sv::log::MessageTypes(type)
              << msg
              << sv::log::endl;
}

/*void MainWindow::on_checkRequestZoneSize_toggled(bool checked)
{
  ui->spinZNZoneSize->setEnabled(!checked && ui->checkManualZNEdit->isChecked());
}*/

void MainWindow::on_radioLoadData_toggled(bool checked)
{
  ui->bnToNextStep->setEnabled(checked && !ui->lineSaveFilePath->text().isEmpty());
}

void MainWindow::on_radioDoNotLoad_toggled(bool checked)
{
  ui->bnToNextStep->setEnabled(checked && !ui->lineSaveFilePath->text().isEmpty());
}

void MainWindow::on_radioTraining_toggled(bool checked)
{
  ui->bnToNextStep->setEnabled(checked && !ui->lineSaveFilePath->text().isEmpty());
}

void MainWindow::on_bnToNextStep_clicked()
{
  if(ui->radioLoadData->isChecked())
    ui->stackedWidget->setCurrentIndex(1);

  else if(ui->radioDoNotLoad->isChecked())
    ui->stackedWidget->setCurrentIndex(2);

  else
  {}
}

void MainWindow::backToBegin()
{
  ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_bnAddTask_clicked()
{
  TaskEditor* te = new TaskEditor(this);

  switch (te->exec()) {

    case TaskEditor::Accepted:
    {
      QString error = QString();
      if(!addTask(te->task(), error))
        QMessageBox::critical(this, "Ошибка", error);

    }
    break;

    case TaskEditor::Error:

      QMessageBox::critical(this, "Ошибка", te->lastError());
      break;

    default:
      break;
  }

  delete te;
}

void MainWindow::on_bnEditTask_clicked()
{
  TaskEditor* te = nullptr;

  try {
  //  qDebug() << "curr row" <<  ui->tableWidget->currentRow();
    if(ui->tableWidget->currentRow() < 0)
      throw SvException("No rows. Nothing to edit");

    int current_row = ui->tableWidget->currentRow();

    bool ok;
    qint64 current_task_id = ui->tableWidget->item(ui->tableWidget->currentItem()->row(), 0)->data(Qt::UserRole).toLongLong(&ok);
    if(!ok)
      throw SvException("Error on trying to determine task id from item data.");

    if(!m_tasks.contains(current_task_id))
      throw SvException("current task_id not found. on_bnEditTask_clicked");

    zn1::Task& current_task = m_tasks[current_task_id];

    te = new TaskEditor(this, &current_task);

    switch (te->exec()) {

      case TaskEditor::Accepted:
      {
        foreach (auto task, m_tasks.values()) {

          if(task.id() == current_task_id)
            continue;

          if(te->task()->save_path() == task.save_path())
            throw SvException(QString("Такой путь для извлечения данных уже задан. Недопустимо указывать один путь для разных задач."));

        }

        ui->tableWidget->item(current_row, 0)->setText(te->task()->marker());
        ui->tableWidget->item(current_row, 1)->setText(te->task()->begin().toString("dd.MM.yyyy hh:mm:ss"));
        ui->tableWidget->item(current_row, 2)->setText(te->task()->end().toString("dd.MM.yyyy hh:mm:ss"));
        ui->tableWidget->item(current_row, 3)->setText(te->task()->save_path());

        current_task.setData(te->task()->marker(), te->task()->period(), te->task()->path(), te->task()->file_name());

      }
        break;

      case TaskEditor::Error:

        QMessageBox::critical(this, "Ошибка", te->lastError());
        break;

      default:
        break;
    }

    delete te;


  }
  catch(SvException& e) {

    QMessageBox::critical(this, "Error", e.error);

    if(te)
      delete te;

  }
}

void MainWindow::on_bnRemoveTask_clicked()
{
  if(ui->tableWidget->currentRow() < 0)
    return;

  int current_row = ui->tableWidget->currentRow();

  bool ok;
  qint64 current_task_id = ui->tableWidget->item(ui->tableWidget->currentItem()->row(), 0)->data(Qt::UserRole).toLongLong(&ok);
  if(!ok)
    throw SvException("Error on trying to determine task id from item data.");

  if(!m_tasks.contains(current_task_id))
    throw SvException("current task_id not found. on_bnRemoveTask_clicked");

  m_tasks.remove(current_task_id);


  for (auto w: m_widget_items[current_task_id])
    delete w;

  m_widget_items[current_task_id].clear();
  m_widget_items.remove(current_task_id);

  ui->tableWidget->removeRow(current_row);

}

bool MainWindow::makeTree()
{

  QSqlError serr;
  QSqlQuery* q = new QSqlQuery(PGDB->db);
  int column_count = _model->rootItem()->columnCount();

  try {

    _model->clear();

    /** группа Конфигурация пульта **/
    _standRoot = _model->rootItem()->insertChildren(_model->rootItem()->childCount(), 1, column_count);
    _standRoot->parent_index = _model->rootItem()->index;
    _standRoot->is_main_row = true;
    _standRoot->item_type = itStandRoot;
    _standRoot->setData(1, "Конфигурация пульта");
//    _standRoot->setData(0, QString(" "));
    for(int i = 0; i < column_count; i++) _standRoot->setInfo(i, ItemInfo());
    _standRoot->setInfo(0, ItemInfo(itStandRootIcon, ""));


    // добавляем разделы Общие сведения, Параметры запуска, Конфигурация КСУТС сервера, Логгер КСУТС сервера
//    _general_info = _standRoot->insertChildren(_standRoot->childCount(), 1, column_count);
//    _general_info->parent_index = _standRoot->index;
//    _general_info->is_main_row = true;
//    _general_info->item_type = itStandInfo;
//    _general_info->setData(1, "Общие сведения");

    _autostart = _standRoot->insertChildren(_standRoot->childCount(), 1, column_count);
    _autostart->parent_index = _standRoot->index;
    _autostart->is_main_row = true;
    _autostart->item_type = itAutostart;
    _autostart->setData(1, "Параметры автозапуска");

    _ksuts_config = _standRoot->insertChildren(_standRoot->childCount(), 1, column_count);
    _ksuts_config->parent_index = _standRoot->index;
    _ksuts_config->is_main_row = true;
    _ksuts_config->item_type = itConfig;
    _ksuts_config->setData(1, "Конфигурация КСУТС сервера");

    _ksuts_logger = _standRoot->insertChildren(_standRoot->childCount(), 1, column_count);
    _ksuts_logger->parent_index = _standRoot->index;
    _ksuts_logger->is_main_row = true;
    _ksuts_logger->item_type = itLogger;
    _ksuts_logger->setData(1, "Логи КСУТС сервера");

    /** разделитель 1 **/
    TreeItem* div1 = _model->rootItem()->insertChildren(_model->rootItem()->childCount(), 1, column_count);
    div1->parent_index = _model->rootItem()->index;
    div1->item_type = itUndefined;
    div1->setData(1, QString(100, ' '));


    /**      группа "Устройства"      */
    _devicesRoot = _model->rootItem()->insertChildren(_model->rootItem()->childCount(), 1, column_count);
    _devicesRoot->parent_index = _model->rootItem()->index;
    _devicesRoot->is_main_row = true;
    _devicesRoot->item_type = itDevicesRoot;
    for(int i = 0; i < column_count; i++) _devicesRoot->setInfo(i, ItemInfo());
    _devicesRoot->setInfo(0, ItemInfo(itDevicesRootIcon, ""));
//    _devicesRoot->setData(0, QString(" "));
    // определяем общее кол-во и кол-во привязанных устройств для этой стойки
    serr = PGDB->execSQL(QString(SQL_SELECT_DEVICES_COUNT_STR), q);
    if(serr.type() != QSqlError::NoError) _exception.raise(serr.text());
    q->first();

    _devicesRoot->setData(1, QString("Устройства %1").arg(q->value(0).toString()));

    q->finish();

    // заполняем список устройств
    pourDevicesToRoot(_devicesRoot);

    // заполняем список сигналов
    pourSignalsToDevices(_devicesRoot);

    /** разделитель 2 **/
    TreeItem* div2 = _model->rootItem()->insertChildren(_model->rootItem()->childCount(), 1, column_count);
    div2->parent_index = _model->rootItem()->index;
    div2->item_type = itUndefined;
    div2->setData(1, QString(100, ' '));


    /**    группа "Хранилища"       **/
    _storagesRoot = _model->rootItem()->insertChildren(_model->rootItem()->childCount(), 1, column_count);
    _storagesRoot->parent_index = _model->rootItem()->index;
    _storagesRoot->is_main_row = true;
    _storagesRoot->item_type = itStoragesRoot;
    _storagesRoot->setData(1, QString("Хранилища"));
//    _storagesRoot->setData(0, QString(" "));
    for(int i = 0; i < column_count; i++) _storagesRoot->setInfo(i, ItemInfo());
    _storagesRoot->setInfo(0, ItemInfo(itStoragesRootIcon, ""));


    // читаем все! хранилища
    pourStoragesToRoot(_storagesRoot);

    // заполняем список устройств
    pourDevicesToStorages(_storagesRoot);

    // сигналы
    pourSignalsToStorages(_storagesRoot);


    delete q;

    ui->treeView->expandToDepth(0);

    return true;

  }

  catch(SvException& e) {

    q->finish();
    delete q;

    mainlog << sv::log::Time << sv::log::mtCritical << e.error << sv::log::endl;

    return false;

  }

}

bool MainWindow::addTask(zn1::Task* newtask, QString& error)
{
  for(auto& task: m_tasks) {

    if(task.save_path() == newtask->save_path()) {

      error = QString("Невозможно добавить задачу. Указанный путь для сохранения уже задан");
      return false;
    }
  }



//  ui->treeViewFilters->

  ui->tableWidget->insertRow(ui->tableWidget->rowCount());
  int row = ui->tableWidget->rowCount() - 1;

  m_widget_items.insert(newtask->id(), QList<QTableWidgetItem*>());

//  m_widget_items[row].append(new QTableWidgetItem(newtask->id()));
//  ui->tableWidget->setItem(row, 0, m_widget_items[row].last());

  m_widget_items[newtask->id()].append(new QTableWidgetItem(newtask->marker()));
  ui->tableWidget->setItem(row, 0, m_widget_items[newtask->id()].last());
  m_widget_items[newtask->id()].last()->setData(Qt::UserRole, newtask->id());

  m_widget_items[newtask->id()].append(new QTableWidgetItem(newtask->begin().toString("dd.MM.yyyy hh:mm:ss")));
  ui->tableWidget->setItem(row, 1, m_widget_items[newtask->id()].last());

  m_widget_items[newtask->id()].append(new QTableWidgetItem(newtask->end().toString("dd.MM.yyyy hh:mm:ss")));
  ui->tableWidget->setItem(row, 2, m_widget_items[newtask->id()].last());

  m_widget_items[newtask->id()].append(new QTableWidgetItem(newtask->save_path()));
  ui->tableWidget->setItem(row, 3, m_widget_items[newtask->id()].last());

  m_tasks.insert(newtask->id(), zn1::Task(newtask));

  return true;
}
