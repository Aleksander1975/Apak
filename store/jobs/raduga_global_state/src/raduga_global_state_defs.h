#ifndef RADUGA_GLOBAL_STATE_DEFS
#define RADUGA_GLOBAL_STATE_DEFS

/**********************************************************************
 *  автор Свиридов С.А. НИИ РПИ
 * *********************************************************************/

#include <QtGlobal>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../../svlib/SvException/svexception.h"
#include "../../../../../Modus/global/global_defs.h"

#define P_WATCH "watch"

namespace apak {

  namespace rdga {

    struct ProtocolParams {

      quint16 interval      = DEFAULT_INTERVAL;

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

        P = P_INTERVAL;
        if(object.contains(P)) {

          if(object.value(P).toInt(-1) < 0)
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                              .arg(P)
                              .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                              .arg("Интервал обновления данных должен быть задан целым числом в миллисекундах"));

          p.interval = object.value(P).toInt(DEFAULT_INTERVAL);

        }
        else
          p.interval = DEFAULT_INTERVAL;

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

        j.insert(P_INTERVAL,      QJsonValue(interval).toInt());

        return j;

      }
    };

    struct SignalParams {

      bool watch = true;

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


  //      // watch
        P = P_WATCH;
        if(object.contains(P)) {

          if(!object.value(P).isBool())
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                               .arg(P).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                               .arg("Тип параметра должен быть boolean [true|false]"));

          p.watch = object.value(P).toBool(true);

        }
        else
          p.watch = true;

        return p;

      }

      QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
      {
        QJsonDocument jd;
        jd.setObject(toJsonObject());

        return QString(jd.toJson(format));
      }

      QJsonObject toJsonObject() const
      {
        QJsonObject j;

        j.insert(P_WATCH,      QJsonValue(watch).toBool());

        return j;

      }
    };
  }
}


#endif // RADUGA_GLOBAL_STATE_DEFS

