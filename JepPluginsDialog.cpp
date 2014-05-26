#include "JepPluginsDialog.h"
#include "ui_JepPluginsDialog.h"

using namespace JsExtensions::Internal;

JepPluginsDialog::JepPluginsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JepPluginsDialog)
{
    ui->setupUi(this);

    //
    // init table model
    //
    m_model.setColumnCount(5);
    m_model.setHorizontalHeaderLabels(QStringList() << "Name" << "Enable" << "Priority" << "Trace" << "Description");

    ui->pluginsTable->setModel(&m_model);
    ui->pluginsTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->pluginsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

JepPluginsDialog::~JepPluginsDialog()
{
}

void JepPluginsDialog::plugins2Model(const QList<JsPluginInfo>& plugins)
{
    m_model.setRowCount(plugins.size());

    for (int row = 0; row < plugins.size(); ++row)
    {
        const JsPluginInfo& plugin = plugins[row];

        QStandardItem *item = new QStandardItem(plugin.name);
        item->setEditable(false);
        m_model.setItem(row, 0, item);

        item = new QStandardItem();
        item->setCheckable(true);
        item->setCheckState(plugin.isEnabled ? Qt::Checked : Qt::Unchecked);
        m_model.setItem(row, 1, item);

        item = new QStandardItem();
        item->setData(plugin.priority, Qt::DisplayRole);
        m_model.setItem(row, 2, item);

        item = new QStandardItem();
        item->setCheckable(true);
        item->setCheckState(plugin.trace ? Qt::Checked : Qt::Unchecked);
        m_model.setItem(row, 3, item);

        item = new QStandardItem(plugin.description);
        item->setEditable(false);
        m_model.setItem(row, 4, item);
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
