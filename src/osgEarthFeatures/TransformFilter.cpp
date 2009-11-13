/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2009 Pelican Ventures, Inc.
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include <osgEarthFeatures/TransformFilter>

using namespace osgEarth;
using namespace osgEarthFeatures;

TransformFilter::TransformFilter(const SpatialReference* outputSRS, bool makeGeocentric ) :
_outputSRS( outputSRS ),
_makeGeocentric( makeGeocentric )
{
    //NOP
}

bool
TransformFilter::push( Feature* input, const FilterContext& context )
{
    for( int p=0; p<input->getNumParts(); p++ )
    {
        osg::Vec3dArray* part = input->getPart( p );
        bool success = _outputSRS->transformPoints( context.profile()->getSRS(), part, false );
        if ( !success )
            return false;

        if ( _makeGeocentric && _outputSRS->isGeographic() )
        {
            const osg::EllipsoidModel* em = context.profile()->getSRS()->getEllipsoid();
            for( int i=0; i<part->size(); i++ )
            {
                double x, y, z;
                em->convertLatLongHeightToXYZ(
                    osg::DegreesToRadians( (*part)[i].y() ),
                    osg::DegreesToRadians( (*part)[i].x() ),
                    (*part)[i].z(),
                    x, y, z );
                (*part)[i].set( x, y, z );
            }
        }
    }

    return true;
}

FilterContext
TransformFilter::push( FeatureList& input, const FilterContext& context )
{
    bool ok = true;
    for( FeatureList::iterator i = input.begin(); i != input.end(); i++ )
        if ( !push( i->get(), context ) )
            ok = false;

    FilterContext outputCx( context );

    outputCx.isGeocentric() = _makeGeocentric;

    outputCx.profile() = new FeatureProfile(
        _outputSRS.get(),
        context.profile()->getGeometryType(),
        context.profile()->getDimensionality(),
        context.profile()->isMultiGeometry() );

    return outputCx;
}