#ifndef RECOVERY_DEFS
#define RECOVERY_DEFS

#include <QtCore>
#include <QMap>
#include <QDebug>
#include <QtNetwork/qhostaddress.h>

#include "../../../../c++/Modus/global/signal/sv_signal.h"
#include "../../../../c++/Modus_Libs/APAK/storages/zn_k1/src/zn_global.h"
#include "../../../../c++/svlib/SvAbstractLogger/svabstractlogger.h"

// имена параметров (отсутствующие в global_defs.h)
#define P_GLOBAL                  "global"
#define P_READER                  "reader"
#define P_PICKER                  "picker"
#define P_CHART                   "chart"
//#define P_DISPLAY                 "display"
#define P_SYSTEMS                 "systems"
#define P_ZN_DATA_FILE            "zn_data_file"
#define P_TASKS                   "tasks"
#define P_MARKER                  "marker"
#define P_BEGIN                   "begin"
#define P_END                     "end"
#define P_FILE_NAME               "file_name"
//#define P_ZN_MARKER               "zn_marker"
//#define P_HOST                    "host"
#define P_ZONE                    "zone"
//#define P_PASS                    "pass"
//#define P_QUEUE_LEN               "queue_len"
//#define P_WRITE_BUF               "write_buf"
#define P_ASK_ZONE_SIZE           "ask_zone_size"
#define P_START_BYTE              "start_byte"
#define P_ZONE_SIZE_GB            "zone_size_gb"
#define P_ZONE_SIZE               "zone_size_mb"
#define P_FULL_FILE_NAME          "full_file_name"
#define P_BUFF_SIZE               "buff_size_mb"

#define P_X_RANGE                 "x_range"
#define P_X_TICK_COUNT            "x_tick_count"
#define P_Y_RANGE                 "y_range"
#define P_Y_AUTOSCALE             "y_autoscale"
#define P_X_AUTOSCROLL_TYPE       "x_autoscroll_type"
#define P_X_MESURE_UNIT           "x_measure_unit"
#define P_X_TICK_PERIOD           "x_tick_period"

#define DEFAULT_MARKER_TEMPLATE   "[MARKER]"
#define DEFAULT_BEGIN_TEMPLATE    "[BEGIN]"
#define DEFAULT_END_TEMPLATE      "[END]"

#define DEFAULT_FILE_NAME_TEMPLATE DEFAULT_MARKER_TEMPLATE "_" DEFAULT_BEGIN_TEMPLATE "_" DEFAULT_END_TEMPLATE ".znr"

const int     SIGLEN                 = 7;
const char    ZNR_SIGNATURE[SIGLEN]  = {'Z', 'N', 'R', 'D', 'A', 'T', 'A'};
const quint8  ZNR_VERSION            = 1;

namespace zn1 {

//  ChartXAutoscrollType  stringToXAutoscrollType (const QString& type);
//  ChartXMeasureUnit     stringToXMeasureUnit    (const QString& name);

  /*! структуры для описания и хранения задач для извлечения
   * необходимых данных из общего массива данных */
  class TaskPeriod
  {
  public:
    explicit TaskPeriod():
      m_begin(QDateTime::currentDateTime()),
      m_end(QDateTime::currentDateTime())
    {  }

    explicit TaskPeriod(QDateTime begin, QDateTime end):
      m_begin(begin),
      m_end(end)
    {

    }

    void setBegin(QDateTime begin)
    {
      m_begin = begin;
    }

    void setEnd(QDateTime end)
    {
      m_end = end;
    }

    void setData(QDateTime begin, QDateTime end)
    {
      m_begin = begin;
      m_end   = end;

    }

    QDateTime begin() { return m_begin; }
    QDateTime end()   { return m_end; }

    bool isValid() { return !m_begin.isNull() && m_begin.isValid() && !m_end.isNull() && m_end.isValid(); }
    bool contains(QDateTime point) { return ((point >= m_begin) && (point <= m_end)); }
    bool contains(qint64 point) const { return ((point >= m_begin.toMSecsSinceEpoch()) && (point <= m_end.toMSecsSinceEpoch())); }

