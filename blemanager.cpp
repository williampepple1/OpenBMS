#include "blemanager.h"
#include <QDebug>
#include <thread>

BleManager::BleManager(QObject *parent)
    : QObject(parent)
    , m_data(new BmsData(this))
    , m_pollTimer(new QTimer(this))
{
    m_pollTimer->setInterval(2000);
    connect(m_pollTimer, &QTimer::timeout, this, &BleManager::requestCellInfo);

    // Internal signals bridge SimpleBLE background threads to Qt main thread
    connect(this, &BleManager::deviceFoundInternal,
            this, &BleManager::onDeviceFound, Qt::QueuedConnection);
    connect(this, &BleManager::scanStoppedInternal,
            this, &BleManager::onScanStopped, Qt::QueuedConnection);
    connect(this, &BleManager::bleDataReceived,
            this, &BleManager::onBleDataReceived, Qt::QueuedConnection);
    connect(this, &BleManager::connectionResult,
            this, &BleManager::onConnectionResult, Qt::QueuedConnection);
    connect(this, &BleManager::peerDisconnected,
            this, &BleManager::onPeerDisconnected, Qt::QueuedConnection);
}

BleManager::~BleManager()
{
    m_pollTimer->stop();
    if (m_connectedPeripheral.has_value()) {
        try {
            if (!m_notifyServiceUuid.empty())
                m_connectedPeripheral->unsubscribe(m_notifyServiceUuid, m_notifyCharUuid);
            m_connectedPeripheral->disconnect();
        } catch (...) {}
    }
}

void BleManager::setStatusText(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}

void BleManager::setScanning(bool v)
{
    if (m_scanning != v) {
        m_scanning = v;
        emit scanningChanged();
    }
}

void BleManager::setConnected(bool v)
{
    if (m_connected != v) {
        m_connected = v;
        m_data->setConnected(v);
        emit connectedChanged();
    }
}

// ── Scanning ──

void BleManager::startScan()
{
    if (m_scanning)
        return;

    if (!SimpleBLE::Adapter::bluetooth_enabled()) {
        setStatusText("Bluetooth is not enabled on this system");
        return;
    }

    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty()) {
        setStatusText("No Bluetooth adapter found");
        return;
    }

    m_adapter = adapters[0];
    m_peripherals.clear();
    m_devices.clear();
    emit devicesChanged();

    m_adapter->set_callback_on_scan_found([this](SimpleBLE::Peripheral p) {
        QString name = QString::fromStdString(
            p.identifier().empty() ? "Unknown Device" : p.identifier());
        QString address = QString::fromStdString(p.address());
        int rssi = p.rssi();

        // Store peripheral object in main thread via queued signal
        {
            std::lock_guard<std::mutex> lock(m_peripheralsMutex);
            for (auto &existing : m_peripherals)
                if (existing.address() == p.address())
                    return;
            m_peripherals.push_back(std::move(p));
        }

        emit deviceFoundInternal(name, address, rssi);
    });

    m_adapter->set_callback_on_scan_stop([this]() {
        emit scanStoppedInternal();
    });

    m_adapter->scan_start();
    setScanning(true);
    setStatusText("Scanning for BLE devices...");

    QTimer::singleShot(15000, this, [this]() {
        if (m_scanning && m_adapter.has_value()) {
            try { m_adapter->scan_stop(); } catch (...) {}
        }
    });
}

void BleManager::stopScan()
{
    if (m_adapter.has_value() && m_scanning) {
        try { m_adapter->scan_stop(); } catch (...) {}
        setScanning(false);
        setStatusText("Scan stopped");
    }
}

void BleManager::onDeviceFound(const QString &name, const QString &address, int rssi)
{
    QVariantMap device;
    device["name"] = name;
    device["address"] = address;
    device["rssi"] = rssi;
    m_devices.append(device);
    emit devicesChanged();

    setStatusText(QString("Found %1 device(s)").arg(m_devices.size()));
}

void BleManager::onScanStopped()
{
    setScanning(false);
    setStatusText(QString("Scan complete — %1 device(s) found").arg(m_devices.size()));
}

// ── Connection ──

