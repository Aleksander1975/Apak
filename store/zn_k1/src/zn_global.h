#ifndef ZN_GLOBAL
#define ZN_GLOBAL

#include <QDataStream>
#include <QDateTime>
#include "QtNetwork/qhostaddress.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#include "../../../svlib/SvCRC/1.1/sv_crc.h"
#include "../../../svlib/SvException/svexception.h"
#include "../../../Modus/global/global_defs.h"
#include "../../../Modus/global/signal/sv_signal.h"
#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"

#define P_HOST                    "host"
#define P_ZONE                    "zone"
#define P_QUEUE_LEN               "queue"
#define P_WRITE_BUF_SIZE          "bufsize"

#define P_MARKER                  "marker"

#define DEFAULT_DATETIME_FORMAT   "yyyy-MM-dd hh:mm:ss"


///дефолтные значения
#define DEFAULT_PORT              10000
#define DEFAULT_INTERVAL          1000
#define DEFAULT_QUEUE_LEN         1000
#define DEFAULT_WRITE_BUFFER_LEN  102400  // bytes. 100 kB

#define DEFAULT_READ_BUFFER_LEN   16      // megabytes. 16 MB
#define DEFAULT_READ_INTERVAL     5       // 1 сек.
#define DEFAULT_ZONE_SIZE_MB      3840    // 3.75 Gb

/// команды для работы с устройством
#define CMD_CONNECT               200
#define CMD_ANSWER                100
#define CMD_WRITE                 501
#define ACC_CODE_WRITE            2
#define CMD_READ                  502
#define ACC_CODE_READ             3
#define CMD_SIZE                  503

/// служебные определения
#define TAG_CONNECTION_STATE      "c"
#define TAG_AUTHORIZATION         "a"
#define TAG_WRITE_STATE           "w"

#define STATE_NO_CONNECTION       0
#define STATE_NO_AUTHORITY        0
#define STATE_NO_WRITING          0
#define STATE_NO_READING          0

#define STATE_CONNECTION_OK       1
#define STATE_AUTHORITY_OK        1
#define STATE_WRITING_OK          1
#define STATE_READING_OK          1

#define STATE_OK                  "OK"

namespace zn1 {

  /// общие. структура записи
  const char  BunchMarker[4] = {char(0xAA), char(0xAA), char(0xAA), char(0xAA)};

#pragma pack(push,1)
  struct BunchHeader {

    char    marker[sizeof(zn1::BunchMarker)];
    qint64  coarseDateTime;
    quint32 dataLength;
    quint16 crc16;

    explicit BunchHeader():
      zn1::BunchHeader{0}
    {

    }

    explicit BunchHeader(qint64 coarseDateTime):
      coarseDateTime(coarseDateTime),
      dataLength(0),
      crc16(0)
    {
      memcpy(&marker[0], &zn1::BunchMarker[0], sizeof(marker));
    }

    bool fromRawData(const char* data)
    {
      QByteArray b(data, sizeof(marker) + sizeof(coarseDateTime)  + sizeof(dataLength) + sizeof(crc16));
      QDataStream s(b);
      s.setByteOrder(QDataStream::LittleEndian); // !

      s.readRawData(&marker[0], sizeof(marker));

      s >> coarseDateTime >> dataLength >> crc16;

      return (crc16 == crc::crc16ccitt(data, sizeof(marker) + sizeof(coarseDateTime) + sizeof(dataLength)));
    }

    QByteArray toByteArray(quint32 dataLength)
    {
      QByteArray result;

      QDataStream stream(&result, QIODevice::WriteOnly);
      stream.setByteOrder(QDataStream::LittleEndian);

      stream.writeRawData(&marker[0], sizeof(marker));

      stream << coarseDateTime
             << dataLength;
      quint16 crc16 = crc::crc16ccitt(result);
//      qDebug() << "toByteArray" << QString(result.toHex()) << QString::number(crc16, 16);
      stream << crc16;

      return result;

    }

    quint32 length()
    {
      return sizeof(marker) + sizeof(coarseDateTime) + sizeof(quint32) + sizeof(quint16);
    }

  };
#pragma pack(pop)

