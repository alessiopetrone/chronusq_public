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

#include <basisset/basisset_def.hpp>
#include <basisset/reference.hpp>
#include <cxxapi/output.hpp>

#include <util/matout.hpp>
#include <cqlinalg/blas1.hpp>
#include <cqlinalg/blas3.hpp>

namespace ChronusQ {

  // Basis set keyword map
  // TODO: Keywords for the Sapporo sets
  std::unordered_map<std::string,std::string> basisKeyword = {
    {  "STO-3G"         , "sto3g.gbs"                     },
    {  "STO-6G"         , "sto6g.gbs"                     },
    {  "3-21G"          , "3-21g.gbs"                     },
    {  "4-31G"          , "4-31g.gbs"                     },
    {  "6-31G"          , "6-31g.gbs"                     },
    {  "6-31G(D)"       , "6-31g*.gbs"                    },
    {  "6-31++G(D)"     , "6-31++g*.gbs"                  },
    {  "6-311G"         , "6-311g.gbs"                    },
    {  "6-311+G(D)"     , "6-311+g*.gbs"                  },
    {  "6-311+G(D,P)"   , "6-311+g**.gbs"                 },
    {  "6-311+G(2D,P)"  , "6-311+g_2d_p.gbs"              },
    {  "CC-PVDZ"        , "cc-pvdz.gbs"                   },
    {  "CC-PVTZ"        , "cc-pvtz.gbs"                   },
    {  "CC-PVQZ"        , "cc-pvqz.gbs"                   },
    {  "CC-PV5Z"        , "cc-pv5z.gbs"                   },
    {  "CC-PV6Z"        , "cc-pv6z.gbs"                   },
    {  "CC-PVDZ-RI"     , "cc-pvdz_ri.gbs"                },
    {  "DEF2-SVP"       , "def2-svp.gbs"                  },
    {  "DEF2-SVPD"      , "def2-svpd.gbs"                 },
    {  "DEF2-TZVP"      , "def2-tzvp.gbs"                 },
    {  "SAPPORO-DKH3-DZP-2012-ALL" , "sapporo-dkh3-dzp-2012_all.gbs" },
    {  "SAPPORO-DKH3-DZP-2012-NO"  , "sapporo-dkh3-dzp-2012_no.gbs"  },
    {  "SAPPORO-DKH3-DZP-2012-SP"  , "sapporo-dkh3-dzp-2012_sp.gbs"  },
    {  "SAPPORO-DKH3-DZP-ALL"      , "sapporo-dkh3-dzp_all.gbs"      },
    {  "SAPPORO-DKH3-DZP-NO"       , "sapporo-dkh3-dzp_no.gbs"       },
    {  "SAPPORO-DKH3-DZP-SP"       , "sapporo-dkh3-dzp_sp.gbs"       },
    {  "SAPPORO-DKH3-QZP-2012-ALL" , "sapporo-dkh3-qzp-2012_all.gbs" },
    {  "SAPPORO-DKH3-QZP-2012-NO"  , "sapporo-dkh3-qzp-2012_no.gbs"  },
    {  "SAPPORO-DKH3-QZP-2012-SP"  , "sapporo-dkh3-qzp-2012_sp.gbs"  },
    {  "SAPPORO-DKH3-QZP-ALL"      , "sapporo-dkh3-qzp_all.gbs"      },
    {  "SAPPORO-DKH3-QZP-NO"       , "sapporo-dkh3-qzp_no.gbs"       },
    {  "SAPPORO-DKH3-QZP-SP"       , "sapporo-dkh3-qzp_sp.gbs"       },
    {  "SAPPORO-DKH3-TZP-2012-ALL" , "sapporo-dkh3-tzp-2012_all.gbs" },
    {  "SAPPORO-DKH3-TZP-2012-NO"  , "sapporo-dkh3-tzp-2012_no.gbs"  },
    {  "SAPPORO-DKH3-TZP-2012-SP"  , "sapporo-dkh3-tzp-2012_sp.gbs"  },
    {  "SAPPORO-DKH3-TZP-ALL"      , "sapporo-dkh3-tzp_all.gbs"      },
    {  "SAPPORO-DKH3-TZP-NO"       , "sapporo-dkh3-tzp_no.gbs"       },
    {  "SAPPORO-DKH3-TZP-SP"       , "sapporo-dkh3-tzp_sp.gbs"       },
    {  "SAPPORO-DZP-2012-ALL"      , "sapporo-dzp-2012_all.gbs"      },
    {  "SAPPORO-DZP-2012-NO"       , "sapporo-dzp-2012_no.gbs"       },
    {  "SAPPORO-DZP-2012-SP"       , "sapporo-dzp-2012_sp.gbs"       },
    {  "SAPPORO-DZP-ALL"           , "sapporo-dzp_all.gbs"           },
    {  "SAPPORO-DZP-NO"            , "sapporo-dzp_no.gbs"            },
    {  "SAPPORO-DZP-SP"            , "sapporo-dzp_sp.gbs"            },
    {  "SAPPORO-QZP-2012-ALL"      , "sapporo-qzp-2012_all.gbs"      },
    {  "SAPPORO-QZP-2012-NO"       , "sapporo-qzp-2012_no.gbs"       },
    {  "SAPPORO-QZP-2012-SP"       , "sapporo-qzp-2012_sp.gbs"       },
    {  "SAPPORO-QZP-ALL"           , "sapporo-qzp_all.gbs"           },
    {  "SAPPORO-QZP-NO"            , "sapporo-qzp_no.gbs"            },
    {  "SAPPORO-QZP-SP"            , "sapporo-qzp_sp.gbs"            },
    {  "SAPPORO-TZP-2012-ALL"      , "sapporo-tzp-2012_all.gbs"      },
    {  "SAPPORO-TZP-2012-NO"       , "sapporo-tzp-2012_no.gbs"       },
    {  "SAPPORO-TZP-2012-SP"       , "sapporo-tzp-2012_sp.gbs"       },
    {  "SAPPORO-TZP-ALL"           , "sapporo-tzp_all.gbs"           },
    {  "SAPPORO-TZP-NO"            , "sapporo-tzp_no.gbs"            },
    {  "SAPPORO-TZP-SP"            , "sapporo-tzp_sp.gbs"            }
  };


