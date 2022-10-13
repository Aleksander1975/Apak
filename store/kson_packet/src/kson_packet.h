#ifndef KSON_PACK_H
#define KSON_PACK_H

//#include <QMutex>
//#include <QMutexLocker>

#include "kson_packet_global.h"

#include "protocol_params.h"
#include "signal_params.h"

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

    // "Таймер приёма" - таймер, отсчитывающий предельно допустимое время между информационными
    // кадрами от сети КСОН к системе АПАК:
    QTimer* m_receiveTimer;

    // "Таймер подтверждения" - таймер, отсчитывающий предельно допустимое время от
    // посылки нами информационного кадра к сети КСОН, до получения нами
    // пакета подтверждения от сети КСОН.
    QTimer* m_conformTimer;

    // Время посылки информационного кадра от АПАК к КСОН. Оно необходимо нам, чтобы проверить,
    // что в пакете подтверждения от КСОН указано то же самое время.
    quint32 m_packetTimeTo_KSON;

    // Время из информационного кадра от КСОН. Оно необходимо нам, чтобы использовать его
    // в пакете подтверждения на этот информационный кадр:
    quint32 m_packetTimeFrom_KSON;

    // Счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
    uint m_interactionErrorCounter;

    // В этой переменной, при разборе информационного кадра от КСОН,
    // формируем поле статуса пакета подтверждения:
    uint8_t m_status;

    // ==== СТРУКТУРЫ ДАННЫХ ДЛЯ ФОРМИРОВАНИЯ ИНФОРМАЦИОННОГО КАДРА ОТ АПАК К КСОН ====:
    // 1И. Битовый массив размером m_params.data_len*8 битов, в котором формируется "информационный блок"
    //    (длиной m_params.data_len байтов) информационного кадра от АПАК к КСОН:
    QBitArray m_bitsInformBlock_to_KSON;

    // 2И. Байтовый массив размером m_params.data_len байтов, в котором хранится "информационный блок"
    //    (длиной m_params.data_len байтов) информационного кадра от АПАК к КСОН:
    QByteArray m_byteInformBlock_to_KSON;

    // 3И. Битовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    //    (номер - это индекс в этом массиве),
    //    устанавливает актуальны ли её сигналы (false - не актуальны, true - актуальны).
    //    Считаем, что все сигналы от АПАК (группы 0 - 9) - ВСЕГДА актуальны.
    QBitArray m_relevance_by_group_to_KSON;

    // 4И. Байтовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    //    (номер - это номер бита в этом массиве),
    //    устанавливает актуальны ли её сигналы: значение бита 0 - не актуальны, 1 - актуальны).
    //    Считаем, что все сигналы от АПАК (группы 0 - 9) - ВСЕГДА актуальны.
    QByteArray m_relevanceGroup_Byte_to_KSON;

    // 5И. Информационный кадр от АПАК к КСОН:
    QByteArray m_send_data;

    // 6И. Словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    //    "m_bitsInformBlock" ставит в соответствие сигнал, бит которого должен быть записан
    //    в массив "m_bitsInformBlock":
    QMap<uint16_t, modus::SvSignal*> m_signal_by_bitNumberInInformBlock_to_KSON;

    // 7И. Словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    //    "m_bitsInformBlock", ставит в соответствие номер бита сигнала, который
    //    должен быть записан в массив "m_bitsInformBlock":
    QMap<uint16_t, uint8_t> m_bitNumberInSignal_by_bitNumberInInformBlock_to_KSON;

    // ==== СТРУКТУРЫ ДАННЫХ ДЛЯ ПРОВЕРКИ ИНФОРМАЦИОННОГО КАДРА ОТ КСОН К АПАК ====:
    // 1В. Словарь, который каждому смещению (в байтах) значения сигнала относительно начала информационного
    //     блока, передаваемого из КСОН в АПАК, ставит в соответствие указатель на этот сигнал.
    QMap<uint8_t, modus::SvSignal*> m_signals_by_byte_from_KSON;

    // 2В. Список указателей на сигналы, отсортированный по возрастанию смещений
    //     значений сигналов (в байтах) относительно начала информационного блока,
    //     передаваемого из КСОН в АПАК.
    QList <modus::SvSignal*> m_signals_from_KSON;

    // 3В. Словарь, который каждому сигналу ставит в соответствие структуру SignalParams,
    //     хранящую параметры для этого сигнала:
    QMap<modus::SvSignal*, apak::SignalParams> m_params_by_signal_from_KSON;

    // 4В. Битовый массив, который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    //     определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).
    //     В этом массиве АКТУАЛЬНОСТЬ сигналов группы определяется, исходя из наличия сигналов с
    //     параметром, указывающим на принадлежность к данной группе, в файле сигналов устройства КСОН.
    QBitArray m_relevance_by_group_from_KSON;


  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;
    void start(void) override;

  private slots:
    void sendInformFrame(void);
    void packageFrom_KSON (modus::BUFF* buffer);
    void noConfirmationPackage(void);
    void noReceivePackage (void);
    void sendConfirmationPackage (void);
  };
}


#endif // KSON_PACK_H
