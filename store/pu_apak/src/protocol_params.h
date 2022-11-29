#ifndef PROTOCOL_PARAMS
#define PROTOCOL_PARAMS


#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"


// === Имена параметров, используемых в файле описания устройств "config_apak.json",
// === при описании протокольной библиотеки АПАК для обмена между ПУ АПАК: ===

// Имя параметра "период посылки запроса POST к серверу другого ПУ АПАК":
#define SEND_INTERVAL       "send_interval"

// Значение по умолчанию для параметра "период посылки запроса POST к серверу другого ПУ АПАК":
#define DEFAULT_SEND_INTERVAL   1000


namespace apak {

  struct ProtocolParams
   // Структура, хранящая параметр протокола обмена между ПУ АПАК:
   //       - период посылки запроса POST на сервер другого ПУ АПАК.
  {
    // Период посылки запроса POST на сервер другого ПУ АПАК:
    quint16 send_interval  = 0;

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
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла
    // "config_apak.json" для сервера "mdserver", и содержащем информацию только
    // о ПУ АПАК.

    // Выход: Структура ProtocolParams, заполненная параметрами протокольной части ПУ АПАК.
    {
      ProtocolParams p;
      QString P;

      // Считываем значение параметра "период посылки запроса POST на сервер другого ПУ АПАК":
      P = SEND_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toString())
                            .arg("Интервал отправки данных на сервер другого ПУ АПАК должен быть задан целым числом в миллисекундах"));
        p.send_interval = object.value(P).toInt(DEFAULT_SEND_INTERVAL);
      }
      else
        p.send_interval = DEFAULT_SEND_INTERVAL;

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

      j.insert (SEND_INTERVAL,     QJsonValue(send_interval).toInt());

      return j;
    }
  };
}

#endif // PROTOCOL_PARAMS

