#ifndef PROTOCOL_PARAMS
#define PROTOCOL_PARAMS


#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"


// === Имена параметров, используемых при описании протокольной библиотеки "libapak_kson_packet" ===
// === для работы с устройством КСОН в файле описания устройств "config_apak.json": ===

// Имя параметра "период поступления данных от АПАК к КСОН":
#define SEND_INTERVAL       "send_interval"

// Имя параметра "предельно допустимое время между информационными кадрами от КСОН":
#define RECEIVE_INTERVAL    "rec_interval"

// Имя параметра "предельно допустимое время от передачи информационного кадра КСОН'у
// до получения пакета подтверждения от КСОН'a":
#define CONFORM_INTERVAL   "conform_interval"

// Имя параметра "размер информационного блока в информационном кадре от АПАК к КСОН":
#define DATA_LEN        "data_len"

// Имя параметра "предельно допустимое количество идущих подряд ошибок взаимодействия":
#define NUMBER_OF_ERRORS "number_errror"

// Значение по умолчанию для параметра "период поступления данных от АПАК к КСОН":
#define DEFAULT_SEND_INTERVAL   1000

// Значение по умолчанию для параметра "предельно допустимое время между
// информационными пакетами от КСОН":
#define DEFAULT_RECEIVE_INTERVAL 6000

// Значение по умолчанию для параметра "предельно допустимое время от передачи информационного кадра КСОН'у
// до получения пакета подтверждения от КСОН'a":
#define DEFAULT_CONFORM_INTERVAL  200

// Значение по умолчанию для параметра "размер информационного блока
// в информационном кадре от АПАК к КСОН":
#define DEFAULT_DATA_LEN    2

// Значение по умолчанию для параметра "предельно допустимое
// количество идущих подряд ошибок взаимодействия":
#define DEFAULT_NUMBER_OF_ERRORS  3



namespace apak {

  struct ProtocolParams
   // Структура, хранящая параметры протокола обмена АПАК c КСОН:
   //       - период поступления данных от АПАК к КСОН,
   //       - предельно допустимое время между информационными кадрами от КСОН,
   //       - предельно допустимое время от передачи информационного кадра КСОН'у
   //             до получения пакета подтверждения от КСОН'a,
   //       - размер информационного блока в информационном кадре от АПАК к КСОН,
   //       - предельно допустимое количество идущих подряд ошибок взаимодействия.
  {
    // Период поступления данных от АПАК к КСОН:
    quint16 send_interval  = 0;

    // Предельно допустимое время между информационными кадрами от КСОН:
    quint16 receive_interval = 0;

    // Предельно допустимое время от передачи информационного кадра КСОН'у
    // до получения пакета подтверждения от КСОН'a:
    quint16 conform_interval = 0;

    // Размер информационного блока в информационном кадре от АПАК к КСОН:
    quint16 data_len = 0;

    // Предельно допустимое количество идущих подряд ошибок взаимодействия:
    quint16 numberOfErrors = 0;

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
    // об устройстве КСОН.

    // Выход: Структура ProtocolParams, заполненная параметрами протокольной части
    // устройства КСОН
    {
      ProtocolParams p;
      QString P;

      // Считываем значение параметра "период поступления данных от АПАК к КСОН":
      P = SEND_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toString())
                            .arg("Интервал отправки данных должен быть задан целым числом в миллисекундах"));
        p.send_interval = object.value(P).toInt(DEFAULT_SEND_INTERVAL);
      }
      else
        p.send_interval = DEFAULT_SEND_INTERVAL;

      // Считываем значение параметра "предельно допустимое время между информационными кадрами от КСОН":
      P = RECEIVE_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Предельно допустимый интервал между информационными кадрами должен быть задан целым числом в миллисекундах"));

        p.receive_interval = object.value(P).toInt(DEFAULT_RECEIVE_INTERVAL);
      }
      else
        p.receive_interval = DEFAULT_RECEIVE_INTERVAL;

      // Считываем значение параметра "предельно допустимое время от передачи информационного кадра КСОН'у
      // до получения пакета подтверждения от КСОН'a":
      P = CONFORM_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Предельно допустимый интервал между информационным кадром и пакетом подтверждения должен быть задан целым числом в миллисекундах"));

        p.conform_interval = object.value(P).toInt(DEFAULT_CONFORM_INTERVAL);
      }
      else
        p.conform_interval = DEFAULT_CONFORM_INTERVAL;

      // Считываем значение параметра "размер информационного блока в информационном кадре от АПАК к КСОН":
      P = DATA_LEN;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Размер информационного блока в информационном кадре от АПАК к КСОН должен быть задан целым числом"));

        p.data_len = object.value(P).toInt(DEFAULT_DATA_LEN);
      }
      else
        p.data_len = DEFAULT_DATA_LEN;

      // Считываем значение параметра "предельно допустимое количество идущих подряд ошибок взаимодействия":
      P = NUMBER_OF_ERRORS;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Предельно допустимое количество идущих подряд ошибок взаимодействия должно быть задано целым числом"));

        p.numberOfErrors = object.value(P).toInt(DEFAULT_NUMBER_OF_ERRORS);
      }
      else
        p.numberOfErrors = DEFAULT_NUMBER_OF_ERRORS;

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

      j.insert( SEND_INTERVAL,     QJsonValue(send_interval).toInt());
      j.insert (RECEIVE_INTERVAL,  QJsonValue(receive_interval).toInt());
      j.insert (CONFORM_INTERVAL,  QJsonValue(conform_interval).toInt());
      j.insert (DATA_LEN,          QJsonValue(data_len).toInt());
      j.insert (NUMBER_OF_ERRORS,  QJsonValue(numberOfErrors).toInt());

      return j;
    }
  };
}

#endif // PROTOCOL_PARAMS

