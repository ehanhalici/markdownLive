#ifndef CODEEDITOR_HPP
#define CODEEDITOR_HPP

#include <QTextEdit>
#include <QWidget>
#include <QSet>

class CodeEditor; // Forward declaration

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor); 

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    CodeEditor *codeEditor;
};

class CodeEditor : public QTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void processLineNumberAreaEvent(QMouseEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
    QSet<int> foldedBlocks;

    void toggleFold(int blockNumber);
    int getHeaderLevel(const QString &text);
};

#endif // CODEEDITOR_HPP
