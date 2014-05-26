#ifndef JEP_H
#define JEP_H

#include <extensionsystem/iplugin.h>
#include <QList>
#include <QDir>

#include "JsExtensions_global.h"
#include "JepAPI.h"

namespace JsExtensions {
namespace Internal {

class JsExtensionsPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "JsExtensions.json")

public:
    JsExtensionsPlugin();
    ~JsExtensionsPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
    void onSettings();

private:
    void loadPlugins(const QDir& dir, QString* errorString);
    void invokePluginsFunction(QString functionName, bool optional = true);

    QList<JsPlugin*> m_plugins;
    QList<JsPluginInfo> m_pluginInfos;
};

} // namespace Internal
} // namespace JsExtensions

#endif // JEP_H

