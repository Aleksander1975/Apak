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

#ifndef SV_CHARTWIDGET_H
#define SV_CHARTWIDGET_H

#include <QtCore/QTimer>
#include <QWidget>
#include <QMutex>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QSpinBox>
#include <QtCore/QTime>
#include <QtCore/QDebug>

#include "../../../../../job/qcustomplot/qcustomplot.h"
#include "../../../../../job/svlib/SvException/1.1/sv_exception.h"

#include "../../../../../job/Modus/global/global_defs.h"
#include "../../recovery_defs.h"

#include "sv_graph.h"


namespace svchart {


  /* надписи для сдвига по оси Х */
  const QMap<zn1::ChartXAutoscrollType, QString> ChartXAutoscrollCaptions = {
               {zn1::xtNone,       QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: <\320\235\320\265\321\202>", Q_NULLPTR)},
               {zn1::xtTick,       QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: \320\235\320\260 \320\276\320\264\320\275\321\203 \321\202\320\276\321\207\320\272\321\203", Q_NULLPTR)},
               {zn1::xtHalfChart,  QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: \320\235\320\260 1/2 \320\263\321\200\320\260\321\204\320\270\320\272\320\260", Q_NULLPTR)},
               {zn1::xtChart,      QApplication::translate("Form", "\320\220\320\262\321\202\320\276\321\201\320\264\320\262\320\270\320\263: \320\235\320\260 \321\206\320\265\320\273\321\213\320\271 \320\263\321\200\320\260\321\204\320\270\320\272", Q_NULLPTR)}
  };

  struct GRAPH {
    QCPGraph*             graph;
    svgraph::GraphParams  params;

    GRAPH& operator=(GRAPH& other) {

      graph   = other.graph;
      params  = other.params;

      return *this;

    }
  };
  
  class SvChartWidget;
  
}


class svchart::SvChartWidget: public QWidget
{
    Q_OBJECT

public:
    SvChartWidget(zn1::ChartParams params, QWidget *parent = 0);
    ~SvChartWidget() { close(); deleteLater(); }
    
    QCustomPlot *customplot()       { return m_customplot; }
    zn1::ChartParams& params() { return m_params; }
    
    void setActualXRange();
    void setActualYRange();
    
    void setMaxMinY(qreal y) {

      if(y > m_y_max) m_y_max = y; // * 1.01;
      if(y < m_y_min) m_y_min = y; // * 1.01;
    }

    void setMaxMinX(qint64 dateTime) {

      if(dateTime > m_x_max) m_x_max = dateTime;
      if(dateTime < m_x_min) m_x_min = dateTime;
    }

    void addGraph(uint graph_id, svgraph::GraphParams &graphParams);
    
    void removeGraph(uint graph_id);
    
    bool findGraph(uint graph_id) { return m_graphs.find(graph_id) != m_graphs.end(); }
    
    void setGraphParams(uint graph_id, const svgraph::GraphParams &graphParams);
    
    QList<uint> graphList() { return m_graphs.keys(); }
    
    int graphCount() { return m_customplot->graphCount(); }
    
    void appendData(uint graph_id, qint64 dateTime, float value);
    
//    void insertData(uint graph_id, QCPData xy);
    
    svgraph::GraphParams graphParams(uint graph_id) { return m_graphs.value(graph_id)->params; }
    
//    int pointCount() { return m_customplot->graph()->data()->count(); }
    
    QMutex mutex;
    
private:
    
    QCustomPlot*                   m_customplot;
    QMap<uint, svchart::GRAPH*>    m_graphs;
    zn1::ChartParams               m_params;
    
    qreal m_y_max = -1000000000;
    qreal m_y_min =  1000000000;

    qint64 m_x_max = 0;
    qint64 m_x_min = 0x7FFFFFFFFFFFFFFF;

    /** виджеты **/
    void setupUi();
    
    QVBoxLayout *vlayMain;
    QHBoxLayout *hlay1;
    QPushButton *bnResetChart;
    QFrame      *frameXRange;
    QHBoxLayout *hlayXRange;
    QSpacerItem *hspacer1;
    QPushButton *bnXRangeDown;
    QSpacerItem *hspacer2;
    QPushButton *bnXRangeActual;
    QSpacerItem *hspacer3;
    QSpinBox    *spinXRange;
    QPushButton *bnXSetRange;
    QSpacerItem *hspacer4;
    QPushButton *bnXRangeUp;
    QSpacerItem *hspacer5;
    QHBoxLayout *hlay2;
    QFrame      *frameYRange;
    QVBoxLayout *vlayYRange;
    QPushButton *bnYAutoscale;
    QSpacerItem *vspacer1;
    QPushButton *bnYRangeDown;
    QSpacerItem *vspacer2;
    QPushButton *bnYRangeActual;
    QSpacerItem *vspacer3;
    QPushButton *bnYRangeUp;
    QSpacerItem *vspacer4;
    
private slots:
    void on_bnXRangeUp_clicked();
    void on_bnXRangeDown_clicked();
    void on_bnXRangeActual_clicked();
    void on_bnXSetRange_clicked();
    void on_bnYRangeUp_clicked();
    void on_bnYRangeDown_clicked();
    void on_bnYRangeActual_clicked();
    void on_bnResetChart_clicked();
    void on_bnYAutoscale_clicked(bool checked);

};




//class svchart::Chart: public QChart
//{
//    Q_OBJECT
//public:
//    Chart(ChartParams &params, QGraphicsItem *parent = Q_NULLPTR, Qt::WindowFlags wFlags = 0);
//    virtual ~Chart();
    
//    QLineSeries *m_series;
//    QValueAxis *axX;
//    QValueAxis *axY;

//private:
//    QStringList m_titles;
//    qreal m_step;
//    qreal m_x;
//    qreal m_y;
    
//    svchart::ChartParams _params;
    
//protected:
//    bool sceneEvent(QEvent *event);

//private:
//    bool gestureEvent(QGestureEvent *event);
//};


#endif /* SV_CHARTWIDGET_H */
