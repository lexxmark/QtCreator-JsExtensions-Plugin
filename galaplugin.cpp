#include "galaplugin.h"
#include "galaconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <extensionsystem/pluginspec.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace Gala::Internal;

static const QString galaPluginSuffix = QString::fromLatin1("gala");

GalaPlugin::GalaPlugin()
{
    // Create your members
}

GalaPlugin::~GalaPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

void GalaPlugin::loadPlugins(const QDir& dir, QString* errorString)
{
    QFileInfoList files = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs, QDir::DirsFirst);
    for (auto file: files)
    {
        if (file.isDir())
        {
            // process sub dirs
            loadPlugins(QDir(file.filePath()), errorString);
            continue;
        }

        if (file.isFile() && (file.suffix() == galaPluginSuffix))
        {
            // try to load plugin
            QScopedPointer<GalaJSPlugin> galaPlugin(new GalaJSPlugin(this));
            QString pluginErrors;
            if (!galaPlugin->loadPlugin(file.absoluteFilePath(), &pluginErrors))
            {
                // save errors
                *errorString += QChar::CarriageReturn;
                *errorString += pluginErrors;
            }
            else
            {
                if (!galaPlugin->isDisabled())
                {
                    // save plugin
                    m_plugins.append(galaPlugin.take());
                }
            }
        }
    }
}

void GalaPlugin::invokePluginsFunction(QString functionName, bool optional)
{
    for (auto plugin: m_plugins)
    {
        QJSEngine* engine  = plugin->jsEngine();
        Q_ASSERT(engine);

        try
        {
            // try to find function
            QJSValue res = engine->evaluate(functionName);
            if (res.isError())
            {
                if (!optional)
                {
                    plugin->enableDebug();
                    plugin->debug(tr("Cannot find '%1' function.").arg(functionName));
                }
                continue;
            }

            // check functionName is a function
            if (!res.isCallable())
            {
                if (!optional)
                    plugin->enableDebug();
                plugin->debug(tr("'%1' is not a function.").arg(functionName));
                continue;
            }

            // invoke functionName function and check result
            res = res.call();
            if (res.isError())
            {
                if (!optional)
                    plugin->enableDebug();
                plugin->debug(tr("'%1' function error: '%2'.").arg(functionName, res.toString()));
                continue;
            }
        }
        catch (...)
        {
            if (!optional)
                plugin->enableDebug();
            plugin->debug(tr("Unhandeled exception while calling '%1' function.").arg(functionName));
        }
    }
}

bool GalaPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)
/*
    QAction *action = new QAction(tr("Settings"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+G")));
    connect(action, SIGNAL(triggered()), this, SLOT(onSettings()));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("GalaPlugin"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);
*/

    // search and load gala plugins
    QDir pluginDir(pluginSpec()->location());
    if (!pluginDir.exists())
    {
        *errorString = tr("Plugin dir doesn't exist.");
        return false;
    }

    loadPlugins(pluginDir, errorString);

    if (!errorString->isEmpty())
    {
        return false;
    }

    if (m_plugins.isEmpty())
    {
        *errorString += tr("\nCannot find any Gala plugin in '%1'.").arg(pluginSpec()->location());
        return false;
    }

    // sort plugins
    std::sort(m_plugins.begin(), m_plugins.end(), [](GalaJSPlugin* left, GalaJSPlugin* right) {
        return left->order() < right->order();
    });

    invokePluginsFunction(QString::fromLatin1("initialize"));

    return true;
}

void GalaPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.

    invokePluginsFunction(QString::fromLatin1("extensionsInitialized"));
}

ExtensionSystem::IPlugin::ShutdownFlag GalaPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)

    invokePluginsFunction(QString::fromLatin1("aboutToShutdown"));

    return SynchronousShutdown;
}

void GalaPlugin::onSettings()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("Show settings dialog"),
                             tr("This is an action from GalaPlugin."));
}

Q_EXPORT_PLUGIN2(Gala, GalaPlugin)

