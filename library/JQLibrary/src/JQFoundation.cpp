﻿/*
    This file is part of JQLibrary

    Copyright: Jason

    Contact email: 188080501@qq.com

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 2.1 or version 3 as published by the Free
    Software Foundation and appearing in the file LICENSE.LGPLv21 and
    LICENSE.LGPLv3 included in the packaging of this file. Please review the
    following information to ensure the GNU Lesser General Public License
    requirements will be met: https://www.gnu.org/licenses/lgpl.html and
    http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

// JQFoundation header include
#include "JQFoundation.h"

// C++ lib import
#include <iostream>

// Qt lib import
#include <QDebug>
#include <QSharedMemory>
#include <QHash>
#include <QBuffer>
#include <QMetaMethod>
#include <QImage>
#include <QTextCursor>
#include <QPalette>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QUuid>
#include <QTimer>
#include <QTime>
#include <QDateTime>
#include <QTextStream>

// Windows lib import
#ifdef Q_OS_WIN
#   include <windows.h>
#endif

using namespace JQFoundation;

QString JQFoundation::hashString(const QByteArray &key, const QCryptographicHash::Algorithm &algorithm)
{
    return QCryptographicHash::hash( key, algorithm ).toHex();
}

QString JQFoundation::variantToString(const QVariant &value)
{
    QString result;

    if ( ( value.type() == 31 ) || ( value.type() == 51 ) || ( value.type() == QVariant::Invalid ) ) { return "NULL"; }

    switch ( value.type() )
    {
        case QVariant::Bool:
        {
            result = ( ( value.toBool() ) ? ( "1" ) : ( "0" ) );
            break;
        }
        case QVariant::ByteArray:
        {
            result = QString( "\\x" );
            result += value.toByteArray().toHex();
            break;
        }
        case QVariant::String:
        {
            result = value.toString();
            break;
        }
        case QVariant::Int:
        case QVariant::Double:
        {
            result = QString::number( value.toDouble(), 'f', 8 );
            while ( result.endsWith( '0' ) )
            {
                result = result.mid( 0, result.size() - 1 );
            }
            if ( result.endsWith( '.' ) )
            {
                result = result.mid( 0, result.size() - 1 );
            }
            if ( result == "" )
            {
                result = "0";
            }

            break;
        }
        default:
        {
            if ( value.type() == QVariant::nameToType( "QJsonValue" ) )
            {
                const auto &&jsonValue = value.value< QJsonValue >();

                switch ( jsonValue.type() )
                {
                    case QJsonValue::Null:
                    {
                        result = "NULL";
                        break;
                    }
                    case QJsonValue::Bool:
                    {
                        result = ( ( jsonValue.toBool() ) ? ( "1" ) : ( "0" ) );
                        break;
                    }
                    case QJsonValue::String:
                    {
                        result = jsonValue.toString();
                        break;
                    }
                    case QJsonValue::Double:
                    {
                        result = QString::number( jsonValue.toDouble(), 'f', 8 );
                        while ( result.endsWith( '0' ) )
                        {
                            result = result.mid( 0, result.size() - 1 );
                        }
                        if ( result.endsWith( '.' ) )
                        {
                            result = result.mid( 0, result.size() - 1 );
                        }
                        if ( result == "" )
                        {
                            result = "0";
                        }

                        break;
                    }
                    default:
                    {
                        qDebug() << "JQFoundation::variantToString: unexpected json type:" << jsonValue;
                        result = jsonValue.toString();
                        break;
                    }
                }
            }
            else
            {
                qDebug() << "JQFoundation::variantToString: unexpected type:" << value;
                result = value.toString();
            }
            break;
        }
    }

    return result;
}

QString JQFoundation::createUuidString()
{
    return QUuid::createUuid().toString().mid( 1, 36 );
}

QJsonObject JQFoundation::jsonFilter(const QJsonObject &source, const QStringList &leftKey, const QJsonObject &mix)
{
    QJsonObject result;

    for ( const auto &key: leftKey )
    {
        auto buf = source.find( key );
        if ( buf != source.end() )
        {
            result[ buf.key() ] = buf.value();
        }
    }

    if ( !mix.isEmpty() )
    {
        for ( auto it = mix.begin(); it != mix.end(); ++it )
        {
            result.insert( it.key(), it.value() );
        }
    }

    return result;
}

QJsonObject JQFoundation::jsonFilter(const QJsonObject &source, const char *leftKey, const QJsonObject &mix)
{
    return JQFoundation::jsonFilter( source, QStringList( { leftKey } ), mix );
}

QVariantList JQFoundation::listVariantMapToVariantList(const QList< QVariantMap > &source)
{
    QVariantList result;

    for ( const auto &data: source )
    {
        result.push_back( data );
    }

    return result;
}

QVariantMap JQFoundation::mapKeyTranslate(const QVariantMap &source, const QMap< QString, QString > &keyMap)
{
   QVariantMap result;

    for ( auto sourceIt = source.begin(); sourceIt != source.end(); ++sourceIt )
    {
        const auto &&keyMapIt = keyMap.find( sourceIt.key() );
        if ( keyMapIt == keyMap.end() ) { continue; }

        result[ keyMapIt.value() ] = sourceIt.value();
    }

    return result;
}

QVariantList JQFoundation::listKeyTranslate(const QVariantList &source, const QMap< QString, QString > &keyMap)
{
    QVariantList result;

    for ( const auto &data: source )
    {
        result.push_back( mapKeyTranslate( data.toMap(), keyMap ) );
    }

    return result;
}

QList< QVariantMap > JQFoundation::listKeyTranslate(const QList< QVariantMap > &source, const QMap<QString, QString> &keyMap)
{
    QList< QVariantMap > result;

    for ( const auto &data: source )
    {
        result.push_back( mapKeyTranslate( data, keyMap ) );
    }

    return result;
}

QSharedPointer< QTimer > JQFoundation::setTimerCallback(const int &interval, const std::function<void (bool &continueFlag)> &callback, const bool &callbackOnStart)
{
    QSharedPointer< QTimer > timer( new QTimer );

    QObject::connect( timer.data(), &QTimer::timeout, [ timer, callback ]()
    {
        bool continueFlag = true;
        callback( continueFlag );
        if ( continueFlag )
        {
            timer->start();
        }
    } );

    timer->setInterval( interval );
    timer->setSingleShot( true );

    if ( callbackOnStart )
    {
        bool continueFlag = true;
        callback( continueFlag );
        if ( continueFlag )
        {
            timer->start();
        }
    }
    else
    {
        timer->start();
    }

    return timer;
}

void JQFoundation::setDebugOutput(const QString &rawTargetFilePath_, const bool &argDateFlag_)
{
    static QString rawTargetFilePath;
    static bool argDateFlag;

    rawTargetFilePath = rawTargetFilePath_;
    argDateFlag = argDateFlag_;

    class HelperClass
    {
    public:
        static void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &message_)
        {
            QString message;

            switch ( type )
            {
                case QtDebugMsg:
                {
                    message = message_;
                    break;
                }
                case QtWarningMsg:
                {
                    message.append( "Warning: " );
                    message.append( message_ );
                    break;
                }
                case QtCriticalMsg:
                {
                    message.append( "Critical: " );
                    message.append( message_ );
                    break;
                }
                case QtFatalMsg:
                {
                    message.append( "Fatal: " );
                    message.append( message_ );
                    break;
                }
                default: { break; }
            }

            QString currentTargetFilePath;

            if ( argDateFlag )
            {
                currentTargetFilePath = rawTargetFilePath.arg( ( ( argDateFlag ) ? ( QDateTime::currentDateTime().toString("yyyy_MM_dd") ) : ( "" ) ) );
            }
            else
            {
                currentTargetFilePath = rawTargetFilePath;
            }

            if ( !QFileInfo::exists( currentTargetFilePath ) )
            {
                QDir().mkpath( QFileInfo( currentTargetFilePath ).path() );
            }

            QFile file( currentTargetFilePath );
            file.open( QIODevice::WriteOnly | QIODevice::Append );

            QTextStream textStream( &file );
            textStream << QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) << ": " << message << endl;
        }
    };

    qInstallMessageHandler( HelperClass::messageHandler );
}

void JQFoundation::openDebugConsole()
{
#ifdef Q_OS_WIN
    class HelperClass
    {
    public:
        static void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &message_)
        {
            QString message;

            switch ( type )
            {
                case QtDebugMsg:
                {
                    message = message_;
                    break;
                }
                case QtWarningMsg:
                {
                    message.append( "Warning: " );
                    message.append( message_ );
                    break;
                }
                case QtCriticalMsg:
                {
                    message.append( "Critical: " );
                    message.append( message_ );
                    break;
                }
                case QtFatalMsg:
                {
                    message.append( "Fatal: " );
                    message.append( message_ );
                    break;
                }
                default: { break; }
            }

            std::cout << QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ).toUtf8().data()
                      << ": " << message.toUtf8().data() << std::endl;
        }
    };

    qInstallMessageHandler( HelperClass::messageHandler );

    AllocConsole();
#endif
}

#if !(defined Q_OS_IOS) && !(defined Q_OS_ANDROID) && !(defined Q_OS_WINPHONE)
bool JQFoundation::singleApplication(const QString &flag)
{
    static QSharedMemory *shareMem = nullptr;

    if (shareMem)
    {
        return true;
    }

    shareMem = new QSharedMemory( "JQFoundationSingleApplication_" + flag );

    for ( auto count = 0; count < 2; ++count )
    {
        if (shareMem->attach( QSharedMemory::ReadOnly ))
        {
            shareMem->detach();
        }
    }

    if ( shareMem->create( 1 ) )
    {
        return true;
    }

    return false;
}
#else
bool JQFoundation::singleApplication(const QString &)
{
    return true;
}
#endif

#if !(defined Q_OS_IOS) && !(defined Q_OS_ANDROID) && !(defined Q_OS_WINPHONE)
bool JQFoundation::singleApplicationExist(const QString &flag)
{
    QSharedMemory shareMem( "JQFoundationSingleApplication_" + flag );

    for ( auto count = 0; count < 2; ++count )
    {
        if (shareMem.attach( QSharedMemory::ReadOnly ))
        {
            shareMem.detach();
        }
    }

    if ( shareMem.create( 1 ) )
    {
        return false;
    }

    return true;
}
#else
bool JQFoundation::singleApplicationExist(const QString &)
{
    return false;
}
#endif

QByteArray JQFoundation::pixmapToByteArray(const QPixmap &pixmap, const QString &format, int quality)
{
    QByteArray bytes;
    QBuffer buffer( &bytes );

    buffer.open( QIODevice::WriteOnly );
    pixmap.save( &buffer, format.toLatin1().data(), quality );

    return bytes;
}

QByteArray JQFoundation::imageToByteArray(const QImage &image, const QString &format, int quality)
{
    QByteArray bytes;
    QBuffer buffer( &bytes );

    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer, format.toLatin1().data(), quality );

    return bytes;
}

QString JQFoundation::snakeCaseToCamelCase(const QString &source)
{
    const auto &&splitList = source.split( '_', QString::SkipEmptyParts );
    QString result;

    for ( const auto &splitTag: splitList )
    {
        if ( result.isEmpty() || ( splitTag.size() == 1 ) )
        {
            result += splitTag;
        }
        else
        {
            result += splitTag[ 0 ].toUpper();
            result += splitTag.mid( 1 );
        }
    }

    return result;
}

#if ( ( defined Q_OS_MAC ) && !( defined Q_OS_IOS ) ) || ( defined Q_OS_WIN ) || ( defined Q_OS_LINUX )
QPair< int, QByteArray > JQFoundation::startProcessAndReadOutput(const QString &program, const QStringList &arguments, const int &maximumTime)
{
    QPair< int, QByteArray > reply;

    QProcess process;
    process.setProgram( program );
    process.setArguments( arguments );
    process.start();

    QObject::connect( &process, static_cast< void(QProcess::*)(int) >( &QProcess::finished ), [ &reply ](const int &exitCode)
    {
        reply.first = exitCode;
    } );
    QObject::connect( &process, &QIODevice::readyRead, [ &process, &reply ]()
    {
        reply.second.append( process.readAll() );
    } );

    process.waitForFinished( maximumTime );

    return reply;
}
#endif