  /**
   *  Path / Molecule BasisSet constructor 
   *
   *  Constructs a BasisSet object and populates member data given
   *  a basis set name and a Molecule object
   *
   *  \param [in] basisName Either a basis file in G94 format
   *                        or a known keyword which maps to a basis
   *                        file
   *  \param [in] mol       Molecule for which to construct the BasisSet
   */
  BasisSet::BasisSet(std::string _basisName, const Molecule &mol,
    bool _forceCart, bool doPrint) {

    forceCart = _forceCart;
    basisName = _basisName;

    std::string uppercase(basisName);

    // Convert the passed string to uppercase
    std::transform(basisName.begin(),basisName.end(),uppercase.begin(),
      [](unsigned char c){ return std::toupper(c); });  

    // Possibly find appropriate basis file for keyword
    if( basisKeyword.find(uppercase) != basisKeyword.end() )
      basisName = basisKeyword[basisName];

    // Generate the reference basis set of that keyword
    ReferenceBasisSet ref(basisName, _forceCart, doPrint);

    // Update appropriate shell set and coefficients for the Molecule
    // object
    std::tie(shells,unNormCont) = std::move(ref.generateShellSet(mol));

    // Obtain a copy of the basis centers
    std::for_each(mol.atoms.begin(),mol.atoms.end(),
      [&]( const Atom &at ){ centers.emplace_back(at.coord); }
    );

    // Update the BasisSt member data
    update();
    
  }; // BasisSet::BasisSet 


