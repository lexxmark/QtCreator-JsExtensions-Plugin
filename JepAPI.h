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

class JsPlugin: public QObject
{
    Q_OBJECT

public:
    explicit JsPlugin(QObject* parent = nullptr);
    ~JsPlugin();

    qint32 order() const { return m_order; }
    bool isDisabled() const { return m_isDisabled; }
    bool trace() const { return m_trace; }

    QJSEngine* jsEngine() { return m_jsEngine.data(); }
    bool loadPlugin(QString pluginPath, QString* errorString);

    void installJsContext(QJSEngine* jsEngine);
    void installQmlContext(QQmlEngine* qmlEngine);

    void changeDebugIndent(qint32 delta);
    QPair<QWidget*, QQuickView*> createQuickViewWidget(QString qmlUrl, QObject* parent);

public slots:
    bool loadAPI(QString libFileName);
    void debug(QString str);

    QJSValue createQuickView(QString qmlUrl, QObject* parent);
    QJSValue createQObject(QString type, QObject* parent);

    QJSValue point(int x, int y) { return m_jsEngine->toScriptValue(QPoint(x, y)); }
    QJSValue rect(int x, int y, int w, int h) { return m_jsEngine->toScriptValue(QRect(x, y, w, h)); }
    QJSValue size(int w, int h) { return m_jsEngine->toScriptValue(QSize(w, h)); }
    QJSValue sizePolicy(int hor, int ver, int type) { return m_jsEngine->toScriptValue(QSizePolicy(QSizePolicy::Policy(hor), QSizePolicy::Policy(ver), QSizePolicy::ControlType(type))); }

    bool registerNavigationWidgetFactory(QJSValue factory, QString displayName, int priority, QString id, QString activationSequence);

    void dumpPluginManagerObjects();
    void dumpCommands();

private:
    bool enableDebug();

    QScopedPointer<QJSEngine> m_jsEngine;
    QMap<QByteArray, std::function<QObject*(QObject*)>> m_factories;

    // attributes
    qint32 m_order;
    bool m_isDisabled;
    bool m_trace;

