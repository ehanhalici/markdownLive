#include "MainWindow.hpp"
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCharFormat>
#include <QRegularExpressionMatchIterator>
#include <qcolor.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qregularexpression.h>
#include <qtextformat.h>


void MarkdownHighlighter::boldRegex(const QString &text, bool isCurrentLine) {
    // 1. Örnek: **Bold** (**Kalın**)
    QRegularExpression boldRegex("\\*\\*(.*?)\\*\\*");
    QRegularExpressionMatchIterator i = boldRegex.globalMatch(text);

    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        
        int start = match.capturedStart();
        int length = match.capturedLength();
        
        // Ortadaki yazı: Yazı (Grup 1)
        int textStart = match.capturedStart(1);
        int textLength = match.capturedLength(1);

        if (isCurrentLine) {
            // İmleç bu satırda: Sadece yazıyı GRİ yap (Syntax belli olsun)
            setFormat(start, 2, theme.DARKGRAY); 
            setFormat(start + length - 2, 2, theme.DARKGRAY); 
            setFormat(textStart, textLength, theme.B_GRAY);
        } 
        else {
            // İmleç BAŞKA satırda: ** işaretlerini GİZLE            
            setFormat(start, 2, theme.HIDDEN); // İlk **
            setFormat(start + length - 2, 2, theme.HIDDEN); // Son **
            
            // Ortadaki yazıyı belirginleştir
            setFormat(textStart, textLength, theme.B_BLUE);
        }
    }    
}

void MarkdownHighlighter::header(const QString &text, bool isCurrentLine) {
    // 2. Örnek: # Başlık
    QRegularExpression headerRegex("^(#+)\\s+(.*)");
    QRegularExpressionMatch matchHeader = headerRegex.match(text);
    if (matchHeader.hasMatch()) {
        int hashLength = matchHeader.capturedLength(1); // # işaretlerinin uzunluğu
        
        if (!isCurrentLine) {
            // Başka satır: # işaretini gizle, yazıyı dev yap
            setFormat(0, hashLength, theme.HIDDEN);

            qreal fps = theme.B_GRAY.fontPointSize();
            theme.B_GRAY.setFontPointSize(16 + (6 - hashLength) * 2); // H1 büyük, H6 küçük
            setFormat(hashLength, text.length() - hashLength, theme.B_GRAY);
            theme.B_GRAY.setFontPointSize(fps);
            
        } else {
             setFormat(0, hashLength, theme.GRAY);
        }
    }
}

void MarkdownHighlighter::italic(const QString &text, bool isCurrentLine) {
    // 3. Örnek: İtalik (*yazı* veya _yazı_)
    // Regex: Tek yıldız veya tek alt çizgi arasındaki metni bul
    QRegularExpression italicRegex("(\\*|_)(.*?)\\1");
    QRegularExpressionMatchIterator i_italic = italicRegex.globalMatch(text);

    while (i_italic.hasNext()) {
        QRegularExpressionMatch match = i_italic.next();
    
        // Eğer Bold (**) ile çakışıyorsa atla (Basit bir önlem)
        if (match.captured(2).startsWith('*') || match.captured(2).startsWith('_')) continue;

        int start = match.capturedStart();
        int length = match.capturedLength();

        if (isCurrentLine) {
            // Aktif satır: İşaretleri gri yap
            setFormat(start, 1, theme.RED);             // baştaki *
            setFormat(start + length - 1, 1, theme.RED); // sondaki *
        } else {
            // Pasif satır: İşaretleri GİZLE
            setFormat(start, 1, theme.HIDDEN);
            setFormat(start + length - 1, 1, theme.HIDDEN);

            setFormat(start + 1, length - 2, theme.I_GRAY);
        }
    }
}

