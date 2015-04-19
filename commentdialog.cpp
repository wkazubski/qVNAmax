#include "commentdialog.h"
#include "ui_commentdialog.h"

QString CommentDialog::comment;
QString comment_new;
bool CommentDialog::ask_for_comment;

CommentDialog::CommentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommentDialog)
{
    ui->setupUi(this);
    comment_new = comment;
    ui->CommentLineEdit->setText(comment_new);
    ui->CommentCheckBox->setChecked(ask_for_comment);
}

CommentDialog::~CommentDialog()
{
    delete ui;
}

void CommentDialog::on_CommentLineEdit_editingFinished()
{
    comment_new = ui->CommentLineEdit->text();
}

void CommentDialog::on_buttonBox_accepted()
{
    comment = comment_new;
    ask_for_comment = ui->CommentCheckBox->isChecked();
}
