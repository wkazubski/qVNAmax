#ifndef COMMENTDIALOG_H
#define COMMENTDIALOG_H

#include <QDialog>

namespace Ui {
class CommentDialog;
}

class CommentDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CommentDialog(QWidget *parent = 0);
    ~CommentDialog();

    static QString comment;
    
    static bool ask_for_comment;

    static bool window_open;

private slots:
    void on_CommentLineEdit_editingFinished();

    void on_buttonBox_accepted();

private:
    Ui::CommentDialog *ui;
};

#endif // COMMENTDIALOG_H
