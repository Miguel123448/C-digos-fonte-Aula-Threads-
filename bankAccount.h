/* bankAccount.h simulates a simple bank account
 *
 * Joel Adams, Calvin College, Fall 2013.
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_ACCOUNTS     5
#define INITIAL_BALANCE  1000.00

// Shared Variables
pthread_mutex_t lock[NUM_ACCOUNTS];
double bankAccountBalance[NUM_ACCOUNTS];
FILE  *logFile[NUM_ACCOUNTS];
double totalDeposited[NUM_ACCOUNTS];
double totalWithdrawn[NUM_ACCOUNTS];

void initAccounts(void) {
   for (int i = 0; i < NUM_ACCOUNTS; i++) {
      char fileName[64];
      pthread_mutex_init(&lock[i], NULL);
      bankAccountBalance[i] = INITIAL_BALANCE;
      totalDeposited[i] = 0;
      totalWithdrawn[i] = 0;
      sprintf(fileName, "extrato_conta_%d.log", i);
      logFile[i] = fopen(fileName, "w");
      if (logFile[i] == NULL) {
         perror("Erro ao abrir o arquivo de extrato");
         exit(1);
      }
      fprintf(logFile[i], "=== Conta %d | Saldo Inicial: %.2f ===\n", i, INITIAL_BALANCE);
   }
}

// add amount to bankAccountBalance
void deposit(int account, double amount) {
   //pthread_mutex_lock( &lock );
   pthread_mutex_lock( &lock[account] );
   bankAccountBalance[account] += amount;
   totalDeposited[account] += amount;
   fprintf(logFile[account], "Thread %lu | DEPOSITO | Valor: %.2f | Novo Saldo: %.2f\n",
           (unsigned long)pthread_self(), amount, bankAccountBalance[account]);
  // pthread_mutex_unlock( &lock );
   pthread_mutex_unlock( &lock[account] );
}

// subtract amount from bankAccountBalance
void withdraw(int account, double amount) {
  // pthread_mutex_lock( &lock );
   pthread_mutex_lock( &lock[account] );
   if (bankAccountBalance[account] >= amount) {
      bankAccountBalance[account] -= amount;
      totalWithdrawn[account] += amount;
      fprintf(logFile[account], "Thread %lu | SAQUE | Valor: %.2f | Novo Saldo: %.2f\n",
              (unsigned long)pthread_self(), amount, bankAccountBalance[account]);
   } else {
      fprintf(logFile[account], "Thread %lu | SAQUE NEGADO (Saldo Insuficiente) | Tentativa: %.2f\n",
              (unsigned long)pthread_self(), amount);
   }
  // pthread_mutex_unlock( &lock );
   pthread_mutex_unlock( &lock[account] );
}

double getBalance(int account) {
   pthread_mutex_lock( &lock[account] );
   double balance = bankAccountBalance[account];
   fprintf(logFile[account], "Thread %lu | CONSULTA DE SALDO | Saldo Atual: %.2f\n",
           (unsigned long)pthread_self(), balance);
   pthread_mutex_unlock( &lock[account] );
   return balance;
}

void transfer(int from, int to, double amount) {
   if (from == to) return;
   if (from < to) {
      pthread_mutex_lock( &lock[from] );
      pthread_mutex_lock( &lock[to] );
   } else {
      pthread_mutex_lock( &lock[to] );
      pthread_mutex_lock( &lock[from] );
   }
   if (bankAccountBalance[from] >= amount) {
      bankAccountBalance[from] -= amount;
      bankAccountBalance[to]   += amount;
      fprintf(logFile[from], "Thread %lu | TRANSF. ENVIADA para Conta %d | Valor: %.2f | Saldo: %.2f\n",
              (unsigned long)pthread_self(), to, amount, bankAccountBalance[from]);
      fprintf(logFile[to], "Thread %lu | TRANSF. RECEBIDA da Conta %d | Valor: %.2f | Saldo: %.2f\n",
              (unsigned long)pthread_self(), from, amount, bankAccountBalance[to]);
   } else {
      fprintf(logFile[from], "Thread %lu | TRANSF. NEGADA para Conta %d (Sem Saldo) | Tentativa: %.2f\n",
              (unsigned long)pthread_self(), to, amount);
   }
   pthread_mutex_unlock( &lock[from] );
   pthread_mutex_unlock( &lock[to] );
}

void cleanup() {
   for (int i = 0; i < NUM_ACCOUNTS; i++) {
      fclose(logFile[i]);
      pthread_mutex_destroy(&lock[i]);
   }
}

