#include "MainWindow.hpp"

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    // Sadece editörün viewport'undaki olaylara bakıyoruz
    if (obj == editor->viewport()) {
        
        // Fare Tıklaması (Mouse Press)
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            // Sadece SOL tık ve CTRL tuşu basılıysa (Obsidian Modu)
            // İstersen && kısmını silip direkt tıklamayla da çalıştırabilirsin.
            if (mouseEvent->button() == Qt::LeftButton && (mouseEvent->modifiers() & Qt::ControlModifier)) {
                
                // Tıklanan koordinattaki imleci al
                QTextCursor cursor = editor->cursorForPosition(mouseEvent->pos());
                
                // İmlecin olduğu satırın tamamını al
                QString line = cursor.block().text();
                
                // İmlecin satır içindeki pozisyonu (0'dan başlar)
                int posInBlock = cursor.positionInBlock();
                
                gotoInternalPage(line, posInBlock);
                searchTags(line);
                return true;
            }
        }
        
        // Opsiyonel: İmleç link üzerindeyken el işareti çıksın (Mouse Move)
        if (event->type() == QEvent::MouseMove) {
             QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
             // Ctrl basılıyken link üzerindeyse el yap, yoksa normal imleç (I-Beam)
             // Bu kısım biraz işlemci yiyebilir, şimdilik pas geçiyorum.
        }
    }
    
    // Diğer tüm olayları Qt'nin standart işleyişine bırak
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::gotoInternalPage(QString &text, int positionInBlock) {
    // Regex Açıklaması:
    // \[\\[           -> [[ ile başla
    // ([^#|\]]+)      -> GRUP 1 (Dosya Adı): #, | veya ] görene kadar al.
    // (#[^|\]]+)?     -> GRUP 2 (Anchor - Opsiyonel): # ile başla, | veya ] görene kadar al. (BUNU IGNORE EDECEĞİZ)
    // (\|([^\]]+))?   -> GRUP 3 (Label - Opsiyonel): | ile başla.
    // \]\]            -> ]] ile bitir

    QRegularExpression re("\\[\\[([^#|\\]]+)(#[^|\\]]+)?(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator i = re.globalMatch(text);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();

        // Tıklanan yer bu link yapısının sınırları içinde mi?
        if (positionInBlock >= match.capturedStart() && positionInBlock <= match.capturedEnd()) {

            // GRUP 1: Sadece dosya adını verir (Örn: "BaskaBirDosya")
            // Yanındaki #^satir kısmı Grup 2'dedir, onu almıyoruz.
            QString targetName = match.captured(1).trimmed(); 

            // --- DOSYA YOLUNU BULMA VE AÇMA ---
            QDir rootDir(fileModel->rootPath());

            if (!targetName.contains(".")) {
                targetName += ".md";
            }

            QString fullPath = rootDir.absoluteFilePath(targetName);

            if (QFile::exists(fullPath)) {
                openFile(fullPath);
            } else {
                // Dosya yoksa kullanıcıya bildir
                 QMessageBox::information(this, "Bulunamadı", 
                                         "Dosya yok: " + targetName);
            }

            return; 
        }
    }
}

// 1. SENİN VERDİĞİN FONKSİYON (Tamamlanmış Hali)
void MainWindow::searchTags(QString &text) {
    // Regex: Başında boşluk veya satır başı olan #etiket'leri bul
    QRegularExpression tagRegex("(?<=\\s|^)(#[a-zA-Z0-9_\\-]+)");
    QRegularExpressionMatchIterator i = tagRegex.globalMatch(text);

    if (i.hasNext()) {
        // İlk eşleşen etiketi al (Örneğin "#proje")
        QRegularExpressionMatch match = i.next();
        QString foundTag = match.captured(1);

        // Sol paneli "Ara" sekmesine (Index 1) geçir
        leftTabWidget->setCurrentIndex(1);

        // Arama fonksiyonunu çağır
        performGlobalSearch(foundTag);
    }
}

// 2. TÜM DOSYALARDA ARAMA YAPAN FONKSİYON
void MainWindow::performGlobalSearch(const QString &searchTerm) {
    // Listeyi temizle
    searchResultList->clear();
    
    // Hangi klasörde arayacağız? (FileModel'in root path'i)
    QString rootPath = fileModel->rootPath();
    if (rootPath.isEmpty()) return;

    // Alt klasörler dahil tüm .md ve .txt dosyalarını tarayan iterator
    QDirIterator it(rootPath, QStringList() << "*.md" << "*.txt", 
                    QDir::Files, QDirIterator::Subdirectories);

    int resultCount = 0;

    while (it.hasNext()) {
        QString filePath = it.next();
        
        // Dosyayı aç ve satır satır oku
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            in.setCodec("UTF-8");
    #else
            in.setEncoding(QStringConverter::Utf8);
    #endif

            int lineNumber = 0;
            while (!in.atEnd()) {
                QString line = in.readLine();
                lineNumber++;

                // Eğer satır aranan kelimeyi içeriyorsa (Büyük/Küçük harf duyarsız)
                if (line.contains(searchTerm, Qt::CaseInsensitive)) {
                    
                    // --- GÖRÜNÜM AYARLAMA ---
                    // Listede nasıl görünecek? Örn: "Notlar.md:15 - #proje bitmeli"
                    QString fileName = QFileInfo(filePath).fileName();
                    QString displayText = QString("%1:%2 -> %3")
                                          .arg(fileName)
                                          .arg(lineNumber)
                                          .arg(line.trimmed()); // Satır başı boşluklarını sil

                    QListWidgetItem *item = new QListWidgetItem(displayText);
                    
                    // --- GİZLİ VERİ SAKLAMA (Kritik Nokta) ---
                    // Tıklayınca dosyayı bulabilmek için dosya yolunu ve satır nosunu item'in içine gizliyoruz.
                    item->setData(Qt::UserRole, filePath);       // Gizli Veri 1: Dosya Yolu
                    item->setData(Qt::UserRole + 1, lineNumber); // Gizli Veri 2: Satır No
                    
                    // İkon ekleyelim (Şık dursun)
                    item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));

                    searchResultList->addItem(item);
                    resultCount++;
                }
            }
            file.close();
        }
    }
    
    // Kullanıcıya bilgi ver (Status bar veya listenin başına ekleyerek)
    if (resultCount == 0) {
        searchResultList->addItem("Sonuç bulunamadı: " + searchTerm);
    }
}

// 3. SONUCA TIKLAYINCA ORAYA GİTME
void MainWindow::onSearchResultClicked(QListWidgetItem *item) {
    // Gizlediğimiz verileri geri alıyoruz
    QString filePath = item->data(Qt::UserRole).toString();
    int lineNumber = item->data(Qt::UserRole + 1).toInt();

    if (filePath.isEmpty()) return;

    // 1. Dosyayı aç (Mevcut openFile fonksiyonunu kullanıyoruz)
    openFile(filePath);

    // 2. İlgili satıra git
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start); // En başa dön
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1); // Satır kadar aşağı in
    cursor.select(QTextCursor::LineUnderCursor); // Satırı seç (belirgin olsun diye)
    
    editor->setTextCursor(cursor); // İmleci güncelle
    editor->setFocus(); // Editöre odaklan
}
