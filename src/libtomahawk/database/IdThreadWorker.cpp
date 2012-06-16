/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2012 Leo Franchi <lfranchi@kde.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "IdThreadWorker.h"

#include "Artist.h"
#include "Album.h"
#include "Database.h"
#include "DatabaseImpl.h"

using namespace Tomahawk;

namespace {
    enum QueryType {
        ArtistType,
        AlbumType
    };
}


static QWaitCondition s_waitCond;
static QMutex s_mutex;
//static QSet< boost::promise< unsigned int >* > s_cachedPromises;

struct QueueItem
{
    boost::promise< unsigned int > promise;
    artist_ptr artist;
    album_ptr album;
    QueryType type;
    bool create;
};

// TODO Q_GLOBAL_STATIC
QQueue< QueueItem* > IdThreadWorker::s_workQueue = QQueue< QueueItem* >();

IdThreadWorker::IdThreadWorker( Database* db )
    : QThread()
    , m_db( db )
    , m_stop( false )
{
    moveToThread( this );
}


IdThreadWorker::~IdThreadWorker()
{
    wait();
}


void
IdThreadWorker::stop()
{
    {
        QMutexLocker l( &s_mutex );
        m_stop = true;
    }

    s_waitCond.wakeOne();
}


QueueItem*
internalGet( const artist_ptr& artist, const album_ptr& album, bool autoCreate, QueryType type )
{
    QueueItem* item = new QueueItem;
    item->artist = artist;
    item->album = album;
    item->type = type;
    item->create = autoCreate;

    return item;
}


boost::unique_future< unsigned int >
IdThreadWorker::getArtistId( const artist_ptr& artist, bool autoCreate )
{
    QueueItem* item = internalGet( artist, album_ptr(), autoCreate, ArtistType );

//    qDebug() << "QUEUEING ARTIST:" << artist->name();
    s_mutex.lock();
    s_workQueue.enqueue( item );
    s_mutex.unlock();
    s_waitCond.wakeOne();
//    qDebug() << "DONE WOKE UP THREAD:" << artist->name();

    return item->promise.get_future();
}


boost::unique_future< unsigned int >
IdThreadWorker::getAlbumId( const album_ptr& album, bool autoCreate )
{
    QueueItem* item = internalGet( artist_ptr(), album, autoCreate, AlbumType );

//    qDebug() << "QUEUEING ALUBM:" << album->artist()->name() << album->name();
    s_mutex.lock();
    s_workQueue.enqueue( item );
    s_mutex.unlock();
    s_waitCond.wakeOne();
//    qDebug() << "DONE WOKE UP THREAD:" << album->artist()->name() << album->name();

    return item->promise.get_future();
}


void
IdThreadWorker::run()
{
    m_impl = m_db->impl()->clone();

    while ( !m_stop )
    {
        s_mutex.lock();
//        qDebug() << "IdWorkerThread waiting on condition...";
        s_waitCond.wait( &s_mutex );
//        qDebug() << "IdWorkerThread WOKEN UP";
        if ( !s_workQueue.isEmpty() )
        {
            QueueItem* item = s_workQueue.dequeue();
            s_mutex.unlock();

//            qDebug() << "WITH CONTENTS:" << (item->type == ArtistType ? item->artist->name() :  item->album->artist()->name() + " _ " + item->album->name());
            if ( item->type == ArtistType )
            {
                unsigned int id = m_impl->artistId( item->artist->name(), item->create );
                item->promise.set_value( id );

                item->artist->id();
                delete item;
            }
            else if ( item->type == AlbumType )
            {
                unsigned int artistId = m_impl->artistId( item->album->artist()->name(), item->create );
                unsigned int albumId = m_impl->albumId( artistId, item->album->name(), item->create );
                item->promise.set_value( albumId );

                item->album->id();
                delete item;
            }
        }
        else
        {
            s_mutex.unlock();
        }
    }
}