void MarkdownHighlighter::internalLink(const QString &text, bool isCurrentLine) {
    // 4. Örnek: Linkler [Google](https://google.com)
    // Grup 1: Görünen Metin (Google)
    // Grup 2: URL (https://google.com)
    QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator i_link = linkRegex.globalMatch(text);

    while (i_link.hasNext()) {
        QRegularExpressionMatch match = i_link.next();

        int fullStart = match.capturedStart();
        int fullLength = match.capturedLength();

        int textStart = match.capturedStart(1);
        int textLength = match.capturedLength(1);

        if (isCurrentLine) {
            // Aktif satır: [ ve ]( ve ) kısımlarını grileştir
            // Sadece syntax karakterlerini boyuyoruz, aradakiler normal kalıyor
            setFormat(fullStart, 1, theme.GRAY); // [
            setFormat(textStart + textLength, 2, theme.GRAY); // ](
            setFormat(fullStart + fullLength - 1, 1, theme.GRAY); // )

            // URL kısmını biraz daha silik yapabiliriz
            setFormat(match.capturedStart(2), match.capturedLength(2), theme.GRAY);
        } else {
            // Pasif satır: URL'i ve Parantezleri GİZLE, Sadece Metni Göster
            // 1. Kısım: "[" işaretini gizle
            setFormat(fullStart, 1, theme.HIDDEN);

            // 2. Kısım: "](url)" kısmını komple gizle
            // Metnin bitişinden bloğun sonuna kadar olan kısım
            int suffixStart = textStart + textLength;
            int suffixLength = fullLength - (textLength + 1); // +1 baştaki [
            setFormat(suffixStart, suffixLength, theme.HIDDEN);

            // 3. Kısım: Ortadaki metni Mavi ve Altı Çizili yap
            setFormat(textStart, textLength, theme.L_BLUE);
        }
    }
}

void MarkdownHighlighter::externalLink(const QString &text, bool isCurrentLine) {
    // Regex Açıklaması:
    // \[             -> [ ile başla
    // ([^\]]+)       -> GRUP 1 (Görünen Metin): ] görene kadar al
    // \]             -> ] ile kapat
    // \[             -> [ ile tekrar başla (ikinci kısım)
    // ([^\]]+)       -> GRUP 2 (Referans ID): ] görene kadar al
    // \]             -> ] ile bitir

    QRegularExpression refLinkRegex("\\[([^\\]]+)\\]\\[([^\\]]+)\\]");
    QRegularExpressionMatchIterator i = refLinkRegex.globalMatch(text);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();

        int fullStart = match.capturedStart();
        int fullLen = match.capturedLength();
        
        // Grup 1: Görünen Metin (Örn: "Google")
        int textStart = match.capturedStart(1);
        int textLen = match.capturedLength(1);

        // Grup 2: Referans ID (Örn: "1" veya "google-url")
        int refStart = match.capturedStart(2);
        int refLen = match.capturedLength(2);

        if (isCurrentLine) {
            // Aktif satır: Köşeli parantezleri gri yap
            // [Metin] kısmı
            setFormat(fullStart, 1, theme.GRAY); // [
            setFormat(textStart + textLen, 2, theme.GRAY); // ][ (Aradaki geçiş)
            setFormat(fullStart + fullLen - 1, 1, theme.GRAY); // ]
            
            // Referans ID kısmını biraz silik yapabiliriz
            setFormat(refStart, refLen, theme.GRAY);

        } else {
            // Pasif satır: 
            // 1. [Metin] parantezlerini gizle
            // 2. [ReferansID] kısmını TAMAMEN gizle

            // [ işaretini gizle
            setFormat(fullStart, 1, theme.HIDDEN);
            
            // ][ReferansID] kısmını komple gizle (Metnin sonundan bloğun sonuna kadar)
            // textStart + textLen -> Metnin bittiği yer
            // fullLen - (textLen + 1) -> Geri kalan uzunluk (+1 baştaki [ için)
            int suffixStart = textStart + textLen;
            int suffixLen = fullLen - (textLen + 1);
            setFormat(suffixStart, suffixLen, theme.HIDDEN);

            // Ortadaki metni Link formatına çevir
            setFormat(textStart, textLen, theme.L_BLUE);
        }
    }
}

void MarkdownHighlighter::code(const QString &text, bool isCurrentLine) {
    // 5. Örnek: Inline Code (`kod`)
    QRegularExpression codeRegex("`([^`]+)`");
    QRegularExpressionMatchIterator i_code = codeRegex.globalMatch(text);

    while (i_code.hasNext()) {
        QRegularExpressionMatch match = i_code.next();
        int start = match.capturedStart();
        int length = match.capturedLength();


        if (!isCurrentLine) {
            // Pasifse tırnakları (` `) gizle
            QTextCharFormat transparent;
            transparent.setForeground(Qt::transparent);
            transparent.setFontPointSize(1);

            setFormat(start, 1, transparent); // Baştaki `
            setFormat(start + length - 1, 1, transparent); // Sondaki `

            // Ortaya format at
            setFormat(start + 1, length - 2, theme.C_RED);
        } else {
            // Aktifse tırnaklar görünsün ama tüm blok boyansın
            setFormat(start, length, theme.C_RED);
        }
    }
}

