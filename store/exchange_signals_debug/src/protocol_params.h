#ifndef PROTOCOL_PARAMS
#define PROTOCOL_PARAMS


#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"


// === Имена параметров, используемых при описании протокольной библиотеки "libexchange_signals_debug" ===
// === для работы с модулем МП МОС в файле описания устройств "config_apak.json": ===

// Имя параметра "период установки значений сигналов, подлежащих передаче через МОС" в мс:
#define SET_SIGNAL_INTERVAL         "set_interval"

// Имя параметра "период чтения значений и параметров сигналов, принятых через МОС" в мс:
#define READ_SIGNAL_INTERVAL        "read_interval"

// Значение по умолчанию для параметра "период установки значений сигналов, подлежащих передаче через МОС":
#define DEFAULT_SET_SIGNAL_INTERVAL     1000

// Значение по умолчанию для параметра "период чтения значений и параметров сигналов, принятых через МОС":
#define DEFAULT_READ_SIGNAL_INTERVAL    1000


namespace apak {

  struct ProtocolParams
   // Структура, хранящая параметры, используемые при описании протокольной библиотеки
   // "libexchange_signals_debug" для работы с модулем МП МОС в файле описания устройств
   // "config_apak.json":
   //       - период установки значений сигналов, подлежащих передаче через МОС,
   //       - период чтения значений и параметров сигналов, принятых через МОС.
  {
    // Период установки значений сигналов, подлежащих передаче через МОС:
    quint16 set_signal_interval  = 0;

    // Период чтения значений и параметров сигналов, принятых через МОС:
    quint16 read_signal_interval = 0;


    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
      {
        throw SvException(QString ("МП МОС: Ошибка при разборе параметров протокола: ") + err.errorString());
      }
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
    // о модуле МП МОС.

    // Выход: Структура ProtocolParams, заполненная параметрами протокольной части
    // модуля МП МОС.
    {
      ProtocolParams p;
      QString P;

      // Считываем значение параметра "период установки значений сигналов, подлежащих передаче через МОС":
      P = SET_SIGNAL_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("МП МОС: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toString())
                            .arg("Период установки значений сигналов, подлежащих передаче через МОС должен быть задан целым числом в миллисекундах"));
        p.set_signal_interval = object.value(P).toInt(DEFAULT_SET_SIGNAL_INTERVAL);
      }
      else
        p.set_signal_interval = DEFAULT_SET_SIGNAL_INTERVAL;

      // Считываем значение параметра "период чтения значений и параметров сигналов, принятых через МОС":
      P = READ_SIGNAL_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("МП МОС: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Период чтения значений и параметров сигналов, принятых через МОС должен быть задан целым числом в миллисекундах"));

        p.read_signal_interval = object.value(P).toInt(DEFAULT_READ_SIGNAL_INTERVAL);
      }
      else
        p.read_signal_interval = DEFAULT_READ_SIGNAL_INTERVAL;

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

      j.insert (SET_SIGNAL_INTERVAL,     QJsonValue(set_signal_interval));
      j.insert (READ_SIGNAL_INTERVAL,  QJsonValue(read_signal_interval));

      return j;
    }
  };
}

#endif // PROTOCOL_PARAMS

