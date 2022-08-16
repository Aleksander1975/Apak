
/**********************************************************************
 *  автор Свиридов С.А. НИИ РПИ
 * *********************************************************************/

#ifndef ZN_K1_PICKER_H
#define ZN_K1_PICKER_H

#include <QDataStream>
#include <QThread>

#include "../../../../../job/APAK/global_apak_defs.h"

#include "../../../../../job/svlib/SvException/svexception.h"
#include "../../../../../job/svlib/SvCRC/1.1/sv_crc.h"
#include "../../../../../job/svlib/SvAbstractLogger/svabstractlogger.h"

#include "../../recovery_defs.h"

//#include "../../zn_global.h"

#include "task_editor.h"


namespace zn1 {

  class ZNPicker: public QThread
  {

    Q_OBJECT

  public:
    ZNPicker();

    bool configure(const QString& json);
    QString lastError() const { return m_last_error; }

  private:

    zn1::RecoveryConfig   m_config;

    QString m_last_error;

    bool m_is_running;

    quint64 m_full_data_size;
    Bunch m_current_bunch;

    double m_total_read_size;

//    void setState(int doChangeFlags, const QString& writeState = STATE_OK, const QString& authorization = STATE_OK, const QString& connectionState = STATE_OK);
//    void setState(int writeState, int authorization, int connectionState);

  protected:
    void run() override;

  private slots:

  public slots:
//    void start();
    void stop();

  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
//    void read_maximum(qint64 max);
    void read_progress(int position);
    void find_progress(int count);

    void current_position(qint64 position);

  };
}

#endif // ZN_K1_PICKER_H
