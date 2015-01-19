/*!
 * 
 *
 * \brief       Convex quadratic benchmark function.
 * 
 *
 * \author      T. Voss
 * \date        2010-2011
 *
 *
 * \par Copyright 1995-2015 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://image.diku.dk/shark/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SHARK_OBJECTIVEFUNCTIONS_BENCHMARK_SPHERE_H
#define SHARK_OBJECTIVEFUNCTIONS_BENCHMARK_SPHERE_H

#include <shark/ObjectiveFunctions/AbstractObjectiveFunction.h>
#include <shark/Rng/GlobalRng.h>

namespace shark {
/**
 * \brief Convex quadratic benchmark function.
 */
struct Sphere : public SingleObjectiveFunction {
	
	Sphere(unsigned int numberOfVariables = 5):m_numberOfVariables(numberOfVariables) {
		m_features |= CAN_PROPOSE_STARTING_POINT;
	}

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "Sphere"; }

	std::size_t numberOfVariables()const{
		return m_numberOfVariables;
	}
	
	bool hasScalableDimensionality()const{
		return true;
	}

	void setNumberOfVariables( std::size_t numberOfVariables ){
		m_numberOfVariables = numberOfVariables;
	}

	void configure(const PropertyTree &node) {
		m_numberOfVariables = node.get("numberOfVariables", 5l);
	}

	void proposeStartingPoint(SearchPointType &x) const {
		x.resize(numberOfVariables());

		for (unsigned int i = 0; i < x.size(); i++) {
			x(i) = Rng::uni(0, 1);
		}
	}

	double eval(const SearchPointType &p) const {
		m_evaluationCounter++;
		return norm_sqr(p);
	}
private:
	std::size_t m_numberOfVariables;
};

}

#endif
