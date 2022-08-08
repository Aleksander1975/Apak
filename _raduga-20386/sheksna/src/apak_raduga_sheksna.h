#ifndef APAK_RADUGA_SHEKSNA_DATATYPE_COOLECTION_H
#define APAK_RADUGA_SHEKSNA_DATATYPE_COOLECTION_H

#include <QMutex>
#include <QMutexLocker>

#include "apak_raduga_sheksna_global.h"

#include "../../../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"
#include "raduga_defs.h"

#include "data_type_collection.h"

#include "../../../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

    APAK_RADUGA_SHEKSNA_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

const int  SYSTEM_NAME_LEN = 16;

const int  RDGA_NAME_LEN = 12;
const int  APAK_NAME_LEN = 8;

const char RDGA_NAME[SYSTEM_NAME_LEN] = {'R','a','d','u','g','a','-','2','0','3','8','6','\0','\0','\0','\0' };
const char APAK_NAME[SYSTEM_NAME_LEN] = {'A','P','A','K',' ','R','K','I','\0','\0','\0','\0','\0','\0','\0','\0' };


#define GOOD_PARSED   0
#define DO_RESET      1
#define DO_NOT_RESET  2

namespace raduga {

  #pragma pack(push,1)
  struct Header
  {
    char    system_name[SYSTEM_NAME_LEN];
    quint16 abonent_id;
    quint16 activity_id;
    quint16 pack_id;
    char    reserv[10];
  };
  #pragma pack(pop)

  struct PARSERESULT {

    PARSERESULT();
    PARSERESULT(bool reset, QDateTime dt = QDateTime())
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
  ~SvRaduga();

  bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

protected:
  void run() override;

//  void disposeInputSignal (modus::SvSignal* signal) override;
//  void disposeOutputSignal(modus::SvSignal* signal) override;

  void disposeSignal (modus::SvSignal* signal) override;

//  void validateSignals(QDateTime& lastParsedTime) override;

private:
  QList<modus::SvSignal*>    p_input_signals;
  QList<modus::SvSignal*>    p_output_signals;

  raduga::ProtocolParams     m_params;

//  raduga::DATA               m_data;

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

  raduga::PARSERESULT parse();

  void confirmation();

  /* output signals */
  raduga::DataTypeCollection type1_output_signals;
  raduga::DataTypeCollection type2_output_signals;
  raduga::DataTypeCollection type3_output_signals;
  raduga::DataTypeCollection type5_output_signals;
  raduga::DataTypeCollection type9_output_signals;
  raduga::DataTypeCollection type53_output_signals;

  QMap<quint16, raduga::SvAbstractSignalCollection*> output_signal_collections;

  void putout();

  QByteArray m_raduga20386 = QByteArray(&RDGA_NAME[0], RDGA_NAME_LEN);

private slots:
  void output_queue(modus::SvSignal* signal)
  {
    QMutexLocker(&p_out_signal_queue.mutex);
    p_out_signal_queue.queue.enqueue(signal);
  }

};

#endif // APAK_RADUGA_SHEKSNA_DATATYPE_COOLECTION_H
