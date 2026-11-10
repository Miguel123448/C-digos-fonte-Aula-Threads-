/* mutualExclusion.c demonstrates the Mutual Exclusion pattern using pthreads
 *  to deposit and withdraw $$ from a bank account.
 *  When an even number of threads are used, the final balance should be $0.00,
 *   but this does not happen, due to a race condition.
 *
 * Joel Adams, Calvin College, Fall 2013.
 *
 * Usage: ./mutualExclusion [evenNumberOfThreads]
 * 
 * Exercise:
 *   1. Build and run several times, for each of 2, 4, 8, 16, ... threads.
 *   2. In bankAccount.h:
 *      - Surround each nonlocal access to bankAccountBalance with:
 *             pthread_mutex_lock( &lock );   // before the access
 *             pthread_mutex_unlock( &lock ); // after the access
 *   3. Save, rebuild, rerun, and note the difference in results.
 */

#include <stdio.h>        // printf()
#include <stdlib.h>       // atoi(), exit(), ...
#include <pthread.h>      // pthread types and functions
#include <time.h>

#include "bankAccount.h"  // bank account info

// utility function to identify even-odd numbers
unsigned odd(unsigned long num) { return num % 2; }

// simulate id performing 1000 transactions 
void do1000Transactions(unsigned long id) {
   unsigned int seed = (unsigned int) time(NULL) ^ (unsigned int) id;
   for (unsigned i = 0; i < 1000; i++) {
      int    op      = rand_r(&seed) % 4;
      int    account = rand_r(&seed) % NUM_ACCOUNTS;
      double amount  = (rand_r(&seed) % 500) + 1.0;
      switch (op) {
         case 0:
            deposit(account, amount);
            break;
         case 1:
            withdraw(account, amount);
            break;
         case 2:
            getBalance(account);
            break;
         default: {
            int to = rand_r(&seed) % NUM_ACCOUNTS;
            while (to == account) {
               to = rand_r(&seed) % NUM_ACCOUNTS;
            }
            transfer(account, to, amount);
         }
      }
   }
}

void* child(void * buf) { 
   unsigned long childID = (unsigned long) buf;
   do1000Transactions(childID);  
   return NULL;
}

unsigned long processCommandLine(int argc, char** argv) {
   if (argc == 2) {
      return strtoul(argv[1], 0, 10);
   } else if (argc == 1) {
      return 1;
   } else {
      fprintf(stderr, "\nUsage: ./mutualExclusion [evenNumberOfThreads]\n");
      exit(1);
   }
}

int main(int argc, char** argv) {
  pthread_t * children;                   // dynamic array of child threads
  unsigned long id = 0;                   // loop control variable
  unsigned long numThreads = 0;           // desired # of threads
                                          // get desired # of threads
  numThreads = processCommandLine(argc, argv);
                                          // allocate array of handles
  children = malloc( numThreads * sizeof(pthread_t) );
  initAccounts();
                                          // FORK:
  for (id = 1; id < numThreads; id++) {
     pthread_create( &(children[id-1]),   // our handle for the child 
                      NULL,               // attributes of the child
                      child,              // the function it should run
                      (void*) id );       // args to that function
  }
  
  do1000Transactions(0);

  for (id = 1; id < numThreads; id++) {   // JOIN:
     pthread_join( children[id-1], NULL );
  }

  double totalBalance = 0, totalDep = 0, totalWit = 0;
  printf("\n=== SALDOS FINAIS (%lu threads) ===\n", numThreads);
  for (int i = 0; i < NUM_ACCOUNTS; i++) {
     printf("Conta %d: R$ %.2f\n", i, bankAccountBalance[i]);
     totalBalance += bankAccountBalance[i];
     totalDep     += totalDeposited[i];
     totalWit     += totalWithdrawn[i];
  }

  double expected = NUM_ACCOUNTS * INITIAL_BALANCE + totalDep - totalWit;
  double diff = totalBalance - expected;
  if (diff < 0) diff = -diff;

  printf("\nSoma dos saldos finais: R$ %.2f\n", totalBalance);
  printf("Esperado (inicial + depositos - saques): %.2f + %.2f - %.2f = R$ %.2f\n",
         NUM_ACCOUNTS * INITIAL_BALANCE, totalDep, totalWit, expected);
  printf("Conferencia: %s\n\n", diff < 0.005 ? "OK - nenhum valor foi perdido"
                                             : "FALHOU - houve condicao de corrida");
  
  free(children);                         // deallocate array
  cleanup();                              // deallocate mutex
  return 0;
}

