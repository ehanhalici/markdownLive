#include "CodeEditor.hpp"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QRegularExpression>
#include <QMouseEvent> // Mouse olayları için


LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
// --- LineNumberArea ---

QSize LineNumberArea::sizeHint() const {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    codeEditor->lineNumberAreaPaintEvent(event);
}

// YENİ: Tıklamayı Editör'e ilet
void LineNumberArea::mousePressEvent(QMouseEvent *event) {
    codeEditor->processLineNumberAreaEvent(event);
}

// --- CodeEditor ---

CodeEditor::CodeEditor(QWidget *parent) : QTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);

    connect(document(), &QTextDocument::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, [this](int){ update(); lineNumberArea->update(); });
    connect(this, &QTextEdit::cursorPositionChanged, [this](){ lineNumberArea->update(); });

    updateLineNumberAreaWidth(0);
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) { max /= 10; digits++; }

    // --- YENİ: Numaralar için Özel Font Ayarı ---
    QFont numFont = this->font(); // Editörün fontunu al
    numFont.setPointSize(6);
    // İstersen font ailesini de değiştirebilirsin:
    // numFont.setFamily("Segoe UI"); 
    
    QFontMetrics numMetrics(numFont);
    // --------------------------------------------

    // Genişliği bu yeni küçük fonta göre hesapla
    // +25 piksel padding (ok işareti ve kenar boşluğu için)
    int space = 25 + numMetrics.horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) lineNumberArea->scroll(0, dy);
    else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
    QTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

// --- YENİ: Başlık Seviyesini Bulma ---
int CodeEditor::getHeaderLevel(const QString &text) {
    // Regex: Satır başındaki # işaretlerini say
    static QRegularExpression headerRegex("^(#+)\\s");
    QRegularExpressionMatch match = headerRegex.match(text);
    if (match.hasMatch()) {
        return match.captured(1).length(); // # Sayısı (Level)
    }
    return 0; // Başlık değil
}

// --- YENİ: Katlama Mantığı (THE BRAIN) ---
void CodeEditor::toggleFold(int blockNumber) {
    QTextBlock startBlock = document()->findBlockByNumber(blockNumber);
    if (!startBlock.isValid()) return;

    int level = getHeaderLevel(startBlock.text());
    if (level == 0) return; // Sadece başlıklar katlanabilir

    // Durumu tersine çevir (Katlıysa aç, açıksa katla)
    bool isFolding = !foldedBlocks.contains(blockNumber);
    
    if (isFolding) foldedBlocks.insert(blockNumber);
    else foldedBlocks.remove(blockNumber);

    // Sonraki blokları işle
    QTextBlock block = startBlock.next();
    while (block.isValid()) {
        int nextLevel = getHeaderLevel(block.text());

        // EĞER aynı seviyede veya daha üst seviyede bir başlık görürsek DUR.
        // Örn: ## altındaysak, başka bir ## veya # görene kadar devam et.
        // ### görürsek devam et (çünkü o bizim altımızda).
        if (nextLevel > 0 && nextLevel <= level) {
            break; 
        }

        // Bloğu Gizle / Göster
        // setVisible fonksiyonu satırı tamamen yok eder (height = 0 olur)
        block.setVisible(!isFolding);
        
        // Eğer içerdeki alt başlık daha önce katlanmışsa, 
        // açarken onun içeriğini açmamalıyız (Smart Unfold). 
        // Ama şimdilik basit "hepsini aç/kapat" yapıyoruz.
        
        block = block.next();
    }
    
    // Değişikliklerin görünmesi için layout'u güncelle
    document()->adjustSize();
    viewport()->update();
    lineNumberArea->update();
}

