#include "JsExtensionsPlugin.h"
#include "JsExtensionsConstants.h"
#include "JepPluginsDialog.h"

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
#include <QDebug>

#include <QtPlugin>

using namespace JsExtensions::Internal;

static const QString jepPluginSuffix = "jep.js";

JsExtensionsPlugin::JsExtensionsPlugin()
{
    // Create your members
}

JsExtensionsPlugin::~JsExtensionsPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

void JsExtensionsPlugin::loadPlugins(const QDir& dir, QSettings* settings, QString* errorString)
{
    QFileInfoList files = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs, QDir::DirsFirst);
    foreach (const QFileInfo &file, files)
    {
        if (file.isDir())
        {
            // process sub dirs
            loadPlugins(QDir(file.filePath()), settings, errorString);
            continue;
        }

        if (file.isFile() && (file.completeSuffix() == jepPluginSuffix))
        {
            // try to load plugin
            QScopedPointer<JsPlugin> jsPlugin(new JsPlugin(this));
            QString pluginErrors;
            if (!jsPlugin->loadPlugin(file.absoluteFilePath(), &pluginErrors))
            {
                // save errors
                *errorString += QChar::CarriageReturn;
                *errorString += pluginErrors;
            }
            else
            {
                // restore plugin info from settings
                jsPlugin->info().restore(pluginSettings());

                // collect all plugin infos
                m_pluginInfos.append(jsPlugin->info());

                if (jsPlugin->info().isEnabled)
                {
                    // save plugin
                    m_plugins.append(jsPlugin.take());
                }
            }
        }
    }
}

void JsExtensionsPlugin::invokePluginsFunction(QString functionName, bool optional)
{
    foreach (JsPlugin *plugin, m_plugins)
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
                    plugin->debug(tr("Cannot find '%1' function.").arg(functionName));
                continue;
            }

            // check functionName is a function
            if (!res.isCallable())
            {
                plugin->debug(tr("'%1' is not a function.").arg(functionName));
                continue;
            }

            GContext::TraceRecord trace(plugin, "", 0, functionName.toLatin1().data());

            // invoke functionName function and check result
            res = res.call();
            if (res.isError())
            {
                plugin->debug(tr("'%1' function error: '%2'.").arg(functionName, res.toString()));
                continue;
            }
        }
        catch (...)
        {
            plugin->debug(tr("Unhandeled exception while calling '%1' function.").arg(functionName));
        }
    }
}

QSettings* JsExtensionsPlugin::pluginSettings()
{
    return Core::ICore::settings();
}

bool JsExtensionsPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    //
    // setup "JepSettings" action
    //
    QAction *action = new QAction(tr("Settings..."), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+J")));
    connect(action, SIGNAL(triggered()), this, SLOT(onSettings()));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("JsExtensionsPlugin"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    //
    // search and load jep plugins
    //
    QDir pluginsDir(pluginSpec()->location());
    if (!pluginsDir.exists())
    {
        *errorString = tr("Plugin dir doesn't exist.");
        return false;
    }

    if (!pluginsDir.cd("jep_plugins"))
    {
        *errorString = tr("'jep_plugins' sub-dir doesn't exist.");
        return false;
    }

    {
        QSettings* settings = pluginSettings();
        settings->beginGroup("JsExtensionsPlugin");

        loadPlugins(pluginsDir, settings, errorString);

        settings->endGroup(); // JsExtensionsPlugin
    }

    if (!errorString->isEmpty())
    {
        return false;
    }

    if (m_plugins.isEmpty())
    {
        *errorString += tr("\nCannot find any jep plugin in '%1'.").arg(pluginsDir.absolutePath());
        return false;
    }

    // reorder plugins
    std::sort(m_plugins.begin(), m_plugins.end(), [](JsPlugin* left, JsPlugin* right) {
        return left->info().priority < right->info().priority;
    });

    invokePluginsFunction("initialize");

    return true;
}

void JsExtensionsPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.

    invokePluginsFunction("extensionsInitialized");
}

ExtensionSystem::IPlugin::ShutdownFlag JsExtensionsPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)

    {
        QSettings* settings = pluginSettings();
        settings->beginGroup("JsExtensionsPlugin");

        // save plugin info to settings
        for (auto it = m_pluginInfos.begin(); it != m_pluginInfos.end(); ++it)
        {
            (*it).save(pluginSettings());
        }

        settings->endGroup(); //"JsExtensionsPlugin"
    }

    invokePluginsFunction("aboutToShutdown");

    return SynchronousShutdown;
}

void JsExtensionsPlugin::onSettings()
{
    QSettings* settings = pluginSettings();
    settings->beginGroup("JsExtensionsPlugin");
    settings->beginGroup("SettingsDialog");

    JepPluginsDialog dlg(Core::ICore::dialogParent());
    dlg.restoreGeometry(settings->value("geometry").toByteArray());

    dlg.plugins2Model(m_pluginInfos);
    if (dlg.exec() == QDialog::Accepted)
        dlg.model2Plugins(m_pluginInfos);

    settings->setValue("geometry", dlg.saveGeometry());

    settings->endGroup(); //SettingsDialog
    settings->endGroup(); //"JsExtensionsPlugin"
}

Q_EXPORT_PLUGIN2(JsExtensions, JsExtensionsPlugin)

