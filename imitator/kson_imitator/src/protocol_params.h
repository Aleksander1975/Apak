#ifndef PROTOCOL_PARAMS
#define PROTOCOL_PARAMS


#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"


// === Имена параметров, используемых при описании протокольной части имитатора КСОН ===
// === в файле описания имитаторов "config_apak_imitator.json": ===

// Имя параметра "период поступления данных от имитатора КСОН к АПАК":
#define SEND_INTERVAL       "send_interval"

// Имя параметра "предельно допустимое время между информационными кадрами от АПАК":
#define RECEIVE_INTERVAL    "rec_interval"

// Имя параметра "предельно допустимое время от передачи информационного кадра АПАК'у
// до получения пакета подтверждения от АПАК'a":
#define CONFORM_INTERVAL   "conform_interval"

// Имя параметра "размер информационного блока в информационном кадре ПОСЫЛАЕМОМ имитатором КСОН к АПАК":
#define SEND_DATA_LEN        "send_data_len"

// Имя параметра "размер информационного блока в информационном кадре ПРИНИМАЕМОМ имитатором КСОН от АПАК":
#define RECEIVE_DATA_LEN     "receive_data_len"

// Имя параметра "предельно допустимое количество идущих подряд ошибок взаимодействия":
#define NUMBER_OF_ERRORS "number_errror"

// Значение по умолчанию для параметра "период поступления данных от АПАК к КСОН":
#define DEFAULT_SEND_INTERVAL   1000

// Значение по умолчанию для параметра "предельно допустимое время между
// информационными пакетами от АПАК":
#define DEFAULT_RECEIVE_INTERVAL 6000

// Значение по умолчанию для параметра "предельно допустимое время от передачи информационного кадра АПАК'у
// до получения пакета подтверждения от АПАК'a":
#define DEFAULT_CONFORM_INTERVAL  200

// Значение по умолчанию для параметра "размер информационного блока
// в информационном кадре ПОСЫЛАЕМОМ имитатором КСОН к АПАК":
#define DEFAULT_SEND_DATA_LEN     33

// Значение по умолчанию для параметра "размер информационного блока
// в информационном кадре ПРИНИМАЕМОМ имитатором КСОН от АПАК":
#define DEFAULT_RECEIVE_DATA_LEN    2


// Значение по умолчанию для параметра "предельно допустимое
// количество идущих подряд ошибок взаимодействия":
#define DEFAULT_NUMBER_OF_ERRORS  3



namespace apak {

  struct ProtocolParams
   // Структура, хранящая параметры протокола обмена АПАК c КСОН:
   //       - период поступления данных от имитатора КСОН к АПАК,
   //       - предельно допустимое время между информационными кадрами от АПАК,
   //       - предельно допустимое время от передачи информационного кадра АПАК'у
   //             до получения пакета подтверждения от АПАК'a,
   //       - размер информационного блока в информационном кадре ПОСЫЛАЕМОМ имитатором КСОН к АПАК,
   //       - размер информационного блока в информационном кадре ПРИНИМАЕМОМ имитатором КСОН от АПАК,
   //       - предельно допустимое количество идущих подряд ошибок взаимодействия.
  {
    // Период поступления данных от имитатора КСОН к АПАК:
    quint16 send_interval  = 0;

    // Предельно допустимое время между информационными кадрами от АПАК:
    quint16 receive_interval = 0;

    // Предельно допустимое время от передачи информационного кадра АПАК'у
    // до получения пакета подтверждения от АПАК'a:
    quint16 conform_interval = 0;

    // Размер информационного блока в информационном кадре ПОСЫЛАЕМОМ имитатором КСОН к АПАК:
    quint16 send_data_len = 0;

    // Размер информационного блока в информационном кадре ПРИНИМАЕМОМ имитатором КСОН от АПАК:
    quint16 receive_data_len = 0;

    // Предельно допустимое количество идущих подряд ошибок взаимодействия:
    quint16 numberOfErrors = 0;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(QString("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + err.errorString());

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
    // "config_apak_imitator.json" для сервера "mdserver", и содержащем информацию только
    // об имитаторе устройства КСОН.

    // Выход: Структура ProtocolParams, заполненная параметрами протокольной части
    // имитатора устройства КСОН
    {
      ProtocolParams p;
      QString P;

      // Считываем значение параметра "период поступления данных от имитатора КСОН к АПАК":
      P = SEND_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toString())
                            .arg("Интервал отправки данных должен быть задан целым числом в миллисекундах"));
        p.send_interval = object.value(P).toInt(DEFAULT_SEND_INTERVAL);
      }
      else
        p.send_interval = DEFAULT_SEND_INTERVAL;

      // Считываем значение параметра "предельно допустимое время между информационными кадрами от АПАК":
      P = RECEIVE_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Предельно допустимый интервал между информационными кадрами должен быть задан целым числом в миллисекундах"));

        p.receive_interval = object.value(P).toInt(DEFAULT_RECEIVE_INTERVAL);
      }
      else
        p.receive_interval = DEFAULT_RECEIVE_INTERVAL;

      // Считываем значение параметра "предельно допустимое время от передачи информационного кадра АПАК'у
      // до получения пакета подтверждения от АПАК'a":
      P = CONFORM_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Предельно допустимый интервал между информационным кадром и пакетом подтверждения должен быть задан целым числом в миллисекундах"));

        p.conform_interval = object.value(P).toInt(DEFAULT_CONFORM_INTERVAL);
      }
      else
        p.conform_interval = DEFAULT_CONFORM_INTERVAL;

      // Считываем значение параметра "размер информационного блока в информационном кадре ПОСЫЛАЕМОМ имитатором КСОН к АПАК":
      P = SEND_DATA_LEN;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Размер информационного блока в информационном кадре ПОСЫЛАЕМОМ имитатором КСОН к АПАК должен быть задан целым числом"));

        p.send_data_len = object.value(P).toInt(DEFAULT_SEND_DATA_LEN);
      }
      else
        p.send_data_len = DEFAULT_SEND_DATA_LEN;

      // Считываем значение параметра "размер информационного блока в информационном кадре ПРИНИМАЕМОМ имитатором КСОН от АПАК должен быть задан целым числом":
      P = RECEIVE_DATA_LEN;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Размер информационного блока в информационном кадре ПРИНИМАЕМОМ имитатором КСОН от АПАК должен быть задан целым числом"));

        p.receive_data_len = object.value(P).toInt(DEFAULT_RECEIVE_DATA_LEN);
      }
      else
        p.receive_data_len = DEFAULT_RECEIVE_DATA_LEN;

      // Считываем значение параметра "предельно допустимое количество идущих подряд ошибок взаимодействия":
      P = NUMBER_OF_ERRORS;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КСОН: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
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

      j.insert (SEND_INTERVAL,     QJsonValue(send_interval).toInt());
      j.insert (RECEIVE_INTERVAL,  QJsonValue(receive_interval).toInt());
      j.insert (CONFORM_INTERVAL,  QJsonValue(conform_interval).toInt());
      j.insert (SEND_DATA_LEN,     QJsonValue(send_data_len).toInt());
      j.insert (RECEIVE_DATA_LEN,  QJsonValue(receive_data_len).toInt());
      j.insert (NUMBER_OF_ERRORS,  QJsonValue(numberOfErrors).toInt());

      return j;
    }
  };
}

#endif // PROTOCOL_PARAMS

