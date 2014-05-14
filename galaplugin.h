#ifndef GALA_H
#define GALA_H

#include <extensionsystem/iplugin.h>
#include <QList>
#include <QDir>

#include "gala_global.h"
#include "galaAPI.h"

namespace Gala {
namespace Internal {

class GalaPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Gala.json")

public:
    GalaPlugin();
    ~GalaPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
    void onSettings();

private:
    void loadPlugins(const QDir& dir, QString* errorString);
    void invokePluginsFunction(QString functionName, bool optional = true);

    QList<GalaJSPlugin*> m_plugins;
};

} // namespace Internal
} // namespace Gala

#endif // GALA_H

