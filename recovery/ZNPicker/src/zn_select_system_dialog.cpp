#include "zn_select_system_dialog.h"

ZNSystemSelectDialog::ZNSystemSelectDialog(QWidget *parent, const QString& json) :
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

bool ZNSystemSelectDialog::loadJson(const QString& jsonPath)
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


    /// ищем опсания внешних систем
    if(!m_configuration.contains(P_GLOBAL) || !m_configuration.value(P_GLOBAL).isObject())
      throw SvException(QString("В заданной конфигурации не найдена или неверная секция \"%1\"").arg(P_GLOBAL));

    QJsonObject global = m_configuration.value(P_GLOBAL).toObject();

    if(!global.contains(P_SYSTEMS))
      throw SvException(QString("В заданной конфигурации не найдена секция \"%1\"").arg(P_GLOBAL));

    if(!global.value(P_SYSTEMS).isArray())
      throw SvException(QString("Неверная конфигурация. Секция \"%1\" не является массивом").arg(P_SYSTEMS));

    for(QJsonValue v: global.value(P_SYSTEMS).toArray()) {

      if(!v.isObject())
        continue;

      QJsonObject o = v.toObject();

      if(!o.contains(P_MARKER))
        continue;

      QString system_name = o.value(P_MARKER).toString();
      QString description = o.contains(P_DESCRIPTION) ? o.value(P_DESCRIPTION).toString() : "";

      ui->tableWidget->insertRow(ui->tableWidget->rowCount());

      m_widget_items.append(new QTableWidgetItem(system_name));
      ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, m_widget_items.last());

      m_widget_items.append(new QTableWidgetItem(description));
      ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, m_widget_items.last());

      m_records.insert(ui->tableWidget->rowCount() - 1, system_name);

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

ZNSystemSelectDialog::~ZNSystemSelectDialog()
{
  foreach (auto r, m_widget_items)
    delete r;

  close();
  delete ui;
}

void ZNSystemSelectDialog::accept()
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

