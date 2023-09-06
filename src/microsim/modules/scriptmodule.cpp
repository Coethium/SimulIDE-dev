/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "scriptmodule.h"
#include "scriptstdstring.h"
#include "scriptarray.h"
#include "editorwindow.h"
#include "utils.h"

using namespace std;

void MessageCallback( const asSMessageInfo* msg, void* param )
{
    QString type = " ERROR ";
    if     ( msg->type == asMSGTYPE_WARNING     ) type = " WARNING ";
    else if( msg->type == asMSGTYPE_INFORMATION ) type = " INFO ";

    QString deb = QString( msg->section )+" line: "+QString::number( msg->row )
                  +" "+QString::number( msg->col )+type+QString( msg->message );

    EditorWindow::self()->outPane()->appendLine( deb.remove("\n") );
    //qDebug() << msg->section << "line:" << msg->row << msg->col << type << msg->message;
}

void print( string &msg )
{
    qDebug() << msg.c_str();
}

ScriptModule::ScriptModule( QString name )
            : eElement( name )
{
    m_aEngine = NULL;
    m_context = NULL;

    m_aEngine = asCreateScriptEngine();
    if( m_aEngine == 0 ) { qDebug() << "Failed to create script engine."; return; }

    m_context = m_aEngine->CreateContext();
    if( m_context == 0 ) { qDebug() << "Failed to create the context."; return; }

    m_aEngine->SetEngineProperty( asEP_AUTO_GARBAGE_COLLECT   , false );
    m_aEngine->SetEngineProperty( asEP_BUILD_WITHOUT_LINE_CUES, true );
    m_aEngine->SetEngineProperty( asEP_OPTIMIZE_BYTECODE      , true );

    m_aEngine->SetMessageCallback( asFUNCTION( MessageCallback ), 0, asCALL_CDECL );
    RegisterStdString( m_aEngine );
    RegisterScriptArray( m_aEngine, true );

    m_aEngine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);

    m_jit = NULL;
#ifndef SIMULIDE_W32 // Defined in .pro file for win32
    m_jit = new asCJITCompiler( JIT_NO_SUSPEND | JIT_SYSCALL_NO_ERRORS );
    m_aEngine->SetEngineProperty( asEP_INCLUDE_JIT_INSTRUCTIONS, 1 );
    m_aEngine->SetJITCompiler( m_jit );
#endif
}
ScriptModule::~ScriptModule()
{
    if( m_context ) m_context->Release();
    if( m_aEngine ) m_aEngine->ShutDownAndRelease();
    if( m_jit )     delete m_jit;
}

void ScriptModule::setScriptFile( QString scriptFile, bool )
{
    m_script = fileToString( scriptFile, "ScriptModule::setScriptFile" );
}

void ScriptModule::setScript( QString script )
{
    m_script = script;
}

int ScriptModule::compileScript()
{
    if( !m_aEngine ) return -1;

    std::string script = m_script.toStdString();
    int len = m_script.size();

    asIScriptModule* mod = m_aEngine->GetModule( 0, asGM_ALWAYS_CREATE );
    int r = mod->AddScriptSection("script", &script[0], len );
    if( r < 0 ) { qDebug() << "\nScriptModule::compileScript: AddScriptSection() failed\n"; return -1; }

    r = mod->Build();
    if( r < 0 ) { qDebug() << endl << m_elmId+" ScriptModule::compileScript Error"<< endl; return -1; }

    //qDebug() << "\nScriptModule::compileScript: Build() Success\n";
    return 0;
}

/*int ScriptModule::SaveBytecode(asIScriptEngine *engine, const char *outputFile)
{
    CBytecodeStream stream;
    int r = stream.Open( outputFile );
    if( r < 0 )
    {
        qDebug() << "Failed to open output file for writing";
        return -1;
    }

    asIScriptModule *mod = engine->GetModule("build");
    if( mod == 0 )
    {
        qDebug() << "Failed to retrieve the compiled bytecode";
        return -1;
    }

    r = mod->SaveByteCode( &stream );
    if( r < 0 )
    {
        qDebug() <<  "Failed to write the bytecode";
        return -1;
    }

    qDebug() <<  "Bytecode successfully saved";

    return 0;
}*/

void ScriptModule::callFunction( asIScriptFunction* func )
{
    prepare( func );
    execute();
}

void ScriptModule::execute()
{
    m_status = m_context->Execute();
    if( m_status != asEXECUTION_FINISHED ) printError( m_context );
}

void ScriptModule::printError( asIScriptContext* context ) // The execution didn't finish as we had planned. Determine why.
{
    if( m_status == asEXECUTION_ABORTED )
        qDebug() << "The script was aborted before it could finish. Probably it timed out.";
    else if( m_status == asEXECUTION_EXCEPTION )
    {
        qDebug() << "The script ended with an exception." ;

        // Write some information about the script exception
        asIScriptFunction* func = m_context->GetExceptionFunction();
        qDebug() << "func:" << func->GetDeclaration();
        qDebug() << "modl:" << func->GetModuleName();
        qDebug() << "sect:" << func->GetScriptSectionName();
        qDebug() << "line:" << context->GetExceptionLineNumber();
        qDebug() << "desc:" << context->GetExceptionString();
    }
    else qDebug() << "The script ended for some unforeseen reason:" << m_status;
}
