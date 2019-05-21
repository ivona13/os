#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <unistd.h>

#include <assert.h>

#include <stdbool.h>

#include <semaphore.h>

#include <math.h>


pthread_mutex_t monitor;

pthread_cond_t red[5];

long last = 0;

// globalne varijable
unsigned long long MS[5] = {
  (unsigned long long) 0
};
int I = 0, U = -1;
int KRAJ = 0;
int BROJ_DRETVI = 5;

#define MAX_ITERACIJA 10000000

//generiranje broja između 0 i 1
double rand_0_to_1() {

  return (double) rand() / (RAND_MAX + 1.0);
}

//drugi random generator
unsigned long long rnd() {
  unsigned long long A = 8531648002905263869ULL, B = 18116652324302926351ULL;
  return (rand() * A) % B;
}

void bit(unsigned long long n) {
  for (int i = 0; i <= 63; i++)
    printf("%llu", (n & (1ULL << i)) >> i);

}

// zahtjev da se najvise dvije iste znamenke mogu pojaviti u nizu
bool ispunjava_zahtjev(unsigned long long n) {
  for (int i = 0; i <= 61; i++) {
    int prvi_bit = (n & (1ULL << i)) >> i;
    int drugi_bit = (n & (1ULL << (i + 1))) >> (i + 1);
    int treci_bit = (n & (1ULL << (i + 2))) >> (i + 2);

    if (prvi_bit == drugi_bit)
      if (drugi_bit == treci_bit)
        return false;

  }
  return true;
}

bool prost(unsigned long long broj) {
  for (int i = 2; i <= 30000 && i <= broj; ++i) {
    if (!(broj % i))
      return false;
  }
  return true;
}

// struktura za pohranu dretve
typedef struct _thread_data_t {
  int id;
  int sleepsecond;
}
thread_data_t;

void * provjera_func(void * args) {

  while (KRAJ != 1) {
    thread_data_t * data = (thread_data_t * ) args;

    int id = data -> id;
    int index;
    unsigned long long broj;

    pthread_mutex_lock( & monitor);

    while (MS[I % 5] == 0 || last == MS[I % 5])
      pthread_cond_wait( & red[id], & monitor);

    broj = MS[I % 5];
    last = broj;
    printf("Dretva %d dohvatila broj %llu.\n", id, broj);

    I++;

    printf("Dretva %d potrošila broj %llu.\n", id, broj);

    pthread_mutex_unlock( & monitor);

  }
}

// thread funkcija
void * thr_func(void * arg) {

  thread_data_t * data = (thread_data_t * ) arg;

  int id = data -> id;
  srand(time(NULL));
  if (KRAJ) {
    pthread_exit(NULL);
  }

  while (KRAJ != 1) {

    unsigned long long x = rnd() | 1ULL;

    int iteracija = MAX_ITERACIJA;

    //unaprijed oznacen broj iteracija u kojima provjeravamo je li broj prost
    // inace generiraj novi
    while (!ispunjava_zahtjev(x) || !prost(x)) {

      if (x <= 0xffffffffffffffffULL - 2) {
        x += 2;
        --iteracija;
        if (!iteracija) {
          iteracija = MAX_ITERACIJA;
          x = rnd() | 1ULL;
          break;
        }

      }

      x = rnd() | 1ULL;
      iteracija = MAX_ITERACIJA;
      continue;
    }

    pthread_mutex_lock( & monitor);
    MS[U % 5] = x;
    U++;
    pthread_cond_signal( & red[id]);
    pthread_mutex_unlock( & monitor);
  }

  pthread_exit(NULL);
}

// funkcija za glavnu dretvu
void * main_thr_func(void * arg) {

  while (!KRAJ) {
    thread_data_t * data = (thread_data_t * ) arg;

    int BROJ_DRETVI = data -> id;
    int second = data -> sleepsecond;
    int rc = 0;
    int i = 0;

    pthread_t radna_dretva[BROJ_DRETVI];
    thread_data_t radna_dretva_data[BROJ_DRETVI];
    pthread_t provjera_dretva[BROJ_DRETVI];
    thread_data_t provjera_data[BROJ_DRETVI];

    // radne dretve

    for (i = 0; i < BROJ_DRETVI; ++i) {
      radna_dretva_data[i].id = i;
      if ((rc = pthread_create( & radna_dretva[i], NULL, thr_func, & radna_dretva_data[i]))) {
        fprintf(stderr, "error: Ne mogu stvoriti dretvu.");
        break;
      }
    }

    rc = 0;

    // dretve provjere
    for (i = 0; i < BROJ_DRETVI; ++i) {
      provjera_data[i].id = i;
      if ((rc = pthread_create( & provjera_dretva[i], NULL, provjera_func, & provjera_data[i]))) {
        fprintf(stderr, "error: Ne mogu stvoriti dretvu.");
        break;
      }
    }

    sleep(30);

    KRAJ = 1;
    for (i = 0; i < BROJ_DRETVI; ++i) {
      if (KRAJ == 1)
        break;
      pthread_join(provjera_dretva[i], NULL);

    }

    for (i = 0; i < BROJ_DRETVI; ++i) {
      pthread_join(radna_dretva[i], NULL);
      if (KRAJ == 1)
        break;
    }

  }

  pthread_exit(NULL);
}

int main() {

  int sekunde = 2;
  int rc = 0;

  pthread_t main_thr;

  thread_data_t main_data;

  pthread_mutex_init( & monitor, NULL);

  for (int i = 0; i < BROJ_DRETVI; i++) {
    pthread_cond_init( & red[i], NULL);
  }

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
