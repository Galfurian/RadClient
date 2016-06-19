#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QtCore>
#include <QtNetwork>
#include <QString>
#include <QTcpSocket>
#include <QTextEdit>
#include <QScrollBar>
#include <QDebug>
#include <QWidget>
#include <QFontDatabase>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QStringListModel>
#include <QStringList>
#include <QListView>

#include <QListWidget>
#include <QLineEdit>
#include <QSplitter>

#include "historylineedit.h"
#include "minimap.h"

enum RequestTypes
{
    kNoRequest,
    kReceiveFormat,
    kRoomMap
};

class SocketClient: public QWidget
{
        Q_OBJECT
    public:
        SocketClient(QWidget * parent = 0);

        virtual ~SocketClient();

        bool connectToHost(const QString & hostName, quint16 port);
        void disconnectFromHost();
        bool isConnected();

    public slots:
        bool sendMessage(const QString & message);
        void lineReturnPressed(QString message);

        void readMessage();
        void parseText(const QByteArray & data, int & index);
        void parseIAC(const QByteArray & data, int & index);
        void connectionError(QAbstractSocket::SocketError error);

        void handleMSDP();
        void handleMCCP1();
        void handleMCCP2();

    private: // FUNCTIONS
        QString stripCR(const QString & message);
        bool isOperation(const uchar c);
        bool isCommand(const uchar c);
        QString parsedText;

    private: // COMMON
        // Connection
        QTcpSocket * tcpSocket;
        // Connection buffer.
        QByteArray inputBuffer;
        // Connection status.
        bool connnected;

        // Main Layout.
        QVBoxLayout * mainBox;
        // Central Layout
        QHBoxLayout * centralBox;
        // Side Layout, for the user interface.
        QVBoxLayout * interfaceBox;

        // Current Request from the Server.
        RequestTypes request;

    public: // INPUT LINE
        HistoryLineEdit * inputLine;
        QFont inputLineFont;

    private: // TERMINAL
        QTextEdit * terminal;
        QFont terminalFont;
        QFont::Weight terminalFontWeight;
        QColor terminalTextColor;
        QPalette terminalPalette;

    private: // MINIMAP
        Minimap * minimap;

    private: // TELNET LIST
        QListWidget * commandList;
        void addCommandToList(const QString & command);

    private: // FORMAT PARSING
        void applyFormat(const QString & format);

};


namespace Common // RFC854
{
// Commands
const uchar CEOF  = 236;
const uchar SUSP  = 237;
const uchar ABORT = 238;

const uchar SE    = 240; // SubnegotiationEnd
const uchar NOP   = 241; // NoOperation
const uchar DM    = 242; // DataMark
const uchar BRK   = 243; // Break
const uchar IP    = 244; // InterruptProcess
const uchar AO    = 245; // AbortOutput
const uchar AYT   = 246; // AreYouThere
const uchar EC    = 247; // EraseCharacter
const uchar EL    = 248; // EraseLine
const uchar GA    = 249; // GoAhead
const uchar SB    = 250; // SubnegotiationBegin

// Negotiation
const uchar WILL  = 251;
const uchar WONT  = 252;
const uchar DO    = 253;
const uchar DONT  = 254;

// Escape
const uchar IAC   = 255;

// Types
const uchar IS     = 0;
const uchar SEND   = 1;
const uchar NAWS   = 31; // RFC1073, implemented

// Types (MUD-specific)
const uchar MSDP   = 69;
const uchar MXP    = 91;
const uchar MCCP1  = 85;
const uchar MCCP2  = 86;
const uchar MSP    = 90;

const uchar DRAW_MAP = 91;
const uchar CLR_MAP  = 92;

// Formatting Command
const uchar FORMAT = 100;
};

#endif // SOCKETCLIENT_H
