
/**********************************************************************
 *  автор Свиридов С.А. НИИ РПИ
 * *********************************************************************/

#ifndef COARSE_DATA_PICKER
#define COARSE_DATA_PICKER

#include <QDataStream>
#include <QThread>

#include "../../../../../job/APAK/global_apak_defs.h"

#include "../../../../../job/svlib/SvException/svexception.h"
#include "../../../../../job/svlib/SvCRC/1.1/sv_crc.h"
#include "../../../../../job/svlib/SvAbstractLogger/svabstractlogger.h"

#include "../../recovery_defs.h"

//#include "../../zn_global.h"

#include "filter_editor.h"


namespace zn1 {

  class CoarseDataPicker: public QThread
  {

    Q_OBJECT

  public:
    CoarseDataPicker();

    bool    configure(zn1::RecoveryConfig& config, const QList<zn1::CoarseFilter> coarseFilters);  //const QString& json);
    QString lastError() const { return m_last_error; }

  private:

    zn1::RecoveryConfig   m_config;
    QList<CoarseFilter>   m_coarse_filters;
    QString               m_last_error;
    bool                  m_is_running;
    quint64               m_full_data_size;
    Bunch                 m_current_bunch;
    double                m_total_read_size;

//    void setState(int doChangeFlags, const QString& writeState = STATE_OK, const QString& authorization = STATE_OK, const QString& connectionState = STATE_OK);
//    void setState(int writeState, int authorization, int connectionState);

  protected:
    void run() override;

  public slots:
    void stop();

  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
    void read_progress(int position);
    void find_progress(int count);

    void current_position(qint64 position);

  };
}

#endif // COARSE_DATA_PICKER
