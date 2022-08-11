#ifndef ZN_SELECT_FILE_DIALOG_H
#define ZN_SELECT_FILE_DIALOG_H

#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include "../../../../svlib/SvProgressBarDialog/1.0/progressbar_dialog.h"
#include "../../../../svlib/SvSettings/1.0/sv_settings.h"
#include "../../recovery_defs.h"
#include "../../zn_abstract_outer_system.h"

#include "treeitem.h"
#include "treemodel.h"
#include "sv_graph.h"

namespace Ui {
  class ZNSelectDataDialog;
}

/*struct SignalGraphConfig {

  SignalGraphConfig() {}

  SignalGraphConfig(modus::SignalConfig& config, svgraph::GraphParams& params):
    config(config),
    params(params)
  {

  }

  modus::SignalConfig   config;
  svgraph::GraphParams  params;
};
*/

/*class SignalGraphConfigList
{
public:
  SignalGraphConfigList():
    m_config_list(QList<modus::SignalConfig> ()),
    m_params_list(QList<svgraph::GraphParams>())
  {}

  const QList<modus::SignalConfig>&   configList() const { return m_config_list; }
  const QList<svgraph::GraphParams>&  paramsList() const { return m_params_list; }

  void append(const modus::SignalConfig& config, const svgraph::GraphParams& params)
  {
    m_config_list.append(config);
    m_params_list.append(params);
  }

  void remove(const modus::SignalConfig& config)
  {
    int i = m_config_list.indexOf(config);
    if(i >= 0)
      removeAt(i);
  }

//  void remove(const svgraph::GraphParams& params)
//  {
//    int i = m_params_list.indexOf(params);
//    if(i >= 0)
//      removeAt(i);
//  }

  void removeAt(int index)
  {
    m_config_list.removeAt(index);
    m_params_list.removeAt(index);
  }

  void clear()
  {
    m_config_list.clear();
    m_params_list.clear();
  }

private:
  QList<modus::SignalConfig>  m_config_list;
  QList<svgraph::GraphParams> m_params_list;

};
*/

class ZNSelectDataDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZNSelectDataDialog(const zn1::RecoveryConfig& config, QWidget *parent = 0);
  ~ZNSelectDataDialog();

  QMap<modus::SignalConfig, QList<zn1::InstantValue>> period_values;

private slots:
  void on_bnCancel_clicked();

  void on_treeViewTasks_clicked(const QModelIndex &index);

  void on_bnLoadSignalsData_clicked();

  void on_treeViewSignals_doubleClicked(const QModelIndex &index);

private:
  Ui::ZNSelectDataDialog*           ui;

  zn1::RecoveryConfig               m_config;

  zn1::Task                         m_current_task;

  TreeModel*                        m_task_model;
  TreeModel*                        m_signal_model;

  QMap<TreeItem*, zn1::Task>        m_treeitem_2_task;
  QMap<TreeItem*, modus::SvSignal*> m_treeitem_2_signal;

  zn1::ZNOuterSystem                m_current_system;
  zn1::AbstractOuterSystem*         m_current_system_lib_object = nullptr;

//  QMap<modus::SignalConfig, svgraph::GraphParams>        m_checked_signals;
//  SignalGraphConfigList          m_checked_signals;
  QList<modus::SignalConfig>        m_checked_signals;


  svgraph::SvGraphParamsDialog*     m_graph_params_dialog = nullptr;


};

#endif // ZN_SELECT_FILE_DIALOG_H
