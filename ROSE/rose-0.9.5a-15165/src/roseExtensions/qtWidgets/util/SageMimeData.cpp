#include "sage3basic.h"

#include "SageMimeData.h"
#include "AsmToSourceMapper.h"

#include <QDataStream>
#include <QMimeData>
#include <QStringList>
#include <QDebug>

template< typename LinkType >
static void writeLinks( QDataStream &s, AstAttributeMechanism *attrMech, QByteArray &d_ )
{
    if( !attrMech ) return;
    
    for( AstAttributeMechanism::iterator i( attrMech->begin() );
         i != attrMech->end();
         ++i )
    {
        LinkType *attr( dynamic_cast<LinkType *>( i->second ) );

        if( attr )
        {
            for( typename LinkType::iterator j( attr->begin() ); j != attr->end(); ++j )
            {
                SgNode *link( j->first );
                int bytesWritten( s.writeRawData( ( const char * ) &link,
                                                  sizeof( SgNode * ) ) );
                
                assert( bytesWritten == sizeof( SgNode * ) );
            }
        }
    }
}

QMimeData *createSageMimeData( SgNode *node )
{
    QMimeData *data( new QMimeData() );

    // write node into into QByteArray
    QByteArray d;
    QDataStream s( &d, QIODevice::Unbuffered | QIODevice::ReadWrite );
    
    int bytesWritten( s.writeRawData( ( const char * ) & node, sizeof( SgNode * ) ) );
            
    assert( bytesWritten == sizeof( SgNode * ) );

    data->setData( SG_NODE_MIMETYPE, d );

    // if SgNode is a SgProject just return with only general set
    if( isSgProject( node ) )
        return data;
    
    // traverse up until we find a SgFile
    SgNode *parent( node );
    while( !isSgFile( parent ) && parent )
    {
        parent = parent->get_parent();
    }
    
    AstAttributeMechanism *attrMech( node->get_attributeMechanism() );
    
    if( isSgSourceFile( parent ) )
    {
        data->setData( SG_NODE_SOURCE_MIMETYPE, d );

        QByteArray d_;
        QDataStream s_( &d_, QIODevice::Unbuffered | QIODevice::ReadWrite );
        writeLinks<AstBinaryNodeLink>( s_, attrMech, d_ );
        if( !d_.isEmpty() ) data->setData( SG_NODE_BINARY_MIMETYPE, d_ );
    }

    if( isSgBinaryComposite( parent ) )
    {
        data->setData( SG_NODE_BINARY_MIMETYPE, d );
        
        QByteArray d_;
        QDataStream s_( &d_, QIODevice::Unbuffered | QIODevice::ReadWrite );
        writeLinks<AstSourceNodeLink>( s_, attrMech, d_ );
        if( !d_.isEmpty() ) data->setData( SG_NODE_SOURCE_MIMETYPE, d_ );
    }

    return data;
}

SgNode *getGeneralNode( const QMimeData *data )
{
    SgNodeVector res( getNodes( data, SG_NODE_MIMETYPE ) );

    if( res.size() == 0 )
        return NULL;
    else
        return getNodes( data, SG_NODE_MIMETYPE )[0];
}

SgNodeVector getSourceNodes( const QMimeData *data )
{
    return getNodes( data, SG_NODE_SOURCE_MIMETYPE );
}

SgNodeVector getBinaryNodes( const QMimeData *data )
{
    return getNodes( data, SG_NODE_BINARY_MIMETYPE );
}

SgNodeVector getNodes( const QMimeData *data, const QString& type )
{
    SgNodeVector res;
    if( !data || !data->hasFormat( type ) ) return res;

    QByteArray d( data->data( type ) );
    
    QDataStream s (d);

    while( !s.atEnd() )
    {
        SgNode * node = 0;
        int bytesRead = s.readRawData( (char*)&node,sizeof(SgNode*) );
        
        Q_ASSERT( bytesRead == sizeof(SgNode*) );

        res.push_back( node );
    }

    return res;
}
