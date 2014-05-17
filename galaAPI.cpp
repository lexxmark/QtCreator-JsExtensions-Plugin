#include "galaAPI.h"
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>

#include <QtQml>
#include <QQuickView>

static int coreTypeId = qmlRegisterType<GCore>();
static int messageManagerTypeId = qmlRegisterType<GMessageManager>();
static int modeManagerTypeId = qmlRegisterType<GModeManager>();
static int commandTypeId = qmlRegisterType<GCommand>();
static int editorCommandTypeId = qmlRegisterType<Core::IEditor>();
static int documentCommandTypeId = qmlRegisterType<GDocument>();
static int actionContainerTypeId = qmlRegisterType<GActionContainer>();
static int actionManagerTypeId = qmlRegisterType<GActionManager>();
static int editorManagerTypeId = qmlRegisterType<GEditorManager>();

#define REG_O_FACTORY(ClassName) \
    m_factories.insert(#ClassName, [](QObject* parent)->QObject* { \
        return new ClassName(parent); \
    })

#define REG_W_FACTORY(ClassName) \
    m_factories.insert(#ClassName, [](QObject* parent)->QObject* { \
        return new ClassName(qobject_cast<QWidget*>(parent)); \
    })

GalaJSPlugin::GalaJSPlugin(QObject* parent)
    : QObject(parent),
      m_order(0),
      m_isDisabled(false),
      m_trace(false),
      m_debugIndent(0)
{
    // register creatable objects
    REG_O_FACTORY(QAction);

    REG_W_FACTORY(QWidget);
    REG_W_FACTORY(QPushButton);
    REG_W_FACTORY(QCheckBox);
    REG_W_FACTORY(QRadioButton);
    REG_W_FACTORY(QToolButton);
    REG_W_FACTORY(QLabel);
    REG_W_FACTORY(QLineEdit);
    REG_W_FACTORY(QMessageBox);
    REG_W_FACTORY(QToolBar);
    REG_W_FACTORY(QToolButton);
}

GalaJSPlugin::~GalaJSPlugin()
{
    // first of all delete JS engine
    m_jsEngine.reset();
}

class GalaAPIException
{
public:
    GalaAPIException(QString error)
        : error(error)
    {}

    QString error;
};

bool GalaJSPlugin::loadPlugin(QString pluginPath, QString* errorString)
{
    if (!m_jsEngine.isNull())
    {
        *errorString = "GalaPlugin is initialized already.";
        return false;
    }

    try
    {
        // save plugin path and name
        m_pluginPath = pluginPath;
        m_pluginName = QFileInfo(m_pluginPath).fileName();

        // open plugin file
        QFile scriptFile(pluginPath);
        if (!scriptFile.open(QIODevice::ReadOnly))
        {
            throw GalaAPIException("Cannot open file.");
        }

        // read script
        QTextStream stream(&scriptFile);
        QString contents = stream.readAll();
        scriptFile.close();

        // prepare java script engine
        m_jsEngine.reset(new QJSEngine());
        installJsContext(m_jsEngine.data());

        // load script
        QJSValue res = m_jsEngine->evaluate(contents, pluginPath);
        if (res.isError())
        {
            throw GalaAPIException(QString("Script error: '%1'.").arg(res.toString()));
        }

        // try to find "galaPluginDisable" variable
        res = m_jsEngine->evaluate("galaPluginDisable");
        if (res.isBool())
        {
            m_isDisabled = res.toBool();
        }

        // try to find "galaPluginOrder" variable
        res = m_jsEngine->evaluate("galaPluginOrder");
        if (res.isNumber())
        {
            m_order = res.toInt();
        }

        // try to find "galaPluginTrace" variable
        res = m_jsEngine->evaluate("galaPluginTrace");
        if (res.isBool())
        {
            m_trace = res.toBool();
        }
    }
    catch (const GalaAPIException& exception)
    {
        *errorString = QString("%1\nScript file: '%2'").arg(exception.error, pluginPath);
        m_jsEngine.reset();
        return false;
    }
    catch (...)
    {
        *errorString = QString("Unhandled exception in plugin '%1'.").arg(pluginPath);
        m_jsEngine.reset();
        return false;
    }

    return true;
}

