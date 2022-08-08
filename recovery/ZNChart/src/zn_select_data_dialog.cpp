#include "zn_select_data_dialog.h"
#include "ui_zn_select_data_dialog.h"

ZNSelectDataDialog::ZNSelectDataDialog(const zn1::RecoveryConfig& config, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZNSelectDataDialog)
{
  ui->setupUi(this);

  m_config = config;

  m_checked_signals = QList<modus::SignalConfig>{}; // SignalGraphConfigList();

  // готовим виджет для отображения списка задач
  m_task_model = new TreeModel(QStringList() << "<Маркер" << "|Начало" << "|Конец" << "<Файл",
                                ui->treeViewTasks);

  ui->treeViewTasks->setModel(m_task_model);
  ui->treeViewTasks->setUpdatesEnabled(true);

  ui->treeViewTasks->setColumnWidth(0, 130);
  ui->treeViewTasks->setColumnWidth(1, 130);
  ui->treeViewTasks->setColumnWidth(2, 130);

  for(auto task: m_config.pickerParams.tasks) {

    TreeItem* newitem = m_task_model->rootItem()->insertChildren(m_task_model->rootItem()->childCount(), 1, m_task_model->rootItem()->columnCount());
    newitem->parent_index = m_task_model->rootItem()->index;
    newitem->is_main_row = true;
    newitem->item_type = itConfig;

    newitem->setData(0, task.marker());
    newitem->setData(1, task.begin());
    newitem->setData(2, task.end());
    newitem->setData(3, task.save_path());

    m_treeitem_2_task.insert(newitem, task);

    int i = 0;
    newitem->setInfo(i++, ItemInfo(itConfig, ""));

    while(i < m_task_model->rootItem()->columnCount())
      newitem->setInfo(i++, ItemInfo());

  }

  // готовим виджет для списка сигналов
  m_signal_model = new TreeModel(QStringList() << "|Выбор" << "<Имя сигнала" << "<Параметры" << "<Описание",
                                ui->treeViewSignals);

  ui->treeViewSignals->setUpdatesEnabled(true);

  AppParams::loadLayoutWidget(this);
}

ZNSelectDataDialog::~ZNSelectDataDialog()
{
  AppParams::saveLayoutWidget(this);

  delete ui;
}

void ZNSelectDataDialog::on_bnCancel_clicked()
{
  QDialog::reject();
}

void ZNSelectDataDialog::on_treeViewTasks_clicked(const QModelIndex &index)
{
  try {

    ui->groupBox_2->setEnabled(false);
    qApp->processEvents();

    // очищаем списки
    m_current_system.signals_list.clear();
    m_checked_signals.clear();

    m_signal_model->removeRows(0, m_signal_model->rowCount());
    m_treeitem_2_signal.clear();

    TreeItem* item = m_task_model->itemFromIndex(index);

    if(!m_treeitem_2_task.contains(item))
      throw SvException("Exception in on_treeViewTasks_clicked. treeitems_by_task has no item");

    m_current_task = m_treeitem_2_task.value(item);

    // по маркеру задачи (которая выбрана в данный момент), ищем систему в конфиге
    bool found = false;
    for(auto& s: m_config.outer_systems) {

      if(s.marker == m_current_task.marker()) {

        found = true;
        m_current_system = s;
        break;
      }
    }

    if(!found)
      throw SvException(QString("Marker \"%1\" not found in section \"systems\"").arg(m_current_task.marker()));

    // загружаем список сигналов из файла json
    QList<modus::SignalConfig> signals_list;
    modus::get_signals_from_json(QFileInfo(m_current_system.signals_file).absoluteFilePath(), &signals_list);

    // накидываем элементы на виджет
    for(auto& signal_config: signals_list) {

      signal_config.timeout = 0;      // чтобы не срабатывал сброс сигнала по таймеру
      signal_config.enable  = false;  // сигнал выбран или нет. checkbox

      TreeItem* newitem = m_signal_model->rootItem()->insertChildren(m_signal_model->rootItem()->childCount(), 1, m_signal_model->rootItem()->columnCount());
      newitem->parent_index = m_signal_model->rootItem()->index;
      newitem->is_main_row = true;
      newitem->item_type = itConfig;

      newitem->setCheckable(0, true);
      newitem->setData(0, signal_config.enable);
//      newitem->setChecked(0, signal_config.enable);
      newitem->setData(1, signal_config.name);
      newitem->setData(2, signal_config.params);
      newitem->setData(3, signal_config.description);

      modus::SvSignal* signal = new modus::SvSignal(signal_config);

      m_treeitem_2_signal.insert(newitem, signal);

//      int i = 0;
//      newitem->setInfo(i++, ItemInfo(itConfig, "", true));

//      while(i < m_signal_model->rootItem()->columnCount())
//        newitem->setInfo(i++, ItemInfo());

    }

    ui->treeViewSignals->setModel(m_signal_model);
    ui->treeViewSignals->setColumnWidth(0, 50);
    ui->treeViewSignals->setColumnWidth(1, 260);
    ui->treeViewSignals->setColumnWidth(2, 260);

    // пытаемся загрузить библиотеку-обработчик для выбранной задачи
    if(m_current_system.lib.isEmpty() || !QFileInfo(QDir(QDir::currentPath()).absoluteFilePath(m_current_system.lib)).exists())
      throw SvException(QString("Lib path \"%1\" is empty or does not exists").arg(m_current_system.lib));

    QLibrary lib(QFileInfo(m_current_system.lib).absoluteFilePath());

    if(!lib.load())
      throw SvException(lib.errorString());

    typedef zn1::AbstractOuterSystem *(*create_func)(void);
    create_func create = (create_func)lib.resolve("create");

    if (create)
      m_current_system_lib_object = create();

    else
      throw SvException(lib.errorString());

    if(!m_current_system_lib_object)
      throw SvException("Unknown error after call create()");

    ui->groupBox_2->setEnabled(true);
    qApp->processEvents();

  }
  catch(SvException& e) {

    if(m_current_system_lib_object)
      delete m_current_system_lib_object;

    m_current_system_lib_object = nullptr;

    QMessageBox::critical(this, "Error", e.error);

  }
}

