#ifndef GAMMA_IMITATOR_PARAMS
#define GAMMA_IMITATOR_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"

#define P_ALARM_NAME "alarm_name"

namespace apak {

  struct ProtocolParams {

    quint16 send_interval = 1000;
    QString alarm_name    = "FireAlarm";

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

      P = P_SEND_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал отправки данных должен быть задан целым числом в миллисекундах"));

        p.send_interval = object.value(P).toInt(1000);

      }
      else
        p.send_interval = 1000;

      P = P_ALARM_NAME;
      if(object.contains(P)) {

        p.alarm_name = object.value(P).toString("FireAlarm");

      }
      else
        p.alarm_name = "FireAlarm";

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

      j.insert(P_SEND_INTERVAL,   QJsonValue(send_interval).toInt());
      j.insert(P_ALARM_NAME,      QJsonValue(alarm_name).toString());

      return j;

    }
  };
}

#endif // GAMMA_IMITATOR_PARAMS

