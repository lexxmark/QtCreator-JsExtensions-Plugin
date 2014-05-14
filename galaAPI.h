#ifndef GALA_API_H
#define GALA_API_H

#include <QObject>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/imode.h>
#include <coreplugin/icore.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QJSEngine>
#include <QQmlError>
#include <QtGlobal>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextStream>

#include <QMap>
#include <functional>

class GWrapper: public QObject
{
    Q_OBJECT

protected:
    GWrapper(QJSEngine* jsEngine)
        : m_jsEngine(jsEngine)
    {

    }

    ~GWrapper() {}

    template <typename T>
    QJSValue wrapObject(T* obj) const { return m_jsEngine->toScriptValue(static_cast<QObject*>(obj)); }

    static Core::Id str2id(QString id)
    {
        const char *name = id.toLatin1().constData();
        return Core::Id(name);
    }

    static QString id2str(Core::Id id) { return QString::fromLatin1(id.name()); }

    QJSEngine* m_jsEngine;
};

class GCore: public GWrapper
{
    Q_OBJECT

public:
    GCore(QJSEngine* jsEngine)
        : GWrapper(jsEngine),
          m_owner(Core::ICore::instance())
    {
    }
    ~GCore() {}

    Core::ICore* owner1() { return m_owner; }

public slots:
    QJSValue owner() { return wrapObject(m_owner); }

/*
    static void showNewItemDialog(const QString &title,
                                  const QList<IWizard *> &wizards,
                                  const QString &defaultLocation = QString(),
                                  const QVariantMap &extraVariables = QVariantMap());
*/
    bool showOptionsDialog(QString group, QString page, QWidget *parent) { return m_owner->showOptionsDialog(str2id(group), str2id(page), parent); }
    QString msgShowOptionsDialog() { return m_owner->msgShowOptionsDialog(); }

    bool showWarningWithOptions(QString title, QString text,
                                   QString details/* = QString()*/,
                                   QString settingsCategory/* = Id()*/,
                                   QString settingsId/* = Id()*/,
                                   QWidget *parent/* = 0*/)
    {
        return m_owner->showWarningWithOptions(title, text, details, str2id(settingsCategory), str2id(settingsId), parent);
    }

    //static QSettings *settings(QSettings::Scope scope = QSettings::UserScope);

    //static SettingsDatabase *settingsDatabase();
    //static QPrinter *printer();
    QString userInterfaceLanguage() { return m_owner->userInterfaceLanguage(); }

    QString resourcePath() { return m_owner->resourcePath(); }
    QString userResourcePath() { return m_owner->userResourcePath(); }
    QString documentationPath() { return m_owner->documentationPath(); }
    QString libexecPath() { return m_owner->libexecPath(); }

    QString versionString() { return m_owner->versionString(); }
    QString buildCompatibilityString() { return m_owner->buildCompatibilityString(); }

    QJSValue mainWindow() { return m_jsEngine->toScriptValue(m_owner->mainWindow()); }
    QJSValue dialogParent() { return m_jsEngine->toScriptValue(m_owner->dialogParent()); }
    QJSValue statusBar() { return m_jsEngine->toScriptValue(m_owner->statusBar()); }
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
    void openFiles(const QStringList &fileNames, int flags) { m_owner->openFiles(fileNames, (Core::ICore::OpenFilesFlags)flags); }

    void emitNewItemsDialogRequested() { m_owner->emitNewItemsDialogRequested(); }

    void saveSettings() { m_owner->saveSettings(); }

private:
    Core::ICore* m_owner;
};


class GDocument : public GWrapper
{
    Q_OBJECT

public:
    GDocument(QJSEngine* jsEngine, Core::IDocument *owner)
        : GWrapper(jsEngine),
          m_owner(owner)
    {
    }
    ~GDocument() {}

    Core::IDocument* owner1() { return m_owner; }

public slots:
    QJSValue owner() { return wrapObject(m_owner); }

    void setId(QString id) { m_owner->setId(str2id(id)); }
    QString id() const { return id2str(m_owner->id()); }

