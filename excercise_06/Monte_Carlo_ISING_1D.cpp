/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/

#include <iostream>
#include <fstream>
#include <ostream>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include "Monte_Carlo_ISING_1D.h"

using namespace std;

int main()
{ 
  Input(); 						// Inizialization
  for(int iblk=1; iblk <= nblk; ++iblk) 		// Simulation
  {
    Reset(iblk);   					// Reset block averages
    for(int istep=1; istep <= nstep; ++istep)
    {
      Move(metro);
      Measure();
      Accumulate(); 					// Update block averages
    }
    Averages(iblk);   					// Print results for current block
  }
  ConfFinal(); 						// Write final configuration

  return 0;
}


void Input(void)
{
  ifstream ReadInput;

  cout << "Classic 1D Ising model             " << endl;
  cout << "Monte Carlo simulation             " << endl << endl;
  cout << "Nearest neighbour interaction      " << endl << endl;
  cout << "Boltzmann weight exp(- beta * H ), beta = 1/T " << endl << endl;
  cout << "The program uses k_B=1 and mu_B=1 units " << endl;

// Read seed for random numbers
   int p1, p2;
   ifstream Primes("Primes");
   Primes >> p1 >> p2 ;
   Primes.close();

   ifstream input("seed.in");
   input >> seed[0] >> seed[1] >> seed[2] >> seed[3];
   rnd.SetRandom(seed,p1,p2);
   input.close();
  
// Read input informations
  ReadInput.open("input.dat");

  ReadInput >> temp;
  beta = 1.0/temp;
  cout << "Temperature = " << temp << endl;

  ReadInput >> nspin;
  cout << "Number of spins = " << nspin << endl;

  ReadInput >> J;
  cout << "Exchange interaction = " << J << endl;

  ReadInput >> h;
  cout << "External field = " << h << endl << endl;
    
  ReadInput >> metro;	// if=1 Metropolis else Gibbs !

  ReadInput >> nblk;

  ReadInput >> nstep;

  ReadInput >> should_old;

  ReadInput >> should_equi;    

  if(metro==1) cout << "The program perform Metropolis moves" << endl;
  else cout << "The program perform Gibbs moves" << endl;
  cout << "Number of blocks = " << nblk << endl;
  cout << "Number of steps in one block = " << nstep << endl << endl;
  if(should_old==1) cout << "The simulation will start from an old configuration (file config.0)" << endl;
  else cout << "The simulation will start from a random configuration " << endl << endl;
  if(should_equi==1) cout << "This is an attempt to equilibrate the ensemble" << endl;

  ReadInput.close();

// Prepare arrays for measurements
  iu = 0;	// Energy
  iu2 = 1;	// Squared energy: we use the energy-dissipation theorem
  ic = 2;	// Heat capacity
  im = 3;	// Magnetization
  ix = 4;	// Magnetic susceptibility
 
  n_props = 5; // Number of observables

  if(should_old==1){			// should_old == 1: initial configuration from file config.0
    ifstream input;
    input.open("config.0");
    if(input.fail()){
      cerr << "Unable to open file 'config.0' !" << endl;
      exit(-1);
    }
    for(int i=0; i<nspin; i++) input >> s[i];
  }
  else{					// shoul_old == 0: initial random configuration
    for (int i=0; i<nspin; ++i){
      if(rnd.Rannyu() >= 0.5) s[i] = 1;
      else s[i] = -1;
    }
  }
  
// Evaluate energy etc. of the initial configuration
  Measure();

// Print initial values for the potential energy and virial
  cout << "Initial energy = " << walker[iu]/(double)nspin << endl;
}


void Move(int metro)
{
  int o;
  double p, energy_old, energy_new, sm;
  double energy_up, energy_down;

  for(int i=0; i<nspin; ++i)
  {
  // Select randomly a particle (for C++ syntax, 0 <= o <= nspin-1)
    o = (int)(rnd.Rannyu()*nspin);

    if(metro==1)	// Metropolis algorithm
    {
      sm = (-1)*s[o];
      energy_old = Boltzmann(s[o],o);
      energy_new = Boltzmann(sm,o);
      if((energy_new - energy_old) < 0){
        s[o] = sm;
        accepted ++;
      }
      else{
        p = exp(-beta*(energy_new - energy_old));
        if(p > rnd.Rannyu()){
          s[o] = sm;
          accepted ++;
        }
      }
    }

    else		// Gibbs sampling
    {
      energy_up = Boltzmann(1,o);
      energy_down = Boltzmann(-1,o);
      p = 1/(double)(1 + exp(-beta*(energy_down - energy_up)));
      if(p > rnd.Rannyu()) s[o] = 1;
      else s[o] = -1;
    }
  }
}


double Boltzmann(int sm, int ip)
{
  double ene = -J * sm * ( s[Pbc(ip-1)] + s[Pbc(ip+1)] ) - h * sm;
  return ene;
}


void Measure()
{
  int bin;
  double u = 0.0, m = 0.0;

// cycle over spins
  for (int i=0; i<nspin; ++i)
  {
     u += -J * s[i] * s[Pbc(i+1)] - 0.5 * h * (s[i] + s[Pbc(i+1)]);
     m += s[i];
  }
  walker[iu] = u;
  walker[iu2] = u*u;
  walker[im] = m;
  walker[ix] = m*m;
}


