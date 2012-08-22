#include <cmath>
#include <vector>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

#define kMaxPointsPerCell 9

class RangeList;

class Vec2 {
public:
	Vec2() {};
	Vec2(float _x, float _y) : x(_x), y(_y) {};

	float x,y;

	float length() { return sqrt(x*x + y*y); }

	bool operator ==(const Vec2 &b) const { return x==b.x && y==b.y; }
	Vec2 operator +(Vec2 b) { return Vec2(x+b.x, y+b.y); }
	Vec2 operator -(Vec2 b) { return Vec2(x-b.x, y-b.y); }
	Vec2 operator *(Vec2 b) { return Vec2(x*b.x, y*b.y); }
	Vec2 operator /(Vec2 b) { return Vec2(x/b.x, y*b.y); }

	Vec2 operator +(float n) { return Vec2(x+n, y+n); }
	Vec2 operator -(float n) { return Vec2(x-n, y-n); }
	Vec2 operator *(float n) { return Vec2(x*n, y*n); }
	Vec2 operator /(float n) { return Vec2(x/n, y*n); }

	Vec2 &operator +=(Vec2 b) { x+=b.x; y+=b.y; return *this; }
	Vec2 &operator -=(Vec2 b) { x-=b.x; y-=b.y; return *this; }
	Vec2 &operator *=(Vec2 b) { x*=b.x; y*=b.y; return *this; }
	Vec2 &operator /=(Vec2 b) { x/=b.x; y/=b.y; return *this; }

	Vec2 &operator +=(float n) { x+=n; y+=n; return *this; }
	Vec2 &operator -=(float n) { x-=n; y-=n; return *this; }
	Vec2 &operator *=(float n) { x*=n; y*=n; return *this; }
	Vec2 &operator /=(float n) { x/=n; y/=n; return *this; }
};

///

class PDSampler {
protected:
    boost::mt19937 m_mt;
    boost::uniform_01< float > m_rngFloat;
	std::vector<int> m_neighbors;

	int (*m_grid)[kMaxPointsPerCell];
	int m_gridSize;
	float m_gridCellSize;

public:
	std::vector<Vec2> points;
	float radius;
	bool isTiled;

public:
	PDSampler(float radius, bool isTiled, bool usesGrid=true);
	virtual ~PDSampler() { };

		// generate a random point in square
	Vec2 randomPoint();

		// return tiled coordinates of _v_
	Vec2 getTiled(Vec2 v);

		// return grid x,y for point
	void getGridXY(Vec2 &v, int *gx_out, int *gy_out);

		// add _pt_ to point list and grid
	void addPoint(Vec2 pt);

		// find available angle ranges on boundary for candidate
		// by subtracting occluded neighbor ranges from _rl_
	void findNeighborRanges(int index, RangeList &rl);

	virtual void complete() = 0;
};

class BoundarySampler : public PDSampler {
public:
	BoundarySampler(float radius, bool isTiled) : PDSampler(radius, isTiled) {};

	virtual void complete();
};
