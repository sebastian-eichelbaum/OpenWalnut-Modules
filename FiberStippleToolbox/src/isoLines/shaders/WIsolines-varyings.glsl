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

//////////////////////////////////////////////////////////////////////////////
// Positions (in 0,1 scaled coordinates) where the isovalue hits the corresponding edege
//////////////////////////////////////////////////////////////////////////////
/**
 * Position where the isovalue hits the first edge. If not hit, nonsense in here.
 */
varying vec3 hit0Pos;

/**
 * Position where the isovalue hits the second edge. If not hit, nonsense in here.
 */
varying vec3 hit1Pos;

/**
 * Position where the isovalue hits the third edge. If not hit, nonsense in here.
 */
varying vec3 hit2Pos;

/**
 * Position where the isovalue hits the fourth edge. If not hit, nonsense in here.
 */
varying vec3 hit3Pos;

/**
 * How many edges are involved, but binary coded for easier condition switching.
 */
varying float sumHits;

//////////////////////////////////////////////////////////////////////////////
// Predicates indicating if the certain edge was hit by the isovalue or not. They are float, as bool varyings are not allowed
//////////////////////////////////////////////////////////////////////////////
/**
 * First edge was hit by the isovalue, predicate.
 */
varying float edge0Hit_f;

/**
 * Second edge was hit by the isovalue, predicate.
 */
varying float edge1Hit_f;

/**
 * Third edge was hit by the isovalue, predicate.
 */
varying float edge2Hit_f;

/**
 * Fourth edge was hit by the isovalue, predicate.
 */
varying float edge3Hit_f;

/**
 * Scale factor to increase quad so they overlap and reduce artifacts with bold iso lines.
 */
varying float v_quadScale;

