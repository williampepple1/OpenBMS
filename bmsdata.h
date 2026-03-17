#pragma once

#include <QObject>
#include <QVariantList>
#include <QQmlEngine>

class BmsData : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(int cellCount READ cellCount NOTIFY cellVoltagesChanged)
    Q_PROPERTY(QVariantList cellVoltages READ cellVoltages NOTIFY cellVoltagesChanged)
    Q_PROPERTY(double totalVoltage READ totalVoltage NOTIFY totalVoltageChanged)
    Q_PROPERTY(double current READ current NOTIFY currentChanged)
    Q_PROPERTY(double power READ power NOTIFY powerChanged)
    Q_PROPERTY(int soc READ soc NOTIFY socChanged)
    Q_PROPERTY(double mosfetTemp READ mosfetTemp NOTIFY tempsChanged)
    Q_PROPERTY(double temp1 READ temp1 NOTIFY tempsChanged)
    Q_PROPERTY(double temp2 READ temp2 NOTIFY tempsChanged)
    Q_PROPERTY(int cycleCount READ cycleCount NOTIFY cycleCountChanged)
    Q_PROPERTY(double capacityRemaining READ capacityRemaining NOTIFY capacityChanged)
    Q_PROPERTY(double nominalCapacity READ nominalCapacity NOTIFY capacityChanged)
    Q_PROPERTY(double minCellVoltage READ minCellVoltage NOTIFY cellVoltagesChanged)
    Q_PROPERTY(double maxCellVoltage READ maxCellVoltage NOTIFY cellVoltagesChanged)
    Q_PROPERTY(double avgCellVoltage READ avgCellVoltage NOTIFY cellVoltagesChanged)
    Q_PROPERTY(double cellDelta READ cellDelta NOTIFY cellVoltagesChanged)
    Q_PROPERTY(bool chargingEnabled READ chargingEnabled NOTIFY statusChanged)
    Q_PROPERTY(bool dischargingEnabled READ dischargingEnabled NOTIFY statusChanged)
    Q_PROPERTY(bool balancingActive READ balancingActive NOTIFY statusChanged)
    Q_PROPERTY(int errorsBitmask READ errorsBitmask NOTIFY errorsChanged)

public:
    explicit BmsData(QObject *parent = nullptr) : QObject(parent) {}

    bool connected() const { return m_connected; }
    QString deviceName() const { return m_deviceName; }
    int cellCount() const { return m_cellVoltages.size(); }
    QVariantList cellVoltages() const { return m_cellVoltages; }
    double totalVoltage() const { return m_totalVoltage; }
    double current() const { return m_current; }
    double power() const { return m_totalVoltage * m_current; }
    int soc() const { return m_soc; }
    double mosfetTemp() const { return m_mosfetTemp; }
    double temp1() const { return m_temp1; }
    double temp2() const { return m_temp2; }
    int cycleCount() const { return m_cycleCount; }
    double capacityRemaining() const { return m_capacityRemaining; }
    double nominalCapacity() const { return m_nominalCapacity; }
    bool chargingEnabled() const { return m_chargingEnabled; }
    bool dischargingEnabled() const { return m_dischargingEnabled; }
    bool balancingActive() const { return m_balancingActive; }
    int errorsBitmask() const { return m_errorsBitmask; }

    double minCellVoltage() const {
        if (m_cellVoltages.isEmpty()) return 0;
        double v = 9999;
        for (const auto &c : m_cellVoltages)
            v = qMin(v, c.toDouble());
        return v;
    }

    double maxCellVoltage() const {
        if (m_cellVoltages.isEmpty()) return 0;
        double v = 0;
        for (const auto &c : m_cellVoltages)
            v = qMax(v, c.toDouble());
        return v;
    }

    double avgCellVoltage() const {
        if (m_cellVoltages.isEmpty()) return 0;
        double sum = 0;
        for (const auto &c : m_cellVoltages)
            sum += c.toDouble();
        return sum / m_cellVoltages.size();
    }

    double cellDelta() const {
        return maxCellVoltage() - minCellVoltage();
    }

    void setConnected(bool v) {
        if (m_connected != v) { m_connected = v; emit connectedChanged(); }
    }

    void setDeviceName(const QString &name) {
        if (m_deviceName != name) { m_deviceName = name; emit deviceNameChanged(); }
    }

    void setCellVoltages(const QVector<double> &voltages) {
        m_cellVoltages.clear();
        for (double v : voltages)
            m_cellVoltages.append(v);
        emit cellVoltagesChanged();
    }

    void setTotalVoltage(double v) {
        if (!qFuzzyCompare(m_totalVoltage, v)) {
            m_totalVoltage = v;
            emit totalVoltageChanged();
            emit powerChanged();
        }
    }

    void setCurrent(double v) {
        if (!qFuzzyCompare(m_current, v)) {
            m_current = v;
            emit currentChanged();
            emit powerChanged();
        }
    }

    void setSoc(int v) {
        if (m_soc != v) { m_soc = v; emit socChanged(); }
    }

    void setTemps(double mosfet, double t1, double t2) {
        m_mosfetTemp = mosfet;
        m_temp1 = t1;
        m_temp2 = t2;
        emit tempsChanged();
    }

    void setCycleCount(int v) {
        if (m_cycleCount != v) { m_cycleCount = v; emit cycleCountChanged(); }
    }

    void setCapacity(double remaining, double nominal) {
        m_capacityRemaining = remaining;
        m_nominalCapacity = nominal;
        emit capacityChanged();
    }

    void setStatus(bool charging, bool discharging, bool balancing) {
        m_chargingEnabled = charging;
        m_dischargingEnabled = discharging;
        m_balancingActive = balancing;
        emit statusChanged();
    }

    void setErrorsBitmask(int v) {
        if (m_errorsBitmask != v) { m_errorsBitmask = v; emit errorsChanged(); }
    }

signals:
    void connectedChanged();
    void deviceNameChanged();
    void cellVoltagesChanged();
    void totalVoltageChanged();
    void currentChanged();
    void powerChanged();
    void socChanged();
    void tempsChanged();
    void cycleCountChanged();
    void capacityChanged();
    void statusChanged();
    void errorsChanged();

private:
    bool m_connected = false;
    QString m_deviceName;
    QVariantList m_cellVoltages;
    double m_totalVoltage = 0;
    double m_current = 0;
    int m_soc = 0;
    double m_mosfetTemp = 0;
    double m_temp1 = 0;
    double m_temp2 = 0;
    int m_cycleCount = 0;
    double m_capacityRemaining = 0;
    double m_nominalCapacity = 0;
    bool m_chargingEnabled = false;
    bool m_dischargingEnabled = false;
    bool m_balancingActive = false;
    int m_errorsBitmask = 0;
};
