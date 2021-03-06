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
#ifndef __INCLUDED_SINGLESLATER_KOHNSHAM_HPP__
#define __INCLUDED_SINGLESLATER_KOHNSHAM_HPP__

#include <chronusq_sys.hpp>
#include <singleslater.hpp>
#include <basisset/basisset_util.hpp>
#include <cqlinalg/blasext.hpp>
#include <dft.hpp>

// KS_DEBUG_LEVEL == 1 - Timing
#ifndef KS_DEBUG_LEVEL
#  define KS_DEBUG_LEVEL 0
#endif

namespace ChronusQ {


  /**
   *  \brief A datastructure to hold the information
   *  pertaining to the control of the Kohn--Sham
   *  numerical integration.
   */ 
  struct IntegrationParam {
    double epsilon      = 1e-12; ///< Screening parameter
    size_t nAng         = 302;   ///< # Angular points
    size_t nRad         = 100;   ///< # Radial points
    size_t nRadPerBatch = 4;     ///< # Radial points / macro batch
  };


  /**
   *  \breif The Kohn--Sham class.
   *
   *  Specializes the SingleSlater class for a Kohn--Sham description of the
   *  many-body wave function
   */ 
  template <typename T>
  class KohnSham : public SingleSlater<T> {


  protected:

    // Useful typedefs
    typedef double*                   oper_t;
    typedef std::vector<oper_t>       oper_t_coll;
    typedef std::vector<oper_t_coll>  oper_t_coll2;

  public:

    std::vector<std::shared_ptr<DFTFunctional>> functionals; ///< XC kernels
    IntegrationParam intParam; ///< Numerical integration controls

    bool isGGA_; ///< Whether or not the XC kernel is within the GGA
    double XCEnergy; ///< Exchange-correlation energy

    oper_t_coll VXC; ///< VXC terms

    // Inherit ctors from SingleSlater<T>

    template <typename... Args>
    KohnSham(std::string funcName, 
      std::vector<std::shared_ptr<DFTFunctional>> funclist,
      AOIntegrals &aoi, Args... args) : 
      SingleSlater<T>(aoi,args...), WaveFunctionBase(aoi,args...),
      QuantumBase(aoi.memManager(),args...), isGGA_(false),
      functionals(std::move(funclist)) { 

      // Append HF tags to reference names
      if(this->nC == 1) {
        if(this->iCS) {
          this->refLongName_  += "Restricted " + funcName;
          this->refShortName_ += "R" + funcName;
        } else {
          this->refLongName_  += "Unrestricted " + funcName;
          this->refShortName_ += "U" + funcName;
        }
      } else {
        this->refLongName_  += "Generalized " + funcName;
        this->refShortName_ += "G" + funcName;
      }

      for(auto i = 0; i < this->onePDM.size(); i++)
        VXC.emplace_back(
          this->memManager.template malloc<double>(
            this->memManager.template getSize<T>(this->onePDM[i])
          )
        );


    }; // KohnSham constructor




    template <typename... Args>
    KohnSham(std::string rL, std::string rS, std::string funcName, 
      std::vector<std::shared_ptr<DFTFunctional>> funclist,
      AOIntegrals &aoi, Args... args) : 
      SingleSlater<T>(aoi,args...), WaveFunctionBase(aoi,args...),
      QuantumBase(aoi.memManager(),args...), isGGA_(false),
      functionals(std::move(funclist)) { 

      this->refLongName_  += rL + " " + funcName;
      this->refShortName_ += rS + funcName;

      for(auto i = 0; i < this->onePDM.size(); i++)
        VXC.emplace_back(
          this->memManager.template malloc<double>(
            this->memManager.template getSize<T>(this->onePDM[i])
          )
        );


    }; // KohnSham constructor


    // Copy and Move ctors
      
    template <typename U>
    KohnSham(const KohnSham<U> &other, int dummy = 0); 
    template <typename U>
    KohnSham(KohnSham<U> &&other, int dummy = 0);
    KohnSham(const KohnSham<T> &other);
    KohnSham(KohnSham<T> &&other);


