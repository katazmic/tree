#include<omp.h>
#include <iostream>
#include <complex>
#include <stdio.h>

// Run it with g++ -std=gnu++11 hrchl.cpp -lm -o hrc

#include<vector>
#include<math.h>
#include "hrchl_tree.h"
#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_matrix.h>

using namespace std;


Physics::Physics(){};

void Physics::setupPhysics(double alphai, double k0i, double gi, double alphabari, double q0i, double mubgi, double musmi, double Fpi,double fracFri,double fracFr_begi, int fii,Tree tree){
    
    int n;
    int Bs = 0;
    int NUM_shls_tree, NUM_shls_goy,Dim;
    
    NUM_shls_tree = tree.NUM_shls_tree;
    NUM_shls_goy = tree.NUM_shls_goy;
    Dim = tree.Dim;
    
    k_idx_max = new int[NUM_shls_tree+NUM_shls_goy]; 
    k_idx_min = new int[NUM_shls_tree+NUM_shls_goy]; 
    k_n = new double[NUM_shls_tree+NUM_shls_goy]; 
    a_n = new double[NUM_shls_tree+NUM_shls_goy]; 
    b_n = new double[NUM_shls_tree+NUM_shls_goy]; 
    c_n = new double[NUM_shls_tree+NUM_shls_goy]; 
    
    
    alpha = alphai;
    beta = -alphai/gi;
    gamma = alphai/gi/gi;
    
    k0 = k0i;
    g = gi;
    alphabar = alphabari; 
    q0 = q0i;
    
    mubg = mubgi;
    musm = musmi;
    
    fi = fii;
    Fp = Fpi;  
    fracFr = fracFri;
    fracFr_beg = fracFr_begi;
    
    for(n=0;n<NUM_shls_tree;n++){
        k_idx_min[n] = Bs;
        k_idx_max[n] = Bs+(int) pow((double) Dim,n)-1;
        Bs =  Bs + (int) pow((double) Dim,n);
        cout<<"min: "<<k_idx_min[n]<<"  max: "<<k_idx_max[n]<<"\n";
        
    }
    for(n=NUM_shls_tree;n<(NUM_shls_goy+NUM_shls_tree);n++){
        k_idx_min[n] = Bs;
        k_idx_max[n] = Bs+(int) pow((double) Dim,NUM_shls_tree-1)-1;
        Bs =  Bs + (int) pow((double) Dim,NUM_shls_tree-1);
        cout<<"min: "<<k_idx_min[n]<<"  max: "<<k_idx_max[n]<<"\n";
    }    
    
    
    // setting the coefficients
    
    for(n=0;n<(NUM_shls_tree+NUM_shls_goy);n++){
        k_n[n] = k0*pow(g,n);
        
        a_n[n] = alpha*pow(k_n[n],4)/(1+k_n[n]*k_n[n])*(g*g-1)/pow(g,7);
        b_n[n] = beta*pow(k_n[n],4)/(1+k_n[n]*k_n[n])*(pow(g,4)-1)/pow(g,2); 
        c_n[n] = gamma*pow(k_n[n],4)/(1+k_n[n]*k_n[n])*(g*g-1)*pow(g,5);
        
        
        
        
    }
    
    
    
    
}


void Physics::inhomo_iLim(int n, double frac, double frac_beg, int *limB, int * limE){
    
    int i_beg,i_end,span;
    i_beg = k_idx_min[n];
    i_end = k_idx_max[n];
    span = i_end-i_beg+1;
    
    *limB = i_beg + (int) span*frac_beg;
    *limE = *limB + (int) span*frac -1;
    
};


void Physics::delete_physics(){
    
    
    
    delete[] k_n;
    delete[] a_n;
    delete[] b_n;
    delete[] c_n;
    delete[] k_idx_max;
    delete[] k_idx_min;
    
    
    
    
    
};


Physics::~Physics(){};

Tree::Tree(){};



