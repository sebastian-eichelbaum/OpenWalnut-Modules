//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>

#include <osg/BoundingSphere>
#include <osg/Vec3>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Drawable>

#include "core/common/WPropertyHelper.h"
#include "core/common/math/WMath.h"
#include "core/dataHandler/WDataHandler.h"
#include "core/dataHandler/WDataTexture3D.h"
#include "core/dataHandler/WGridRegular3D.h"
#include "core/graphicsEngine/WGEColormapping.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WGETextureUtils.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/callbacks/WGENodeMaskCallback.h"
#include "core/graphicsEngine/offscreen/WGEOffscreenRenderNode.h"
#include "core/graphicsEngine/offscreen/WGEOffscreenRenderPass.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/shaders/WGEShaderDefineOptions.h"
#include "core/graphicsEngine/shaders/WGEShaderPropertyDefineOptions.h"
#include "core/graphicsEngine/callbacks/WGEShaderAnimationCallback.h"
#include "core/kernel/WKernel.h"

#include "WTuringTextureCreator.h"
#include "WMImageSpaceTensorLIC.h"


  /**
   * @brief Calculates a reaction diffusion texture using turings method.
   *
   * @param target      the memory to store the texture to
   * @param tileWidth   the width of one tile
   * @param tileHeight  the height of one tile
   * @param width       the width of the target mem
   * @param height      the height of the target mem
   * @param iterations  the number of iterations to use
   * @param spotSize    the size of the spots - [0,1]
   * @param spotIrregularity  value specifying irregularity of the spots - [0,1]
   */
