#ifndef JEP_PLUGINS_DIALOG_H
#define JEP_PLUGINS_DIALOG_H

#include <QDialog>
#include <QStandardItemModel>

#include "JepAPI.h"

namespace Ui {
class JepPluginsDialog;
}

namespace JsExtensions {
namespace Internal {

class JepPluginsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JepPluginsDialog(QWidget *parent = 0);
    ~JepPluginsDialog();

    void plugins2Model(const QList<JsPluginInfo>& plugins);
    void model2Plugins(QList<JsPluginInfo>& plugins);

private slots:
    void onSettings();

private:
    QScopedPointer<Ui::JepPluginsDialog> ui;
    QStandardItemModel m_model;
    QList<JsPluginInfo> m_plugins;
};

} // namespace Internal
} // namespace JsExtensions

#endif // JEP_PLUGINS_DIALOG_H
