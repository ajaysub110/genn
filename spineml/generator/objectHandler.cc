#include "objectHandler.h"

// Standard C includes
#include <cmath>

// Standard C++ includes
#include <iostream>
#include <regex>

// pugixml includes
#include "pugixml/pugixml.hpp"

//------------------------------------------------------------------------
// SpineMLGenerator::ObjectHandler::Condition
//------------------------------------------------------------------------
void SpineMLGenerator::ObjectHandler::Condition::onObject(const pugi::xml_node &node,
                                                          unsigned int currentRegimeID, unsigned int targetRegimeID)
{
    // Get triggering code
    auto triggerCode = node.child("Trigger").child("MathInline");
    if(!triggerCode) {
        throw std::runtime_error("No trigger condition for transition between regimes");
    }

    // Expand out any aliases
    std::string triggerCodeString = triggerCode.text().get();
    expandAliases(triggerCodeString, m_Aliases);

    // Write trigger condition
    m_CodeStream << "if(" << triggerCodeString << ")" << CodeStream::OB(2);

    // Loop through state assignements
    for(auto stateAssign : node.children("StateAssignment")) {
        // Expand out any aliases in code string
        std::string stateAssignCodeString = stateAssign.child_value("MathInline");
        expandAliases(stateAssignCodeString, m_Aliases);

        // Write state assignement
        m_CodeStream << stateAssign.attribute("variable").value() << " = " << stateAssignCodeString << ";" << std::endl;
    }

    // If this condition results in a regime change
    if(currentRegimeID != targetRegimeID) {
        m_CodeStream << "_regimeID = " << targetRegimeID << ";" << std::endl;
    }

    // End of trigger condition
    m_CodeStream << CodeStream::CB(2);
}

//------------------------------------------------------------------------
// SpineMLGenerator::ObjectHandler::TimeDerivative
//------------------------------------------------------------------------
void SpineMLGenerator::ObjectHandler::TimeDerivative::onObject(const pugi::xml_node &node,
                                                               unsigned int, unsigned int)
{
    // Expand out any aliases in code string
    std::string codeString = node.child_value("MathInline");
    expandAliases(codeString, m_Aliases);

    // Find name of state variable
    const std::string stateVariable = node.attribute("variable").value();

    // If regex locates obvious linear first-order ODEs of the form '-x / tau' with a closed-form solution
    std::regex regex("\\s*-\\s*" + stateVariable + "\\s*\\/\\s*([a-zA-Z_]+)\\s*");
    std::cmatch match;
    if(std::regex_match(codeString.c_str(), match, regex)) {
        std::cout << "\t\t\t\tLinear dynamics with time constant:" << match[1].str() << " identified" << std::endl;

        // Stash name of tau parameter in class
        m_ClosedFormTauParamName = match[1].str();

        // Build code to decay state variable
        m_CodeStream << stateVariable << " *=  _expDecay" << m_ClosedFormTauParamName << ";" << std::endl;
    }
    // Otherwise use Euler
    else {
        m_CodeStream << stateVariable << " += DT * (" << codeString << ");" << std::endl;
    }
}
//------------------------------------------------------------------------
void SpineMLGenerator::ObjectHandler::TimeDerivative::addDerivedParams(const NewModels::Base::StringVec &paramNames,
                                                                       NewModels::Base::DerivedParamVec &derivedParams) const
{
    // If a closed form tau parameter exists
    if(!m_ClosedFormTauParamName.empty()) {
        // If tau parameter exists
        const size_t tauParamIndex = std::distance(
            paramNames.cbegin(), std::find(paramNames.cbegin(), paramNames.cend(), m_ClosedFormTauParamName));
        if(tauParamIndex < paramNames.size()) {
            // Add a derived parameter to calculate e ^ (-dt / tau)
            derivedParams.push_back(std::make_pair("_expDecay" + m_ClosedFormTauParamName,
                                                   [tauParamIndex](const std::vector<double> &params, double dt)
                                                   {
                                                       return std::exp(-dt / params[tauParamIndex]);
                                                   }));
        }
        // Otherwise, give an error
        else {
            throw std::runtime_error("Time derivative uses time constant '" + m_ClosedFormTauParamName + "' which isn't a model parameter");
        }
    }
}
