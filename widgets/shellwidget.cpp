/***********************************************************************
*Copyright 2010-20XX by 7ymekk
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*
*   @author 7ymekk (7ymekk@gmail.com)
*
************************************************************************/


#include "shellwidget.h"
#include "ui_shellwidget.h"

/*
  dodac 2 listy stringow:
  - commandList - w konstruktorze wczytywac komendy busyboxa i shella, a pozniej szift+tab bedzie podpowiadal komendy
  - fileList - przy przechodzeniu miedzy folderami (komenda 'cd') bedzie wywolywana metoda Phone::getFileList a nastepnie po nacisnieciu tab bedzie podpwiadac foldery
    * obie listy maja byc posortowane alfabetycznie.
    * aby uzyskac podpowiedz nie trzeba wpisywac duzych liter
*/

ShellWidget::ShellWidget(QWidget *parent) :
    QTextEdit(parent)
{
    this->tabPosition = -1;
    this->insertedChars = 0;
    this->cursorPosition = 0;
    this->commandHistoryPosition = 0;
    this->cursor = this->textCursor();
    this->setCursorWidth(3);
    this->setTextCursor(cursor);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->stopped = false;

    QSettings settings;
    this->sdk=settings.value("sdkPath").toString();

    this->fontColor = settings.value("shellFontColor", Qt::black).value<QColor>();

    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, settings.value("shellBackgroundColor", Qt::white).value<QColor>());

    this->setPalette(palette);

    this->setTextColor(this->fontColor);

    //qDebug()<<"MainWindow::showPageShell() - process shell is not running, starting...";
    this->process.setProcessChannelMode(QProcess::MergedChannels);
    this->process.start("\""+sdk+"\""+"adb shell");

    connect(&this->process, SIGNAL(readyRead()), this, SLOT(readFromProcess()));
    connect(&this->process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(processFinished(int,QProcess::ExitStatus)));
}

ShellWidget::~ShellWidget()
{
    this->process.close();
}

