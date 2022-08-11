
/**********************************************************************
 *  автор Свиридов С.А. НИИ РПИ
 * *********************************************************************/

#ifndef ZN_K1_READER_H
#define ZN_K1_READER_H

#include <QDataStream>
//#include <QTcpSocket>
#include <QThread>

#include "../../../../../job/Modus_Libs/APAK/global_apak_defs.h"
#include "../../../../../job/svlib/Network/Tcp/Client/1.0/sv_tcp_client.h"
#include "../../../../../job/svlib/SvException/svexception.h"
#include "../../../../../job/svlib/SvCRC/1.1/sv_crc.h"
#include "../../../../../job/svlib/SvAbstractLogger/svabstractlogger.h"

//#include "../../../job/Modus/global/storage/sv_abstract_storage.h"
//#include "../../../job/Modus/global/dbus/sv_dbus.h"

#include "../../recovery_defs.h"


namespace zn1 {

  class ZNReader: public QThread  //QObject
  {

    Q_OBJECT

    struct ZNstate {
      ZNstate(int c = STATE_NO_CONNECTION, int a = STATE_NO_AUTHORITY, int r = STATE_NO_READING):
        c(c), a(a), r(r)
      {}

      quint16 state() { return quint16(c + ((a * c) << 1) + ((r * a * c) << 2)); }

      quint8 c;
      quint8 a;
      quint8 r;

      bool connected()  { return bool(c > 0); }
      bool authorized() { return bool(a > 0); }
      bool reading()    { return bool(r > 0); }

      void reset()      { c = STATE_NO_CONNECTION; a = STATE_NO_AUTHORITY; r = STATE_NO_READING; }

//      quint8 state;

    };

  public:
    ZNReader();
    ~ZNReader() { /*qDebug() << "destruct";*/ }

//    bool configure(const QString& json, const QString& password); //, double zone_size);
    bool configure(const zn1::RecoveryConfig& config, const QString& password); //, double zone_size);

    QString lastError() const { return m_last_error; }

  protected:
    void run() override;

  private:

//    QTcp*         m_socket;
    sv::tcp::Client*    m_socket;
//    zn1::ReaderParams   m_params;
    zn1::RecoveryConfig m_config;

    ZNstate m_zn_state;

    QString m_last_error;

    bool m_is_running;

    Bunch m_current_bunch;

    quint64 m_full_data_size;
    quint64 m_buff_size_bytes;

//    void setState(int doChangeFlags, const QString& writeState = STATE_OK, const QString& authorization = STATE_OK, const QString& connectionState = STATE_OK);
//    void setState(int writeState, int authorization, int connectionState);

  private slots:
    bool authorization(ZNstate& state, QString& error);
    bool read(ZNstate& state, QString& error);
    void conmessage(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

  public slots:
    void stop();

  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
    void total(int size);
    void parted(int size);
    void zonesize(int size);
    void partsize(int size);
//    void finished();

  };
}

#endif // ZN_K1_READER_H
