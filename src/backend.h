#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>

class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(int crcErrorCount READ crcErrorCount NOTIFY crcErrorCountChanged)
public:
    explicit BackEnd(QObject *parent = nullptr);
    ~BackEnd();

    QStringList ports() const
    {
        return m_ports;
    }

    int crcErrorCount() const
    {
        return m_crcErrorCount;
    }

signals:
    void portsChanged(QStringList ports);
    void frameChanged(QByteArray frame);
    void crcErrorCountChanged(int crcErrorCount);

public slots:
    void onPortSelectionChanged(int index);
    void onUpdatePortsPressed();

private slots:
    void onReadyRead();

private:
    QSerialPort *m_port;
    QStringList m_ports;
    
    QByteArray m_frame;
    int m_crcErrorCount;
};

#endif // BACKEND_H
