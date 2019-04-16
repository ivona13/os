#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <unistd.h>

#include <assert.h>

#include <stdbool.h>

#define MEMBAR __sync_synchronize()


// Lamportov algoritam, polje ulaz za spremanje ULAZ za radnu dretvu, a ulaz_ za dretvu provjere
volatile bool *ulaz, *ulaz_;
// Lamportov algoritam, polje broj za spremanje BROJ za radnu dretvu, a broj_ za  dretvu provjere
volatile int * broj, *broj_;

// globalne varijable
unsigned long long MS[5] = { ( unsigned long long ) 0};
int I = 0, U = -1;
int KRAJ = 0;
int BROJ_DRETVI = 5;


#define MAX_ITERACIJA 10000000;

//generiranje broja između 0 i 1
double rand_0_to_1()
{

      return (double)rand() / (RAND_MAX + 1.0);
}

//drugi random generator
unsigned long long rnd ()
{
    unsigned long long A = 8531648002905263869ULL, B = 18116652324302926351ULL;
    return ( rand() * A ) % B;
}

void bit(unsigned long long n)
{
     for( int i = 0; i <= 63; i++ )
        printf ( "%llu", ( n & ( 1ULL << i ) ) >> i );

}

// zahtjev da se najvise dvije iste znamenke mogu pojaviti u nizu
bool ispunjava_zahtjev ( unsigned long long n )
{
    for( int i = 0; i <= 61; i++)
    {
        int prvi_bit = ( n & ( 1ULL << i ) ) >> i;
        int drugi_bit = ( n & ( 1ULL << ( i + 1 ) ) ) >>  ( i + 1 );
        int treci_bit = ( n & ( 1ULL << ( i + 2 ) ) ) >>  ( i + 2 );

        if( prvi_bit == drugi_bit )
            if( drugi_bit == treci_bit )
                return false;

    }
    return true;
}



bool prost ( unsigned long long broj )
{
    for ( int i = 2; i <= 30000 && i <= broj; ++i )
    {
        if( !( broj % i ) )
            return false;
    }
    return true;
}

// pomocna funkcija za Lamportov algoritam, odredivanje broja koji ce se dodijeliti dretvi
int max(void) {
    int max_value = 0;
    int i = 0;
    for (i = 0; i < BROJ_DRETVI; i++) {
        if (broj[i] > max_value)
            max_value = broj[i];
    }
    return max_value;
}

// Lamportov algoritam, UDI_U_KO
void zakljucaj(int i) {
    ulaz[i] = true;

    broj[i] = 1 + max();

    ulaz[i] = false;

    int j = 0;
    for (j = 0; j < BROJ_DRETVI; j++) {
        // cekaj dok svaka dretva dobije broj
        while (ulaz[j]) {
        /* ne radi nista*/ }
        // Ceakj dok sve dretve s manjim brojem ili s istim brojem ali s vecim prioritetom dovrse posao
        while (broj[j] != 0 && (broj[j] < broj[i] || (broj[j] == broj[i] && j < i))) {
        /* nista*/ }
    }
}

// Lamportov algoritam, IZADI_IZ_KO
void otkljucaj(int i) {

    broj[i] = 0;
}

// struktura za pohranu dretve
typedef struct _thread_data_t {
    int id;
    int sleepsecond;
}
thread_data_t;


void *provjera_func(void *args) {


                thread_data_t * data = (thread_data_t * ) args;

                int id = data->id;
                int index;
                zakljucaj(id);


                      //  printf ( "\nI = %d, U = %d   ",  I % 5, (U + 1) % 5 );
                        printf ( "Dretva %d dohvatila broj %llu.\n", id, MS[I % 5] );
                
                        index = I;
                        I ++;

                    //    printf("\nPotrošio broj.");
                        otkljucaj(id);


                        sleep(5);
                        printf("Dretva %d potrošila broj %llu.\n", id, MS[index]);
              
    }

