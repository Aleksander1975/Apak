#ifndef KRAB_PROTOCOL_PARAMS
#define KRAB_PROTOCOL_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

#define ADDRESS         "address"
#define FUNC_CODE       "func_code"
#define SEND_INTERVAL   "interval"
#define DATA_LEN        "data_len"

#define BYTE          "byte"
#define OFFSET        "offset"
#define LEN           "len"


#define KRAB_DEFAULT_FUNC_CODE     0x10
#define KRAB_DEFAULT_SEND_INTERVAL 1000
#define KRAB_DEFAULT_DATA_LEN      66

namespace krab {

  struct ProtocolParams {

    quint8  address       = 0;
    quint8  func_code     = KRAB_DEFAULT_FUNC_CODE;
    quint16 send_interval = KRAB_DEFAULT_SEND_INTERVAL;
    quint8  data_len = KRAB_DEFAULT_DATA_LEN;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
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
    {
      ProtocolParams p;
      QString P;

      P = ADDRESS;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Адрес устройства Modbus должен быть задан целым числом"));

        p.address = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      P = FUNC_CODE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Код функции Modbus должен быть задан целым числом"));

        p.func_code = object.value(P).toInt(KRAB_DEFAULT_FUNC_CODE);

      }
      else
        p.func_code = KRAB_DEFAULT_FUNC_CODE;

      P = SEND_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал обновления данных должен быть задан целым числом в миллисекундах"));

        p.send_interval = object.value(P).toInt(KRAB_DEFAULT_SEND_INTERVAL);

      }
      else
        p.send_interval = KRAB_DEFAULT_SEND_INTERVAL;

      P = DATA_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Длина поля данных запроса на запись от КРАБа к АПАК должна быть задана целым числом байт"));

        p.data_len = object.value(P).toInt(KRAB_DEFAULT_DATA_LEN);

      }
      else
        p.data_len = KRAB_DEFAULT_DATA_LEN;

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

      j.insert (ADDRESS,       QJsonValue(address).toString());
      j.insert (FUNC_CODE,    QJsonValue (func_code).toString());
      j.insert (SEND_INTERVAL,      QJsonValue(send_interval).toInt());
      j.insert (DATA_LEN,   QJsonValue(data_len).toInt());)

      return j;

    }
  };

  struct SignalParams {

    // Смещение байта, в котором хранится значение сигнала, от начала пакета
    // "запроса на запись" (количество байт от начала пакета):
    quint8 byte = 0;

    // Смещение области битов, в которой хранится значение сигнала, от начала байта
    // (количество бит от начала байта):
    quint8 offset = 0;

    // Размер области бит, которая хранит значение сигнала (количество бит).
    quint8 len = 0;

    static SignalParams fromJson(const QString& json_string) //throw (SvException)
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
    {
      SignalParams p;
      QString P;

      // Считываем значение параметра "смещение байта от начала пакета":
      P = BYTE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Смещение байта от начала пакета должно быть задано целым числом"));

        p.byte = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "смещение области битов от начала байта":
      P = OFFSET;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Cмещение области битов от начала байта должнo быть задано целым числом"));

        p.offset = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "размер области битов":
      P = LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Размер области битов должен быть задан целым числом"));

        p.len = object.value(P).toInt();

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

      j.insert(BYTE,  QJsonValue(byte).toInt());
      j.insert(OFFSET,  QJsonValue(byte).toInt());
      j.insert(LEN,  QJsonValue(byte).toInt());
      return j;

    }
  };
}

#endif // KRAB_PROTOCOL_PARAMS

