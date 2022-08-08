#ifndef ZN_K1_H
#define ZN_K1_H

#include <QDataStream>

#include "zn_writer_global.h"

#include "../../../APAK/global_apak_defs.h"

#include "../../../svlib/Network/Tcp/Client/1.0/sv_tcp_client.h"
#include "../../../svlib/SvException/svexception.h"
#include "../../../svlib/SvCRC/1.1/sv_crc.h"

#include "../../../Modus/global/storage/sv_abstract_storage.h"
#include "../../../Modus/global/dbus/sv_dbus.h"

#include "zn_writer_defs.h"
#include "params.h"

extern "C" {

    ZN_K1SHARED_EXPORT modus::SvAbstractStorage* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace zn1 {

  class ZNWriter: public modus::SvAbstractStorage
  {

    Q_OBJECT

    struct ZNstate {
      ZNstate(int c = 0, int a = 0, int w = 0):
        c(c), a(a), w(w)
      {}

      quint16 state() { return quint16(c + ((a * c) << 1) + ((w * a * c) << 2)); }

      quint8 c;
      quint8 a;
      quint8 w;
    };

  public:
    ZNWriter();
    ~ZNWriter();

    bool configure(modus::StorageConfig* config) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:
//    QFile m_test_file;

    QQueue<zn1::Bunch*>                       bunches;

    sv::tcp::Client*                          m_socket;
    zn1::StorageParams                        m_params;

    modus::SvSignal*                          m_state_signal;

    QMap<modus::SvSignal*, zn1::SignalParams> m_signal_params;
    ZNstate                                   m_zn_state;

  public slots:
    void start() override;
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override
    {
      Q_UNUSED(signal);
    }

    void write();

  };
}

#endif // ZN_K1_H
