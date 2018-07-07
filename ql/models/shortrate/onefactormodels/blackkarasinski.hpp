/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file blackkarasinski.hpp
    \brief Black-Karasinski model
*/

#ifndef quantlib_black_karasinski_hpp
#define quantlib_black_karasinski_hpp

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>

namespace QuantLib {

    //! Standard Black-Karasinski model class.
    /*! This class implements the standard Black-Karasinski model defined by
        \f[
            d\ln r_t = (\theta(t) - \alpha \ln r_t)dt + \sigma dW_t,
        \f]
        where \f$ alpha \f$ and \f$ sigma \f$ are constants.

        \ingroup shortrate
    */
    class BlackKarasinski : public OneFactorModel,
                            public TermStructureConsistentModel {
      public:
        BlackKarasinski(const Handle<YieldTermStructure>& termStructure,
                        Real a = 0.1, Real sigma = 0.1);

        ext::shared_ptr<ShortRateDynamics> dynamics() const;

        ext::shared_ptr<Lattice> tree(const TimeGrid& grid) const;

      protected:
        void generateArguments();

      private:
        class Dynamics;
        class Helper;
        class FittingParameter;

        Real a() const { return a_(0.0); }
        Real sigma() const { return sigma_(0.0); }

        Parameter& a_;
        Parameter& sigma_;
        Parameter phi_;
    };

    //! Short-rate dynamics in the Black-Karasinski model
    /*! The short-rate is here
        \f[
            r_t = e^{\varphi(t) + x_t}
         \f]
         where \f$ \varphi(t) \f$ is the deterministic time-dependent
         parameter (which can not be determined analytically)
         used for term-structure fitting and \f$ x_t \f$ is the state
         variable following an Ornstein-Uhlenbeck process.
    */
    class BlackKarasinski::Dynamics
        : public BlackKarasinski::ShortRateDynamics {
      public:
        Dynamics(const Parameter& fitting, Real alpha, Real sigma)
        : ShortRateDynamics(ext::shared_ptr<StochasticProcess1D>(
                                 new OrnsteinUhlenbeckProcess(alpha, sigma))),
          fitting_(fitting) {}

        Real variable(Time t, Rate r) const {
            return std::log(r) - fitting_(t);
        }

        Real shortRate(Time t, Real x) const {
            return std::exp(x + fitting_(t));
        }
      private:
        Parameter fitting_;
    };

    class BlackKarasinski::FittingParameter
        : public TermStructureFittingParameter {
    private:
        class Impl :public Parameter::Impl{
        public:
            Impl(const Handle<YieldTermStructure>& termStructure,
                Real a, Real sigma)
                :termStructure_(termStructure), a_(a), sigma_(sigma) {}

            Real value(const Array&, Time t) const {
                QL_FAIL("BlackKarasinski::FittingParameter::Impl::value not implemented.");
            }

        private:
            Handle<YieldTermStructure> termStructure_;
            Real a_, sigma_;
        };
    public:
        FittingParameter(const Handle<YieldTermStructure>& termStructure,
                         Real a, Real sigma)
            : TermStructureFittingParameter(ext::shared_ptr<Parameter::Impl>(
                new FittingParameter::Impl(termStructure, a, sigma))) {}
    };


	//inline definitions

    inline ext::shared_ptr<OneFactorModel::ShortRateDynamics>
    BlackKarasinski::dynamics() const {
        return ext::shared_ptr<ShortRateDynamics>(
            new Dynamics(phi_, a(), sigma()));
    }

}


#endif

