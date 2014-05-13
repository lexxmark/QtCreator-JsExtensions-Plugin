#include "galaAPI.h"
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QtQml>
#include <QQuickView>

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
    : QObject(parent)
{
    // register creatable objects
    REG_O_FACTORY(QAction);
    REG_W_FACTORY(QWidget);
    REG_W_FACTORY(QPushButton);
}

GalaJSPlugin::~GalaJSPlugin()
{

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
        *errorString = tr("GalaPlugin is initialized already.");
        return false;
    }

    try
    {
        // save plugin path
        m_pluginPath = pluginPath;

        // open plugin file
        QFile scriptFile(pluginPath);
        if (!scriptFile.open(QIODevice::ReadOnly))
        {
            throw GalaAPIException(tr("Cannot open file."));
        }

        // read script
        QTextStream stream(&scriptFile);
        QString contents = stream.readAll();
        scriptFile.close();

        // prepare java script engine
        m_jsEngine.reset(new QJSEngine());
        installAPI(m_jsEngine.data());

        // load script
        QJSValue res = m_jsEngine->evaluate(contents, pluginPath);
        if (res.isError())
        {
            throw GalaAPIException(tr("Script error: '%1'.").arg(res.toString()));
        }

        // try to find "initialize" function
        res = m_jsEngine->evaluate(QString::fromLatin1("initialize"));
        if (res.isError())
        {
            throw GalaAPIException(tr("Cannot find initialize function: '%1'.").arg(res.toString()));
        }

        // check "initilize" is a function
        if (!res.isCallable())
        {
            throw GalaAPIException(tr("'initialize' is not a function."));
        }

        // invoke "initialize" function and check result
        res = res.call();
        if (res.isError())
        {
            throw GalaAPIException(tr("Initialize function error: '%1'.").arg(res.toString()));
        }
    }
    catch (const GalaAPIException& exception)
    {
        *errorString = tr("%1\nScript file: '%2'").arg(exception.error, pluginPath);
        m_jsEngine.reset();
        return false;
    }
    catch (...)
    {
        *errorString = tr("Unhandled exception in plugin '%1'.").arg(pluginPath);
        m_jsEngine.reset();
        return false;
    }

    return true;
}

void GalaJSPlugin::installAPI(QJSEngine* jsEngine)
{
    QJSValue globalObject = jsEngine->globalObject();

    // setup objects to js context
    GActionManager* am(new GActionManager(jsEngine));
    globalObject.setProperty(QString::fromLatin1("actionManager"), jsEngine->newQObject(am));

    GEditorManager* em(new GEditorManager(jsEngine));
    globalObject.setProperty(QString::fromLatin1("editorManager"), jsEngine->newQObject(em));

    GModeManager* mm(new GModeManager(jsEngine));
    globalObject.setProperty(QString::fromLatin1("modeManager"), jsEngine->newQObject(mm));

    globalObject.setProperty(QString::fromLatin1("galaAPI"), jsEngine->toScriptValue(static_cast<QObject*>(this)));
}

QJSValue GalaJSPlugin::createQuickView(QString qmlUrl, QObject* parent)
{
    QQuickView *view = new QQuickView();
    view->setObjectName(QString::fromLatin1("quickView"));
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
    container->setGeometry(0, 0, s.width(), s.height());

    QJSValue res = m_jsEngine->toScriptValue(container);
    res.setProperty(QString::fromLatin1("quickView"), m_jsEngine->toScriptValue(view));
    return res;
}

QJSValue GalaJSPlugin::createQObject(QString type, QObject* parent)
{
    auto it = m_factories.find(type.toLatin1());
    if (it == m_factories.end())
        return QJSValue();

    return m_jsEngine->toScriptValue(it.value()(parent));
}

bool GalaJSPlugin::loadAPI(QString libFileName)
{
    return false;
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
    qDebug() << str;
    if (m_debugStream)
    {
        *m_debugStream << str << '\n';
    }
}

