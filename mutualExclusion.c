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
#define NUM_CONTAS 5
// utility function to identify even-odd numbers
unsigned odd(unsigned long num) { return num % 2; }

// simulate id performing 1000 transactions 
void do1000Transactions(unsigned long id) {

    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(id + 1);

    for (unsigned i = 0; i < 1000; i++) {

        // Sorteia a operação (0 a 3)
         int operacao = rand_r(&seed) % 4;

        // Sorteia a conta (0 a NUM_CONTAS - 1)
         int numero = rand_r(&seed) % NUM_CONTAS;

        switch (operacao) {

            case 0:
               deposit(100.00, numero);
               break;

            case 1:
               withdraw(100.00, numero);
               break;

            case 2:
               saldo(numero);
               break;

            case 3: {
                  int destino;
                  do {
                     destino = rand_r(&seed) % NUM_CONTAS;
                  } while (destino == numero);

                  transferencia(numero, destino, 100.00);
                  break;
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

void saldo(int conta){
   double resultado;
   pthread_mutex_lock(&lock);
   resultado = bankAccountBalance[conta];
   pthread_mutex_unlock(&lock);
   printf("O saldo da conta é: %f", resultado);
}

void transferencia(int origem, int destino, double valor){
      if(origem==destino){
         printf("Não dá pra transferir para o mesmo destino.");
         return;
      } 
      if (valor<=0){
         printf("Insira um valor válido.");
         return;
      }
      if(bankAccountBalance[origem]<valor){
         printf("A conta de origem não possui saldo.");
         return;
      } 
      

    if (bankAccountBalance[origem] >= valor) {
         pthread_mutex_lock(&lock);
         bankAccountBalance[origem] -= valor;
         bankAccountBalance[destino] += valor;
         pthread_mutex_unlock(&lock);
         printf("Transferencia realizada!\n");
    }
}

void deposit(double amount, int numero) {
  if(amount <=0){
    return;
  }
  pthread_mutex_lock( &lock );
  bankAccountBalance[numero] += amount;
  pthread_mutex_unlock( &lock );
}

// subtract amount from bankAccountBalance
void withdraw(double amount,int numero) {

    pthread_mutex_lock(&lock);

    if (amount > 0 && bankAccountBalance[numero] >= amount) {
        bankAccountBalance[numero] -= amount;
        pthread_mutex_unlock(&lock);
        printf("Saque realizado!\n");
    } else {
        pthread_mutex_unlock(&lock);
        printf("Saldo insuficiente ou valor invalido!\n");
    }
}

void cleanup() {
   pthread_mutex_destroy(&lock);
}

int main(int argc, char** argv) {
  pthread_t * children;                   // dynamic array of child threads
  unsigned long id = 0;                   // loop control variable
  unsigned long numThreads = 0;           // desired # of threads
                                          // get desired # of threads
  numThreads = processCommandLine(argc, argv);
                                          // allocate array of handles
  children = malloc( numThreads * sizeof(pthread_t) );
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
  for(int i =1;i<=NUM_CONTAS;i++){
   printf("\nThe final balance of the %d account using %lu threads is $%.2f.\n\n",
             i,numThreads, bankAccountBalance[i]);
  }
  
  free(children);                         // deallocate array
  cleanup();                              // deallocate mutex
  return 0;
}