void MarkdownHighlighter::taskList(const QString &text, bool isCurrentLine) {
    // 6. Örnek: Task List (- [ ] veya - [x])
    // Regex Açıklaması:
    // ^(\s*)       -> Grup 1: Satır başındaki boşluklar (indentation)
    // (-|\*)       -> Grup 2: Tire veya yıldız işareti
    // \s+          -> Aradaki boşluk
    // \[(x|\s)\]   -> Grup 3: [x] veya [ ] (İçindeki x veya boşluk Grup 4)
    QRegularExpression taskList("^(\\s*)(- |\\* )?\\[(x|X|\\s)\\]");
    QRegularExpressionMatchIterator i_task = taskList.globalMatch(text);

    while (i_task.hasNext()) {
        // HATA DÜZELTİLDİ: i_code.next() -> i_task.next()
        QRegularExpressionMatch match = i_task.next(); 

        int start = match.capturedStart();
        int length = match.capturedLength();

        // [x] veya [ ] içindeki karakter (x mi boşluk mu?)
        QString content = match.captured(3); 
        bool isChecked = (content.contains("x") || content.contains("X"));

        if (isCurrentLine) {
            // --- AKTİF SATIR (Düzenleme Modu) ---
            // Syntax'ı grileştir ama okunabilir tut
            setFormat(start, length, theme.GRAY);

            // Eğer 'x' varsa onu yeşil yap, belli olsun
            if (isChecked) {
                // 'x' karakterinin tam konumunu bulup boyayalım
                // match.capturedStart(3) -> [ ] kısmının başı
                // +1 -> parantezden sonraki karakter
                setFormat(match.capturedStart(3) , 1, theme.B_GREEN);
            }
        } 
        else {
            // --- PASİF SATIR (Önizleme Modu) ---

            // 1. Tire işaretini (-) GİZLE veya çok silik yap
            // Not: Tamamen gizlersen girinti bozulabilir, o yüzden Transparent yerine 
            // arka plan rengiyle aynı yapmak veya çok açık gri yapmak daha güvenlidir.
            // Ama şimdilik Transparent yapıp fontu küçültelim (yer kaplamasın).
            // Grup 2 (- veya *) kısmını bul ve gizle
            // (Regex grubuna göre index hesaplaması gerekebilir, basitleştirilmiş:)
            setFormat(start, match.capturedLength(1) + match.capturedLength(2), theme.HIDDEN);

            // 2. Kutucuğu ([ ]) Şekillendir
            QTextCharFormat color;
            if (isChecked) {
                // YAPILDI ([x]): Yeşil yap
                color = theme.B_GREEN;

                // --- OBSIDIAN EFEKTİ: Metnin geri kalanının üstünü çiz ---
                // Checkbox'tan satır sonuna kadar olan kısmı boya
                int textStart = start + length;
                int textEnd = text.length();
                setFormat(textStart, textEnd - textStart, theme.S_GRAY);

            } else {
                // YAPILMADI ([ ]): Turuncu/Mavi yap (Dikkat çeksin)
                color = theme.B_ORANGE;
            }

            // Kutucuğun kendisine formatı uygula
            // match.capturedStart(3) -> [x] kısmının başladığı yer
            // match.capturedLength(3) -> uzunluğu (3 karakter)
            setFormat(match.capturedStart(3), match.capturedLength(3), color);
        }
    }
}


void MarkdownHighlighter::highlight(const QString &text, bool isCurrentLine) {
    // ========================================================
    // 4. ADIM: HIGHLIGHT ==Vurgu==
    // ========================================================
    // Görünmez Mürekkep Formatı (Syntax Gizlemek İçin)

    // Gri Syntax Formatı (Aktif satırda syntax'ı silik göstermek için)
    
    QRegularExpression highlightRegex("==(.*?)==");
    QRegularExpressionMatchIterator i_high = highlightRegex.globalMatch(text);

    while (i_high.hasNext()) {
        QRegularExpressionMatch match = i_high.next();
        int start = match.capturedStart();
        int len = match.capturedLength();
        int contentStart = match.capturedStart(1);
        int contentLen = match.capturedLength(1);

        QTextCharFormat highlightFmt;
        highlightFmt.setBackground(QColor(60, 60, 0)); // Koyu Sarı (Dark Theme uyumlu)
        highlightFmt.setForeground(Qt::white);         // Yazı beyaz olsun okunsun
        
        // Light Theme kullanıyorsan:
        // highlightFmt.setBackground(Qt::yellow);
        // highlightFmt.setForeground(Qt::black);

        if (isCurrentLine) {
            setFormat(start, 2, theme.GRAY); // ==
            setFormat(start + len - 2, 2, theme.GRAY); // ==
            setFormat(contentStart, contentLen, highlightFmt);
        } else {
            setFormat(start, 2, theme.HIDDEN); // == gizle
            setFormat(start + len - 2, 2, theme.HIDDEN); // == gizle
            setFormat(contentStart, contentLen, highlightFmt);
        }
    }
    
}

