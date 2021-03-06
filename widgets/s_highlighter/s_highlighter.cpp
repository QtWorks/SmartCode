#include "s_highlighter.h"

SHighlighter::SHighlighter(QTextDocument *parent): QSyntaxHighlighter(parent)
{
    keywordFormats["keyword"].setForeground(QBrush(QColor(115, 186, 197)));
    //keywordFormats["keyword"].setFontWeight(QFont::Bold);

    keywordFormats["types"].setForeground(QBrush(QColor(64, 118, 206)));

    keywordFormats["operator"].setForeground(QBrush(QColor(115, 186, 197)));

    keywordFormats["operation"].setForeground(QBrush(QColor(0, 0, 0)));

    keywordFormats["singleLineComment"].setForeground(Qt::gray);
    keywordFormats["multiLineComment"].setForeground(Qt::gray);

    keywordFormats["numbers"].setForeground(QBrush(QColor(226, 123, 0)));
    keywordFormats["quotation"].setForeground(QBrush(QColor(226, 123, 0)));

    keywordFormats["varMember"].setForeground(QBrush(QColor(22, 8, 147)));

    //keywordFormats["function"].setForeground(QBrush(QColor(0, 0, 0)));
    keywordFormats["function"].setFontItalic(true);

    keywordFormats["hidding"].setForeground(QBrush(QColor(176, 177, 178)));
}

void SHighlighter::updateHighlightRules(const QString &highlight_page)
{
    QFile hpage(highlight_page);
    if (!hpage.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open file";
        throw std::invalid_argument("Cannot open file");
    }

    QStringList keywordPatterns;
    QStringList typesPatterns;
    QVector<QRegExp> operators;
    QVector<QRegExp> operations;
    QVector<QRegExp> keyexps;
    QVector<QRegExp> typeexps;
    QVector<QRegExp> varmember;
    QVector<QRegExp> hidding;
    while(!hpage.atEnd())
    {
        QString line = hpage.readLine();
        QStringList list = line.split(" ");

        if(list.size() != 2)
            continue;

        QString syntaxType = list[0];
        QString key = list[1].replace("\n", "");

        if(syntaxType == "keyword")
            keywordPatterns << "\\b"+key+"\\b";
        else if(syntaxType == "type")
            typesPatterns << "\\b"+key+"\\b";
        else if(syntaxType == "operator")
            operators.push_back(QRegExp(key));
        else if(syntaxType == "operation")
            operations.push_back(QRegExp(key));
        else if(syntaxType == "keyexp")
            keyexps.push_back(QRegExp(key));
        else if(syntaxType == "typeexp")
            typeexps.push_back(QRegExp(key));
        else if(syntaxType == "varmember")
            varmember.push_back(QRegExp(key));
        else if(syntaxType == "hidding")
            hidding.push_back(QRegExp(key));
    }

    HighlightingRule rule;
    highlightingRules.clear();

    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = keywordFormats["function"];
    highlightingRules.append(rule);

    for(int i = 0; i < varmember.size(); i++)
    {
        highlightingRules.append( HighlightingRule( keywordFormats["varMember"], varmember.at(i)));
    }

    for(int i = 0; i < operations.size(); i++)
    {
        highlightingRules.append( HighlightingRule( keywordFormats["operation"], operations.at(i)));
    }

    for(int i = 0; i < operators.size(); i++)
    {
        highlightingRules.append( HighlightingRule( keywordFormats["operator"], operators.at(i)));
    }

    for(int i = 0; i < hidding.size(); i++)
    {
        highlightingRules.append( HighlightingRule( keywordFormats["hidding"], hidding.at(i)));
    }

    foreach (const QString &pattern, typesPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormats["types"];
        highlightingRules.append(rule);
    }

    for(int i = 0; i < typeexps.size(); i++)
    {
        highlightingRules.append( HighlightingRule( keywordFormats["types"], typeexps.at(i)));
    }

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormats["keyword"];
        highlightingRules.append(rule);
    }

    for(int i = 0; i < keyexps.size(); i++)
    {
        highlightingRules.append( HighlightingRule( keywordFormats["keyword"], keyexps.at(i)));
    }

    foreach (const QString &pattern, typesPatterns)
    {
        rule.pattern = QRegExp("\\b"+pattern+"+\\x005B+\\x005D");
        rule.format = keywordFormats["types"];
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]{1,40}+\\s+\\x003D+class\\s");
    rule.format = classFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegExp("[\\^#]+include\\s");
    rule.format = keywordFormats["keyword"];
    highlightingRules.append(rule);

    rule.pattern = QRegExp("\\b[0-9]+\\b");
    rule.format = keywordFormats["numbers"];
    highlightingRules.append(rule);

    rule.pattern = QRegExp("//[^\n]*");
    rule.format = keywordFormats["singleLineComment"];
    highlightingRules.append(rule);

    rule.pattern = QRegExp("\"[^\"]*\"");
    rule.format = keywordFormats["quotation"];
    highlightingRules.append(rule);


//    rule.pattern = QRegExp("\"[^\"]*");
//    rule.format = keywordFormats["quotation"];
//    highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void SHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);

            if(rule.pattern == QRegExp("\\bclass+\\s+[A-Za-z0-9_]{1,40}\\s"))
            {
                QString new_type = "\\b";
                int start = text.lastIndexOf( QRegExp("\\bclass+\\s"), index );
                for(int i = start+6; i < length-start-1; i++)
                {
                    new_type += text.at(i);
                }
                new_type += "\\b";

                //qDebug() << new_type;

                HighlightingRule new_rule;
                new_rule.pattern = QRegExp(new_type);
                new_rule.format = keywordFormats["types"];
                highlightingRules.append(new_rule);

                new_rule.pattern = QRegExp("\\b"+new_type+"+\\x005B+[amt]+\\x005D");
                new_rule.format = keywordFormats["types"];
                highlightingRules.append(new_rule);
            }
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else
        {
            commentLength = endIndex - startIndex
                    + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, keywordFormats["multiLineComment"] );
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

