#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "PDSampling.h"
#include "RangeList.h"

// this is just a hack as this is an external lib which our module loader tries to load OW will report a stack trace. However
// with this hack OW just will report that it is not able to create a prototype instance. If we want to omit this too we need
// to create an non empty WModuleList which does nothing.
class WModuleList;
void WLoadModule( WModuleList&m );
namespace
{
    extern "C"                       void WLoadModule( WModuleList& /* m */ ) { }
}
// end of the WModule symbol hacking stuff

PDSampler::PDSampler(float _radius, bool _isTiled, bool usesGrid) :
	radius(_radius),
	isTiled(_isTiled)
{
    m_mt.seed( 0 );

	if( radius < 0.0005 || radius > 0.2 ) {
		printf("Radius (%f) is outside allowable range.\n", radius);
		exit(1);
	}

	if (usesGrid) {
			// grid size is chosen so that 4*radius search only
			// requires searching adjacent cells, this also
			// determines max points per cell
		m_gridSize = (int) ceil(2./(4.*_radius));
		if (m_gridSize<2) m_gridSize = 2;

		m_gridCellSize = 2.0f/m_gridSize;
		m_grid = new int[m_gridSize*m_gridSize][kMaxPointsPerCell];

		for (int y=0; y<m_gridSize; y++) {
			for (int x=0; x<m_gridSize; x++) {
				for (int k=0; k<kMaxPointsPerCell; k++) {
					m_grid[y*m_gridSize + x][k] = -1;
				}
			}
		}
	} else {
		m_gridSize = 0;
		m_gridCellSize = 0;
		m_grid = 0;
	}
}

Vec2 PDSampler::randomPoint()
{
	return Vec2(2*m_rngFloat( m_mt )-1, 2*m_rngFloat( m_mt )-1);
}

namespace
{
    float rescale( float x )
    {
        float result = x;
        if( x < -1.0 )
        {
            result += 2.0;
        }
        else if( x > 1.0 )
        {
            result -= 2.0;
        }
        return result;
    }
}

Vec2 PDSampler::getTiled(Vec2 v)
{
    if( isTiled )
    {
	    return Vec2( rescale( v.x ), rescale( v.y ) );
    }
    else
    {
        return v;
    }
}

void PDSampler::getGridXY(Vec2 &v, int *gx_out, int *gy_out)
{
	int gx = *gx_out = (int) floor(.5*(v.x + 1)*m_gridSize);
	int gy = *gy_out = (int) floor(.5*(v.y + 1)*m_gridSize);
	if (gx<0 || gx>=m_gridSize || gy<0 || gy>=m_gridSize) {
		printf("Internal error, point outside grid was generated, ignoring.\n");
	}
}

void PDSampler::addPoint(Vec2 pt)
{
	int i, gx, gy, *cell;

	points.push_back(pt);

	if (m_grid) {
		getGridXY(pt, &gx, &gy);
		cell = m_grid[gy*m_gridSize + gx];
		for (i=0; i<kMaxPointsPerCell; i++) {
			if (cell[i]==-1) {
				cell[i] = (int) points.size()-1;
				break;
			}
		}
		if (i==kMaxPointsPerCell) {
			printf("Internal error, overflowed max points per grid cell. Exiting.\n");
			exit(1);
		}
	}
}

void PDSampler::findNeighborRanges(int index, RangeList &rl)
{
	if (!m_grid) {
		printf("Internal error, sampler cannot search without grid.\n");
		exit(1);
	}

	Vec2 &candidate = points[index];
	float rangeSqrd = 4*4*radius*radius;
	int i, j, k, gx, gy, N = (int) ceil(4*radius/m_gridCellSize);
	if (N>(m_gridSize>>1)) N = m_gridSize>>1;

	getGridXY(candidate, &gx, &gy);

	int xSide = (candidate.x - (-1 + gx*m_gridCellSize))>m_gridCellSize*.5;
	int ySide = (candidate.y - (-1 + gy*m_gridCellSize))>m_gridCellSize*.5;
	int iy = 1;
	for (j=-N; j<=N; j++) {
		int ix = 1;

		if (j==0) iy = ySide;
		else if (j==1) iy = 0;

		for (i=-N; i<=N; i++) {
			if (i==0) ix = xSide;
			else if (i==1) ix = 0;

				// offset to closest cell point
			float dx = candidate.x - (-1 + (gx+i+ix)*m_gridCellSize);
			float dy = candidate.y - (-1 + (gy+j+iy)*m_gridCellSize);

			if (dx*dx+dy*dy<rangeSqrd) {
				int cx = (gx+i+m_gridSize)%m_gridSize;
				int cy = (gy+j+m_gridSize)%m_gridSize;
				int *cell = m_grid[cy*m_gridSize + cx];

				for (k=0; k<kMaxPointsPerCell; k++) {
					if (cell[k]==-1) {
						break;
					} else if (cell[k]!=index) {
						Vec2 &pt = points[cell[k]];
						Vec2 v = getTiled(pt-candidate);
						float distSqrd = v.x*v.x + v.y*v.y;

						if (distSqrd<rangeSqrd) {
							float dist = sqrt(distSqrd);
							float angle = atan2(v.y,v.x);
							float theta = acos(.25f*dist/radius);

							rl.subtract(angle-theta, angle+theta);
						}
					}
				}
			}
		}
	}
}
void BoundarySampler::complete()
{
	RangeList rl(0,0);
    std::vector< int > candidates;

	addPoint(randomPoint());
	candidates.push_back((int) points.size()-1);

	while (candidates.size()) {
		int c = m_mt() % candidates.size();
		int index = candidates[c];
		Vec2 candidate = points[index];
		candidates[c] = candidates[candidates.size()-1];
		candidates.pop_back();

		rl.reset(0, (float) M_PI*2);
		findNeighborRanges(index, rl);
		while (rl.numRanges) {
			RangeEntry &re = rl.ranges[m_mt() % rl.numRanges];
			float angle = re.min + (re.max-re.min)* m_rngFloat( m_mt );
			Vec2 pt = getTiled(Vec2(candidate.x + cos(angle)*2*radius,
									candidate.y + sin(angle)*2*radius));

			addPoint(pt);
			candidates.push_back((int) points.size()-1);

			rl.subtract(angle - (float) M_PI/3, angle + (float) M_PI/3);
		}
	}
}
