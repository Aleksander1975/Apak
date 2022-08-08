#include "sv_graph.h"
#include "ui_sv_graphparamsdialog.h"

int svgraph::GraphParams::current_color_index = 119;  // index of "red" color in QColor::colorNames()
int svgraph::GraphParams::current_style_index = 0;    // solid line

svgraph::SvGraphParamsDialog::SvGraphParamsDialog(svgraph::GraphParams *params, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvGraphParamsDialog)
{
  ui->setupUi(this);
  
  foreach (auto style, svgraph::penStyles.keys())
    ui->cbLineStyle->addItem(svgraph::penStyles.value(style), static_cast<int>(style));

  QStringList colorNames = QColor::colorNames();

  for (int i = 0; i < colorNames.size(); ++i) {
      QColor color(colorNames[i]);

      ui->cbLineColor->insertItem(i, colorNames[i]);
      ui->cbLineColor->setItemData(i, color, Qt::DecorationRole);
  }
  
  
  foreach (svgraph::GraphType key, GraphTypes.keys()) {
    ui->cbGraphType->addItem(GraphTypes.value(key), int(key));
  }
  
  if(params) {
    graph_params = *params;
    ui->cbGraphType->setEnabled(false);
  }
  
  
  ui->cbGraphType->setCurrentIndex(ui->cbGraphType->findData(QVariant(graph_params.type)));
  ui->cbLineStyle->setCurrentIndex(ui->cbLineStyle->findData(QVariant(static_cast<int>(graph_params.line_style))));
  ui->cbLineColor->setCurrentIndex(ui->cbLineColor->findData(QVariant(graph_params.line_color), Qt::DecorationRole));
  ui->labelLineColor->setStyleSheet(QString("color: %1").arg(ui->cbLineColor->currentText()));
  ui->spinLineWidth->setValue(graph_params.line_width);
//  ui->editLegendText->setText(graph_params.legend);
  
  setModal(true);
  show();
  
}

svgraph::SvGraphParamsDialog::~SvGraphParamsDialog()
{
  delete ui;
  deleteLater();
}


void svgraph::SvGraphParamsDialog::accept()
{
  graph_params.legend     = ui->editLegendText->text();
  graph_params.line_color = QColor(ui->cbLineColor->currentText());
  graph_params.line_style = Qt::PenStyle(ui->cbLineStyle->currentData().toInt());
  graph_params.line_width = ui->spinLineWidth->value();
  graph_params.type       = static_cast<svgraph::GraphType> (ui->cbGraphType->currentData().toInt());
  
  QDialog::accept();
}

void svgraph::SvGraphParamsDialog::on_cbLineColor_currentIndexChanged(const QString &arg1)
{
    ui->labelLineColor->setStyleSheet(QString("color: %1").arg(arg1));
}



void svgraph::SvGraphParamsDialog::on_cbGraphType_currentIndexChanged(const QString &arg1)
{
    ui->editLegendText->setText(arg1);
}