void genReactionDiffusion( unsigned char* target,
                           unsigned int tileWidth, unsigned int tileHeight,
                           unsigned int width, unsigned int height,
                           unsigned int iterations,
                           float spotSize,
                           float spotIrregularity)
{
    /////////////////////////////////////////////////////////////////////////////////////////////
    // 1: get memory
    /////////////////////////////////////////////////////////////////////////////////////////////

    // at first create some mem
    // FIXME clean up this mess. There may be a way to reduce memory cost
    float grid1[tileWidth][tileHeight];
    float grid1Min= 3.0;
    float grid1Max= 5.0;
    float grid1Base= 4.0;

    float grid2[tileWidth][tileHeight];
    float grid2Base= 4.0;

    float delta1[tileWidth][tileHeight];
    float delta2[tileWidth][tileHeight];

    float noise[tileWidth][tileHeight];

    float noiseRangeMin=0.1;
    float noiseRangeMax=5.0;

    float noiseRange= noiseRangeMin + ((noiseRangeMax - noiseRangeMin)*spotIrregularity); // the highter the more irregular "spots"
    float noiseBase=12.0;

    /////////////////////////////////////////////////////////////////////////////////////////////
    // 2: init the grids and create random seed used during turing iteration
    /////////////////////////////////////////////////////////////////////////////////////////////

    // init grids
    srand48(time(0));
    for (unsigned int y=0; y<tileHeight; y++)
      for( unsigned int x=0; x<tileWidth; x++ )
      {
        grid1[x][y]=grid1Base;
        grid2[x][y]=grid2Base;
        noise[x][y]=noiseBase + (-noiseRange + (drand48() * 2.0 * noiseRange));
        delta1[x][y]=0.0;
        delta2[x][y]=0.0;
      }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // 3: turing iteration
    /////////////////////////////////////////////////////////////////////////////////////////////

    float kaMin = 0.02;
    float kaMax = 0.6;
    float ka = (kaMin + ((kaMax - kaMin) * (1.0-spotSize))) / 15.0;        // size of spots
    float d1= 0.25 / 2.0;
    float d2= 0.0625 / 2.0;
    float speed = 1.0;

    for (unsigned int iteration = 0; iteration<iterations; iteration++)
    {
      // go through each cell in grid
      for (unsigned int i = 0; i < tileWidth; i++)
      {
        // this ensures we have an "endless" plane -> creates seamless textures
        unsigned int iPrev = (i + tileWidth - 1) % tileWidth;
        unsigned int iNext = (i + 1) % tileWidth;

        for (unsigned int j = 0; j < tileHeight; j++)
        {
          // this ensures we have an "endless" plane -> creates seamless textures
          unsigned int jPrev = (j + tileHeight - 1) % tileHeight;
          unsigned int jNext = (j + 1) % tileHeight;

          /*
          // try the other laplacian filter
          float laplacian1=  grid1[iPrev][jPrev] +      grid1[i][jPrev]  + grid1[iNext][jPrev] +
                             grid1[iPrev][j]     - (8.0*grid1[i][j])     + grid1[iNext][j] +
                             grid1[iPrev][jNext] +      grid1[i][jNext]  + grid1[iNext][jNext];

          float laplacian2=  grid2[iPrev][jPrev] +      grid2[i][jPrev]  + grid2[iNext][jPrev] +
                             grid2[iPrev][j]     - (8.0*grid2[i][j])     + grid2[iNext][j] +
                             grid2[iPrev][jNext] +      grid2[i][jNext]  + grid2[iNext][jNext];
          */

          // apply laplace filter around current grid point
          float laplacian1=grid1[i][jPrev] + grid1[i][jNext] + grid1[iPrev][j] + grid1[iNext][j] - 4.0 * grid1[i][j];
          float laplacian2=grid2[i][jPrev] + grid2[i][jNext] + grid2[iPrev][j] + grid2[iNext][j] - 4.0 * grid2[i][j];

          // diffusion reaction
          delta1[i][j] = ka * (16 - grid1[i][j] * grid2[i][j]) + d1 * laplacian1;
          delta2[i][j] = ka * (grid1[i][j] * grid2[i][j] - grid2[i][j] - noise[i][j]) + d2 * laplacian2;
        }
      }

      // apply delta and find min and max value
      grid1Min= 1e20;
      grid1Max=-1e20;

      for (unsigned int i = 0; i < tileWidth; i++)
        for (unsigned int j = 0; j < tileHeight; j++)
        {
          grid1[i][j]+=(speed * delta1[i][j]);
          grid2[i][j]+=(speed * delta2[i][j]);
          if (grid2[i][j] < 0)
            grid2[i][j] = 0;

          if (grid1[i][j] < grid1Min)
            grid1Min=grid1[i][j];
          if (grid1[i][j] > grid1Max)
            grid1Max=grid1[i][j];
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // 4: scale grid and copy to target
    /////////////////////////////////////////////////////////////////////////////////////////////

    for (unsigned int x = 0; x < tileWidth; x++)
      for (unsigned int y = 0; y < tileHeight; y++)
        target[(y*width) + x]=255.0*(grid1[x][y] - grid1Min) / (grid1Max - grid1Min);
}

WMImageSpaceTensorLIC::WMImageSpaceTensorLIC():
    WModule()
{
}

WMImageSpaceTensorLIC::~WMImageSpaceTensorLIC()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMImageSpaceTensorLIC::factory() const
{
    return boost::shared_ptr< WModule >( new WMImageSpaceTensorLIC() );
}

const std::string WMImageSpaceTensorLIC::getName() const
{
    return "Image Space Tensor LIC";
}

const std::string WMImageSpaceTensorLIC::getDescription() const
{
    return "";
}

void WMImageSpaceTensorLIC::connectors()
{
    // DTI input
    m_evec1In = WModuleInputData< WDataSetVector >::createAndAdd( shared_from_this(), "evec1", "The first Eigenvectors dataset." );
    m_evec2In = WModuleInputData< WDataSetVector >::createAndAdd( shared_from_this(), "evec2", "The second Eigenvectors dataset." );
    m_evalsIn = WModuleInputData< WDataSetVector >::createAndAdd( shared_from_this(), "evals", "The Eigenvalues dataset." );

    // mesh input
    m_meshIn = WModuleInputData< WTriangleMesh >::createAndAdd( shared_from_this(), "surface", "The optional surface to use." );

    // call WModule's initialization
    WModule::connectors();
}

void WMImageSpaceTensorLIC::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition() );

    m_geometryGroup   = m_properties->addPropertyGroup( "Geometry",  "Selection of used geometry to apply LIC to." );

    m_useSlices       = m_geometryGroup->addProperty( "Use Slices", "Show vectors on slices.", true, m_propCondition );

    m_sliceGroup      = m_geometryGroup->addPropertyGroup( "Slices",  "Slice based LIC." );

    // enable slices
    // Flags denoting whether the glyphs should be shown on the specific slice
    m_showonX        = m_sliceGroup->addProperty( "Show Sagittal", "Show vectors on sagittal slice.", true );
    m_showonY        = m_sliceGroup->addProperty( "Show Coronal", "Show vectors on coronal slice.", true );
    m_showonZ        = m_sliceGroup->addProperty( "Show Axial", "Show vectors on axial slice.", true );

    // The slice positions. These get update externally.
    // TODO(all): this should somehow be connected to the nav slices.
    m_xPos           = m_sliceGroup->addProperty( "Sagittal Position", "Slice X position.", 80 );
    m_yPos           = m_sliceGroup->addProperty( "Coronal Position", "Slice Y position.", 100 );
    m_zPos           = m_sliceGroup->addProperty( "Axial Position", "Slice Z position.", 80 );
    m_xPos->setMin( 0 );
    m_xPos->setMax( 159 );
    m_yPos->setMin( 0 );
    m_yPos->setMax( 199 );
    m_zPos->setMin( 0 );
    m_zPos->setMax( 159 );

    m_licGroup      = m_properties->addPropertyGroup( "LIC",  "LIC properties." );

    m_useLight       = m_licGroup->addProperty( "Use Light", "Check to enable lightning using the Phong model.", false );
    m_lightIntensity = m_licGroup->addProperty( "Light Intensity", "Define how intense the light should be.", 1.0 );
    m_lightIntensity->setMin( 0.0 );
    m_lightIntensity->setMax( 10.0 );

    m_useEdges        = m_licGroup->addProperty( "Edges", "Check to enable blending in edges.", true );
    m_useEdgesColor   = m_licGroup->addProperty( "Edge Color", "Define the color of the edges.", defaultColor::WHITE );
    m_useEdgesStep    = m_licGroup->addProperty( "Edge Step", "Define the steepness of the blend function between color and edge color.", 1.0 );
    m_useEdgesStep->setMin( 0.0 );
    m_useEdgesStep->setMax( 10.0 );

    m_cmapRatio    = m_licGroup->addProperty( "Ratio Colormap to LIC", "Blending ratio between LIC and colormap.", 0.5 );
    m_cmapRatio->setMin( 0.0 );
    m_cmapRatio->setMax( 1.0 );

    m_advancedLicGroup      = m_properties->addPropertyGroup( "Advanced",  "More advanced LIC properties." );
    // show hud?
    m_showHUD        = m_advancedLicGroup->addProperty( "Show HUD", "Check to enable the debugging texture HUD.", false );
    m_noiseRes     = m_advancedLicGroup->addProperty( "Noise Resolution", "The noise is of 128^3 pixels size. This scaler allows "
                                                        "modification of this size.", 1.5 );
    m_noiseRes->setMin( 0 );
    m_noiseRes->setMax( 100 );

    m_numIters     = m_advancedLicGroup->addProperty( "Number of Iterations", "How much iterations along a streamline should be done per frame.",
                                                      30 );
    m_numIters->setMin( 1 );
    m_numIters->setMax( 1000 );

    m_projectionAngleThreshold = m_advancedLicGroup->addProperty( "Projection Angle Threshold", "This defines the threshold of the angle between "
            "tangential plane of the surface and the vector which is going to be projected. You can adjust how steep a vector can be before it is "
            "clipped and NOT projected. Note: all vectors with an angle below this threshold are projected but linearly reduced in influence "
            "depending on the angle.", 90.0 );
    m_projectionAngleThreshold->setMin( 0.0 );
    m_projectionAngleThreshold->setMax( 180.0 );

    m_faClip = m_advancedLicGroup->addProperty( "Clip FA", "Clip by FA", 0.1 );
    m_faClip->setMin( 0.0 );
    m_faClip->setMax( 1.0 );

    // call WModule's initialization
    WModule::properties();
}