// --- YENİ: Tıklama Olayı ---
void CodeEditor::processLineNumberAreaEvent(QMouseEvent *event) {
    // Tıklanan Y koordinatından hangi satırda olduğumuzu bul
    // Bu işlem biraz matematik gerektirir
    QTextBlock block = cursorForPosition(QPoint(0, 0)).block();
    int blockNumber = block.blockNumber();
    
    int top = (int) document()->documentLayout()->blockBoundingRect(block).translated(0, -verticalScrollBar()->value()).top();
    int bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();

    while (block.isValid() && top <= event->y()) {
        if (block.isVisible() && event->y() >= top && event->y() <= bottom) {
            // Tıklanan satır bulundu!
            
            // Eğer başlık ise katlama fonksiyonunu çağır
            if (getHeaderLevel(block.text()) > 0) {
                toggleFold(blockNumber);
            }
            break;
        }

        block = block.next();
        // Gizli blokları atlamamız lazım, visible olmayanı geç
        if (!block.isVisible()) continue; 
        
        top = bottom;
        bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();
        blockNumber = block.blockNumber(); // Dikkat: blockNumber'ı doğrudan artırmıyoruz, block'tan alıyoruz
    }
}

// --- GÜNCELLENMİŞ: Çizim Olayı ---
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    
    // --- 1. ARKA PLAN RENGİ ---
    // Buraya istediğin rengi verebilirsin. 
    // #202020: Çok koyu gri (Obsidian Sol Panel Rengi)
    // #1e1e1e: Editörle aynı renk (Daha bütünleşik durur)
    painter.fillRect(event->rect(), QColor("#181818")); 

    // --- 2. NUMARA FONTU AYARI ---
    QFont numFont = this->font();
    numFont.setPointSize(10); // Boyutu küçült (Yukarıdakiyle aynı olmalı)
    // numFont.setBold(true); // İstersen kalın yapabilirsin
    painter.setFont(numFont); // Ressama "Bu fontu kullan" diyoruz
    
    // Bu fontun ölçülerini al (Ortalamak için lazım)
    QFontMetrics numMetrics(numFont);

    // -----------------------------

    QTextBlock block = cursorForPosition(QPoint(0, 0)).block();
    int blockNumber = block.blockNumber();
    
    int top = (int) document()->documentLayout()->blockBoundingRect(block).translated(0, -verticalScrollBar()->value()).top();
    int bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            // --- OK İŞARETİ ÇİZİMİ ---
            int level = getHeaderLevel(block.text());
            if (level > 0) {
                bool isFolded = foldedBlocks.contains(blockNumber);
                
                painter.setPen(QColor("#242424"));
                painter.setBrush(QColor("#242424"));
                
                QPolygonF arrow;
                int arrowSize = 4; 
                // Ok işaretini yeni fontun yüksekliğine göre ortala
                int midY = top + numMetrics.height() / 2; 
                int arrowX = lineNumberArea->width() - 10; 
                
                if (isFolded) { // Kapalıyken (Sağ Ok)
                    arrow << QPointF(arrowX, midY - arrowSize)
                          << QPointF(arrowX + arrowSize, midY)
                          << QPointF(arrowX, midY + arrowSize);
                } else { // Açıkken (Aşağı Ok)
                    arrow << QPointF(arrowX - arrowSize, midY - 2)
                          << QPointF(arrowX + arrowSize, midY - 2)
                          << QPointF(arrowX, midY + arrowSize - 2);
                }
                painter.drawPolygon(arrow);
            }
            // -----------------------

            // Aktif satır kontrolü
            bool isCurrentLine = (textCursor().blockNumber() == blockNumber);
            
            // Numaraların Rengi
            if (isCurrentLine)
                 painter.setPen(QColor("#bababa")); // Aktifse parlak gri/beyaz
            else
                 painter.setPen(QColor("#5c6370")); // Pasifse soluk gri (VS Code stili)
            
            // Numarayı Çiz
            // height() olarak 'numMetrics.height()' kullanıyoruz ki küçük fonta göre ortalansın
            painter.drawText(0, top, lineNumberArea->width() - 15, numMetrics.height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }

        block = block.next();
        top = bottom;
        
        while(block.isValid() && !block.isVisible()) {
             block = block.next();
        }
        
        if(block.isValid()) {
             bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();
        }
        blockNumber = block.blockNumber();
    }
}
