// CppSyntaxHighlighter.cpp
#include "syntaxhighlightercpp.h"

SyntaxHighlighterCPP::SyntaxHighlighterCPP(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keyword format
    keywordFormat.setForeground(QColor(86, 156, 214)); // Blue
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bclass\\b" << "\\bconst\\b" << "\\benum\\b"
                    << "\\bexplicit\\b" << "\\bfriend\\b" << "\\binline\\b"
                    << "\\bnamespace\\b" << "\\boperator\\b" << "\\bprivate\\b"
                    << "\\bprotected\\b" << "\\bpublic\\b" << "\\bsignals\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bvirtual\\b" << "\\bvolatile\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b"
                    << "\\bdo\\b" << "\\bswitch\\b" << "\\bcase\\b" << "\\bdefault\\b"
                    << "\\bbreak\\b" << "\\bcontinue\\b" << "\\breturn\\b"
                    << "\\btry\\b" << "\\bcatch\\b" << "\\bthrow\\b"
                    << "\\bint\\b" << "\\bfloat\\b" << "\\bdouble\\b" << "\\bchar\\b"
                    << "\\bbool\\b" << "\\bvoid\\b" << "\\bauto\\b" << "\\blong\\b"
                    << "\\bshort\\b" << "\\bunsigned\\b" << "\\bsigned\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Class name format
    classFormat.setForeground(QColor(78, 201, 176)); // Teal
    classFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Single line comment format
    singleLineCommentFormat.setForeground(QColor(106, 153, 85)); // Green
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Multi-line comment format
    multiLineCommentFormat.setForeground(QColor(106, 153, 85)); // Green

    // Quotation format
    quotationFormat.setForeground(QColor(206, 145, 120)); // Orange
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Function format
    functionFormat.setForeground(QColor(220, 220, 170)); // Yellow
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Number format
    numberFormat.setForeground(QColor(181, 206, 168)); // Light green
    rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Preprocessor format
    preprocessorFormat.setForeground(QColor(155, 155, 155)); // Gray
    rule.pattern = QRegularExpression("^\\s*#.*");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighterCPP::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multi-line comments
    setCurrentBlockState(0);
    QRegularExpression startExpression("/\\*");
    QRegularExpression endExpression("\\*/");

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = endExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}