  class Record
  {
  public:
    Record():
      m_date_time(0),
      m_apak_marker(QString()),
      m_record_data(QByteArray())
    {

    }

    Record(qint64  dateTime, const QString& apak_marker, const QByteArray& data):
      m_date_time(dateTime),
      m_apak_marker(apak_marker)
    {
      QDataStream stream(&m_record_data, QIODevice::WriteOnly);
      stream.setByteOrder(QDataStream::LittleEndian);

      stream << dateTime;
      stream << static_cast<quint16>(data.length());
      stream << static_cast<quint16>(apak_marker.length());

      stream.writeRawData(apak_marker.toStdString().c_str(), apak_marker.length());
      stream.writeRawData(data.data(), data.length());

//      qDebug() << "Record" << QDateTime::fromMSecsSinceEpoch(dateTime)
//               << apak_marker << data.length();
    }

    static int headsz() { return sizeof(m_date_time) + sizeof(m_data_length) + sizeof(m_apak_marker_length); }

    bool fromRawData(const char* data)
    {
//qDebug() << 11;
      QByteArray r{data, headsz()};
      QDataStream stream(r);
      stream.setByteOrder(QDataStream::LittleEndian); // !
//qDebug() << 12;
      stream >> m_date_time >> m_data_length >> m_apak_marker_length;
//qDebug() << 13;
      /// вводим жесткие ограничения на значения, которые могут быть в заголовке записи
      if((m_date_time <= 0) || (m_data_length == 0) || (m_apak_marker_length > 32) || (m_apak_marker_length == 0)) {
        qDebug() << "fromRawData false";
        return false;
      }
//qDebug() << 14;
//      qDebug() << "apak_marker" << QByteArray(data + sz, m_apak_marker_length).toHex();

      m_apak_marker = QString::fromLocal8Bit(data + headsz(), m_apak_marker_length);
      m_record_data = QByteArray::fromRawData(data + headsz() + m_apak_marker_length, m_data_length);
//qDebug() << 15;
      return true;

//qDebug() << "Record fromRawData"
//         << QDateTime::fromMSecsSinceEpoch(m_date_time)
//         << m_apak_marker
//         << m_data_length
//         << sizeof(m_date_time) +
//            sizeof(m_data_length) +
//            sizeof(m_apak_marker_length) +
//            m_apak_marker.length() +
//            m_record_data.length();

    }

    const QByteArray& data()        const { return m_record_data;       }
    const QString&    marker()      const { return m_apak_marker;       }
    uint              marker_hash()       { return qHash(m_apak_marker);}
    qint64            dateTime()          { return m_date_time;         }
    quint16           dataLength()        { return m_data_length;       }
    quint16           markerLength()      { return m_apak_marker_length;}

//    int length() { return m_record_data.length(); }

    qint64 size() { return  sizeof(m_date_time) +
                            sizeof(m_data_length) +
                            sizeof(m_apak_marker_length) +
                            m_apak_marker.length() +
                            m_record_data.length(); }

  private:
    qint64      m_date_time;
    quint16     m_data_length;
    quint16     m_apak_marker_length;
    QString     m_apak_marker;
    QByteArray  m_record_data;


  };

  class Bunch
  {

  public:
    enum States {
      Undefined,
      Underway,
      Undelivered,
      Delivered,
      Ready
    };

    Bunch(qint64 coarseDateTime = 0, States state = Underway):
      m_header(BunchHeader(coarseDateTime)),
      m_data(QByteArray()),
      m_state(state),
      m_record_count(0)
    {

    }

    const BunchHeader header()  const { return m_header; }
    const QByteArray& data()    const { return m_data;   }
    States      state()               { return m_state;  }

    bool setHeader(const char* data)
    {
      return m_header.fromRawData(data);
    }

    void setState(States state)
    {
      m_state = state;
    }

    void appendRecord(Record* record)
    {
      m_data.append(record->data());
      m_record_count++;
    }