void ZNSelectDataDialog::on_bnLoadSignalsData_clicked()
{
  if(!m_current_system_lib_object)
    return;

  if(!QFileInfo(m_current_task.save_path()).exists()) {

    QMessageBox::critical(this, "Error", QString("Файл с данными '%1' не найден").arg(m_current_task.save_path()));
    return;
  }

  sv::ProgressBarDialog* pbar = new sv::ProgressBarDialog("Извлечение данных");
  connect(pbar, &sv::ProgressBarDialog::cancel, m_current_system_lib_object, &zn1::AbstractOuterSystem::stop);
  connect(m_current_system_lib_object, &zn1::AbstractOuterSystem::progress, pbar, &sv::ProgressBarDialog::progress);

  pbar->setModal(true);
  pbar->show();

  period_values = QMap<modus::SignalConfig, QList<zn1::InstantValue>>();

  bool b = m_current_system_lib_object->getPeriodValues(m_current_task.save_path(),
//             QDir(m_current_task.path()).absoluteFilePath(m_current_task.file_name()),
             m_checked_signals, period_values);
  if(!b) {

    QMessageBox::critical(this, "Ошибка", m_current_system_lib_object->last_error);
    return;

  }

  delete pbar;

//  for(auto signal: period_values.keys()) {

//    for(zn1::InstantValue iv: period_values.value(signal))
//      qDebug() << signal << QDateTime::fromMSecsSinceEpoch(iv.dateTime()) << iv.value();
//  }

  accept();

}

void ZNSelectDataDialog::on_treeViewSignals_doubleClicked(const QModelIndex &index)
{
  TreeItem* item = m_signal_model->itemFromIndex(index);
  modus::SvSignal* signal = m_treeitem_2_signal.value(item);

  modus::SignalConfig cfg(signal->config());

  m_checked_signals.removeOne(cfg);

  cfg.enable = !cfg.enable;

  if(cfg.enable) {

//    m_graph_params_dialog = new svgraph::SvGraphParamsDialog();
//    if(m_graph_params_dialog->exec() == svgraph::SvGraphParamsDialog::Accepted) {

      m_checked_signals.append(cfg); //, m_graph_params_dialog->graph_params);

//    }
  }

  item->setData(0, cfg.enable);
  signal->configure(cfg);

  m_signal_model->dataChanged(m_signal_model->index(index.row(), 0), m_signal_model->index(index.row(), m_signal_model->columnCount()));

}
