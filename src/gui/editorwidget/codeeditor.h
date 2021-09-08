/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

#include "highlighter.h"
#include "outpaneltext.h"
#include "compilerprop.h"
#include "ramtable.h"
#include "updatable.h"

enum bebugState_t{
    DBG_STOPPED = 0,
    DBG_PAUSED,
    DBG_STEPING,
    DBG_RUNNING
};

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class BaseDebugger;
class LineNumberArea;
class CompilerProp;

class CodeEditor : public QPlainTextEdit, public Updatable
{
    Q_OBJECT

    public:
        CodeEditor( QWidget* parent, OutPanelText* outPane );
        ~CodeEditor();
        
        bool driveCirc() { return m_driveCirc; }
        void setDriveCirc( bool drive );

        void setFile(const QString filePath);
        QString getFilePath() { return m_file ; }

        void lineNumberAreaPaintEvent( QPaintEvent* event );
        int  lineNumberAreaWidth();
        
        void setCompiled( bool compiled ) { m_isCompiled = compiled; }
        
        bool debugStarted() { return (m_state > DBG_STOPPED); }
        bool initDebbuger();
        void stopDebbuger();
        void lineReached( int line );

        //void setCompilerPath();

        void compProps();

 static int fontSize() { return m_fontSize; }
 static void setFontSize( int size );

 static int tabSize() { return m_tabSize; }
 static void setTabSize( int size );

 static bool showSpaces() { return m_showSpaces; }
 static void setShowSpaces( bool on );

 static bool spaceTabs() { return m_spaceTabs; }
 static void setSpaceTabs( bool on );

    signals:
        void msg( QString text );

    public slots:
        void slotAddBreak() { m_brkAction = 1; }
        void slotRemBreak() { m_brkAction = 2; }
        void slotClearBreak() { m_brkPoints.clear(); }
        void compile( bool debug=false );
        void upload();
        void runToBreak();
        void step( bool over=false );
        void stepOver();
        void pause();
        void reset();

    private slots:
        void updateLineNumberAreaWidth(int) { setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 ); }
        void updateLineNumberArea( const QRect &, int );
        void highlightCurrentLine();

    protected:
        void resizeEvent(QResizeEvent *event);
        void keyPressEvent( QKeyEvent* event );
        void contextMenuEvent(QContextMenuEvent* event);

    private:
        int  getSintaxCoincidences(QString& fileName, QStringList& instructions );
        void addBreakPoint( int line );
        void remBreakPoint( int line ) { m_brkPoints.removeOne( line ); }
        void updateScreen();
        
        void indentSelection( bool unIndent );

        CompilerProp* m_compDialog;
        
        BaseDebugger* m_debugger;
        OutPanelText* m_outPane;

        LineNumberArea *m_lNumArea;
        Highlighter    *m_hlighter;

        QString m_sintaxPath;
        QString m_compilsPath;

        QString m_file;
        QString m_fileDir;
        QString m_fileName;
        QString m_fileExt;
        QString m_help;

        bebugState_t m_state;
        bebugState_t m_resume;
        QList<int> m_brkPoints;

        int m_brkAction;    // 0 = no action, 1 = add brkpoint, 2 = rem brkpoint
        int m_debugLine;
        int m_lastCycle;

        bool m_isCompiled;
 static bool m_showSpaces;
 static bool m_spaceTabs;
 static bool m_driveCirc;

 static QStringList m_picInstr;
 static QStringList m_avrInstr;

 static int   m_fontSize;
 static int   m_tabSize;

 static QString m_tab;

 static QList<CodeEditor*> m_documents;
 static QFont m_font;
};


// ********************* CLASS LineNumberArea **********************************

class LineNumberArea : public QWidget
{
    Q_OBJECT
    
    public:
        LineNumberArea(CodeEditor *editor);
        ~LineNumberArea();

        QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

        int lastPos;

    protected:
        void contextMenuEvent( QContextMenuEvent *event);
        void paintEvent(QPaintEvent *event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

    private:
        CodeEditor *m_codeEditor;
};

#endif