    QJSValue save(const QString &fileName /*= QString()*/, bool autoSave/* = false*/)
    {
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

    bool setContents(const QByteArray &contents) { return m_owner->setContents(contents); }

    QString filePath() const { return m_owner->filePath(); }
    void setFilePath(const QString &filePath) { m_owner->setFilePath(filePath); }
    QString displayName() const { return m_owner->displayName(); }
    void setDisplayName(const QString &name) { m_owner->setDisplayName(name); }

    bool isFileReadOnly() const { return m_owner->isFileReadOnly(); }
    bool isTemporary() const { return m_owner->isTemporary(); }
    void setTemporary(bool temporary) { m_owner->setTemporary(temporary); }

    QString defaultPath() const { return m_owner->defaultPath(); }
    QString suggestedFileName() const { return m_owner->suggestedFileName(); }
    QString mimeType() const { return m_owner->mimeType(); }

    bool shouldAutoSave() const { return m_owner->shouldAutoSave(); }
    bool isModified() const { return m_owner->isModified(); }
    bool isSaveAsAllowed() const { return m_owner->isSaveAsAllowed(); }

    //ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    //bool reload(QString *errorString, ReloadFlag flag, ChangeType type) = 0;

    void checkPermissions() { m_owner->checkPermissions(); }

    QJSValue autoSave(const QString &filePath)
    {
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

    void setRestoredFrom(const QString &name) { m_owner->setRestoredFrom(name); }
    void removeAutoSaveFile() { m_owner->removeAutoSaveFile(); }

    bool hasWriteWarning() const { return m_owner->hasWriteWarning(); }
    void setWriteWarning(bool has) { m_owner->setWriteWarning(has); }

    //InfoBar *infoBar();

private:
    Core::IDocument* m_owner;
};

class GEditorManager : public GWrapper
{
    Q_OBJECT

public:
    GEditorManager(QJSEngine* jsEngine)
        : GWrapper(jsEngine),
          m_owner(Core::EditorManager::instance())
    {

    }
    ~GEditorManager() {}

    Core::EditorManager* owner1() { return m_owner; }

public slots:
    QJSValue owner() { return wrapObject(owner1()); }

    //EditorToolBar *createToolBar(QWidget *parent = 0);

    //QString splitLineNumber(QString *fileName);

    // returns IEditor*
    QJSValue openEditor(const QString &fileName, const QString &editorId = QString(), int flags = 0, bool *newEditor = 0)
    {
        return wrapObject(m_owner->openEditor(fileName, str2id(editorId), Core::EditorManager::OpenEditorFlags(flags), newEditor));
    }

    // returns IEditor*
    QJSValue openEditorAt(const QString &fileName,  int line, int column = 0, const QString &editorId = QString(), int flags = 0, bool *newEditor = 0)
    {
        return wrapObject(m_owner->openEditorAt(fileName, line, column, str2id(editorId), Core::EditorManager::OpenEditorFlags(flags), newEditor));
    }

    // returns IEditor*
    QJSValue openEditorWithContents(const QString &editorId, QString *titlePattern = 0, const QByteArray &contents = QByteArray(), int flags = 0)
    {
        return wrapObject(m_owner->openEditorWithContents(str2id(editorId), titlePattern, contents, Core::EditorManager::OpenEditorFlags(flags)));
    }

    bool openExternalEditor(const QString &fileName, const QString &editorId)
    {
        return m_owner->openExternalEditor(fileName, str2id(editorId));
    }

    QStringList getOpenFileNames()
    {
        return m_owner->getOpenFileNames();
    }

    QString getOpenWithEditorId(const QString &fileName, bool *isExternalEditor = 0)
    {
        return QString::fromLatin1(m_owner->getOpenWithEditorId(fileName, isExternalEditor).name());
    }

    GDocument* currentDocument()
    {
        return new GDocument(m_jsEngine, m_owner->currentDocument());
    }

    // returns IEditor*
    QJSValue currentEditor()
    {
        return wrapObject(m_owner->currentEditor());
    }

    QJSValue visibleEditors()
    {
        auto editors = m_owner->visibleEditors();
        QJSValue array = m_jsEngine->newArray(editors.size());

        quint32 i = 0;
        for (auto editor: editors)
        {
            array.setProperty(i++, m_jsEngine->toScriptValue(editor));
        }
        return array;
    }

    QJSValue documents()
    {
        QList<Core::DocumentModel::Entry *> documents = m_owner->documentModel()->documents();
        QJSValue array = m_jsEngine->newArray(documents.size());

        for (quint32 i = 0; i < (quint32)documents.size(); ++i)
        {
            array.setProperty(i, m_jsEngine->newQObject(new GDocument(m_jsEngine, documents[i]->document)));
        }

        return array;
    }

    void activateEditor(Core::IEditor *editor, int flags = 0)
    {
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
        m_owner->addCurrentPositionToNavigationHistory(editor, saveState);
    }

    void cutForwardNavigationHistory()
    {
        m_owner->cutForwardNavigationHistory();
    }

    bool saveEditor(Core::IEditor *editor)
    {
        if (!editor)
            return false;

        return m_owner->saveEditor(editor);
    }

    bool closeEditors(const QList<Core::IEditor *> &editorsToClose, bool askAboutModifiedEditors/* = true*/)
    {
        return m_owner->closeEditors(editorsToClose, askAboutModifiedEditors);
    }

    void closeEditor(Core::IEditor *editor, bool askAboutModifiedEditors/* = true*/)
    {
        m_owner->closeEditor(editor, askAboutModifiedEditors);
    }

    //MakeWritableResult makeFileWritable(IDocument *document);

    QByteArray saveState() { return m_owner->saveState(); }
    bool restoreState(const QByteArray &state) { return m_owner->restoreState(state); }

    bool hasSplitter() { return m_owner->hasSplitter(); }

    void saveSettings() { m_owner->saveSettings(); }
    void readSettings() { m_owner->readSettings(); }

    //Internal::OpenEditorsWindow *windowPopup();
    void showPopupOrSelectDocument() { m_owner->showPopupOrSelectDocument(); }

    void showEditorStatusBar(const QString &id,
                           const QString &infoText,
                           const QString &buttonText = QString(),
                           QObject *object = 0, const char *member = 0)
    {
        m_owner->showEditorStatusBar(id, infoText, buttonText, object, member);
    }

    void hideEditorStatusBar(const QString &id) { m_owner->hideEditorStatusBar(id); }

    //EditorFactoryList editorFactories(const MimeType &mimeType, bool bestMatchOnly = true);
    //ExternalEditorList externalEditors(const MimeType &mimeType, bool bestMatchOnly = true);

    //void setReloadSetting(IDocument::ReloadSetting behavior);
    //IDocument::ReloadSetting reloadSetting();

    void setAutoSaveEnabled(bool enabled) { m_owner->setAutoSaveEnabled(enabled); }
    bool autoSaveEnabled() { return m_owner->autoSaveEnabled(); }
    void setAutoSaveInterval(int interval) { m_owner->setAutoSaveInterval(interval); }
    int autoSaveInterval() { return m_owner->autoSaveInterval(); }
    bool isAutoSaveFile(const QString &fileName) { return m_owner->isAutoSaveFile(fileName); }

//    QString defaultTextCodec() { return m_owner->defaultTextCodec(); }

    qint64 maxTextFileSize() { return m_owner->maxTextFileSize(); }

    void setWindowTitleAddition(const QString &addition) { m_owner->setWindowTitleAddition(addition); }
    QString windowTitleAddition() { return m_owner->windowTitleAddition(); }

    void setWindowTitleVcsTopic(const QString &topic) { m_owner->setWindowTitleVcsTopic(topic); }
    QString windowTitleVcsTopic() { return m_owner->windowTitleVcsTopic(); }

    //void addSaveAndCloseEditorActions(QMenu *contextMenu, DocumentModel::Entry *entry);
    //void addNativeDirActions(QMenu *contextMenu, DocumentModel::Entry *entry);

    bool closeAllEditors(bool askAboutModifiedEditors/* = true*/) { return m_owner->closeAllEditors(askAboutModifiedEditors); }
    void closeAllEditorsExceptVisible() { m_owner->closeAllEditorsExceptVisible(); }

    bool saveDocument(GDocument *documentParam) { return m_owner->saveDocument(documentParam->owner1()); }
    bool saveDocumentAs(GDocument *documentParam) { return m_owner->saveDocumentAs(documentParam->owner1()); }
    void revertToSaved() { m_owner->revertToSaved(); }
    void revertToSaved2(GDocument *document) { m_owner->revertToSaved(document->owner1()); }
    void closeEditor() { m_owner->closeEditor(); }
    void closeOtherEditors() { m_owner->closeOtherEditors(); }
    void doEscapeKeyFocusMoveMagic() { m_owner->doEscapeKeyFocusMoveMagic(); }

private:
    Core::EditorManager* m_owner;
};

class GModeManager : public GWrapper
{
    Q_OBJECT

public:
    GModeManager(QJSEngine* jsEngine)
        : GWrapper(jsEngine),
          m_owner(qobject_cast<Core::ModeManager*>(Core::ModeManager::instance()))
    {
    }
    ~GModeManager() {}

    Core::ModeManager* owner1() { return m_owner; }

public slots:
    QJSValue owner() { return m_jsEngine->toScriptValue(m_owner); }

    QJSValue currentMode() { return m_jsEngine->toScriptValue(m_owner->currentMode()); }
    QJSValue mode(QString id) { return m_jsEngine->toScriptValue(m_owner->mode(str2id(id))); }

    void addAction(QAction *action, int priority) { m_owner->addAction(action, priority); }
    void addProjectSelector(QAction *action) { m_owner->addProjectSelector(action); }
    void addWidget(QWidget *widget) { m_owner->addWidget(widget); }

    void activateMode(QString id) { m_owner->activateMode(str2id(id)); }
    void setFocusToCurrentMode() { m_owner->setFocusToCurrentMode(); }
    bool isModeSelectorVisible() { return m_owner->isModeSelectorVisible(); }

    void setModeSelectorVisible(bool visible) { m_owner->setModeSelectorVisible(visible); }

private:
    Core::ModeManager* m_owner;
};

class GCommand : public GWrapper
{
    Q_OBJECT
public:
    GCommand(Core::Command* owner, QJSEngine* jsEngine)
        : GWrapper(jsEngine),
          m_owner(owner)
    {
    }
    ~GCommand() {}

    Core::Command* owner1() { return m_owner; }

public slots:

    QJSValue owner() { return m_jsEngine->toScriptValue(m_owner); }

    void setDefaultKeySequence(const QString &key) { m_owner->setDefaultKeySequence(key); }
    QString defaultKeySequence() const { return m_owner->defaultKeySequence(); }
    QString keySequence() const { return m_owner->keySequence(); }
    void setDescription(const QString &text) { m_owner->setDescription(text); }
    QString description() const { return m_owner->description(); }

    QString id() const { return id2str(m_owner->id()); }

    QAction *action() const { return m_owner->action(); }
    QString context() const
    {
        Core::Context ctx = m_owner->context();
        return ctx.isEmpty() ? QString() : QString::fromLatin1(ctx.at(0).name());
    }

    void setAttribute(int attr) { m_owner->setAttribute((Core::Command::CommandAttribute)attr); }
    void removeAttribute(int attr) { m_owner->removeAttribute((Core::Command::CommandAttribute)attr); }
    bool hasAttribute(int attr) const { return m_owner->hasAttribute((Core::Command::CommandAttribute)attr); }

    bool isActive() const { return m_owner->isActive(); }

    void setKeySequence(const QString &key) { m_owner->setKeySequence(key); }
    QString stringWithAppendedShortcut(const QString &str) const { return m_owner->stringWithAppendedShortcut(str); }

    bool isScriptable1() const { return m_owner->isScriptable(); }
    bool isScriptable2(const QString & context) const { return m_owner->isScriptable(Core::Context(str2id(context))); }

private:
    Core::Command* m_owner;
};


class GActionContainer : public GWrapper
{
    Q_OBJECT

public:
    GActionContainer(Core::ActionContainer *owner, QJSEngine* jsEngine)
    : GWrapper(jsEngine),
      m_owner(owner)
    {
    }
    ~GActionContainer() {}

public slots:
    void setOnAllDisabledBehavior(int behavior) { m_owner->setOnAllDisabledBehavior(Core::ActionContainer::OnAllDisabledBehavior(behavior)); }
    int onAllDisabledBehavior() const { return m_owner->onAllDisabledBehavior(); }

    QString id() const { return id2str(m_owner->id()); }

    QJSValue menu() const { return wrapObject(m_owner->menu()); }
    QJSValue menuBar() const { return wrapObject(m_owner->menuBar()); }

    // returns QAction*
    QJSValue insertLocation(QString group) const { return wrapObject(m_owner->insertLocation(str2id(group))); }
    void appendGroup(QString group) { m_owner->appendGroup(str2id(group)); }
    void insertGroup(QString before, QString group) { m_owner->insertGroup(str2id(before), str2id(group)); }
    void addAction(GCommand *action, QString group/* = QString()*/) { m_owner->addAction(action->owner1(), str2id(group)); }
    void addMenu1(GActionContainer *menu, QString group/* = QString()*/) { m_owner->addMenu(menu->m_owner, str2id(group)); }
    void addMenu2(GActionContainer *before, GActionContainer *menu, QString group/* = QString()*/) { m_owner->addMenu(before->m_owner, menu->m_owner, str2id(group)); }
    GCommand *addSeparator(const QString &context, QString group/* = QString()*/, QAction **outSeparator/* = 0*/) { return new GCommand(m_owner->addSeparator(Core::Context(str2id(context)), str2id(group), outSeparator), m_jsEngine); }

    void clear() { m_owner->clear(); }

private:
    Core::ActionContainer *m_owner;
};

class GActionManager : public GWrapper
{
    Q_OBJECT

public:
    explicit GActionManager(QJSEngine* jsEngine)
        : GWrapper(jsEngine),
          m_owner(Core::ActionManager::instance())
    {
    }
    ~GActionManager() {}

public slots:
    GActionContainer *createMenu(QString id) { return new GActionContainer(m_owner->createMenu(str2id(id)), m_jsEngine); }
    GActionContainer *createMenuBar(QString id) { return new GActionContainer(m_owner->createMenuBar(str2id(id)), m_jsEngine); }

    GCommand *registerAction(QAction *action, QString id, const QString &context, bool scriptable/* = false*/)
    {
        Core::Context ctx(str2id(context));
        return new GCommand(m_owner->registerAction(action, str2id(id), ctx, scriptable), m_jsEngine);
    }

    GCommand *command(QString id) { return new GCommand(m_owner->command(str2id(id)), m_jsEngine); }
    GActionContainer *actionContainer(QString id) { return new GActionContainer(m_owner->actionContainer(str2id(id)), m_jsEngine); }

    QJSValue commands()
    {
        auto commands = m_owner->commands();
        QJSValue array = m_jsEngine->newArray(commands.size());

        quint32 i = 0;
        for (auto command: commands)
        {
            array.setProperty(i++, m_jsEngine->newQObject(new GCommand(command, m_jsEngine)));
        }
        return array;
    }

    void unregisterAction(QAction *action, QString id) { m_owner->unregisterAction(action, str2id(id)); }

    void setPresentationModeEnabled(bool enabled) { m_owner->setPresentationModeEnabled(enabled); }
    bool isPresentationModeEnabled() { return m_owner->isPresentationModeEnabled(); }

private:
    Core::ActionManager* m_owner;
};

class GalaJSPlugin: public QObject
{
    Q_OBJECT

public:
    explicit GalaJSPlugin(QObject* parent = nullptr);
    ~GalaJSPlugin();

    quint32 order() const { return m_order; }
    QJSEngine* jsEngine() { return m_jsEngine.data(); }
    bool loadPlugin(QString pluginPath, QString* errorString);

    void installAPI(QJSEngine* jsEngine);

public slots:
    bool loadAPI(QString libFileName);
    bool enableDebug();
    void debug(QString str);

    QJSValue createQuickView(QString qmlUrl, QObject* parent);
    QJSValue createQObject(QString type, QObject* parent);

    QJSValue point(int x, int y) { return m_jsEngine->toScriptValue(QPoint(x, y)); }
    QJSValue rect(int x, int y, int w, int h) { return m_jsEngine->toScriptValue(QRect(x, y, w, h)); }
    QJSValue size(int w, int h) { return m_jsEngine->toScriptValue(QSize(w, h)); }
    QJSValue sizePolicy(int hor, int ver, int type) { return m_jsEngine->toScriptValue(QSizePolicy(QSizePolicy::Policy(hor), QSizePolicy::Policy(ver), QSizePolicy::ControlType(type))); }

private:
    QScopedPointer<QJSEngine> m_jsEngine;
    QMap<QByteArray, std::function<QObject*(QObject*)>> m_factories;

    qint32 m_order;
    QString m_pluginPath;
    QScopedPointer<QTextStream> m_debugStream;
};

#endif // GALA_API_H

