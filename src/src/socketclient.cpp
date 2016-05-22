#include "../inc/socketclient.h"

SocketClient::SocketClient(QWidget * parent):
    QWidget(parent),
    connnected(false),
    request(kNoRequest)
{
    tcpSocket = new QTcpSocket();
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)), Qt::DirectConnection);

    terminalFont = QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    terminalPalette = QPalette(QPalette::Base, QColor(10, 10, 10));
    terminalFontWeight = QFont::Normal;
    terminalTextColor = Qt::white;

    mainBox = new QVBoxLayout(this);

    centralBox = new QHBoxLayout();

    interfaceBox = new QVBoxLayout();
    interfaceBox->setAlignment(Qt::AlignTop);

    inputLine = new HistoryLineEdit(this);
    inputLine->setFont(terminalFont);
    inputLine->installEventFilter(this);
    inputLine->connect(inputLine,SIGNAL(lineExecuted(QString)), this, SLOT(lineReturnPressed(QString)));

    terminal = new QTextEdit();
    terminal->setObjectName(QStringLiteral("terminal"));
    terminal->setPalette(terminalPalette);
    terminal->setTextColor(Qt::white);
    terminal->setFont(terminalFont);
    terminal->setFontPointSize(12);
    terminal->setReadOnly(true);

    minimap = new Minimap();
    minimap->setMinimumSize(400, 400);
    minimap->setMaximumSize(400, 400);

    commandList = new QListWidget();
    commandList->setMaximumSize(400, 400);

    mainBox->addLayout(centralBox);
    mainBox->addWidget(inputLine);

    centralBox->addWidget(terminal);
    centralBox->addLayout(interfaceBox);

    interfaceBox->addWidget(minimap);
    interfaceBox->addWidget(commandList);
}

SocketClient::~SocketClient()
{
    delete(tcpSocket);
}

bool SocketClient::connectToHost(const QString & hostName, quint16 port)
{
    if(!tcpSocket)
    {
        return false;
    }
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        return false;
    }
    tcpSocket->connectToHost(hostName, port);
    connnected = true;
    return tcpSocket->waitForConnected();
}

void SocketClient::disconnectFromHost()
{
    if(tcpSocket)
    {
        tcpSocket->disconnectFromHost();
        connnected = false;
    }
}

bool SocketClient::isConnected()
{
    if(tcpSocket)
    {
        return (tcpSocket->state() == QAbstractSocket::ConnectedState);
    }
    return false;
}

bool SocketClient::sendMessage(const QString & message)
{
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        return false;
    }
    QByteArray messageArray = message.toUtf8()+'\n';
    tcpSocket->write(messageArray, messageArray.size());
    return tcpSocket->waitForBytesWritten();
}

void SocketClient::lineReturnPressed(QString message)
{
    // Check if the selected terminal is connected.
    if(!isConnected())
    {
        return;
    }
    // Check if the input line is empty.
    if (message.isEmpty())
    {
        return;
    }
    // Sent the message.
    this->sendMessage(message);
    // Clear the input line.
    this->inputLine->clear();
}

void SocketClient::readMessage()
{
    if(!connnected)
    {
        return;
    }
    buffer.clear();
    while (tcpSocket->bytesAvailable() > 0)
    {
        buffer.append(tcpSocket->readAll());
    }

    int currentIndex = 0;
    while (currentIndex < buffer.size())
    {
        const uchar currentChar = uchar(buffer[currentIndex]);
        if (currentChar == Common::DM)
        {
            this->addCommandToList("DM");
            currentIndex += 1;
        }
        else if (currentChar == Common::IAC)
        {
            this->addCommandToList("IAC");
            currentIndex += parseIAC(buffer.mid(currentIndex));
        }
        else
        {
            if(request == kRoomMap)
            {
                currentIndex += parseText(buffer.mid(currentIndex));
                if (parsedText.isEmpty())
                {
                    continue;
                }
                minimap->appendMap(parsedText);
            }
            else if(request == kReceiveFormat)
            {
                currentIndex += parseText(buffer.mid(currentIndex));
                if (!parsedText.isEmpty())
                {
                    this->applyFormat(parsedText);
                }
            }
            else
            {
                currentIndex += parseText(buffer.mid(currentIndex));
                if (!parsedText.isEmpty())
                {
                    terminal->insertPlainText(stripCR(parsedText));
                    terminal->verticalScrollBar()->setValue(terminal->verticalScrollBar()->maximum());
                }
            }
        }
    }
}

int SocketClient::parseText(const QByteArray & data)
{
    // Total consumed characters.
    int consumed = 0;
    // Length of the string from the start to the first occurrence of '\0'.
    int length = data.indexOf('\0');
    // If there is no '\0' inside the string, use the entire length of the string instead.
    if (length == -1)
    {
        length = data.size();
        consumed = length;
    }
    else
    {
        // Add 1 in order to remove the final '\0'.
        consumed = length + 1;
    }
    // First clear the parsed text container.
    parsedText.clear();
    // Get the string of the desired length.
    parsedText = QString::fromLocal8Bit(data.constData(), length);
    // Return the number of consumed characters.
    return consumed;
}

