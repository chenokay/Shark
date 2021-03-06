//===========================================================================
/*!
 * 
 *
 * \brief       Offers the functions to create and to work with a
 * recurrent neural network.
 * 
 * 
 *
 * \author      O. Krause
 * \date        2010
 *
 *
 * \par Copyright 1995-2017 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://shark-ml.org/>
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
#ifndef SHARK_MODELS_ONLINERNNET_H
#define SHARK_MODELS_ONLINERNNET_H

#include <shark/Core/DLLSupport.h>
#include <shark/Models/AbstractModel.h>
#include <shark/Models/RecurrentStructure.h>
namespace shark{

//!  \brief A recurrent neural network regression model optimized
//!         for online learning. 
//!
//! The OnlineRNNet can only process a single input at a time. Internally
//! it stores the last activation as well as the derivatives which get updated 
//! over the course of the sequence. Instead of feeding in the whole sequence,
//! the inputs must be given one after another. However if the whole sequence is
//! available in advance, this implementation is not advisable, since it is a lot slower
//! than RNNet which is targeted to whole sequences. 
//! 
//! All network state is stored in the State structure which can be created by createState()
//! which has to be supplied to eval.
//! A new time sequence is started by generating a new state object.
//! When the network is created the user has to decide whether gradients
//! are needed. In this case additional ressources are allocated in the state object on creation
//! and eval makes sure that the gradient is properly updated between steps, this is costly.
//! It is possible to skip steps updating the parameters, e.g. when no reward signal is available.
//!
//! Note that eval can only work with batches of size one and eval without  a state object can not
//! be called.
class OnlineRNNet:public AbstractModel<RealVector,RealVector>
{
private:
	struct InternalState: public State{
		
		InternalState(RecurrentStructure const& structure)
		: activation(structure.numberOfUnits(),0.0)
		, lastActivation(structure.numberOfUnits(),0.0)
		, unitGradient(structure.parameters(),structure.numberOfNeurons(),0.0){}
		//!the activation of the network at time t (after evaluation)
		RealVector activation;
		//!the activation of the network at time t-1 (before evaluation)
		RealVector lastActivation;

		//!\brief the gradient of the hidden units with respect to every weight
		//!
		//!The gradient \f$ \frac{\delta y_k(t)}{\delta w_{ij}} \f$ is stored in this
		//!structure. Using this gradient, the derivative of the Network can be calculated as
		//!\f[ \frac{\delta E(y(t))}{\delta w_{ij}}=\sum_{k=1}^n\frac{\delta E(y(t))}{\delta y_k} \frac{\delta y_k(t)}{\delta w_{ij}} \f]
		//!where \f$ y_k(t) \f$ is the activation of neuron \f$ k \f$ at timestep \f$ t \f$
		//!the gradient needs to be updated after every timestep using the formula
		//!\f[ \frac{\delta y_k(t+1)}{\delta w_{ij}}= y'_k(t)= \left[\sum_{l=1}^n w_{il}\frac{\delta y_l(t)}{\delta w_{ij}} +\delta_{kl}y_l(t-1)\right]\f]
		//!so if the gradient is needed, don't forget to call weightedParameterDerivative at every timestep!
		RealMatrix unitGradient;
	};
public:
	//! creates a configured neural network
	//!
	//! \brief structure The structure of the OnlineRNNet
	//! \brief computeGradient Whether the network will be used to compute gradients
	SHARK_EXPORT_SYMBOL OnlineRNNet(RecurrentStructure* structure, bool computeGradient);

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "OnlineRNNet"; }

	//!  \brief Feeds a timestep of a time series to the model and
	//!         calculates it's output. The batches must have size 1.
	//!
	//!  \param  pattern Input patterns for the network.
	//!  \param  output Used to store the outputs of the network.
	//!  \param  state the current state of the RNN that is updated by eval
	SHARK_EXPORT_SYMBOL void eval(RealMatrix const& pattern,RealMatrix& output, State& state)const;
	
	
	/// \brief It is forbidding to call eval without a state object.
	SHARK_EXPORT_SYMBOL void eval(RealMatrix const& pattern,RealMatrix& output)const{
		throw SHARKEXCEPTION("[OnlineRNNet::eval] Eval can not be called without state object");
	}
	using AbstractModel<RealVector,RealVector>::eval;

	/// obtain the input dimension
	std::size_t inputSize() const{
		return mpe_structure->inputs();
	}

	/// obtain the output dimension
	std::size_t outputSize() const{
		return mpe_structure->outputs();
	}

	//!\brief calculates the weighted sum of gradients w.r.t the parameters
	//!
	//!Uses an iterative update scheme to calculate the gradient at timestep t from the gradient
	//!at timestep t-1 using forward propagation. This Methods requires O(n^3) Memory and O(n^4) computations,
	//!where n is the number of neurons. So if the network is very large, RNNet should be used!
	//!
	//! \param pattern the pattern to evaluate
	//! \param coefficients the oefficients which are used to calculate the weighted sum
	//! \param gradient the calculated gradient
	//! \param state the current state of the RNN
	SHARK_EXPORT_SYMBOL void weightedParameterDerivative(
		RealMatrix const& pattern, RealMatrix const& coefficients,
		State const& state, RealVector& gradient
	)const;

	//! get internal parameters of the model
	RealVector parameterVector() const{
		return mpe_structure->parameterVector();
	}
	//! set internal parameters of the model
	void setParameterVector(RealVector const& newParameters){
		mpe_structure->setParameterVector(newParameters);
	}

	//!number of parameters of the network
	std::size_t numberOfParameters() const{
		return mpe_structure->parameters();
	}
	
	boost::shared_ptr<State> createState()const{
		return boost::shared_ptr<State>(new InternalState( *mpe_structure));
	}
	

	//!  \brief This Method sets the activation of the output neurons
	//!
	//!  This is usefull when teacher forcing is used. When the network
	//!  is trained to predict a timeseries and diverges from the sequence
	//!  at an early stage, the resulting gradient might not be very helpfull.
	//!  In this case, teacher forcing can be applied to prevent diverging.
	//!  However, the network might become unstable, when teacher-forcing is turned off
	//!  because there is no force which prevents it from diverging anymore.
	//!
	//!  \param  state  The current state of the network
	//!  \param  activation  Input patterns for the network.
	void setOutputActivation(State& state, RealVector const& activation){
		InternalState& s = state.toState<InternalState>();
		s.activation.resize(mpe_structure->numberOfUnits());
		subrange(s.activation,mpe_structure->numberOfUnits()-outputSize(),mpe_structure->numberOfUnits()) = activation;
	}
	
	
	
protected:
	
	//! the topology of the network.
	RecurrentStructure* mpe_structure;
	
	//! stores whether the network should compute a gradient
	bool m_computeGradient;
	
};
}

#endif //RNNET_H