void Tree::setupNodes(){

    int it,i,n,j,X,k;
    int Bs = 0;
    nds[0].has_pr = 0;
    nds[0].i_pr = -1;
    if(NUM_shls_tree!=1){ 
        nds[0].has_ch = Dim;
        nds[0].i_ch = new int[Dim];
        for(i=0;i<Dim;i++){
            nds[0].i_ch[i] = i+1;
            nds[i+1].i_pr = 0;
            nds[i+1].has_pr = 1;
        }
    }
    
    if(NUM_shls_goy!=0 && NUM_shls_tree==1){
        nds[0].has_ch = 1;
        nds[0].i_ch = new int[1]; 
        *nds[0].i_ch = 1;
        nds[1].i_pr = 0;
        nds[1].has_pr = 1;
    }
    
    
    for(n=1;n<NUM_shls_tree;n++){
        
        if(n<NUM_shls_tree-1){
            Bs = (n!=1 ?  Bs + (int) pow((double) Dim,n-1) :0);
            it = 0;
            for(j=Bs+1;j<Bs + (int) pow((double) Dim,n)+1;j++){
                nds[j].has_ch=Dim;
                
                X = (int) pow((double) Dim,n) + it*Dim + Bs;
                it = it+1;
                nds[j].i_ch = new int(Dim);
                for(i=0;i<Dim;i++){
                    nds[j].i_ch[i] = i+X+1;
                    nds[i+X+1].i_pr = j;
                    nds[i+X+1].has_pr = 1;
                    if(nds[i+X+1].has_ch!=1)
                        nds[i+X+1].has_ch=0;
                    nds[nds[j].i_ch[i]].has_ch = 0;
                }
            }
        }
        if(n==NUM_shls_tree-1){
            Bs = (n!=1 ?  Bs + (int) pow((double) Dim,n-1) :0);
            it = 0;
            for(j=Bs+1;j<Bs + (int) pow((double) Dim,n)+1;j++){
                nds[j].has_ch=(NUM_shls_goy == 0 ? 0 : 1);
            }          
        }
    }
    
    
    /////// Setting up the rest of the nodes which obey GOY //////
    j = Bs+1;
    while(j<N_nds){
        //nds[j].idx = j;
        nds[j].i_ch = new int[1];
        nds[j].has_ch = 1;
        if(j<(N_nds-(int) pow((double) Dim,NUM_shls_tree-1))){
            *nds[j].i_ch = j + (int) pow((double) Dim,NUM_shls_tree-1);
            nds[j + (int) pow((double) Dim,NUM_shls_tree-1)].i_pr = j;
            nds[j + (int) pow((double) Dim,NUM_shls_tree-1)].has_pr = 1;
        }
        else
            nds[j].has_ch=0;
        
        
        j++;
    }
    
    
};

void Tree::setupTree(int DimIn, int NUM_tree, int NUM_goy){
    
    Dim = DimIn;
    NUM_shls_tree = NUM_tree; // for the fierarchical part! 1 is teh minimum  makes it GOY shell model
    NUM_shls_goy = NUM_goy; // total 30 for reasonable results. 
    
    //com from input
    N_nds_tree =  (int) ((int) pow( (double)Dim,(NUM_shls_tree))-1)/(Dim-1);
    N_nds_goy = NUM_shls_goy*((int) pow((double)Dim,NUM_shls_tree-1));  
    N_nds = N_nds_tree + N_nds_goy;
    
    nds = new Node[N_nds];  
    setupNodes();
    
    
};


int Tree::find_n(int i){
    int n = 0;
    int Bs = 0;
    while(i>Bs-1){
        Bs = (n<NUM_shls_tree ? Bs + (int) pow((double) Dim,n) : Bs + (int) pow((double) Dim,NUM_shls_tree-1));
        if(i<N_nds_tree+N_nds_goy)
            n=n+1;
    }
    return n-1;
};


void Tree::delete_tree(){
    
    int i;
    for(i=0;i<N_nds;i++)
        delete[] nds[i].i_ch;
    delete[] nds;
    
};


Tree::~Tree(){};



