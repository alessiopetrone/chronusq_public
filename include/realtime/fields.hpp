/* 
 *  This file is part of the Chronus Quantum (ChronusQ) software package
 *  
 *  Copyright (C) 2014-2017 Li Research Group (University of Washington)
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *  
 *  Contact the Developers:
 *    E-Mail: xsli@uw.edu
 *  
 */

#ifndef __INCLUDED_REALTIME_FIELDS_HPP__
#define __INCLUDED_REALTIME_FIELDS_HPP__

#include <chronusq_sys.hpp>
#include <fields.hpp>
#include <realtime/enums.hpp>

#include <realtime/fields/envelope.hpp>

namespace ChronusQ {

  /**
   *  \brief Base struct for the specification of an electromagnetic field
   *  perturbation.
   *
   *  Provides a minimal structure to build a TDEMField specification
   *  knowing that the TDEMField will all inherit EMFieldBase.
   *
   *  All TDEMField classes are derived from this one.
   */ 
  struct TDEMFieldBase {

    std::shared_ptr<FieldEnvelopeBase> envelope; ///< Field envelope

    // Default / delete constructors
    TDEMFieldBase()                      = delete;
    TDEMFieldBase(const TDEMFieldBase &) = default;
    TDEMFieldBase(TDEMFieldBase &&)      = default;

    /**
     *  \brief TDEMFieldBase constructor.
     *
     *  \param [in] env Shared pointer to a FieldEnvelope specification
     */
    TDEMFieldBase(std::shared_ptr<FieldEnvelopeBase> env): envelope(env){ };


    /**
     *  \brief Obtain the cartesian field tensor at a specificed time.
     *
     *  Pure virtual, to be specified by derived classes.
     *
     *  All derived classes are aware of some tensorial representation
     *  of the EM field perturbation, \f$ \hat{v} \f$. This function returns
     *
     *  \f[
     *    v(t) = \hat{v} \cdot f(t)
     *  \f]
     *
     *  where \f$ f \f$ is the envelope function.
     *
     *
     *  \param [in] t Time to evaluate the field amplitude
     *  \returns      The field amplitude at time \f$ t \f$.
     */ 
    virtual std::valarray<double> getAmp(double t) = 0; 

  };

  /**
   *  \brief Tensorial specification of an EM field perturbation.
   *
   *  Handles the specification of various descriptions of the EM field
   *  perturbation, such as the multipole expansion.
   *
   *  Expectes field amplidudes of the following:
   *  1. Dipole operator     - [x, y, z]
   *  2. Quadrupole operator - [xx, xy, xz, yy, yz, zz]
   *  ect...
   */ 
  template <typename _UnitVector>
  struct TDEMField : public TDEMFieldBase, public EMField<_UnitVector> {
    
    TDEMField()                  = delete;
    TDEMField(const TDEMField &) = default;
    TDEMField(TDEMField &&)      = default;


    TDEMField(EMFieldTyp em, FieldGauge fg, 
      std::shared_ptr<FieldEnvelopeBase> env, const _UnitVector &uv) :
      TDEMFieldBase(env), EMField<_UnitVector>(em,fg,uv){ }; 

    TDEMField(EMFieldTyp em, std::shared_ptr<FieldEnvelopeBase> env, 
      const _UnitVector &uv) :
      TDEMFieldBase(env), EMField<_UnitVector>(em,Length,uv){ }; 

    template <typename Env>
    TDEMField(EMFieldTyp em, FieldGauge fg, Env env, const _UnitVector &uv) :
      TDEMField(em,fg,cast(std::make_shared<Env>(env)),uv){ } 

    template <typename Env>
    TDEMField(EMFieldTyp em, Env env, const _UnitVector &uv) :
      TDEMField(em,Length,cast(std::make_shared<Env>(env)),uv){ } 

    inline std::valarray<double> getAmp(double t) { 
      return envelope->getAmp(t) * EMField<_UnitVector>::getAmp();
    };

  };




  // Convienence alias
  using TDEMFieldBase_ptr = std::shared_ptr<TDEMFieldBase>;





  /**
   *  \brief Cast a templated TDEMField shared_ptr to
   *  Base class.
   *
   *  dynamic_pointer_cast TDEMField<> -> TDEMFieldBase
   *
   *  \param [in] x Shared pointer for a TDEMField object
   *  \returns      Shared pointer for a TDEMFieldBase object
   */ 
  template < typename _UnitVector >
  std::shared_ptr<TDEMFieldBase> 
    cast(std::shared_ptr<TDEMField<_UnitVector>> x) {
    return std::dynamic_pointer_cast<
             TDEMFieldBase,TDEMField<_UnitVector>>(x);
  };

  // TDEMField aliases
  using TDDipoleField = TDEMField<std::array<double,3>>;

  /**
   *  \brief Full specification of the time-dependent EM perturbation
   *  for a RealTime simulation.
   *
   *  General to multiple fields.
   */ 
  struct TDEMPerturbation {

    std::vector<
      std::shared_ptr<TDEMFieldBase>
        > fields; ///< Fields for the EM perturbation


    /**
     *  \brief Add a field to the perturbation.
     *
     *  \param [in] f    Tensorial field specification
     */ 
    inline void addField(std::shared_ptr<TDEMFieldBase> f) {
      fields.push_back(f);
    };

    template <typename _TDField>
    inline void addField(std::shared_ptr<_TDField> f){ addField(cast(f)); };

    template <typename _TDField>
    inline void addField(_TDField f){ addField(std::make_shared<_TDField>(f)); }

    template <typename Env, typename _UV>
    inline void addField(EMFieldTyp em, FieldGauge fg, Env env, 
      const _UV &uv) {
      
      addField(TDEMField<_UV>(em,fg,env,uv));    

    }

    template <typename Env, typename _UV>
    inline void addField(EMFieldTyp em, Env env, const _UV &uv) {
      addField(TDEMField<_UV>(em,env,uv));    
    }


    /**
     *  \brief Obtain the tensorial field amplitude at a specific time
     *  as the sum of all the field contributions.
     *
     *  FIXME: This function only works if all fields in perturbation
     *  are of the same type (i.e. electric dipole, etc).
     *
     *  \param [in]
     */ 
    std::valarray<double> getAmp(double t){

      assert(fields.size() != 0);

      std::valarray<double> amp = fields[0]->getAmp(t);
      for(auto i = 1; i < fields.size(); i++)
        amp += fields[i]->getAmp(t);

      return amp;

    }

    EMPerturbation getPert(double t) {

      EMPerturbation pert;

      for(auto &field : fields) {
        auto amp = field->getAmp(t);

        if(amp.size() == 3) {
          TDDipoleField &fld = dynamic_cast<TDDipoleField&>(*field);
          std::array<double,3> dple = {amp[0], amp[1], amp[2]};

          pert.addField(fld.emFieldTyp,fld.fieldGauge,dple);
        }

      }

      return pert;

    }
    
  }; // struct TDEMPerturbation
};

#endif
