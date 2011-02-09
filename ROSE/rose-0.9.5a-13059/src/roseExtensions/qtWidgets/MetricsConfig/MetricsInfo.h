
#ifndef METRICSINFO_H
#define METRICSINFO_H

#include <QString>

class SgNode;

/**
 * \brief Stores MetricAttribute Normalization
 */
class MetricsInfo
{
    public:
        MetricsInfo()
            : caption( "" ),
              //isEnabled( isShown_ ),
              minValue( 0.0 ),
              maxValue( 0.0 ),
              minNormalize( 0.0 ),
              maxNormalize( 0.0 ),
              normalizeByRange( true ),
              listId( -1 )
        {}

        MetricsInfo( const QString& name_,/* bool isShown_,*/ double minValue_, double maxValue_, bool normalizeByRange_ = true )
            : //name( name_ ),
              caption( name_ ),
              //isEnabled( isShown_ ),
              minValue( minValue_ ),
              maxValue( maxValue_ ),
              minNormalize( minValue_ ),
              maxNormalize( maxValue_ ),
              normalizeByRange( normalizeByRange_ ),
              listId( -1 )
        {}

        bool operator==( const MetricsInfo& other )
        {
            if( caption == other.caption &&
                minValue == other.minValue &&
                minNormalize == other.minNormalize &&
                maxNormalize == other.maxNormalize &&
                normalizeByRange == other.normalizeByRange &&
                normalizeMetricName == other.normalizeMetricName &&
                listId == other.listId )
                return true;

            return false;
        }

        bool operator!=( const MetricsInfo& other )
        {
            return !( *this == other );
        }
        
        double eval( SgNode *node, const QString& metricName ) const;

        QString caption;
        double minValue;
        double maxValue;
        double minNormalize;
        double maxNormalize;
        bool normalizeByRange;
        QString normalizeMetricName;
        int listId;
};

#endif