  /**
   *  Updates the member data of a BasisSet object.
   *
   *  Computes nShell, nBasis and nPrimitive. Determines  maxL and maxPrim.
   *  Constructs the shell mappings to basis function number and center number
   */ 
  void BasisSet::update() {

    // Compute the number of shells
    nShell = shells.size();

    // Compute the number of Basis functions and primitives
    std::tie(nBasis,nPrimitive) = std::accumulate(shells.begin(),shells.end(),
      std::pair<size_t,size_t>{0,0},
      [](std::pair<size_t,size_t> init, libint2::Shell &sh) ->
         std::pair<size_t,size_t> {
        return { init.first + sh.size(), 
                 init.second + sh.size()*sh.alpha.size() };
      }
    );

    // Determine the maximum angular momentum
    maxL = std::max_element(shells.begin(), shells.end(),
      [](libint2::Shell &sh1, libint2::Shell &sh2){
        return sh1.contr[0].l < sh2.contr[0].l;
      }
    )->contr[0].l;

    // Determine the maximum contraction depth
    maxPrim = std::max_element(shells.begin(), shells.end(),
      [](libint2::Shell &sh1, libint2::Shell &sh2){
        return sh1.alpha.size() < sh2.alpha.size();
      }
    )->alpha.size();


    // Reset maps
    mapSh2Cen.clear();
    mapSh2Bf.clear();
    mapCen2BfSt.clear();

    // Create basis maps
    // Maps Sh # -> BF #
    // Maps Sh # -> Center index
    size_t run(0);
    std::for_each(shells.begin(),shells.end(),
      [&](libint2::Shell &sh) {
        auto cenIt = std::find_if(centers.begin(),centers.end(),
          [&](cart_t &cen){ 
            //return sh.O == cen; 
            return std::abs(sh.O[0] - cen[0]) < 1e-5 and  
                   std::abs(sh.O[1] - cen[1]) < 1e-5 and  
                   std::abs(sh.O[2] - cen[2]) < 1e-5; 
          }
        );
        mapSh2Cen.emplace_back(std::distance(centers.begin(),cenIt));

        mapSh2Bf.emplace_back(run);
        run += sh.size(); 
        
      }
    );


    for(auto iAtm = 0; iAtm < centers.size(); iAtm++) {

      auto it = std::find_if(mapSh2Cen.begin(),mapSh2Cen.end(),
                  [&](size_t x){ return x == iAtm; });

      size_t firstShell = std::distance(mapSh2Cen.begin(),it);
      mapCen2BfSt.emplace_back(mapSh2Bf[firstShell]);

    }

  }; // BasisSet::update



  /**
   *  Outputs relevant information for the BasisSet object
   *  to a specified output.
   *
   *  \param [in/out] out Ouput device
   *  \paral [in]     mol BasisSet object to output.
   */ 
  std::ostream& operator<<(std::ostream &out, const BasisSet &basis) {

    out << std::endl;
    out << "Basis Set Information:" << std::endl << BannerTop
        << std::endl << std::endl;
    
    out << std::left;

    out << "  " << std::setw(25) << "NBasis" << basis.nBasis 
        << std::endl;
    out << "  " << std::setw(25) << "NPrimitive" << basis.nPrimitive 
        << std::endl;
    out << "  " << std::setw(25) << "NShell" << basis.nShell 
        << std::endl;
    out << "  " << std::setw(25) << "Max Primitive" << basis.maxPrim 
        << std::endl;
    out << "  " << std::setw(25) << "Max L" << basis.maxL << std::endl;

    out << std::endl << std::endl;

    out << "  " << "Shell Information:" << std::endl << bannerMid << std::endl;

    out << "  " << "  " << std::left;
    out << std::setw(5) << "#" ;
    out << std::setw(5) << "L" ;
    out << std::setw(15) << std::right << "Exponents";
    out << std::setw(30) << std::right << "Normalized Contraction";
  //out << std::setw(30) << std::right << "Unnormalized Contraction";
    out << std::endl << std::endl;
    
    for(auto iShell = 0; iShell < basis.nShell; iShell++){
      out << "  " << "  " << std::left << std::setprecision(4) 
          << std::scientific;
      out << std::setw(5) << iShell ;
      out << std::setw(5) << basis.shells[iShell].contr[0].l << std::right;
      out << std::setw(15) << basis.shells[iShell].alpha[0];
      out << std::setw(30) << basis.shells[iShell].contr[0].coeff[0];
    //out << std::setw(30) << basis.unNormCont[iShell][0];

      out << std::endl;
      for(auto iPrim = 1; iPrim < basis.shells[iShell].alpha.size(); iPrim++){
        out << "  " << "  " << std::setw(5+5) << " ";
        out << std::setw(15) << basis.shells[iShell].alpha[iPrim];
        out << std::setw(30) << basis.shells[iShell].contr[0].coeff[iPrim];
      //out << std::setw(30) << basis.unNormCont[iShell][iPrim];

        out << std::endl;
      }
      out << std::endl;
    }
    out << std::endl << BannerEnd << std::endl;

    return out; // return std::ostream reference

  }; // BasisSet::operator<<