void BleManager::connectToDevice(int index)
{
    SimpleBLE::Peripheral peripheral;
    {
        std::lock_guard<std::mutex> lock(m_peripheralsMutex);
        if (index < 0 || index >= static_cast<int>(m_peripherals.size()))
            return;
        peripheral = m_peripherals[index];
    }

    stopScan();
    QString name = QString::fromStdString(
        peripheral.identifier().empty() ? "JK BMS" : peripheral.identifier());
    m_data->setDeviceName(name);
    setStatusText("Connecting to " + name + "...");

    // Connect in background thread — SimpleBLE::connect() blocks
    std::thread([this, peripheral]() mutable {
        try {
            peripheral.connect();
            emit connectionResult(true, QString());

            peripheral.set_callback_on_disconnected([this]() {
                emit peerDisconnected();
            });

        } catch (const std::exception &e) {
            emit connectionResult(false, QString::fromStdString(e.what()));
        } catch (...) {
            emit connectionResult(false, "Unknown connection error");
        }
    }).detach();

    m_connectedPeripheral = peripheral;
}

void BleManager::onConnectionResult(bool success, const QString &error)
{
    if (success) {
        setupBleConnection();
    } else {
        setStatusText("Connection failed: " + error);
        m_connectedPeripheral.reset();
    }
}

void BleManager::disconnectDevice()
{
    m_pollTimer->stop();
    if (m_connectedPeripheral.has_value()) {
        try {
            if (!m_notifyServiceUuid.empty())
                m_connectedPeripheral->unsubscribe(m_notifyServiceUuid, m_notifyCharUuid);
            m_connectedPeripheral->disconnect();
        } catch (...) {}
        m_connectedPeripheral.reset();
    }
    m_notifyServiceUuid.clear();
    m_notifyCharUuid.clear();
    m_writeServiceUuid.clear();
    m_writeCharUuid.clear();
    setConnected(false);
    setStatusText("Disconnected");
}

void BleManager::onPeerDisconnected()
{
    m_pollTimer->stop();
    m_connectedPeripheral.reset();
    setConnected(false);
    setStatusText("Device disconnected");
}

// ── BLE Service Discovery ──

void BleManager::setupBleConnection()
{
    if (!m_connectedPeripheral.has_value())
        return;

    auto &p = m_connectedPeripheral.value();
    bool foundNotify = false;
    bool foundWrite = false;

    try {
        for (auto &service : p.services()) {
            std::string sUuid = service.uuid();
            // Match FFE0 service (case-insensitive)
            if (sUuid.find("ffe0") == std::string::npos &&
                sUuid.find("FFE0") == std::string::npos)
                continue;

            for (auto &ch : service.characteristics()) {
                std::string cUuid = ch.uuid();
                bool isFFE2 = cUuid.find("ffe2") != std::string::npos ||
                              cUuid.find("FFE2") != std::string::npos;
                bool isFFE1 = cUuid.find("ffe1") != std::string::npos ||
                              cUuid.find("FFE1") != std::string::npos;

                // Prefer FFE2 for writing (old BLE module)
                if (isFFE2 && (ch.can_write_command() || ch.can_write_request())) {
                    m_writeServiceUuid = sUuid;
                    m_writeCharUuid = cUuid;
                    m_useWriteCommand = ch.can_write_command();
                    foundWrite = true;
                }

                if (isFFE1) {
                    if (ch.can_notify() && !foundNotify) {
                        m_notifyServiceUuid = sUuid;
                        m_notifyCharUuid = cUuid;
                        foundNotify = true;
                    }
                    if (!foundWrite && (ch.can_write_command() || ch.can_write_request())) {
                        m_writeServiceUuid = sUuid;
                        m_writeCharUuid = cUuid;
                        m_useWriteCommand = ch.can_write_command();
                        foundWrite = true;
                    }
                }
            }
        }
    } catch (const std::exception &e) {
        setStatusText("Service discovery error: " + QString::fromStdString(e.what()));
        return;
    }

    if (!foundNotify) {
        setStatusText("Notify characteristic not found — not a JK BMS?");
        return;
    }
    if (!foundWrite) {
        setStatusText("Write characteristic not found — not a JK BMS?");
        return;
    }

    // Subscribe to notifications
    try {
        p.notify(m_notifyServiceUuid, m_notifyCharUuid,
                 [this](SimpleBLE::ByteArray bytes) {
                     QByteArray qba(reinterpret_cast<const char*>(bytes.data()),
                                    static_cast<int>(bytes.size()));
                     emit bleDataReceived(qba);
                 });
    } catch (const std::exception &e) {
        setStatusText("Notification subscribe error: " + QString::fromStdString(e.what()));
        return;
    }

    setConnected(true);
    setStatusText("Connected — requesting data...");

    m_frameBuffer.clear();
    requestCellInfo();
    m_pollTimer->start();
}

