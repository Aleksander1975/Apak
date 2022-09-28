#ifndef KSON_PACK_H
#define KSON_PACK_H

#include <QMutex>
#include <QMutexLocker>

#include "kson_packet_global.h"

#include "protocol_params.h"
#include "../../../global_apak_defs.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"
#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

     KSON_PACKET_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace apak {

  class SvKsonPacket: public modus::SvAbstractProtocol
  {
    Q_OBJECT

  public:
    SvKsonPacket();
    ~SvKsonPacket();

    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:
    apak::ProtocolParams  m_params;

    // Указатель на сигнал - "данные от сети КСОН". Значение этого сигнала - это весь информационный
    // кадр от КСОН, а не только блок параметрической информации из этого кадра.
    modus::SvSignal*      m_data_signal;

    // Указатель на сигнал - "состояние сети КСОН":
    modus::SvSignal*      m_state_signal;

    // Указатель на сигнал "время от сети КСОН":
    modus::SvSignal*      m_time_signal;

    // "Таймер посылки" - таймер, отсчитывающий период посылки информационных
    // кадров от сисмемы АПАК к сети КСОН:
    QTimer* m_sendTimer;

    // "Таймер подтверждения" - таймер, отсчитывающий предельно допустимое время от
    // посылки нами информационного кадра к сети КСОН, до получения нами
    // пакета подтверждения от сети КСОН.
    QTimer* m_conformTimer;

    // Счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
    uint interactionErrorCounter;

    // В этой переменной, при разборе информационного пакета от КСОН,
    // формируем поле статуса пакета подтверждения:
    uint8_t status;

    // В этой переменной хранится время последнего информационного пакета от АПАК к КСОН:
    qint32 packetTimeTo_KSON;

    // Флаг ошибки пакета подтверждения. Устанавливается, если поле статуса пакета подтверждения
    // содержит значение, говорящее об ошибке в информационном пакете от АПАК к КСОН
    // или имеются любые ошибки в самом пакете подтверждения.
    bool confirmationPackageErrorFlag;

    // Битовый массив размером m_params.data_len*8 битов, в котором формируется "информационный блок"
    //(длиной m_params.data_len байтов) информационного кадра от АПАК к КСОН:
    QBitArray m_bitsInformBlock;

    // Байтовый массив размером m_params.data_len байтов, в котором хранится "информационный блок"
    //(длиной m_params.data_len байтов) информационного кадра от АПАК к КСОН:
    QByteArray m_byteInformBlock;

    // Битовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    // (номер - это индекс в этом массиве),
    // устанавливает актуальны ли её сигналы (false - не актуальны, true - актуальны).
    // Считаем, что все сигналы от АПАК (группы 0 - 9) - ВСЕГДА актуальны.
    QBitArray m_relevance_by_group;

    // Байтовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    // (номер - это номер бита в этом массиве),
    // устанавливает актуальны ли её сигналы: значение бита 0 - не актуальны, 1 - актуальны).
    // Считаем, что все сигналы от АПАК (группы 0 - 9) - ВСЕГДА актуальны.
    QByteArray m_relevanceGroup_Byte;

    // Информационный кадр от АПАК к КСОН:
    QByteArray m_send_data;

    // Словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsInformBlock":
    QMap<uint16_t, modus::SvSignal*> m_signal_by_bitNumberInInformBlock;

    // Словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock", ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsInformBlock":
    QMap<uint16_t, uint8_t> m_bitNumberInSignal_by_bitNumberInInformBlock;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;
    void start() override;

  private slots:
    void parse(modus::BUFF* buffer);
//    void checkup();

  signals:
    // Сигнал, который приказывает интерфейсной части разорвать соединение с сервером:
    void breakConnection(void);

  };
}


#endif // KSON_PACK_H
