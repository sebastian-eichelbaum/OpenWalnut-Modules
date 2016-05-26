//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
// For more information see http://www.openwalnut.org/copying
//
// This file is part of OpenWalnut.
//
// OpenWalnut is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWalnut is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with OpenWalnut. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------

#ifndef WTURINGTEXTURECREATOR_H
#define WTURINGTEXTURECREATOR_H

#include <vector>

#include <boost/thread.hpp>

#include <core/common/WThreadedRunner.h>
#include <core/graphicsEngine/WGETexture.h>

class WTuringTextureCreator
{
public:

    WTuringTextureCreator( std::size_t numThreads = boost::thread::hardware_concurrency() );

    ~WTuringTextureCreator();

    osg::ref_ptr< WGETexture3D > create( std::size_t sizeX, std::size_t sizeY, std::size_t sizeZ );

    void setSpotSize( float size );

    void setSpotIrregularity( float irr );

    void setNumIterations( std::size_t iter );

private:

    class TextureThread : public WThreadedRunner
    {
    public:

        TextureThread( std::size_t id, std::size_t max );

        ~TextureThread();

        void setTextureSize( std::size_t sizeX, std::size_t sizeY, std::size_t sizeZ );

        void setSpotFactor( float spotFactor );

        void setDiffusionConstants( float d1, float d2 );

        void setBufferPointers( std::vector< float > const* concentration1, std::vector< float > const* concentration2,
                                std::vector< float > const* noise, std::vector< float >* delta1, std::vector< float >* delta2 );

        virtual void threadMain();

    private:

        std::size_t m_id;

        std::size_t m_maxThreads;

        std::size_t m_sizeX;

        std::size_t m_sizeY;

        std::size_t m_sizeZ;

        float m_spotFactor;

        float m_diffusionConstant1;

        float m_diffusionConstant2;

        std::vector< float > const* m_concentration1;
        std::vector< float > const* m_concentration2;
        std::vector< float > const* m_noise;
        std::vector< float >* m_delta1;
        std::vector< float >* m_delta2;
    };

    float m_numIterations;

    float m_spotIrregularity;

    float m_spotSize;


    std::vector< boost::shared_ptr< TextureThread > > m_threads;
};

#endif  // WTURINGTEXTURECREATOR_H
