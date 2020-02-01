#ifndef PROPERTYNAMEDIALOG_H
#define PROPERTYNAMEDIALOG_H
#include <QDialog>
#include <QLabel>
#include <QLineEdit>

class Property;

class PropertyNameDialog : public QDialog {
public:
    PropertyNameDialog(const QString& initialValue,
                       QWidget* const parent = nullptr);

    static bool sRenameProperty(Property* const prop,
                                QWidget* const parent);

    QString name() const
    { return mName->text(); }
private:
    bool validate();

    QLineEdit* mName;
    QPushButton* mOkButton;
    QLabel* mErrorLabel;
};

#endif // PROPERTYNAMEDIALOG_H
