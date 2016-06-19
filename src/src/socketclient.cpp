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
    // Check the connection.
    if(!connnected)
    {
        return;
    }
    // Clear the input buffer.
    inputBuffer.clear();
    // Untile there are bytes available, append the content to the input buffer.
    while (tcpSocket->bytesAvailable() > 0)
    {
        inputBuffer.append(tcpSocket->readAll());
    }
    // Set the index to the begin.
    int index = 0;
    while (index < inputBuffer.size())
    {
        // Retrieve the character at the index.
        const uchar currentChar = uchar(inputBuffer.at(index));
        // If it has to be Interpreted as Command.
        if (currentChar == Common::IAC)
        {
            // Add the operation to the list.
            this->addCommandToList("IAC");
            // Increment the index.
            index++;
            // Parse the telnet command.
            this->parseIAC(inputBuffer, index);
        }
        else
        {
            this->parseText(inputBuffer, index);
            if (!parsedText.isEmpty())
            {
                if(request == kRoomMap)
                {
                    minimap->appendMap(parsedText);
                }
                else if(request == kReceiveFormat)
                {
                    this->applyFormat(parsedText);
                }
                else
                {
                    terminal->insertPlainText(this->stripCR(parsedText));
                    terminal->verticalScrollBar()->setValue(terminal->verticalScrollBar()->maximum());
                }
            }
        }
    }
}

void SocketClient::parseText(const QByteArray & data, int & index)
{
    qInfo() << "Parsing Text...";
    // Total consumed characters.
    int consumed = 0;
    // Length of the string from the start to the first occurrence of '\0'.
    int length = data.indexOf('\0', index);
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
    parsedText = QString::fromLocal8Bit(data.mid(index).constData(), length);
    // Increment the index based on the number of consumed characters.
    index += consumed;
}

void SocketClient::parseIAC(const QByteArray & data, int & index)
{
    qInfo() << "Parsing IAC...";
    if (!data.isEmpty())
    {
        if (this->isOperation(data.at(index)))
        {
            qInfo() << "Found operation...";
            const uchar operation = data.at(index);
            if (operation == Common::DO)
            {
                // Add the operation to the list.
                this->addCommandToList("DO");
                // Increment the index.
                index++;
                // Check if we are out of bound.
                if(index > data.size())
                {
                    return;
                }
                // Retrieve the option.
                const uchar option = data.at(index);
                if(option == Common::CLR_MAP)
                {
                    // Add the operation to the list.
                    this->addCommandToList("CLR_MAP");
                    // Increment the index.
                    index++;
                    // Execute the operation.
                    minimap->clearMap();
                }
                else if(option == Common::DRAW_MAP)
                {
                    // Add the operation to the list.
                    this->addCommandToList("ROOM_MAP");
                    // Increment the index.
                    index++;
                    // Set the request.
                    request = kRoomMap;
                }
                else if(option == Common::FORMAT)
                {
                    // Add the operation to the list.
                    this->addCommandToList("FORMAT");
                    // Increment the index.
                    index++;
                    // Set the request.
                    request = kReceiveFormat;
                }
            }
            else if(operation == Common::DONT)
            {
                // Add the operation to the list.
                this->addCommandToList("DONT");
                // Increment the index.
                index++;
                // Check if we are out of bound.
                if(index > data.size())
                {
                    return;
                }
                // Retrieve the option.
                const uchar option = data.at(index);
                if(option == Common::DRAW_MAP)
                {
                    // Add the operation to the list.
                    this->addCommandToList("ROOM_MAP");
                    // Increment the index.
                    index++;
                    // Set the request.
                    request = kNoRequest;
                }
                else if(option == Common::FORMAT)
                {
                    // Add the operation to the list.
                    this->addCommandToList("FORMAT");
                    // Increment the index.
                    index++;
                    // Set the request.
                    request = kNoRequest;
                }
            }
            else if (operation == Common::WILL)
            {
                // Add the operation to the list.
                this->addCommandToList("WILL");
                // Increment the index.
                index++;
                // Check if we are out of bound.
                if(index > data.size())
                {
                    return;
                }
                // Retrieve the option.
                const uchar option = data.at(index);
                if(option == Common::MSDP)
                {
                    // Add the operation to the list.
                    this->addCommandToList("MSDP");
                    // Increment the index.
                    index++;
                    // Handle the request.
                    this->handleMSDP();
                }
                else if(option == Common::MCCP1)
                {
                    // Add the operation to the list.
                    this->addCommandToList("MCCP1");
                    // Increment the index.
                    index++;
                    // Handle the request.
                    this->handleMCCP1();
                }
                else if(option == Common::MCCP2)
                {
                    // Add the operation to the list.
                    this->addCommandToList("MCCP2");
                    // Increment the index.
                    index++;
                    // Handle the request.
                    this->handleMCCP2();
                }
            }
            else if (operation == Common::WONT)
            {
                // Add the operation to the list.
                this->addCommandToList("WONT");
                // Increment the index.
                index++;
            }
            else
            {
                qInfo() << "Operation not recognized...";
                // Increment the index.
                index++;
            }
        }
        else
        {
            qInfo() << "Is not operation...";
        }
    }
    else
    {
        qInfo() << "Data is empty...";
    }
    if(index < data.size())
    {
        if((data.at(index) == '\n') || (data.at(index) == '\0'))
        {
            qInfo() << "Remove closing character...";
            // Increment the index.
            index++;
        }
    }
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

void SocketClient::handleMSDP()
{
    // Prepare the response.
    QString response;
    response.append(static_cast<char>(Common::IAC));
    response.append(static_cast<char>(Common::DONT));
    response.append(static_cast<char>(Common::MSDP));
    response.append('\n');
    // Send the response.
    this->sendMessage(response);
    // Add the operation to the list.
    this->addCommandToList("Response: IAC-DONT-MSDP");
}

void SocketClient::handleMCCP1()
{
    // Prepare the response.
    QString response;
    response.append(static_cast<char>(Common::IAC));
    response.append(static_cast<char>(Common::DONT));
    response.append(static_cast<char>(Common::MCCP1));
    response.append('\n');
    // Send the response.
    this->sendMessage(response);
    // Add the operation to the list.
    this->addCommandToList("Response: IAC-DONT-MCCP1");
}

void SocketClient::handleMCCP2()
{
    // Prepare the response.
    QString response;
    response.append(static_cast<char>(Common::IAC));
    response.append(static_cast<char>(Common::DONT));
    response.append(static_cast<char>(Common::MCCP2));
    response.append('\n');
    // Send the response.
    this->sendMessage(response);
    // Add the operation to the list.
    this->addCommandToList("Response: IAC-DONT-MCCP2");
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
