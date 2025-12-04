#ifndef H_THEME
#define  H_THEME

#include <QTextCharFormat>

// Renk Paleti ve Stiller Yapısı
struct MarkdownTheme {
    QTextCharFormat HIDDEN;
    
    QTextCharFormat DARKGRAY;
    QTextCharFormat B_DARKGRAY;
    
    QTextCharFormat GRAY;
    QTextCharFormat B_GRAY;
    QTextCharFormat I_GRAY;
    QTextCharFormat S_GRAY;
    QTextCharFormat P_GRAY;
    
    QTextCharFormat BLUE;
    QTextCharFormat B_BLUE;
    QTextCharFormat L_BLUE; // Link Blue
    QTextCharFormat SLATE_BLUE; // Tags
    
    QTextCharFormat RED;
    QTextCharFormat B_RED;
    QTextCharFormat C_RED; // Code Red
    
    QTextCharFormat GREEN;
    QTextCharFormat B_GREEN;
    
    QTextCharFormat ORANGE;
    QTextCharFormat B_ORANGE;
    
    QTextCharFormat PURPLE;
    QTextCharFormat B_PURPLE;
    QTextCharFormat L_PURPLE;
    
    QTextCharFormat TEAL;
    QTextCharFormat LINE;

    // Yapıcı Fonksiyon: Tüm atamalar burada yapılır
    MarkdownTheme() {
        // --- GİZLİ ---
        HIDDEN.setForeground(Qt::transparent);
        HIDDEN.setFontPointSize(1);

        // --- GRİ TONLARI ---
        DARKGRAY.setForeground(QColor(220, 220, 220));
        
        B_DARKGRAY.setForeground(QColor(220, 220, 220));
        B_DARKGRAY.setFontWeight(QFont::Bold);

        GRAY.setForeground(QColor(200, 200, 200));
        
        B_GRAY.setForeground(QColor(200, 200, 200));
        B_GRAY.setFontWeight(QFont::Bold);
        
        I_GRAY.setForeground(QColor(200, 200, 200));
        I_GRAY.setFontItalic(true);
        
        S_GRAY.setForeground(QColor(200, 200, 200));
        S_GRAY.setFontStrikeOut(true);
        
        P_GRAY.setForeground(QColor(160, 160, 160));

        // --- MAVİLER ---
        BLUE.setForeground(QColor(200, 200, 255));
        
        B_BLUE.setForeground(QColor(200, 200, 255));
        B_BLUE.setFontWeight(QFont::Bold);

        L_BLUE.setForeground(QColor(40, 40, 140));
        L_BLUE.setFontUnderline(true);

        SLATE_BLUE.setForeground(QColor(106, 90, 205));
        // Not: Sabit font boyutu vermek bazen sorun olabilir, editör fontuna göre ölçeklemek daha iyidir.
        // Şimdilik orijinal kodunu korudum:
        QFont tagFont("Consolas", 8); 
        SLATE_BLUE.setFont(tagFont);

        // --- KIRMIZILAR ---
        RED.setForeground(QColor(255, 200, 200));
        
        B_RED.setForeground(QColor(255, 200, 200));
        B_RED.setFontWeight(QFont::Bold);

        // Code Bloğu (Orijinal kodunda iki kere setForeground vardı, ilkini background yaptım)
        // Genelde kod bloklarının arkası koyu, yazısı renkli olur.
        C_RED.setBackground(QColor(40, 40, 40));  // Arka plan (Orijinal kodundaki ilk satır)
        C_RED.setForeground(QColor(200, 50, 50)); // Yazı Rengi

        // --- YEŞİLLER ---
        GREEN.setForeground(QColor(200, 255, 200));
        
        B_GREEN.setForeground(QColor(200, 255, 200));
        B_GREEN.setFontWeight(QFont::Bold);

        // --- TURUNCULAR ---
        ORANGE.setForeground(QColor(255, 140, 0));
        
        B_ORANGE.setForeground(QColor(255, 140, 0));
        B_ORANGE.setFontWeight(QFont::Bold);

        // --- MORLAR ---
        PURPLE.setForeground(QColor(138, 43, 226));
        
        B_PURPLE.setForeground(QColor(138, 43, 226));
        B_PURPLE.setFontWeight(QFont::Bold);
        
        L_PURPLE.setForeground(QColor(138, 43, 226));
        L_PURPLE.setFontUnderline(true);

        // --- DİĞER ---
        TEAL.setForeground(QColor(0, 128, 128));

        // Yatay Çizgi (HR)
        LINE.setForeground(Qt::transparent);
        LINE.setFontStrikeOut(true);
        LINE.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        LINE.setUnderlineColor(Qt::gray);
    }
};

#endif
