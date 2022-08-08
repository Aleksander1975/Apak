﻿#ifndef ZN_K1_DEFS
#define ZN_K1_DEFS

/**********************************************************************
 *  автор Свиридов С.А. НИИ РПИ
 * *********************************************************************/

#include <QtGlobal>
#include <QHostAddress>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/SvException/svexception.h"
#include "../../../Modus/global/global_defs.h"

#include "zn_global.h"

namespace zn1 {

  struct StorageParams {

    QHostAddress host      = QHostAddress();
    quint16      port      = DEFAULT_PORT;
    quint16      interval  = DEFAULT_INTERVAL;
    QString      zone      = QString();
    QString      pass      = QString();
    quint32      queue_len = DEFAULT_QUEUE_LEN;
    quint32      write_buf = DEFAULT_WRITE_BUFFER_LEN;

    static StorageParams fromJsonString(const QString& json_string) //throw (SvException)
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

    static StorageParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      StorageParams p;
      QString P;


      // host
      P = P_HOST;
      if(object.contains(P)) {

        QString host = object.value(P).toString("").toLower();

        if(QHostAddress(host).toIPv4Address() == 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Допускаются только ip адреса в формате 'xxx.xxx.xxx.xxx' (192.168.1.1)"));

        p.host = QHostAddress(host);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      // receive port
      P = P_PORT;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Номер порта должен быть задан целым положительным числом в диапазоне [1..65535]"));

        p.port = object.value(P).toInt(DEFAULT_PORT);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      // interval
      P = P_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Интервал записи данных должен быть задан целым положительным числом"));

        p.interval = object.value(P).toInt(DEFAULT_INTERVAL);

      }
      else
        p.interval = DEFAULT_INTERVAL;

      // zone name
      P = P_ZONE;
      if(object.contains(P)) {

        p.zone = object.value(P).toString();

        if(p.zone.isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Имя зоны для записи не может быть пустым"));
      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      // pass
      P = P_PASS;
      if(object.contains(P)) {

        p.pass = object.value(P).toString();

        if(p.pass.isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Пароль для подключения к накопителю не может быть пустым"));
      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      // queue length
      P = P_QUEUE_LEN;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Длина очереди на запись должна быть задан целым положительным числом"));

        p.queue_len = object.value(P).toInt(DEFAULT_QUEUE_LEN);

      }
      else
        p.queue_len = DEFAULT_QUEUE_LEN;

      /* write biffer length */
      P = P_WRITE_BUF_SIZE;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                             .arg(P).arg(object.value(P).toVariant().toString())
                             .arg("Размер буфера записи должен быть задан целым положительным числом в байтах"));

        p.write_buf = object.value(P).toInt(DEFAULT_WRITE_BUFFER_LEN);

      }
      else
        p.write_buf = DEFAULT_WRITE_BUFFER_LEN;

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

      j.insert(P_HOST,      QJsonValue(host.toString()).toString());
      j.insert(P_PORT,      QJsonValue(static_cast<int>(port)).toInt());
      j.insert(P_INTERVAL,  QJsonValue(static_cast<int>(interval)).toInt());
      j.insert(P_ZONE,      QJsonValue(zone).toString());
      j.insert(P_PASS,      QJsonValue(pass).toString());
      j.insert(P_QUEUE_LEN, QJsonValue(static_cast<int>(queue_len)).toInt());
      j.insert(P_WRITE_BUF_SIZE, QJsonValue(static_cast<int>(write_buf)).toInt());

      return j;

    }
  };


  struct SignalParams {

    QString marker  = "";

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

      P = P_MARKER;
      if(object.contains(P)) {

        if(!object.value(P).isString())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                            .arg("Маркер записи должен быть задан строкой."));

        p.marker = object.value(P).toString();

      }
      else
        p.marker = "";

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

      j.insert(P_MARKER,  QJsonValue(marker).toString());

      return j;

    }
  };

}



#endif // ZN_K1_DEFS