    /**
     *  \brief Kohn-Sham specialization of formFock
     *
     *  Compute VXC and increment the fock matrix
     */  
    virtual void formFock(EMPerturbation &pert, bool increment = false, double HFX = 0.) {

      // FIXME: Add preprocesor
#if KS_DEBUG_LEVEL > 0
      std::chrono::duration<double> durHF(0.) ;
      std::chrono::duration<double> durXC(0.) ;
      std::chrono::duration<double> duraddXC(0.) ;
      auto topHFpart = std::chrono::high_resolution_clock::now();
#endif

      SingleSlater<T>::formFock(pert,increment,functionals.back()->xHFX);

#if KS_DEBUG_LEVEL > 0
      auto botHFpart = std::chrono::high_resolution_clock::now();
      auto topXCpart = std::chrono::high_resolution_clock::now();
#endif

      formVXC();

#if KS_DEBUG_LEVEL > 0
      auto botXCpart = std::chrono::high_resolution_clock::now();
      auto topaddXCpart = std::chrono::high_resolution_clock::now();
#endif

      // Add VXC in Fock matrix
      size_t NB = this->aoints.basisSet().nBasis;
      for(auto i = 0ul; i < this->fock.size(); i++)
        MatAdd('N','N', NB, NB, T(1.), this->fock[i], NB, T(1.), VXC[i], NB,
          this->fock[i], NB);

#if KS_DEBUG_LEVEL > 0
      auto botaddXCpart = std::chrono::high_resolution_clock::now();
      durHF = botHFpart - topHFpart;
      durXC = botXCpart - topXCpart;
      duraddXC = botaddXCpart - topaddXCpart;
      std::cerr << "HF FormFock " << durHF.count() << std::endl;
      std::cerr << "XC FormFock " << durXC.count() << std::endl;
      std::cerr << "addXC FormFock " << duraddXC.count() << std::endl;
#endif

    }; // formFock

    /**
     *  \brief Kohn-Sham specialization of computeEnergy
     *
     *  Compute EXC and add it to the HF energy 
     */  
    virtual void computeEnergy() {

      SingleSlater<T>::computeEnergy();
      // Add EXC in the total energy
      this->totalEnergy += XCEnergy;
        
    }; // computeEnergy

    // KS specific functions
    // See include/singleslater/kohnsham/vxc.hpp for docs.

    void formVXC(); 

    void evalDen(SHELL_EVAL_TYPE typ, size_t NPts,size_t NBE, size_t NB, 
      std::vector<std::pair<size_t,size_t>> &subMatCut, double *SCR1,
      double *SCR2, double *DENMAT, double *Den, double *GDenX, double *GDenY, double *GDenZ,
      double *BasisScr);

    void formZ_vxc(DENSITY_TYPE denTyp, bool isGGA, size_t NPts, size_t NBE, size_t IOff, 
      double epsScreen, std::vector<double> &weights, double *ZrhoVar1,
      double *ZsigmaVar1, double *ZsigmaVar2, 
      double *DenS, double *DenZ, double *DenY, double *DenX, 
      double *GDenS, double *GDenZ, double *GDenY, double *GDenX, 
      double *Kx, double *Ky, double *Kz, 
      double *Hx, double *Hy, double *Hz,
      double *BasisScratch, double *ZMAT);

    double energy_vxc(size_t NPts, std::vector<double> &weights, double *EpsEval, double *Den);

    void mkAuxVar(bool isGGA, 
      double epsScreen, size_t NPts_Batch, 
      double *n, double *mx, double *my, double *mz,
      double *dndX, double *dndY, double *dndZ, 
      double *dmxdX, double *dmxdY, double *dmxdZ, 
      double *dmydX, double *dmydY, double *dmydZ, 
      double *dmzdX, double *dmzdY, double *dmzdZ, 
      double *Mnorm, double *Kx, double *Ky, double *Kz, 
      double *Hx, double *Hy, double *Hz,
      bool* Msmall, double *nColl, double *gammaColl );

    void loadVXCder(size_t NPts, double *Den, double *sigma, double *EpsEval, double*VRhoEval, 
      double *VsigmaEval, double *EpsSCR, double *VRhoSCR, double *VsigmaSCR); 

    void constructZVars(DENSITY_TYPE denTyp, bool isGGA, size_t NPts, 
      double *VrhoEval, double *VsigmaEval, double *ZrhoVar1, 
      double *ZsigmaVar1, double *ZsigmaVar2);


  }; // class KohnSham


}; // namespace ChronusQ

#endif