void WMImageSpaceTensorLIC::initOSG( boost::shared_ptr< WGridRegular3D > grid, boost::shared_ptr< WTriangleMesh > mesh )
{
    // remove the old slices
    m_output->clear();

    if( mesh && !m_useSlices->get( true ) )
    {
        // we have a mesh and want to use it
        // create geometry and geode
        osg::Geometry* surfaceGeometry = new osg::Geometry();
        osg::ref_ptr< osg::Geode > surfaceGeode = osg::ref_ptr< osg::Geode >( new osg::Geode );

        surfaceGeometry->setVertexArray( mesh->getVertexArray() );
        osg::DrawElementsUInt* surfaceElement;
        surfaceElement = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES, 0 );
        std::vector< size_t > tris = mesh->getTriangles();
        surfaceElement->reserve( tris.size() );
        for( unsigned int vertId = 0; vertId < tris.size(); ++vertId )
        {
            surfaceElement->push_back( tris[vertId] );
        }
        surfaceGeometry->addPrimitiveSet( surfaceElement );

        // normals
        surfaceGeometry->setNormalArray( mesh->getVertexNormalArray() );
        surfaceGeometry->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

        // texture coordinates
        surfaceGeometry->setTexCoordArray( 0, mesh->getTextureCoordinateArray() );

        // render
        surfaceGeode->addDrawable( surfaceGeometry );
        m_output->insert( surfaceGeode );
    }
    else if( !mesh && !m_useSlices->get( true ) )
    {
        warnLog() << "No surface connected to input but surface render mode enabled. Nothing rendered.";
    }
    else
    {
        // create a new geode containing the slices
        osg::ref_ptr< osg::Node > xSlice = wge::genFinitePlane( grid->getOrigin(), grid->getNbCoordsY() * grid->getDirectionY(),
                                                                                   grid->getNbCoordsZ() * grid->getDirectionZ() );

        osg::ref_ptr< osg::Node > ySlice = wge::genFinitePlane( grid->getOrigin(), grid->getNbCoordsX() * grid->getDirectionX(),
                                                                                   grid->getNbCoordsZ() * grid->getDirectionZ() );

        osg::ref_ptr< osg::Node > zSlice =  wge::genFinitePlane( grid->getOrigin(), grid->getNbCoordsX() * grid->getDirectionX(),
                                                                                    grid->getNbCoordsY() * grid->getDirectionY() );

        // disable picking
        xSlice->setName( "_X SLice" );
        ySlice->setName( "_Y SLice" );
        zSlice->setName( "_Z SLice" );

        // The movement of the slice is done in the shader. An alternative would be WGELinearTranslationCallback but there, the needed matrix is
        // not available in the shader
        osg::StateSet* ss = xSlice->getOrCreateStateSet();
        ss->addUniform( new WGEPropertyUniform< WPropInt >( "u_vertexShift", m_xPos ) );
        ss->addUniform( new osg::Uniform( "u_vertexShiftDirection", grid->getDirectionX().as< osg::Vec3f >() ) );  // the axis to move along
        ss = ySlice->getOrCreateStateSet();
        ss->addUniform( new WGEPropertyUniform< WPropInt >( "u_vertexShift", m_yPos ) );
        ss->addUniform( new osg::Uniform( "u_vertexShiftDirection", grid->getDirectionY().as< osg::Vec3f >() ) );  // the axis to move along
        ss = zSlice->getOrCreateStateSet();
        ss->addUniform( new WGEPropertyUniform< WPropInt >( "u_vertexShift", m_zPos ) );
        ss->addUniform( new osg::Uniform( "u_vertexShiftDirection", grid->getDirectionZ().as< osg::Vec3f >() ) );  // the axis to move along

        // set callbacks for en-/disabling the nodes
        xSlice->addUpdateCallback( new WGENodeMaskCallback( m_showonX ) );
        ySlice->addUpdateCallback( new WGENodeMaskCallback( m_showonY ) );
        zSlice->addUpdateCallback( new WGENodeMaskCallback( m_showonZ ) );

        // disable culling.
        xSlice->setCullingActive( false );
        ySlice->setCullingActive( false );
        zSlice->setCullingActive( false );

        // add the transformation nodes to the output group
        m_output->insert( xSlice );
        m_output->insert( ySlice );
        m_output->insert( zSlice );
        m_output->dirtyBound();
    }
}