void MarkdownHighlighter::strike(const QString &text, bool isCurrentLine) {
    // ========================================================
    // 5. ADIM: STRIKETHROUGH ~~Üstü Çizili~~
    // ========================================================
    QRegularExpression strikeRegex("~~(.*?)~~");
    QRegularExpressionMatchIterator i_strike = strikeRegex.globalMatch(text);

    while (i_strike.hasNext()) {
        QRegularExpressionMatch match = i_strike.next();
        int start = match.capturedStart();
        int len = match.capturedLength();
        
        if (isCurrentLine) {
            setFormat(start, 2, theme.GRAY);
            setFormat(start + len - 2, 2, theme.GRAY);
            setFormat(match.capturedStart(1), match.capturedLength(1), theme.S_GRAY);
        } else {
            setFormat(start, 2, theme.HIDDEN);
            setFormat(start + len - 2, 2, theme.HIDDEN);
            setFormat(match.capturedStart(1), match.capturedLength(1), theme.S_GRAY);
        }
    }

    // ========================================================
    // 6. ADIM: HORIZONTAL RULE (--- veya ***)
    // ========================================================
    // Sadece --- veya *** içeren satırlar
    QRegularExpression hrRegex("^(\\-{3,}|\\*{3,})$");
    if (hrRegex.match(text).hasMatch()) {
        if (!isCurrentLine) {
            // Metni tamamen gizle
            setFormat(0, text.length(), theme.HIDDEN);
            
            // Tüm satıra uygula
            setFormat(0, text.length(), theme.LINE);
        } else {
            setFormat(0, text.length(), theme.S_GRAY);
        }
    }
}

void MarkdownHighlighter::quote(const QString &text, bool isCurrentLine) {
    QTextCharFormat transparentFmt;
    transparentFmt.setForeground(Qt::transparent);
    transparentFmt.setFontPointSize(1); // Yer kaplamasın

    // Gri Syntax Formatı (Aktif satırda syntax'ı silik göstermek için)
    QTextCharFormat syntaxFmt;
    syntaxFmt.setForeground(Qt::gray);

   // ========================================================
    // 7. ADIM: BLOCKQUOTES (> Alıntı)
    // ========================================================
    QRegularExpression quoteRegex("^>\\s(.*)");
    if (quoteRegex.match(text).hasMatch()) {
        if (!isCurrentLine) {
            // > işaretini gizle
            setFormat(0, 1, transparentFmt);
            
            // Metni italik ve gri yap
            setFormat(1, text.length() - 1, theme.DARKGRAY);
        } else {
            setFormat(0, 1, syntaxFmt);
        }
    } 
}


