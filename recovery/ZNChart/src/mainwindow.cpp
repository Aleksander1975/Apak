#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_current_dir = QDir(qApp->applicationDirPath());
  m_config_file_name = "config.json"; //QString("%1%2%3").arg(qApp->applicationDirPath()).arg(QDir::separator()).arg(qApp->applicationName() + ".json");

  QDir::setCurrent(qApp->applicationDirPath());

  QString error;
  if(!loadConfig(error)) {

    QMessageBox::critical(this, "Ошибка", error);
    return;
  }

  m_chart = new svchart::SvChartWidget(m_config.chartParams, ui->tabData);
  ui->verticalLayout_7->addWidget(m_chart);

  AppParams::loadLayout(this);
}

bool MainWindow::loadConfig(QString& error)
{
  QFile json_file(QDir(m_current_dir).filePath(m_config_file_name));

  try {

    if(!json_file.open(QIODevice::ReadWrite))
      throw SvException(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QByteArray json = json_file.readAll();
    QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

//    m_config = zn1::ReaderParams::fromJsonObject(jdoc.object());

    m_config = zn1::RecoveryConfig::fromJsonObject(jdoc.object());

    json_file.close();
    return true;

  } catch (SvException& e) {

    if(json_file.isOpen())
      json_file.close();

    error = e.error;

    return false;

  }
}

MainWindow::~MainWindow()
{
  AppParams::saveLayout(this);

  delete ui;
}

void MainWindow::on_bnAddData_clicked()
{
  ZNSelectDataDialog* fd = new ZNSelectDataDialog(m_config, this);

  if(fd->exec() == ZNSelectDataDialog::Accepted) {

//    std::vector<std::pair<qint64, QString>> sorted_data;
    QList<QString> distinct_signals;
    QList<qint64>  distinct_dt;
    QMap<QPair<qint64, QString>, float> values_map;

    foreach (const modus::SignalConfig& signal_config, fd->period_values.keys()) {

      if(!distinct_signals.contains(signal_config.name))
        distinct_signals.append(signal_config.name);

      // вычисляем хэш, чтобы присвоить новому графику уникальный id.
      // если график с таким id уже есть, то сначала удаляем его, потом добавляем заново
      uint uid = qHash(QString(GRAPH_LEGEND_PATTERN)
                           .arg(signal_config.name)
                           .arg(fd->period_values.value(signal_config).first().dateTime())
                           .arg(fd->period_values.value(signal_config).last().dateTime()));

      if(m_chart->findGraph(uid))
        continue;

      svgraph::GraphParams gp{};
      gp.legend = QString(GRAPH_LEGEND_PATTERN)
                  .arg(signal_config.name)
                  .arg(QDateTime::fromMSecsSinceEpoch(fd->period_values.value(signal_config).first().dateTime()).toString(DEFAULT_DATETIME_FORMAT))
                  .arg(QDateTime::fromMSecsSinceEpoch(fd->period_values.value(signal_config).last().dateTime()).toString(DEFAULT_DATETIME_FORMAT));


      m_chart->addGraph(uid, gp);


//qint64 i = 0;

      for(auto instant_value: fd->period_values.value(signal_config)) {

        m_chart->appendData(uid, QDateTime::fromMSecsSinceEpoch(instant_value.dateTime()).toTime_t() /*i++*/, instant_value.value().toFloat());

        if(!distinct_dt.contains(instant_value.dateTime()))
          distinct_dt.append(instant_value.dateTime());

        // для сортировки по дате времени
//        sorted_data.push_back(std::pair<qint64, QString>{instant_value.dateTime(), signal_config.name});

        values_map.insert(QPair<qint64, QString>{instant_value.dateTime(), signal_config.name}, instant_value.value().toFloat());

      }

      m_chart->setActualXRange();
      m_chart->setActualYRange();

      QListWidgetItem *wi = new QListWidgetItem(gp.legend, ui->graphList);
      wi->setData(Qt::DecorationRole, gp.line_color);
      wi->setData(Qt::UserRole, uid);

      svgraph::GraphParams::setNextStyle();
    }

    // формируем таблицу значений
    QString html = QString("<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "<body>\n"
                           "\t<table border=\"1\" cellpadding=\"5\" cellspacing=\"0\" style=\"border-collapse: collapse; border:#747474;\" >\n"
                           "\t\t<thead>\n"
                           "\t\t\t<tr>\n"
                           "\t\t\t\t<th scope=\"row\" style=\"width: %1px\">Дата/Время</th>\n").arg(23*12);

    for(auto sn: distinct_signals)
      html.append(QString("\t\t\t\t<th scope=\"col\" style=\"width: %1px;\">%2</th>\n").arg(sn.length()*12).arg(sn));

    html.append("\t\t\t</tr>\n"
                "\t\t</thead>\n"
                "\t\t<tbody>\n");


    for(auto dt: distinct_dt) {

      html.append("\t\t\t<tr>\n");
      html.append(QString("\t\t\t\t<td style=\"text-align:center; white-space:nowrap;\">%1</td>\n").arg(QDateTime::fromMSecsSinceEpoch(dt).toString(DEFAULT_DATETIME_FORMAT)));

      for(auto signal_name: distinct_signals) {

        auto key = QPair<qint64, QString>{dt, signal_name};
        html.append(QString("\t\t\t\t<td style=\"text-align:center; white-space:nowrap;\">%1</td>\n").arg((values_map.contains(key) ? QString::number(values_map.value(key), 'g', 2) : "&nbsp;")));

      }

      html.append("\t\t\t</tr>\n");
    }

    html.append("\t\t</tbody>\n"
                "\t</table>\n"
                "</body>\n"
                "</html>");

    ui->textValuesTable->setHtml(html);

//    QFile g("/home/user/h.html");
//    g.open(QIODevice::WriteOnly);
//    g.write(html.toUtf8());
//    g.close();

  }

  delete fd;
}


void MainWindow::on_bnEditGraph_clicked()
{
  uint uid = ui->graphList->currentIndex().data(Qt::UserRole).toUInt();

  svgraph::GraphParams p = m_chart->graphParams(uid);

  svgraph::SvGraphParamsDialog* chDlg = new svgraph::SvGraphParamsDialog(&p, this);

  if(chDlg->exec() == QDialog::Accepted) {

    p = chDlg->graph_params;
    m_chart->setGraphParams(uid, p);

    ui->graphList->currentItem()->setData(Qt::DecorationRole, p.line_color);

  }

  delete chDlg;

}

void MainWindow::on_bnRemoveGraph_clicked()
{
  uint uid = ui->graphList->currentIndex().data(Qt::UserRole).toUInt();

  m_chart->removeGraph(uid);
  ui->graphList->takeItem(ui->graphList->currentRow());

  ui->bnEditGraph->setEnabled(ui->graphList->count() > 0);
  ui->bnRemoveGraph->setEnabled(ui->graphList->count() > 0);
}

void MainWindow::on_graphList_doubleClicked(const QModelIndex &index)
{
  Q_UNUSED(index);

  on_bnEditGraph_clicked();
}

void MainWindow::on_graphList_currentRowChanged(int currentRow)
{
  Q_UNUSED(currentRow);

  ui->bnEditGraph->setEnabled(ui->graphList->count() > 0);
  ui->bnRemoveGraph->setEnabled(ui->graphList->count() > 0);
}