// ── Protocol: Send Requests ──

void BleManager::requestCellInfo()
{
    if (!m_connected || !m_connectedPeripheral.has_value())
        return;
    sendRequest(CMD_CELL_INFO);
}

void BleManager::sendRequest(uint8_t command)
{
    if (!m_connectedPeripheral.has_value() || m_writeServiceUuid.empty())
        return;

    // 20-byte JK BMS BLE request frame
    uint8_t frame[20] = {};
    frame[0] = 0xAA;
    frame[1] = 0x55;
    frame[2] = 0x90;
    frame[3] = 0xEB;
    frame[4] = command;
    frame[5] = 0x00;

    uint8_t checksum = 0;
    for (int i = 0; i < 19; i++)
        checksum += frame[i];
    frame[19] = checksum;

    SimpleBLE::ByteArray payload(reinterpret_cast<const char*>(frame), 20);

    try {
        if (m_useWriteCommand)
            m_connectedPeripheral->write_command(m_writeServiceUuid, m_writeCharUuid, payload);
        else
            m_connectedPeripheral->write_request(m_writeServiceUuid, m_writeCharUuid, payload);
    } catch (const std::exception &e) {
        qWarning() << "BLE write failed:" << e.what();
    }
}

// ── Protocol: Frame Assembly & Parsing ──

void BleManager::onBleDataReceived(const QByteArray &value)
{
    if (m_frameBuffer.size() > MAX_FRAME_SIZE) {
        m_frameBuffer.clear();
    }

    // Flush buffer when a new frame header arrives (same as esphome-jk-bms)
    if (value.size() >= 4 &&
        static_cast<uint8_t>(value[0]) == 0x55 &&
        static_cast<uint8_t>(value[1]) == 0xAA &&
        static_cast<uint8_t>(value[2]) == 0xEB &&
        static_cast<uint8_t>(value[3]) == 0x90) {
        m_frameBuffer.clear();
    }

    m_frameBuffer.append(value);

    if (m_frameBuffer.size() >= MIN_FRAME_SIZE) {
        // CRC validation: sum of bytes 0..298 must equal byte 299
        uint8_t computed = 0;
        for (int i = 0; i < 299; i++)
            computed += static_cast<uint8_t>(m_frameBuffer[i]);

        uint8_t remote = static_cast<uint8_t>(m_frameBuffer[299]);
        if (computed != remote) {
            qWarning() << "CRC mismatch:" << Qt::hex << computed << "!=" << remote;
            m_frameBuffer.clear();
            return;
        }

        QByteArray frame = m_frameBuffer.left(MIN_FRAME_SIZE);
        m_frameBuffer.clear();
        processFrame(frame);
    }
}

void BleManager::processFrame(const QByteArray &frame)
{
    if (frame.size() < MIN_FRAME_SIZE)
        return;

    uint8_t frameType = static_cast<uint8_t>(frame[4]);
    switch (frameType) {
    case 0x02:
        parseCellInfoFrame(frame);
        break;
    case 0x01:
        qDebug() << "Settings frame received (not yet parsed)";
        break;
    case 0x03:
        qDebug() << "Device info frame received (not yet parsed)";
        break;
    default:
        qWarning() << "Unknown frame type:" << Qt::hex << frameType;
    }
}