  /**
   *  \brief Return the uncontracted shell set of the current
   *  contracted shell set
   */ 
  std::vector<libint2::Shell> BasisSet::uncontractShells() {

    std::vector<libint2::Shell> newShells;

    for(auto &shell : shells) // Loop over shells
    for(auto &a : shell.alpha) // Loop over primitives
      newShells.push_back(
        libint2::Shell{
          { a },
          { {shell.contr[0].l, shell.contr[0].pure, { 1.0 } } },
          { { shell.O[0], shell.O[1], shell.O[2] } }
        }
      );
    


    return newShells;


  }; // BasisSet::uncontractShells


  void BasisSet::makeMapPrim2Cont(double *SUn, double *MAP, CQMemManager &mem) {

    memset(MAP,0,nPrimitive * nBasis * sizeof(double));

    double *rA = MAP;

    // Compute the unnormalized mapping
    for(auto iSh = 0; iSh < nShell; iSh++) {

      size_t nPrim = shells[iSh].alpha.size();
      size_t nBf   = shells[iSh].size();

      for(auto iP = 0ul; iP < nPrim; iP++)
      for(auto iB = 0ul; iB < nBf;   iB++) 
        rA[iB + (iP*nBf + iB)*nBasis] = unNormCont[iSh][iP];

      rA += nPrim * nBf * nBasis + nBf;

    } // loop over shells


 
    // Compute SUn * MAP
    double *SCR = mem.malloc<double>(nBasis*nPrimitive);
    Gemm('N','T',nPrimitive,nBasis,nPrimitive,1.,SUn,nPrimitive,
      MAP,nBasis,0.,SCR,nPrimitive);

    // Create the Libint2 Engine and set the precision
    libint2::Engine engine(libint2::Operator::overlap,
      maxPrim, maxL, 0);
    engine.set_precision(0.);

    const auto& buf_vec = engine.results();

    // Loop over the shells and the basis functions,
    // calculate the diagonals of contracted overlap integrals
    size_t n1;
    for(size_t s1 = 0,  Itot = 0 ; s1<nShell; s1++, Itot += n1 ) {
    // Itot is the beginning basis function index of current shell

      // Get the size of the shell
      n1 = shells[s1].size();  

      // This computes the diagonal shell block for s1
      engine.compute(shells[s1],shells[s1]);

      if( buf_vec[0] == nullptr ) continue;
      const double* buff = buf_vec[0];

      for(size_t i = 0; i < n1; i++) {
      // i loop over the function in the current shell             

        double fact = InnerProd<double>(nPrimitive,MAP + (i+Itot) ,nBasis,
                                                   SCR + (i+Itot)*nPrimitive,1);
        // i+I is the basis function index 
        double alpha = buff[i + i*n1];
        // alpha is the diagonal element of overlap of basis function i+Itot
         
        Scale(nPrimitive,std::sqrt(alpha)/std::sqrt(fact),
          MAP + (i+Itot),nBasis);

      } // for size_t i = 0
    } // for size_t s1 = 0
    mem.free(SCR);

  };  // BasisSet::makeMapPrim2Cont

















