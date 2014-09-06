#ifndef JEP_API_H
#define JEP_API_H

#include <QObject>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/imode.h>
#include <coreplugin/icore.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/inavigationwidgetfactory.h>

#include <QQmlEngine>
#include <QQmlError>
#include <QtGlobal>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextStream>

#include <QMap>
#include <functional>

class QQuickView;

namespace JsExtensions {
namespace Internal {

class MyEventFilter: public QObject
{
    Q_OBJECT

public:
    MyEventFilter(QDialog* dlg, QWidget* w, QQuickView *v);
    ~MyEventFilter();

protected:
    bool eventFilter(QObject *, QEvent *event) override;

private:
    QDialog *m_dlg;
    QWidget *m_w;
    QQuickView *m_v;
};

class JsPlugin;

struct JsPluginInfo
{
    QString name;
    QString description;
    qint32 priority;
    bool isEnabled;
    bool trace;
    JsPlugin* plugin;

    JsPluginInfo(): priority(0), isEnabled(true), trace(false), plugin(nullptr)
    {}

    void save(QSettings* settings) const;
    void restore(QSettings* settings);

    bool hasSettings();
    bool invokeSettings(QObject *parent, QString& errors);
};

class JsPlugin: public QObject
{
    Q_OBJECT

public:
    explicit JsPlugin(QObject* parent = nullptr);
    ~JsPlugin();

    const JsPluginInfo& info() const { return m_info; }
    JsPluginInfo& info() { return m_info; }

    QJSEngine* jsEngine() { return m_jsEngine.data(); }
    bool loadPlugin(QString pluginPath, QString* errorString);

    void installJsContext(QJSEngine* jsEngine);
    void installQmlContext(QQmlEngine* qmlEngine);

    void changeDebugIndent(qint32 delta);
    QPair<QWidget*, QQuickView*> createQuickViewWidget(QString qmlUrl, QObject* parent);

signals:
    void aboutToShutdown();

public slots:
    bool loadLib(QString libFileName);
    bool include(QString jsFileName);
    void debug(QString str);

    QString loadFile(QString fileName);
    bool saveFile(QString fileName, QString content);

    bool execute(QString cmd);

    QString getClipboard();
    void setClipboard(QString content);

    QVariantMap QObject2JsObject(QObject* qObject);
    void JsObject2QObject(QVariantMap object, QObject* qObject);

    QJSValue createQuickView(QString qmlUrl, QObject* parent);
    int quickDialogExec(QString qmlUrl, QObject* parent);
    QJSValue createQObject(QString type, QObject* parent);

    QJSValue point(int x, int y) { return m_jsEngine->toScriptValue(QPoint(x, y)); }
    QJSValue rect(int x, int y, int w, int h) { return m_jsEngine->toScriptValue(QRect(x, y, w, h)); }
    QJSValue size(int w, int h) { return m_jsEngine->toScriptValue(QSize(w, h)); }
    QJSValue sizePolicy(int hor, int ver, int type) { return m_jsEngine->toScriptValue(QSizePolicy(QSizePolicy::Policy(hor), QSizePolicy::Policy(ver), QSizePolicy::ControlType(type))); }

    bool registerNavigationWidgetFactory(QJSValue factory, QString displayName, int priority, QString id, QString activationSequence);

    void dumpPluginManagerObjects();

private:
    bool enableDebug();
    QString normalizeFileName(QString fileName);

    QScopedPointer<QJSEngine> m_jsEngine;
    QMap<QByteArray, std::function<QObject*(QObject*)>> m_factories;

    // plugin info
    QString m_pluginPath;
    JsPluginInfo m_info;

    // debugging/tracing
    QScopedPointer<QTextStream> m_debugStream;
    qint32 m_debugIndent;
};

struct GContext
{
    JsPlugin* plugin;
    QJSEngine* jsEngine;

    bool isValid() const
    {
        return plugin && jsEngine;
    }

    struct TraceRecord
    {
        TraceRecord(JsPlugin* plugin, const char* file, int line, const char* func)
            : plugin(plugin)
        {
            Q_ASSERT(plugin);
            if (plugin->info().trace)
            {
                record = QString("%1 : %2 : %3").arg(file).arg(line).arg(func);

                plugin->debug(QString("Start : ") + record);
                plugin->changeDebugIndent(+4);
            }
        }

        ~TraceRecord()
        {
            if (plugin->info().trace)
            {
                plugin->changeDebugIndent(-4);
                plugin->debug(QString("End   : ") + record);
            }
        }

