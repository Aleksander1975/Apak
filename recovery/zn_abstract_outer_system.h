#ifndef ZN_ABSTRACT_OUTER_SYSTEM
#define ZN_ABSTRACT_OUTER_SYSTEM

#include <QtCore>

#include "../../job/Modus/global/signal/sv_signal.h"


namespace zn1 {

  class InstantValue
  {
  public:
    InstantValue():
      m_date_time(0),
      m_value(QVariant())
    {  }

    InstantValue(qint64 date_time, QVariant value):
      m_date_time(date_time),
      m_value(value)
    {  }

          qint64   dateTime() const { return m_date_time; }
    const QVariant value()    const { return m_value;     }


  private:
    qint64   m_date_time;
    QVariant m_value;
  };

  class AbstractOuterSystem: public QObject
  {
    Q_OBJECT

  public:
    AbstractOuterSystem() {  }

    virtual ~AbstractOuterSystem() {}

    QString last_error = QString();

    virtual bool getPeriodValues(const QString& data_file, const QList<modus::SignalConfig>& signal_config_list, QMap<modus::SignalConfig, QList<InstantValue>>& result) = 0;

  protected:
    bool p_running = false;

  public slots:
    void stop()
    {
      p_running = false;
    }

  signals:
    void progress(int size);

  };
}

#endif // ZN_ABSTRACT_OUTER_SYSTEM

