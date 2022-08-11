#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPair>
#include <QHash>
#include <QList>
#include <QFileDialog>
#include <QThread>
#include <QInputDialog>

#include "zn_select_dialog.h"
#include "zn_reader.h"
#include "../../../../../job/svlib/SvWidgetLogger/1.1/sv_widget_logger.h"

namespace Ui {
  class MainWindow;
}

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

private slots:
  void on_bnSelectZN_clicked();

  void on_checkManualZNEdit_toggled(bool checked);

  void on_bnStart_clicked();

  void on_bnSelectFile_clicked();

  void on_bnStop_clicked();

private:
  Ui::MainWindow *ui;

  zn1::ZNReader*        m_reader = nullptr;
//  QThread*              m_thread = nullptr;
  sv::SvWidgetLogger*   m_logger = nullptr;

  zn1::RecoveryConfig m_config;
  QString               m_config_file_name;

  bool loadConfig(QString& error);
  bool saveConfig(QString& error);

private slots:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
  void setProgressZoneSize(int size);
  void setCurrentProgress(int size);

  void on_checkRequestZoneSize_toggled(bool checked);

  void setButtonsStateStarted();
  void setButtonsStateStopped();

signals:
  void stop();


};

#endif // MAINWINDOW_H