void Tree::printTree(Physics phys,int gp,int p,int c,int gc){
    
    int i,j,k,l;
    int NUM = NUM_shls_tree+NUM_shls_goy;
    
    cout<<"TREE: \n";
    
    
    for(i=0;i<(NUM);i++){
        cout<<"<<";
        for(j=phys.k_idx_min[i];j<phys.k_idx_max[i]+1;j++)
            cout<<nds[j].has_ch<<" ";//cout<<nds[j].idx<<" ";
        cout<<">>\n";
    }
    
    
    if(p==1){
        
        cout<<" -- Parents -- \n ";
        for(i=0;i<(NUM);i++){
            cout<<"<<";
            for(j=phys.k_idx_min[i];j<phys.k_idx_max[i]+1;j++)
                cout<<(nds[j].has_pr==1?nds[j].i_pr:-1)<<" ";
            cout<<">>\n";
        }  
    }
    
    if(gp==1){
        cout<<" -- Grand Parents -- \n";
        for(i=0;i<(NUM);i++){
            cout<<"<<";
            for(j=phys.k_idx_min[i];j<phys.k_idx_max[i]+1;j++)
                cout<<(nds[nds[j].i_pr].has_pr==1?nds[nds[j].i_pr].i_pr:-1)<<" ";
            cout<<">>\n";
        }
    }
    
    if(c==1){
        cout<<" -- Children -- \n";
        for(i=0;i<(NUM_shls_tree-1);i++){
            cout<<"<<";
            for(j=phys.k_idx_min[i];j<phys.k_idx_max[i]+1;j++){
                cout<<"(";
                for(k=0;k<Dim-1;k++)
                    cout<<(nds[j].has_ch==Dim?nds[j].i_ch[k]:-1)<<",";
                cout<<(nds[j].has_ch==Dim?nds[j].i_ch[k]:-1)<<") ";}
            cout<<">>\n";
        }
        for(i=NUM_shls_tree-1;i<(NUM_shls_goy+NUM_shls_tree);i++){
            cout<<"<<";
            for(j=phys.k_idx_min[i];j<phys.k_idx_max[i]+1;j++)
                cout<<(nds[j].has_ch==1? nds[j].i_ch[0]:-1)<<" ";
            
            cout<<">>\n";
        }
    }
    
    if (gc==1){
        
        cout<<" -- Grand Children -- \n";
        for(i=0;i<(NUM_shls_tree);i++){
            cout<<"<<";
            
            for(j=phys.k_idx_min[i];j<phys.k_idx_max[i]+1;j++){
                cout<<"(";
                for(l=0;l<Dim;l++){
                    for(k=0;k<Dim;k++){
                        ((l==Dim-1 && k==Dim-1)?cout<<(nds[nds[j].i_ch[l]].has_ch==1?nds[nds[j].i_ch[l]].i_ch[k]:-1):cout<<(nds[nds[j].i_ch[l]].has_ch==1?nds[nds[j].i_ch[l]].i_ch[k]:-1)<<",");
                    }
                }
                cout<<") ";}
            cout<<">>\n";
        }
    }
}




