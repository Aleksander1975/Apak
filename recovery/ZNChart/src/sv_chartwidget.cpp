/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "sv_chartwidget.h"


svchart::SvChartWidget::SvChartWidget(zn1::ChartParams params, QWidget *parent)
{
  this->setParent(parent);
  
  m_params = params;
  
  setupUi();
  
  // configure bottom axis to show date instead of number:
  QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
  dateTicker->setDateTimeFormat("yyyy.MM.dd\nhh:mm:ss.zzz");
  m_customplot->xAxis->setTicker(dateTicker);

  spinXRange->setValue(m_params.x_range);
  bnYAutoscale->setChecked(m_params.y_autoscale);

//  m_customplot->xAxis->setTickLabelType(QCPAxis::ltDateTime);

  on_bnResetChart_clicked();
  
}

void svchart::SvChartWidget::setupUi()
{
//    this->setObjectName(QStringLiteral("SvChartWidget1"));
  
    vlayMain = new QVBoxLayout(this);
    vlayMain->setObjectName(QStringLiteral("vlayMain"));
    
    hlay1 = new QHBoxLayout();
    hlay1->setSpacing(6);
    hlay1->setObjectName(QStringLiteral("hlay1"));
    hlay1->setContentsMargins(4, -1, -1, -1);
    
    bnResetChart = new QPushButton(this);
    bnResetChart->setObjectName(QStringLiteral("bnResetChart"));
    bnResetChart->setMaximumSize(QSize(25, 16777215));
    
    QIcon icon;
    icon.addFile(QStringLiteral(":/res/icons/Refresh.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnResetChart->setIcon(icon);

    hlay1->addWidget(bnResetChart);

    frameXRange = new QFrame(this);
    frameXRange->setObjectName(QStringLiteral("frameXRange"));
    frameXRange->setFrameShape(QFrame::StyledPanel);
    frameXRange->setFrameShadow(QFrame::Raised);
    hlayXRange = new QHBoxLayout(frameXRange);
    hlayXRange->setObjectName(QStringLiteral("hlayXRange"));
    hlayXRange->setContentsMargins(4, 4, 4, 4);
    hspacer1 = new QSpacerItem(122, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer1);

    bnXRangeDown = new QPushButton(frameXRange);
    bnXRangeDown->setObjectName(QStringLiteral("bnXRangeDown"));
    bnXRangeDown->setMaximumSize(QSize(25, 16777215));
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/res/icons/Zoom out.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXRangeDown->setIcon(icon1);

    hlayXRange->addWidget(bnXRangeDown);

    hspacer2 = new QSpacerItem(123, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer2);

    bnXRangeActual = new QPushButton(frameXRange);
    bnXRangeActual->setObjectName(QStringLiteral("bnXRangeActual"));
    bnXRangeActual->setMaximumSize(QSize(25, 16777215));
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/res/icons/Search.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXRangeActual->setIcon(icon2);

    hlayXRange->addWidget(bnXRangeActual);

    hspacer3 = new QSpacerItem(122, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer3);

    spinXRange = new QSpinBox(frameXRange);
    spinXRange->setObjectName(QStringLiteral("spinXRange"));
    spinXRange->setMinimum(10);
    spinXRange->setMaximum(10000);

    hlayXRange->addWidget(spinXRange);

    bnXSetRange = new QPushButton(frameXRange);
    bnXSetRange->setObjectName(QStringLiteral("bnXSetRange"));
    bnXSetRange->setMaximumSize(QSize(25, 16777215));
    QIcon icon3;
    icon3.addFile(QStringLiteral(":/res/icons/Ok2.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXSetRange->setIcon(icon3);

    hlayXRange->addWidget(bnXSetRange);

    hspacer4 = new QSpacerItem(123, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer4);

    bnXRangeUp = new QPushButton(frameXRange);
    bnXRangeUp->setObjectName(QStringLiteral("bnXRangeUp"));
    bnXRangeUp->setMaximumSize(QSize(25, 16777215));
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/res/icons/Zoom in.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnXRangeUp->setIcon(icon4);

    hlayXRange->addWidget(bnXRangeUp);

    hspacer5 = new QSpacerItem(122, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hlayXRange->addItem(hspacer5);


    hlay1->addWidget(frameXRange);


    vlayMain->addLayout(hlay1);

    hlay2 = new QHBoxLayout();
    hlay2->setObjectName(QStringLiteral("hlay2"));
    frameYRange = new QFrame(this);
    frameYRange->setObjectName(QStringLiteral("frameYRange"));
    frameYRange->setFrameShape(QFrame::StyledPanel);
    frameYRange->setFrameShadow(QFrame::Raised);
    vlayYRange = new QVBoxLayout(frameYRange);
    vlayYRange->setSpacing(2);
    vlayYRange->setObjectName(QStringLiteral("vlayYRange"));
    vlayYRange->setContentsMargins(4, 4, 4, 4);
    bnYAutoscale = new QPushButton(frameYRange);
    bnYAutoscale->setObjectName(QStringLiteral("bnYAutoscale"));
    bnYAutoscale->setMaximumSize(QSize(25, 16777215));
    QIcon icon5;
    icon5.addFile(QStringLiteral(":/res/icons/Stats.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnYAutoscale->setIcon(icon5);
    bnYAutoscale->setCheckable(true);
  
    vlayYRange->addWidget(bnYAutoscale);

    vspacer1 = new QSpacerItem(20, 84, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer1);

    bnYRangeDown = new QPushButton(frameYRange);
    bnYRangeDown->setObjectName(QStringLiteral("bnYRangeDown"));
    bnYRangeDown->setMaximumSize(QSize(25, 16777215));
    bnYRangeDown->setIcon(icon4);

    vlayYRange->addWidget(bnYRangeDown);

    vspacer2 = new QSpacerItem(20, 85, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer2);

    bnYRangeActual = new QPushButton(frameYRange);
    bnYRangeActual->setObjectName(QStringLiteral("bnYRangeActual"));
    bnYRangeActual->setMaximumSize(QSize(25, 16777215));
    bnYRangeActual->setIcon(icon2);

    vlayYRange->addWidget(bnYRangeActual);

    vspacer3 = new QSpacerItem(20, 84, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer3);

    bnYRangeUp = new QPushButton(frameYRange);
    bnYRangeUp->setObjectName(QStringLiteral("bnYRangeUp"));
    bnYRangeUp->setMaximumSize(QSize(25, 16777215));
    QIcon icon6;
    icon6.addFile(QStringLiteral(":/res/icons/Zoom out.ico"), QSize(), QIcon::Normal, QIcon::Off);
    bnYRangeUp->setIcon(icon6);

    vlayYRange->addWidget(bnYRangeUp);

    vspacer4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlayYRange->addItem(vspacer4);

    hlay2->addWidget(frameYRange);


    m_customplot = new QCustomPlot(this);
    m_customplot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_customplot->xAxis->setRange(0, m_params.x_range, Qt::AlignLeft);
    
    m_customplot->yAxis->setRange(0, 1, Qt::AlignCenter);
//    _customplot->axisRect()->setupFullAxesBox(true);
//    _customplot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    m_customplot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_customplot->legend->setVisible(true);
        
    hlay2->addWidget(m_customplot);
    
    vlayMain->addLayout(hlay2);


//    retranslateUi(this);

    QMetaObject::connectSlotsByName(this);
} // setupUi

void svchart::SvChartWidget::addGraph(uint graph_id, svgraph::GraphParams &graphParams)
{
  /* если такой график уже есть, то ничего не добавляем и выходим */
  if(findGraph(graph_id))
    return;
    
  svchart::GRAPH* g = new svchart::GRAPH;
  g->graph = m_customplot->addGraph();
//  g->params = graphParams;
  
  m_graphs.insert(graph_id, g);

  setGraphParams(graph_id, graphParams);
  
}

void svchart::SvChartWidget::setGraphParams(uint graph_id, const svgraph::GraphParams &graphParams)
{
  QPen pen(graphParams.line_color);
  pen.setStyle(Qt::PenStyle(graphParams.line_style));
  pen.setWidth(graphParams.line_width);
    
  m_graphs.value(graph_id)->graph->setPen(pen);

  m_graphs.value(graph_id)->graph->setLineStyle(QCPGraph::lsStepCenter);
  m_graphs.value(graph_id)->graph->setScatterStyle(QCPScatterStyle::ssDisc); //ssTriangleInverted);
  
  m_graphs.value(graph_id)->graph->setName(graphParams.legend);

  m_customplot->repaint();
  
}

void svchart::SvChartWidget::removeGraph(uint graph_id)
{
  /* очищаем и удаляем graph */
  m_graphs.value(graph_id)->graph->data().clear();//clearData();
  m_customplot->removeGraph(m_graphs.value(graph_id)->graph);
  
  /* удаляем GRAPH */
  delete m_graphs.value(graph_id);
  
  /* удаляем запись о графике из map'а */
  m_graphs.remove(graph_id);
  
  m_customplot->replot();
  
}

void svchart::SvChartWidget::appendData(uint graph_id, qint64 dateTime, float value)
{
//  double x = m_graphs.value(graph_id)->graph->data()->dataRange().length(); // count();
  m_graphs.value(graph_id)->graph->addData(dateTime, value); // data()->insert(x, QCPData(dateTime, value));

  setMaxMinY(value);
  setMaxMinX(dateTime);

//  if(m_params.y_autoscale)
//    setActualYRange();
}

//void svchart::SvChartWidget::insertData(uint graph_id, QCPData xy)
//{
//  m_graphs.value(graph_id)->graph->addData(data()->insert(xy.key, xy);
  
//  setMaxMinY(xy.value);
  
//  if(m_params.y_autoscale)
//    setActualYRange();
//}

void svchart::SvChartWidget::on_bnXRangeUp_clicked()
{
  m_params.x_range *= 1.25;
  m_customplot->xAxis->setRangeUpper(m_params.x_range);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnXRangeDown_clicked()
{
  m_params.x_range /= 1.25;
  m_customplot->xAxis->setRangeUpper(m_params.x_range);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnXRangeActual_clicked()
{
  m_customplot->xAxis->setRange(m_x_min, m_x_max - m_x_min /*m_customplot->graph()->dataCount()*/, Qt::AlignLeft);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnXSetRange_clicked()
{
  m_params.x_range = spinXRange->value();
  m_customplot->xAxis->setRangeUpper(m_params.x_range);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnYRangeUp_clicked()
{
  m_customplot->yAxis->setRangeUpper(m_customplot->yAxis->range().upper * 1.25);
  m_customplot->yAxis->setRangeLower(m_customplot->yAxis->range().lower * 1.25);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnYRangeDown_clicked()
{
  m_customplot->yAxis->setRangeUpper(m_customplot->yAxis->range().upper * 0.75);
  m_customplot->yAxis->setRangeLower(m_customplot->yAxis->range().lower * 0.75);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnYRangeActual_clicked()
{
  setActualYRange();
}

void svchart::SvChartWidget::setActualXRange()
{
  m_customplot->xAxis->setRange(m_x_min, m_x_max);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::setActualYRange()
{
  m_customplot->yAxis->setRange(m_y_min - (m_y_min / 20), m_y_max + m_y_max / 20);
  m_customplot->replot(QCustomPlot::rpQueuedReplot);
}

void svchart::SvChartWidget::on_bnResetChart_clicked()
{
  for(int i = 0; i < m_customplot->graphCount(); i++)
    m_customplot->graph(i)->data().clear(); //clearData();
  
  m_customplot->xAxis->setRange(m_x_min, m_x_min + spinXRange->value(), Qt::AlignLeft);
  
  m_y_max = -1000000000;
  m_y_min =  1000000000;
  
  m_customplot->replot();
  
}

void svchart::SvChartWidget::on_bnYAutoscale_clicked(bool checked)
{
  if(checked)
    on_bnYRangeActual_clicked();
  
  m_params.y_autoscale = checked;
  
}
