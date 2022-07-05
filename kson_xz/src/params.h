#ifndef UPS_PROTOCOL_PARAMS
#define UPS_PROTOCOL_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../../../svlib/SvException/svexception.h"
#include "../../../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"

#define P_ADDRESS       "address"
#define P_FUNC_CODE     "func_code"
#define P_REGISTER      "register"
#define P_OFFSET        "offset"
#define P_LEN           "len"
#define P_REGISTER_LEN  "register_len"

#define UPS_DEFAULT_FUNC_CODE     0x04
#define UPS_DEFAULT_SEND_INTERVAL 1000
#define UPS_DEFAULT_REGISTER_LEN  2

namespace ups {

  struct ProtocolParams {

    quint8  address       = 0;
    quint8  func_code     = UPS_DEFAULT_FUNC_CODE;
    quint16 interval      = UPS_DEFAULT_SEND_INTERVAL;
    quint8  register_len  = UPS_DEFAULT_REGISTER_LEN;

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

        p.func_code = object.value(P).toInt(UPS_DEFAULT_FUNC_CODE);

      }
      else
        p.func_code = UPS_DEFAULT_FUNC_CODE;

      P = P_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал обновления данных должен быть задан целым числом в миллисекундах"));

        p.interval = object.value(P).toInt(UPS_DEFAULT_SEND_INTERVAL);

      }
      else
        p.interval = UPS_DEFAULT_SEND_INTERVAL;


      P = P_REGISTER_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Длина регистра должна быть задана целым числом в байтах"));

        p.register_len = object.value(P).toInt(UPS_DEFAULT_REGISTER_LEN);

      }
      else
        p.register_len = UPS_DEFAULT_REGISTER_LEN;

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

    quint16 registr       = 0;
    quint16 offset        = 0;
    quint16 len           = 0;

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

      P = P_REGISTER;
      if(object.contains(P)) {

        if(!(object.value(P).isString() && object.value(P).toString().startsWith("0x")))
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Номер регистра должен быть задан в виде строки двухбайтным числом в формате hex: \"0x0000\""));

        bool ok;
        p.registr = object.value(P).toString().toUInt(&ok, 0);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Номер регистра должен быть задан в виде строки двухбайтным числом в формате hex: \"0x0000\""));
      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      P = P_OFFSET;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Смещение должно быть задан целым числом"));

        p.offset = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      P = P_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Длина данных должна быть задана целым числом"));

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

      j.insert(P_REGISTER,  QJsonValue(registr).toInt());
      j.insert(P_OFFSET,    QJsonValue(offset).toInt());
      j.insert(P_LEN,       QJsonValue(len).toInt());

      return j;

    }
  };
}

#endif // UPS_PROTOCOL_PARAMS

