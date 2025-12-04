#include "MainWindow.hpp"
#include <qtabwidget.h>
#include "CodeEditor.hpp"


void MainWindow::setupUi() {

    QToolBar *toolbar = addToolBar("Main Toolbar");
    QAction *openFolderAction = toolbar->addAction("Klasör Aç");
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::onOpenFolder);

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    createLeftTab();
    createRightPanel();


    mainSplitter->addWidget(leftTabWidget);
    mainSplitter->addWidget(rightPanelStack);

    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 4);

    editor->viewport()->installEventFilter(this);
    editor->setMouseTracking(true);

    
    DebugOpenFolder();
    DebugOpenFile();
}

void MainWindow::createLeftTab() {

    // ========================================================
    // SOL PANEL: DİKEY TAB YAPISI
    // ========================================================
    leftTabWidget = new QTabWidget(this);
    
    // Sekmeleri sola (dikey) yerleştir
    leftTabWidget->setTabPosition(QTabWidget::West); 
    
    // Opsiyonel: Sekmeleri biraz genişletmek istersen (Stil)
    // leftTabWidget->setStyleSheet("QTabBar::tab { height: 100px; width: 30px; }");

    // --- TAB 1: DOSYA GEZGİNİ ---
    fileTreeView = new QTreeView(this);
    fileModel = new QFileSystemModel(this);
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    fileModel->setRootPath(""); 
    fileTreeView->setModel(fileModel);
    fileTreeView->setHeaderHidden(true);
    for (int i = 1; i < 4; ++i) fileTreeView->hideColumn(i);
    connect(fileTreeView, &QTreeView::clicked, this, &MainWindow::onFileClicked);

    // --- TAB 2: ARAMA SONUÇLARI (Placeholder) ---
    searchResultList = new QListWidget(this);
    // Şimdilik boş duracak, ileride buraya "Dosya.md:15 - Bulunan kelime" gibi itemler ekleyeceğiz.
    // Tıklandığında o satıra gitmesi için şimdiden sinyal hazırlığı yapabilirsin ama şart değil.

    // Sekmeleri Ekle
    // İstersen metin yerine icon kullanabilirsin (QIcon::fromTheme("folder") gibi)
    
    connect(searchResultList, &QListWidget::itemClicked, 
            this, &MainWindow::onSearchResultClicked);
    leftTabWidget->addTab(fileTreeView, "Dosyalar"); 
    leftTabWidget->addTab(searchResultList, "Ara");

}


void MainWindow::createRightPanel() {
    // ========================================================
    // SAĞ PANEL (Aynen Korundu)
    // ========================================================
    rightPanelStack = new QStackedWidget(this);

    // 1. Editör
    //editor = new QTextEdit(this);
    editor = new CodeEditor(this);
    QFont font("Consolas", 12);
    editor->setFont(font);
    
    // 2. Resim
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    
    imageScrollArea = new QScrollArea(this);
    imageScrollArea->setWidget(imageLabel);
    imageScrollArea->setWidgetResizable(true);

    rightPanelStack->addWidget(editor);
    rightPanelStack->addWidget(imageScrollArea);
}
