#include "backend.h"

#include <QTimer>
#include <QContiguousCache>


BackEnd::BackEnd(QObject *parent) : QObject(parent),
    m_port(new QSerialPort(this)),
    m_crcErrorCount(0)
{
    connect(m_port, &QSerialPort::readyRead, this, &BackEnd::onReadyRead);

    QTimer::singleShot(0, this, &BackEnd::onUpdatePortsPressed);
}

BackEnd::~BackEnd()
{
    if (m_port->isOpen()) m_port->close();
}

void BackEnd::onPortSelectionChanged(int index)
{
    if (m_port->isOpen()) m_port->close();

    m_port->setPortName(m_ports.at(index));
    m_port->setBaudRate(115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    m_port->open(QIODevice::ReadOnly);
}

void BackEnd::onUpdatePortsPressed()
{
    m_ports.clear();

    for (auto portInfo : QSerialPortInfo::availablePorts())
    {
        m_ports << portInfo.systemLocation();
    }

    emit portsChanged(m_ports);
}

/**********************************************************************
 * Description: crc calculation, adds a 8 bit unsigned to 16 bit crc
 * *******************************************************************/
uint16_t crc16(uint16_t crc, uint8_t value)
{
    const uint16_t crc_polynome = 0x1021;

    crc = crc^static_cast<uint16_t>(value) << 8;

    for (int i=0; i<8; i++)
    {
        if (crc & 0x8000)
        {
            crc = (crc << 1) ^ crc_polynome;
        }
        else
        {
            crc = (crc << 1);
        }
    }

    return crc;
}

void BackEnd::onReadyRead()
{
    static QByteArray data;

    data.append(m_port->readAll());

    int start = data.lastIndexOf(QByteArrayLiteral("\xA8"));
    int frameSize = 0;

    // Find startsequence aka 0xA801 bzw 0xA881 or return.
    while (true)
    {
        if (start == -1) return;


        if (data.size() < start+2)
        {
            if (start == 0) return;

            start = data.lastIndexOf(QByteArrayLiteral("\xA8"), start-1);
            continue;
        }


        if (data.at(start+1) != (char)0x01)
        {
            if (data.at(start +1) != (char)0x81)
            {
                if (start == 0) return;

                start = data.lastIndexOf(QByteArrayLiteral("\xA8"), start-1);
                continue;
            }
        }

        frameSize = static_cast<uint8_t>(data.at(start+2)) * 2 + 5;
        if (data.size() - start < frameSize)
        {
            if (start == 0) return;

            start = data.lastIndexOf(QByteArrayLiteral("\xA8"), start-1);
            continue;
        }

        break;
    }

    QByteArray newFrame = data.mid(start, frameSize);
    data = data.mid(start+frameSize);

    uint16_t crc=0;
    for (int i=0; i<frameSize-2; i++)
    {
        crc = crc16(crc, newFrame.at(i));
    }

    uint16_t newCrc = (newFrame.at(frameSize-2) << 8) | newFrame.at(frameSize-1);

    if ( newCrc != crc)
    {
        m_crcErrorCount++;
        emit crcErrorCountChanged(m_crcErrorCount);

        //return;

    }

    if (m_frame != newFrame)
    {
        m_frame = newFrame;
        emit frameChanged(m_frame);
    }
}
