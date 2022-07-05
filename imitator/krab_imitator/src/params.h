#ifndef KRAB_PROTOCOL_PARAMS
#define KRAB_PROTOCOL_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../../../svlib/SvException/svexception.h"
#include "../../../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"

#define P_ADDRESS       "address"
#define P_FUNC_CODE     "func_code"
#define P_REGISTER_LEN  "register_len"

#define P_BYTE          "byte"

#define KRAB_DEFAULT_FUNC_CODE     0x10
#define KRAB_DEFAULT_SEND_INTERVAL 1000
#define KRAB_DEFAULT_REGISTER_LEN  2

namespace hmi {

  struct ProtocolParams {

    quint8  address       = 0;
    quint8  func_code     = KRAB_DEFAULT_FUNC_CODE;
    quint16 interval      = KRAB_DEFAULT_SEND_INTERVAL;
    quint8  register_len  = KRAB_DEFAULT_REGISTER_LEN;

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

      P = P_ADDRESS;
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

      P = P_FUNC_CODE;
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

      P = P_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал обновления данных должен быть задан целым числом в миллисекундах"));

        p.interval = object.value(P).toInt(KRAB_DEFAULT_SEND_INTERVAL);

      }
      else
        p.interval = KRAB_DEFAULT_SEND_INTERVAL;


      P = P_REGISTER_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Длина регистра должна быть задана целым числом в байтах"));

        p.register_len = object.value(P).toInt(KRAB_DEFAULT_REGISTER_LEN);

      }
      else
        p.register_len = KRAB_DEFAULT_REGISTER_LEN;

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

      j.insert(P_ADDRESS,       QJsonValue(address).toString());
      j.insert(P_INTERVAL,      QJsonValue(interval).toInt());
      j.insert(P_REGISTER_LEN,  QJsonValue(register_len).toInt());

      return j;

    }
  };

  struct SignalParams {

    quint16 byte       = 0;

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

      P = P_BYTE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Номер байта должен быть задан целым числом"));

        p.byte = object.value(P).toInt();

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

      j.insert(P_BYTE,  QJsonValue(byte).toInt());

      return j;

    }
  };
}

#endif // KRAB_PROTOCOL_PARAMS