// thread funkcija
void * thr_func(void * arg) {

    thread_data_t * data = (thread_data_t * ) arg;

    int id = data -> id;
    srand(time(NULL));
    zakljucaj(id);

    if(KRAJ) {
      pthread_exit(NULL);
    }


        unsigned long long x = rnd() | 1ULL;

        int iteracija = MAX_ITERACIJA;

        //unaprijed oznacen broj iteracija u kojima provjeravamo je li broj prost
        // inace generiraj novi
        while( !ispunjava_zahtjev ( x ) || !prost ( x ) )
        {



            if ( x <= 0xffffffffffffffffULL - 2 )
            {
                x += 2;
                --iteracija;
                if ( !iteracija )
                {
                    iteracija = MAX_ITERACIJA;
                    x = rnd() | 1ULL;
                    break;

                }


            }

            x = rnd() | 1ULL;
            iteracija = MAX_ITERACIJA;
            continue;
        }


        U++;

        MS[ U % 5 ] = x;

        otkljucaj(id);


    pthread_exit(NULL);
}



// funkcija za glavnu dretvu
void * main_thr_func(void * arg) {

    while(!KRAJ) {
    thread_data_t * data = (thread_data_t * ) arg;

    int BROJ_DRETVI = data -> id;
    int second = data -> sleepsecond;
    int rc = 0;
    int i = 0;

    pthread_t radna_dretva[BROJ_DRETVI];
    thread_data_t radna_dretva_data[BROJ_DRETVI];
    pthread_t provjera_dretva[BROJ_DRETVI];
    thread_data_t provjera_data[BROJ_DRETVI];

    ulaz = malloc(BROJ_DRETVI * sizeof( * ulaz));
    broj = malloc(BROJ_DRETVI * sizeof( * broj));

    // inicijalizaciaj Lamportovog algoritma
    for (i = 0; i < BROJ_DRETVI; ++i) {
        ulaz[i] = false;
        broj[i] = 0;
    }
    // radne dretve
 
    for (i = 0; i < BROJ_DRETVI; ++i) {
        radna_dretva_data[i].id = i;
        if ((rc = pthread_create( & radna_dretva[i], NULL, thr_func, & radna_dretva_data[i]))) {
            fprintf(stderr, "error: Ne mogu stvoriti dretvu."  );
            break;
        }
    }





    for (i = 0; i < BROJ_DRETVI; ++i) {
        pthread_join(radna_dretva[i], NULL);
        if(KRAJ == 1)
            break;
    }



    ulaz_ = malloc(BROJ_DRETVI * sizeof( * ulaz_));
    broj_ = malloc(BROJ_DRETVI * sizeof( * broj_));
    for (i = 0; i < BROJ_DRETVI; ++i) {
          ulaz_[i] = false;
          broj_[i] = 0;
    }
    rc = 0;
          
    // dretve provjere
    for (i = 0; i < BROJ_DRETVI; ++i) {
        provjera_data[i].id = i;
        if ((rc = pthread_create( & provjera_dretva[i], NULL, provjera_func, & provjera_data[i]))) {
          fprintf(stderr, "error: Ne mogu stvoriti dretvu." );
          break;
        }
    }

    for (i = 0; i < BROJ_DRETVI; ++i) {
        if(KRAJ == 1)
            break;
        pthread_join(provjera_dretva[i], NULL);

    }


    sleep(30);


   KRAJ = 1;


  }

    pthread_exit(NULL);
}

int main() {

    int sekunde = 2;
    int rc = 0;

    pthread_t main_thr;


    thread_data_t main_data;


    // glavna dretva
    main_data.id = BROJ_DRETVI;
    main_data.sleepsecond = sekunde;
    if ((rc = pthread_create( & main_thr, NULL, main_thr_func, & main_data))) {
        fprintf(stderr, "error: Ne mogu stvoriti dretvu. ");
        return EXIT_FAILURE;
    }

    pthread_join(main_thr, NULL);


    return EXIT_SUCCESS;
}