        JsPlugin* plugin;
        QString record;
    };
};

#define G_TRACE GContext::TraceRecord trace(m_gContext.plugin, __FILE__, __LINE__, Q_FUNC_INFO)
#define G_TRACE2(plugin) GContext::TraceRecord trace(plugin, __FILE__, __LINE__, Q_FUNC_INFO)

class GWrapper: public QObject
{
    Q_OBJECT

protected:
    GWrapper(GContext gContext)
        : m_gContext(gContext)
    {
        Q_ASSERT(m_gContext.isValid());
    }

    ~GWrapper() {}

    template <typename T>
    QJSValue wrapObject(T* obj) const
    { return m_gContext.jsEngine->toScriptValue(static_cast<QObject*>(obj)); }

    static Core::Id str2id(QString id) { return Core::Id(id.toLatin1().constData()); }
    static QString id2str(Core::Id id) { return QString::fromLatin1(id.name()); }

    GContext m_gContext;
};

class GCore: public GWrapper
{
    Q_OBJECT

public:
    GCore(GContext gContext)
        : GWrapper(gContext),
          m_owner(Core::ICore::instance())
    {
        Q_ASSERT(m_owner);
        G_TRACE;
    }
    ~GCore() {G_TRACE;}

    Core::ICore* owner() { return m_owner; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    bool showOptionsDialog(QString group, QString page, QWidget *parent) { G_TRACE; return m_owner->showOptionsDialog(str2id(group), str2id(page), parent); }
    QString msgShowOptionsDialog() { G_TRACE; return m_owner->msgShowOptionsDialog(); }

    bool showWarningWithOptions(QString title, QString text,
                                   QString details/* = QString()*/,
                                   QString settingsCategory/* = Id()*/,
                                   QString settingsId/* = Id()*/,
                                   QWidget *parent/* = 0*/)
    {
        G_TRACE;
        return m_owner->showWarningWithOptions(title, text, details, str2id(settingsCategory), str2id(settingsId), parent);
    }

    QString userInterfaceLanguage() { G_TRACE; return m_owner->userInterfaceLanguage(); }

    QString resourcePath() { G_TRACE; return m_owner->resourcePath(); }
    QString userResourcePath() { G_TRACE; return m_owner->userResourcePath(); }
    QString documentationPath() { G_TRACE; return m_owner->documentationPath(); }
    QString libexecPath() { G_TRACE; return m_owner->libexecPath(); }

    QString versionString() { G_TRACE; return m_owner->versionString(); }
    QString buildCompatibilityString() { G_TRACE; return m_owner->buildCompatibilityString(); }

    QJSValue mainWindow() { G_TRACE; return m_gContext.jsEngine->toScriptValue(m_owner->mainWindow()); }
    QJSValue dialogParent() { G_TRACE; return m_gContext.jsEngine->toScriptValue(m_owner->dialogParent()); }
    QJSValue statusBar() { G_TRACE; return m_gContext.jsEngine->toScriptValue(m_owner->statusBar()); }
/*
    enum OpenFilesFlags {
        None = 0,
        SwitchMode = 1,
        CanContainLineNumbers = 2,
         /// Stop loading once the first file fails to load
        StopOnLoadFail = 4
    };
    */
    void openFiles(const QStringList &fileNames, int flags) { G_TRACE; m_owner->openFiles(fileNames, (Core::ICore::OpenFilesFlags)flags); }

    void emitNewItemsDialogRequested() { G_TRACE; m_owner->emitNewItemsDialogRequested(); }

private:
    Core::ICore* m_owner;
};

class GMessageManager: public GWrapper
{
    Q_OBJECT

public:
    GMessageManager(GContext gContext)
        : GWrapper(gContext),
          m_owner(qobject_cast<Core::MessageManager*>(Core::MessageManager::instance()))
    {
        G_TRACE;
        Q_ASSERT(m_owner);
    }
    ~GMessageManager() { G_TRACE; }

    Core::MessageManager* owner() { return m_owner; }

public slots:
    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    void showOutputPane() { G_TRACE; m_owner->showOutputPane(); }
/*
    enum PrintToOutputPaneFlag {
        NoModeSwitch   = IOutputPane::NoModeSwitch,
        ModeSwitch     = IOutputPane::ModeSwitch,
        WithFocus      = IOutputPane::WithFocus,
        EnsureSizeHint = IOutputPane::EnsureSizeHint,
        Silent         = 256,
        Flash          = 512
    };
*/
    void write(QString text, int flags) { G_TRACE; m_owner->write(text, (Core::MessageManager::PrintToOutputPaneFlags)flags); }

private:
    Core::MessageManager* m_owner;
};

class GModeManager : public GWrapper
{
    Q_OBJECT

public:
    GModeManager(GContext gContext)
        : GWrapper(gContext),
          m_owner(qobject_cast<Core::ModeManager*>(Core::ModeManager::instance()))
    {
        Q_ASSERT(m_owner);
        G_TRACE;
    }
    ~GModeManager() { G_TRACE; }

