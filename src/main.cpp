#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // --- OBSIDIAN TARZI DARK THEME BAŞLANGIÇ ---
    app.setStyle("Fusion"); // Modern bir görünüm motoru

    QString obsidianStyle = R"(
        /* GENEL AYARLAR */
        QMainWindow, QWidget {
            background-color: #181818;  /* Koyu Gri Arka Plan */
            color: #dcddde;             /* Açık Gri Yazı */
            font-family: "Segoe UI", "Cantarell", sans-serif;
            font-size: 12pt;
        }


        /* TOOLBAR (ARAÇ ÇUBUĞU) */
        QToolBar {
            background-color: #181818;  /* Ana arka planla aynı */
            border-bottom: 1px solid #202020; /* Editörden ayıran ince çizgi */
            spacing: 1px; /* Butonlar arası boşluk */
            padding: 1px;
            border: none;
        }

        /* Toolbar Tutacağı (Sürüklenebilir kısım) */
        QToolBar::handle {
            background: #181818;
            width: 2px;
            margin: 4px;
        }

        /* TOOLBAR BUTONLARI */
        QToolBar QToolButton {
            color: #dcddde;          /* Yazı rengi */
            background-color: transparent; /* Arkası şeffaf olsun */
            border: 1px solid transparent;
            border-radius: 4px;      /* Köşeleri yuvarlat */
            padding: 5px 10px;       /* Buton içi boşluk */
            font-weight: bold;
        }

        /* Mouse Üzerine Gelince (Hover) */
        QToolBar QToolButton:hover {
            background-color: #202020; /* Hafif aydınlat */
            border: 1px solid #252525; /* İnce çerçeve */
        }

        /* Tıklayınca (Pressed) */
        QToolBar QToolButton:pressed {
            background-color: #131313; /* İçe gömülme efekti */
            border: 1px solid #303030;
        }
        
        /* Buton devre dışıysa (Disabled) */
        QToolBar QToolButton:disabled {
            color: #303030;
        }


        /* SOL PANEL (TABLAR) */
        QTabWidget::pane {
            border: 1px solid #202020;
            background-color: #181818;
        }
        QTabBar::tab {
            background: #181818;
            color: #888888;
            padding: 8px 15px;
            border-top-left-radius: 1px;
            border-top-right-radius: 1px;
        }
        QTabBar::tab:selected {
            background: #181818;
            color: #dcddde;
            border-bottom: 1px solid #7a4fd6; /* Obsidian Moru Çizgi */
        }

        /* DOSYA AĞACI VE LİSTE */
        QTreeView, QListView {
            background-color: #181818; /* Daha koyu panel */
            border: 1px;
            color: #bababa;
            selection-background-color: #212121; /* Seçili satır */
            selection-color: #ffffff;
        }
        QHeaderView::section {
            background-color: #181818;
            color: #bababa;
            border: none;
        }

        /* METİN EDİTÖRÜ */
        QTextEdit {
            background-color: #181818; /* VS Code/Obsidian Editor Rengi */
            color: #bababa;
            border: none;
            selection-background-color: #2550B0; /* Metin seçimi mavisi */
            selection-color: #ffffff;
        }

        /* SCROLL BAR (Modern İnce Çubuk) */
        QScrollBar:vertical {
            border: 1px;
            background: #181818;
            width: 1px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #181818;
            min-height: 1px;
            border-radius: 1px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        /* SPLITTER (Ayraç) */
        QSplitter::handle {
            background-color: #181818;
            border: 1px;
            border-radius: 1px;
        }
    )";

    app.setStyleSheet(obsidianStyle);
    // --- DARK THEME BİTİŞ ---

    MainWindow window;
    window.show();

    return app.exec();
}
