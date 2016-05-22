#ifndef HISTORYLINEEDIT_H
#define HISTORYLINEEDIT_H

#include <QLineEdit>

class HistoryLineEdit : public QLineEdit
{
        Q_OBJECT
    public:
        explicit HistoryLineEdit(QWidget * parent = 0);

        /// @brief Number of available lines.
        int lineCount() const { return lines.size(); }

        /// @brief Stored history.
        QStringList history() const { return lines; }

        /// @brief Overwrite the line history.
        void setHistory(const QStringList& history);

        /// @brief Sets the completer used on a per-word completion
        /// Unlike setCompleter(), this suggests completion at every entered word
        /// If \c completer is null it will remove the current completer
        void setWordCompleter(QCompleter* completer);

        /// @brief Sets a prefix that is ignored by the word completer.
        void setWordCompleterPrefix(const QString& prefix);

        /// @brief Sets the minimum number of characters required to display the word completer.
        void setWordCompleterMinChars(int min_chars);

        /// @brief Sets the maximum number of suggestions that the completer should show.
        /// If more than this many suggestions are found the completer isn't shown.
        void setWordCompleterMaxSuggestions(int max);

    public slots:
        /// @brief Executes the current line.
        void execute();

    signals:
        /// @brief Emitted when some text is executed.
        void lineExecuted(QString);

    protected:
        void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
        void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;

        void previous_line();
        void next_line();

        /// @brief Current word being edited (used to fire the completer).
        QString current_word() const;

    private slots:
        /// @brief Autocompletes the current word.
        void autocomplete(const QString& completion);

    private:
        /// @brief Returns the index of the character starting the currently edited word.
        int word_start() const;

        void selectLine();

        int         current_line;
        QStringList lines;
        QString     unfinished;

        QCompleter* completer;
        QString     completion_prefix;
        int         completion_minchars;
        int         completion_max;

    public: // SETTINGS
        bool keepSelected;
};

#endif // HISTORYLINEEDIT_H
