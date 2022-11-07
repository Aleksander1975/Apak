#ifndef DUMMY_IMITATOR_PARAMS
#define DUMMY_IMITATOR_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

// Имя параметра "период поступления данных от "заглушки" к АПАК",
// используемое при описании "заглушки" в файле описания
// имитаторов:
#define SEND_INTERVAL   "interval"

// Значение периода поступления данных от "заглушки" в систему АПАК ПО УМОЛЧАНИЮ
// (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для "заглушки" не задано
// другое значение):
#define DUMMY_DEFAULT_SEND_INTERVAL 1000

namespace Dummy {

  struct ProtocolParams
  // Структура, хранящая параметр протокола - период в мс поступления данных от "заглушки" в систему АПАК.
  {
    // Период поступления данных в мс от "заглушки" в систему АПАК:
    quint16 send_interval = DUMMY_DEFAULT_SEND_INTERVAL;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла имитаторов для
    // сервера "mdserver", и содержащем информацию только о "заглушке".

    // Выход: Структура ProtocolParams, заполненная параметром "заглушки" (периодом поступления
    // данных от "заглушки").
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

    // Вход: объект QJsonObject, описывающий "заглушку".

    // Выход: Структура ProtocolParams, заполненная параметром "заглушки" (периодом поступления
    // данных от "заглушки").
   {
      ProtocolParams p;
      QString P;

      P = SEND_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал отправки данных должен быть задан целым числом в миллисекундах"));

        p.send_interval = object.value(P).toInt(DUMMY_DEFAULT_SEND_INTERVAL);

      }
      else
        p.send_interval = DUMMY_DEFAULT_SEND_INTERVAL;

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

      j.insert(SEND_INTERVAL,   QJsonValue(send_interval).toInt());

      return j;

    }
  };

  struct SignalParams
  // Структура, хранящая параметры сигнала имитатора устройства. Для "заглушки" не используется.
  {
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

      return j;
    }
  };

}

#endif // DUMMY_IMITATOR_PARAMS