    bool operator==(const TaskPeriod& other) { return (other.m_begin == m_begin) && (other.m_end == m_end); }

  private:
    QDateTime m_begin;
    QDateTime m_end;

  };

  class Task {

  public:
    Task():
      m_id(0),
      m_marker(QString()),
      m_begin(QDateTime::currentDateTime()),
      m_end(QDateTime::currentDateTime()),
      m_path(QString()),
      m_file_name(QString())
    {
      m_period.setData(m_begin, m_end);
    }

    Task(Task* task):
      m_id(task->id()),
      m_marker(task->marker()),
      m_begin(task->begin()),
      m_end(task->end()),
      m_path(task->path()),
      m_file_name(task->file_name())
    {
      m_period.setData(m_begin, m_end);
    }

    Task& operator=(const Task& other)
    {
      if(this == &other)
        return *this;

      m_id = other.m_id;
      m_marker = other.m_marker;
      m_begin = other.m_begin;
      m_end = other.m_end;
      m_path = other.m_path;
      m_file_name = other.m_file_name;
      m_period.setData(m_begin, m_end);

      return *this;
    }

    qint64      id()                const { return m_id;            }
    uint        marker_hash()             { return qHash(m_marker); }

    const QString     marker()      const { return m_marker;        }
    const QString     path()        const { return m_path;          }
    const QString     file_name()   const { return m_file_name;     }
    const QDateTime   begin()       const { return m_begin;         }
    const QDateTime   end()         const { return m_end;           }

    const TaskPeriod& period()      const { return m_period;        }

    QString   save_path()
    {
      QString path      = m_path;
      QString file_name = m_file_name;

      return QDir(path.replace(DEFAULT_MARKER_TEMPLATE,  m_marker, Qt::CaseInsensitive)).filePath(file_name
                                                                  .replace(DEFAULT_MARKER_TEMPLATE, m_marker, Qt::CaseInsensitive)
                                                                  .replace(DEFAULT_BEGIN_TEMPLATE,  m_begin.toString("ddMMyyyy-hhmmss"), Qt::CaseInsensitive)
                                                                  .replace(DEFAULT_END_TEMPLATE,    m_end.toString("ddMMyyyy-hhmmss"), Qt::CaseInsensitive)
                                                                .append(file_name.endsWith(".znr") ? "" : ".znr"));
    }

    void setId(quint64 id)
    {
      m_id    = id;
    }

    void setBegin(QDateTime begin)
    {
      m_begin   = begin;
      m_period  = TaskPeriod(m_begin, m_end);
    }

    void setEnd(QDateTime end)
    {
      m_end     = end;
      m_period  = TaskPeriod(m_begin, m_end);
    }

    void setPeriod(QDateTime begin, QDateTime end)
    {
      m_begin   = begin;
      m_end     = end;
      m_period  = TaskPeriod(m_begin, m_end);
    }

    void setPeriod(TaskPeriod period)
    {
      setPeriod(period.begin(), period.end());
    }

    void setMarker(const QString& marker)
    {
      m_marker    = marker;
    }

    void setPath(const QString& path)
    {
      m_path    = path;
    }

    void setFileName(const QString& file_name)
    {
      m_file_name = file_name;
    }

    void setData(const QString& marker, TaskPeriod period, const QString& path, const QString& file_name)
    {
      setData(marker, period.begin(), period.end(), path, file_name);
    }

    void setData(const QString& marker, QDateTime begin, QDateTime end, const QString& path, const QString& file_name)
    {
      setPeriod(begin, end);

      m_marker    = marker;
      m_path      = path;
      m_file_name = file_name;
    }

  private:

    qint64      m_id;
    QString     m_marker;
    QDateTime   m_begin;
    QDateTime   m_end;
    QString     m_path;
    QString     m_file_name;
    TaskPeriod  m_period;

  };


