#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPair>

//#include "../../../../../../c++/Modus_Libs/APAK/storages/zn_k1/src/zn_global.h"

#include "../../../../../../c++/svlib/SvSettings/1.0/sv_settings.h"
//#include "../../recovery_defs.h"
#include "sv_chartwidget.h"
#include "zn_select_data_dialog.h"

#define GRAPH_LEGEND_PATTERN "%1, %2 - %3"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  bool loadConfig(QString& error);

private slots:
  void on_bnAddData_clicked();

  void on_bnEditGraph_clicked();

  void on_bnRemoveGraph_clicked();

  void on_graphList_doubleClicked(const QModelIndex &index);

  void on_graphList_currentRowChanged(int currentRow);

private:
  Ui::MainWindow*         ui;

  svchart::SvChartWidget* m_chart         = nullptr;
  zn1::RecoveryConfig     m_config;

  QDir                    m_current_dir;
  QString                 m_config_file_name;

//  svgraph::GraphParams    m_global_graph_params = svgraph::GraphParams{};

};

#endif // MAINWINDOW_H