void Reset(int iblk)	// Reset block averages
{
   
   if(iblk == 1)
   {
       for(int i=0; i<n_props; ++i)
       {
           glob_av[i] = 0;
           glob_av2[i] = 0;
       }
   }

   for(int i=0; i<n_props; ++i)
   {
     blk_av[i] = 0;
   }
   blk_norm = 0;
   attempted = nstep;
   accepted = 0;
}


void Accumulate(void)	// Update block averages
{

   for(int i=0; i<n_props; ++i)
   {
     blk_av[i] = blk_av[i] + walker[i];
   }
   blk_norm = blk_norm + 1.0;
}


void Averages(int iblk)		//Print results for current block
{
    
   ofstream Ene, Heat, Mag, Chi;
   const int wd=12;
    
    cout << "Block number " << iblk << endl;
    if (metro==1) cout << "Acceptance rate " << accepted/attempted << endl << endl;

    if(should_equi==1){
      if(metro==1) Ene.open("equi.ene.metro",ios::app);
      else Ene.open("equi.ene.gibbs",ios::app);
    }
    else{
      if(metro==1) Ene.open("output.ene.metro",ios::app);
      else Ene.open("output.ene.gibbs",ios::app);
    }

    stima_u = blk_av[iu]/blk_norm/(double)nspin;	// Energy per particle
    glob_av[iu]  += stima_u;
    glob_av2[iu] += stima_u*stima_u;
    err_u = Error(glob_av[iu],glob_av2[iu],iblk);
    Ene << setw(wd) << iblk <<  setw(wd) << stima_u << setw(wd) << glob_av[iu]/(double)iblk << setw(wd) << err_u << endl;

    Ene.close();

    if(should_equi==1){
      if(metro==1) Heat.open("equi.heat.metro",ios::app);
      else Heat.open("equi.heat.gibbs",ios::app);
    }
    else{
      if(metro==1) Heat.open("output.heat.metro",ios::app);
      else Heat.open("output.heat.gibbs",ios::app);
    }

    stima_c = beta*beta*( ((blk_av[iu2]/blk_norm) - pow( blk_av[iu]/blk_norm , 2))/(double)nspin );	// Heat capacity per particle: proportional to the squared energy oscillations
    glob_av[ic]  += stima_c;
    glob_av2[ic] += stima_c*stima_c;
    err_c = Error(glob_av[ic],glob_av2[ic],iblk);
    Heat << setw(wd) << iblk <<  setw(wd) << stima_c << setw(wd) << glob_av[ic]/(double)iblk << setw(wd) << err_c << endl;

    Heat.close();

    if(should_equi==1){
      if(metro==1) Mag.open("equi.mag.metro",ios::app);
      else Mag.open("equi.mag.gibbs",ios::app);
    }
    else{
      if(metro==1) Mag.open("output.mag.metro",ios::app);
      else Mag.open("output.mag.gibbs",ios::app);
    }

    stima_m = blk_av[im]/blk_norm/(double)nspin;	// Magnetization per particle
    glob_av[im]  += stima_m;
    glob_av2[im] += stima_m*stima_m;
    err_m = Error(glob_av[im],glob_av2[im],iblk);
    Mag << setw(wd) << iblk <<  setw(wd) << stima_m << setw(wd) << glob_av[im]/(double)iblk << setw(wd) << err_m << endl;

    Mag.close();

    if(should_equi==1){
      if(metro==1) Chi.open("equi.chi.metro",ios::app);
      else Chi.open("equi.chi.gibbs",ios::app);
    }
    else{
      if(metro==1) Chi.open("output.chi.metro",ios::app);
      else Chi.open("output.chi.gibbs",ios::app);
    }

    stima_x = beta*(blk_av[ix]/blk_norm/(double)nspin);		// Susceptibility per particle
    glob_av[ix]  += stima_x;
    glob_av2[ix] += stima_x*stima_x;
    err_x = Error(glob_av[ix],glob_av2[ix],iblk);
    Chi << setw(wd) << iblk <<  setw(wd) << stima_x << setw(wd) << glob_av[ix]/(double)iblk << setw(wd) << err_x << endl;

    Chi.close();

    cout << "----------------------------" << endl << endl;
}


void ConfFinal(void)
{
  ofstream WriteConf;
  WriteConf.open("config.final");
  cout << "Print final configuration to file config.final " << endl << endl;

  for (int i=0; i<nspin; ++i)
  {
    WriteConf << s[i] << endl;
  }
  WriteConf.close();

  rnd.SaveSeed();
}

int Pbc(int i)  //Algorithm for periodic boundary conditions
{
    if(i >= nspin) i = i - nspin;
    else if(i < 0) i = i + nspin;
    return i;
}

double Error(double sum, double sum2, int iblk)
{
    return sqrt((sum2/(double)iblk - pow(sum/(double)iblk,2))/(double)iblk);
}

/****************************************************************
*****************************************************************
    _/    _/  _/_/_/  _/       Numerical Simulation Laboratory
   _/_/  _/ _/       _/       Physics Department
  _/  _/_/    _/    _/       Universita' degli Studi di Milano
 _/    _/       _/ _/       Prof. D.E. Galli
_/    _/  _/_/_/  _/_/_/_/ email: Davide.Galli@unimi.it
*****************************************************************
*****************************************************************/