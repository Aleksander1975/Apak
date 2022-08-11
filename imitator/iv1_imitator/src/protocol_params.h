#ifndef IV1_IMITATOR_PARAMS
#define IV1_IMITATOR_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

// Имя параметра "период поступления данных от ИВ-1 к АПАК",
// используемое при описании имитатора устройства ИВ-1 в файле описания
// имитаторов:
#define SEND_INTERVAL   "interval"

// Имя параметра "порядковый номер датчика",
// используемое при описании сигналов имитатора ИВ-1 в файле описания сигналов ИВ-1:
#define SERIAL_NUMBER_OF_THE_SENSOR     "serial_number"

// Имя параметра "старший байт заводского номер датчика",
// используемое при описании сигналов имитатора ИВ-1 в файле описания сигналов ИВ-1:
#define HIGH_BYTE_OF_SENSOR_FACTORY_NUMBER  "high_factory_number"

// Имя параметра "младший байт заводского номер датчика",
// используемое при описании сигналов имитатора ИВ-1 в файле описания сигналов ИВ-1:
#define LOW_BYTE_OF_SENSOR_FACTORY_NUMBER  "low_factory_number"

// Имя параметра "тип сигнала(температура / влажность)",
// используемое при описании сигналов имитатора ИВ-1 в файле описания сигналов ИВ-1:
#define SIGNAL_TYPE "signal_type"

// Значение периода поступления данных от устройства ИВ-1 в систему АПАК ПО УМОЛЧАНИЮ
// (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для устройства ИВ-1 не задано
// другое значение):
#define IV1_DEFAULT_SEND_INTERVAL 1000

namespace iv1 {

  struct ProtocolParams
  // Структура, хранящая параметр протокола - период в мс поступления данных от ИВ-1 в систему АПАК.
  {
    // Период поступления данных в мс от усторйства ИВ-1 в систему АПАК:
    quint16 send_interval = IV1_DEFAULT_SEND_INTERVAL;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла имитаторов для
    // сервера "mdserver", и содержащем информацию только об устройстве ИВ-1.

    // Выход: Структура ProtocolParams, заполненная параметром устройства ИВ-1 (периодом поступления
    // данных от устройства ИВ-1).
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

    static ProtocolParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    // Конструктор:

    // Вход: объект QJsonObject, описывающий устройство ИВ-1.

    // Выход: Структура ProtocolParams, заполненная параметром устройства ИВ-1 (периодом поступления
    // данных от устройства ИВ-1).
   {
      ProtocolParams p;
      QString P;

      P = SEND_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал отправки данных должен быть задан целым числом в миллисекундах"));

        p.send_interval = object.value(P).toInt(IV1_DEFAULT_SEND_INTERVAL);

      }
      else
        p.send_interval = IV1_DEFAULT_SEND_INTERVAL;

      return p;

    }

    QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      j.insert(SEND_INTERVAL,   QJsonValue(send_interval).toInt());

      return j;

    }
  };

  struct SignalParams
  // Структура, хранящая параметры сигнала устройства ИВ-1:
  // - порядковый номер датчика,
  // - старший байт заводского номера датчика,
  // - младший байт заводского номера датчика,
  // - тип сигнала: 0 - температура, 1 - влажность.
  {
    // Порядковый номер датчика:
    quint8 serial_number = 0;

    // Старший байт заводского номера датчика:
    quint8 high_factory_number = 0;

    // Младший байт заводского номера датчика:
    quint8 low_factory_number = 0;

    // Тип сигнала: 0 - температура, 1 - влажность:
    quint8 signal_type;


    static SignalParams fromJson(const QString& json_string) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла для имитатора
    // устройства ИВ-1, и содержащем информацию об одном сигнале.

    // Выход: Заполненная параметрами сигнала устройства ИВ-1 структура SignalParams.
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

    static SignalParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    // Конструктор:

    // Вход: объект QJsonObject, описывающий один сигнал устройства ИВ-1.

    // Выход: Заполненная параметрами сигнала устройства ИВ-1 структура SignalParams.
    {
      SignalParams p;
      QString P;

      // Считываем значение параметра "Последовательный номер датчика":
      P = SERIAL_NUMBER_OF_THE_SENSOR;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Порядковый номер датчика должен быть задан целым числом"));

        p.serial_number = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "Старший байт заводского номера датчика":
      P = HIGH_BYTE_OF_SENSOR_FACTORY_NUMBER;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Старший байт заводского номера датчика должен быть задан целым числом"));

        p.high_factory_number = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "Младший байт заводского номера датчика":
      P = LOW_BYTE_OF_SENSOR_FACTORY_NUMBER;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Младший байт заводского номера датчика должен быть задан целым числом"));

        p.low_factory_number = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "Тип сигнала" (0 - температура, 1 - влажность):
      P = SIGNAL_TYPE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Тип сигнала должен быть: 0 - для температуры или 1 - для влажности"));

        p.signal_type = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      return p;

    }

    QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      j.insert(SERIAL_NUMBER_OF_THE_SENSOR, QJsonValue(serial_number).toInt());
      j.insert(HIGH_BYTE_OF_SENSOR_FACTORY_NUMBER, QJsonValue(high_factory_number).toInt());
      j.insert(LOW_BYTE_OF_SENSOR_FACTORY_NUMBER, QJsonValue(low_factory_number).toInt());
      j.insert(SIGNAL_TYPE, QJsonValue(signal_type).toInt());

      return j;
    }
  };

}

#endif // IV1_IMITATOR_PARAMS

