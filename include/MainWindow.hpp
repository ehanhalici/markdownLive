#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QToolBar>
#include <QAction>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTextEdit>
#include <QSplitter>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCharFormat>
#include <QRegularExpressionMatchIterator>
#include <cstdio>
#include <qcolor.h>
#include <qobject.h>
#include <qregularexpression.h>


#include <QStackedWidget> // <--- YENİ: Katman yönetimi için
#include <QLabel>         // <--- YENİ: Resmi göstermek için
#include <QScrollArea>    // <--- YENİ: Büyük resimlerde kaydırma çubuğu için

#include <QTabWidget>  // <--- EKLENDİ
#include <QListWidget> // <--- EKLENDİ

#include <QListWidget>
#include <QDirIterator>

#include "Theme.hpp"

// --- YENİ SINIF: Markdown Boyayıcı ---
class MarkdownHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    MarkdownHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {}
    
    // Editörün kendisine ihtiyacımız var (Cursor konumunu bilmek için)
    void setEditor(QTextEdit *editor) { m_editor = editor; }

protected:
    void highlightBlock(const QString &text) override;
private:
    QTextEdit *m_editor = nullptr;
    void boldRegex(const QString &text, bool isCurrentLine);
    void header(const QString &text, bool isCurrentLine);
    void italic(const QString &text, bool isCurrentLine);
    void internalLink(const QString &text, bool isCurrentLine);
    void externalLink(const QString &text, bool isCurrentLine);
    void code(const QString &text, bool isCurrentLine);
    void taskList(const QString &text, bool isCurrentLine);
    void highlight(const QString &text, bool isCurrentLine);
    void strike(const QString &text, bool isCurrentLine);
    void quote(const QString &text, bool isCurrentLine);
    void wikiLink(const QString &text, bool isCurrentLine);
    void image(const QString &text, bool isCurrentLine);
    void math(const QString &text, bool isCurrentLine);
    bool fencedCodeBlock(const QString &text);
    void tags(const QString &text, bool isCurrentLine);

    MarkdownTheme theme;
};
// -------------------------------------

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFolder();
    void onFileClicked(const QModelIndex &index);
    void onCursorPositionChanged(); // <--- YENİ SLOT (İmleç takibi için)
    void onSearchResultClicked(QListWidgetItem *item);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    void setupUi();
    void createLeftTab();
    void createRightPanel();
    
    void DebugOpenFile();
    void DebugOpenFolder();

    void openFile(const QString &filePath);
    void gotoInternalPage(QString &text, int positionInBlock);
    void searchTags(QString &text);
    void performGlobalSearch(const QString &searchTerm);
    
    QSplitter *mainSplitter;
    QFileSystemModel *fileModel;    
    MarkdownHighlighter *highlighter; // ana metin goruntuleyici

    QStackedWidget *rightPanelStack; // Sağ paneli tutan ana kapsayıcı
    QTextEdit *editor;               // Katman 1: Editör
    QLabel *imageLabel;              // Katman 2: Resim Alanı
    QScrollArea *imageScrollArea;    // Resmi kaydırmak için çerçeve

    // --- SOL PANEL DEĞİŞİKLİĞİ ---
    QTabWidget *leftTabWidget;    // Sol paneli tutan sekmeli yapı
    QTreeView *fileTreeView;      // Tab 1: Dosya Ağacı
    QListWidget *searchResultList;// Tab 2: Arama Sonuçları (Boş liste)
};

#endif // MAINWINDOW_H
