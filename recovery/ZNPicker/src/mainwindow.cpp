#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

//  ui->comboReadType->clear();
//  ui->comboReadType->addItem("Все", QVariant("all"));
//  ui->comboReadType->addItem("Выбранные задачи", QVariant("tasks"));
//  ui->comboReadType->setCurrentIndex(0);

  m_config_file_name = "config.json"; //QString("%1%2%3").arg(qApp->applicationDirPath()).arg(QDir::separator()).arg(qApp->applicationName() + ".json");

  QDir::setCurrent(qApp->applicationDirPath());

  QString error;
  if(!loadConfig(error)) {

    QMessageBox::critical(this, "", error);
    return;
  }

  ui->lineOpenFilePath->setText(m_config.zn_data_file);

  for(zn1::Task task: m_config.pickerParams.tasks) {

    if(!addTask(&task, error))
      QMessageBox::critical(this, "Ошибка", error);

  }

  sv::log::Options logopt;
  logopt.enable = true;
  logopt.level = m_config.log_level;

  m_logger = new sv::SvWidgetLogger{logopt, ui->textLog};

  AppParams::loadLayout(this);

}

bool MainWindow::loadConfig(QString& error)
{
  QFile json_file(m_config_file_name);
  try {

    if(!json_file.exists()) {

      if(!json_file.open(QIODevice::WriteOnly))
        throw SvException(json_file.errorString());

      json_file.write(zn1::PickerParams().toByteArray());
      json_file.close();

    }

    if(!json_file.open(QIODevice::ReadOnly))
      throw SvException(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QByteArray json = json_file.readAll();

    QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    jdoc.object().value(P_FULL_FILE_NAME).toString();
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

MainWindow::~MainWindow()
{
  QString error;
  if(!saveConfig(error))
    QMessageBox::critical(this, "Ошибка", error);

  AppParams::saveLayout(this);

  delete m_logger;

  delete ui;
}

bool MainWindow::saveConfig(QString& error)
{
  // формируем и сохраняем json файл конфигурации
  m_config.zn_data_file = ui->lineOpenFilePath->text();

  m_config.pickerParams.tasks.clear();

  for(auto task: m_tasks) {

    m_config.pickerParams.tasks.append(task);
  }

  QFile json_file(m_config_file_name);

  try {

    if(!json_file.open(QIODevice::WriteOnly))
      throw SvException(json_file.errorString());

    json_file.write(m_config.toByteArray());

    if(json_file.error() != QFile::NoError)
      throw SvException(json_file.errorString());

  } catch (SvException e) {

    if(json_file.isOpen())
      json_file.close();

    error = e.error;

    return false;

  }

  json_file.close();
  return true;
}

bool MainWindow::addTask(zn1::Task* newtask, QString& error)
{
  for(auto& task: m_tasks) {

    if(task.save_path() == newtask->save_path()) {

      error = QString("Невозможно добавить задачу. Указанный путь для сохранения уже задан");
      return false;
    }
  }

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

void MainWindow::on_bnSelectFile_clicked()
{
  QString full_file_name = QFileDialog::getOpenFileName(this, "Выберите файл с данными", QString(), "ZN Data (*.zndata);;Все файлы (*.*)");

  if(!full_file_name.isEmpty())
    ui->lineOpenFilePath->setText(full_file_name);
}

void MainWindow::on_bnStart_clicked()
{
  bool ok;
  m_config.pickerParams.start_byte = ui->lineCurrentPos->text().toLongLong(&ok);

  if(!ok) {

    qDebug() << "wrong value";
    return;
  }


  // формируем и сохраняем json файл конфигурации
  QString error;
  if(!saveConfig(error)) {

    QMessageBox::critical(this, "Ошибка", error);
    return;
  }

  //! создаем объект-экстрактор
  m_picker = new zn1::ZNPicker();

  if(!m_picker->configure(m_config_file_name)) {

    QMessageBox::critical(this, "Ошибка", m_picker->lastError());
    return;
  }

  connect(m_picker, &zn1::ZNPicker::started,  this,                &MainWindow::setButtonsStateStarted);
  connect(m_picker, &zn1::ZNPicker::finished, this,                &MainWindow::setButtonsStateStopped);
  connect(m_picker, &zn1::ZNPicker::message,  this,                &MainWindow::message);

  connect(this,       &MainWindow::stop,      m_picker,            &zn1::ZNPicker::stop);

  connect(m_picker, &zn1::ZNPicker::finished, m_picker,            &zn1::ZNPicker::deleteLater);

  connect(m_picker, &zn1::ZNPicker::read_progress,ui->progressBar, &QProgressBar::setValue);
  connect(m_picker, &zn1::ZNPicker::find_progress,this,            &MainWindow::find_progress);

  connect(m_picker, &zn1::ZNPicker::current_position, this, [=](qint64 position)-> void { ui->lineCurrentPos->setText(QString::number(position)); });

  m_picker->start();

}

void MainWindow::setButtonsStateStarted()
{
  ui->bnStart->setEnabled(false);
  ui->bnStop->setEnabled(true);
  ui->gbTasks->setEnabled(false);
  ui->gbCommonParams->setEnabled(false);
  qApp->processEvents();
}

void MainWindow::setButtonsStateStopped()
{
  ui->bnStart->setEnabled(true);
  ui->bnStop->setEnabled(false);
  ui->gbTasks->setEnabled(true);
  ui->gbCommonParams->setEnabled(true);
  qApp->processEvents();
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

void MainWindow::on_tableWidget_itemSelectionChanged()
{
  ui->bnEditTask->setEnabled(!ui->tableWidget->selectedItems().isEmpty());
  ui->bnRemoveTask->setEnabled(!ui->tableWidget->selectedItems().isEmpty());
}

void MainWindow::on_bnStop_clicked()
{
    emit stop();
}

void MainWindow::find_progress(int count)
{
  ui->lblFoundCount->setText(QString::number(count));

  // здесь есть проблемка обновления значения количества найденных данных
  // так как чтение данных в потоке m_picker происходит быстро, и происходит вызов этого слота,
  // то здесь образуется длинная очередь сообщений с новым значением
  // использование qApp->processEvents() приводит к вылету приложения при значении 5879 (почему именно это число не знаю)
  // ui->lblFoundCount->repaint() очень сильно тормозит программу, вплоть до полного зависания и вылета

  // самый лучший вариант здесь это ui->lblFoundCount->update(), ибо:
  // This function does not cause an immediate repaint;
  // instead it schedules a paint event for processing when Qt returns to the main event loop.
   //This permits Qt to optimize for more speed and less flicker than a call to repaint() does.
  ui->lblFoundCount->update();
}

void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
  Q_UNUSED(item);

  on_bnEditTask_clicked();
}