  std::vector<std::vector<std::array<int,3>>> cart_ang_list;
  void pop_cart_ang_list() {
    // generate angular momentum list, can only be called once. 
    int k,xx,yy,x,y,z;
    for (  k = 0 ; k <= LIBINT2_MAX_AM ; k++ ){
      cart_ang_list.emplace_back();  //loop over possible angular momentum
      for (  xx=0 ; xx<k+1 ; xx++ ){ 
        x = k -xx ; 
        for ( yy = 0 ; yy<xx+1 ; yy++ ){ 
          y = xx-yy; 
          z = k - x - y;
          cart_ang_list[k].push_back({x,y,z});
        }
      }
    }
  };


// sperical to cartesian transform matrix elements

//math functions, like factorial, double factorial and bionomials
//--------------------------------------//
// factorial function:  t! = 1*2*3...*t //
//--------------------------------------//
double factorial(int t){
  int i;
  double tmp = 1.0;
  if (t<0 ) std::cout<<"Factorial (t!) only defined on domain t in [0,inf)"<<std::endl;
  if (t==0) return 1.0;
  else {
    for(i=1;i<=t;i++) tmp *= i;
    return tmp;
  }
} //factorial

//-----------------------------------------------//
// double factorial:  (2t-1)!! = 1*3*5*..*(2t-1) //
//-----------------------------------------------//
double doubleFact(int t){
  int i;
  double tmp = 1.0;
  if (t<0 ) std::cout<<"Double factorial (t!!) only defined on domain t in [0,inf)" 
                     <<std::endl;
  if (t==0) return 1.0;
  else  {
    for(i=1;i<=t;i++) tmp *= (2*i-1);
    return tmp;
  }
} // doubleFact

//---------------------------------------------------------//
// polynomial coeff:   (x+a)^l= sum(coeff * x^i * a^(l-i)) //
//---------------------------------------------------------//
double polyCoeff(int l, int i){
  if (l>= i) return factorial(l)/( factorial(i)*factorial(l-i) );
  else std::cout<<"polyCoeff error"<<std::endl;
}


std::complex <double> cart2sphCoeff(int L, int m, int lx, int ly, int lz){

//calculate the cartesian to spherical transformation coefficient.

  int Ltotal;
  dcomplex coeff(0.0);
  Ltotal = lx+ly+lz;
  double tmp = 0.0;
  if (L!=Ltotal) {
    return  coeff;
  }
  double j;
  j = (double(lx+ly)-std::abs(double(m)))/2;
  if (fmod(j,1)>0) {
    return coeff;
  }
  dcomplex sumval(0.0);
  dcomplex ttmmpp,sumsumval;
  dcomplex pref,absmchooselxm2k,ichoosej;
  int i,k;
  if (Ltotal == L) {
  pref = sqrt(factorial(lx*2)*factorial(2*ly)
          *factorial(2*lz)*factorial(L)
          *factorial(L-std::abs(m))
     /(factorial(2*L)*factorial(lx)*factorial(ly)
      *factorial(lz)*factorial(L+std::abs(m))))
     /(factorial(L)*pow(2,L));
  
  i = 0;
  
  while (i<=double((L-std::abs(m))/2) ) {
    sumsumval = 0.0;
    for ( k = 0 ; k <= j ; k++ ) {
      if (m>=0) {
        ttmmpp = double(std::abs(m)-lx+2*k)/2;
      }
      else {
        ttmmpp = -double(std::abs(m)-lx+2*k)/2;
      }
      
      if ((std::abs(m)>=(lx-2*k))&&((lx-2*k)>=0)) {
        absmchooselxm2k =polyCoeff(std::abs(m),lx-2*k);
      }
      else {
        absmchooselxm2k = 0.0;
      }
      sumsumval = sumsumval + polyCoeff(j,k)
                  *absmchooselxm2k*pow(-1.0,ttmmpp);
    }
    if (i<j||(j<0)) {
       ichoosej = 0.0;
    }
    else {
      ichoosej = polyCoeff(i,j);
    }
    sumval = sumval + polyCoeff(L,i)*ichoosej*pow(-1,i)
                *factorial(2*L-2*i)/
                (factorial(L-std::abs(m)-2*i))*sumsumval;
    i = i + 1;
  }
  coeff = pref * sumval;
  return coeff;
  }
} // cart2sphCoeff   

std::vector<std::vector<double>> car2sph_matrix;
void pop_car2sph_matrix() {
  int l[3];
  int m;
  int carsize; //cartesian size
  double scalecoeff;

  for ( int L = 0 ; L <= LIBINT2_MAX_AM ; L++ ) {
    carsize = (L+1)*(L+2)/2;
    car2sph_matrix.emplace_back((2*L+1)*carsize, 0.0);
    if ( L==0 ){
      car2sph_matrix[L][0]=1.0;
    } 
    else if (L==1) {
      car2sph_matrix[L][0*3+0] = 1.0;
      car2sph_matrix[L][1*3+1] = 1.0;
      car2sph_matrix[L][2*3+2] = 1.0;
    }
    else {
      for ( int p=0 ; p < L+1 ; p++ )
      for ( int q=0 ; q < carsize ; q++ ){
        for ( int k=0 ; k<3 ; k++ ) l[k]=cart_ang_list[L][q][k];
        m = -L+p;    
        if ( m<0 ) {
          auto cplxcoeff = cart2sphCoeff(L,m,l[0],l[1],l[2]);   // complex coefficient
          
          car2sph_matrix[L][p*carsize+q] = sqrt(2.0)*(-cplxcoeff.imag());

          car2sph_matrix[L][(2*L-p)*carsize+q] = sqrt(2.0)*cplxcoeff.real();

          double scalecoeff = sqrt(doubleFact(L)/
          (doubleFact(l[0])*doubleFact(l[1])*doubleFact(l[2])));
        car2sph_matrix[L][p*carsize+q] = car2sph_matrix[L][p*carsize+q] *scalecoeff;
        car2sph_matrix[L][(2*L-p)*carsize+q] = car2sph_matrix[L][(2*L-p)
                                               *carsize+q]*scalecoeff; 
      } else if ( m ==0 ) {
        car2sph_matrix[L][p*carsize+q] = cart2sphCoeff(L,m,l[0],l[1],l[2]).real();
        scalecoeff = sqrt(doubleFact(L)/
          (doubleFact(l[0])*doubleFact(l[1])*doubleFact(l[2])));
        car2sph_matrix[L][p*carsize+q] = car2sph_matrix[L][p*carsize+q]*scalecoeff;   
      } 
    }
  }

 } //loop over angular momentum

} //pop_car2sph_matrix()

void cart2sph_transform( int l_i, int l_j, std::vector<double> &shell_element_sph, std::vector<double> &shell_element_cart) {

  int cart_i = (l_i+1)*(l_i+2)/2;
  int cart_j = (l_j+1)*(l_j+2)/2;
  int cartsize = cart_i*cart_j;
  int sphsize  = (2*l_i+1)*(2*l_j+1);
  double tempVal;

  if (sphsize != shell_element_sph.size() )
    std::cout<<"spherical dimension doesn't match"<<std::endl;
  if (cartsize != shell_element_cart.size() )
    std::cout<<"cartesian dimension doesn't match"<<std::endl;
 
  for( int i = 0 ; i<2*l_i+1 ; i++  ) {
    for( int j = 0 ; j<2*l_j+1 ; j++ ) {
      tempVal = 0.0;
      for( int p = 0 ; p<cart_i ; p++ ) {
        for( int q = 0 ; q<cart_j ; q++ ) {
          tempVal += car2sph_matrix[l_i][i*cart_i+p]*car2sph_matrix[l_j][j*cart_j+q]
                     *shell_element_cart[p*cart_j+q];
        }
      }

    if (std::abs(tempVal)<1.0e-15) tempVal=0.0;
    shell_element_sph[i*(2*l_j+1)+j] = tempVal;
    }
  }   
} //cart2sph_transform   


}; // namespace ChronusQ

