/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2011, Willow Garage
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */

#ifndef OMPL_BASE_GENERIC_PARAM_
#define OMPL_BASE_GENERIC_PARAM_

#include "ompl/util/Console.h"
#include "ompl/util/ClassForward.h"
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace ompl
{
    namespace base
    {

        /// @cond IGNORE
        /** \brief Forward declaration of ompl::base::GenericParam */
        ClassForward(GenericParam);
        /// @endcond

        /** \brief Motion planning algorithms often employ parameters
            to guide their exploration process. (e.g., goal
            biasing). Motion planners (and some of their components)
            use this class to declare what the parameters are, in a
            generic way, so that they can be set externally. */
        class GenericParam
        {
        public:

            /** \brief The constructor of a parameter takes the name of the parameter (\e name) */
            GenericParam(const std::string &name) : name_(name), msg_(&msgDefault_)
            {
            }

            /** \brief The constructor of a parameter takes the name of the parameter (\e name) and a context (\e context) for that parameter.
                The contex is used for console output */
            GenericParam(const std::string &name, const msg::Interface &context) : name_(name), msg_(&context)
            {
            }

            virtual ~GenericParam(void)
            {
            }

            /** \brief Get the name of the parameter */
            const std::string& getName(void) const
            {
                return name_;
            }

            /** \brief Set the name of the parameter */
            void setName(const std::string &name)
            {
                name_ = name;
            }

            /** \brief Set the value of the parameter. The value is taken in as a string, but converted to the type of that parameter. */
            virtual bool setValue(const std::string &value) = 0;

            /** \brief Retrieve the value of the parameter, as a string. */
            virtual std::string getValue(void) const = 0;

        private:

            msg::Interface msgDefault_;

        protected:

            /** \brief The name of the parameter */
            std::string           name_;

            /** \brief Interface for publishing console messages */
            const msg::Interface *msg_;
        };


        /** \brief This is a helper class that instantiates
            planner parameters of different types. */
        template<typename T>
        class SpecificParam : public GenericParam
        {
        public:

            /** \brief The type for the 'setter' function for this planner parameter */
            typedef boost::function<void(T)> SetterFn;

            /** \brief The type for the 'getter' function for this planner parameter */
            typedef boost::function<T()>     GetterFn;

            /** \brief An explicit instantiation of a planner
                parameter requires the \e setter function and optionally the \e
                getter function, in addition to the \e planner and
                the parameter \e name. */
            SpecificParam(const std::string &name, const SetterFn &setter, const GetterFn &getter = GetterFn()) :
                GenericParam(name), setter_(setter), getter_(getter)
            {
            }

            SpecificParam(const std::string &name, const msg::Interface &context, const SetterFn &setter, const GetterFn &getter = GetterFn()) :
                GenericParam(name, context), setter_(setter), getter_(getter)
            {
            }

            virtual ~SpecificParam(void)
            {
            }

            virtual bool setValue(const std::string &value)
            {
                bool result = true;
                try
                {
                    setter_(boost::lexical_cast<T>(value));
                }
                catch (boost::bad_lexical_cast &e)
                {
                    result = false;
                    msg_->warn("Invalid value format specified for parameter '%s': %s", name_.c_str(), e.what());
                }

                if (getter_)
                    msg_->debug("The value of parameter '" + name_ + "' is now: '" + getValue() + "'");
                else
                    msg_->debug("The value of parameter '" + name_ + "' was set to: '" + value + "'");
                return result;
            }

            virtual std::string getValue(void) const
            {
                if (getter_)
                    try
                    {
                        return boost::lexical_cast<std::string>(getter_());
                    }
                    catch (boost::bad_lexical_cast &e)
                    {
                        msg_->warn("Unable to parameter '%s' to string: %s", name_.c_str(), e.what());
                        return "";
                    }
                else
                    return "";
            }

        protected:

            /** \brief The setter function for this parameter */
            SetterFn setter_;

            /** \brief The getter function for this parameter */
            GetterFn getter_;
        };

        /** \brief Maintain a set of parameters */
        class ParamSet
        {
        public:

            /** \brief This function declares a parameter \e name, and specifies the \e setter and \e getter functions. */
            template<typename T>
            void declareParam(const std::string &name, const msg::Interface &context, const typename SpecificParam<T>::SetterFn &setter,
                              const typename SpecificParam<T>::GetterFn &getter = typename SpecificParam<T>::GetterFn())
            {
                params_[name].reset(new SpecificParam<T>(name, context, setter, getter));
            }

            /** \brief This function declares a parameter \e name, and specifies the \e setter and \e getter functions. */
            template<typename T>
            void declareParam(const std::string &name, const typename SpecificParam<T>::SetterFn &setter,
                              const typename SpecificParam<T>::GetterFn &getter = typename SpecificParam<T>::GetterFn())
            {
                params_[name].reset(new SpecificParam<T>(name, setter, getter));
            }

            /** \brief Add a parameter to the set */
            void add(const GenericParamPtr &param);

            /** \brief Remove a parameter from the set */
            void remove(const std::string &name);

            /** \brief Include the params of a different ParamSet into this one. Optionally include a prefix for each of the parameters */
            void include(const ParamSet &other, const std::string &prefix = "");

            /** \brief Planning algorithms typically have parameters
                that can be set externally. While each planner will
                have getter and setter functions specifically for
                those parameters, this function allows setting
                parameters generically, for any planner, by specifying
                the parameter name \e key and its value \e value (both
                as string). This makes it easy to automatically
                configure planners using external sources (e.g., a
                configuration file). The function returns true if the
                parameter was parsed successfully and false
                otherwise. */
            bool setParam(const std::string &key, const std::string &value);

            /** \brief Set a list of key-value pairs as parameters for
                the planner. Return true if all parameters were set
                successfully. This function simply calls setParam() multiple times */
            bool setParams(const std::map<std::string, std::string> &kv);

            /** \brief Get the known parameter as a map from names to values cast as string */
            void getParams(std::map<std::string, std::string> &params) const;

            /** \brief List the names of the known parameters */
            void getParamNames(std::vector<std::string> &params) const;

            /** \brief List the values of the known parameters, in the same order as getParamNames() */
            void getParamValues(std::vector<std::string> &vals) const;

            /** \brief Get the map from parameter names to parameter descriptions */
            const std::map<std::string, GenericParamPtr>& getParams(void) const;

            /** \brief Get the number of parameters maintained by this instance */
            std::size_t size(void) const
            {
                return params_.size();
            }

            /** \brief Clear all the set parameters */
            void clear(void);

            /** \brief Print the parameters to a stream */
            void print(std::ostream &out) const;

        private:

            std::map<std::string, GenericParamPtr> params_;
        };
    }
}

#endif