void BleManager::parseCellInfoFrame(const QByteArray &data)
{
    if (data.size() < MIN_FRAME_SIZE)
        return;

    // Auto-detect JK02_24S vs JK02_32S: check if cells 25-32 have valid voltages
    bool is32s = false;
    for (int i = 24; i < 32; i++) {
        uint16_t mv = getUint16LE(data, 6 + i * 2);
        if (mv > 100 && mv < 5000) {
            is32s = true;
            break;
        }
    }

    int maxCells = is32s ? 32 : 24;
    int cellOffset = is32s ? 16 : 0;       // 8 extra cells × 2 bytes
    int dataOffset = cellOffset * 2;        // doubled through resistances region

    // ── Cell voltages: 2 bytes LE each, unit = mV ──
    QVector<double> voltages;
    for (int i = 0; i < maxCells; i++) {
        uint16_t mv = getUint16LE(data, 6 + i * 2);
        if (mv > 100 && mv < 5000)
            voltages.append(mv * 0.001);
    }
    m_data->setCellVoltages(voltages);

    // ── Temperatures ──
    if (is32s) {
        // 32S: MOSFET temp at 112+offset, sensors at 130+offset, 132+offset
        m_data->setTemps(
            getInt16LE(data, 112 + dataOffset) * 0.1,
            getInt16LE(data, 130 + dataOffset) * 0.1,
            getInt16LE(data, 132 + dataOffset) * 0.1);
    } else {
        // 24S: sensors at 130, 132; MOSFET at 134
        m_data->setTemps(
            getInt16LE(data, 134 + dataOffset) * 0.1,
            getInt16LE(data, 130 + dataOffset) * 0.1,
            getInt16LE(data, 132 + dataOffset) * 0.1);
    }

    // ── Total voltage: uint32 LE at 118, unit = mV ──
    m_data->setTotalVoltage(getUint32LE(data, 118 + dataOffset) * 0.001);

    // ── Current: int32 LE at 126, unit = mA (positive = charging) ──
    m_data->setCurrent(getInt32LE(data, 126 + dataOffset) * 0.001);

    // ── SOC: uint8 at 141 ──
    m_data->setSoc(static_cast<uint8_t>(data[141 + dataOffset]));

    // ── Capacity: uint32 LE at 142 (remaining) and 146 (nominal), unit = mAh ──
    m_data->setCapacity(
        getUint32LE(data, 142 + dataOffset) * 0.001,
        getUint32LE(data, 146 + dataOffset) * 0.001);

    // ── Cycle count: uint32 LE at 150 ──
    m_data->setCycleCount(static_cast<int>(getUint32LE(data, 150 + dataOffset)));

    // ── MOSFET status ──
    bool charging    = static_cast<uint8_t>(data[166 + dataOffset]) != 0;
    bool discharging = static_cast<uint8_t>(data[167 + dataOffset]) != 0;
    bool balancing   = static_cast<uint8_t>(data[140 + dataOffset]) != 0;
    m_data->setStatus(charging, discharging, balancing);

    // ── Errors bitmask ──
    uint16_t errors = is32s ? getUint16LE(data, 134 + dataOffset)
                            : getUint16LE(data, 136 + dataOffset);
    m_data->setErrorsBitmask(errors);

    setStatusText("Receiving data...");
}

// ── Little-endian helpers (JK BMS byte order) ──

uint16_t BleManager::getUint16LE(const QByteArray &d, int offset)
{
    if (offset + 1 >= d.size()) return 0;
    return static_cast<uint16_t>(
        (static_cast<uint8_t>(d[offset + 1]) << 8) |
         static_cast<uint8_t>(d[offset]));
}

uint32_t BleManager::getUint32LE(const QByteArray &d, int offset)
{
    if (offset + 3 >= d.size()) return 0;
    return (static_cast<uint32_t>(getUint16LE(d, offset + 2)) << 16) |
            static_cast<uint32_t>(getUint16LE(d, offset));
}

int16_t BleManager::getInt16LE(const QByteArray &d, int offset)
{
    return static_cast<int16_t>(getUint16LE(d, offset));
}

int32_t BleManager::getInt32LE(const QByteArray &d, int offset)
{
    return static_cast<int32_t>(getUint32LE(d, offset));
}
