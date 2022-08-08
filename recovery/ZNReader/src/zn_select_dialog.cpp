#include "zn_select_dialog.h"

ZNSelector::ZNSelector(QWidget *parent, const QString& json) :
  QDialog(parent),
  ui(new Ui::SelectZNDialog)
{
  ui->setupUi(this);

  if(!loadJson(json)) {

    QDialog::done(Error);
  }

  connect(ui->bnSelect,     &QPushButton::clicked,        this, &QDialog::accept);
  connect(ui->bnCancel,     &QPushButton::clicked,        this, &QDialog::reject);
  connect(ui->tableWidget,  &QTableWidget::doubleClicked, this, &QDialog::accept);

  this->setModal(true);
  this->show();
}

bool ZNSelector::loadJson(const QString& jsonPath)
{
  QFile json_file(jsonPath);

  try {

    if(!json_file.open(QIODevice::ReadOnly))
      throw SvException(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QByteArray json = json_file.readAll();
    QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    m_configuration = jdoc.object();


    /// ищем опсания защищенных хранилищ
    if(!m_configuration.contains(P_STORAGES))
      throw SvException(QString("В заданной конфигурации не найдена секция \"%1\"").arg(P_STORAGES));

    if(!m_configuration.value(P_STORAGES).isArray())
      throw SvException(QString("Неверная конфигурация. Секция \"%1\" не является массивом").arg(P_STORAGES));

    for(QJsonValue v: m_configuration.value(P_STORAGES).toArray()) {

      if(!v.isObject())
        continue;

      QJsonObject o = v.toObject();

      if(!o.contains(P_NAME) || !o.contains(P_PARAMS) || !o.value(P_PARAMS).isObject())
        continue;

      QJsonObject p = o.value(P_PARAMS).toObject();

      if(!p.contains(P_ZONE) || !p.contains(P_HOST))
        continue;

      QString name = o.value(P_NAME).toString();
      QString zone = p.value(P_ZONE).toString();
      QString host = p.value(P_HOST).toString();
      quint16 port = p.value(P_PORT).toInt();

      ui->tableWidget->insertRow(ui->tableWidget->rowCount());

      m_widget_items.append(new QTableWidgetItem(name));
      ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, m_widget_items.last());

      m_widget_items.append(new QTableWidgetItem(host));
      ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, m_widget_items.last());

      m_widget_items.append(new QTableWidgetItem(QString::number(port)));
      ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 2, m_widget_items.last());

      m_widget_items.append(new QTableWidgetItem(zone));
      ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 3, m_widget_items.last());

      m_records.insert(ui->tableWidget->rowCount() - 1, ZnRecord(zone, host, port));

    }

    return true;

  }
  catch (SvException e) {

    if(json_file.isOpen())
      json_file.close();

    m_last_error = e.error;

    return false;
  }
}

ZNSelector::~ZNSelector()
{
  foreach (auto r, m_widget_items)
    delete r;

  close();
  delete ui;
}

void ZNSelector::accept()
{
  try {

    /* делаем всякие проверки вводимых данных */
    if(ui->tableWidget->currentRow() < 0) {

      QMessageBox::critical(this, "Ошибка", "Ни одна строка не выбрана");
      return;
    }

    m_selected_record = m_records.value(ui->tableWidget->currentRow());

    QDialog::done(Accepted);

  }

  catch(SvException& e) {

    m_last_error = e.error;
    QDialog::done(Error);
  }

}

