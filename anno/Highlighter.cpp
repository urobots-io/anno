// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "Highlighter.h"
#include <QPalette>

Highlighter::Highlighter(QTextDocument *parent, const QPalette & palette, Type type)
    : QSyntaxHighlighter(parent)
{
    int h, s, v;
    palette.base().color().getHsv(&h, &s, &v);
    bool dark = v < 100;

    HighlightingRule rule;

    keywordFormat.setForeground(dark ? Qt::cyan : Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;

    switch (type) {
    case JSon:
        break;
    case JScript:
        keywordPatterns
            << "\\babstract\\b" << "\\barguments\\b" << "\\bawait\\b" << "\\bboolean\\b"
            << "\\bbreak\\b" << "\\bbyte\\b" << "\\bcase\\b" << "\\bcatch\\b"
            << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b" << "\\bcontinue\\b"
            << "\\bdebugger\\b" << "\\bdefault\\b" << "\\bdelete\\b" << "\\bdo\\b"
            << "\\bdouble\\b" << "\\belse\\b" << "\\benum\\b" << "\\beval\\b"
            << "\\bexport\\b" << "\\bextends\\b" << "\\bfalse\\b" << "\\bfinal\\b"
            << "\\bfinally\\b" << "\\bfloat\\b" << "\\bfor\\b" << "\\bfunction\\b"
            << "\\bgoto\\b" << "\\bif\\b" << "\\bimplements\\b" << "\\bimport\\b"
            << "\\bin\\b" << "\\binstanceof\\b" << "\\bint\\b" << "\\binterface\\b"
            << "\\blet\\b" << "\\blong\\b" << "\\bnative\\b" << "\\bnew\\b"
            << "\\bnull\\b" << "\\bpackage\\b" << "\\bprivate\\b" << "\\bprotected\\b"
            << "\\bpublic\\b" << "\\breturn\\b" << "\\bshort\\b" << "\\bstatic\\b"
            << "\\bsuper\\b" << "\\bswitch\\b" << "\\bsynchronized\\b" << "\\bthis\\b"
            << "\\bthrow\\b" << "\\bthrows\\b" << "\\btransient\\b" << "\\btrue\\b"
            << "\\btry\\b" << "\\btypeof\\b" << "\\bvar\\b" << "\\bvoid\\b"
            << "\\bvolatile\\b" << "\\bwhile\\b" << "\\bwith\\b" << "\\byield\\b";
    }

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(dark ? Qt::magenta : Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(dark ? Qt::red : Qt::darkRed);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    numberFormat.setForeground(dark ? Qt::green : Qt::red);
    rule.pattern = QRegularExpression(R"([\+\-]?\b[0-9]+\.?[0-9]*|\btrue\b|\bfalse\b)");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    scriptFormat.setForeground(dark ? Qt::yellow : Qt::darkYellow);

    singleLineCommentFormat.setForeground(dark ? Qt::gray : Qt::darkGreen);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::gray);

    commentStartExpression = QRegularExpression("/\\*");    
    commentEndExpression = QRegularExpression("\\*/");

    codeStartExpression = QRegularExpression("{{");
    codeEndExpression = QRegularExpression("}}");
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    bool code = false;

    int startIndex = 0;
    if (previousBlockState() != 1 && previousBlockState() != 2) {
        startIndex = text.indexOf(commentStartExpression);            

        if (startIndex == -1) {
            startIndex = text.indexOf(codeStartExpression);
            code = true;
        }
    } else {
        code = previousBlockState() == 2;
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch match = code
                ? codeEndExpression.match(text, startIndex)
                : commentEndExpression.match(text, startIndex);

        int endIndex = match.capturedStart();

        int blockLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(code ? 2 : 1);
            blockLength = text.length() - startIndex;
        } else {
            blockLength = endIndex - startIndex + match.capturedLength();
        }

        if (code) setFormat(startIndex, blockLength, scriptFormat);
        else setFormat(startIndex, blockLength, multiLineCommentFormat);

        auto oldStartIndex = startIndex;

        startIndex = text.indexOf(commentStartExpression, oldStartIndex + blockLength);
        if (startIndex >= 0) {
            code = false;
        }
        else {
            code = true;
            startIndex = text.indexOf(codeStartExpression, oldStartIndex + blockLength);
        }
    }
}