int func_hmdsi(double t, const double y[], double dydt[], void *params){
    
    
    ParamForODE *param = (ParamForODE*)params;
    
    Physics *V = param->phys;
    
    Tree *T = param->tree;
    
    
    complex<double> *dphidt=(complex<double> *)&dydt[0];
    complex<double> *phi=(complex<double> *)&y[0];
    
    int i,j,k,n,N,N_nds;
    
    int fr,limB,limE;
    double D;
    D = 0.; 
    complex<double> Ca,Cb,Cc,Sm,disp,Z_fl,frcng,diff;
    
    
    
    complex<double> I (0.0,1.0);
    
    complex<double> SpZF (0.0,0.0);
    N_nds = T->N_nds;
    
#pragma omp parallel shared(phi,dphidt,V) private(i,j,n,Ca,Cb,Cc,disp,Z_fl,frcng) num_threads(1)
    {
#pragma omp for
        for(i=0;i<N_nds;i++){
            
            dphidt[i]=0;      
            n=T->find_n(i);
            
            Ca =   ( (T->nds[i].has_pr==1 && T->nds[T->nds[i].i_pr].has_pr==1) ?  V->a_n[n]*(conj(phi[T->nds[T->nds[i].i_pr].i_pr])*conj(phi[T->nds[i].i_pr])) :0 );
            
            Cb = 0;
            if(T->nds[i].has_pr==1 && T->nds[i].has_ch!=0 ){
                
                for(j = 0; j< T->nds[i].has_ch;j++){
                    Cb  = Cb + 1./T->nds[i].has_ch*V->b_n[n]*(conj(phi[T->nds[i].i_pr])*conj(phi[T->nds[i].i_ch[j]]));
                }
            }
            
            Cc = 0;
            if(T->nds[i].has_ch!=0 && T->nds[T->nds[i].i_ch[0]].has_ch!=0 ){
                for(j = 0; j< T->nds[i].has_ch;j++){
                    for(k=0;k< T->nds[T->nds[i].i_ch[j]].has_ch;k++){
                        Cc  = Cc + (1./T->nds[i].has_ch)*(1./T->nds[T->nds[i].i_ch[j]].has_ch)*V->c_n[n]*(conj(phi[T->nds[i].i_ch[j]])*conj(phi[T->nds[T->nds[i].i_ch[j]].i_ch[k]]))  ;
                    }
                }
                
            }
            
            
            // dissipations                                                                                                   
            disp  =  -(V->musm*pow(V->k_n[n],-6)+V->mubg*pow(V->k_n[n],4))*phi[i];
            
            // forcing terms                           
            // to incorporate for inhomogeneities
            fr =0;
            V->inhomo_iLim(n,V->fracFr, V->fracFr_beg, &limB, &limE);
            if(i>=limB & i<=limE)
                fr=1;
            //
            frcng = ( ((n==V->fi || n==V->fi+1) & fr==1)?  V->Fp : 0);
            if(i>1 &i<N_nds-1)
	      diff = ( (T->find_n(T->nds[i].i_pr) == T->find_n(T->nds[i-1].i_pr)  ) & (T->find_n(T->nds[i].i_pr) ==T->find_n(T->nds[i+1].i_pr) ) &(T->find_n(i)<T->NUM_shls_goy+T->NUM_shls_tree-3) ? -D*(phi[i-1] - 2.*phi[i] + phi[i+1]) : 0);
	    //	    cout<<pow(2.,(int) 2*T->find_n(i))<<"\n"; 
            dphidt[i] = Ca+Cb+Cc+disp+frcng+diff;
            
        }
        
    }
    return GSL_SUCCESS;
    
};


double get_data_NAME(FILE *someFile, const char * nameData)
{
    double vlu;
    int dataExists;
    char dat;
    int dg,i;
    char data[20]; // = malloc(20*sizeof(char));                                                             
    
    dataExists = 0;
    dat = fgetc(someFile);
    
    while(dat != EOF && dataExists == 0)
    {
        i=0;
        dat = fgetc(someFile);
        
        if(dat == nameData[i])
        {
            while(dat == nameData[i])
            {
                dat = fgetc(someFile);
                i++;
            }
            
            if(nameData[i] == '\0')
            {
                dataExists = 1;
            }
        }
    }
    
    dg =0;
    dat = fgetc(someFile);
    
    while((dat == '\t') || (dat == ' '))
    {
        dat = fgetc(someFile);
    }
    while((dat != '\t') && (dat != ' ') && (dat != EOF) && (dat != '\0')  )
    {
        data[dg] = dat;
        dat = fgetc(someFile);
        dg++;
    }
    vlu =(double) atof(data);
    
    if(dataExists ==0){
        printf("\n You goofed! Make sure the correct spelling of %s exists in the INPUT file. \n \n",nameData);
        abort();
    }
    
    return vlu;
    
    
};
