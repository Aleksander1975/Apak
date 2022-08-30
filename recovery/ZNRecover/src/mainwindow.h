#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPair>
#include <QHash>
#include <QList>
#include <QFileDialog>
#include <QThread>
#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "zn_select_dialog.h"
#include "zn_reader.h"
#include "filter_editor.h"
#include "treemodel.h"
#include "treeitem.h"
#include "set_period_dialog.h"
#include "coarse_data_picker.h"

#include "../../../../svlib/SvSettings/1.0/sv_settings.h"

#include "../../../../../job/svlib/SvWidgetLogger/1.1/sv_widget_logger.h"

#define MAIN_TREE_HEADERS "<Система/Сигнал;|Начало;|Конец;<Описание"

namespace Ui {
  class MainWindow;
}

enum ActionState {
  Stopped,
  Started
};

class Interval
{
public:
  explicit Interval(QDateTime start, QDateTime end):
    m_start(start),
    m_end(end)
  {

  }

  bool isValid() { return !m_start.isNull() && m_start.isValid() && !m_end.isNull() && m_end.isValid(); }
  bool contains(QDateTime point) { return ((point >= m_start) && (point <= m_end)); }

  bool operator==(const Interval& other) { return (other.m_start == m_start) && (other.m_end == m_end); }

private:
  QDateTime m_start;
  QDateTime m_end;

};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow*           ui;

  zn1::ZNReader*            m_reader = nullptr;
  zn1::CoarseDataPicker*    m_coarse_picker = nullptr;

  sv::SvWidgetLogger*       m_logger = nullptr;

  zn1::RecoveryConfig       m_config;
  QString                   m_config_file_name;

  QSqlDatabase              m_db;

  ActionState               m_read_state = Stopped;
  ActionState               m_pick_state = Stopped;

  QMap<qint64, zn1::FineFilter> m_tasks;

  TreeModel*                m_model = nullptr;

  QDateTime                 m_filter_last_begin = QDateTime();
  QDateTime                 m_filter_last_end   = QDateTime();

  bool loadConfig(QString& error);
  bool saveConfig(QString& error);
  bool createDb(QString& error);
  void updateFilters() ;

  bool makeTree(const QString& filter_marker = QString(), const QString& filter_signal = QString(), bool show_selected_only = false);

private slots:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
//  void setProgressZoneSize(int size);
//  void setCurrentProgress(int size);
  void setStatusBarText(const QString& text);

  void readData();

  void on_bnSelectFile_clicked();

  void setReadStateStarted();
  void setReadStateStopped();

  void setPickStateStarted();
  void setPickStateStopped();

  void changeFilter(const QModelIndex& index);

  void on_radioLoadData_toggled(bool checked);

  void on_radioDoNotLoad_toggled(bool checked);

  void on_radioTraining_toggled(bool checked);

  void on_bnToNextStep_clicked();

  void backToBegin();

  void on_cbSystems_currentIndexChanged(int index);

  void on_lineQuickSearch_returnPressed();

  void on_checkFilteredOnly_toggled(bool checked);

  void on_treeViewFilters_clicked(const QModelIndex &index);

  void on_bnDiscardQuickSearch_clicked();

  void on_treeViewFilters_doubleClicked(const QModelIndex &index);

  void on_treeViewFilters_pressed(const QModelIndex &index);

  void on_bnPickData_clicked();

  void on_bnEdit_clicked();

  void on_bnAcceptChanges_clicked();

  void on_bnCancelChanges_clicked();

  void on_bnToStep3_clicked();

  void on_bnChangeFilter_clicked();

  signals:
    void stopReading();
    void stopPicking();


};

#endif // MAINWINDOW_H