    Core::ModeManager* owner() { return m_owner; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    QJSValue currentMode() { G_TRACE; return wrapObject(m_owner->currentMode()); }
    QJSValue mode(QString id) { G_TRACE; return wrapObject(m_owner->mode(str2id(id))); }

    void addAction(QAction *action, int priority) { G_TRACE; m_owner->addAction(action, priority); }
    void addProjectSelector(QAction *action) { G_TRACE; m_owner->addProjectSelector(action); }
    void addWidget(QWidget *widget) { G_TRACE; m_owner->addWidget(widget); }

    void activateMode(QString id) { G_TRACE; m_owner->activateMode(str2id(id)); }
    void setFocusToCurrentMode() { G_TRACE; m_owner->setFocusToCurrentMode(); }
    bool isModeSelectorVisible() { G_TRACE; return m_owner->isModeSelectorVisible(); }

    void setModeSelectorVisible(bool visible) { G_TRACE; m_owner->setModeSelectorVisible(visible); }

private:
    Core::ModeManager* m_owner;
};

class GCommand : public GWrapper
{
    Q_OBJECT
public:
    GCommand(GContext gContext, Core::Command* owner)
        : GWrapper(gContext),
          m_owner(owner)
    {
        Q_ASSERT(m_owner);
        G_TRACE;
    }
    ~GCommand() { G_TRACE; }