void ShellWidget::keyPressEvent(QKeyEvent *e)
{
    if (this->isReadOnly())
    {
        return;
    }
    if (e->modifiers() == Qt::ControlModifier)
    {
        if (e->key() == Qt::Key_C)
        {
            this->process.write(QString(QChar(0x3)).toAscii());
        }
        if (e->key() == Qt::Key_D)
        {
            this->process.write(QString(QChar(0x3)).toAscii());
            this->process.write("exit\n");
        }
        if (e->key() == Qt::Key_S)
        {
            this->stopped = true;
            this->oldToolTip = this->toolTip();
            this->setToolTip("Shell output stopped.  Press Ctrl + q to resume.");
            this->setFocus();
        }
        if (e->key() == Qt::Key_Q)
        {
            this->stopped = false;
            this->setToolTip(oldToolTip);
            this->readFromProcess();
            this->setFocus();
        }
        else if (e->key() == Qt::Key_Left)
        {
            if (this->cursorPosition < this->insertedChars)
            {
                int pos = this->cursor.position();
                this->cursor.movePosition(QTextCursor::PreviousWord);
                this->setTextCursor(this->cursor);
                this->cursorPosition+=pos-this->cursor.position();
            }
        }
        else if (e->key() == Qt::Key_Right)
        {
            if (this->cursorPosition > 0)
            {
                int pos = this->cursor.position();
                this->cursor.movePosition(QTextCursor::NextWord);
                this->setTextCursor(this->cursor);
                this->cursorPosition-=this->cursor.position()-pos;
            }
        }
        else if (e->key() == Qt::Key_Backspace)
        {
            //usun poprzedzajace slowo
        }
        else if (e->key() == Qt::Key_Delete)
        {
            //usun nastepne slowo
        }
        return;
    }
    else if ((e->modifiers() & Qt::SHIFT) && (e->modifiers() & Qt::CTRL))
    {
        if (e->key() == Qt::Key_V)
        {
            QClipboard *clipboard = QApplication::clipboard();
            QString tmp = clipboard->text(QClipboard::Clipboard);
            if (tmp.length()>0)
            {
                this->insertedChars+=tmp.length();
                this->command.insert(this->command.length()-this->cursorPosition,tmp);
                this->insertPlainText(tmp);
            }
        }
        else if (e->key() == Qt::Key_C)
        {
            QClipboard *clipboard = QApplication::clipboard();
            QString tmp = this->textCursor().selection().toPlainText();

            clipboard->setText(tmp,QClipboard::Clipboard);
        }
        return;
    }

    this->cursor.movePosition(QTextCursor::End);
    this->cursor.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,this->cursorPosition);
    this->setTextCursor(this->cursor);

    if (e->key() == Qt::Key_Return)
    {
        this->cursor.movePosition(QTextCursor::End);
        this->setTextCursor(this->cursor);
        this->cursorPosition = 0;
        this->insertedChars = 0;
        qDebug() << this->command;
        executeCommand(this->command);

        if (this->command == "exit")
        {
        this->command.clear();
        this->commandHistoryPosition = -1;
            emit closed(tabPosition);
        }else
        {
            this->command.clear();
            this->commandHistoryPosition = -1;
        }

    }
    else if (e->key() == Qt::Key_Up)
    {
        if (this->commandHistory.length() -1 > this->commandHistoryPosition)
        {
            if (command.length()>0)
            {
                this->cursor.movePosition(QTextCursor::End);
                for (int i = 0 ; i < this->insertedChars ; i++)
                    this->cursor.deletePreviousChar();
                this->cursorPosition = 0;
                this->insertedChars = 0;
                command.clear();
            }
            this->commandHistoryPosition++;
            this->command = this->commandHistory.at(this->commandHistoryPosition);
            this->insertedChars = this->command.length();
            this->insertPlainText(this->command);
        }
    }
    else if (e->key() == Qt::Key_Down)
    {
        if (this->commandHistoryPosition > 0)
        {
            if (command.length()>0)
            {
                this->cursor.movePosition(QTextCursor::End);
                for (int i = 0 ; i < this->insertedChars ; i++)
                    this->cursor.deletePreviousChar();
                this->cursorPosition = 0;
                this->insertedChars = 0;
                command.clear();
            }
            this->commandHistoryPosition--;
            this->command = this->commandHistory.at(this->commandHistoryPosition);
            this->insertedChars = this->command.length();
            this->insertPlainText(this->command);
        }
    }
    else if (e->key() == Qt::Key_Left)
    {
        if (this->cursorPosition < this->insertedChars)
        {
            this->cursor.movePosition(QTextCursor::Left);
            this->setTextCursor(this->cursor);
            this->cursorPosition++;
        }
    }
    else if (e->key() == Qt::Key_Right)
    {
        if (this->cursorPosition > 0)
        {
            this->cursor.movePosition(QTextCursor::Right);
            this->setTextCursor(this->cursor);
            this->cursorPosition--;
        }
    }
    else if (e->key() == Qt::Key_Delete)
    {
        if (this->cursorPosition > 0)
        {
            this->cursor.movePosition(QTextCursor::Right);
            this->setTextCursor(this->cursor);
            this->cursor.deletePreviousChar();
            this->command.remove(this->command.length()-this->cursorPosition-1,1);
            this->insertedChars--;
            this->cursorPosition--;
        }
    }
    else if (e->key() == Qt::Key_Backspace)
    {
        if (this->insertedChars > this->cursorPosition)
        {
            this->cursor.deletePreviousChar();
            this->command.remove(this->command.length()-this->cursorPosition-1,1);
            this->insertedChars--;
        }
    }
    else if(e->key() == Qt::Key_Escape)
    {
        this->process.write(QString(QChar(0x3)).toAscii());
    }
    else if (e->text().length()>0)
    {
        this->insertPlainText(e->text());
        this->insertedChars++;
        this->command.insert(this->command.length()-this->cursorPosition,e->text());
    }

}

void ShellWidget::executeCommand(QString command)
{
    if (command == "qtadb -help")
    {
        this->append(tr("\nQtADB shell help\n"));
        this->append(tr("CTRL+C                - interrupt executing command"));
        this->append(tr("ESC                   - interrupt executing command"));
        this->append(tr("Shift+CTRL+C          - copy selected text to clipboard"));
        this->append(tr("Shift+CTRL+V          - paste text from clipboard"));
        this->append(tr("Enter/Return          - execute command"));
        this->append(tr("Up (arrow)            - display previous executed command"));
        this->append(tr("Down (arrow)          - display next executed command"));
        this->append(tr("Left(arrow)           - move cursor to the left"));
        this->append(tr("Right(arrow)          - move cursor to the right"));
        this->append(tr("CTRL+Left(arrow)      - move cursor to the left skipping over the word"));
        this->append(tr("CTRL+Right(arrow)     - move cursor to the right skipping over the word"));
        this->append(tr("Delete                - delete next char"));
        this->append(tr("Backspace             - delete previous char"));
        this->process.write("\n");
    }
    else
    {
        this->process.write(command.toLatin1()+"\n");
    }

    this->commandHistory.prepend(command);
}

