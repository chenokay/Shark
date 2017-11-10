//===========================================================================
/*!
 * 
 *
 * \brief       Implements the Mean Model that can be used for ensemble classifiers
 * 
 * 
 *
 * \author      Kang Li, O. Krause
 * \date        2014
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
//===========================================================================

#ifndef SHARK_MODELS_MEANMODEL_H
#define SHARK_MODELS_MEANMODEL_H

namespace shark {
/// \brief Calculates the weighted mean of a set of models
template<class ModelType>
class MeanModel : public ModelType::ModelBaseType
{
private:
	template<class InputBatch>
	void doEval(InputBatch const& patterns, RealMatrix& outputs)const{
		m_models[0].eval(patterns,outputs);
		outputs *=m_weight[0];
		for(std::size_t i = 1; i != m_models.size(); i++) 
			noalias(outputs) += m_weight[i] * m_models[i](patterns);
		outputs /= m_weightSum;
	}
	template<class InputBatch>
	void doEval(InputBatch const& patterns, blas::vector<unsigned int>& outputs)const{
		//evaluate all models on the batch and obtain number of classes
		std::vector<blas::vector<unsigned int> > responses(m_models.size());
		unsigned int numClasses = 0;
		for(std::size_t i = 0; i != m_models.size(); ++i){
			m_models[i].eval(patterns, responses[i]);
			numClasses = std::max(numClasses, max(responses[i]));
		}
		numClasses += 1;
		//compute weighted class probabilities
		RealMatrix classProbs(patterns.size1(), numClasses,0.0);
		for(std::size_t i = 0; i != m_models.size(); ++i){
			for(std::size_t p = 0; p != patterns.size1(); ++p){
				classProbs(p,responses[i](p)) += m_weight[i];
			}
		}
		
		outputs.resize(patterns.size1());
		for(std::size_t p = 0; p != patterns.size1(); ++p){
			outputs(p) = arg_max(row(classProbs,p));
		}
	}
public:
	typedef typename ModelType::ModelBaseType ModelBaseType;
	typedef typename ModelBaseType::BatchInputType BatchInputType;
	typedef typename ModelBaseType::BatchOutputType BatchOutputType;
	typedef typename ModelBaseType::ParameterVectorType ParameterVectorType;
	/// Constructor
	MeanModel():m_weightSum(0){}
	
	std::string name() const
	{ return "MeanModel"; }

	using ModelBaseType::eval;
	void eval(BatchInputType const& patterns, BatchOutputType& outputs)const{
		doEval(patterns,outputs);
	}
	
	void eval(BatchInputType const& patterns, BatchOutputType& outputs, State& state)const{
		eval(patterns,outputs);
	}


	/// This model does not have any parameters.
	ParameterVectorType parameterVector() const {
		return {};
	}

	/// This model does not have any parameters
	void setParameterVector(ParameterVectorType const& param) {
		SHARK_ASSERT(param.size() == 0);
	}
	void read(InArchive& archive){
		archive >> m_models;
		archive >> m_weight;
		archive >> m_weightSum;
	}
	void write(OutArchive& archive)const{
		archive << m_models;
		archive << m_weight;
		archive << m_weightSum;
	}

	/// \brief Removes all models from the ensemble
	void clearModels(){
		m_models.clear();
		m_weight.clear();
		m_weightSum = 0.0;
	}

	/// \brief Adds a new model to the ensemble.
	///
	/// \param model the new model
	/// \param weight weight of the model. must be > 0
	void addModel(ModelType const& model, double weight = 1.0){
		SHARK_RUNTIME_CHECK(weight > 0, "Weights must be positive");
		m_models.push_back(model);
		m_weight.push_back(weight);
		m_weightSum += weight;
	}
	
	ModelType const& getModel(std::size_t index){
		return m_models[index];
	}

	/// \brief Returns the weight of the i-th model
	double const& weight(std::size_t i)const{
		return m_weight[i];
	}
	
	/// \brief sets the weight of the i-th model
	void setWeight(std::size_t i, double newWeight){
		m_weightSum=newWeight - m_weight[i];
		m_weight[i] = newWeight;
	}
	
	/// \brief Returns the number of models.
	std::size_t numberOfModels()const{
		return m_models.size();
	}

protected:
	/// \brief collection of models.
	std::vector<ModelType> m_models;

	/// \brief Weight of the mean.
	RealVector m_weight;

	/// \brief Total sum of weights.
	double m_weightSum;
};


}
#endif // SHARK_MODELS_MEANMODEL_H