    QByteArray toByteArray()
    {
      QByteArray result;

      result.append(m_header.toByteArray(m_data.length())).append(m_data);

      return result;
    }

    void appendToStream(QDataStream* const stream)
    {
      QByteArray h = m_header.toByteArray(m_data.length());
      qDebug() << "appendToStream" << QString(h.toHex());
      stream->writeRawData(h.data(), int(m_header.length()));
      stream->writeRawData(m_data, int(m_data.length()));
    }

    void makeByteArray(QByteArray& ba)
    {
      ba.append(m_header.toByteArray(m_data.length())).append(m_data);
    }

    quint32 length()
    {
      return quint32(m_header.length() + m_data.length());
    }

    quint16 recordCount()
    {
      return m_record_count;
    }

  private:
    BunchHeader  m_header;
    QByteArray   m_data;
    States       m_state;

    quint16 m_record_count;

  };

  /// общие. работа с устройством

  enum ReplyCode {

    Success         = 0,
    Failure         = 1,
    NotSupportedCmd = 2,
    WrongZoneName   = 3,
    WrongPassword   = 4,
    AlreadyInUse    = 5,
    WrongParams     = 8
  };

  const QMap<ReplyCode, QString> ReplyCodeMap = {{Success,          "Успех"                       },
                                                 {Failure,          "Отказ ЗН-К1"                 },
                                                 {NotSupportedCmd,  "Команда не поддерживается"   },
                                                 {WrongZoneName,    "Неверное имя зоны данных"    },
                                                 {WrongPassword,    "Неправильный пароль"         },
                                                 {AlreadyInUse,     "Устройство уже используется" },
                                                 {WrongParams,      "Неверные параметры команды" }};


  /// авторизация
  struct AuthorizeRequest
  {
    AuthorizeRequest(QString zone_name, QString pass, quint16 access_code, quint16 cmd = CMD_CONNECT):
      cmd(cmd),
      zone_name(zone_name),
      pass(pass),
      access_code(access_code)
    {
      length = 4 + zone_name.length() + 4 + pass.length() + 2;
    }

    QByteArray toByteArray()
    {
      QByteArray result;

      QDataStream stream(&result, QIODevice::ReadWrite);
      stream.setByteOrder(QDataStream::LittleEndian); // !
      stream << length
             << cmd
             << static_cast<quint32>(zone_name.length());

      stream.writeRawData(zone_name.toStdString().c_str(), zone_name.length());

      stream << static_cast<quint32>(pass.length());

      stream.writeRawData(pass.toStdString().c_str(), pass.length());

      stream << access_code;

      return result;

    }

    quint32     length;
    quint16     cmd;
    QString     zone_name;
    QString     pass;
    quint16     access_code;

  };

  struct AuthorizeReply
  {
    AuthorizeReply():
      length(0),
      reply_code(0),
      request_code(0),
      result(0),
      additional(QByteArray())
    {    }

    static AuthorizeReply parse(const QByteArray& ba)
    {
      AuthorizeReply result;

      if(ba.size() < 10)
        return result;

      QDataStream stream(ba);
      stream.setByteOrder(QDataStream::LittleEndian); //!

      stream >> result.length
             >> result.reply_code
             >> result.request_code
             >> result.result;

      result.additional = ba.right(result.length - 10); // согласно протокола

      return result;

    }

    quint32     length;
    quint16     reply_code;
    quint16     request_code;
    quint16     result;
    QByteArray  additional;

  };

  /// запись данных
  struct WriteRequest
  {
    WriteRequest(const QByteArray& data):
      cmd(CMD_WRITE),
      data(data)
    {
      length = data.length();
    }

    QByteArray toByteArray()
    {
      QByteArray result;

      QDataStream stream(&result, QIODevice::ReadWrite);
      stream.setByteOrder(QDataStream::LittleEndian); // !
      stream << length << cmd;

      result.append(data);

      return result;

    }

    quint32     length;
    quint16     cmd;
    QByteArray  data;

  };

  struct WriteReply
  {
    WriteReply():
      length(0),
      reply_code(0),
      request_code(0),
      result(0),
      additional(QByteArray())
    {    }

