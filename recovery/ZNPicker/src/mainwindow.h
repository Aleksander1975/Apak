#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPair>
#include <QHash>
#include <QList>
#include <QFileDialog>
#include <QThread>
#include <QTableWidgetItem>

#include "task_editor.h"
#include "zn_picker.h"
#include "../../recovery_defs.h"

#include "../../../../../../c++/svlib/SvWidgetLogger/1.1/sv_widget_logger.h"
#include "../../../../../../c++/svlib/SvSettings/1.0/sv_settings.h"

namespace Ui {
  class MainWindow;
}

//class TaskTableItem
//{
//public:
//  explicit TaskTableItem(int row, zn1::Task* task)
//  {

//  }

//  void append(int row, const zn1::Task& task)
//  {
//    m_task = task;
//  }

//  void setTask(const zn1::Task& task) { m_task = task; }



//private:
//  zn1::Task m_task;


//};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_bnAddTask_clicked();

  void on_bnEditTask_clicked();

  void on_bnRemoveTask_clicked();

  void on_bnSelectFile_clicked();

  void on_bnStart_clicked();

private:
  Ui::MainWindow *ui;

//  QMap<uint, QList<zn1::TaskPeriod>>     m_task_intervals;
  QMap<qint64, zn1::Task>             m_tasks;
//  QMap<int, TaskTableItem>            m_tasks;

  QMap<int, QList<QTableWidgetItem*>> m_widget_items;

  zn1::ZNPicker*                      m_picker = nullptr;

  zn1::RecoveryConfig                 m_config;
  QString                             m_config_file_name;

  sv::SvWidgetLogger*   m_logger = nullptr;

  QTimer m_label_repaint_timer;

  bool loadConfig(QString& error);
  bool saveConfig(QString& error);

  bool addTask(zn1::Task* newtask, QString& error);


signals:
  void stop();

private slots:
  void setButtonsStateStarted();
  void setButtonsStateStopped();

  void on_tableWidget_itemSelectionChanged();
  void on_bnStop_clicked();

  void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

public slots:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
  void find_progress(int count);

};

#endif // MAINWINDOW_H
