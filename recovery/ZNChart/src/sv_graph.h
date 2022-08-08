
/**********************************************************************
 *  автор Свиридов С.А. НИИ РПИ
 * *********************************************************************/

#ifndef SV_GRAPH_H
#define SV_GRAPH_H

#include <QDialog>
#include <QColor>
#include <QStringList>
#include <QMap>
#include <QDebug>


namespace Ui {
class SvGraphParamsDialog;
}

namespace svgraph {

  enum GraphType{
    gtDefault = 0,
    gtDiscrete,
    gtFloat,
    gtText,
    gtMixed
  };

  /* режимы отображения */
  const QMap<GraphType, QString> GraphTypes = { {gtDefault,   "По умолчанию"},
                                                {gtDiscrete,  "Дискретный"  },
                                                {gtFloat,     "Аналоговый"  },
                                                {gtText,      "Текстовый"   },
                                                {gtMixed,     "Смешанный"   }};

//  const QStringList penStyleNames           = {"Solid",
//                                               "Dash",
//                                               "Dot",
//                                               "Dash Dot",
//                                               "Dash Dot Dot"};

  const QMap<Qt::PenStyle, QString> penStyles = {{Qt::SolidLine,      "Solid"       },
                                                 {Qt::DashLine,       "Dash"        },
                                                 {Qt::DotLine,        "Dot"         },
                                                 {Qt::DashDotLine,    "Dash Dot"    },
                                                 {Qt::DashDotDotLine, "Dash Dot Dot"}};

  struct GraphParams {

    static int current_color_index;
    static int current_style_index;

    GraphParams()
    {
      line_color  = QColor(QColor::colorNames().at(current_color_index));
      line_style  = penStyles.keys().at(current_style_index);
    }

    GraphParams(const svgraph::GraphParams &other)
    {
      legend      = other.legend;
      line_color  = other.line_color;
      line_style  = other.line_style;
      type        = other.type;
      line_width  = other.line_width;
    }

    GraphType     type        = gtDefault;
    int           line_width  = 1;
    QColor        line_color  = Qt::red;
    Qt::PenStyle  line_style  = Qt::SolidLine;
    QString       legend      = "";
    
    svgraph::GraphParams &operator= (svgraph::GraphParams &p) {
      legend      = p.legend;
      line_color  = p.line_color;
      line_style  = p.line_style;
      type        = p.type;
      line_width  = p.line_width;
      
      return *this;
    }

    static void setNextStyle()
    {
      // line color
      current_color_index++;

      if(current_color_index >= QColor::colorNames().count())
        current_color_index = 0;

//      this->line_color = QColor(QColor::colorNames().at(current_color_index));

      // line style
//      current_style_index++;

//      if(current_style_index >= penStyles.keys().count())
//        current_style_index = 0;

//      this->line_style = penStyles.keys().at(current_style_index);

//      return *this;
    }
  };
  
  class SvGraphParamsDialog;
}


class svgraph::SvGraphParamsDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvGraphParamsDialog(svgraph::GraphParams *params = nullptr, QWidget *parent = nullptr);
  ~SvGraphParamsDialog();
  
  svgraph::GraphParams graph_params;
  
public slots:
  void accept() Q_DECL_OVERRIDE;
  
 
private slots:
  void on_cbLineColor_currentIndexChanged(const QString &arg1);
  
//  void on_buttonBox_clicked(QAbstractButton *button);
  
  void on_cbGraphType_currentIndexChanged(const QString &arg1);
  
private:
  Ui::SvGraphParamsDialog *ui;
};

#endif // SV_GRAPH_H
