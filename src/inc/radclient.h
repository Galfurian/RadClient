#ifndef RADCLIENT_H
#define RADCLIENT_H

#include <QMainWindow>
#include <QLineEdit>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QCloseEvent>
#include <QShortcut>

#include "ui_radclient.h"
#include "socketclient.h"

class RadClient : public QMainWindow, public Ui::RadClient
{
        Q_OBJECT

    public:
        explicit RadClient(QWidget *parent = 0);
        ~RadClient();

    private:
        QFont clientFont;
        QCursor clientCursor;
        QPixmap clientCursorImage;
        QPalette terminalPalette;

    private:
        void loadActions();
        void loadMenus();

        QMenu * fileMenu;
        QMenu * helpMenu;

        QActionGroup * alignmentGroup;

        QAction * connectAct;
        QAction * disconnectAct;
        QAction * saveAct;
        QAction * exitAct;
        QAction * aboutAct;
        QAction * aboutQtAct;

    private slots:
        void closeEvent(QCloseEvent * event);
        void closeTab(const int & index);
        void closeCurrentTab();

        void funConnect();
        void funDisconnect();
        void funSave();
        void funExit();
        void funAbout();
        void funAboutQt();

};

#endif // RADCLIENT_H
