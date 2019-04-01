#include <stdio.h>
#include <stdint.h>
#include <inttypes.h> //ovaj zapravo uključuje prethodni
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>


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



int main () {


    unsigned long long MS[5] = { ( unsigned long long ) 0};

    srand(time(NULL));

    clock_t zadnji_zahtjev = clock(), predzadnji_zahtjev = clock(), start = clock();

    int I = 0, U = -1;

    while(1)
    {

        unsigned long long iteracija = MAX_ITERACIJA;

        predzadnji_zahtjev = zadnji_zahtjev;

        unsigned long long x = rnd() | 1ULL;



        //unaprijed oznacen broj iteracija u kojima provjeravamo je li broj prost
        // inace generiraj novi
        while( !ispunjava_zahtjev ( x ) || !prost ( x ) )
        {

            zadnji_zahtjev = clock();

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
        if ( zadnji_zahtjev - predzadnji_zahtjev >= 1 )
        {
            double p = rand_0_to_1();
            //printf("Generirana vjerojatnost : %f\n", p);
            if ( p >= 0.5 )
            {
                // ispisi vrijeme koje je proteklo od pokretanja programa
                double sekunde = ( ( double ) ( -start + zadnji_zahtjev) ) / CLOCKS_PER_SEC;
                printf ( "Vrijeme: %.3f s - ", sekunde ) ;
                printf ( "MS [] = { " );
                for( int i = 0; i < 5; ++i )
                {
                    int ms = MS[i] % 100;
                    if ( ms < 10 )
                        printf ( "0" );
                    printf ( "%d ",  ms);

                }
                printf ( "} " );

                printf ( "I = %d, U = %d   ",  I % 5, (U + 1) % 5 );
                printf ( "MS[I]=%llu\n", MS[I % 5] );
                //bit(MS[I % 5]);
                I ++;

            }
            // pretpostavimo uniformnu distribuciju na intervalu [0, 100]; vjerojatnost
            // da je broj u intervalu od [0, a] je 1/ (a - 0);
            if ( p <= 0.1 )
            {
                break;
            }
        }


    }

    return 0;
}