// 1. WIKILINKS: [[Not Adi]] veya [[Not Adi|Görünen]]
void MarkdownHighlighter::wikiLink(const QString &text, bool isCurrentLine) {
    QRegularExpression wikiRegex("\\[\\[([^|\\]]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator i_wiki = wikiRegex.globalMatch(text);

    while (i_wiki.hasNext()) {
        QRegularExpressionMatch match = i_wiki.next();
        
        int fullStart = match.capturedStart();
        int fullLen = match.capturedLength();
        
        // Grup 1: Link (Örn: DosyaAdi)
        // Grup 3: Etiket (Örn: Görünenİsim)
        bool hasLabel = !match.captured(3).isEmpty();

        if (isCurrentLine) {
            // Aktif satır: [[, |, ]] karakterlerini gri yap

            setFormat(fullStart, 2, theme.GRAY); // [[
            setFormat(fullStart + fullLen - 2, 2, theme.GRAY); // ]]
            
            if (hasLabel) {
                setFormat(match.capturedStart(2), 1, theme.GRAY); // | işareti
            }
        } else {
            // Pasif satır: Syntax'ı gizle, sadece etiketi (yoksa linki) göster

            if (hasLabel) {
                // [[Link|Etiket]] -> Sadece "Etiket" görünsün
                int labelStart = match.capturedStart(3);
                int labelLen = match.capturedLength(3);
                
                // Baştan etikete kadar gizle
                setFormat(fullStart, labelStart - fullStart, theme.HIDDEN);
                // Sondaki ]] gizle
                setFormat(labelStart + labelLen, 2, theme.HIDDEN);
                // Etiketi boya
                setFormat(labelStart, labelLen, theme.L_PURPLE);
            } else {
                // [[Link]] -> Sadece "Link" görünsün
                setFormat(fullStart, 2, theme.HIDDEN); // [[
                setFormat(fullStart + fullLen - 2, 2, theme.HIDDEN); // ]]
                setFormat(fullStart + 2, fullLen - 4, theme.L_PURPLE);
            }
        }
    }
}

// 2. RESİMLER: ![Alt](src)
void MarkdownHighlighter::image(const QString &text, bool isCurrentLine) {
    QRegularExpression imgRegex("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator i_img = imgRegex.globalMatch(text);

    while (i_img.hasNext()) {
        QRegularExpressionMatch match = i_img.next();
        int fullStart = match.capturedStart();
        int fullLen = match.capturedLength();

        if (isCurrentLine) {
            // Aktif satır: Syntax gri
            setFormat(fullStart, 1, theme.GRAY); // !
            setFormat(match.capturedStart(1) + match.capturedLength(1), 2, theme.GRAY); // ](
            setFormat(fullStart + fullLen - 1, 1, theme.GRAY); // )
        } else {
            // Pasif satır: Resim syntax'ını biraz farklı göster (Editörde resim render etmek zordur)
            // Biz burada sadece "Bu bir resimdir" diye belirtelim.
            
            // Syntax'ı tamamen gizlemek yerine silikleştiriyoruz ki kullanıcı resim olduğunu anlasın
            // (Tam görselleştirme için QTextDocumentFragment gerekir, highlighter yapamaz)
            setFormat(fullStart, fullLen, theme.ORANGE);
        }
    }
}

// 3. MATEMATİK / LATEX: $$...$$ veya $...$
void MarkdownHighlighter::math(const QString &text, bool isCurrentLine) {
    // Regex: $ veya $$ arasında kalanlar
    QRegularExpression mathRegex("(\\$\\$|\\$)(.*?)(\\1)");
    QRegularExpressionMatchIterator i_math = mathRegex.globalMatch(text);

    while (i_math.hasNext()) {
        QRegularExpressionMatch match = i_math.next();
        
        int start = match.capturedStart();
        int len = match.capturedLength();
        int contentStart = match.capturedStart(2);
        int contentLen = match.capturedLength(2);

        if (isCurrentLine) {
            QTextCharFormat syntaxFmt;
            syntaxFmt.setForeground(Qt::gray);
            
            // $ işaretlerini gri yap
            setFormat(start, match.capturedLength(1), syntaxFmt);
            setFormat(start + len - match.capturedLength(1), match.capturedLength(1), syntaxFmt);
            setFormat(contentStart, contentLen, theme.TEAL);
        } else {
            // Pasif: $ işaretlerini gizle
            setFormat(start, match.capturedLength(1), theme.HIDDEN);
            setFormat(start + len - match.capturedLength(1),1, theme.HIDDEN);
            setFormat(contentStart, contentLen, theme.TEAL);
        }
    }
}

// 4. ÇOK SATIRLI KOD BLOKLARI (State Machine Gerektirir)
// Bu fonksiyon bool döner: Eğer kod bloğu içindeysek TRUE döner.
bool MarkdownHighlighter::fencedCodeBlock(const QString &text) {
    int currentState = previousBlockState(); 
    int nextState = currentState;
    
    // ``` ile başlayan satırları bul
    QRegularExpression fenceRegex("^```");
    
    if (fenceRegex.match(text).hasMatch()) {
        if (currentState == 1) {
            nextState = 0; // Kapat
        } else {
            nextState = 1; // Aç
        }
    }

    setCurrentBlockState(nextState);

    // Eğer kod bloğu içindeysek veya şu anki satır fence ise
    if (currentState == 1 || nextState == 1) {
        // Tüm satırı boya
        setFormat(0, text.length(), theme.P_GRAY);
        return true; // Kod bloğu içindeyiz, diğer formatları uygulama!
    }
    return false; // Normal metin
}


void MarkdownHighlighter::tags(const QString &text, bool isCurrentLine) {
    // Başlık (# ) ile karışmaması için # işaretinden sonra boşluk OLMAMALI
    QRegularExpression tagRegex("(?<=\\s|^)(#[a-zA-Z0-9_\\-]+)");
    QRegularExpressionMatchIterator i = tagRegex.globalMatch(text);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
                
        // Etiketler genelde oval bir kutu içinde gösterilir (Obsidian stili)
        // Qt'de bunu yapmak için background rengi verip radius veremeyiz (TextEdit kısıtlıdır)
        // Ama arka plan rengi verebiliriz:
        // tagFmt.setBackground(QColor(230, 230, 250)); // Lavender
        
        setFormat(match.capturedStart(), match.capturedLength(), theme.SLATE_BLUE);
    }
}
