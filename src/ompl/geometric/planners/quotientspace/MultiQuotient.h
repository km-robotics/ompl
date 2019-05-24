/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2019, University of Stuttgart
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
*   * Neither the name of the University of Stuttgart nor the names 
*     of its contributors may be used to endorse or promote products 
*     derived from this software without specific prior written 
*     permission.
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

/* Author: Andreas Orthey */

#ifndef OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_MULTIQUOTIENT_
#define OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_MULTIQUOTIENT_
#include "Quotient.h"
#include <type_traits>
#include <queue>

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace ompl
{
    namespace geometric
    {
        /** \brief A sequence of multiple quotient-spaces
             The class MultiQuotient can be used with any planner which inherits
             the og::Quotient class. 
           
             Example usage with QRRT 
             (using a sequence si_vec of ob::SpaceInformationPtr)
               ob::PlannerPtr planner =
                 std::make_shared<MultiQuotient<og::QRRT> >(si_vec); */
           
        template <class T>
        class MultiQuotient: public ob::Planner
        {

        static_assert(std::is_base_of<og::Quotient, T>::value, "Template must inherit from Quotient");

        public:
            const bool DEBUG{false};
            MultiQuotient(std::vector<ob::SpaceInformationPtr> &si_vec, std::string type = "QuotientPlanner");
            void setProblemDefinition(std::vector<ob::ProblemDefinitionPtr> &pdef_vec_);

            virtual ~MultiQuotient() override;

            void getPlannerData(base::PlannerData &data) const override;
            ob::PlannerStatus solve(const base::PlannerTerminationCondition &ptc) override;
            void setup() override;
            void clear() override;
            void setProblemDefinition(const ob::ProblemDefinitionPtr &pdef) override;

            /// Number of quotient-spaces
            int getLevels() const;
            std::vector<int> getFeasibleNodes() const;
            std::vector<int> getNodes() const;
            std::vector<int> getDimensionsPerLevel() const;
            void setStopLevel(uint level_);

        protected:
            std::vector<base::PathPtr> solutions_;
            /// Sequence of quotient-spaces
            std::vector<og::Quotient*> quotientSpaces_;

            /// Indicator if a solution has been found on the current quotient-spaces
            bool foundKLevelSolution_{false};
            /// Current level on which we have not yet found a path
            uint currentQuotientLevel_{0};
            /// \brief Sometimes we only want to plan until a certain quotient-space
            /// level (for debugging for example). This variable sets the stopping
            /// level.
            uint stopAtLevel_;

            std::vector<ob::SpaceInformationPtr> siVec_;
            std::vector<ob::ProblemDefinitionPtr> pdefVec_;

            /// Compare function for priority queue 
            struct CmpQuotientSpacePtrs
            {
                // ">" operator: smallest value is top in queue
                // "<" operator: largest value is top in queue (default)
                bool operator()(const Quotient* lhs, const Quotient* rhs) const
                {
                     return lhs->getImportance() < rhs->getImportance();
                }
            };
            /// \brief Priority queue of quotient-spaces which keeps track of how often
            /// every tree on each space has been expanded.
            typedef std::priority_queue<og::Quotient*, std::vector<og::Quotient*>, CmpQuotientSpacePtrs> QuotientSpacePriorityQueue;
            QuotientSpacePriorityQueue priorityQueue_;
        };
    }
}
#include "src/MultiQuotient.ipp"
#endif