void ShellWidget::readFromProcess()
{
    if (this->stopped)
        return;
    QString tmp = QString::fromUtf8(this->process.readAll());
    QStringList tmp2;
    QString print;
    int i;

    for (i = 0; i < tmp.length(); i++)
    {
        if (tmp.at(i).unicode() == 13)
            tmp[i] = ' ';
        if (tmp.at(i).unicode() == 10)
            tmp[i] = '\n';
    }
    tmp.remove(0,tmp.indexOf("\n"));
    if (tmp.contains(QChar( 0x1b ), Qt::CaseInsensitive))
    {
        QSettings settings;
        tmp.remove("[0m");
        if (settings.value("colorShellFiles").toBool())
        {
            tmp2 = tmp.split(QChar( 0x1b ), QString::SkipEmptyParts, Qt::CaseInsensitive);

            while (tmp2.size() > 0)
            {
                print = tmp2.takeFirst();
                if (print.contains("0;30"))//black
                {
                    this->setTextColor(this->fontColor);
                }
                else if (print.contains("0;34"))//blue
                {
                    this->setTextColor(Qt::blue);
                }
                else if (print.contains("0;32"))//green
                {
                    this->setTextColor(Qt::green);
                }
                else if (print.contains("0;36"))//cyan
                {
                    this->setTextColor(Qt::cyan);
                }
                else if (print.contains("0;31"))//red
                {
                    this->setTextColor(Qt::red);
                }
                else if (print.contains("0;35"))//purple
                {
                    this->setTextColor(QColor::fromRgb(0, 0, 0));
                }
                else if (print.contains("0;33"))//brown
                {
                    this->setTextColor(QColor::fromRgb(0, 0, 0));
                }
                else if (print.contains("0;37"))//light gray
                {
                    this->setTextColor(Qt::lightGray);
                }
                else if (print.contains("1;30"))//dark gray
                {
                    this->setTextColor(Qt::darkGray);
                }
                else if (print.contains("[1;34"))//dark gray
                {
                    this->setTextColor(Qt::blue);
                }
                else if (print.contains("1;32"))//light green
                {
                    this->setTextColor(Qt::green);
                }
                else if (print.contains("1;36"))//light cyan
                {
                    this->setTextColor(Qt::cyan);
                }
                else if (print.contains("1;31"))//light red
                {
                    this->setTextColor(Qt::red);
                }
                else if (print.contains("1;35"))//light purple
                {
                    this->setTextColor(QColor::fromRgb(0, 0, 0));
                }
                else if (print.contains("1;33"))//yellow
                {
                    this->setTextColor(Qt::yellow);
                }
                else if (print.contains("1;37"))//white
                {
                    this->setTextColor(Qt::white);
                }
                print.remove(QRegExp("\\[\\d;\\d+m"));
                this->insertPlainText(print);
                this->setTextColor(this->fontColor);
            }
        }
        else
        {
            tmp.remove(QChar( 0x1b ), Qt::CaseInsensitive);
            tmp.remove(QRegExp("\\[\\d;\\d+m"));
            this->insertPlainText(tmp);
        }
    }
    else
    {
        this->append(tmp);
    }
    this->ensureCursorVisible();

    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    emit alert(tabPosition);
    //qDebug()<<"readShell() - "<<tmp;
}



void ShellWidget::processFinished(int exitcode,QProcess::ExitStatus status)
{
    switch (status)
    {
    case QProcess::NormalExit:
        this->append(QString("\r\nShell exited normally with code ")+exitcode);
        break;
    case QProcess::CrashExit:
        this->setTextColor(Qt::red);
        this->append(QString("\r\nShell exited abnormally with code ")+exitcode);
    }

    this->setReadOnly(true);

    emit alert(tabPosition);
}