void WMImageSpaceTensorLIC::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_evec1In->getDataChangedCondition() );
    m_moduleState.add( m_evec2In->getDataChangedCondition() );
    m_moduleState.add( m_evalsIn->getDataChangedCondition() );
    m_moduleState.add( m_meshIn->getDataChangedCondition() );
    // Remember the condition provided to some properties in properties()? The condition can now be used with this condition set.
    m_moduleState.add( m_propCondition );

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Preparation 1: create noise texture
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // we need a noise texture with a sufficient resolution. Create it.
    const size_t resX = 128;
    float rdIrr = 0.0;
    float rdSpot = 0.689;
    unsigned int rdIter = 4000;
    //unsigned char* rdField = new unsigned char[ resX * resX ];

    osg::ref_ptr< osg::Image > ima = new osg::Image;
    ima->allocateImage( resX, resX, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE );
    genReactionDiffusion( ima->data(), resX, resX, resX, resX, rdIter, rdSpot, rdIrr );

    WGETexture2D::SPtr randTexture( new WGETexture2D( ima ) );
    randTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    randTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );

    // done.
    ready();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Preparation 2: initialize offscreen renderer and hardwire it
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // create the root node for all the geometry
    m_root = osg::ref_ptr< WGEManagedGroupNode > ( new WGEManagedGroupNode( m_active ) );

    // root geometry node for the offscreen path
    m_output = osg::ref_ptr< WGEGroupNode > ( new WGEGroupNode() );

    // the WGEOffscreenRenderNode manages each of the render-passes for us
    osg::ref_ptr< WGEOffscreenRenderNode > offscreen = new WGEOffscreenRenderNode(
        WKernel::getRunningKernel()->getGraphicsEngine()->getViewer()->getCamera()
    );

    // allow en-/disabling the HUD:
    offscreen->getTextureHUD()->addUpdateCallback( new WGENodeMaskCallback( m_showHUD ) );

    // setup all the passes needed for image space advection
    osg::ref_ptr< WGEShader > transformationShader = new WGEShader( "WMImageSpaceTensorLIC-Transformation", m_localPath );

    // This should not be needed. But somehow, the u_vertexShift uniforms does not get removed by OSG when switching to mesh.
    transformationShader->addPreprocessor( WGEShaderPreprocessor::SPtr(
        new WGEShaderPropertyDefineOptions< WPropBool >( m_useSlices, "VERTEXSHIFT_DISABLED", "VERTEXSHIFT_ENABLED" )
    ) );

    osg::ref_ptr< WGEOffscreenRenderPass > transformation = offscreen->addGeometryRenderPass(
        m_output,
        transformationShader,
        "Transformation"
    );
    transformation->bind( randTexture, 3 );
    // apply colormapping to transformation
    WGEColormapping::apply( transformation, transformationShader, 4 );

    osg::ref_ptr< WGEShader > edgeShader = new WGEShader( "WMImageSpaceTensorLIC-Edge", m_localPath );
    osg::ref_ptr< WGEOffscreenRenderPass > edgeDetection =  offscreen->addTextureProcessingPass(
        edgeShader,
        "Edge Detection"
    );

    // we use two advection passes per frame as the input A of the first produces the output B whereas the second pass uses B as input and
    // produces A as output. This way we can use A as input for the next step (clipping and blending).
    osg::ref_ptr< WGEOffscreenRenderPass > advection =  offscreen->addTextureProcessingPass(
        new WGEShader( "WMImageSpaceTensorLIC-Advection", m_localPath ),
        "Advection"
    );

    // finally, put it back on screen, clip it, color it and apply depth buffer to on-screen buffer
    osg::ref_ptr< WGEOffscreenRenderPass > clipBlend =  offscreen->addTextureProcessingPass(
        new WGEShader( "WMImageSpaceTensorLIC-ClipBlend", m_localPath ),
        "Clip & Blend"
    );

    // finally, put it back on screen, clip it, color it and apply depth buffer to on-screen buffer
    osg::ref_ptr< WGEOffscreenRenderPass > postprocessing =  offscreen->addFinalOnScreenPass(
        new WGEShader( "WMImageSpaceTensorLIC-Postprocessing", m_localPath ),
        "Postprocessing"
    );

    // hardwire the textures to use for each pass:

    // Transformation Pass, needs Geometry
    //  * Creates 2D projected Vectors in RG and BA
    //  * Lighting and projected noise in out2.rg, depth in b
    //  * Depth
    osg::ref_ptr< osg::Texture2D > transformationOut1  = transformation->attach( WGECamera::COLOR_BUFFER0 );
    osg::ref_ptr< osg::Texture2D > transformationOut2  = transformation->attach( WGECamera::COLOR_BUFFER1 );
    osg::ref_ptr< osg::Texture2D > transformationOut3  = transformation->attach( WGECamera::COLOR_BUFFER2 );
    osg::ref_ptr< osg::Texture2D > transformationColormapped  = transformation->attach( WGECamera::COLOR_BUFFER3 );
    osg::ref_ptr< osg::Texture2D > transformationDepth = transformation->attach( WGECamera::DEPTH_BUFFER );
    // and some uniforms
    transformation->addUniform( new WGEPropertyUniform< WPropDouble >( "u_noiseResoultuion", m_noiseRes ) );
    transformation->addUniform( new WGEPropertyUniform< WPropDouble >( "u_projectionAngleThreshold", m_projectionAngleThreshold ) );
    transformation->addUniform( new WGEPropertyUniform< WPropDouble >( "u_clipFA", m_faClip ) );

    // Edge Detection Pass, needs Depth as input
    //  * Edges in R, noise in G
    osg::ref_ptr< osg::Texture2D > edgeDetectionOut1 = edgeDetection->attach( WGECamera::COLOR_BUFFER0 );
    edgeDetection->bind( transformationDepth, 0 );
    edgeDetection->bind( transformationOut2,  1 );

    // Advection Pass, needs edges and projected vectors as well as noise texture
    //  * Advected noise in luminance channel
    osg::ref_ptr< osg::Texture2D > advectionOutA  = advection->attach( WGECamera::COLOR_BUFFER0, GL_RGBA );
    advection->bind( transformationOut1, 0 );
    advection->bind( edgeDetectionOut1,  1 );

    // advection needs some uniforms controlled by properties
    osg::ref_ptr< osg::Uniform > numIters = new WGEPropertyUniform< WPropInt >( "u_numIter", m_numIters );
    advection->addUniform( numIters );

    osg::Uniform* animationTime( new osg::Uniform( "u_animation", 0 ) );
    animationTime->setUpdateCallback( new WGEShaderAnimationCallback() );
    advection->addUniform( animationTime );

    // provide the Gbuffer input, with several mipmap levels
    advectionOutA->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    edgeDetectionOut1->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    transformationColormapped->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );

    // Final clipping and blending phase, needs Advected Noise, Edges, Depth and Light
    osg::ref_ptr< osg::Texture2D > merged = clipBlend->attach( WGECamera::COLOR_BUFFER0, GL_RGBA );
    clipBlend->bind( advectionOutA, 0 );
    clipBlend->bind( edgeDetectionOut1, 1 );
    clipBlend->bind( transformationColormapped, 2 );
    clipBlend->bind( transformationOut2, 3 );
    clipBlend->bind( transformationOut3, 4 );

    // final pass needs some uniforms controlled by properties
    clipBlend->addUniform( new WGEPropertyUniform< WPropBool >( "u_useEdges", m_useEdges ) );
    clipBlend->addUniform( new WGEPropertyUniform< WPropColor >( "u_useEdgesColor", m_useEdgesColor ) );
    clipBlend->addUniform( new WGEPropertyUniform< WPropDouble >( "u_useEdgesStep", m_useEdgesStep ) );
    clipBlend->addUniform( new WGEPropertyUniform< WPropBool >( "u_useLight", m_useLight ) );
    clipBlend->addUniform( new WGEPropertyUniform< WPropDouble >( "u_lightIntensity", m_lightIntensity ) );
    clipBlend->addUniform( new WGEPropertyUniform< WPropDouble >( "u_cmapRatio", m_cmapRatio ) );

    // The final pass should also blend properly:
    postprocessing->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );

    // Final clipping and blending phase, needs Advected Noise, Edges, Depth and Light
    postprocessing->bind( advectionOutA, 0 );
    postprocessing->bind( edgeDetectionOut1, 1 );
    postprocessing->bind( transformationColormapped, 2 );
    postprocessing->bind( transformationOut2, 3 );
    postprocessing->bind( transformationOut3, 4 );
    postprocessing->bind( merged, 5 );

    // final pass needs some uniforms controlled by properties
    postprocessing->addUniform( new WGEPropertyUniform< WPropBool >( "u_useEdges", m_useEdges ) );
    postprocessing->addUniform( new WGEPropertyUniform< WPropColor >( "u_useEdgesColor", m_useEdgesColor ) );
    postprocessing->addUniform( new WGEPropertyUniform< WPropDouble >( "u_useEdgesStep", m_useEdgesStep ) );
    postprocessing->addUniform( new WGEPropertyUniform< WPropBool >( "u_useLight", m_useLight ) );
    postprocessing->addUniform( new WGEPropertyUniform< WPropDouble >( "u_lightIntensity", m_lightIntensity ) );
    postprocessing->addUniform( new WGEPropertyUniform< WPropDouble >( "u_cmapRatio", m_cmapRatio ) );

    // add everything to root node
    m_root->insert( offscreen );

    // Cull proxy. Updated on dataset change
    osg::ref_ptr< osg::Node > cullProxy;

    // register scene
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_root );

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Main loop
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // main loop
    while( !m_shutdownFlag() )
    {
        debugLog() << "Waiting ...";
        m_moduleState.wait();

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        // To query whether an input was updated, simply ask the input:
        bool dataUpdated = m_evec1In->handledUpdate() || m_evec2In->handledUpdate() || m_meshIn->handledUpdate();
        bool propertyUpdated = m_useSlices->changed();
        boost::shared_ptr< WDataSetVector > dataSetEvec1 = m_evec1In->getData();
        boost::shared_ptr< WDataSetVector > dataSetEvec2 = m_evec2In->getData();
        boost::shared_ptr< WDataSetVector > dataSetEvals = m_evalsIn->getData();
        boost::shared_ptr< WTriangleMesh > mesh = m_meshIn->getData();

        bool dataValid = ( dataSetEvals && dataSetEvec1 && dataSetEvec2 );

        // is data valid? If not, remove graphics
        if( !dataValid )
        {
            debugLog() << "Resetting.";
            WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( offscreen );
            continue;
        }

        // something interesting for us?
        if( dataValid && !dataUpdated && !propertyUpdated )
        {
            continue;
        }

        // prefer vector dataset if existing
        boost::shared_ptr< WGridRegular3D > grid;
        // get grid and prepare OSG
        grid = boost::dynamic_pointer_cast< WGridRegular3D >( dataSetEvec1->getGrid() );
        m_xPos->setMax( grid->getNbCoordsX() - 1 );
        m_yPos->setMax( grid->getNbCoordsY() - 1 );
        m_zPos->setMax( grid->getNbCoordsZ() - 1 );
        initOSG( grid, mesh );

        // prepare offscreen render chain
        transformation->bind( dataSetEvec1->getTexture(), 0 );
        transformation->bind( dataSetEvec2->getTexture(), 1 );
        transformation->bind( dataSetEvals->getTexture(), 2 );

        // Update CullProxy when we get new data
        // add a cull-proxy as we modify the geometry on the GPU
        WBoundingBox bbox = grid->getVoxelBoundingBox();
        m_root->remove( cullProxy );
        cullProxy = wge::generateCullProxy( bbox );
        m_root->insert( cullProxy );
        debugLog() << "Done";
    }

    // clean up
    m_root->clear();
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_root );
}

