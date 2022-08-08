#ifndef RADUGA_IMITATOR_H
#define RADUGA_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "raduga_imitator_global.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"
#include "abstract_signal_collection.h"
#include "data_type_collection.h"

#include "../../../recovery/lib/radga/raduga_defs.h"

#include "../../../../svlib/SvAbstractLogger/svabstractlogger.h"
#include "../../../../svlib/SvException/svexception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

    RADUGA_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}



//const int  RDGA_NAME_LEN = 12;
//const char RDGA_NAME[SYSTEM_NAME_LEN] = {'R','a','d','u','g','a','-','2','0','3','8','6','\0','\0','\0','\0' };


#define GOOD_PARSED   0
#define DO_RESET      1
#define DO_NOT_RESET  2

namespace raduga {


  struct TREATRESULT {

    TREATRESULT();
    TREATRESULT(bool reset, QDateTime dt = QDateTime())
    { do_reset = reset; parse_time = dt; }

    bool do_reset;
    QDateTime parse_time;

  };

  class SvRaduga;

}

class raduga::SvRaduga: public modus::SvAbstractProtocol
{
  Q_OBJECT

public:
  SvRaduga();

  bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;
  bool bindSignal (modus::SvSignal* signal, modus::SignalBinding binding) override;

public slots:
  void start() override;

  void signalUpdated(modus::SvSignal* signal) override
  {
    Q_UNUSED(signal);
  }

  void signalChanged(modus::SvSignal* signal) override
  {
    Q_UNUSED(signal);
  }


private:
  QList<modus::SvSignal*>    p_input_signals;
  QList<modus::SvSignal*>    p_output_signals;

  raduga::ProtocolParams     m_params;

  raduga::Header             m_header;
  size_t                     m_hsz = sizeof(raduga::Header);

  /* intput signals */
  raduga::DataTypeCollection type1_input_signals;
  raduga::DataTypeCollection type2_input_signals;
  raduga::DataTypeCollection type3_input_signals;
  raduga::DataTypeCollection type5_input_signals;
  raduga::DataTypeCollection type9_input_signals;
  raduga::DataTypeCollection type53_input_signals;

  QMap<quint16, raduga::SvAbstractSignalCollection*> input_signal_collections;

  /* output signals */
  raduga::DataTypeCollection type1_output_signals;
  raduga::DataTypeCollection type2_output_signals;
  raduga::DataTypeCollection type3_output_signals;
  raduga::DataTypeCollection type5_output_signals;
  raduga::DataTypeCollection type9_output_signals;
  raduga::DataTypeCollection type53_output_signals;

  raduga::DataTypeCollection type11_output_signals;
  raduga::DataTypeCollection type12_output_signals;
  raduga::DataTypeCollection type13_output_signals;
  raduga::DataTypeCollection type14_output_signals;
  raduga::DataTypeCollection type15_output_signals;
  raduga::DataTypeCollection type16_output_signals;

  raduga::DataTypeCollection type17_output_signals;
  raduga::DataTypeCollection type18_output_signals;
  raduga::DataTypeCollection type19_output_signals;
  raduga::DataTypeCollection type20_output_signals;

  QMap<quint16, raduga::SvAbstractSignalCollection*> output_signal_collections;

private slots:
  void putout();

};

#endif // RADUGA_IMITATOR_H