    // plugin info
    QString m_pluginPath;
    QString m_pluginName;

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
            if (plugin->trace())
            {
                record = QString("%1 : %2 : %3").arg(file).arg(line).arg(func);

                plugin->debug(QString("Start : ") + record);
                plugin->changeDebugIndent(+4);
            }
        }

        ~TraceRecord()
        {
            if (plugin->trace())
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

/*
    static void showNewItemDialog(const QString &title,
                                  const QList<IWizard *> &wizards,
                                  const QString &defaultLocation = QString(),
                                  const QVariantMap &extraVariables = QVariantMap());
*/
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

    //static QSettings *settings(QSettings::Scope scope = QSettings::UserScope);

    //static SettingsDatabase *settingsDatabase();
    //static QPrinter *printer();
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
    /* Raises and activates the window for the widget. This contains workarounds for X11. */
    //static void raiseWindow(QWidget *widget);

    //static IContext *currentContextObject();
    // Adds and removes additional active contexts, these contexts are appended
    // to the currently active contexts.
    //static void updateAdditionalContexts(const Context &remove, const Context &add);
    //static void addContextObject(IContext *context);
    //static void removeContextObject(IContext *context);
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

    void saveSettings() { G_TRACE; m_owner->saveSettings(); }

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


class GDocument : public GWrapper
{
    Q_OBJECT

public:
    GDocument(GContext gContext, Core::IDocument *owner)
        : GWrapper(gContext),
          m_owner(owner)
    {
        G_TRACE;
        Q_ASSERT(m_owner);
    }
    ~GDocument() { G_TRACE; }

    Core::IDocument* owner() { return m_owner; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

//    void setId(QString id) { G_TRACE; m_owner->setId(str2id(id)); }
//    QString id() const { G_TRACE; return id2str(m_owner->id()); }

    QJSValue save(const QString &fileName /*= QString()*/, bool autoSave/* = false*/)
    {
        G_TRACE;
        QString errors;
        if (m_owner->save(&errors, fileName, autoSave))
        {
            return QJSValue(true);
        }
        else
        {
            qDebug("%s", errors.toLatin1().data());
            return QJSValue(false);
        }
    }

    bool setContents(const QByteArray &contents) { G_TRACE; return m_owner->setContents(contents); }

    QString filePath() const { G_TRACE; return m_owner->filePath(); }
    void setFilePath(const QString &filePath) { G_TRACE; m_owner->setFilePath(filePath); }
    QString displayName() const { G_TRACE; return m_owner->displayName(); }
    void setDisplayName(const QString &name) { G_TRACE; m_owner->setDisplayName(name); }

    bool isFileReadOnly() const { G_TRACE; return m_owner->isFileReadOnly(); }
    bool isTemporary() const { G_TRACE; return m_owner->isTemporary(); }
    void setTemporary(bool temporary) { G_TRACE; m_owner->setTemporary(temporary); }

    QString defaultPath() const { G_TRACE; return m_owner->defaultPath(); }
    QString suggestedFileName() const { G_TRACE; return m_owner->suggestedFileName(); }
    QString mimeType() const { G_TRACE; return m_owner->mimeType(); }

    bool shouldAutoSave() const { G_TRACE; return m_owner->shouldAutoSave(); }
    bool isModified() const { G_TRACE; return m_owner->isModified(); }
    bool isSaveAsAllowed() const { G_TRACE; return m_owner->isSaveAsAllowed(); }

    //ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    //bool reload(QString *errorString, ReloadFlag flag, ChangeType type) = 0;

    void checkPermissions() { G_TRACE; m_owner->checkPermissions(); }

    QJSValue autoSave(const QString &filePath)
    {
        G_TRACE;
        QString errors;
        if (m_owner->autoSave(&errors, filePath))
        {
            return QJSValue(true);
        }
        else
        {
            qDebug("%s", errors.toLatin1().data());
            return QJSValue(false);
        }
    }

    void setRestoredFrom(const QString &name) { G_TRACE; m_owner->setRestoredFrom(name); }
    void removeAutoSaveFile() { G_TRACE; m_owner->removeAutoSaveFile(); }

    bool hasWriteWarning() const { G_TRACE; return m_owner->hasWriteWarning(); }
    void setWriteWarning(bool has) { G_TRACE; m_owner->setWriteWarning(has); }

    //InfoBar *infoBar();

private:
    Core::IDocument* m_owner;
};

class GEditorManager : public GWrapper
{
    Q_OBJECT

public:
    GEditorManager(GContext gContext)
        : GWrapper(gContext),
          m_owner(Core::EditorManager::instance())
    {
        Q_ASSERT(m_owner);
        G_TRACE;
    }
    ~GEditorManager() { G_TRACE; }

    Core::EditorManager* owner() { return m_owner; }

public slots:

    QJSValue gOwner() { G_TRACE; return wrapObject(m_owner); }

    //EditorToolBar *createToolBar(QWidget *parent = 0);

    //QString splitLineNumber(QString *fileName);

    // returns IEditor*
    QJSValue openEditor(const QString &fileName, const QString &editorId = QString(), int flags = 0, bool *newEditor = 0)
    {
        G_TRACE;
        return wrapObject(m_owner->openEditor(fileName, str2id(editorId), Core::EditorManager::OpenEditorFlags(flags), newEditor));
    }

    // returns IEditor*
    QJSValue openEditorAt(const QString &fileName,  int line, int column = 0, const QString &editorId = QString(), int flags = 0, bool *newEditor = 0)
    {
        G_TRACE;
        return wrapObject(m_owner->openEditorAt(fileName, line, column, str2id(editorId), Core::EditorManager::OpenEditorFlags(flags), newEditor));
    }

    // returns IEditor*
    QJSValue openEditorWithContents(const QString &editorId, QString *titlePattern = 0, const QByteArray &contents = QByteArray(), int flags = 0)
    {
        G_TRACE;
        return wrapObject(m_owner->openEditorWithContents(str2id(editorId), titlePattern, contents, Core::EditorManager::OpenEditorFlags(flags)));
    }

    bool openExternalEditor(const QString &fileName, const QString &editorId)
    {
        G_TRACE;
        return m_owner->openExternalEditor(fileName, str2id(editorId));
    }

    QStringList getOpenFileNames()
    {
        G_TRACE;
        return m_owner->getOpenFileNames();
    }

    QString getOpenWithEditorId(const QString &fileName, bool *isExternalEditor = 0)
    {
        G_TRACE;
        return QString::fromLatin1(m_owner->getOpenWithEditorId(fileName, isExternalEditor).name());
    }

    GDocument* currentDocument()
    {
        G_TRACE;
        return new GDocument(m_gContext, m_owner->currentDocument());
    }

    // returns IEditor*
    QJSValue currentEditor()
    {
        G_TRACE;
        return wrapObject(m_owner->currentEditor());
    }

    QJSValue visibleEditors()
    {
        G_TRACE;
        auto editors = m_owner->visibleEditors();
        QJSValue array = m_gContext.jsEngine->newArray(editors.size());

        quint32 i = 0;
        for (auto editor: editors)
        {
            array.setProperty(i++, m_gContext.jsEngine->toScriptValue(editor));
        }
        return array;
    }

    QJSValue documents()
    {
        G_TRACE;
        QList<Core::IDocument *> documents = m_owner->documentModel()->openedDocuments();

        QJSValue array = m_gContext.jsEngine->newArray(documents.size());

        for (quint32 i = 0; i < (quint32)documents.size(); ++i)
        {
            array.setProperty(i, m_gContext.jsEngine->newQObject(new GDocument(m_gContext, documents[i])));
        }

        return array;
    }

    void activateEditor(Core::IEditor *editor, int flags = 0)
    {
        G_TRACE;
        m_owner->activateEditor(editor, Core::EditorManager::OpenEditorFlags(flags));
    }

    //void activateEditorForEntry(DocumentModel::Entry *entry, OpenEditorFlags flags = 0);
    //IEditor *activateEditorForDocument(IDocument *document, OpenEditorFlags flags = 0);
    //IEditor *activateEditorForDocument(Internal::EditorView *view, IDocument *document, OpenEditorFlags flags = 0);

    //DocumentModel *documentModel();
    //bool closeDocuments(const QList<Core::IDocument *> &documents, bool askAboutModifiedEditors = true);
    //void closeEditor(DocumentModel::Entry *entry);
    //void closeOtherEditors(Core::IDocument *document);

    void addCurrentPositionToNavigationHistory(Core::IEditor *editor/* = 0*/, const QByteArray &saveState/* = QByteArray()*/)
    {
        G_TRACE;
        m_owner->addCurrentPositionToNavigationHistory(editor, saveState);
    }

    void cutForwardNavigationHistory()
    {
        G_TRACE;
        m_owner->cutForwardNavigationHistory();
    }

    bool saveEditor(Core::IEditor *editor)
    {
        G_TRACE;
        if (!editor)
            return false;

        return m_owner->saveEditor(editor);
    }

    bool closeEditors(const QList<Core::IEditor *> &editorsToClose, bool askAboutModifiedEditors/* = true*/)
    {
        G_TRACE;
        return m_owner->closeEditors(editorsToClose, askAboutModifiedEditors);
    }

    void closeEditor(Core::IEditor *editor, bool askAboutModifiedEditors/* = true*/)
    {
        G_TRACE;
        m_owner->closeEditor(editor, askAboutModifiedEditors);
    }

    //MakeWritableResult makeFileWritable(IDocument *document);

    QByteArray saveState() { G_TRACE; return m_owner->saveState(); }
    bool restoreState(const QByteArray &state) { G_TRACE; return m_owner->restoreState(state); }

    bool hasSplitter() { G_TRACE; return m_owner->hasSplitter(); }

    void saveSettings() { G_TRACE; m_owner->saveSettings(); }
    void readSettings() { G_TRACE; m_owner->readSettings(); }

    //Internal::OpenEditorsWindow *windowPopup();
    void showPopupOrSelectDocument() { G_TRACE; m_owner->showPopupOrSelectDocument(); }

    void showEditorStatusBar(const QString &id,
                           const QString &infoText,
                           const QString &buttonText = QString(),
                           QObject *object = 0, const char *member = 0)
    {
        G_TRACE;
        m_owner->showEditorStatusBar(id, infoText, buttonText, object, member);
    }

    void hideEditorStatusBar(const QString &id) { G_TRACE; m_owner->hideEditorStatusBar(id); }

    //EditorFactoryList editorFactories(const MimeType &mimeType, bool bestMatchOnly = true);
    //ExternalEditorList externalEditors(const MimeType &mimeType, bool bestMatchOnly = true);

    //void setReloadSetting(IDocument::ReloadSetting behavior);
    //IDocument::ReloadSetting reloadSetting();

    void setAutoSaveEnabled(bool enabled) { G_TRACE; m_owner->setAutoSaveEnabled(enabled); }
    bool autoSaveEnabled() { G_TRACE; return m_owner->autoSaveEnabled(); }
    void setAutoSaveInterval(int interval) { G_TRACE; m_owner->setAutoSaveInterval(interval); }
    int autoSaveInterval() { G_TRACE; return m_owner->autoSaveInterval(); }
    bool isAutoSaveFile(const QString &fileName) { G_TRACE; return m_owner->isAutoSaveFile(fileName); }

//    QString defaultTextCodec() { return m_owner->defaultTextCodec(); }

    qint64 maxTextFileSize() { G_TRACE; return m_owner->maxTextFileSize(); }

    void setWindowTitleAddition(const QString &addition) { G_TRACE; m_owner->setWindowTitleAddition(addition); }
    QString windowTitleAddition() { G_TRACE; return m_owner->windowTitleAddition(); }

    void setWindowTitleVcsTopic(const QString &topic) { G_TRACE; m_owner->setWindowTitleVcsTopic(topic); }
    QString windowTitleVcsTopic() { G_TRACE; return m_owner->windowTitleVcsTopic(); }

    //void addSaveAndCloseEditorActions(QMenu *contextMenu, DocumentModel::Entry *entry);
    //void addNativeDirActions(QMenu *contextMenu, DocumentModel::Entry *entry);

    bool closeAllEditors(bool askAboutModifiedEditors/* = true*/) { G_TRACE; return m_owner->closeAllEditors(askAboutModifiedEditors); }
    void closeAllEditorsExceptVisible() { G_TRACE; m_owner->closeAllEditorsExceptVisible(); }

    bool saveDocument(GDocument *documentParam) { G_TRACE; return m_owner->saveDocument(documentParam->owner()); }
    bool saveDocumentAs(GDocument *documentParam) { G_TRACE; return m_owner->saveDocumentAs(documentParam->owner()); }
    void revertToSaved() { G_TRACE; m_owner->revertToSaved(); }
    void revertToSaved2(GDocument *document) { G_TRACE; m_owner->revertToSaved(document->owner()); }
    void closeEditor() { G_TRACE; m_owner->closeEditor(); }
    void closeOtherEditors() { G_TRACE; m_owner->closeOtherEditors(); }
    void doEscapeKeyFocusMoveMagic() { G_TRACE; m_owner->doEscapeKeyFocusMoveMagic(); }

private:
    Core::EditorManager* m_owner;
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
    void addAction(GCommand *action, QString group/* = QString()*/) { G_TRACE; m_owner->addAction(action->owner(), str2id(group)); }
    void addMenu1(GActionContainer *menu, QString group/* = QString()*/) { G_TRACE; m_owner->addMenu(menu->m_owner, str2id(group)); }
    void addMenu2(GActionContainer *before, GActionContainer *menu, QString group/* = QString()*/) { G_TRACE; m_owner->addMenu(before->m_owner, menu->m_owner, str2id(group)); }
    GCommand *addSeparator(const QString &context, QString group/* = QString()*/, QAction **outSeparator/* = 0*/) { G_TRACE; return new GCommand(m_gContext, m_owner->addSeparator(Core::Context(str2id(context)), str2id(group), outSeparator)); }

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

    GActionContainer *createMenu(QString id) { G_TRACE; return new GActionContainer(m_gContext, m_owner->createMenu(str2id(id))); }
    GActionContainer *createMenuBar(QString id) { G_TRACE; return new GActionContainer(m_gContext, m_owner->createMenuBar(str2id(id))); }

    GCommand *registerAction(QAction *action, QString id, const QString &context, bool scriptable/* = false*/)
    {
        G_TRACE;
        Core::Context ctx(str2id(context));
        return new GCommand(m_gContext, m_owner->registerAction(action, str2id(id), ctx, scriptable));
    }

    GCommand *command(QString id) { G_TRACE; return new GCommand(m_gContext, m_owner->command(str2id(id))); }
    GActionContainer *actionContainer(QString id) { G_TRACE; return new GActionContainer(m_gContext, m_owner->actionContainer(str2id(id))); }

    QJSValue commands()
    {
        G_TRACE;
        auto commands = m_owner->commands();
        QJSValue array = m_gContext.jsEngine->newArray(commands.size());

        quint32 i = 0;
        for (auto command: commands)
        {
            array.setProperty(i++, m_gContext.jsEngine->newQObject(new GCommand(m_gContext, command)));
        }
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

    //void saveSettings(int position, QWidget *widget);
    //void restoreSettings(int position, QWidget *widget);

private:
    JsPlugin* m_owner;
    QJSValue m_factory;

    QString m_displayName;
    int m_priority;
    QString m_id;
    QString m_activationSequence;
};


#endif // JEP_API_H

