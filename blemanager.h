#pragma once

#include <QObject>
#include <QVariantList>
#include <QQmlEngine>
#include <QTimer>

#include <simpleble/SimpleBLE.h>

#include <optional>
#include <vector>
#include <string>
#include <mutex>

#include "bmsdata.h"

class BleManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(BmsData* data READ data CONSTANT)

public:
    explicit BleManager(QObject *parent = nullptr);
    ~BleManager();

    bool scanning() const { return m_scanning; }
    QVariantList devices() const { return m_devices; }
    bool connected() const { return m_connected; }
    QString statusText() const { return m_statusText; }
    BmsData* data() { return m_data; }

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void connectToDevice(int index);
    Q_INVOKABLE void disconnectDevice();

signals:
    void scanningChanged();
    void devicesChanged();
    void connectedChanged();
    void statusTextChanged();

    void deviceFoundInternal(QString name, QString address, int rssi);
    void scanStoppedInternal();
    void bleDataReceived(QByteArray data);
    void connectionResult(bool success, QString error);
    void peerDisconnected();

private slots:
    void onDeviceFound(const QString &name, const QString &address, int rssi);
    void onScanStopped();
    void onBleDataReceived(const QByteArray &data);
    void onConnectionResult(bool success, const QString &error);
    void onPeerDisconnected();

private:
    void setupBleConnection();
    void requestCellInfo();
    void sendRequest(uint8_t command);
    void processFrame(const QByteArray &frame);
    void parseCellInfoFrame(const QByteArray &data);

    void setStatusText(const QString &text);
    void setScanning(bool v);
    void setConnected(bool v);

    static uint16_t getUint16LE(const QByteArray &d, int offset);
    static uint32_t getUint32LE(const QByteArray &d, int offset);
    static int16_t  getInt16LE(const QByteArray &d, int offset);
    static int32_t  getInt32LE(const QByteArray &d, int offset);

    std::optional<SimpleBLE::Adapter> m_adapter;
    std::vector<SimpleBLE::Peripheral> m_peripherals;
    std::mutex m_peripheralsMutex;
    std::optional<SimpleBLE::Peripheral> m_connectedPeripheral;

    QVariantList m_devices;
    bool m_scanning = false;
    bool m_connected = false;
    QString m_statusText = QStringLiteral("Ready to scan");

    QByteArray m_frameBuffer;
    QTimer *m_pollTimer = nullptr;
    BmsData *m_data = nullptr;

    std::string m_notifyServiceUuid;
    std::string m_notifyCharUuid;
    std::string m_writeServiceUuid;
    std::string m_writeCharUuid;
    bool m_useWriteCommand = false;

    static constexpr uint16_t MIN_FRAME_SIZE = 300;
    static constexpr uint16_t MAX_FRAME_SIZE = 400;
    static constexpr uint8_t CMD_CELL_INFO = 0x96;
    static constexpr uint8_t CMD_DEVICE_INFO = 0x97;
};