  /*! структуры для хранения конфигурации:
   * ReaderParams - параметры утилиты чтения данных из черного ящика
   * PickerParams - параметры утилиты-сборщика данных
   * GraphParams - параметры программы графического тотбражения архивных данных
   * GlobalParams - общие параметры для всех утилит
   **/
  struct ReaderParams {

    QHostAddress    host            = QHostAddress();
    quint16         port            = DEFAULT_PORT;
    QString         zone            = QString();
    QString         pass            = QString();

    quint64         start_byte      = 0;
    quint32         timeout         = DEFAULT_READ_INTERVAL;

    bool            ask_zone_size   = true;
    double          zone_size_gb    = 0.0;
    quint64         zone_size_mb    = 0;
    quint64         buff_size_mb    = DEFAULT_READ_BUFFER_LEN;  // в мегабайтах // 16 mb

    static ReaderParams fromJsonString(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(err.errorString());

      try {
        return fromJsonObject(jd.object());
      }
      catch(SvException& e) {
        throw e;
      }
    }

    static ReaderParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      ReaderParams p;
      QString P;

      // host
      P = P_HOST;
      if(object.contains(P)) {

        QString host = object.value(P).toString("").toLower();

        if(QHostAddress(host).toIPv4Address() == 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Допускаются только ip адреса в формате 'xxx.xxx.xxx.xxx' (192.168.1.1)"));

        p.host = QHostAddress(host);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      // receive port
      P = P_PORT;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Номер порта должен быть задан целым положительным числом в диапазоне [1..65535]"));

        p.port = object.value(P).toInt(DEFAULT_PORT);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      // zone name
      P = P_ZONE;
      if(object.contains(P)) {

        p.zone = object.value(P).toString();

        if(p.zone.isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Имя зоны для записи не может быть пустым"));
      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // read biffer length
      P = P_BUFF_SIZE;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Размер буфера записи должен быть задан целым положительным числом в мегабайтах"));

        p.buff_size_mb = object.value(P).toInt(DEFAULT_READ_BUFFER_LEN);

      }
      else
        p.buff_size_mb = DEFAULT_READ_BUFFER_LEN;

      // start reading byte
      P = P_START_BYTE;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Номер первого читаемого байта должен быть задан целым положительным числом"));

        p.start_byte = object.value(P).toInt(0);

      }
      else
        p.start_byte = 0;

      // timeout
      P = P_TIMEOUT;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Таймаут должен быть задан целым положительным числом в секундах"));

        p.timeout = object.value(P).toInt(DEFAULT_TIMEOUT);

      }
      else
        p.timeout = DEFAULT_TIMEOUT;

      // request for zone size
      P = P_ASK_ZONE_SIZE;
      if(object.contains(P)) {

        p.ask_zone_size = object.value(P).toBool(true);

      }
      else
        p.ask_zone_size = true;

      // zone_size_gb
      P = P_ZONE_SIZE_GB;
      if(object.contains(P))
      {
        if(object.value(P).toDouble(0) <= 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Неверное значение размера зоны. Размер зоны должен быть задан числом в гигабайтах"));

        p.zone_size_gb  = object.value(P).toDouble();
        p.zone_size_mb     = p.zone_size_gb * 1024 * 1024 * 1024;

      }
      else
        p.zone_size_mb = 0;

      return p;

    }

    QByteArray toByteArray(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return jd.toJson(format);
    }

    QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      return QString(toByteArray(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject p;

      p.insert(P_HOST,            QJsonValue(host.toString()).toString());
      p.insert(P_PORT,            QJsonValue(static_cast<int>(port)).toInt());
      p.insert(P_ZONE,            QJsonValue(zone).toString());
      p.insert(P_PASS,            QJsonValue(pass).toString());
      p.insert(P_ZONE_SIZE_GB,    QJsonValue(static_cast<double>(zone_size_gb)).toDouble());
      p.insert(P_TIMEOUT,         QJsonValue(static_cast<int>(timeout)).toInt());
      p.insert(P_ASK_ZONE_SIZE,   QJsonValue(ask_zone_size).toBool());
      p.insert(P_START_BYTE,      QJsonValue(static_cast<int>(start_byte)).toInt());
      p.insert(P_BUFF_SIZE,       QJsonValue(static_cast<int>(buff_size_mb)).toInt());

      return p;

    }
  };

  struct PickerParams {

    qint64          start_byte      = 0;

    QList<Task>  tasks     = QList<Task>();

    static PickerParams fromJsonString(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(err.errorString());

      try {
        return fromJsonObject(jd.object());
      }
      catch(SvException& e) {
        throw e;
      }
    }

    static PickerParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      PickerParams p;
      QString P;

      // start reading byte
      P = P_START_BYTE;
      if(object.contains(P))
      {
        bool ok;
        p.start_byte = object.value(P).toVariant().toLongLong(&ok);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Номер первого читаемого байта должен быть задан целым положительным числом"));

      }
      else
        p.start_byte = 0;

      if(object.contains(P_TASKS)) {

        if(!object.value(P_TASKS).isArray())
          throw SvException(QString("Неверная конфигурация json. Раздел \"%1\" отсутствует или не является массивом").arg(P_TASKS));

        else
        {
          QJsonArray tasks = object.value(P_TASKS).toArray();

          for(QJsonValue t: tasks) {

            if(!t.isObject())
              continue;

            QJsonObject to = t.toObject();
            Task task = Task{};

            // task id
            P = P_ID;
            if(to.contains(P))
            {
              bool ok;
              qint64 id = to.value(P).toVariant().toLongLong(&ok);
              if(!ok)
                throw SvException(QString(IMPERMISSIBLE_VALUE)
                                   .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                   .arg("Идентификатор задачи должен быть задан целым положительным числом"));

              for(zn1::Task task: p.tasks) {

                if(task.id() == id)
                  throw SvException(QString(IMPERMISSIBLE_VALUE)
                                     .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                     .arg(QString("Не уникальный идентификатор задачи: %1").arg(id)));
              }

              task.setId(id);

            }
            else
              task.setId(QDateTime::currentMSecsSinceEpoch());

            // marker
            P = P_MARKER;
            if(to.contains(P)) {

              QString m = to.value(P).toString("");

              if(m.isEmpty())
                throw SvException(QString(IMPERMISSIBLE_VALUE)
                                   .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                   .arg("Не допустим пустой маркер"));

              task.setMarker(m);

            }
            else
              throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact))).arg(P));

            // begin
            P = P_BEGIN;
            if(to.contains(P)) {

              QDateTime b = QDateTime::fromString(to.value(P).toString(""), DEFAULT_DATETIME_FORMAT);

              if(b.isNull() || !b.isValid())
                throw SvException(QString(IMPERMISSIBLE_VALUE)
                                   .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                   .arg("Не допустимое значение начала периода"));

              task.setBegin(b);

            }
            else
              throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact))).arg(P));

            // end
            P = P_END;
            if(to.contains(P)) {

              QDateTime e = QDateTime::fromString(to.value(P).toString(""), DEFAULT_DATETIME_FORMAT);

              if(e.isNull() || !e.isValid())
                throw SvException(QString(IMPERMISSIBLE_VALUE)
                                   .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                   .arg("Не допустимое значение конца периода"));

              task.setEnd(e);

            }
            else
              throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact))).arg(P));

            // path
            P = P_PATH;
            if(to.contains(P)) {

              QString path = to.value(P).toString("");

              if(path.isEmpty())
                throw SvException(QString(IMPERMISSIBLE_VALUE)
                                   .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                   .arg("Необходимо указать путь для сохранения данных"));

              task.setPath(path);

            }
            else
              throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact))).arg(P));

            // save_file
            P = P_FILE_NAME;
            if(to.contains(P)) {

              QString file_name = to.value(P).toString("");

              if(file_name.isEmpty())
                throw SvException(QString(IMPERMISSIBLE_VALUE)
                                   .arg(P).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact)))
                                   .arg("Необходимо указать имя файла для сохранения загруженных данных"));

              task.setFileName(file_name);

            }
            else
              throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(to).toJson(QJsonDocument::Compact))).arg(P));

            p.tasks.append(task);

          }
        }
      }

      return p;

    }

    QByteArray toByteArray(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return jd.toJson(format);
    }

    QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      return QString(toByteArray(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      QJsonObject p;
//      p.insert(P_FULL_FILE_NAME, QJsonValue(full_file_name).toString());
//      p.insert(P_LOG_LEVEL,      QJsonValue(sv::log::levelToString(log_level)).toString());
      p.insert(P_START_BYTE,     QJsonValue(start_byte));

      QJsonArray a;
      for(const Task& task: tasks) {

        QJsonObject t;

        t.insert(P_ID,        task.id());
        t.insert(P_MARKER,    task.marker());
        t.insert(P_BEGIN,     task.begin().toString(DEFAULT_DATETIME_FORMAT));
        t.insert(P_END,       task.end().toString(DEFAULT_DATETIME_FORMAT));
        t.insert(P_PATH,      task.path());
        t.insert(P_FILE_NAME, task.file_name());

        a.append(t);
      }

      j.insert(P_PARAMS,  QJsonValue(p));
      j.insert(P_TASKS,   QJsonValue(a));

      return j;

    }
  };

  enum ChartXAutoscrollType {
    xtUndefined = -1,
    xtNone = 0,
    xtTick,
    xtHalfChart,
    xtChart
  };

  enum ChartXMeasureUnit {
    xmuUndefined = -1,
    xmuTick,
    xmuMillisecond,
    xmuSecond
  };


  /* сдвиг по оси Х */
  const QMap<ChartXAutoscrollType, QString> ChartXAutoscrollNames = {
               {xtNone,       "none"  },
               {xtTick,       "tick"  },
               {xtHalfChart,  "half"  },
               {xtChart,      "chart" }
  };

  const QMap<ChartXMeasureUnit, QString> ChartXMeasureUnitNames = {
               {xmuTick,        "tick"  },
               {xmuMillisecond, "msec"  },
               {xmuSecond,      "second"}
  };

  inline ChartXAutoscrollType  stringToXAutoscrollType (const QString& type)
  {
    return ChartXAutoscrollNames.key(type, xtUndefined);
  }

  inline ChartXMeasureUnit     stringToXMeasureUnit    (const QString& name)
  {
    return ChartXMeasureUnitNames.key(name, xmuUndefined);
  }

  struct ChartParams {
    qreal                 y_range           = 1.0;
    bool                  y_autoscale       = false;
    qint64                x_range           = 300;
    qint64                x_tick_count      = 26;
    qint64                x_tick_period     = 200;  // в миллисекундах
    ChartXAutoscrollType  x_autoscroll_type = xtTick;
    ChartXMeasureUnit     x_measure_unit    = xmuTick;


    static ChartParams fromJsonString(const QString& json_string) //throw (SvException)
    {
      try {

        QJsonParseError err;
        QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

        if(err.error != QJsonParseError::NoError)
          throw SvException(err.errorString());

        return fromJsonObject(jd.object());
      }
      catch(SvException& e) {
        throw e;
      }
    }

    static ChartParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      ChartParams p;
      QString P;

      // x_range
      P = P_X_RANGE;
      if(object.contains(P)) {

        bool ok;
        p.x_range = object.value(P).toVariant().toUInt(&ok);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Диапазон оси X должен быть задан целым положительным числом"));
      }

      // x_tick_count
      P = P_X_TICK_COUNT;
      if(object.contains(P))
      {
        bool ok;
        p.x_tick_count = object.value(P).toVariant().toUInt(&ok);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Количество отсчетов по оси X должно быть задано целым положительным числом"));
      }

      // y_range
      P = P_Y_RANGE;
      if(object.contains(P)) {

        bool ok;
        p.y_range = object.value(P).toVariant().toReal(&ok);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Диапазон оси Y должен быть задан целым числом с плавающей запятой"));
      }

      // y_autoscale
      P = P_Y_AUTOSCALE;
      if(object.contains(P)) {

        p.y_autoscale = object.value(P).toBool(true);

      }

      // x_autoscroll_type
      P = P_X_AUTOSCROLL_TYPE;
      if(object.contains(P))
      {
        p.x_autoscroll_type = stringToXAutoscrollType(object.value(P).toString());

        if(p.x_autoscroll_type == ChartXAutoscrollType::xtUndefined)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg(QString("Неверное значение для параметра \"%1\". Допустимые значения: none, tick, half, chart")
                                                .arg(P_X_AUTOSCROLL_TYPE)));
      }

      // x_measure_unit
      P = P_X_MESURE_UNIT;
      if(object.contains(P))
      {
        p.x_measure_unit = stringToXMeasureUnit(object.value(P).toString());

        if(p.x_measure_unit == ChartXMeasureUnit::xmuUndefined)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg(QString("Неверное значение для параметра \"%1\". Допустимые значения: tick, msec, second")
                                                .arg(P_X_MESURE_UNIT)));
      }

      // x_tick_period
      P = P_X_TICK_PERIOD;
      if(object.contains(P))
      {
        bool ok;
        p.x_tick_period = object.value(P).toVariant().toUInt(&ok);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                             .arg("Период отсчета по оси X должен быть задан целым положительным числом"));

      }

      return p;
    }

    QByteArray toByteArray(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return jd.toJson(format);
    }

    QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      return QString(toByteArray(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject p;

      p.insert(P_Y_RANGE,           QJsonValue(y_range));
      p.insert(P_Y_AUTOSCALE,       QJsonValue(y_autoscale));
      p.insert(P_X_RANGE,           QJsonValue(x_range));
      p.insert(P_X_TICK_COUNT,      QJsonValue(x_tick_count));
      p.insert(P_X_TICK_PERIOD,     QJsonValue(x_tick_period));
      p.insert(P_X_AUTOSCROLL_TYPE, QJsonValue(zn1::ChartXAutoscrollNames.value(x_autoscroll_type)));
      p.insert(P_X_MESURE_UNIT,     QJsonValue(zn1::ChartXMeasureUnitNames.value(x_measure_unit)));

      return p;

    }

  };

  struct ZNOuterSystem {
    QString marker        = "";
    QString lib           = "";
    QString signals_file  = "";
    QString description   = "";

    QList<modus::SignalConfig> signals_list = QList<modus::SignalConfig>();

  };

  struct RecoveryConfig {

    ReaderParams readerParams;
    PickerParams pickerParams;
    ChartParams  chartParams;

    sv::log::Level  log_level     = sv::log::llInfo;
    QString         zn_data_file  = QString();

    QList<ZNOuterSystem> outer_systems       = QList<ZNOuterSystem>();

    static RecoveryConfig fromJsonString(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(err.errorString());

      try {
        return fromJsonObject(jd.object());
      }
      catch(SvException& e) {
        throw e;
      }
    }

    static RecoveryConfig fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      RecoveryConfig p;
      QString P;

      if(!object.contains(P_GLOBAL) || !object.value(P_GLOBAL).isObject())
        throw SvException(QString("Неверная конфигурация json. Раздел \"%1\" отсутствует или не является объектом").arg(P_PARAMS));

      QJsonObject global = object.value(P_GLOBAL).toObject();

      // zn_data_file
      P = P_ZN_DATA_FILE;
      if(global.contains(P)) {

        p.zn_data_file = global.value(P).toString();

      }
      else
        p.zn_data_file = "";

      // log_level
      P = P_LOG_LEVEL;
      if(global.contains(P))
      {
        p.log_level = sv::log::stringToLevel(global.value(P).toString(""));

        if(p.log_level == sv::log::llUndefined)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(QString(QJsonDocument(global).toJson(QJsonDocument::Compact)))
                             .arg(QString("Неверное значение для параметра \"%1\". Допустимые значения: error, warning, info, debug, debug2")
                                                .arg(P_LOG_LEVEL)));
      }
      else
        p.log_level = sv::log::llInfo;

      // systems
      if(global.contains(P_SYSTEMS) && global.value(P_SYSTEMS).isArray())
      {
        QJsonArray systems = global.value(P_SYSTEMS).toArray();

        for(QJsonValue s: systems) {

          if(!s.isObject())
            continue;

          QJsonObject so = s.toObject();
          ZNOuterSystem system;

          // marker
          P = P_MARKER;
          if(so.contains(P)) {

            if(so.value(P).toString("").isEmpty())
              throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P).arg(QString(QJsonDocument(so).toJson(QJsonDocument::Compact)))
                                 .arg("Не допустим пустой маркер"));

            system.marker = so.value(P).toString("");

          }
          else
            throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(so).toJson(QJsonDocument::Compact))).arg(P));


          // lib
          P = P_LIB;
          if(so.contains(P)) {

            if(so.value(P).toString("").isEmpty())
              throw SvException(QString(IMPERMISSIBLE_VALUE)
                                .arg(P)
                                .arg(QString(QJsonDocument(so).toJson(QJsonDocument::Compact)))
                                .arg("Имя библиотеки драйвера протокола не может быть пустым"));

            system.lib = so.value(P).toString("");

          }
          else
            throw SvException(QString(MISSING_PARAM).arg(P));

          // description
          P = P_DESCRIPTION;
          system.description = so.contains(P) ? so.value(P).toString("") : "";


          // читаем сигналы
          P = P_SIGNALS;
          if(so.contains(P)) {

            if(so.value(P).toString("").isEmpty())
              throw SvException(QString(IMPERMISSIBLE_VALUE)
                                .arg(P)
                                .arg(QString(QJsonDocument(so).toJson(QJsonDocument::Compact)))
                                .arg(QString("Путь к списку сигналов для внешней системы '%1' пуст").arg(system.marker)));

            system.signals_file = so.value(P).toString();

            // грузим данные из файла (и других файлов, на которые могут быть ссылки)
            modus::get_signals_from_json(system.signals_file, &system.signals_list);


          }
          else
            throw SvException(QString(MISSING_PARAM).arg(P));

          p.outer_systems.append(system);

        }
      }
      else
        throw SvException(QString("Неверная конфигурация json. Раздел \"%1\" отсутствует или не является массивом").arg(P_SYSTEMS));


      // reader params
      if(object.contains(P_READER) && object.value(P_READER).isObject())
      {
        p.readerParams = zn1::ReaderParams::fromJsonObject(object.value(P_READER).toObject());
      }

      // picker params
      if(object.contains(P_PICKER) && object.value(P_PICKER).isObject())
      {
        p.pickerParams = zn1::PickerParams::fromJsonObject(object.value(P_PICKER).toObject());
      }

      // chart params
      if(object.contains(P_CHART) && object.value(P_CHART).isObject())
      {
        p.chartParams = zn1::ChartParams::fromJsonObject(object.value(P_CHART).toObject());
      }

      return p;

    }

    QByteArray toByteArray(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return jd.toJson(format);
    }

    QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      return QString(toByteArray(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject p;

      QJsonArray as;
      for(auto system: outer_systems) {

        QJsonObject os;

        os.insert(P_MARKER,         QJsonValue(system.marker));
        os.insert(P_LIB,            QJsonValue(system.lib));
        os.insert(P_DESCRIPTION,    QJsonValue(system.description));
        os.insert(P_SIGNALS,        QJsonValue(system.signals_file));

        as.append(QJsonValue(os));

      }

      QJsonObject global;

      global.insert(P_ZN_DATA_FILE, QJsonValue(zn_data_file));
      global.insert(P_LOG_LEVEL,    QJsonValue(sv::log::levelToString(log_level)));
      global.insert(P_SYSTEMS,      QJsonValue(as));

      p.insert(P_GLOBAL,            QJsonValue(global));
      p.insert(P_READER,            QJsonValue(readerParams.toJsonObject()));
      p.insert(P_PICKER,            QJsonValue(pickerParams.toJsonObject()));
      p.insert(P_CHART,             QJsonValue(chartParams.toJsonObject()));

      return p;
    }

  };

  struct ZNRHeader {

    QString     marker    = QString();
    QString     begin     = QString();
    QString     end       = QString();
    int         len       = 0;

    ZNRHeader()
    { }

    ZNRHeader(const QString& marker, const QString& begin, const QString& end):
      marker(marker),
      begin(begin),
      end(end)
    { }

    QByteArray toByteArray()
    {
      QByteArray b;
      QDataStream s(&b, QIODevice::WriteOnly);
      len = 0;

      s.writeRawData(&ZNR_SIGNATURE[0], SIGLEN);
      len += SIGLEN;

      s << quint8(ZNR_VERSION);
      len += sizeof(quint8);

      s << quint8(marker.length());
      len += sizeof(quint8);

      s.writeRawData(marker.toStdString().c_str(), marker.length());
      len += marker.length();

      s << quint8(begin.length());
      len += sizeof(quint8);

      s.writeRawData(begin.toStdString().c_str(), begin.length());
      len += begin.length();

      s << quint8(end.length());
      len += sizeof(quint8);

      s.writeRawData(end.toStdString().c_str(), end.length());
      len += end.length();

      return b;
    }

    static ZNRHeader fromByteArray(const QByteArray& ba)
    {
      ZNRHeader result;

      QDataStream s(ba);

      // читаем сигнатуру файла и версию
      char    signature[SIGLEN];
      quint8  version;

      result.len += s.readRawData(&signature[0], SIGLEN);

      s >> version;
      result.len += sizeof(version);

      if(memcmp(&signature, ZNR_SIGNATURE, SIGLEN)) {
//        qDebug() << QString("Неверный формат файла");
         throw SvException(QString("Неверный формат файла"));
      }


      if(version > ZNR_VERSION) {
//        qDebug() << QString("Версия файла не поддерживается");
        throw SvException(QString("Версия файла не поддерживается"));
      }


      quint8 l;

      // длина маркера
      s >> l;
      result.len += sizeof(l);

      // маркер
      char m[l];

      result.len += s.readRawData(&m[0], l);
      result.marker = QString::fromLatin1(&m[0], l);

      // длина даты начала
      s >> l;
      result.len += sizeof(l);

      // дата начала (в текстовом формате)
      char b[l];

      result.len += s.readRawData(&b[0], l);
      result.begin = QString::fromLatin1(&b[0], l);

      // длина даты конца
      s >> l;
      result.len += sizeof(l);

      // дата конца (в текстовом формате)
      char e[l];

      result.len += s.readRawData(&e[0], l);
      result.begin = QString::fromLatin1(&e[0], l);

      return result;

    }
  };

  struct ZNR {

    ZNRHeader   header    = ZNRHeader();
    QByteArray  data      = QByteArray();

    ZNR()
    { }

    ZNR(const QString& marker, const QString& begin, const QString& end, const QByteArray& data = QByteArray()):
      header(marker, begin, end),
      data(data)
    { }

    QByteArray toByteArray()
    {
      QByteArray b;
      QDataStream s(&b, QIODevice::WriteOnly);

      s.writeRawData(header.toByteArray().data(), header.len);
      s.writeRawData(data.data(), data.length());

      return b;
    }

    static ZNR fromByteArray(QByteArray& ba)
    {
      ZNR result;
      QDataStream s(ba);

      result.header = ZNRHeader::fromByteArray(ba);
      result.data.resize(ba.length() - result.header.len);
      s.readRawData(result.data.data(), ba.length() - result.header.len);

      return result;

    }
  };

}


#endif // RECOVERY_DEFS

