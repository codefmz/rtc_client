#ifndef IP_EDIT_H
#define IP_EDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QBoxLayout>
#include <QPaintEvent>
#include <map>

class IPEdit : public QWidget
{
    Q_OBJECT
public:
    explicit IPEdit(QWidget *parent = 0);
    ~IPEdit();
    
    QString text();
    void setText(const QString& text);
    
protected:
    void init();
    void initForEdit(QLineEdit* edit);
    QLineEdit* nextEdit(QLineEdit* curEdit);
    QLineEdit* preEdit(QLineEdit *curEdit);
    bool isEdit(QObject* object);
    
    virtual void paintEvent(QPaintEvent* event);
    virtual bool eventFilter(QObject* object, QEvent* event);
    
protected slots:
    void editTextChanged(const QString& text);
    
private:
    QBoxLayout* m_layout;
    std::map<QLineEdit*, int> editIndexMap;
    QLineEdit *editArr[4];
    int curEditIndex;
    const int EDIT_NUM = 4;
};

#endif // IP_EDIT_H
