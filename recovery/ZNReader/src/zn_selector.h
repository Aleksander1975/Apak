#ifndef ZN_SELECTOR_H
#define ZN_SELECTOR_H

#include <QDialog>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDir>

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "ui_select_zn_dialog.h"
#include "zn_reader_defs.h"

#include "../../../../../c++/Modus/global/global_defs.h"

#include "../../../../../c++/svlib/SvException/svexception.h"

namespace Ui {
  class SelectZNDialog;
}

struct ZnRecord {

  ZnRecord() { }

  ZnRecord(const QString& zone, const QString& host, quint16 port):
    zone(zone), host(host), port(port)
  {  }

  QString   zone;
  QString   host;
  quint16   port;

};

class ZNSelector : public QDialog
{
    Q_OBJECT

  public:
    enum Result { Accepted = QDialog::Accepted, Rejected = QDialog::Rejected, Error = -1 };
    enum ShowMode:bool { smNew = true, smEdit = false };

    explicit ZNSelector(QWidget *parent, const QString& json);
    ~ZNSelector();

    ZnRecord* selectedRecord() { return &m_selected_record; }

    QString lastError() { return m_last_error; }

    int showMode;

  public slots:
    void accept() Q_DECL_OVERRIDE;

private:
    Ui::SelectZNDialog *ui;

    ZnRecord    m_selected_record;
    QJsonObject m_configuration;
    QString     m_last_error = "";

    QMap<int, ZnRecord> m_records;
    QList<QTableWidgetItem*> m_widget_items;

    bool loadJson(const QString& jsonPath);

};

#endif // ZN_SELECTOR_H
