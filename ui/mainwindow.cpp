/*
  mainwindow.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config-gammaray-version.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutpluginsdialog.h"

#include "include/objecttypefilterproxymodel.h"
#include "include/toolfactory.h"

#include <network/objectbroker.h>
#include <network/modelroles.h>

#include "kde/krecursivefilterproxymodel.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <private/qguiplatformplugin_p.h> //krazy:exclude=camelcase
#else
#include <qpa/qplatformtheme.h>           //krazy:exclude=camelcase
#include <private/qguiapplication_p.h>    //krazy:exclude=camelcase
#endif

#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStringListModel>
#include <QStyleFactory>
#include <QTextCodec>
#include <QTreeView>
#include <QTextBrowser>
#include <QDialogButtonBox>

using namespace GammaRay;

static const char progName[] = PROGRAM_NAME;
static const char progVersion[] = GAMMARAY_VERSION_STRING;
static const char progDesc[] = "The Qt application inspection and manipulation tool";
static const char progURL[] = "http://www.kdab.com/gammaray";

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
  // we don't want application styles to propagate to the GammaRay window,
  // so set the platform default one.
  // unfortunately, that's not recursive by default, unless we have a style sheet set
  setStyleSheet(QLatin1String("I_DONT_EXIST {}"));

  QStyle *defaultStyle = 0;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QGuiPlatformPlugin defaultGuiPlatform;
  defaultStyle = QStyleFactory::create(defaultGuiPlatform.styleName());
#else
  foreach (const QString &styleName, QGuiApplicationPrivate::platform_theme->defaultThemeHint(QPlatformTheme::StyleNames).toStringList()) {
    if ((defaultStyle = QStyleFactory::create(styleName))) {
      break;
    }
  }
#endif
  if (defaultStyle) {
    // do not set parent of default style
    // this will cause the style being deleted too early through ~QObject()
    // other objects (e.g. the script engine debugger) still might have a
    // reference on the style during destruction
    setStyle(defaultStyle);
  }

  ui->setupUi(this);

  connect(ui->actionRetractProbe, SIGNAL(triggered(bool)), SLOT(close()));

  connect(QApplication::instance(), SIGNAL(aboutToQuit()), SLOT(close()));
  connect(ui->actionQuit, SIGNAL(triggered(bool)),
          QApplication::instance(), SLOT(quit()));
  ui->actionQuit->setIcon(QIcon::fromTheme("application-exit"));

  connect(ui->actionPlugins, SIGNAL(triggered(bool)),
          this, SLOT(aboutPlugins()));
  connect(ui->actionAboutQt, SIGNAL(triggered(bool)),
          QApplication::instance(), SLOT(aboutQt()));
  connect(ui->actionAboutGammaRay, SIGNAL(triggered(bool)), SLOT(about()));
  connect(ui->actionAboutKDAB, SIGNAL(triggered(bool)), SLOT(aboutKDAB()));

  setWindowIcon(QIcon(":gammaray/GammaRay-128x128.png"));

  QAbstractItemModel *model = ObjectBroker::model("com.kdab.GammaRay.ToolModel");
  model->setData(QModelIndex(), QVariant::fromValue<QWidget*>(this), ToolModelRole::ToolWidgetParent);
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setDynamicSortFilter(true);
  proxyModel->setSourceModel(model);
  proxyModel->sort(0);
  ui->toolSelector->setModel(proxyModel);
  ui->toolSelector->resize(ui->toolSelector->minimumSize());
  connect(ui->toolSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(toolSelected()));

  // hide unused tool bar for now
  ui->mainToolBar->setHidden(true);

  QString appName = qApp->applicationName();
  if (appName.isEmpty() && !qApp->arguments().isEmpty()) {
    appName = qApp->arguments().first().remove(qApp->applicationDirPath());
    if (appName.startsWith('.')) {
        appName = appName.right(appName.length() - 1);
    }
    if (appName.startsWith('/')) {
        appName = appName.right(appName.length() - 1);
    }
  } else {
    appName = tr("PID %1").arg(qApp->applicationPid());
  }
  setWindowTitle(tr("%1 (%2)").arg(progName).arg(appName));

  selectInitialTool();

  // get some sane size on startup
  resize(1024, 768);
}

MainWindow::~MainWindow()
{
}

void MainWindow::about()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("About %1").arg(progName));
  QVBoxLayout *lay = new QVBoxLayout;
  dialog.setLayout(lay);
  QLabel *title = new QLabel;
  QFont titleFont = dialog.font();
  titleFont.setBold(true);
  title->setFont(titleFont);
  title->setText(tr("<b>%1 %2</b><p>%3").arg(progName).arg(progVersion).arg(progDesc));

  lay->addWidget(title);

  QLabel *informativeText = new QLabel;
  informativeText->setTextInteractionFlags(Qt::TextBrowserInteraction);
  informativeText->setOpenExternalLinks(true);
  informativeText->setWordWrap(true);
  informativeText->setText(
    trUtf8("<qt>"
           "<p>Copyright (C) 2010-2013 Klarälvdalens Datakonsult AB, "
           "a KDAB Group company, <a href=\"mailto:info@kdab.com\">info@kdab.com</a></p>"
           "<p><u>Authors:</u><br>"
           "Allen Winter &lt;allen.winter@kdab.com&gt;<br>"
           "Andreas Holzammer &lt;andreas.holzammer@kdab.com&gt;<br>"
           "David Faure &lt;david.faure@kdab.com&gt;<br>"
           "Kevin Funk &lt;kevin.funk@kdab.com&gt;<br>"
           "Laurent Montel &lt;laurent.montel@kdab.com&gt;<br>"
           "Milian Wolff &lt;milian.wolff@kdab.com&gt;<br>"
           "Patrick Spendrin &lt;patrick.spendrin@kdab.com&gt;<br>"
           "Stephen Kelly &lt;stephen.kelly@kdab.com&gt;<br>"
           "Till Adam &lt;till@kdab.com&gt;<br>"
           "Thomas McGuire &lt;thomas.mcguire@kdab.com&gt;<br>"
           "Tobias Koenig &lt;tobias.koenig@kdab.com&gt;<br>"
           "Volker Krause &lt;volker.krause@kdab.com&gt;<br></p>"
           "<p>StackWalker code Copyright (c) 2005-2009, Jochen Kalmbach, All rights reserved</p>"
           "</qt>"));
  lay->addWidget(informativeText);
  QDialogButtonBox *buttonBox = new QDialogButtonBox;
  buttonBox->addButton(QDialogButtonBox::Close);
  connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(close()));
  lay->addWidget(buttonBox);

  dialog.setWindowIcon(QPixmap(":gammaray/GammaRay-128x128.png"));

  dialog.exec();
}

void MainWindow::aboutPlugins()
{
  AboutPluginsDialog dlg(this);
  dlg.setFixedSize(800, 600);
  dlg.exec();
}

void MainWindow::aboutKDAB()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("About KDAB"));
  QVBoxLayout *lay = new QVBoxLayout;
  dialog.setLayout(lay);
  QLabel *title = new QLabel;
  QFont titleFont = dialog.font();
  titleFont.setBold(true);
  title->setFont(titleFont);
  title->setText(trUtf8("Klarälvdalens Datakonsult AB (KDAB)"));

  lay->addWidget(title);

  QLabel *informativeText = new QLabel;
  informativeText->setTextInteractionFlags(Qt::TextBrowserInteraction);
  informativeText->setOpenExternalLinks(true);
  informativeText->setWordWrap(true);
  informativeText->setText(
    tr("<qt><p>%1 is supported and maintained by KDAB</p>"
       "KDAB, the Qt experts, provide consulting and mentoring for developing "
       "Qt applications from scratch and in porting from all popular and legacy "
       "frameworks to Qt. Our software products increase Qt productivity and our "
       "Qt trainers have trained 50% of commercial Qt developers globally.</p>"
       "<p>Please visit <a href='http://www.kdab.com'>http://www.kdab.com</a> "
       "to meet the people who write code like this. "
       "We also offer Qt training courses."
       "</p></qt>").arg(progName));

  lay->addWidget(informativeText);
  QDialogButtonBox *buttonBox = new QDialogButtonBox;
  buttonBox->addButton(QDialogButtonBox::Close);
  connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(close()));
  lay->addWidget(buttonBox);

  dialog.setWindowIcon(QPixmap(":gammaray/kdablogo160.png"));

  dialog.exec();
}

void MainWindow::selectInitialTool()
{
  static const QString initialTool("GammaRay::ObjectInspector");

  QAbstractItemModel *model = ui->toolSelector->model();
  QModelIndexList matches =
    model->match(model->index(0, 0), ToolModelRole::ToolId, initialTool);
  if (matches.isEmpty()) {
    return;
  }

  ui->toolSelector->setCurrentIndex(matches.first());
  toolSelected();
}

void MainWindow::toolSelected()
{
  ui->actionsMenu->clear();
  const int row = ui->toolSelector->currentIndex().row();
  if (row == -1) {
    return;
  }

  const QModelIndex mi = ui->toolSelector->model()->index(row, 0);
  QWidget *toolWidget = mi.data(ToolModelRole::ToolWidget).value<QWidget*>();
  if (!toolWidget) {
    toolWidget = createErrorPage(mi);
    ui->toolSelector->model()->setData(mi, QVariant::fromValue(toolWidget), ToolModelRole::ToolWidget);
  }

  Q_ASSERT(toolWidget);
  if (ui->toolStack->indexOf(toolWidget) < 0) { // newly created
    if (toolWidget->layout()) {
      toolWidget->layout()->setContentsMargins(11, 0, 0, 0);
    }
    ui->toolStack->addWidget(toolWidget);
  }

  ui->toolStack->setCurrentIndex(ui->toolStack->indexOf(toolWidget));

  foreach (QAction *action, toolWidget->actions()) {
    ui->actionsMenu->addAction(action);
  }
  ui->actionsMenu->setEnabled(!ui->actionsMenu->isEmpty());
}

QWidget* MainWindow::createErrorPage(const QModelIndex& index)
{
  QLabel *page = new QLabel(this);
  page->setAlignment(Qt::AlignCenter);
  // TODO show the actual plugin error message as well as any other useful information (eg. file name) we have, once the tool model has those
  page->setText(tr("Tool %1 failed to load.").arg(index.data(ToolModelRole::ToolId).toString()));
  return page;
}

#include "mainwindow.moc"