int SocketClient::parseIAC(const QByteArray & data)
{
    if (data.isEmpty())
    {
        return 0;
    }

    if (data.size() >= 2 && isCommand(data[1]))
    {
        return 2;
    }

    if (data.size() >= 3 && isOperation(data[1]))
    {
        const uchar operation = data[1];
        const uchar option    = data[2];

        // If WONT Logout, disconnect.
        if (operation == Common::WONT)
        {
            this->addCommandToList("WONT");
            this->disconnectFromHost();
        }

        if (operation == Common::DO)
        {
            this->addCommandToList("DO");
            if(option == Common::CLR_MAP)
            {
                this->addCommandToList("CLR_MAP");
                minimap->clearMap();
                return 4;
            }
            else if(option == Common::DRAW_MAP)
            {
                this->addCommandToList("ROOM_MAP");
                request = kRoomMap;
                return 4;
            }
            else if(option == Common::FORMAT)
            {
                this->addCommandToList("FORMAT");
                request = kReceiveFormat;
                return 4;
            }
        }
        else if(operation == Common::DONT)
        {
            this->addCommandToList("DONT");
            if(option == Common::DRAW_MAP)
            {
                this->addCommandToList("ROOM_MAP");
                request = kNoRequest;
                return 4;
            }
            else if(option == Common::FORMAT)
            {
                this->addCommandToList("FORMAT");
                request = kNoRequest;
                return 4;
            }
        }
        return 3;
    }
    return 0;
}

void SocketClient::connectionError(QAbstractSocket::SocketError error)
{
    if(error == QAbstractSocket::AddressInUseError)
    {
        terminal->append("Address is already in use.");
    }
    else if(error == QAbstractSocket::ConnectionRefusedError)
    {
        terminal->append("Connection refused.");
    }
    else if(error == QAbstractSocket::HostNotFoundError)
    {
        terminal->append("Host not found.");
    }
    else if(error == QAbstractSocket::RemoteHostClosedError)
    {
        terminal->append("Remote host closed.");
    }
    else
    {
        terminal->append("Unhandled erro.");
    }
    tcpSocket->abort();
    tcpSocket->close();
    connnected = false;
}

QString SocketClient::stripCR(const QString & message)
{
    QString newMessage(message);
    newMessage.remove('\r');
    // Also remove terminal control codes.
    newMessage.remove(QRegExp("\033\\[[0-9;]*[A-Za-z]"));
    return newMessage;
}

bool SocketClient::isOperation(const uchar c)
{
    return (c == Common::WILL || c == Common::WONT || c == Common::DO ||c == Common::DONT);
}

bool SocketClient::isCommand(const uchar c)
{
    return (c == Common::DM);
}

void SocketClient::addCommandToList(const QString & command)
{
    commandList->addItem(command);
    commandList->verticalScrollBar()->setValue(commandList->verticalScrollBar()->maximum());
}

void SocketClient::applyFormat(const QString & format)
{
    addCommandToList("Format:" + format);
    if(format == "RESET")
    {
        terminal->setFont(terminalFont);
        terminal->setFontWeight(terminalFontWeight);
        terminal->setPalette(terminalPalette);
        terminal->setTextColor(terminalTextColor);
        terminal->setFontItalic(false);
    }
    else if(format == "CURSOR-HOME")
    {

    }
    else if(format == "CLEAR-SCREEN")
    {
        terminal->clear();
    }
    else if(format == "BOLD")
    {
        terminal->setFontWeight( QFont::DemiBold );
    }
    else if(format == "ITALIC")
    {
        terminal->setFontItalic(true);
    }
    else if(format == "BLACK")
    {
        terminal->setTextColor(Qt::black);
    }
    else if(format == "RED")
    {
        terminal->setTextColor(Qt::red);
    }
    else if(format == "GREEN")
    {
        terminal->setTextColor(Qt::green);
    }
    else if(format == "BROWN")
    {
        terminal->setTextColor(QColor(165,42,42));
    }
    else if(format == "YELLOW")
    {
        terminal->setTextColor(Qt::yellow);
    }
    else if(format == "BLUE")
    {
        terminal->setTextColor(Qt::blue);
    }
    else if(format == "MAGENTA")
    {
        terminal->setTextColor(Qt::magenta);
    }
    else if(format == "CYAN")
    {
        terminal->setTextColor(Qt::cyan);
    }
    else if(format == "GRAY")
    {
        terminal->setTextColor(Qt::gray);
    }
    else if(format == "DARK-GRAY")
    {
        terminal->setTextColor(Qt::darkGray);
    }
}
