#include "ip_edit.h"
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QStyleOptionButton>
#include <QKeyEvent>
#include <QShortcutEvent>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QClipboard>
#include <QDebug>

IPEdit::IPEdit(QWidget *parent) : QWidget(parent)
{
    init();
}

IPEdit::~IPEdit()
{
}

void IPEdit::init()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(3);
    m_layout->setContentsMargins(2, 2, 2, 2);

    for (int i = 0; i < EDIT_NUM; ++i) {
        auto edit = new QLineEdit(this);
        initForEdit(edit);

        if (i > 0) {
            auto dotLab = new QLabel(".", this);
            m_layout->addWidget(dotLab);
        }
        m_layout->addWidget(edit, 1);
        editIndexMap.insert({edit, i});
        editArr[i] = edit;
    }
}

void IPEdit::initForEdit(QLineEdit* edit)
{
    edit->setFrame(false);
    edit->setAlignment(Qt::AlignCenter);
    edit->installEventFilter(this);
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(QRegularExpression("^(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$"), this);
    edit->setValidator(validator);
    connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(editTextChanged(const QString&)));
}

QLineEdit* IPEdit::nextEdit(QLineEdit* curEdit)
{
    auto iter = editIndexMap.find(curEdit);
    if (iter == editIndexMap.end()) {
        return nullptr;
    }

    if (iter->second + 1 >= EDIT_NUM) {
        return nullptr;
    }

    return editArr[iter->second + 1];
}

QLineEdit *IPEdit::preEdit(QLineEdit *curEdit)
{
    auto iter = editIndexMap.find(curEdit);
    if (iter == editIndexMap.end()) {
        return nullptr;
    }

    if (iter->second - 1 < 0) {
        return nullptr;
    }

    return editArr[iter->second - 1];
}

bool IPEdit::isEdit(QObject* object)
{
    auto iter = editIndexMap.find((QLineEdit *)object);
    return iter != editIndexMap.end();
}

QString IPEdit::text()
{
    QString res;
    for (int i = 0; i < EDIT_NUM; ++i) {
        auto sec = editArr[i]->text();
        if (sec.isEmpty()) {
            return "";
        }

        if (i < EDIT_NUM - 1) {
            res.append(sec + ".");
        } else {
            res.append(sec);
        }
    }

    return res;
}

void IPEdit::setText(const QString& text)
{
    static QRegularExpression reg("^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");
    QRegularExpressionMatch match = reg.match(text);
    if (!match.hasMatch()) {
        return;
    }

    QStringList ips = text.split(".");
    for (auto i = 0; i < EDIT_NUM; ++i) {
        editArr[i]->setText(ips[i]);
    }
}

void IPEdit::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    QStyleOptionFrame option;
    option.initFrom(this);
    option.lineWidth = 1;
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter, this);
    QWidget::paintEvent(event);
}

bool IPEdit::eventFilter(QObject* object, QEvent* event)
{
    if (isEdit(object)) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Period || keyEvent->key() == Qt::Key_Right) {
                QLineEdit* next = nextEdit(qobject_cast<QLineEdit*>(object));
                if (next) {
                    next->setFocus();
                    next->selectAll();
                }
            } else if(keyEvent->key() == Qt::Key_Left) {
                QLineEdit *before = preEdit(qobject_cast<QLineEdit*>(object));
                if (before) {
                    before->setFocus();
                    before->selectAll();
                }
            } else if (keyEvent->matches(QKeySequence::Paste)) {
                QString clip = QApplication::clipboard()->text(QClipboard::Clipboard);
                if (clip.split(".").size() == 4) {
                    setText(clip);
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(object, event);
}

void IPEdit::editTextChanged(const QString& text)
{
    QLineEdit* curEdit = qobject_cast<QLineEdit*>(sender());
    if (text.size() == 3) {
        QLineEdit* next = nextEdit(curEdit);
        if (next) {
            next->setFocus();
            next->selectAll();
        }
    }
}