    Core::Command* owner() { return m_owner; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    void setDefaultKeySequence(const QString &key) { G_TRACE; m_owner->setDefaultKeySequence(key); }
    QString defaultKeySequence() const { G_TRACE; return m_owner->defaultKeySequence(); }
    QString keySequence() const { G_TRACE; return m_owner->keySequence(); }
    void setDescription(const QString &text) { G_TRACE; m_owner->setDescription(text); }
    QString description() const { G_TRACE; return m_owner->description(); }

    QString id() const { G_TRACE; return id2str(m_owner->id()); }

    QJSValue action() const { G_TRACE; return wrapObject(m_owner->action()); }
    QString context() const
    {
        G_TRACE;
        Core::Context ctx = m_owner->context();
        return ctx.isEmpty() ? QString() : QString::fromLatin1(ctx.at(0).name());
    }

    void setAttribute(int attr) { G_TRACE; m_owner->setAttribute((Core::Command::CommandAttribute)attr); }
    void removeAttribute(int attr) { G_TRACE; m_owner->removeAttribute((Core::Command::CommandAttribute)attr); }
    bool hasAttribute(int attr) const { G_TRACE; return m_owner->hasAttribute((Core::Command::CommandAttribute)attr); }

    bool isActive() const { G_TRACE; return m_owner->isActive(); }

    void setKeySequence(const QString &key) { G_TRACE; m_owner->setKeySequence(key); }
    QString stringWithAppendedShortcut(const QString &str) const { G_TRACE; return m_owner->stringWithAppendedShortcut(str); }

    bool isScriptable1() const { G_TRACE; return m_owner->isScriptable(); }
    bool isScriptable2(const QString & context) const { G_TRACE; return m_owner->isScriptable(Core::Context(str2id(context))); }

private:
    Core::Command* m_owner;
};


class GActionContainer : public GWrapper
{
    Q_OBJECT

public:
    GActionContainer(GContext gContext, Core::ActionContainer *owner)
    : GWrapper(gContext),
      m_owner(owner)
    {
        Q_ASSERT(m_owner);
        G_TRACE;
    }
    ~GActionContainer() { G_TRACE; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    void setOnAllDisabledBehavior(int behavior) { G_TRACE; m_owner->setOnAllDisabledBehavior(Core::ActionContainer::OnAllDisabledBehavior(behavior)); }
    int onAllDisabledBehavior() const { G_TRACE; return m_owner->onAllDisabledBehavior(); }

    QString id() const { G_TRACE; return id2str(m_owner->id()); }

    QJSValue menu() const { G_TRACE; return wrapObject(m_owner->menu()); }
    QJSValue menuBar() const { G_TRACE; return wrapObject(m_owner->menuBar()); }

    // returns QAction*
    QJSValue insertLocation(QString group) const { G_TRACE; return wrapObject(m_owner->insertLocation(str2id(group))); }
    void appendGroup(QString group) { G_TRACE; m_owner->appendGroup(str2id(group)); }
    void insertGroup(QString before, QString group) { G_TRACE; m_owner->insertGroup(str2id(before), str2id(group)); }
    void addAction(JsExtensions::Internal::GCommand *action, QString group/* = QString()*/) { G_TRACE; m_owner->addAction(action->owner(), str2id(group)); }
    void addMenu1(JsExtensions::Internal::GActionContainer *menu, QString group/* = QString()*/) { G_TRACE; m_owner->addMenu(menu->m_owner, str2id(group)); }
    void addMenu2(JsExtensions::Internal::GActionContainer *before, JsExtensions::Internal::GActionContainer *menu, QString group/* = QString()*/) { G_TRACE; m_owner->addMenu(before->m_owner, menu->m_owner, str2id(group)); }
    JsExtensions::Internal::GCommand *addSeparator(const QString &context, QString group/* = QString()*/, QAction **outSeparator/* = 0*/) { G_TRACE; return new GCommand(m_gContext, m_owner->addSeparator(Core::Context(str2id(context)), str2id(group), outSeparator)); }

    void clear() { G_TRACE; m_owner->clear(); }

private:
    Core::ActionContainer *m_owner;
};

class GActionManager : public GWrapper
{
    Q_OBJECT

public:
    explicit GActionManager(GContext gContext)
        : GWrapper(gContext),
          m_owner(Core::ActionManager::instance())
    {
        Q_ASSERT(m_owner);
        G_TRACE;
    }
    ~GActionManager() { G_TRACE; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    JsExtensions::Internal::GActionContainer *createMenu(QString id) { G_TRACE; return new GActionContainer(m_gContext, m_owner->createMenu(str2id(id))); }
    JsExtensions::Internal::GActionContainer *createMenuBar(QString id) { G_TRACE; return new GActionContainer(m_gContext, m_owner->createMenuBar(str2id(id))); }

    JsExtensions::Internal::GCommand *registerAction(QAction *action, QString id, const QString &context, bool scriptable/* = false*/)
    {
        G_TRACE;
        Core::Context ctx(str2id(context));
        return new GCommand(m_gContext, m_owner->registerAction(action, str2id(id), ctx, scriptable));
    }

    JsExtensions::Internal::GCommand *command(QString id)
    {
        G_TRACE;
        Core::Command* cmd = m_owner->command(str2id(id));
        if (!cmd) {
            m_gContext.plugin->debug(QString("Cannot create command with id: %1").arg(id));
            return nullptr;
        }

        return new GCommand(m_gContext, cmd);
    }
    JsExtensions::Internal::GActionContainer *actionContainer(QString id) { G_TRACE; return new GActionContainer(m_gContext, m_owner->actionContainer(str2id(id))); }

    QJSValue commands()
    {
        G_TRACE;
        QList<Core::Command *> commands = m_owner->commands();
        QJSValue array = m_gContext.jsEngine->newArray(commands.size());

        quint32 i = 0;
        foreach (Core::Command *command, commands)
            array.setProperty(i++, m_gContext.jsEngine->newQObject(new GCommand(m_gContext, command)));
        return array;
    }

    void unregisterAction(QAction *action, QString id) { G_TRACE; m_owner->unregisterAction(action, str2id(id)); }

    void setPresentationModeEnabled(bool enabled) { G_TRACE; m_owner->setPresentationModeEnabled(enabled); }
    bool isPresentationModeEnabled() { G_TRACE; return m_owner->isPresentationModeEnabled(); }

private:
    Core::ActionManager* m_owner;
};

class GNavigationWidgetFactory: public Core::INavigationWidgetFactory
{
    Q_OBJECT

public:
    GNavigationWidgetFactory(JsPlugin* owner, QJSValue factory, QString displayName, int priority, QString id, QString activationSequence);

    QString displayName() const { return m_displayName; }
    int priority() const { return m_priority; }
    Core::Id id() const { return Core::Id(m_id.toLatin1().constData()); }
    QKeySequence activationSequence() const { return QKeySequence(m_activationSequence); }

    Core::NavigationView createWidget();

private:
    JsPlugin* m_owner;
    QJSValue m_factory;

    QString m_displayName;
    int m_priority;
    QString m_id;
    QString m_activationSequence;
};

} // namespace Internal
} // namespace JsExtensions

#endif // JEP_API_H

