#ifndef HMI_PROTOCOL_PARAMS
#define HMI_PROTOCOL_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../../../svlib/SvException/svexception.h"
#include "../../../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"

#define P_REGISTER        "register"
#define P_START_REGISTER  "start_register"
#define P_REGISTER_COUNT  "register_count"

namespace moxa {

  struct ProtocolParams {

    quint16 start_register = 0x1000;
    quint16 register_count = 10;
    quint16 interval       = 1000;

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

      P = P_START_REGISTER;
      if(object.contains(P)) {

        if(!object.value(P).isString())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Начальный регистр должен быть задан в виде строки двухбайтным числом в формате hex: \"0x0000\""));

        bool ok;
        p.start_register = object.value(P).toString().toUInt(&ok, 0);

        if(!ok)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Номер регистра должен быть задан в виде строки двухбайтным числом в формате hex: \"0x0000\""));
      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));


      P = P_REGISTER_COUNT;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Количество запрашиваемых регистров, должно быть задано целым числом"));

        p.register_count = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      P = P_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал запроса должен быть задан целым числом в миллисекундах"));

        p.interval = object.value(P).toInt();

      }
      else
        p.interval = 1000;

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

      j.insert(P_START_REGISTER,  QJsonValue(start_register).toString());
      j.insert(P_REGISTER_COUNT,  QJsonValue(register_count).toInt());
      j.insert(P_INTERVAL,        QJsonValue(interval).toInt());

      return j;

    }
  };

  struct SignalParams {

    quint16 registr       = 0;
//    quint16 offset        = 0;
//    quint16 len           = 0;

    static SignalParams fromJsonString(const QString& json_string) //throw (SvException)
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

        if(!(object.value(P).isString()))
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

//      P = P_OFFSET;
//      if(object.contains(P)) {

//        if(object.value(P).toInt(-1) < 0)
//          throw SvException(QString(IMPERMISSIBLE_VALUE)
//                                 .arg(P)
//                                 .arg(object.value(P).toVariant().toString())
//                                 .arg("Смещение должно быть задан целым числом"));

//        p.offset = object.value(P).toInt();

//      }
//      else
//        throw SvException(QString(MISSING_PARAM).arg(P));

//      P = P_LEN;
//      if(object.contains(P)) {

//        if(object.value(P).toInt(-1) < 0)
//          throw SvException(QString(IMPERMISSIBLE_VALUE)
//                            .arg(P)
//                            .arg(object.value(P).toVariant().toString())
//                            .arg("Длина данных должна быть задана целым числом"));

//        p.len = object.value(P).toInt();

//      }
//      else
//        throw SvException(QString(MISSING_PARAM).arg(P));

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
//      j.insert(P_OFFSET,    QJsonValue(offset).toInt());
//      j.insert(P_LEN,       QJsonValue(len).toInt());

      return j;

    }
  };
}

#endif // HMI_PROTOCOL_PARAMS