    static WriteReply parse(const QByteArray& ba)
    {
      WriteReply result;

      if(ba.length() < 10)
        return result;

      QDataStream stream(ba);
      stream.setByteOrder(QDataStream::LittleEndian); //!

      stream >> result.length
             >> result.reply_code
             >> result.request_code
             >> result.result;

      result.additional = ba.right(result.length - 10); // согласно протокола

      return result;

    }

    quint32     length;
    quint16     reply_code;
    quint16     request_code;
    quint16     result;
    QByteArray  additional;

  };

  /// чтение данных
#pragma pack(push,1)
  struct ReadRequest
  {
    ReadRequest(quint64 start_byte, quint64 byte_count):
      length(16),
      cmd(CMD_READ),
      start_byte(start_byte),
      byte_count(byte_count)
    {

    }

    QByteArray toByteArray()
    {
      QByteArray result;

      QDataStream stream(&result, QIODevice::ReadWrite);
      stream.setByteOrder(QDataStream::LittleEndian); // !
      stream << length << cmd << start_byte << byte_count;

      return result;

    }

    quint32 length;
    quint16 cmd;
    quint64 start_byte;
    quint64 byte_count;

  };
#pragma pack(pop)

#pragma pack(push,1)
  struct ReadReply
  {
    ReadReply():
      failed(false),
      length(0),
      reply_code(0),
      request_code(0),
      result(0),
      start_byte(0),
      data(QByteArray())
    {    }

    static ReadReply parse(const QByteArray& ba)
    {
      ReadReply result;

      if(ba.length() < 10) {

        result.failed = true;
        return result;
      }

      QDataStream stream(ba);
      stream.setByteOrder(QDataStream::LittleEndian); //!

      stream >> result.length           // здесь общий размер запрошенных данных, а не одного пакета!
             >> result.reply_code
             >> result.request_code
             >> result.result;

      if(result.result == ReplyCode::Success) {

        if(ba.length() > 17) {

          stream >> result.start_byte;
          result.data = ba.right(ba.size() - 18); // согласно протокола
//          qDebug() << "result: length" << result.length << "data size" << result.data.size();

        }
        else
          result.failed = true;

      }

      return result;

    }

    bool        failed;
    quint32     length;
    quint16     reply_code;
    quint16     request_code;
    quint16     result;
    quint64     start_byte;
    QByteArray  data;

  };
#pragma pack(pop)

  /// запрос размера данных
  struct DataSizeRequest
  {
    DataSizeRequest():
      cmd(CMD_SIZE)
    {
      length = 0;
    }

    QByteArray toByteArray()
    {
      QByteArray result;

      QDataStream stream(&result, QIODevice::ReadWrite);
      stream.setByteOrder(QDataStream::LittleEndian); // !
      stream << length << cmd;

      return result;

    }

    quint32     length;
    quint16     cmd;

  };

  struct DataSizeReply
  {
    DataSizeReply():
      failed(false),
      length(0),
      reply_code(0),
      request_code(0),
      result(0),
      data_size(0),
      data_pointer(0),
      additional(QByteArray())
    {    }

    static DataSizeReply parse(const QByteArray& ba)
    {
      DataSizeReply result;

      if(ba.length() < 10) { // согласно протокола

        result.failed = true;
        return result;
      }

      QDataStream stream(ba);
      stream.setByteOrder(QDataStream::LittleEndian); //!

      stream >> result.length
             >> result.reply_code
             >> result.request_code
             >> result.result;

      if(result.result == ReplyCode::Success) {

         if(ba.length() > 25) {

           stream >> result.data_size
                  >> result.data_pointer;

           result.additional = ba.right(result.length - 26); // согласно протокола

         }
         else
           result.failed = true;

      }

      return result;

    }

    bool        failed;
    quint32     length;
    quint16     reply_code;
    quint16     request_code;
    quint16     result;
    quint64     data_size;
    quint64     data_pointer;
    QByteArray  additional;

  };

}

#endif // ZN_GLOBAL

