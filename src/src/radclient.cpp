#include "../inc/radclient.h"

#include <QCursor>
#include <QColor>
#include <QMessageBox>
#include <QGraphicsRectItem>
#include <QBitmap>

RadClient::RadClient(QWidget * parent) :
    QMainWindow(parent),
    clientFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)),
    terminalPalette(QPalette::Base, QColor(10, 10, 10))
{
    // Prepare the UI.
    setupUi(this);

    // Connect buttons.
    connect(Ui_RadClient::tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    connect(new QShortcut(Qt::CTRL + Qt::Key_N, this), SIGNAL(activated()), this, SLOT(funConnect()));
    connect(new QShortcut(Qt::CTRL + Qt::Key_W, this), SIGNAL(activated()), this, SLOT(closeCurrentTab()));
    connect(new QShortcut(Qt::CTRL + Qt::Key_Q, this), SIGNAL(activated()), this, SLOT(funExit()));

    funConnect();

    loadActions();
    loadMenus();
}

RadClient::~RadClient()
{

}

void RadClient::loadActions()
{
    connectAct    = new QAction(tr("&New Connection"), this);
    disconnectAct = new QAction(tr("&Disconnect"), this);
    saveAct       = new QAction(tr("&Save Log"), this);
    exitAct       = new QAction(tr("&Exit"), this);
    aboutAct      = new QAction(tr("&About"), this);
    aboutQtAct    = new QAction(tr("&About Qt"), this);

    connectAct->setStatusTip(tr("Connect to the Mud."));
    disconnectAct->setStatusTip(tr("Disconnect from the Mud."));
    saveAct->setStatusTip(tr("Save the current terminal inside a log file."));
    exitAct->setStatusTip(tr("Exit from the client."));
    aboutAct->setStatusTip(tr("About RadMud client."));
    aboutQtAct->setStatusTip(tr("About Qt."));

    connect(connectAct, SIGNAL(triggered()), this, SLOT(funConnect()));
    connect(disconnectAct, SIGNAL(triggered()), this, SLOT(funDisconnect()));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(funSave()));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(funExit()));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(funAbout()));
    connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(funAboutQt()));
}

void RadClient::loadMenus()
{
    fileMenu = Ui_RadClient::menuBar->addMenu(tr("&File"));
    fileMenu->addAction(connectAct);
    fileMenu->addAction(disconnectAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = Ui_RadClient::menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void RadClient::closeEvent(QCloseEvent * event)
{
    event->ignore();
    funExit();
}

void RadClient::closeTab(const int & index)
{
    if (index == -1)
    {
        return;
    }
    SocketClient * currentTerminal = qobject_cast<SocketClient *>(Ui_RadClient::tabWidget->currentWidget());
    // Check if there is a terminal.
    if(!currentTerminal)
    {
        return;
    }
    // Check if the selected terminal is connected.
    if(currentTerminal->isConnected())
    {
        Ui_RadClient::statusBar->showMessage("The connection is still active.");
        return;
    }
    // Removes the tab at position index from this stack of widgets.
    Ui_RadClient::tabWidget->removeTab(index);
    // The page widget itself is not deleted.
    delete(currentTerminal);
}

void RadClient::closeCurrentTab()
{
    this->closeTab(Ui_RadClient::tabWidget->currentIndex());
}

void RadClient::funConnect()
{
    SocketClient * newTerminal = new SocketClient(this);

    // Try to connect.
    if(newTerminal->connectToHost("localhost", 4000))
    {
        Ui_RadClient::tabWidget->addTab(newTerminal, "Tab");
        Ui_RadClient::tabWidget->setCurrentWidget(newTerminal);
        newTerminal->inputLine->setFocus();
    }
    else
    {
        Ui_RadClient::statusBar->showMessage("Cannot connect to the Mud.");
        delete(newTerminal);
    }
}

void RadClient::funDisconnect()
{
    SocketClient * currentTerminal = qobject_cast<SocketClient *>(Ui_RadClient::tabWidget->currentWidget());
    // Check if there is a terminal.
    if(!currentTerminal)
    {
        return;
    }
    // Check if the selected terminal is connected.
    if(currentTerminal->isConnected())
    {
        currentTerminal->disconnectFromHost();
    }
    closeCurrentTab();
}

void RadClient::funSave()
{

}

void RadClient::funExit()
{
    for(int it = 0; it < Ui::RadClient::tabWidget->count(); ++it)
    {
        SocketClient * terminal = qobject_cast<SocketClient *>(Ui_RadClient::tabWidget->widget(it));
        if(terminal->isConnected())
        {
            Ui_RadClient::statusBar->showMessage("There are still connections to the Mud.");
            return;
        }
    }
    QCoreApplication::quit();
}

void RadClient::funAbout()
{
    QMessageBox::information(NULL, "About RadClient", "This is a client developed mainly for connections with the Mud called RadMud.");
}

void RadClient::funAboutQt()
{
}
