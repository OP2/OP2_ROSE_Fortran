

#ifndef TASKLIST_H
#define TASKLIST_H

#include <QListWidget>
#include "TaskInterface.h"


/**
 * \brief Item of the TaskList widget
 */
class TaskListItem : public QListWidgetItem
{
    public:
        TaskListItem(Task * t, QListWidget * parent =0);
        ~TaskListItem();

        Task * getTask() { return task; }
    protected:
        Task * task;
};

/**
 * \brief List of (running, paused, done) processes
 * - the process is setup before, and the signals finished* are connected
 * - the list calls the RoseProcess::run
 * - processes items exist until the user removes them
 */
class TaskList : public QListWidget
{
    Q_OBJECT
    public:
        TaskList(QWidget * parent = 0);
        virtual ~TaskList() {}

        /// Adds a new task to execute
        /// takes ownership of the task object
        void submitTask (Task * task);

    signals:
        void clicked(const QString & file, int line);

    protected slots:
        void on_itemClicked ( QListWidgetItem * item );
        void taskOutputClicked( const QString &file, int line )
        { emit clicked( file, line ); }

    protected:
        TaskListItem * lastClickedItem;
};

#endif
