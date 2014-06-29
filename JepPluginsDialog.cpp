#include "JepPluginsDialog.h"
#include "ui_JepPluginsDialog.h"

#include <QPushButton>
#include <QMessageBox>

using namespace JsExtensions::Internal;

JepPluginsDialog::JepPluginsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JepPluginsDialog)
{
    ui->setupUi(this);

    //
    // init table model
    //
    //m_model.setColumnCount(6);
    m_model.setHorizontalHeaderLabels(QStringList() << "Name" << "Enable" << "Priority" << "Trace" << "Settings" << "Description");
    m_model.horizontalHeaderItem(4)->setToolTip("Settings");

    ui->pluginsTable->setModel(&m_model);

    QHeaderView* header = ui->pluginsTable->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::Fixed);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->pluginsTable->setColumnWidth(4, header->height()*3/2);
}

JepPluginsDialog::~JepPluginsDialog()
{
}

void JepPluginsDialog::plugins2Model(const QList<JsPluginInfo>& plugins)
{
    m_plugins = plugins;
    m_model.setRowCount(m_plugins.size());

    for (int row = 0; row < m_plugins.size(); ++row)
    {
        JsPluginInfo& plugin = m_plugins[row];

        QStandardItem *item = new QStandardItem(plugin.name);
        item->setEditable(false);
        item->setToolTip(plugin.name);
        m_model.setItem(row, 0, item);

        item = new QStandardItem();
        item->setEditable(false);
        item->setCheckable(true);
        item->setCheckState(plugin.isEnabled ? Qt::Checked : Qt::Unchecked);
        m_model.setItem(row, 1, item);

        item = new QStandardItem();
        item->setData(plugin.priority, Qt::DisplayRole);
        m_model.setItem(row, 2, item);

        item = new QStandardItem();
        item->setEditable(false);
        item->setCheckable(true);
        item->setCheckState(plugin.trace ? Qt::Checked : Qt::Unchecked);
        m_model.setItem(row, 3, item);

        item = new QStandardItem(" ");
        item->setEditable(false);
        //item->setToolTip(plugin.description);
        m_model.setItem(row, 4, item);

        item = new QStandardItem(plugin.description);
        item->setEditable(false);
        item->setToolTip(plugin.description);
        m_model.setItem(row, 5, item);

        if (plugin.hasSettings()) {
            QPushButton* bttn = new QPushButton("...");
            bttn->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
            bttn->setProperty("row", row);
            QObject::connect(bttn, SIGNAL(clicked()), this, SLOT(onSettings()));
            ui->pluginsTable->setIndexWidget(m_model.index(row, 4), bttn);
        }
    }
}

void JepPluginsDialog::model2Plugins(QList<JsPluginInfo>& plugins)
{
    Q_ASSERT(plugins.size() == m_model.rowCount());

    for (int row = 0; row < plugins.size(); ++row)
    {
        JsPluginInfo& plugin = plugins[row];

        QStandardItem *item = m_model.item(row, 0);
        Q_ASSERT(plugin.name == item->text());

        item = m_model.item(row, 1);
        Q_ASSERT(item->isCheckable());
        plugin.isEnabled = (item->checkState() == Qt::Checked);

        item = m_model.item(row, 2);
        bool ok = true;
        plugin.priority = item->data(Qt::DisplayRole).toInt(&ok);
        Q_ASSERT(ok);

        item = m_model.item(row, 3);
        Q_ASSERT(item->isCheckable());
        plugin.trace = (item->checkState() == Qt::Checked);
    }
}

void JepPluginsDialog::onSettings()
{
    int row = sender()->property("row").toInt();

    QString errors;
    if (!m_plugins[row].invokeSettings(this, errors))
        QMessageBox::information(this, "Error", errors);
}