void GalaJSPlugin::installJsContext(QJSEngine* jsEngine)
{
    QJSValue globalObject = jsEngine->globalObject();

    GContext gContext;
    gContext.plugin = this;
    gContext.jsEngine = jsEngine;

    // setup objects to js context
    GCore* c(new GCore(gContext));
    globalObject.setProperty("core", jsEngine->newQObject(c));

    GMessageManager* msm(new GMessageManager(gContext));
    globalObject.setProperty("messageManager", jsEngine->newQObject(msm));

    GActionManager* am(new GActionManager(gContext));
    globalObject.setProperty("actionManager", jsEngine->newQObject(am));

    GEditorManager* em(new GEditorManager(gContext));
    globalObject.setProperty("editorManager", jsEngine->newQObject(em));

    GModeManager* mm(new GModeManager(gContext));
    globalObject.setProperty("modeManager", jsEngine->newQObject(mm));

    globalObject.setProperty("galaAPI", jsEngine->toScriptValue(static_cast<QObject*>(this)));
}

void GalaJSPlugin::installQmlContext(QQmlEngine* qmlEngine)
{
    QQmlContext* context = qmlEngine->rootContext();

    GContext gContext;
    gContext.plugin = this;
    gContext.jsEngine = qmlEngine;

    // setup objects to qml context
    QObject* c(new GCore(gContext));
    context->setContextProperty("core", c);

    QObject* msm(new GMessageManager(gContext));
    context->setContextProperty("messageManager", msm);

    QObject* am(new GActionManager(gContext));
    context->setContextProperty("actionManager", am);

    QObject* em(new GEditorManager(gContext));
    context->setContextProperty("editorManager", em);

    QObject* mm(new GModeManager(gContext));
    context->setContextProperty("modeManager", mm);

    context->setContextProperty("galaAPI", static_cast<QObject*>(this));
}

void GalaJSPlugin::changeDebugIndent(qint32 delta)
{
    m_debugIndent += delta;
    Q_ASSERT(m_debugIndent >= 0);
    if (m_debugIndent < 0)
        m_debugIndent = 0;
}


QJSValue GalaJSPlugin::createQuickView(QString qmlUrl, QObject* parent)
{
    QQmlEngine* qmlEngine = new QQmlEngine(this);
    installQmlContext(qmlEngine);

    QQuickView *view = new QQuickView(qmlEngine, nullptr);
    view->setObjectName("quickView");
    QWidget *container = QWidget::createWindowContainer(view, qobject_cast<QWidget*>(parent));
    container->setFocusPolicy(Qt::TabFocus);

    // make absolute file path related to plugin dir
    QFileInfo fi(QFileInfo(m_pluginPath).absolutePath(), qmlUrl);
    if (fi.isFile())
    {
        qmlUrl = fi.absoluteFilePath();
    }

    view->setSource(QUrl(qmlUrl));

    QSize s = view->initialSize();
    container->setMinimumSize(s);

    QJSValue res = m_jsEngine->toScriptValue(container);
    res.setProperty("quickView", m_jsEngine->toScriptValue(view));
    return res;
}

QJSValue GalaJSPlugin::createQObject(QString type, QObject* parent)
{
    auto it = m_factories.find(type.toLatin1());
    if (it == m_factories.end())
        return QJSValue();

    QObject* object = it.value()(parent);

    return m_jsEngine->toScriptValue(object);
}

bool GalaJSPlugin::loadAPI(QString libFileName)
{
    QLibrary library(libFileName, this);
    if (!library.load())
    {
        debug(tr("Cannot load '%1'.").arg(libFileName));
        return false;
    }

    // library should export C function
    // extern "C" MY_EXPORT bool loadAPI(QJSEngine* jsEngine, QString* errors)
    typedef bool (*LoadAPIFunction)(QJSEngine*, QString*);

    LoadAPIFunction loadAPIFunc = (LoadAPIFunction)library.resolve("loadAPI");
    if (!loadAPIFunc)
    {
        debug(tr("Cannot resolve 'loadAPI' function."));
        library.unload();
        return false;
    }

    QString errors;
    if (!loadAPIFunc(m_jsEngine.data(), &errors))
    {
        debug(tr("'loadAPI' function failed (%1).").arg(errors));
        library.unload();
        return false;
    }

    return true;
}

bool GalaJSPlugin::enableDebug()
{
    if (!m_debugStream.isNull())
        return true;

    if (m_jsEngine.isNull())
        return false;

    QFile* file(new QFile(this));
    file->setFileName(m_pluginPath + tr(".log"));
    if (!file->open(QIODevice::WriteOnly))
    {
        delete file;
        return false;
    }

    m_debugStream.reset(new QTextStream(file));
    return true;
}

void GalaJSPlugin::debug(QString str)
{
    qDebug() << QString("%1 : %2").arg(m_pluginName, str);

    enableDebug();

    if (m_debugStream)
    {
        QString indent(m_debugIndent, QChar::Space);
        *m_debugStream << indent << str << endl;
    }
}

