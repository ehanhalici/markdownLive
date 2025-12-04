#include "MainWindow.hpp"


void MarkdownHighlighter::highlightBlock(const QString &text) {
    if (!m_editor) return;

    // Editördeki imleci al
    QTextCursor cursor = m_editor->textCursor();
    // İmlecin olduğu satırın metnini al
    QString currentLineText = cursor.block().text();

    // Basit Karşılaştırma: 
    // Eğer şu an boyanan metin (text), imlecin olduğu metne (currentLineText) eşitse,
    // bu satırı "Aktif Satır" (Source Mode) olarak kabul et.
    // (Not: İçeriği birebir aynı olan iki farklı satır varsa ikisi de aktif görünür, 
    // ama başlangıç seviyesi için bu kabul edilebilir bir durumdur.)
    bool isCurrentLine = (text == currentLineText);

    // 1. ÖNCE KOD BLOKLARINI KONTROL ET
    // Eğer kod bloğu içindeysek, fonksiyon diğer regexlere bakmadan burada biter.
    if (fencedCodeBlock(text)) {
        // Ancak: Eğer aktif satırsak ve bu satır ``` ise, 
        // kullanıcının syntax'ı görmesi için rengi biraz açabiliriz (isteğe bağlı)
        return; 
    }

    // 2. BAŞLIKLAR (Header tüm satırı etkilediği için önce yapılır)
    header(text, isCurrentLine);
    
    // 3. HORIZONTAL RULE ve BLOCKQUOTE (Satır başı yapıları)
    // HR fonksiyonunu elindeki 'strike' fonksiyonunun içindeki HR kısmından ayırıp 
    // buraya eklemeni öneririm, yoksa strike() içinde kalabilir.
    quote(text, isCurrentLine);

    // 4. SATIR İÇİ ELEMANLAR (Sıralama çok fark etmez ama linkler önce gelirse iyi olur)
    tags(text, isCurrentLine);
    wikiLink(text, isCurrentLine);
    image(text, isCurrentLine);
    internalLink(text, isCurrentLine);
    externalLink(text, isCurrentLine);
    // 5. STİL ELEMANLARI
    boldRegex(text, isCurrentLine);
    italic(text, isCurrentLine);
    highlight(text, isCurrentLine); // ==Highlight==
    strike(text, isCurrentLine);    // ~~Strike~~
    math(text, isCurrentLine);      // $$Math$$
    code(text, isCurrentLine);      // `Inline Code`
    taskList(text, isCurrentLine);  // - [ ] Task
    

}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    
    // Highlighter'ı editöre bağla
    highlighter = new MarkdownHighlighter(editor->document());
    highlighter->setEditor(editor);
    
    // Cursor hareketini dinle (Canlı değişim için)
    connect(editor, &QTextEdit::cursorPositionChanged, 
            this, &MainWindow::onCursorPositionChanged);
            
    setWindowTitle("Markdown Live Editor");
    resize(1000, 600);
}
MainWindow::~MainWindow() {
    // Qt'nin Parent-Child sistemi sayesinde pointer'ları
    // manuel silmemize gerek yok (root widget silinince hepsi silinir).
}


void MainWindow::onOpenFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Proje Klasörü Seç", 
                                                    QDir::homePath(), 
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        // Modeli seçilen klasöre odakla
        QModelIndex rootIndex = fileModel->setRootPath(dir);
        fileTreeView->setRootIndex(rootIndex);
    }
}

void MainWindow::DebugOpenFolder() {
    QString dir = QString("/home/emrehan/notes/obsidian_notlar/");

    if (!dir.isEmpty()) {
        // Modeli seçilen klasöre odakla
        QModelIndex rootIndex = fileModel->setRootPath(dir);
        fileTreeView->setRootIndex(rootIndex);
    }
}


void MainWindow::openFile(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists() || !fileInfo.isFile()) return;

    // Dosya uzantısını al ve küçük harfe çevir (PNG, png, Png fark etmesin)
    QString extension = fileInfo.suffix().toLower();
    
    // Resim uzantıları listesi
    QStringList imageExtensions = {"png", "jpg", "jpeg", "gif", "bmp", "svg", "webp"};

    if (imageExtensions.contains(extension)) {
        // --- SENARYO 1: RESİM DOSYASI ---
        
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // Resmi etikete yükle
            imageLabel->setPixmap(pixmap);
            
            // Büyük resimlerin ekrana sığması için (Opsiyonel)
            // Eğer resim pencereden büyükse, sığacak şekilde küçült (Aspect Ratio koruyarak)
            //imageLabel->setScaledContents(false); // Kaydırma çubuğu kullanacaksan false
             
            // GÖRÜNÜMÜ DEĞİŞTİR: Stack'in 1. sayfasına (Resim) geç
            rightPanelStack->setCurrentWidget(imageScrollArea);
            
            setWindowTitle(fileInfo.fileName() + " (Resim)");
        }
    } 
    else {
        // --- SENARYO 2: METİN DOSYASI ---
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            in.setCodec("UTF-8"); 
    #else
            in.setEncoding(QStringConverter::Utf8);
    #endif
            editor->setPlainText(in.readAll());
            file.close();
            
            // GÖRÜNÜMÜ DEĞİŞTİR: Stack'in 0. sayfasına (Editör) geç
            rightPanelStack->setCurrentWidget(editor);
            
            setWindowTitle(fileInfo.fileName());
            if(highlighter) highlighter->rehighlight();
        }
    }
}



void MainWindow::onFileClicked(const QModelIndex &index) {
    QString filePath = fileModel->filePath(index);
    // Klasör değilse aç
    if (!fileModel->isDir(index)) {
        openFile(filePath);
    }
}


// İmleç değiştiğinde yapılacaklar
void MainWindow::onCursorPositionChanged() {
    // Highlighter'a "Hey, cursor değişti, satırları yeniden boya" diyoruz.
    // rehighlight() fonksiyonu tüm metni tekrar tarar.
    // Büyük dosyalarda performans için sadece değişen bloğu rehighlightBlock ile tetiklemek gerekir
    // ama şimdilik bu yeterli.
    highlighter->rehighlight();
}


void MainWindow::DebugOpenFile() {
    QString filePath = QString("/home/emrehan/notes/obsidian_notlar/kod/araçlar/obsidian.md");
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        in.setCodec("UTF-8"); 
#else
        in.setEncoding(QStringConverter::Utf8);
#endif
        
        // DİKKAT: Burada setMarkdown KULLANMIYORUZ.
        // Çünkü setMarkdown veriyi bozar. Ham metni yüklüyoruz.
        // Görselliği "MarkdownHighlighter" halledecek.
        editor->setPlainText(in.readAll());
        
        file.close();
        setWindowTitle("DEBUG");
    }
}



